/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

/*  plusspc.c

	plusspace package

	Routines for reading and writing rasters in the form used for 
	the first stab at rasters in BE2.  The stream is encoded as in 
	MacPaint and then nibbled and each nibble is ASCII-fied by adding ' '.
	The nibbles are stored with the second half first!

 */

#include <andrewos.h>
ATK_IMPL("plusspace.H")
#include <stdio.h>


#include <plusspace.H>
#define class_StaticEntriesOnly
#include <pixelimage.H>		/* for WHITEBYTE, BLACKBYTE */
#include <dataobject.H>		/* for Read errors */
#undef class_StaticEntriesOnly


static FILE *f;	/* to avoid passing 'file' to all PutXxxx functions */

	
ATKdefineRegistryNoInit(plusspace, ATK);
static void PutNibblePair(unsigned char byte);
static void PutSame(unsigned char byte, long  count);
static void PutDiffer(unsigned char *start, long  length);
static long GetPrintableNibblePair(FILE  *fp);


static void
PutNibblePair(unsigned char byte)
	{
	putc((byte&0xF) + ' ', f);		/* put low four bits before the upper ! */
	putc(((byte>>4)&0xF) + ' ', f);
}

	static void
PutSame(unsigned char byte, long  count)
		{
	while (count > 129) 
		PutNibblePair(257-129), PutNibblePair(~byte), count -= 129;
	PutNibblePair(257-count), PutNibblePair(~byte);
}

	static void
PutDiffer(unsigned char *start, long  length)
		{
	while (length > 0) {
		long tlength = (length>128) ? 128 : length;
		PutNibblePair(tlength-1);
		while (tlength-- > 0)
			PutNibblePair(~(*start++));
		length -= 128;
	}
}


/* plusspace__WriteRow(file, byteaddr, nbytes)  
		Writes to 'file' the encoded version of the 'nbytes' bytes
		beginning at byte with address 'byteaddr'.

		Bytes are processed by a state machine with three states:
		    SameBytes - count same bytes until get to unsame one.
		    Differ - count different bytes until get two the same
		    Differ1 - two equal bytes have been found.
		As each state finds the end of its group, it calls one of the
		output routines and sets the variables for the new state.

		This code is virutally identical to that in paint.c.
*/
	void
plusspace::WriteRow(FILE  *file, unsigned char *byteaddr, long  nbytes)
				{
	enum state {SameBytes, Differ, Differ1} CurSt;
	unsigned char thischar;	/* current input char */
	unsigned char prevchar;	/* previous char in differing string */
	unsigned char *startstring;	/* start of a string of differing bytes */
	long samecount = 0;	/* count occurrences of samechar */
	
	f = file;
	CurSt = Differ;
	startstring = byteaddr;
	prevchar = *byteaddr - 1;	/* not equal to first character */

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



/* GetPrintableNibblePair(fp)
	Gets two consecutive chars from the datastream.  
	The first's low nibble is the result's LOW nibble 
	and the second's low nibble is the result's HIGH nibble (!).
   	The nibbles have had `space' added to the so as to make them printable.
	When end is detected, return 128.  This will store a maximum number of
	bytes of with a single bit set.  (Vertical stripes.)
*/
#define ERRORBYTE 128
	static long
GetPrintableNibblePair(FILE  *fp)
	{
	 long a, b;
	a = getc(fp);
	if (a == EOF) return ERRORBYTE;
	if (a == '\\') 
		{ungetc(a, fp); return ERRORBYTE;}
	b = getc(fp);
	if (b == EOF) return ERRORBYTE;
	if (b == '\\') 
		{ungetc(b, fp); return ERRORBYTE;}
	return ( (a & 0xF) | ((b << 4) & 0xF0) );
}

/* plusspace__ReadRow(file, row, length) 
	Unpacks encoded pairs of nibbles from 'file'
	and stores the resulting data stream in 'row'.
	A byte value c where  c < 128  indicates that the
	next c+1 bytes from file are to be copied to row.
	If c>= 128, then 257-c copies of the next byte are stored.
	returns 0 for success.  -1 for failure
*/
	long
plusspace::ReadRow(FILE  *file		/* where to get them from */, unsigned char *row	/* where to put bytes */, long  length	/* how many bytes in row must be filled */)
				{
	int sofar;		/* length unpacked so far */
	int curr;		/* current char from in stream */

	sofar = 0;
	while (sofar < length)	{
		curr = GetPrintableNibblePair(file);
		if (curr < 128)	{
			/* next curr+1 are straight bitmap */
			curr++;		/* add in the +1 */
			if (curr > length - sofar) {
				/* ERROR: string extends beyond length
				    given by length.  Store to end of line
				    and then read and ignore the rest of the batch */
				return dataobject::BADFORMAT;
			}
			sofar += curr;
			while (curr--)
				*row++ =~GetPrintableNibblePair(file);
		}
		else {
			/* next char repeats (257-curr) times */
			int repchar = ~GetPrintableNibblePair(file);
			curr = 257 - curr;
			if (curr > length - sofar) {
				/* ERROR: code gives line longer than length
				    ignore bytes beyond length */
				return dataobject::BADFORMAT;
			}
			sofar += curr;
			while (curr--)
				*row++ = repchar;
		}
	}
	return dataobject::NOREADERROR;
}
