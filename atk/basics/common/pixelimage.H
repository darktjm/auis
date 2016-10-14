#ifndef _pixelimage_H_
#define _pixelimage_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

/** \addtogroup libbasics
 * @{ */
 /* 
  *	pixelimg.H
  */

 

/*
 * Pixel Image -- a simple data structure for blasting bits 

*/

/*	The bits of a pixel image are stored rowwise.  Each row is rounded up to
	a multiple of 16 bits (halfword alignment).  The bits in each word are 
	left to right in their screen order;  the high order bit in the even numbered 
	byte in the word is the leftmost of the bits on the screen.  	The rightmost
	displayed bit is the low order bit of the odd numbered byte of the word.
	A bit value of 1 means foreground color; a 0 means background color.
	(The foreground is black: red=0, green=0, blue=0.)
*/


#include <point.h>
#include <rect.h>


#define WHITEBYTE	0
#define BLACKBYTE	0xFF
#define WHITEHALFWORD  ((WHITEBYTE <<8) | WHITEBYTE)
#define BLACKHALFWORD  ((BLACKBYTE <<8) | BLACKBYTE)
#define pixelimage_DATACHANGED 1

#include <observable.H>

class pixelimage : public  observable {
public:
	TRACED_CLASS(pixelimage);




	 void NotifyObservers(/* struct pixelimage *self, */ long status);
		/* Notifies observers and then clears ChangedRect
			and Resized.
		    The client of pixelimage must call NotifyObservers. */



	virtual void Resize(/* struct pixelimage *self, */ long width, long height);
		/* Changes the width and height of the image.  The upper left
		corners of the old and new images are aligned. */
	virtual class pixelimage * Clone(/* struct pixelimage *self */)  ;
		/* make a new pixelimage with all fields the same
			(both point to same bits array) */
	virtual void Clear(/* struct pixelimage *self */);
		/* set all bits of the raster to WHITE */
	virtual long GetPixel(/* struct pixelimage *self, */ long x, long y)  ;
		/*  return the value of an individual bit.  */
	virtual void SetPixel(/* struct pixelimage *self, */ long x, long y, long pixelValue);
		/* set the value of the addressed bit to 'pixelValue' */

	virtual long GetRow(/* struct pixelimage *self, */ long x, long y, long length, unsigned short *dest)
			 ;
		/* stores in dest the length bits of self from the row starting at (x,y)
			This routine shifts the bits so the bit in (x,y) is the 
			leftmost bit in dest and others follow in byte order. */
	virtual long SetRow(/* struct pixelimage *self, */ long x, long y, long length, unsigned short *src)
			 ;
		/* stores the first length bits of src into the row of self at (x,y)
			This routine shifts the bits so the leftmost bit of src
			is stored at (x,y) and others follow in byte order. */
	virtual long GetColumn(/* struct pixelimage *self, */ long x, long y, long length, unsigned short *dest)
			 ;
		/* stores in dest the length bits of self from the column starting at (x,y)
			The bit in (x,y) is the leftmost bit in dest
			and others follow in byte order. */
	virtual long SetColumn(/* struct pixelimage *self, */ long x, long y, long length, unsigned short *src)
			 ;
		/* stores the first length bits of src into the column of self at (x,y)
			The leftmost bit of src is stored at (x,y)
			and others follow in byte order. */

	/* * * * * *
		the following MODIFY their 'sub' parameter to lie entirely within the
		rectangle devoted to the raster. They may set the width or height
		of the rectangle to a negative value 
	* * * * * */

	virtual void PaintSubraster(/* struct pixelimage *self, */ struct rectangle *sub, long byte);
		/* sets the indicated Subraster to the color given by 'byte', which is
			WHITEBYTE or BLACKBYTE */
	virtual void GraySubraster(/* struct pixelimage *self, */ struct rectangle *sub, long level);
		/* sets the indicated Subraster to the gray level given by 'byte',
			which is in the range from 1 t0 16. */
	virtual void InvertSubraster(/* struct pixelimage *self, */ struct rectangle *sub);
		/* change ones to zeros and zeros to ones in indicated subraster */
	virtual void MirrorLRSubraster(/* struct pixelimage *self, */ struct rectangle *sub);
		/* reflect the subraster about its vertical axis */
	virtual void MirrorUDSubraster(/* struct pixelimage *self, */ struct rectangle *sub);
		/* reflect the subraster about its horizontal axis */
	virtual void GetRotatedSubraster(/* struct pixelimage *self, */ struct rectangle *sub,
			class pixelimage *target);
		/* copies the subraster to the target raster, interchanging rows 
		    and columns.  If the target is too small in either dimension, 
		    it is Resized. */
	virtual void GetScaledSubraster(/* struct pixelimage *self, */ struct rectangle *sub,
			 long NewW, long NewH, class pixelimage *target);
		/* Scales the subraster by scaling factors and puts result into
		    target. Target is resized as needed. */
	virtual void BlitSubraster(/* struct pixelimage *self, */ long x, long y, class pixelimage 			*source, struct rectangle *sub,  int function);
		/* Blit the subraster given within the source into self starting at
		    the (x, y) coordinates (as an offset from the origin of self) using
		    the function given. */



	inline long GetHeight()	{ return ((this)->numRows); }
	inline long GetWidth()	{ return ((this)->pixelsPerRow); }
	inline long GetRowWidth()	{ return ((this)->RowWidth); }
	inline unsigned char * GetBitsPtr()	{ return ((this)->bits); }
	inline void SetBitsPtr(unsigned char *ptr)	{ ((this)->bits = (ptr)); }
	inline void SetRO(boolean v)		{ ((this)->ReadOnly = (v)); }
	inline boolean GetRO()		{ return ((this)->ReadOnly); }
	inline boolean GetResized()	{ return ((this)->Resized); }
	inline struct rectangle * GetChanged()	{ return (&(this->ChangedRect)); }



	/* struct classhdr *ClassID, */
	pixelimage();
		/* initializes to having 'bits' == NULL */
	/* struct classhdr *ClassID, */
		/* discards the 'bits' value if non-NULL */
	static class pixelimage * Create(long width, long height)  ;
		/* creates a pixelimage object with 'bits' for a raster
			of the given width and height */
	static void ClipRange(long *tstart, long *tlength, long start, long length);
		/* tstart...tstart+tlength is a range of integers.  The txxxx values
		are modified, if necessary, so they denote only that portion
		of their original range that lies within start...start+length */		


	long numRows;		/* Number of rows of image's pixels */
	long pixelsPerRow;	/* Number of pixels per row */
	long RowWidth;		/* Number of bytes per row.
					(Will be even.)  */
	unsigned char *bits;		/* Array of the bits */

	struct rectangle ChangedRect;  /* bound the most recently changed subraster */
	boolean Resized;		/* another value for NotifyObservers */
	boolean ReadOnly;		/* when True, the raster bits are locked */
};

/* Predefined constants for use in BlitSubraster(). These are defined exactly the same way as the transfer constants in class graphic. See graphic.{ch,doc} for details. */
#define	pixelimage_SOURCE	0xCC
#define	pixelimage_DEST		0xAA

/* Some synonyms for ease of use */
#define	pixelimage_COPY		pixelimage_SOURCE
#define	pixelimage_OR		(pixelimage_SOURCE | pixelimage_DEST)
#define	pixelimage_XOR		(pixelimage_SOURCE ^ pixelimage_DEST)
#define	pixelimage_AND		(pixelimage_SOURCE & pixelimage_DEST)
#define	pixelimage_WHITE	0x00
#define	pixelimage_BLACK	0xFF
#define	pixelimage_INVERT	((~pixelimage_DEST)&0xFF)

/** @} */
#endif /* _pixelimage_H_ */
