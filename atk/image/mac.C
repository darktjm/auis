/* mac.c - class description for interface from MACpaint format to image */
/*
	Copyright Carnegie Mellon University 1992 - All rights reserved
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

/*
 *
 * adapted from code by Patrick Naughton (naughton@sun.soe.clarkson.edu)
 *
 * macin.c
 * Mark Majhor
 * August 1990
 *
 * routines for reading MAC files
 *
 * Copyright 1990 Mark Majhor (see the included file
 * "mrmcpyrght.h" for complete copyright information)
 */
#ifndef _MRM_COPYRIGHT_

/****
  Copyright 1990 Mark Majhor

  Permission to use, copy, modify, distribute, and sell this
  software and its documentation for any purpose is hereby granted
  without fee, provided that the above copyright notice appear in
  all copies and that both that copyright notice and this
  permission notice appear in supporting documentation. The
  author makes no representations about the suitability of this
  software for any purpose. It is provided "as is" without express
  or implied warranty.

  THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
  INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS,
  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, INDIRECT
  OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
  LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT,
  NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
  CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
****/

#include <andrewos.h>

#ifndef __SABER__
static UNUSED const char MRMCopyright[] = "Copyright 1990 Mark Majhor";
#endif
#define _MRM_COPYRIGHT_
#endif

ATK_IMPL("mac.H")
#include <stdio.h>
#include <math.h>
#include <ctype.h>
#include <image.H>
/*#include <mac.h>*/
/****************************************************************
 * mac.h:
 *
 * adapted from code by Patrick Naughton (naughton@sun.soe.clarkson.edu)
 *
 * macin.h
 * Mark Majhor
 * August 1990
 *
 * routines for reading MAC files
 *
 * Copyright 1990 Mark Majhor (see the included file
 * "mrmcpyrght.h" for complete copyright information)
 *
 ****************************************************************/
# define MAC_MAGIC	0x0

typedef unsigned char BYTE;	/* 8 bits unsigned		*/

/*
 * macin return codes
 */
#define MACIN_SUCCESS       0   /* success */

#define MACIN_ERR_BAD_SD   -1   /* bad screen descriptor */
#define MACIN_ERR_BAD_SIG  -2   /* bad signature */
#define MACIN_ERR_EOD      -3   /* unexpected end of raster data */
#define MACIN_ERR_EOF      -4   /* unexpected end of input stream */
#define MACIN_ERR_FAO      -5   /* file already open */
#define MACIN_ERR_IAO      -6   /* image already open */
#define MACIN_ERR_NFO      -7   /* no file open */
#define MACIN_ERR_NIO      -8   /* no image open */

static int macin_open_image();
static int macin_close_file();
#if 0
static int macin_fatal();
#endif

#define	MAC_HDR_LEN	512
#define ADD_HDR_LEN	128
#define	MAX_LINES	720
#define	BYTES_LINE	72


#include <mac.H>

/****
 **
 ** local variables
 **
 ****/

static BYTE file_open = 0;	/* status flags */
static BYTE image_open = 0;

static FILE *ins;		/* input stream */

/****
 **
 ** global variables
 **
 ****/

static int  macin_img_width;           /* image width */
static int  macin_img_height;          /* image height */
static int  macin_img_depth;	       /* image depth */
static int  macin_img_planes;	       /* image planes */
static int  macin_img_BPL;	       /* image bytes per line */

/*
 * open MAC image in the input stream; returns MACIN_SUCCESS if
 * successful. (might also return various MACIN_ERR codes.)
 */
/* ARGSUSED */

ATKdefineRegistry(mac, image, NULL);
static int macin_open_image(FILE  *s);
static int macin_close_file();
#if 0
static void macin_fatal(char  *msg);
#endif
static void tellAboutImage(char  *name);


static int macin_open_image(FILE  *s)
{
  BYTE mhdr[MAC_HDR_LEN];
  char *hp;		/* header pointer */

  /* make sure there isn't already a file open */
  if (file_open)
    return(MACIN_ERR_FAO);

  /* remember that we've got this file open */
  file_open = 1;
  ins = s;

  /*
   * the mac paint files that came with xmac had an extra
   * 128 byte header on the front, with a image name in it.
   * true mac paint images don't seem to have this extra
   * header.  The following code tries to figure out what
   * type the image is and read the right amount of file
   * header (512 or 640 bytes).
   */
  /* read in the mac file header */
  hp = (char *) mhdr;
  if (fread((byte *)hp, 1, ADD_HDR_LEN, ins) != ADD_HDR_LEN)
    return MACIN_ERR_EOF;

  if (mhdr[0] != MAC_MAGIC)
    return MACIN_ERR_BAD_SD;

  /* Get image name  (if available) */
  if (mhdr[1] != 0) {				/* if name header */
    if (fread((byte *)hp, 1, MAC_HDR_LEN, ins) != MAC_HDR_LEN)
      return MACIN_ERR_EOF;
  } else
    /* else read rest of header */
    if (fread((byte *)hp, 1, MAC_HDR_LEN - ADD_HDR_LEN, ins) != MAC_HDR_LEN - ADD_HDR_LEN)
      return MACIN_ERR_EOF;

  /* Now set relevant values */
  macin_img_width  = BYTES_LINE * 8;
  macin_img_height = MAX_LINES;
  macin_img_depth  = 1;		/* always monochrome */
  macin_img_planes = 1;		/* always 1 */
  macin_img_BPL    = BYTES_LINE;

  return MACIN_SUCCESS;
}

/*
 * close an open MAC file
 */

static int macin_close_file()
{
  /* make sure there's a file open */
  if (!file_open)
    return MACIN_ERR_NFO;

  /* mark file (and image) as closed */
  file_open  = 0;
  image_open = 0;

  /* done! */
  return MACIN_SUCCESS;
}

#if 0
/*
 * semi-graceful fatal error mechanism
 */

static void macin_fatal(char  *msg)
     {
  printf("Error reading MacPaint file: %s\n", msg);
  exit(0);
}
#endif

/*
 * these are the routines added for interfacing to xloadimage
 */

/*
 * tell someone what the image we're loading is.  this could be a little more
 * descriptive but I don't care
 */

static void tellAboutImage(char  *name)
{
  printf("%s is a %dx%d MacPaint image\n",
    name, macin_img_width, macin_img_height);
}

int
mac::Load( char  *fullname, FILE  *fp )
            { 
  FILE *f;
  BYTE *pixptr, ch;
  int eof;
  register int scanLine;
  register unsigned int i, j, k;

  if((f = fp) == 0) {
      if (! (f = fopen(fullname, "r"))) {
	  fprintf(stderr, "Couldn't open mac file %s.\n", fullname);
	  return(-1);
      }
  }
  if (macin_open_image(f) != MACIN_SUCCESS) {  /* read image header */
    macin_close_file();
    if(!fp) fclose(f);
    return(-1);
  }

  (this)->newBitImage( macin_img_width, macin_img_height);

  pixptr = &((this)->Data()[0]);
  scanLine = 0; k = 0;

  while (scanLine < macin_img_height) {
      if ((eof = fgetc(f)) == -1) break;
      ch = (BYTE) eof;	/* Count byte */
      i = (unsigned int) ch;
      if (ch < 0x80) {	/* Unpack next (I+1) chars as is */
	  for (j = 0; j <= i; j++) {
	      if (scanLine < macin_img_height) {
		  if ((eof = fgetc(f)) == -1) break;
		  *pixptr++ = (BYTE) eof;
		  k++;
		  if (!(k %= BYTES_LINE)) {
		      scanLine++;
		  }
	      }
	  }
      } else {	/* Repeat next char (2's comp I) times */
	  if ((eof = fgetc(f)) == -1) break;
	  ch = (BYTE) eof;
	  for (j = 0; j <= 256 - i; j++) {
	      if (scanLine < macin_img_height) {
		  *pixptr++ = (BYTE) ch;
		  k++;
		  if (!(k %= BYTES_LINE)) {
		      scanLine++;
		  }
	      }
	  }
      }
  }
  if (scanLine < macin_img_height) {
      if(!fp) fclose(f);
      return -1;
  }
  macin_close_file();

  if(!fp) fclose(f);
  return(0);
}

int 
mac::Ident( char  *fullname )
        {
  FILE        *f;
  unsigned int  ret;

  if (! (f = fopen(fullname, "r")))
    return(0);
  if (macin_open_image(f) == MACIN_SUCCESS) {
    tellAboutImage(fullname);
    ret = 1;
  } else
    ret = 0;
  macin_close_file();
  fclose(f);
  return(ret);
}

long
mac::Read( FILE  *file, long  id )
            {
    if((this)->Load( NULL, file) == 0)
	return(dataobject_NOREADERROR);
    else
	return(dataobject_BADFORMAT);
}

long
mac::Write( FILE  *file, long  writeID, int  level )
                {
    return((this)->image::Write( file, writeID, level));
}

long
mac::WriteNative( FILE  *file, char  *filename )
            {
return(0);
}
