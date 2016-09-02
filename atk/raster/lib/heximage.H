ATK_INTER
#ifndef _heximage_H_
#define _heximage_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/config/COPYRITE'     *
\* ********************************************************************** */

/*
	$Disclaimer: 
// Permission to use, copy, modify, and distribute this software and its 
// documentation for any purpose and without fee is hereby granted, provided 
// that the above copyright notice appear in all copies and that both that 
// copyright notice and this permission notice appear in supporting 
// documentation, and that the name of IBM not be used in advertising or 
// publicity pertaining to distribution of the software without specific, 
// written prior permission. 
//                         
// THE COPYRIGHT HOLDERS DISCLAIM ALL WARRANTIES WITH REGARD 
// TO THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF 
// MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL ANY COPYRIGHT 
// HOLDER BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL 
// DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, 
// DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE 
// OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION 
// WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
// 
//  $
*/


 


/*  heximage.H

	heximage package

	Routines for reading and writing rasters in hexadecimal
	(as required for readhexstring in postscript)


 */

#include <pixelimage.H>

#define  heximage_PROGRAMMERVERSION    1
#define BUFBITS (1<<17)


#include <ATK.H>
class heximage : public ATK   {
public:
	virtual ATKregistryEntry *ATKregistry();




	static void WriteRow(FILE *file,  unsigned char *byteaddr,  long nbytes, boolean invert);
		/* write to 'file' the heximage form representation of the 'nbytes'
		    bytes starting at location 'byteaddr' 
		    invert the image if 'invert' is true */
	
	static long ReadRow(FILE *file,  unsigned char *row,  long length)  ;
		/* Reads from 'file' the encoding of bytes to fill in 'row'.  Row will be
		    truncated or padded (with WHITE) to exactly 'length' bytes.
		*/
	static long ReadImage(FILE *file, class pixelimage *pix, long width, long height)  ;
		/* Read a raster image from 'file' and put it in 'pix' 
			The width and height must be supplied 
			and will be stored in the pix.
			return error code */
	static void WriteImage(FILE *file, class pixelimage *pix, struct rectangle *sub);
		/* Write a raster image to 'file' from 'pix'  */

	static void WritePostscript(FILE *file, class pixelimage *pix, struct rectangle *sub,
			double xfrac, double yfrac);
		/* write to 'file' the postscript code 
			to print the subraster 'sub' of 'pix' */

};


#endif /* _heximage_H_ */
