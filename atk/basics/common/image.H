#ifndef _image_H_
#define _image_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1989 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

/*
 * Copyright 1989, 1990, 1991 Jim Frost
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

/** \addtogroup libbasics
 * @{ */
typedef unsigned long  Pixel;     /* what X thinks a pixel is */
typedef unsigned short Intensity; /* what X thinks an RGB intensity is */
typedef unsigned char  byte;      /* byte type */

typedef struct rgbmap {
  unsigned int  size;       /* size of RGB map */
  unsigned int  used;       /* number of colors used in RGB map */
  unsigned int  compressed; /* image uses colormap fully */
  Intensity    *red;        /* color values in X style */
  Intensity    *green;
  Intensity    *blue;
} RGBMap;

#define IBITMAP		0 /* image is a bitmap */
#define IGREYSCALE	1 /* image is 8-bit Greyscale */
#define IRGB		2 /* image is RGB */
#define ITRUE		3 /* image is true color */

#define NUM_IMAGE_TYPES 4

#define BITMAPP(self)		((self)->type == IBITMAP)
#define GREYSCALEP(self)	((self)->type == IGREYSCALE)
#define RGBP(self)		((self)->type == IRGB)
#define TRUEP(self)		((self)->type == ITRUE)

#define TRUE_RED(PIXVAL)   (((PIXVAL) & 0xff0000) >> 16)
#define TRUE_GREEN(PIXVAL) (((PIXVAL) & 0xff00) >> 8)
#define TRUE_BLUE(PIXVAL)  ((PIXVAL) & 0xff)
#define RGB_TO_TRUE(R,G,B) \
  ((((R) & 0xff00) << 8) | ((G) & 0xff00) | ((B) >> 8))

unsigned long doMemToVal(unsigned char *p, unsigned int len);
unsigned long doValToMem(unsigned long   val, unsigned char *p, unsigned int len);
unsigned long doMemToValLSB(unsigned char *p, unsigned int len);
unsigned long doValToMemLSB(unsigned long   val, unsigned char *p, unsigned int len);

/* special case 1-byte transfers so they're inline
 */

#define memToVal(PTR,LEN)    ((LEN) == 1 ? (unsigned long)(*(PTR)) : \
			      doMemToVal(PTR,LEN))
#define memToValLSB(PTR,LEN) ((LEN) == 1 ? (unsigned long)(*(PTR)) : \
			      doMemToValLSB(PTR,LEN))
#define valToMem(VAL,PTR,LEN)    ((LEN) == 1 ? \
				  (unsigned long)(*(PTR) = (byte)(VAL)) : \
				  doValToMem(VAL,PTR,LEN))
#define valToMemLSB(VAL,PTR,LEN) ((LEN) == 1 ? \
				  (unsigned long)(*(PTR) = (byte)(VAL)) : \
				  (int)doValToMemLSB(VAL,PTR,LEN))

/* this returns the (approximate) intensity of an RGB triple
 */

#define colorIntensity(R,G,B) \
  (RedIntensity[(R) >> 8] + GreenIntensity[(G) >> 8] + BlueIntensity[(B) >> 8])

#include <dataobject.H>

class image : public  dataobject  {
public:
	TRACED_CLASS(image);



   long Read( FILE *file, long id )  ;
   long Write( FILE *file, long writeID, int level )  ;
   const char * ViewName()  ;
   long WriteOtherFormat(FILE *file, long writeid, int level, dataobject::otherformat usagetype, const char *boundary)  ;
   boolean ReadOtherFormat(FILE *file, const char *fmt, const char *encoding, const char *description)  ;


  
	static boolean InitializeClass();

  
	image();
  
  static int depthToColors( int n )  ;
  static int colorsToDepth( int n )  ;


  virtual void Reset()  ;
  virtual void SetSaveFormatString( const char *saveformat );
  virtual void newRGBMapData( unsigned int size )  ;
  virtual void freeRGBMapData()  ;
  virtual void newBitImage( unsigned int width, unsigned int height )  ;
  virtual void newGreyImage( unsigned int width, unsigned int height, unsigned int depth )  ;
  virtual void newRGBImage( unsigned int width, unsigned int height, unsigned int depth )  ;
  virtual void newTrueImage( unsigned int width, unsigned int height )  ;
  virtual void freeImageData()  ;
  virtual class image * Dither()  ;
  virtual void Brighten( unsigned int percent )  ;
  virtual class image * Normalize()  ;
  virtual void Gray()  ;
  virtual class image * Bit2Grey()  ;
  virtual void Compress()  ;
  virtual class image * Zoom( unsigned int xzoom, unsigned int yzoom )  ;
  virtual class image * Reduce( unsigned int n )  ;
  virtual class image * Expand()  ;
  virtual void GammaCorrect( float disp_gam )  ;
  virtual class image * Halftone()  ;
  virtual void Duplicate( class image *target )  ;
  virtual long WriteNative( FILE *file, const char *filename )  ;

  virtual long SendBeginData(FILE *file, long writeID, int level)  ;
  virtual long SendImageData(FILE *file)  ;
  virtual long SendEndData(FILE *file, long writeID, int id)  ;

  virtual long GetBeginData(FILE *file, long id)  ;
  virtual long GetImageData(FILE *file)  ;
  virtual long GetEndData(FILE *file, long id)  ;

  /* The following method should be overridden by subclasses */
  virtual int Load( const char *fullname, FILE *fp )  ;

  virtual class image *ColorDither(RGBMap *map);
  virtual class image *ColorDitherToCube(int reds, int greens, int blues);
  
  inline unsigned int  &Type() { return ((this)->type); }
  inline byte  * &Data() { return ((this)->data); }
  inline unsigned int  &Width() { return ((this)->width); }
  inline unsigned int  &Height() { return ((this)->height); }
  inline unsigned int  &Depth() { return ((this)->depth); }
  inline unsigned int  &Pixlen() { return ((this)->pixlen); }
  inline unsigned int  &BitsPerPixel() { return ((this)->bitsperpixel); }
  inline unsigned int &RGBSize() { return ((this)->rgb->size); }
  inline unsigned int &RGBUsed() { return ((this)->rgb->used); }
  inline Intensity *&RedMap() { return ((this)->rgb->red); }
  inline Intensity *&GreenMap() { return ((this)->rgb->green); }
  inline Intensity *&BlueMap() { return ((this)->rgb->blue); }
  inline Intensity &RedPixel(int redIndex) { return (*((this)->rgb->red + redIndex)); }
  inline Intensity &GreenPixel(int greenIndex) { return (*((this)->rgb->green + greenIndex)); }
  inline Intensity &BluePixel(int blueIndex) { return (*((this)->rgb->blue + blueIndex)); }
  inline long GetOriginX() { return (0); }
  inline long GetOriginY() { return (0); }
  inline void SetJPEGSaveQuality(int q) { ((this)->jpegSaveQuality = (q)); }
  inline int   GetJPEGSaveQuality() { return ((this)->jpegSaveQuality); }
  inline char  * SaveFormatString() { return ((this)->saveformatstring); }


  unsigned int type;  /* type of image */
  RGBMap *rgb; /* RGB map of image if IRGB type */
  unsigned int width;  /* width of image in pixels */
  unsigned int height; /* height of image in pixels */
  unsigned int depth;  /* depth of image in bits if IRGB type */
  unsigned int pixlen; /* length of pixel (in bytes) if IRGB type */
  unsigned int bitsperpixel; /* length of pixel (in bits) if IRGB type */
  byte *data; /* data rounded to full byte for each row */
  boolean inited; /* has the image been transformed into an XImage? */
  int jpegSaveQuality;
  char *origData;
  long origDataSize;
  long lastModified;
  char *saveformatstring;
};

#define image_NEW (1<<5)
#define image_CHANGED (1<<6)
/** @} */
#endif /* _image_H_ */
