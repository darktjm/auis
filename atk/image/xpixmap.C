/* xpixmap.c - class description for interface from Xpixmap format to image */
/*
	Copyright Carnegie Mellon University 1992 - All rights reserved
*/

/* xpixmap.c:
 *
 * XPixMap format file read and identify routines.  these can handle any
 * "format 1" XPixmap file with up to BUFSIZ - 1 chars per pixel.  it's
 * not nearly as picky as it might be.
 *
 * unlike most image loading routines, this is X specific since it
 * requires X color name parsing.  to handle this we have global X
 * variables for display and screen.  it's ugly but it keeps the rest
 * of the image routines clean.
 *
 * Copyright 1989 Jim Frost.  See included file "copyright.h" for complete
 * copyright information.
 */
#ifndef _JIM_COPYRIGHT_
/*
 * Copyright 1989, 1993 Jim Frost
 *
 * Permission to use, copy, modify, distribute, and sell this software
 * and its documentation for any purpose is hereby granted without fee,
 * provided that the above copyright notice appear in all copies and
 * that both that copyright notice and this permission notice appear
 * in supporting documentation.  The author makes no representations
 * about the suitability of this software for any purpose.  It is
 * provided "as is" without express or implied warranty.
 *
 * THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN
 * NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS
 * OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE
 * OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE
 * USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <andrewos.h>

#ifndef __SABER__
static UNUSED const char Copyright[]= "Copyright 1989, 1993 Jim Frost";
#endif
#define _JIM_COPYRIGHT_
#endif

ATK_IMPL("xpixmap.H")
#include <X11/X.h>
#include <X11/Xlib.h>
#include <environ.H>
#include <image.H>
#include <xpixmap.H>

/* SUPPRESS 530 */
/* SUPPRESS 560 */

static char host[256];
static Display *Disp; /* X display, null if in "identify" mode */
static int      Scrn; /* X screen number */

#define XPM_FORMAT 1


ATKdefineRegistry(xpixmap, image, NULL);

static void corrupted(const char   *fullname, FILE  *f)
{
  fclose(f);
  printf("%s: X Pixmap file is corrupted\n", fullname);
}

int
xpixmap::Load( const char  *fullname, FILE  *fp )
{ FILE          *f;
  class xpixmap *self=this;
  char           buf[BUFSIZ];
  const char     *hostp;
  unsigned int   value;
  unsigned int   w, h;    /* image dimensions */
  unsigned int   cpp;     /* chars per pixel */
  unsigned int   ncolors; /* number of colors */
  unsigned int   depth;   /* depth of image */
  char         **ctable;  /* color table */
  XColor         xcolor;
  unsigned int   a, x, y;
  int            c;
  byte          *dptr;
  char		 colorName[20], *quote;
  char		 colorCode[10], formatStr[20];

  if((f = fp) == 0) {
      if (! (f = fopen(fullname, "r"))) {
	  fprintf(stderr, "Couldn't open xpixmap file %s.\n", fullname);
	  return(-1);
      }
  }

  if ((hostp = environ::Get("DISPLAY")))
      strcpy(host, hostp);
  else
      strcpy(host, "unix:0");
  Disp = XOpenDisplay(host);
  Scrn = DefaultScreen(Disp);
 
  /* read #defines until we have all that are necessary or until we
   * get an error
   */

  w= h= ncolors= 0;
  for (;;) {
    if (! fgets(buf, BUFSIZ - 1, f)) {
      if(!fp) fclose(f);
      return(-1);
    }
    if(*buf != '"')
	continue;
    if (sscanf(buf, "\"%d %d %d %d\"", &w, &h, &ncolors, &cpp) != 4) {
      if(!fp) fclose(f);
      return(-1);
    }
    else break;
  }

  if (w <= 0 || h <= 0 || ncolors <= 0 || cpp <= 0) {
    if(!fp) fclose(f);
    return(-1);
  }

  while (((c= fgetc(f)) != EOF) && (c != '"')) ;
  ungetc(c, f);

  for (depth= 1, value= 2; value < ncolors; value <<= 1, depth++)
    ;
  (self)->newRGBImage(w, h, depth);
  (self)->RGBUsed()= ncolors;

  /* read the colors array and build the image colormap
   */

  ctable= (char **)malloc(sizeof(char *) * ncolors);
  xcolor.flags= DoRed | DoGreen | DoBlue;
  sprintf(formatStr, "\"%%%dc c %%[^\"]", cpp);

  for (a= 0; a < ncolors; a++) {
 
    /* read pixel value & color def and parse it 
     */

    *(ctable + a)= (char *)malloc(cpp);
    fgets(buf, BUFSIZ - 1, f);

    sscanf(buf, formatStr, colorCode, colorName);

    colorCode[cpp] = (char) 0;
    if((quote = strchr(colorName, '"')))
	*quote = (char)0;

    strcpy(*(ctable+a), colorCode);
 
    if (Disp) {
      if (! XParseColor(Disp, DefaultColormap(Disp, Scrn), colorName, &xcolor)) {
	printf("%s: %s: Bad color name\n", fullname, colorName);
	return(-1);
      }
      (self)->RedPixel(a) = xcolor.red;
      (self)->GreenPixel(a) = xcolor.green;
      (self)->BluePixel(a) = xcolor.blue;
    }
  }


  /* read in image data
   */

  dptr = (self)->Data();
  for (y= 0; y < h; y++) {
    while (((c= fgetc(f)) != EOF) && (c != '"'))
      ;
    for (x= 0; x < w; x++) {
      for (a= 0; a < cpp; a++) {
	if ((c= fgetc(f)) == '\\')
	  c= fgetc(f);
	if (c == EOF) {
	  corrupted(fullname, f);
	  return(-1);
	}
	buf[a]= (char)c;
      }
      for (a= 0; a < ncolors; a++)
	if (!strncmp(*(ctable + a), buf, cpp))
	  break;
      if (a == ncolors) { /* major uncool */
	if(!fp) fclose(f);
	printf("%s: Pixel data doesn't match color data\n", fullname);
	return(-1);
      }
      valToMem((unsigned long)a, dptr, (self)->Pixlen());
      dptr += (self)->Pixlen();
    }
    if ((c= fgetc(f)) != '"') {
      corrupted(fullname, f);
      return(-1);
    }
  }
  if(!fp) fclose(f);
  return(0);
}

long
xpixmap::Read( FILE  *file, long  id )
{  
    if((this)->Load(NULL, file) == 0)
	return(dataobject_NOREADERROR);
    else
	return(dataobject_BADFORMAT);
}

long
xpixmap::Write( FILE  *file, long  writeID, int  level )
{
    return((this)->image::Write(file, writeID, level));
}

long
xpixmap::WriteNative( FILE  *file, const char  *filename )
{
    return(0);
}
