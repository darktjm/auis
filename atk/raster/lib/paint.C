/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'     *
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

/*  paint.c

	paint package

	Routines for reading and writing rasters in the form
	defined by MacPaint.

 */

#include <andrewos.h> /* sys/types.h */
ATK_IMPL("paint.H")
#include <stdio.h>
#include <netinet/in.h>


#include <paint.H>
#include <pixelimage.H>
#include <dataobject.H>

#define DEBUG(x)  {if (debug) {printf x; fflush(stdout);}}
#define debug FALSE

#define PAINTWIDTH 576		/* in pixels */
#define PAINTWIDTHINBYTES (PAINTWIDTH >> 3)
#define PAINTHEIGHT 720 	/* in pixels */
#define PAINTHEADERLENGTH 512	/* in bytes */


static unsigned char masks[] = {0xFF, 0x80, 0xC0, 0xE0, 0xF0, 0xF8, 0xFC, 0xFE};



static FILE *outf;	/* to avoid passing 'file' to two PutXxxx functions */

	
ATKdefineRegistry(paint, ATK, NULL);
static void PutSame(unsigned char byte, long  count);
static void PutDiffer(unsigned char *start, long  length);


static void
PutSame(unsigned char byte, long  count)
		{
	while (count > 129) 
		putc(257-129, outf), putc(byte, outf), count -= 129;
	putc(257-count, outf), putc(byte, outf);
}

	static void
PutDiffer(unsigned char *start, long  length)
		{
	while (length > 0) {
		long tlength = (length>128) ? 128 : length;
		putc(tlength-1, outf);
		while (tlength-- > 0)
			putc((*start++), outf);
		length -= 128;
	}
}

/* paint__WriteRow(file, byteaddr, nbytes)  
		Writes to 'file' the encoded version of the 'nbytes' bytes
		beginning at byte with address 'byteaddr'.

		Bytes are processed by a state machine with three states:
		    SameBytes - count same bytes until get to unsame one.
		    Differ - count different bytes until get two the same
		    Differ1 - two equal bytes have been found.
		As each state finds the end of its group, it calls one of the
		output routines and sets the variables for the new state.

		This code is virtually identical to that in plusspc.c.
*/
	void
paint::WriteRow(FILE  *file, unsigned char *byteaddr, long  nbits)
				{
	enum state {SameBytes, Differ, Differ1} CurSt;
	unsigned char thischar;	/* current input char */
	unsigned char prevchar;	/* previous char in differing string */
	unsigned char *startstring;	/* start of a string of differing bytes */
	long samecount = 0;	/* count occurrences of samechar */
	long nbytes = (nbits+7)>>3;

	outf = file;
	CurSt = Differ;
	startstring = byteaddr;
	prevchar = (*byteaddr)-1;		/* not an initial match */

	while (thischar = *byteaddr++, nbytes-- > 0) switch (CurSt) {
		case SameBytes:
			if (thischar == prevchar) 
				samecount ++;
			else {
				/* found a differing character.
					put out the preceeding same seq */
				PutSame(prevchar, samecount);
				startstring = byteaddr-1;	/* loc of value in 'thischar' */
				prevchar = thischar;
				CurSt = Differ;
			}
			break;
		case Differ:
			if (thischar == prevchar)
				/* two in a row are the same.
				    Go check for three-in-a-row */
				CurSt = Differ1;
			else 
				prevchar = thischar;
			break;
		case Differ1:
			if (thischar == prevchar) {
				/* three-in-a-row: output differing bytes
				    and switch to SameBytes state*/
				PutDiffer(startstring, byteaddr - startstring - 3);
					/* the '- 3' is because we now have
					three equal bytes */
				samecount = 3;
				CurSt = SameBytes;
			}
			else 
				CurSt = Differ;
			prevchar = thischar;
			break;
	}
	switch (CurSt) {
	case SameBytes:
		PutSame(prevchar, samecount);
		break;
	case Differ:
	case Differ1:
		PutDiffer(startstring, byteaddr-startstring - 1);	
			/* the '- 1' is because we have advanced an extra
			character in the while condition */
		break;
	}
}



/* paint__ReadRow(file, row, length)
	Copies one row of bits from 'file' to 'pix'
	returns status as in dataobj.H
*/
	long
paint::ReadRow(FILE  *file			/* where to get bytes from */, unsigned char *row		/* where to put them */, long  length		/* how many bits in row must be filled */)
				{
	long sofar;		/* length unpacked so far */
	int retval = dataobject_NOREADERROR;	/* no error so far */

	unsigned char savebyte = *(row+((length+7)>>3)-1);	/* save last byte */

	sofar = 0;
	while (sofar < length)	{
		int curr = getc(file);
		int databyte = 0;
		if (curr == EOF) {
			retval = dataobject_PREMATUREEOF;
			break;
		}
		else if (curr < 128)	{
			/* next curr+1 are straight bitmap */
			curr++;		/* add in the +1 */
			if (curr > length - sofar) {
				/* ERROR: string extends beyond 'length' */
				retval = dataobject_BADFORMAT;
				break;
			}
			sofar += curr;
			while (curr--)
				*row++ = databyte = getc(file);
		}
		else {
			/* next char repeats (257-curr) times */
			databyte = getc(file);
			curr = 257 - curr;
			if (curr > length - sofar) {
				/* ERROR: code gives line longer than length
				    ignore bytes beyond length */
				retval = dataobject_BADFORMAT;
				break;
			}
			sofar += curr;
			while (curr--)
				*row++ = databyte;
		}
		if (databyte == EOF) {
			retval = dataobject_PREMATUREEOF;
			break;
		}
	}

	if (length & 0x7) {
		/* fix the last byte if length is not a multiple of 8 bits */
		long mask = masks[length & 0x7];
		unsigned char *loc = row+((length+7)>>3)-1;
		*loc = (*loc & mask) | (savebyte & ~mask);
	}
	return retval;
}




/* paint__ReadImage(ClassID, file, pix)
	Read a raster image from 'file' and put it in 'pix'
	return error code, as defined in dataobj.ih
*/
	long
paint::ReadImage(FILE  *file, class pixelimage  *pix)
			{
	unsigned char *where;	/* where to store next row */
	long row;		/* count rows */
	long status = dataobject_NOREADERROR;

	(pix)->Resize( PAINTWIDTH, PAINTHEIGHT);
	where = (pix)->GetBitsPtr();
	fread(where, PAINTHEADERLENGTH, 1, file);	/* skip header */
	for (row = 0; row < PAINTHEIGHT; row ++, where += PAINTWIDTHINBYTES) {
		status = paint::ReadRow(file, where, PAINTWIDTHINBYTES);
		if (status != dataobject_NOREADERROR) break;
	}
	(pix)->NotifyObservers( pixelimage_DATACHANGED);
	return status;
}


/* paint_WriteImage(ClassID, file, pix, sub)
	Write the subraster 'sub' image to 'file' from 'pix'
	Truncate or pad with zero to PAINTWIDTH x PAINTHEIGHT
*/
	void
paint::WriteImage(FILE  *file, class pixelimage  *pix, struct rectangle  *sub)
				{
	long left, top, width, height;
	unsigned char buf[PAINTWIDTHINBYTES + 2];
	long row, i;

	for (i = 0; i < PAINTHEADERLENGTH; i++)
			/* Write out 512 nulls to outfile.  If this
			is done, and the magic number is missing at
			the start of the file, the default patterns
			are used and don't have to be written. */
		putc(0, file);

	rectangle_GetRectSize(sub, &left, &top, &width, &height);
	if (width > PAINTWIDTH) width = PAINTWIDTH;
	if (height > PAINTHEIGHT) height = PAINTHEIGHT;
	if (width < 0) width = 0;
	if (height < 0) height = 0;

	for (i = width >> 3; i < PAINTWIDTHINBYTES; i++)
		buf[i] = 0;		/* pad with zero on the right */

	for (row = top;  row < top + height; row++) {
		(pix)->GetRow( left, row, width, (unsigned short *)buf);
		paint::WriteRow(file, buf, PAINTWIDTH);
	}
	for (i = 0; i < PAINTWIDTHINBYTES; i++)
		buf[i] = 0;		/* clear buffer */
	for (row = height; row < PAINTHEIGHT; row ++)
		/* pad with zeroes on the bottom */
		paint::WriteRow(file, (unsigned char *)buf, PAINTWIDTH);
}


