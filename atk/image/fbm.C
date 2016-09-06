/* fbm.c - class description for interface FBM format to image */
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
 * adapted from code by Michael Mauldin, (mlm) at Carnegie-Mellon
 * University, (fbm tools) and Kirk L. Johnson, (tuna@athena.mit.edu),
 * (gif.c).
 *
 * fbmin.c
 * Mark Majhor
 * August 1990
 *
 * routines for reading FBM files
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


ATK_IMPL("fbm.H")
#include <stdio.h>
#include <math.h>
#include <ctype.h>
#include <image.H>

/*#include <fbm.h>*/
/*****************************************************************
 * fbm.h: FBM Library 0.9 (Beta Test)  07-Mar-89  Michael Mauldin
 *
 * Copyright (C) 1989 by Michael Mauldin.  Permission is granted to
 * use this file in whole or in part provided that you do not sell it
 * for profit and that this copyright notice is retained unchanged.
 *
 * fbm.h: Fuzzy Bitmap Definition
 *
 *****************************************************************/
typedef unsigned char BYTE;	/* 8 bits unsigned		*/

# define FBM_MAX_TITLE		80		/* For title and credits */

# define BIG			1		/* msb first byte order */
# define LITTLE			0		/* lsb first byte order */

#define FBMIN_SUCCESS       0   /* success */

#define FBMIN_ERR_BAD_SD   -1   /* bad screen descriptor */
#define FBMIN_ERR_BAD_SIG  -2   /* bad signature */
#define FBMIN_ERR_EOD      -3   /* unexpected end of raster data */
#define FBMIN_ERR_EOF      -4   /* unexpected end of input stream */
#define FBMIN_ERR_FAO      -5   /* file already open */
#define FBMIN_ERR_IAO      -6   /* image already open */
#define FBMIN_ERR_NFO      -7   /* no file open */
#define FBMIN_ERR_NIO      -8   /* no image open */

# define FBM_MAGIC	"%bitmap"

/* FBM bitmap headers in files (null terminated 12 character ascii strings) */
typedef struct fbm_filehdr_struct {
	char	magic[8];		/* 2 bytes FBM_MAGIC number */
	char	cols[8];		/* Width in pixels */
	char	rows[8];		/* Height in pixels */
	char	planes[8];		/* Depth (1 for B+W, 3 for RGB) */
	char	bits[8];		/* Bits per pixel */
	char	physbits[8];		/* Bits to store each pixel */
	char	rowlen[12];		/* Length of a row in bytes */
	char	plnlen[12];		/* Length of a plane in bytes */
	char	clrlen[12];		/* Length of colormap in bytes */
	char	aspect[12];		/* ratio of Y to X of one pixel */
	char	title[FBM_MAX_TITLE];	/* Null terminated title */
	char	credits[FBM_MAX_TITLE];	/* Null terminated credits */
} FBMFILEHDR;

#include <fbm.H>

/****
 **
 ** local variables
 **
 ****/

static BYTE file_open = 0;	/* status flags */
static BYTE image_open = 0;

static FILE *ins;		/* input stream */
static FBMFILEHDR phdr;		/* header structure */

/****
 **
 ** global variables
 **
 ****/

static unsigned int  fbmin_img_width;           /* image width */
static unsigned int  fbmin_img_height;          /* image height */
static unsigned int  fbmin_img_depth;	       /* image depth */
static unsigned int  fbmin_img_bits;	       /* color bits */
static unsigned int  fbmin_img_rowlen;	       /* length of one row of data */
static unsigned int  fbmin_img_plnlen;	       /* length of one plane of data */
static unsigned int  fbmin_img_clrlen;	       /* length of the colormap */
static unsigned int  fbmin_img_aspect;	       /* image aspect ratio */
static unsigned int  fbmin_img_physbits;	       /* physical bits per pixel */
static char *fbmin_img_title;		/* name of image */
static char *fbmin_img_credit;		/* credit for image */

/*
 * open FBM image in the input stream; returns FBMIN_SUCCESS if
 * successful. (might also return various FBMIN_ERR codes.)
 */

ATKdefineRegistry(fbm, image, NULL);
static int fbmin_open_image(FILE  *s);
static int fbmin_close_file();
static int fbmin_image_test();
static void tellAboutImage(const char  *name);


static int fbmin_open_image(FILE  *s)
{
  char *hp;		/* header pointer */

  /* make sure there isn't already a file open */
  if (file_open)
    return(FBMIN_ERR_FAO);

  /* remember that we've got this file open */
  file_open = 1;
  ins = s;

  /* read in the fbm file header */
  hp = (char *) &phdr;
  if (fread((byte *)hp, 1, sizeof(phdr), ins) != sizeof(phdr))
    return FBMIN_ERR_EOF;

  if (strncmp(FBM_MAGIC, phdr.magic, sizeof(FBM_MAGIC)) != 0)
    return FBMIN_ERR_BAD_SIG;

  /* Now extract relevant features of FBM file header */
  fbmin_img_width    = atoi(phdr.cols);
  fbmin_img_height   = atoi(phdr.rows);
  fbmin_img_depth    = atoi(phdr.planes);
  fbmin_img_bits     = atoi(phdr.bits);
  fbmin_img_rowlen   = atoi(phdr.rowlen);
  fbmin_img_plnlen   = atoi(phdr.plnlen);
  fbmin_img_clrlen   = atoi(phdr.clrlen);
  fbmin_img_aspect   = atoi(phdr.aspect);
  fbmin_img_physbits = atoi(phdr.physbits);
  fbmin_img_title    = phdr.title;
  fbmin_img_credit   = phdr.credits;

  if (fbmin_image_test() != FBMIN_SUCCESS)
    return FBMIN_ERR_BAD_SD;

  return FBMIN_SUCCESS;
}

/*
 * close an open FBM file
 */

static int fbmin_close_file()
{
  /* make sure there's a file open */
  if (!file_open)
    return FBMIN_ERR_NFO;

  /* mark file (and image) as closed */
  file_open  = 0;
  image_open = 0;

  /* done! */
  return FBMIN_SUCCESS;
}
    
static int fbmin_image_test()
{
  if (fbmin_img_width < 1 || fbmin_img_width > 32767) {
    fprintf (stderr, "Invalid width (%d) on input\n", fbmin_img_width);
    return FBMIN_ERR_BAD_SD;
  }

  if (fbmin_img_height < 1 || fbmin_img_height > 32767) {
    fprintf (stderr, "Invalid height (%d) on input\n", fbmin_img_height);
    return (0);
  }

  if (fbmin_img_depth != 1 && fbmin_img_depth != 3) {
    fprintf (stderr, "Invalid number of planes (%d) on input %s\n",
	     fbmin_img_depth, "(must be 1 or 3)");
    return FBMIN_ERR_BAD_SD;
  }

  if (fbmin_img_bits < 1 || fbmin_img_bits > 8) {
    fprintf (stderr, "Invalid number of bits (%d) on input %s\n",
	     fbmin_img_bits, "(must be [1..8])");
    return FBMIN_ERR_BAD_SD;
  }

  if (fbmin_img_physbits != 1 && fbmin_img_physbits != 8) {
    fprintf (stderr, "Invalid number of physbits (%d) on input %s\n",
	     fbmin_img_physbits, "(must be 1 or 8)");
    return FBMIN_ERR_BAD_SD;
  }

  if (fbmin_img_rowlen < 1 || fbmin_img_rowlen > 32767) {
    fprintf (stderr, "Invalid row length (%d) on input\n",
	     fbmin_img_rowlen);
    return FBMIN_ERR_BAD_SD;
  }

  if (fbmin_img_depth > 1 && fbmin_img_plnlen < 1) {
    fprintf (stderr, "Invalid plane length (%d) on input\n",
	     fbmin_img_plnlen);
    return FBMIN_ERR_BAD_SD;
  }

  if (fbmin_img_aspect < 0.01 || fbmin_img_aspect > 100.0) {
    fprintf (stderr, "Invalid aspect ratio %d on input\n",
	     fbmin_img_aspect);
    return FBMIN_ERR_BAD_SD;
  }
    return FBMIN_SUCCESS;
}

/*
 * these are the routines added for interfacing to xloadimage
 */

/*
 * tell someone what the image we're loading is.  this could be a little more
 * descriptive but I don't care
 */

static void tellAboutImage(const char  *name)
     {
  printf("%s is a %dx%d FBM image with %d colors\n", name,
    fbmin_img_width, fbmin_img_height, fbmin_img_clrlen / 3);
}

int
fbm::Load( const char  *fullname, FILE  *fp )
            { 
  FILE *f;
  unsigned int x, y, j, k, rowlen, plnlen;
  unsigned char *pixptr, *cm;
  unsigned char *r, *g, *b;

  if((f = fp) == 0) {
      if (! (f = fopen(fullname, "r"))) {
	  fprintf(stderr, "Couldn't open fbm file %s.\n", fullname);
	  return(-1);
      }
  }

  if (fbmin_open_image(f) != FBMIN_SUCCESS) {  /* read image header */
    fbmin_close_file();
    if(!fp) fclose(f);
    return(-1);
  }

  (this)->newRGBImage( fbmin_img_width, fbmin_img_height, fbmin_img_bits);

  /* if image has a local colormap, override global colormap
   */
  if (fbmin_img_clrlen > 0) {
    cm = (unsigned char *) malloc(fbmin_img_clrlen);

    if (fread(cm, 1, fbmin_img_clrlen, ins) != fbmin_img_clrlen) {
      fprintf (stderr, "can't read colormap (%d bytes)\n", fbmin_img_clrlen);
      return(-1);
    }
    /*
     * fbm color map is organized as
     * buf[3][16]
     */
    y = fbmin_img_clrlen / 3;
    r = &cm[0], g = &cm[y], b = &cm[2 * y];
    for (x = 0; x < y; x++, r++, g++, b++) {
	(this)->RedPixel( x) = *r << 8;
	(this)->GreenPixel( x) = *g << 8;
	(this)->BluePixel( x) = *b << 8;
   }
    (this)->RGBUsed() = y;

  } else
    cm = NULL;

  rowlen = fbmin_img_rowlen;
  plnlen = fbmin_img_plnlen;

  for (k = 0; k < fbmin_img_depth; k++) {
    pixptr = &((this)->Data()[k * plnlen]);

    for (j = 0; j < fbmin_img_height; j++, pixptr += rowlen) {
      if (fread(pixptr, 1, rowlen, ins) != rowlen) {
	printf("%s: Short read within image data\n", fullname);
        exit(1);
      }
    }
  }

  if (cm != NULL)
    free(cm);

  fbmin_close_file();
  if(!fp) fclose(f);
  return(0);
}

int 
fbm::Ident( const char  *fullname )
        {
  FILE        *f;
  unsigned int  ret;

  if (! (f = fopen(fullname, "r")))
    return(0);
  if (fbmin_open_image(f) == FBMIN_SUCCESS) {
    tellAboutImage(fullname);
    ret = 1;
  } else
    ret = 0;
  fbmin_close_file();
  fclose(f);
  return(ret);
}

long
fbm::Read( FILE  *file, long  id )
            {
    if((this)->Load( NULL, file) == 0)
	return(dataobject_NOREADERROR);
    else
	return(dataobject_BADFORMAT);
}

long
fbm::Write( FILE  *file, long  writeID, int  level )
                {
    return((this)->image::Write( file, writeID, level));
}

long
fbm::WriteNative( FILE  *file, const char  *filename )
            {
return(0);
}
