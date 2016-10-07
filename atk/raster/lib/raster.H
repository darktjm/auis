#ifndef _raster_H_
#define _raster_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

/** \addtogroup libraster Object raster
 *  A dynamically loadable ATK class file.  Contains raster, rasterview,
 *  pheximage, oldRF, paint, plusspace, rasterio, xbm, xwdio, cmuwm,
 *  rasterimage, rastoolview.
 * @{ */
/*

	A raster  refers to a rasterimage containing the raw bits.  It can specify
	scaling, cropping, reflection, rotation, and inversion of the image from the raw bits.

	For a description of the data stream, see itc/be2/raster/rasterspec.d
*/

#include <rect.h>

#include <rasterimage.H>	/* because some macros call routines from it */

#define  raster_PROGRAMMERVERSION    1

/* status values for notify observer */
#define  raster_BITSCHANGED    2  /* the parameters in the
					struct raster have changed */
#define  raster_BOUNDSCHANGED  3  /* the raster is viewing a different
					part of the rasterimage */

#include <dataobject.H>
class raster : public  dataobject  {
public:
	virtual ATKregistryEntry *ATKregistry();




	 long Read(/* struct raster *self, */ FILE *file, long id) ;
	 long Write(/* struct raster *self, */ FILE *file, long id, int level)
			 ;
	 boolean ReadOtherFormat(FILE *file, const char *fmt, const char *encoding, const char *description)  ;
	 long WriteOtherFormat(FILE *file, long writeid, int level, dataobject::otherformat usagetype, const char *boundary)  ;
	 void ObservedChanged(/* struct raster *self, */ class observable *pix, long status);
	 void SetAttributes( struct attributes *attributes );



	virtual void SetPix(/* struct raster *self, */ class rasterimage *pix);
		/* changes to refer to a new rasterimage;  handles observing */
	virtual void Resize(/* struct raster *self, */ long width, long height);
		/* changes the size of the raster;  resets subraster to whole */
	virtual long ReadSubRaster(/* struct raster *self, */ FILE *file, struct rectangle *r)
				 ;
		/* read from file into the subraster with upper left pixel at (x, y) */
	virtual long WriteSubRaster(/* struct raster *self, */ FILE *file, long objectid, 
			struct rectangle *subraster)   ;
		/* write the subraster at (x,y) with given width and height to file */
	virtual void WriteShare(/* struct raster *self, */ FILE *file, struct rectangle *subraster);
		/* write a "share" record for the indicated 'subraster' of 'self' */



	inline void Clear()		{ ((this->pix)->Clear()); }
	inline long GetWidth()	{ return ((this->pix)->GetWidth()); }
	inline long GetHeight()	{ return ((this->pix)->GetHeight()); }
	inline class rasterimage * GetPix()		{ return (this->pix); }



	
	raster();
	
	virtual ~raster();
	static class raster * Create(/* struct classhdr *ClassID, */ long width, long height)
			 ;
		/* creates a raster object with a rasterimage having
		space for a raster of the given width and height */



	class rasterimage *pix;	/* the image storage */
	boolean	readOnly;
	struct rectangle subraster;	/* choose subraster of rasterimage */

	/* the raster may be a negative image or reflection of the bits in the
	    underlying raster: */
	char  options;
		/* bits of 'option' value */
#define  raster_INVERT	(1<<0)	/* exchange black and white */
#define  raster_FLIP	(1<<1)	/* exch top and bottom */
#define  raster_FLOP	(1<<2)	/* exch left and right */
#define  raster_ROTATE	(1<<3)	/* rotate 90 clockwise */

#define raster_UNITSCALE  136533	 /*0x20000 print at nominal screen size */
	/*  default print scaling is 0x10000 which means 1/2 screen size */

	long xScale, yScale;	/* scaling for printing */
};
/** @} */
#endif /* _raster_H_ */
