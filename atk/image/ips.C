/*
 * Copyright 1989, 1990, 1991, 1992 by John Bradley and
 *                       The University of Pennsylvania
 *
 * Permission to use, copy, and distribute for non-commercial purposes,
 * is hereby granted without fee, providing that the above copyright
 * notice appear in all copies and that both the copyright notice and this
 * permission notice appear in supporting documentation.
 *
 * The software may be modified for your own purposes, but modified versions
 * may not be distributed.
 *
 * This software is provided "as is" without any expressed or implied warranty.
 *
 * The author may be contacted via:
 *    US Mail:   John Bradley
 *               GRASP Lab, Room 301C
 *               3401 Walnut St.
 *               Philadelphia, PA  19104
 *
 *    Phone:     (215) 898-8813
 *    EMail:     bradley@cis.upenn.edu
 */

/*
	$Disclaimer: 
 * Permission to use, copy, modify, and distribute this software and its 
 * documentation for any purpose and without fee is hereby granted, provided 
 * that the above copyright notice appear in all copies and that both that 
 * copyright notice and this permission notice appear in supporting 
 * documentation, and that the name of IBM not be used in advertising or 
 * publicity pertaining to distribution of the software without specific, 
 * written prior permission. 
 *                         
 * THE COPYRIGHT HOLDERS DISCLAIM ALL WARRANTIES WITH REGARD 
 * TO THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF 
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL ANY COPYRIGHT 
 * HOLDER BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL 
 * DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, 
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE 
 * OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION 
 * WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 * 
 *  $
*/

#include <andrewos.h>
#include <math.h>
#include <image.H>
#include <print.H>
#define AUXMODULE 1
#include <imagev.H>

#define PIX2INCH 72.0   /* # of pixels per inch, at 100% scaling */

#define USNORMAL	0
#define A4		1
#define B5		2
#define USLEGAL		3
#define Bsize		4
#define A4by5		5
#define A35mm		6

/* sizes of pages in inches */
static double paperSize[7][2] = { { 8.500, 11.000},   /* US NORMAL */
				  { 8.267, 11.811},   /* A4 */
				  { 7.283, 10.630},   /* B5 */
				  { 8.500, 14.000},   /* US LEGAL */
				  {11.000, 17.000},   /* B-size */
				  { 3.875,  4.875},   /* 4 by 5 */
				  { 0.945,  1.417}};  /* 35mm (24x36) */

#if 0 /* unused */
/* size of l+r margin and t+b margin.  image is centered */
static double margins[7][2] = { { 1.000, 1.000},   /* US NORMAL */
				{ 1.000, 1.000},   /* A4 */
				{ 1.000, 1.000},   /* B5 */
				{ 1.000, 1.000},   /* US LEGAL */
				{ 1.000, 1.000},   /* B-size */
				{ 0.275, 0.275},   /* 4 by 5 */
				{ 0.078, 0.078}};  /* 35mm (24x36) */
#endif

/* RANGE forces a to be in the range b..c (inclusive) */
#define RANGE(a,b,c) { if ((a) < (b)) (a) = (b);  if ((a) > (c)) (a) = (c); }

static void setScale( class imagev  *self, int  xscale , int  yscale );
void writePS( class imagev  *self, FILE  *fp, int  *wpts , int  *hpts, int  toplevel );
static int rle_encode(byte  *scanline , byte  *rleline, int  wide);
static void psColorImage(FILE  *fp, boolean usereg);
static void psColorMap(FILE  *fp, int  color , int  nc, byte  *rmap , byte  *gmap , byte  *bmap);
static void psRleCmapImage(FILE  *fp, int color);
static void epsPreview(class imagev  *self, FILE  *fp, byte  *pic);
static int writeBWStip(FILE  *fp, byte  *pic, const char  *prompt, int   w , int   h);

/* local variables */
static double sz_inx, sz_iny;     /* image size, in inches */
static double pos_inx, pos_iny;   /* top-left offset of image, in inches */
static int dpix, dpiy;		/* # of image pixels per inch */
static double psizex, psizey;	/* current paper size, in inches */
static int colorType; 

/***************************************************/
static void setPaper()
{
  psizex = paperSize[USNORMAL][0];
  psizey = paperSize[USNORMAL][1];
}

/***************************************************/
static void setScale( class imagev  *self, int  xscale , int  yscale )
        {
    class image *image = (class image *) (self)->GetDataObject();
    double hsx, hsy;
    int w = (image)->Width() , h = (image)->Height();

    sz_inx = (double) w / PIX2INCH * (xscale / 100.0);  
    sz_iny = (double) h / PIX2INCH * (yscale / 100.0);  

  /* round to integer .001ths of an inch */
    sz_inx = floor(sz_inx * 1000.0 + 0.5) / 1000.0;
    sz_iny = floor(sz_iny * 1000.0 + 0.5) / 1000.0;

    dpix = (int) (PIX2INCH / (xscale / 100.0));  
    dpiy = (int) (PIX2INCH / (yscale / 100.0));  

  /* make sure 'center' of image is still on page */
    hsx = sz_inx/2;  hsy = sz_iny/2;
    RANGE(pos_inx, -hsx, psizex-hsx);
    RANGE(pos_iny, -hsy, psizey-hsy);

  /* round to integer .001ths of an inch */
   pos_inx = floor(pos_inx * 1000.0 + 0.5) / 1000.0;
   pos_iny = floor(pos_iny * 1000.0 + 0.5) / 1000.0;
}


#if 0 /* unused */
/***************************************************/
static void centerImage(void)
{
  pos_inx = psizex/2 - sz_inx/2;
  pos_iny = psizey/2 - sz_iny/2;

  /* round to integer .001ths of an inch */
  pos_inx = floor(pos_inx * 1000.0 + 0.5) / 1000.0;
  pos_iny = floor(pos_iny * 1000.0 + 0.5) / 1000.0;
}
#endif


/***************************************************/
void writePS( class imagev  *self, FILE  *fp, int  *wpts , int  *hpts, int  toplevel )
                {
    class image *imagep = (class image *) (self)->GetDataObject();
    class image *new_c = new image;
    int i, j, q, err, rpix, gpix, bpix, nc;
    int iw, ih, ox, oy, slen, lwidth, bits, colorps = 1, w, h;
    byte *inpix, rmap[256], gmap[256], bmap[256];
    boolean rle = TRUE;

    if(self->orig && imagep != self->orig)
	imagep = self->orig;
    sz_inx = sz_iny = pos_inx = pos_iny = psizex = psizey = 0;
    colorType = (imagep)->Type();
    dpix = dpiy = 0;
    nc = (imagep)->RGBUsed();
    w = (imagep)->Width();
    h = (imagep)->Height();

    setPaper();
    setScale(self, 100, 100);

  /* printed image will have size iw,ih (in picas) */
    iw = (int) (sz_inx * 72.0 + 0.5);
    if(wpts) *wpts = iw;

    ih = (int) (sz_iny * 72.0 + 0.5);   
    if(hpts) *hpts = ih;
  /* compute offset to bottom-left of image (in picas) */
    ox = (int) (pos_inx * 72.0 + 0.5);
    oy = (int) ((psizey - (pos_iny + sz_iny)) * 72.0 + 0.5);

    if(toplevel == imagev_PURE_POSTSCRIPT) {
	/*** write PostScript header ***/
	fprintf(fp,"%%!PS-Adobe-2.0 EPSF-2.0\n");
	fprintf(fp,"%%%%Creator: ATK image\n");

	fprintf(fp,"%%%%BoundingBox: %d %d %d %d\n", ox, oy, ox+iw, oy+ih);

	fprintf(fp,"%%%%Pages: 1\n");
	fprintf(fp,"%%%%DocumentFonts:\n");
	fprintf(fp,"%%%%EndComments\n");
    }

   (imagep)->Duplicate( new_c);
   switch (colorType) {
       case IRGB:
	   slen = w;
	   bits = 8;
	   colorps = 1;
	   break;
       case ITRUE:
	   (new_c)->Reduce( 256);
	   slen = w*3;
	   bits = 8;
	   colorps = 1;
	   break;
       case IBITMAP:
	   slen = (w+7)/8;
	   bits = 1;
	   colorps = 0;
	   break;
       case IGREYSCALE:
	   slen = w;
	   bits = 8;
	   colorps = 1;
	   break;
       default:
	   break;
   }
   colorType = (new_c)->Type();
   inpix = (new_c)->Data();
   nc = (new_c)->RGBUsed();
   if(! BITMAPP(new_c) )
       for(i = 0; i < nc; i++) {
	   *(rmap + i) = (new_c)->RedPixel( i) >> 8;
	   *(gmap + i) = (new_c)->GreenPixel( i) >> 8;
	   *(bmap + i) = (new_c)->BluePixel( i) >> 8;
       }

   if(FALSE)
       epsPreview(self, fp, inpix);

   if(toplevel == 0){
       fprintf(fp,"%%%%EndProlog\n\n");
       fprintf(fp,"%%%%Page: 1 1\n\n");

       fprintf(fp,"%% remember original state\n");
       fprintf(fp,"/origstate save def\n\n");
   }

   if (toplevel == imagev_REGISTERED_POSTSCRIPT) {
       print::PSRegisterDef("imageDict", "20 dict");
       fprintf(fp,"%% open the temporary dictionary\n");
       fprintf(fp,"imageDict begin\n\n");
   }
   else {
       fprintf(fp,"%% build a temporary dictionary\n");
       fprintf(fp,"20 dict begin\n\n");
   }

   if (colorType == IBITMAP || !rle) {
       fprintf(fp,"%% define string to hold a scanline's worth of data\n");
       fprintf(fp,"/pix %d string def\n\n", slen);
   }

   if (toplevel == imagev_PURE_POSTSCRIPT) {
       fprintf(fp,"%% lower left corner\n");
       fprintf(fp,"%d %d translate\n\n",ox,oy);
   }

   fprintf(fp,"%% size of image (on paper, in 1/72inch coords)\n");
   fprintf(fp,"%d %d scale\n\n",iw,ih);

   if (colorType == IBITMAP) {   /* 1-bit dither code uses 'image' */
       fprintf(fp,"%% dimensions of data\n");
       fprintf(fp,"%d %d %d\n\n",w,h,bits);

       fprintf(fp,"%% mapping matrix\n");
       fprintf(fp,"[%d 0 0 %d 0 %d]\n\n", w, -h, h);

       fprintf(fp,"{currentfile pix readhexstring pop}\n");
       fprintf(fp,"image\n");
    
    /* write the actual image data */

       err = writeBWStip(fp, inpix, "", w, h);
   }

   else {      /* all other formats */
       byte *rleline;
       unsigned long outbytes = 0;
       const char *defname = colorps ? "imageRleCImage" : "imageRleGImage";

    /* imake sure 'colorimage' is defined IF we're gonna use it */
       if (colorps /*&& !rle */) {
	   if (toplevel == imagev_REGISTERED_POSTSCRIPT) {
	       print::PSRegisterHeader("imageColorImage", (print_header_fptr)psColorImage, (void *)TRUE);
	   }
	   else {
	       psColorImage(fp, FALSE);
	   }
       }
    
       if (rle) {   /* write colormap and rle-colormapped image funct */
	   psColorMap(fp, colorps, nc, rmap, gmap, bmap);
	   if (toplevel == imagev_REGISTERED_POSTSCRIPT) {
	       print::PSRegisterDef(defname, "0");
	       print::PSRegisterHeader(defname, (print_header_fptr)psRleCmapImage, (void *)(long)colorps);
	   }
	   else
	       psRleCmapImage(fp, colorps);
       }

       fprintf(fp,"%d %d %d\t\t\t%% dimensions of data\n",w,h,bits);
       fprintf(fp,"[%d 0 0 %d 0 %d]\t\t%% mapping matrix\n", w, -h, h);

       if (rle) fprintf(fp,"%s\n", defname);
       else {
	   fprintf(fp,"{currentfile pix readhexstring pop}\n");
	   if (colorps) fprintf(fp,"false 3 colorimage\n");
	   else fprintf(fp,"image\n");
       }

    /* dump the image data to the file */
       err = 0;

       if (rle) {
	   rleline  = (byte *) malloc(w * 2);  /* much worse than possible */
       }

       for (i = 0; i < h && err != EOF; i++) {
	   int rlen;
	   lwidth = 0;
	   putc('\n',fp);

	   if (rle) {  /* write rle-encoded colormapped image data */
	       rlen = rle_encode(inpix, rleline, w);
	       inpix += w;
	       outbytes += rlen;

	       for (j=0; j<rlen && err != EOF; j++) {
		   err = fprintf(fp,"%02x", rleline[j]);
		   lwidth += 2;

		   if (lwidth>70) { putc('\n',fp); lwidth = 0; }
	       }
	   }
	   else {  /* write non-rle raw (gray/rgb) image data */
	       for (j=0; j<w && err != EOF; j++) {
		   rpix = rmap[*inpix];
		   gpix = gmap[*inpix];
		   bpix = bmap[*inpix];
	  
		   if (colorps) { 
		       err = fprintf(fp,"%02x%02x%02x",rpix,gpix,bpix);
		       lwidth+=6;
		   }
      
		   else {  /* greyscale */
		       q = (rpix*21 + gpix*32 + bpix*11) / 64;
		       err = fprintf(fp,"%02x", q);
		       lwidth+=2;
		   }

		   if (lwidth>70) { putc('\n',fp); lwidth = 0; }
		   inpix++;
	       }
	   }
       }

       if (rle) {
	   free(rleline);
	   fprintf(fp,"\n\n");
	   fprintf(fp,"%%\n");
	   fprintf(fp,"%% Compression made this file %.2lf%% %s\n",
		   100.0 * ((double) outbytes) / 
		   ((double) (new_c)->Width() * (new_c)->Height() * ((colorps) ? 3 : 1)),
		   "of the uncompressed size.");
	   fprintf(fp,"%%\n");
       }
   }

   if(toplevel == imagev_PURE_POSTSCRIPT) 
       fprintf(fp,"\n\nshowpage\n\n");

   fprintf(fp,"%% stop using temporary dictionary\n");
   fprintf(fp,"end\n\n");

   if(toplevel == 0) {
       fprintf(fp,"%% restore original state\n");
       fprintf(fp,"origstate restore\n\n");
   }

   if(toplevel == imagev_PURE_POSTSCRIPT) 
       fprintf(fp,"%%%%Trailer\n");

   (new_c)->Destroy();
}


/**********************************************/
static int rle_encode(byte  *scanline , byte  *rleline, int  wide)
          {
  /* generates a rle-compressed version of the scan line.
   * rle is encoded as such:
   *    <count> <value>                      # 'run' of count+1 equal pixels
   *    <count | 0x80> <count+1 data bytes>  # count+1 non-equal pixels
   *
   * count can range between 0 and 127
   *
   * returns length of the rleline vector
   */
  
  int  i, j, blocklen, isrun, rlen;
  byte block[256], pix;
  
  blocklen = isrun = rlen = 0;

  for (i=0; i<wide; i++) {
    /* there are 5 possible states:
     *   0: block empty.
     *   1: block not empty, block is  a run, current pix == previous pix
     *   2: block not empty, block is  a run, current pix != previous pix
     *   3: block not empty, block not a run, current pix == previous pix
     *   4: block not empty, block not a run, current pix != previous pix
     */

    pix = scanline[i];

    if (!blocklen) {                    /* case 0:  empty */
      block[blocklen++] = pix;
      isrun = 1;
    }

    else if (isrun) {
      if (pix == block[blocklen-1]) {   /* case 1:  isrun, prev==cur */
	block[blocklen++] = pix;
      }
      else {                            /* case 2:  isrun, prev!=cur */
	if (blocklen>1) {               /*   we have a run block to flush */
	  rleline[rlen++] = blocklen-1;
	  rleline[rlen++] = block[0];
	  block[0] = pix;               /*   start new run block with pix */
	  blocklen = 1;
	}
	else {
	  isrun = 0;                    /*   blocklen<=1, turn into non-run */
	  block[blocklen++] = pix;
	}
      }
    }
	
    else {   /* not a run */
      if (pix == block[blocklen-1]) {   /* case 3:  non-run, prev==cur */
	if (blocklen>1) {               /*  have a non-run block to flush */
	  rleline[rlen++] = (blocklen-1) | 0x80;
	  for (j=0; j<blocklen; j++)
	    rleline[rlen++] = block[j];

	  block[0] = pix;               /*  start new run block with pix */
	  blocklen = isrun = 1;
	}
	else {
	  isrun = 1;                    /*  blocklen<=1 turn into a run */
	  block[blocklen++] = pix;
	}
      }
      else {                            /* case 4:  non-run, prev!=cur */
	block[blocklen++] = pix;
      }
    }

    if (blocklen == 128) {   /* max block length.  flush */
      if (isrun) {
	rleline[rlen++] = blocklen-1;
	rleline[rlen++] = block[0];
      }

      else {
	rleline[rlen++] = (blocklen-1) | 0x80;
	for (j=0; j<blocklen; j++) 
	  rleline[rlen++] = block[j];
      }

      blocklen = 0;
    }
  }

  if (blocklen) { /* flush last block */
      if (isrun) {
	  rleline[rlen++] = blocklen-1;
	  rleline[rlen++] = block[0];
      }

      else {
	  rleline[rlen++] = (blocklen-1) | 0x80;
	  for (j=0; j<blocklen; j++) 
	      rleline[rlen++] = block[j];
      }
  }
  return rlen;
}
	  
	    
/**********************************************/
static void psColorImage(FILE  *fp, boolean usereg)
{
  /* spits out code that checks if the PostScript device in question knows about the 'colorimage' operator.  If it doesn't, it defines 'colorimage' in terms of image (ie, generates a greyscale image from RGB data) */

  if (usereg) {
      fprintf(fp,"imageDict begin\n");
  }
  fprintf(fp,"%% define 'colorimage' if it isn't defined\n");
  fprintf(fp,"%%   ('colortogray' and 'mergeprocs' come from xwd2ps\n");
  fprintf(fp,"%%     via xgrab)\n");
  fprintf(fp,"/colorimage where   %% do we know about 'colorimage'?\n");
  fprintf(fp,"  { pop }           %% yes: pop off the 'dict' returned\n");
  fprintf(fp,"  {                 %% no:  define one\n");
  fprintf(fp,"    /colortogray {  %% define an RGB->I function\n");
  fprintf(fp,"      /rgbdata exch store    %% call input 'rgbdata'\n");
  fprintf(fp,"      rgbdata length 3 idiv\n");
  fprintf(fp,"      /npixls exch store\n");
  fprintf(fp,"      /rgbindx 0 store\n");
  fprintf(fp,"      /grays npixls string store  %% str to hold the result\n");
  fprintf(fp,"      0 1 npixls 1 sub {\n");
  fprintf(fp,"        grays exch\n");
  fprintf(fp,"        rgbdata rgbindx       get 20 mul    %% Red\n");
  fprintf(fp,"        rgbdata rgbindx 1 add get 32 mul    %% Green\n");
  fprintf(fp,"        rgbdata rgbindx 2 add get 12 mul    %% Blue\n");
  fprintf(fp,"        add add 64 idiv      %% I = .5G + .31R + .18B\n");
  fprintf(fp,"        put\n");
  fprintf(fp,"        /rgbindx rgbindx 3 add store\n");
  fprintf(fp,"      } for\n");
  fprintf(fp,"      grays\n");
  fprintf(fp,"    } bind def\n\n");

  fprintf(fp,"    %% Utility procedure for colorimage operator.\n");
  fprintf(fp,"    %% This procedure takes two procedures off the\n");
  fprintf(fp,"    %% stack and merges them into a single procedure.\n\n");
  
  fprintf(fp,"    /mergeprocs { %% def\n");
  fprintf(fp,"      dup length\n");
  fprintf(fp,"      3 -1 roll\n");
  fprintf(fp,"      dup\n");
  fprintf(fp,"      length\n");
  fprintf(fp,"      dup\n");
  fprintf(fp,"      5 1 roll\n");
  fprintf(fp,"      3 -1 roll\n");
  fprintf(fp,"      add\n");
  fprintf(fp,"      array cvx\n");
  fprintf(fp,"      dup\n");
  fprintf(fp,"      3 -1 roll\n");
  fprintf(fp,"      0 exch\n");
  fprintf(fp,"      putinterval\n");
  fprintf(fp,"      dup\n");
  fprintf(fp,"      4 2 roll\n");
  fprintf(fp,"      putinterval\n");
  fprintf(fp,"    } bind def\n\n");

  fprintf(fp,"    /colorimage { %% def\n");
  fprintf(fp,"      pop pop     %% remove 'false 3' operands\n");
  fprintf(fp,"      {colortogray} mergeprocs\n");
  fprintf(fp,"      image\n");
  fprintf(fp,"    } bind def\n");
  fprintf(fp,"  } ifelse          %% end of 'false' case\n");
  if (usereg) {
      fprintf(fp,"end\n");
  }
  fprintf(fp,"\n\n\n");
}


/**********************************************/
static void psColorMap(FILE  *fp, int  color , int  nc, byte  *rmap , byte  *gmap , byte  *bmap)
               {
  /* spits out code for the colormap of the following image
     if !color, it spits out a mono-ized graymap */

  int i;

  fprintf(fp,"%% define the colormap\n");
  fprintf(fp,"/cmap %d string def\n\n\n", nc * ((color) ? 3 : 1));

  fprintf(fp,"%% load up the colormap\n");
  fprintf(fp,"currentfile cmap readhexstring\n");

  for (i=0; i<nc; i++) {
    fprintf(fp,"%02x%02x%02x ", rmap[i],gmap[i],bmap[i]);
    if ((i%10) == 9) fprintf(fp,"\n");
  }
  if (i%10) fprintf(fp,"\n");
  fprintf(fp,"pop pop   %% lose return values from readhexstring\n\n\n");
		 
}

/**********************************************/
static void psRleCmapImage(FILE  *fp, int color)
{
  const char *defname = color ? "imageRleCImage" : "imageRleGImage";
  /* spits out code that defines the 'rlecmapimage' or 'rlegmapimage' operator */

  fprintf(fp,"%% procedure expects to have 'w h bits matrix' on stack\n");
  fprintf(fp,"/%s {\n", defname);
  fprintf(fp,"  /buffer 1 string def\n");
  fprintf(fp,"  /rgbval 3 string def\n");
  fprintf(fp,"  /block  384 string def\n\n");

  fprintf(fp,"  %% proc to read a block from file, and return RGB data\n");
  fprintf(fp,"  { currentfile buffer readhexstring pop\n");
  fprintf(fp,"    /bcount exch 0 get store\n");
  fprintf(fp,"    bcount 128 ge\n");
  fprintf(fp,"    {  %% it's a non-run block\n");
  fprintf(fp,"      0 1 bcount 128 sub\n");
  fprintf(fp,"      { currentfile buffer readhexstring pop pop\n\n");

  if (color) {
    fprintf(fp,"        %% look up value in color map\n");
    fprintf(fp,"        /rgbval cmap buffer 0 get 3 mul 3 getinterval store\n\n");
    fprintf(fp,"        %% and put it in position i*3 in block\n");
    fprintf(fp,"        block exch 3 mul rgbval putinterval\n");
    fprintf(fp,"      } for\n");
    fprintf(fp,"      block  0  bcount 127 sub 3 mul  getinterval\n");
    fprintf(fp,"    }\n\n");
  }
  else {
    fprintf(fp,"        %% look up value in gray map\n");
    fprintf(fp,"        /rgbval cmap buffer 0 get 1 getinterval store\n\n");
    fprintf(fp,"        %% and put it in position i in block\n");
    fprintf(fp,"        block exch rgbval putinterval\n");
    fprintf(fp,"      } for\n");
    fprintf(fp,"      block  0  bcount 127 sub  getinterval\n");
    fprintf(fp,"    }\n\n");
  }

  fprintf(fp,"    { %% else it's a run block\n");
  fprintf(fp,"      currentfile buffer readhexstring pop pop\n\n");

  if (color) {
    fprintf(fp,"      %% look up value in colormap\n");
    fprintf(fp,"      /rgbval cmap buffer 0 get 3 mul 3 getinterval store\n\n");
    fprintf(fp,"      0 1 bcount { block exch 3 mul rgbval putinterval } for\n\n");
    fprintf(fp,"      block 0 bcount 1 add 3 mul getinterval\n");
  }
  else {
    fprintf(fp,"      %% look up value in graymap\n");
    fprintf(fp,"      /rgbval cmap buffer 0 get 1 getinterval store\n\n");
    fprintf(fp,"      0 1 bcount { block exch rgbval putinterval } for\n\n");
    fprintf(fp,"      block 0 bcount 1 add getinterval\n");
  }

  fprintf(fp,"    } ifelse\n");
  fprintf(fp,"  } %% end of proc\n");

  if (color) fprintf(fp,"  false 3 colorimage\n");
        else fprintf(fp,"  image\n");

  fprintf(fp,"} bind def\n\n\n");
}



/**********************************************/
static void epsPreview(class imagev  *self, FILE  *fp, byte  *pic)
            {
    class image *image = (class image *) (self)->GetDataObject();
    byte *prev;
    int w = (image)->Width(), h = (image)->Height();

  /* put in an EPSI preview */
  
    if (colorType != IBITMAP) { /* have to generate a preview */
     (image)->Dither();
    }
    prev = (image)->Data();
  
    fprintf(fp,"%%%%BeginPreview: %d %d %d %d\n", w, h, 1, 
	    (w/(72*4) + 1) * h);

    writeBWStip(fp, prev, "% ", w, h);

    fprintf(fp,"%%%%EndPreview\n");
}

static unsigned char hex[16] = {
	'0', '1', '2', '3', '4', '5', '6', '7',
	'8', '9', 'a', 'b', 'c', 'd', 'e', 'f'
};

static unsigned char invhex[16] = {
	'f', 'e', 'd', 'c', 'b', 'a', '9', '8',
	'7', '6', '5', '4', '3', '2', '1', '0'
};

/***********************************/
static int writeBWStip(FILE  *fp, byte  *pic, const char  *prompt, int   w , int   h)
                {
  /* write the given 'pic' (B/W stippled, 1 byte per pixel, 0=blk,1=wht) out as hexadecimal, max of 72 hex chars per line. returns '0' if everythings fine, 'EOF' if writing failed */
  int err = 0, i, j, lwidth;
  int widthbytes = (w+7) >> 3;
  unsigned char *tbl = ((TRUE) ? invhex : hex);

  for (i = 0, lwidth = 0; i < h; i++) {
      fprintf(fp, "%s", prompt);
      for (j = 0; j < widthbytes; j++) {
	  fputc(*(tbl + ((*pic) / 16)), fp); 
	  fputc(*(tbl + ((*pic) & 15)), fp);
	  pic++;
	  lwidth += 2;
	  if (lwidth >= 72) { 
	      fprintf(fp, "\n%s", prompt); 
	      lwidth = 0; 
	  }
      }
      fprintf(fp, "\n");
  }
  return err;
}




