/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1989 - All Rights Reserved      *
 *        For full copyright information see:'andrew/config/COPYRITE'     *
\* ********************************************************************** */

/*
	$Disclaimer: 
// Permission to use, copy, modify, and distribute this software and its 
// documentation for any purpose and without fee is hereby granted, provided 
// that the above copyright notice appear in all copies and that both that 
// copyright notice and this permission notice appear in supporting 
// documentation, and that the name of IBM not be used in advertising or 
// publicity pertaining to distribution of the software without specific, 
// written prior permission. 
//                         
// THE COPYRIGHT HOLDERS DISCLAIM ALL WARRANTIES WITH REGARD 
// TO THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF 
// MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL ANY COPYRIGHT 
// HOLDER BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL 
// DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, 
// DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE 
// OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION 
// WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
// 
//  $
*/

#ifdef NORCSID
#define NORCSID
static char rcsid[]="$Header: /afs/cs.cmu.edu/project/atk-src-C++/atk/basics/common/RCS/jpeg.C,v 3.9 1996/10/11 21:41:59 robr Exp $";
#endif


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
#include <setjmp.h>
#include <image.H>
#include <jpeg.H>
#include <jinclude.h>

static int pWIDE, pHIGH;
static byte r[256],g[256],b[256];
static byte *pic = NULL;

static int colorType, numColors, quality;
static byte *image8, *image24;

static byte *CurImagePtr;
static byte *pic24 = NULL;
static long filesize;
static jmp_buf jmpState;
static external_methods_ptr emethods;


ATKdefineRegistry(jpeg, image, NULL);

static void xv_jpeg_monitor_decompress(decompress_info_ptr  cinfo, long  loopcnt , long  looplimit);
static void xv_jpeg_monitor_compress(compress_info_ptr  cinfo, long  loopcnt , long  looplimit);
static void d_ui_method_selection(decompress_info_ptr  cinfo);
static void output_init (decompress_info_ptr  cinfo);
static void put_color_map (decompress_info_ptr  cinfo, int  num_colors, JSAMPARRAY  colormap);
static void put_pixel_rows (decompress_info_ptr  cinfo, int  num_rows, JSAMPIMAGE  pixel_data);
static void output_term (decompress_info_ptr  cinfo);
static void jselwxv(decompress_info_ptr  cinfo);
static void JPEG_Message (const char  *msgtext);
static void JPEG_Error (const char  *msgtext);
int LoadJFIF(class jpeg  *jpeg, char  *fname, FILE  *f);
static void c_ui_method_selection(compress_info_ptr  cinfo);
static void input_init (compress_info_ptr  cinfo);
static void get_input_row(compress_info_ptr  cinfo, JSAMPARRAY         pixel_row);
static void input_term (compress_info_ptr  cinfo);
static void jselrxv(compress_info_ptr  cinfo);
static int writeJFIF(FILE  *fp);


long
jpeg::WriteNative(FILE  *file, char  *filename)
{
    FILE *fp;
    int i, nc, rv, w, h, size;
    register byte *ip, *ep, *inpix;

  /* get the image into a format that the JPEG software can grok on.  Also, open the output file, so we don't waste time doing this format conversion if we won't be able to write it out */

    pWIDE = pHIGH = 0;
    pic = image24 = pic24 = CurImagePtr = NULL;

    pWIDE = (this)->Width();
    pHIGH = (this)->Height();
    colorType = (this)->Type();
    numColors = nc = (this)->RGBUsed();
  /* see if we can open the output file before proceeding */
    if(!(fp = file) &&
       !(fp = fopen(filename, "w"))) {
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
static void xv_jpeg_monitor_decompress(decompress_info_ptr  cinfo, long  loopcnt , long  looplimit)
{
#ifdef FOO  
  int a,b;
  double percent;

  a = cinfo->completed_passes;
  b = cinfo->total_passes;

  percent = ((a + ((double) loopcnt / looplimit)) / (double) b) * 100.0;

  fprintf(stderr,"jpeg: %lf done.  loop: %ld, %ld  pass: %d, %d\n",
	  percent, loopcnt, looplimit, a, b);
#endif
}
static void xv_jpeg_monitor_compress(compress_info_ptr  cinfo, long  loopcnt , long  looplimit)
{
#ifdef FOO  
  int a,b;
  double percent;

  a = cinfo->completed_passes;
  b = cinfo->total_passes;

  percent = ((a + ((double) loopcnt / looplimit)) / (double) b) * 100.0;

  fprintf(stderr,"jpeg: %lf done.  loop: %ld, %ld  pass: %d, %d\n",
	  percent, loopcnt, looplimit, a, b);
#endif
}

static void
d_ui_method_selection(decompress_info_ptr  cinfo)
{
  /* select output colorspace & quantization parameters */
  if (cinfo->jpeg_color_space == CS_GRAYSCALE) {
      int i;
      cinfo->out_color_space = CS_GRAYSCALE;
      cinfo->quantize_colors = FALSE;
  }
  else {
      cinfo->out_color_space = CS_RGB;
  }

  jselwxv(cinfo);
}


/**************************************************/
static void
output_init (decompress_info_ptr  cinfo)
{
  pWIDE = cinfo->image_width;
  pHIGH = cinfo->image_height;

  if (cinfo->out_color_space == CS_GRAYSCALE || 
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
static void
put_color_map (decompress_info_ptr  cinfo, int  num_colors, JSAMPARRAY  colormap)
{
  int i;
  numColors = num_colors;
  for (i = 0; i < num_colors; i++) {
      r[i] = GETJSAMPLE(colormap[0][i]);
      g[i] = GETJSAMPLE(colormap[1][i]);
      b[i] = GETJSAMPLE(colormap[2][i]);
  }
}

static void
put_pixel_rows (decompress_info_ptr  cinfo, int  num_rows, JSAMPIMAGE  pixel_data)
{
  JSAMPROW ptr0, ptr1, ptr2;
  long col;
  long width = cinfo->image_width;
  int row;
  static unsigned int totrows = 0;

  if (cinfo->out_color_space == CS_GRAYSCALE || 
      cinfo->quantize_colors == TRUE) {
      for (row = 0; row < num_rows; row++) {
	  ptr0 = pixel_data[0][row];
	  for (col = width; col > 0; col--) {
	      *CurImagePtr++ = GETJSAMPLE(*ptr0++);
	  }
	  totrows++;
      }
  }
  else {
      for (row = 0; row < num_rows; row++) {
	  ptr0 = pixel_data[0][row];
	  ptr1 = pixel_data[1][row];
	  ptr2 = pixel_data[2][row];
	  for (col = width; col > 0; col--) {
	      *CurImagePtr++ = GETJSAMPLE(*ptr0++);
	      *CurImagePtr++ = GETJSAMPLE(*ptr1++);
	      *CurImagePtr++ = GETJSAMPLE(*ptr2++);
	  }
	  totrows++;
      }
  }
}

static void output_term (decompress_info_ptr  cinfo)
{
}

static void jselwxv(decompress_info_ptr  cinfo)
     {
  cinfo->methods->output_init = output_init;
  cinfo->methods->put_color_map = put_color_map;
  cinfo->methods->put_pixel_rows = put_pixel_rows;
  cinfo->methods->output_term = output_term;
}

static void JPEG_Message (const char  *msgtext)
{
  char tempstr[200];

  sprintf(tempstr, msgtext,
	  emethods->message_parm[0], emethods->message_parm[1],
	  emethods->message_parm[2], emethods->message_parm[3],
	  emethods->message_parm[4], emethods->message_parm[5],
	  emethods->message_parm[6], emethods->message_parm[7]);
}


/**************************************************/
static void JPEG_Error (const char  *msgtext)
{
  char tempstr[200];
  
  sprintf(tempstr, msgtext,
	  emethods->message_parm[0], emethods->message_parm[1],
	  emethods->message_parm[2], emethods->message_parm[3],
	  emethods->message_parm[4], emethods->message_parm[5],
	  emethods->message_parm[6], emethods->message_parm[7]);
  fprintf(stderr, "%s\n", tempstr);
  (*emethods->free_all) ();	/* clean up memory allocation */
  longjmp(jmpState,1);
}

/*******************************************/
int
jpeg::Load( char  *fullname, FILE  *fp )
/*******************************************/
{
  FILE *f;
  if((f = fp) == 0) {
      if (! (f = fopen(fullname, "r"))) {
	  fprintf(stderr, "Couldn't open gif file %s.\n", fullname);
	  return(-1);
      }
  }
  return(LoadJFIF(this, fullname, f));
}

/*******************************************/
int LoadJFIF(class jpeg  *jpeg, char  *fname, FILE  *f)
{
  int rtval;
  /* These three structs contain JPEG parameters and working data.
   * They must survive for the duration of parameter setup and one
   * call to jpeg_decompress; typically, making them local data in the
   * calling routine is the best strategy.
   */
  struct Decompress_info_struct cinfo;
  struct Decompress_methods_struct dc_methods;
  struct External_methods_struct e_methods;

  numColors = 0;
  pWIDE = pHIGH = 0;
  pic = image8 = image24 = pic24 = CurImagePtr = NULL;
  /* Set up the input file */

  if((cinfo.input_file = f) == NULL && fname &&
       (cinfo.input_file = fopen(fname, "r")) == NULL)
      return(-1);
  
  long filepos=ftell(cinfo.input_file);
  /* figure out the file size (for Informational Purposes Only) */
  fseek(cinfo.input_file, 0L, 2);
  filesize = ftell(cinfo.input_file);
  fseek(cinfo.input_file, filepos, 0);

  /* Set up longjmp for error recovery out of JPEG_Error */
  if(setjmp(jmpState)) {
    fclose(cinfo.input_file);	/* close input file */
    return(-1);		/* no further cleanup needed */
  }

  cinfo.output_file = NULL;	/* if no actual output file involved */

  /* Initialize the system-dependent method pointers. */
  cinfo.methods  = &dc_methods;
  cinfo.emethods = &e_methods;
  emethods = &e_methods;	/* save struct addr for possible access */

  e_methods.error_exit = JPEG_Error; /* provide my own error/message rtns */
  e_methods.trace_message = JPEG_Message;
  e_methods.trace_level = 0;	/* no tracing, thank you */
  e_methods.num_warnings = 0;	/* no warnings emitted yet */
  e_methods.first_warning_level = 0; /* display first corrupt-data warning */
  e_methods.more_warning_level = 3; /* but suppress additional ones */
  jselmemmgr(&e_methods);	/* memory allocation routines */
  dc_methods.d_ui_method_selection = d_ui_method_selection;

  /* Set up default JPEG parameters. */
  j_d_defaults(&cinfo, TRUE);

  /* set up our progress-monitoring function */
  cinfo.methods->progress_monitor = xv_jpeg_monitor_decompress;
  
  /* Set up to read a JFIF or baseline-JPEG file. */
  /* A smarter UI would inspect the first few bytes of the input file */
  /* to determine its type. */
  jselrjfif(&cinfo);

  /* Do it! */
  jpeg_decompress(&cinfo);

  if (pic24) {
      (jpeg)->newTrueImage( cinfo.image_width, cinfo.image_height);
      memmove((jpeg)->Data(), pic24, 3 * cinfo.image_width * cinfo.image_height);
      free(pic24);
  }
  else {
      unsigned int i, w = cinfo.image_width, h = cinfo.image_height;

      if(cinfo.out_color_space == CS_GRAYSCALE)
	  (jpeg)->newGreyImage( w, h, cinfo.data_precision);
      else
	  (jpeg)->newRGBImage( w, h, cinfo.data_precision);
      memmove((jpeg)->Data(), pic, w * h);
      if(cinfo.out_color_space == CS_GRAYSCALE) {
	  (jpeg)->RGBUsed() = (jpeg)->RGBSize() = numColors = 256;
	  for(i = 0; i < numColors-1; i++)
	      (jpeg)->RedPixel( i) =
		(jpeg)->GreenPixel( i) =
		(jpeg)->BluePixel( i) = i << 8;
      }
      else {
	  for(i = 0; i < numColors; i++) {
	      (jpeg)->RedPixel( i) = r[i] << 8;
	      (jpeg)->GreenPixel( i) = g[i] << 8;
	      (jpeg)->BluePixel( i) = b[i] << 8;
	  }
	  (jpeg)->RGBUsed() = (jpeg)->RGBSize() = numColors;
      }
      free(pic);
  }
  /* Close input file */
  fclose(cinfo.input_file);

  /* Got it! */
  return 0;
}


/********* JPEG COMPRESSION FUNCTIONS **********/

/**************************************************/
static void c_ui_method_selection(compress_info_ptr  cinfo)
{
  /* If the input is gray scale, generate a monochrome JPEG file. */
  if (cinfo->in_color_space == CS_GRAYSCALE)
    j_monochrome_default(cinfo);
}


/**************************************************/
static void input_init (compress_info_ptr  cinfo)
{
  int w,h;
  if (colorType == IGREYSCALE) {
    cinfo->input_components = 1;
    cinfo->in_color_space = CS_GRAYSCALE;
    CurImagePtr = image8;
  }
  else {
    cinfo->input_components = 3;
    cinfo->in_color_space = CS_RGB;
    CurImagePtr = image24;
  }

  w = pWIDE;  h = pHIGH;

  cinfo->image_width = w;
  cinfo->image_height = h;
  cinfo->data_precision = 8;
}


/**************************************************/
static void get_input_row(compress_info_ptr  cinfo, JSAMPARRAY         pixel_row)
{
  JSAMPROW ptr0, ptr1, ptr2;
  long col;
  static unsigned int totrows = 0;

  if (cinfo->input_components == 1) {
    ptr0 = pixel_row[0];
    for (col = cinfo->image_width; col > 0; col--) {
      *ptr0++ = *CurImagePtr++;
    }
    totrows++;
  }
  else {
    ptr0 = pixel_row[0];
    ptr1 = pixel_row[1];
    ptr2 = pixel_row[2];
    for (col = cinfo->image_width; col > 0; col--) {
      *ptr0++ = *CurImagePtr++;
      *ptr1++ = *CurImagePtr++;
      *ptr2++ = *CurImagePtr++;
    }
    totrows++;
  }
}


/**************************************************/
static void input_term (compress_info_ptr  cinfo)
{
  /* no work required */
}


/**************************************************/
static void jselrxv(compress_info_ptr  cinfo)
{
  cinfo->methods->input_init = input_init;
  cinfo->methods->get_input_row = get_input_row;
  cinfo->methods->input_term = input_term;
}



/*******************************************/
static int writeJFIF(FILE  *fp)
{
  int retval;
  struct Compress_info_struct cinfo;
  struct Compress_methods_struct c_methods;
  struct External_methods_struct e_methods;

  
  /* Initialize the system-dependent method pointers. */
  cinfo.methods  = &c_methods;
  cinfo.emethods = &e_methods;

  /* Set up longjmp for error recovery out of JPEG_Error */
  if(retval = setjmp(jmpState))
    return retval;		/* no further cleanup needed */

  e_methods.error_exit = JPEG_Error; /* provide my own error/message rtns */
  e_methods.trace_message = JPEG_Message;
  e_methods.trace_level = 0;	/* no tracing, thank you */
  jselmemmgr(&e_methods);	/* memory allocation routines */
  c_methods.c_ui_method_selection = c_ui_method_selection;

/* set up our progress-monitoring function */
  cinfo.methods->progress_monitor = xv_jpeg_monitor_compress;

/* Select input and output */
  cinfo.input_file  = NULL;
  cinfo.output_file = fp;       /* already open */

  j_c_defaults(&cinfo, quality, FALSE);
  jselrxv(&cinfo);		/* we'll be reading from memory */
  jselwjfif(&cinfo);		/* and writing to JFIF file format */

  /* Do it! */
  jpeg_compress(&cinfo);
  
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
jpeg::Ident( char  *fullname )
{
    FILE *f;
    int status = 0;
    boolean ret = FALSE;

    if(f = fopen(fullname, "r")) {
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
