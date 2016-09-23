/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1989 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

/*
 * Portions Copyright 1989, 1990, 1991, 1992 by John Bradley and
 *                                The University of Pennsylvania
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
#include <andrewos.h>
ATK_IMPL("jpeg.H")
#include <image.H>
#include <jpeg.H>
#include <jpeglib.h>
#include <setjmp.h>

static int pWIDE, pHIGH;
static byte *pic = NULL;

static int colorType, quality;
static unsigned int numColors;
static byte *image8, *image24;

static byte *CurImagePtr;
static byte *pic24 = NULL;
static long filesize;
struct my_error_mgr {
  struct jpeg_error_mgr pub;
  jmp_buf jmpState;
};

ATKdefineRegistry(jpeg, image, NULL);

static void xv_jpeg_monitor(j_common_ptr  cinfo);
static void d_ui_method_selection(j_decompress_ptr  cinfo);
static void output_init (j_decompress_ptr  cinfo);
int LoadJFIF(class jpeg  *jpeg, char  *fname, FILE  *f);
static void input_init (j_compress_ptr  cinfo);
static int writeJFIF(FILE  *fp);


long
jpeg::WriteNative(FILE  *file, const char  *filename)
{
    FILE *fp;
    int nc, rv;

  /* get the image into a format that the JPEG software can grok on.  Also, open the output file, so we don't waste time doing this format conversion if we won't be able to write it out */

    pWIDE = pHIGH = 0;
    pic = image24 = pic24 = CurImagePtr = NULL;

    pWIDE = (this)->Width();
    pHIGH = (this)->Height();
    colorType = (this)->Type();
    numColors = nc = (this)->RGBUsed();
  /* see if we can open the output file before proceeding */
    if(!(fp = file) &&
       !(fp = fopen(filename, "wb"))) {
	printf("jpeg: couldn't open file %s\n", filename);
       return(-1);
    }

   switch(colorType) {
	case IBITMAP:
	    (this)->Bit2Grey();
	case IGREYSCALE:
	    image8 = (this)->Data();
	    break;
	case IRGB:
	    (this)->Expand();
	    image24 = (this)->Data();
	    break;
	case ITRUE:
	    image24 = (this)->Data();
	    break;
    }
    colorType = (this)->Type();
    quality = this->saveQuality;
  /* in any event, we've got some valid image.  Do the JPEG Thing */
    rv = writeJFIF(fp);
    
    if(!file && fp)
	fclose(fp);
    return(rv);
}


/*********************************************/
/**** INTERFACE CODE FOR THE JPEG LIBRARY ****/
/*********************************************/

/********* JPEG DECOMPRESSION FUNCTIONS **********/

/**************************************************/
static void xv_jpeg_monitor(j_common_ptr  cinfo)
{
#ifdef FOO  
  int a,b;
  double percent;
  struct jpeg_progress_mgr progress = cinfo->progress;

  a = progress->completed_passes;
  b = progress->total_passes;

  percent = ((a + ((double) progress->pass_counter / progress->pass_limit)) / (double) b) * 100.0;

  fprintf(stderr,"jpeg: %lf done.  loop: %ld, %ld  pass: %d, %d\n",
	  percent, progress->pass_counter, progress->pass_limit, a, b);
#endif
}

static void
d_ui_method_selection(j_decompress_ptr  cinfo)
{
  /* select output colorspace & quantization parameters */
  if (cinfo->jpeg_color_space == JCS_GRAYSCALE) {
      cinfo->out_color_space = JCS_GRAYSCALE;
      cinfo->quantize_colors = FALSE;
  }
  else {
      cinfo->out_color_space = JCS_RGB;
  }
}


/**************************************************/
static void
output_init (j_decompress_ptr  cinfo)
{
  pWIDE = cinfo->image_width;
  pHIGH = cinfo->image_height;

  if (cinfo->out_color_space == JCS_GRAYSCALE || 
      cinfo->quantize_colors == TRUE) {
    pic = (byte *) malloc(pWIDE * pHIGH);
    CurImagePtr = pic;
  }
  else {
    pic24 = (byte *) malloc(pWIDE * pHIGH * 3);
    CurImagePtr = pic24;
  }
}

/**************************************************/
static void JPEG_Error(j_common_ptr cinfo)
{
  cinfo->err->output_message(cinfo);
  longjmp(((struct my_error_mgr *)cinfo)->jmpState, 1);
}

/*******************************************/
int
jpeg::Load( const char * volatile fname, FILE * volatile f )
/*******************************************/
{
  struct jpeg_decompress_struct cinfo = {};
  struct my_error_mgr jerr;
  struct jpeg_progress_mgr prog;
  JSAMPROW row_pointer[1];
  int row_stride;

  numColors = 0;
  pWIDE = pHIGH = 0;
  pic = image8 = image24 = pic24 = CurImagePtr = NULL;
  /* Set up the input file */

  if(f)
    fname = NULL;
  else if(!fname || !(f = fopen(fname, "rb"))) {
      fprintf(stderr, "Couldn't open jpeg file %s.\n", fname);
      return(-1);
  }

  long filepos=ftell(f);
  /* figure out the file size (for Informational Purposes Only) */
  fseek(f, 0L, 2);
  filesize = ftell(f);
  fseek(f, filepos, 0);

  /* Set up longjmp for error recovery out of JPEG_Error */
  if(setjmp(jerr.jmpState)) {
    jpeg_destroy_decompress(&cinfo);
    if(fname) fclose(f);	/* close input file */
    return(-1);		/* no further cleanup needed */
  }

  jerr.pub.error_exit = JPEG_Error; /* provide my own error/message rtns */
  jerr.pub.trace_level = 0;	/* no tracing, thank you */

  cinfo.err = jpeg_std_error(&jerr.pub);
  jpeg_create_decompress(&cinfo);

  /* set up our progress-monitoring function */
  prog.progress_monitor = xv_jpeg_monitor;
  cinfo.progress = &prog;

  jpeg_stdio_src(&cinfo, f);

  /* set defaults based on JFIF header */
  jpeg_read_header(&cinfo, TRUE);
  output_init(&cinfo);
  d_ui_method_selection(&cinfo);

  /* Do it! */
  jpeg_start_decompress(&cinfo);
  row_stride = cinfo.output_width * cinfo.output_components;
  while(cinfo.output_scanline < cinfo.output_height) {
    row_pointer[0] = &CurImagePtr[cinfo.output_scanline * row_stride];
    (void) jpeg_read_scanlines(&cinfo, row_pointer, 1);
  }
  jpeg_finish_decompress(&cinfo);
  
  if (pic24) {
      (this)->newTrueImage( cinfo.image_width, cinfo.image_height);
      memmove((this)->Data(), pic24, 3 * cinfo.image_width * cinfo.image_height);
      free(pic24);
  }
  else {
      unsigned int i, w = cinfo.image_width, h = cinfo.image_height;

      if(cinfo.out_color_space == JCS_GRAYSCALE)
	  (this)->newGreyImage( w, h, cinfo.data_precision);
      else
	  (this)->newRGBImage( w, h, cinfo.data_precision);
      memmove((this)->Data(), pic, w * h);
      if(cinfo.out_color_space == JCS_GRAYSCALE) {
	  (this)->RGBUsed() = (this)->RGBSize() = numColors = 256;
	  for(i = 0; i < numColors-1; i++)
	      (this)->RedPixel( i) =
		(this)->GreenPixel( i) =
		(this)->BluePixel( i) = i << 8;
      }
      else {
	  for(i = 0; i < numColors; i++) {
	      (this)->RedPixel( i) = cinfo.colormap[0][i] << 8;
	      (this)->GreenPixel( i) = cinfo.colormap[1][i] << 8;
	      (this)->BluePixel( i) = cinfo.colormap[2][i] << 8;
	  }
	  (this)->RGBUsed() = (this)->RGBSize() = numColors;
      }
      free(pic);
  }
  jpeg_destroy_decompress(&cinfo);
  /* Close input file */
  if(fname) fclose(f);

  /* Got it! */
  return 0;
}


/********* JPEG COMPRESSION FUNCTIONS **********/

/**************************************************/
static void input_init (j_compress_ptr  cinfo)
{
  int w,h;
  if (colorType == IGREYSCALE) {
    cinfo->input_components = 1;
    cinfo->in_color_space = JCS_GRAYSCALE;
    CurImagePtr = image8;
  }
  else {
    cinfo->input_components = 3;
    cinfo->in_color_space = JCS_RGB;
    CurImagePtr = image24;
  }

  w = pWIDE;  h = pHIGH;

  cinfo->image_width = w;
  cinfo->image_height = h;
  cinfo->data_precision = 8;
}


/*******************************************/
static int writeJFIF(FILE  *fp)
{
  int retval;
  struct jpeg_compress_struct cinfo = {};
  JSAMPROW row_pointer[1];
  int row_stride;
  struct my_error_mgr jerr;
  struct jpeg_progress_mgr prog;

  /* Set up longjmp for error recovery out of JPEG_Error */
  if((retval = setjmp(jerr.jmpState))) {
    jpeg_destroy_compress(&cinfo);
    return retval;		/* no further cleanup needed */
  }

  jerr.pub.error_exit = JPEG_Error; /* provide my own error/message rtns */
  jerr.pub.trace_level = 0;	/* no tracing, thank you */

  cinfo.err = jpeg_std_error(&jerr.pub);
  jpeg_create_compress(&cinfo);

/* set up our progress-monitoring function */
  prog.progress_monitor = xv_jpeg_monitor;
  cinfo.progress = &prog;

/* Select input and output */
  input_init(&cinfo);
  jpeg_stdio_dest(&cinfo, fp);

  jpeg_set_defaults(&cinfo);
  jpeg_set_quality(&cinfo, quality, FALSE);

  /* Do it! */
  jpeg_start_compress(&cinfo, TRUE);
  row_stride = cinfo.input_components * cinfo.image_width;
  while(cinfo.next_scanline < cinfo.image_height) {
    row_pointer[0] = &CurImagePtr[cinfo.next_scanline * row_stride];
    jpeg_write_scanlines(&cinfo, row_pointer, 1);
  }
  jpeg_finish_compress(&cinfo);
  jpeg_destroy_compress(&cinfo);
  return 0;
}

long
jpeg::Read( FILE  *file, long  id )
{
    if((this)->Load( NULL, file) == 0)
	return(dataobject_NOREADERROR);
    else
	return(dataobject_BADFORMAT);
}

long
jpeg::Write( FILE  *file, long  writeID, int  level )
{
    return((this)->image::Write( file, writeID, level));
}

int 
jpeg::Ident( const char  *fullname )
{
    FILE *f;
    int status = 0;
    boolean ret = FALSE;

    if((f = fopen(fullname, "r"))) {
	unsigned char c1, c2, c3, c4;
	char jfif[5];

	if(fscanf(f, "%c%c%c%c", &c1, &c2, &c3, &c4) == 4 &&
	   c1 == 0xFF && c2 == 0xD8 && c3 == 0xFF && c4 == 0xE0) {
	    getc(f); getc(f);
	    if((status = fscanf(f, "%4s", jfif)) == 1) {
		jfif[4] = (char)0;
		if(!strncmp(jfif, "JFIF", 4)) ret = TRUE;
	    }
	}
	fclose(f);
    }
    return(ret);
}

jpeg::jpeg( )
{
    this->saveQuality = 75;
    THROWONFAILURE((TRUE));
}

jpeg::~jpeg( ) {}
