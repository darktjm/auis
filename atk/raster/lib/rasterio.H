ATK_INTER
#ifndef _rasterio_H_
#define _rasterio_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

/** \addtogroup libraster
 * @{ */
/*
 * P_R_P_Q_# (C) COPYRIGHT IBM CORPORATION 1987, 1988
 * LICENSED MATERIALS - PROPERTY OF IBM
 * REFER TO COPYRIGHT INSTRUCTIONS FORM NUMBER G120-2083
 */

/*  rasterio.H

	rasterio package

	Routines for reading and writing rasters in .raster form
	(BE2 rasters version 2.)

	For data stream definition, see itc/be2/raster/spec.d

	We do not implement rasterio_ReadImage and rasterio_WriteImage
	because they would depend on raster.ih and rastimg.ih.
	Since raster.c depends on rasterio.ih, there would be a circularity
	and there would be no reasonable order in which to install 
	itc/be2/raster/raster and itc/be2/raster/libraster.

 */

#define  rasterio_PROGRAMMERVERSION    1

#include <ATK.H>
class rasterio : public ATK 

{
public:
	virtual ATKregistryEntry *ATKregistry();




	static void WriteRow(FILE *file, unsigned char *byteaddr, long nbytes);
		/* write to 'file' the .raster form representation of the 'nbytes'
		    bytes starting at location 'byteaddr' */
	
	static long ReadRow(FILE *file, unsigned char *row, long length)  ;
		/* Reads from 'file' the encoding of bytes to fill in 'row'.  Row will be
		    truncated or padded (with WHITE) to exactly 'length' bytes.
		    Returns the code that terminated the row.  This may be 
			'|'  	correct end of line
			'\0' 	if the length was satisfied (before a terminator)
			EOF 	if the file ended
			'\'  '{' 	other recognized ends.
		*/

	static long ReadImage(FILE *file, class pixelimage *pix)  ;
		/* Read a raster image from 'file' and put it in 'pix' 
			return error code */

	static void WriteImage(FILE *file, class pixelimage *pix, struct rectangle *sub);
		/* Write a raster image to 'file' from 'pix'  */

};
/** @} */
#endif /* _rasterio_H_ */
