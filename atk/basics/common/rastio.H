#ifndef _rastio_H_
#define _rastio_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

/** \addtogroup libbasics
 * @{ */
/*
 * P_R_P_Q_# (C) COPYRIGHT IBM CORPORATION 1987, 1988
 * LICENSED MATERIALS - PROPERTY OF IBM
 * REFER TO COPYRIGHT INSTRUCTIONS FORM NUMBER G120-2083
 */

/*  rastio.ch

	rastio package

	Routines for reading and writing lines of arbitrary,
	    contiguaous memory in a compress format.

*/

#define  rastio_PROGRAMMERVERSION    1

#include <ATK.H>
#include <stdio.h>

class rastio : public ATK  {
public:
	virtual ATKregistryEntry *ATKregistry();


	static void WriteRow(FILE *file, unsigned char *byteaddr, long nbytes);
		/* write to file nbytes
		    bytes starting at location byteaddr */
	static long ReadRow(FILE *file, unsigned char *row, long length)  ;
		/* Reads from 'file' the encoding of bytes to fill in 'row'.  Row will be
		    truncated or padded (with WHITE) to exactly 'length' bytes.
		    Returns the code that terminated the row.  This may be 
			'|'  	correct end of line
			'\0' 	if the length was satisfied (before a terminator)
			EOF 	if the file ended
			'\'  '{' 	other recognized ends.
		*/
};
/** @} */
#endif /* _rastio_H_ */
