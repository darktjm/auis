ATK_INTER
#ifndef _oldRF_H_
#define _oldRF_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

/** \addtogroup libraster
 * @{ */
/*  oldrf.H

	oldRF package

	Routines for reading and writing rasters in the old ITC format
	as defined by rastfile.h

	The data stream begins with a 14 or 16 byte header,
	which is followed by the bits of the data.

 */

#include <pixelimage.H>

#define  oldRF_PROGRAMMERVERSION    1

#define BUFBITS (1<<17)

#include <ATK.H>
class oldRF : public ATK   {
public:
	virtual ATKregistryEntry *ATKregistry();




	static void WriteRow(FILE *file,  unsigned char *byteaddr,  long nbytes);
		/* write to 'file' the oldRF form representation of the 'nbytes'
		    bytes starting at location 'byteaddr' */
	
	static long ReadRow(FILE *file,  unsigned char *row,  long length)  ;
		/* Reads from 'file' the encoding of bytes to fill in 'row'.  Row will be
		    truncated or padded (with WHITE) to exactly 'length' bytes.
		*/
	static long ReadImage(FILE *file, class pixelimage *pix)  ;
		/* Read a raster image from 'file' and put it in 'pix' 
			return error code */
	static void WriteImage(FILE *file, class pixelimage *pix, struct rectangle *sub);
		/* Write a raster image to 'file' from 'pix'  */

};
/** @} */
#endif /* _oldRF_H_ */
