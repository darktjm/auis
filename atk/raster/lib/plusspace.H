#ifndef _plusspace_H_
#define _plusspace_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

/** \addtogroup libraster
 * @{ */
/*  plusspc.H

	plusspace package

	Routines for reading and writing rasters in 'plus-space' form
	(as used for the first BE2 rasters code)

	The data stream is packed with the same scheme as used for MacPaint
	Then bytes are split to nibbles and written by adding ' ' to each nibble.

 */

#define  plusspace_PROGRAMMERVERSION    1

#include <stdio.h>
#include <ATK.H>
class plusspace : public ATK  {
public:
	virtual ATKregistryEntry *ATKregistry();




	static void WriteRow(FILE *file,  unsigned char *byteaddr,  long nbytes);
		/* write to 'file' the plusspace form representation of the 'nbytes'
		    bytes starting at location 'byteaddr' */
	
	static long ReadRow(FILE *file,  unsigned char *row,  long length)  ;
		/* Reads from 'file' the encoding of bytes to fill in 'row'.  Row will be
		    truncated or padded (with WHITE) to exactly 'length' bytes.
		*/

};
/** @} */
#endif /* _plusspace_H_ */
