/* sunraster.c - class description for interface from Sun raster format to image */
/*
	Copyright Carnegie Mellon University 1992,94 - All rights reserved
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

/* sunraster.c:
 *
 * sun rasterfile image type
 *
 * jim frost 09.27.89
 *
 * Copyright 1989, 1991 Jim Frost.
 * See included file "copyright.h" for complete copyright information.
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

ATK_IMPL("sunraster.H")
#include <image.H>
/*#include <sunraster.h>*/
/* sunraster.h
 *
 * this describes the header for Sun rasterfiles.  if you have SunOS, a
 * better description is in /usr/include/rasterfile.h.  this is used
 * instead to improve portability and to avoid distribution problems.
 *
 * Copyright 1989 Jim Frost.  See included file "copyright.h" for complete
 * copyright information.
 */
struct rheader {
  unsigned char magic[4];   /* magic number */
  unsigned char width[4];   /* width of image in pixels */
  unsigned char height[4];  /* height of image in pixels */
  unsigned char depth[4];   /* depth of each pixel */
  unsigned char length[4];  /* length of the image in bytes */
  unsigned char type[4];    /* format of file */
  unsigned char maptype[4]; /* type of colormap */
  unsigned char maplen[4];  /* length of colormap in bytes */
};

/* following the header is the colormap (unless maplen is zero) then
 * the image.  each row of the image is rounded to 2 bytes.
 */

#define RMAGICNUMBER 0x59a66a95 /* magic number of this file type */

/* these are the possible file formats
 */

#define ROLD       0 /* old format, see /usr/include/rasterfile.h */
#define RSTANDARD  1 /* standard format */
#define RRLENCODED 2 /* run length encoding to compress the image */
#define RRGB       3 /* RGB-format instead of BGR in 24 or 32-bit mode */
#define RTIFF      4 /* TIFF <-> rasterfile */
#define RIFF       5 /* IFF (TAAC) <-> rasterfile */

/* these are the possible colormap types.  if it's in RGB format,
 * the map is made up of three byte arrays (red, green, then blue)
 * that are each 1/3 of the colormap length.
 */

#define RNOMAP  0 /* no colormap follows the header */
#define RRGBMAP 1 /* rgb colormap */
#define RRAWMAP 2 /* raw colormap; good luck */

#define RESC 128 /* run-length encoding escape character */



#include <sunraster.H>

/* SUPPRESS 558 */
/* SUPPRESS 560 */


ATKdefineRegistry(sunraster, image, NULL);
static void babble(char            *name, struct rheader  *header);
static void sunread(FILE         *f, byte          *buf, unsigned int len, unsigned int enc  /* true if encoded file */);


static void babble(char            *name, struct rheader  *header)
          {
  printf("%s is a", name);
  switch (memToVal(header->type, 4)) {
  case ROLD:
    printf("n old-style");
    break;
  case RSTANDARD:
    printf(" standard");
    break;
  case RRLENCODED:
    printf(" run-length encoded");
    break;
  case RRGB:
    printf(" RGB"); /* RGB format instead of BGR */
    break;
  case RTIFF:
    printf(" TIFF");
    break;
  case RIFF:
    printf(" RIFF");
    break;
  default:
    printf(" unknown-type");
  }
  printf(" %ldx%ld", memToVal(header->width, 4), memToVal(header->height, 4));

  switch (memToVal(header->depth, 4)) {
  case 1:
    printf(" monochrome");
    break;
  case 8:
    printf(" 8 plane %s",
	   memToVal(header->maplen, 4) > 0 ? "color" : "greyscale");
    break;
  case 24:
    printf(" 24 plane color");
    break;

  case 32:
    /* isn't it nice how the sunraster.h file doesn't bother to mention that
     * 32-bit depths are allowed?
     */
    printf(" 32 plane color");
    break;
  }
  printf(" Sun rasterfile\n");
}

int 
sunraster::Ident( char  *fullname )
        { FILE          *f;
  struct rheader  header;
  int             r;

  if(!(f = fopen(fullname, "r"))) {
    perror("sunrasterIdent");
    return(0);
  }
  switch (fread((byte*)&header, sizeof(struct rheader), 1, f)) {
      case -1:
	  perror("sunrasterIdent");
	  r= 0;
	  break;

      case sizeof(struct rheader):
	  if (memToVal(header.magic, 4) != RMAGICNUMBER) {
	      r = 0;
	      break;
	  }
	  babble(fullname, &header);
	  r = 1;
	  break;

      default:
	  r = 0;
	  break;
  }
  fclose(f);
  return(r);
}

/* read either rl-encoded or normal image data
 */

static void sunread(FILE         *f, byte          *buf, unsigned int len, unsigned int enc  /* true if encoded file */)
                    { static byte repchar, remaining= 0;

  /* rl-encoded read
   */

  if (enc) {
    while (len--)
      if (remaining) {
	remaining--;
	*(buf++)= repchar;
      }
      else {
	if (fread(&repchar, sizeof(byte), 1, f) != 1) {
	  printf("sunrasterLoad: Bad read on image data\n");
	  exit(1);
	}
	if (repchar == RESC) {
	  if (fread(&remaining, sizeof(byte), 1, f) != 1) {
	    printf("sunrasterLoad: Bad read on image data\n");
	    exit(1);
	  }
	  if (remaining == 0)
	    *(buf++)= RESC;
	  else {
	    if (fread(&repchar, sizeof(byte), 1, f) != 1) {
	      printf("sunrasterLoad: Bad read on image data\n");
	      exit(1);
	    }
	    *(buf++)= repchar;
	  }
	}
	else
	  *(buf++)= repchar;
      }
  }

/* normal read
 */

  else {
    if (fread(buf, sizeof(byte), len, f) < len) {
      printf("sunrasterLoad: Bad read on image data\n");
      exit(1);
    }
  }
}

int
sunraster::Load( char  *fullname, FILE  *fp )
            { FILE          *f;
  struct rheader  header;
  unsigned int    mapsize;
  byte           *map;
  byte           *mapred, *mapgreen, *mapblue;
  unsigned int    depth;
  unsigned int    linelen;   /* length of raster line in bytes */
  unsigned int    fill;      /* # of fill bytes per raster line */
  unsigned int    enc;
  byte            fillchar;
  byte           *lineptr;
  unsigned int    x, y, size;

  if((f = fp) == 0) {
      if (! (f = fopen(fullname, "r"))) {
	  fprintf(stderr, "Couldn't open sunraster file %s.\n", fullname);
	  return(-1);
      }
  }
  switch (size = fread((byte*)&header, sizeof(struct rheader), 1, f)) {
      case -1:
	  if(!fp) fclose(f);
	  return(-1);

      case 1 /* sizeof(struct rheader) */:
	  if (memToVal(header.magic, 4) != RMAGICNUMBER) {
	      if(!fp) fclose(f);
	      return(-1);
	  }
	  break;

      default:
	  if(!fp) fclose(f);
	  return(-1);
  }

/* get an image to put the data in
 */

  depth = memToVal(header.depth, 4);
  switch(depth) {
      case 1:
	  (this)->newBitImage( memToVal(header.width, 4), memToVal(header.height, 4));
	  break;
      case 8:
	  (this)->newRGBImage( memToVal(header.width, 4), memToVal(header.height, 4), memToVal(header.depth, 4));
	  break;
      case 24:
      case 32:
	  (this)->newTrueImage( memToVal(header.width, 4), memToVal(header.height, 4));
	  break;
      default:
	  printf("sunrasterLoad: Bad depth %d (only 1, 8, 24 are valid)\n", depth);
	  exit(1);
  }

/* set up the colormap
 */

  if (depth == 1)
    linelen = ((this)->Width() / 8) + ((this)->Width() % 8 ? 1 : 0);
  else
    linelen = (this)->Width() * (this)->Pixlen();
  fill = (linelen % 2 ? 1 : 0);
  /*
   *  Handle color...
   */
  if (mapsize = memToVal(header.maplen, 4)) {
    map = (byte *) malloc(mapsize);
    if (fread(map, sizeof(byte), mapsize, f) < mapsize) {
      return(-1);
    }
    mapsize /= 3;
    mapred = map;
    mapgreen = mapred + mapsize;
    mapblue = mapgreen + mapsize;
    if ((this)->RGBSize() == 0)
	(this)->newRGBMapData( mapsize);
    for (y = 0; y < mapsize; y++) {
	(this)->RedPixel( y) = (*(mapred++) << 8);
	(this)->GreenPixel( y) = (*(mapgreen++) << 8);
	(this)->BluePixel( y) = (*(mapblue++) << 8);
    }
    free(map);
    (this)->RGBUsed() = mapsize;
  }

  /*
   *  Handle 8-bit greyscale via a simple ramp function...
   */
  else if (depth == 8) {
    mapsize = 256*3;
    map = (byte *) malloc(mapsize);
    for (y = 0; y < 256; y += 1) {
      map[y] = map[256+y] = map[2*256+y] = y;
    }
    mapsize /= 3;
    mapred = map;
    mapgreen = mapred + mapsize;
    mapblue = mapgreen + mapsize;
    if ((this)->RGBSize() == 0)
	(this)->newRGBMapData( mapsize);
    for (y = 0; y < mapsize; y++) {
	(this)->RedPixel( y) = (*(mapred++) << 8);
	(this)->GreenPixel( y) = (*(mapgreen++) << 8);
	(this)->BluePixel( y) = (*(mapblue++) << 8);
    }
    free(map);
    (this)->RGBUsed() = mapsize;
  }
  /* 24-bit and 32-bit handle themselves.  currently we don't support
   * a colormap for them.
   */

  enc = (memToVal(header.type, 4) == RRLENCODED);
  lineptr = (this)->Data();

  /* if it's a 32-bit image, we read the line and then strip off the
   * top byte of each pixel to get truecolor format
   */

  if (depth >= 24) {
    unsigned char *buf, *bp;

    buf = (unsigned char *) malloc((this)->Width() * (depth == 24 ? 3 : 4));
    for ( y= 0; y < (this)->Height(); y++) {
      sunread(f, buf, (this)->Width() * (depth == 24 ? 3 : 4), enc);
      bp = buf;
      if (depth == 24) {
	   for (x = 0; x < (this)->Width(); x++) {
		*(lineptr++) = *(bp + 2); /* red */
		*(lineptr++) = *(bp + 1); /* green */
		*(lineptr++) = *bp;       /* blue */
		bp += 3;
	   }
      }
      else {
	   for (x = 0; x < (this)->Width(); x++) {
		*(lineptr++) = *(bp + 3); /* red */
		*(lineptr++) = *(bp + 2); /* green */
		*(lineptr++) = *(bp + 1); /* blue */
		bp += 4;
	   }
      }
      if (fill)
	sunread(f, &fillchar, fill, enc);
    }
    free((char *) buf);
  }
  else {
    for (y = 0; y < (this)->Height(); y++) {
      sunread(f, lineptr, linelen, enc);
      lineptr += linelen;
      if (fill)
	sunread(f, &fillchar, fill, enc);
    }
  }
  if(!fp) fclose(f);
  return(0);
}

long
sunraster::Read( FILE  *file, long  id )
            {
    if((this)->Load( NULL, file) == 0)
	return(dataobject_NOREADERROR);
    else
	return(dataobject_BADFORMAT);
}

long
sunraster::Write( FILE  *file, long  writeID, int  level )
                {
    return((this)->image::Write( file, writeID, level));
}

long
sunraster::WriteNative( FILE  *file, char  *filename )
            {
return(0);
}
