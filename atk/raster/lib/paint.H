ATK_INTER
#ifndef _paint_H_
#define _paint_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

/*  paint.H

	paint package

	Routines for reading and writing rasters in the Macpaint format

 */

#include <pixelimage.H>

#define  paint_PROGRAMMERVERSION    1

#define BUFBITS (1<<17)

#include <ATK.H>
class paint : public ATK   {
public:
	virtual ATKregistryEntry *ATKregistry();




	static void WriteRow(FILE *file, unsigned char *byteaddr, long nbits);
		/* write to 'file' the Macpaint representation of the
			 'nbits' bits starting at location 'byteaddr' */

	static long ReadRow(FILE *file,  unsigned char *row,  long length)  ;
		/* Reads from 'file' the encoding of bytes to fill in 'row'.
		    Row will be truncated or padded (with WHITE)
		    to exactly 'length' bytes. */

	static long ReadImage(FILE *file, class pixelimage *pix)  ;
		/* Read a raster image from 'file' and put it in 'pix'
			return error code, as defined in dataobj.ih */

	static void WriteImage(FILE *file, class pixelimage *pix, struct rectangle *sub);
		/* Write the subraster 'sub' image to 'file' from 'pix' */

};


#endif /* _paint_H_ */
