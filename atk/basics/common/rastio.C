/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

/*  rastio.c

	rastio package

	Routines for reading and writing compress, contiguous data.

	Known problems:
		We could fix ReadRow to not check length before each code byte.

 */
#include <andrewos.h>
ATK_IMPL("rastio.H")
#include <stdio.h>


#include <rastio.H>

#define class_StaticEntriesOnly
#include <dataobject.H>		/* for read return values */
#undef class_StaticEntriesOnly

/* codes for data stream */
#define WHITEZERO	'f'
#define WHITETWENTY	'z'
#define BLACKZERO	'F'
#define BLACKTWENTY	'Z'
#define OTHERZERO	0x1F

#define WHITEBYTE	0
#define BLACKBYTE	0xFF

/* WriteRow table for conversion of a byte value to two character hex representation */

static unsigned char hex[16] = {
	'0', '1', '2', '3', '4', '5', '6', '7',
	'8', '9', 'a', 'b', 'c', 'd', 'e', 'f'
};


/* rastio__WriteRow(file, byteaddr, nbytes)  
Writes to 'file' the encoded version of the 'nbytes' bytes beginning at byte with address 'byteaddr'.

The general scheme is that no byte is output until it has spent at least one cycle in 'curbyte'.  Each incoming byte (in 'c') is compared against 'curbyte' and if they match the only action is to increment the counter 'curcnt'.  If they do not match, an output code is generated for 'curbyte' and its value is replaced by the incoming byte.

To force the last byte out, the byte following it is temporarily replaced with a code guaranteed to be different.

The data is encoded with 4 columns to a line and lines usually have about fifteen bytes.  Column entries can be longer if long runs of similar bytes are encountered.
*/


ATKdefineRegistryNoInit(rastio, ATK);
void
rastio::WriteRow(FILE  *file, unsigned char *byteaddr, long  nbytes)
				{
	unsigned char curbyte;	/* byte enqueued for output */
	unsigned char c;		/* incoming byte */
	long curcnt;			/* number of occurrences of curbyte */
	unsigned char *bend;	/* addr of byte following the row */
	unsigned char savechar;		/* save character replaced */
	long outcnt;			/* count bytes output to this column */
	long colcnt;			/* count columns per line */
	
	bend = byteaddr + nbytes;
	savechar = *bend;
	*bend = *(bend-1) ^ 0x3;	/* make *bend differ from *(bend-1) to
				ensure flushing a code for *(bend-1) 
				WARNING: modifies and restores the pixelimage bits XXX*/

	outcnt = colcnt = 0;
	curbyte = *byteaddr++;		/* get first byte */
	curcnt = 1;		/* and count it */
	while (byteaddr <= bend) {
		c = *byteaddr++;	/* get next byte */
		if (c == curbyte)
			/* same as enqueued byte, just count it */
			curcnt++;
		else {
			/* flush the enqueued byte */

			/* output spacing */
			if (outcnt >= 13) {
				if ((++colcnt & 0x3) == 0)
					fputc('\n', file), fputc(' ', file);
				else	fputc('\t', file);
				outcnt = 0;
			}
			/* generate the encoding */
			switch (curbyte) {
			case WHITEBYTE:
				while (curcnt > 20) 
					fputc(WHITETWENTY, file),
					outcnt++, curcnt -= 20;
				fputc(WHITEZERO + curcnt, file), outcnt++;
				break;
			case BLACKBYTE:
				while (curcnt > 20) 
					fputc(BLACKTWENTY, file),
					outcnt++, curcnt -= 20;
				fputc(BLACKZERO + curcnt, file), outcnt++;
				break;
			default:
				while (curcnt > 16)
					fputc(OTHERZERO+16, file),
					fputc(hex[curbyte / 16], file),
					fputc(hex[curbyte & 15], file),
				        outcnt += 3,
					curcnt -= 16;
				if (curcnt > 1)
					fputc(OTHERZERO+curcnt, file), outcnt++;
				else {}  /* the byte written will represent a single instance */
				fputc(hex[curbyte / 16], file),
				fputc(hex[curbyte & 15], file),
				outcnt += 2;
				break;
			}
			/* enqueue the new incoming byte */
			curbyte = c;
			curcnt = 1;
		}
	}
	fputc(' ', file);  fputc('|', file);  fputc('\n', file);	/* end of row indication */

	*bend = savechar;			/* restore the modified byte */
}

/* rastio__ReadRow(file, row, length) 
Reads from 'file' the encoding of bytes to fill in 'row'.  Row will be truncated or padded (with WHITE) to exactly 'length' bytes.

Returns the code that terminated the row.  This may be
		'|'  	correct end of line
		'\0' 	if the length was satisfied (before a terminator)
		EOF 	if the file ended
		'\'  '{' 	other recognized ends. 
The '|' is the expected end and pads the row with WHITE.
The '\' and '{' are error conditions and may indicate the beginning of some other portion of the data stream.
If the terminator is '\' or '{', it is left at the front of the input.
'|' is gobbled up.
*/

/* macros to generate case entries for switch statement */
#define case1(v) case v
#define case4(v) case v: case (v)+1: case (v)+2: case(v)+3
#define case6(v) case4(v): case ((v)+4): case ((v)+5)
#define case8(v) case4(v): case4((v)+4)

long
rastio::ReadRow(FILE  *file		/* where to get them from */, unsigned char *row	/* where to put bytes */, long  length	/* how many bytes in row must be filled */)
				{
	/* Each input character is processed by the central loop.  There are 
		some input codes which require two or three characters for completion; 
		these are handled by advancing the state machine.  Errors are not 
		processed; instead the state machine is reset to the Ready state 
		whenever a character unacceptable to the curent state is read.  */
	enum stateCode {
			Ready, 		/* any input code is allowed */
			HexDigitPending,	/* have seen the first of a hex digit pair */
			RepeatPending, 	/* repeat code has been seen:
					must be followed by two hex digits */
			RepeatAndDigit};	/* have seen repeat code and its first
					following digit */
	enum stateCode InputState;	/* current state */
	int c;		/* the current input character */
	long repeatcount = 0;	/* current repeat value */
	long hexval;	/* current hex value */
	long pendinghex = 0;		/* the first of a pair of hex characters */
	
	/* We cannot exit when length becomes zero because we need to check 
	to see if a row ending character follows.  Thus length is checked only when we get
	a data generating byte.  If length then is zero, we ungetc the byte */

	InputState = Ready;
	while ((c=getc(file)) != EOF) 
				switch (c) {

	case8(0x0):
	case8(0x8):
	case8(0x10):
	case8(0x18):
	case1(' '):
		/* control characters and space are legal and completely ignored */
		break;
	case1(0x40):	/* '@' */
	case1(0x5B):	/* '[' */
	case4(0x5D):	/*  ']'  '^'  '_'  '`' */
	case4(0x7D):	/* '}'  '~'  DEL  0x80 */
	default:		/* all above 0x80 */
		/* error code:  Ignored at present.  Reset InputState. */
		InputState = Ready;
		break;

	case1(0x7B):	/* '{' */
	case1(0x5C):	/* '\\' */
		/* illegal end of line:  exit anyway */
		ungetc(c, file);		/* retain terminator in stream */
		/* DROP THROUGH */
	case1(0x7C):	/* '|' */
		/* legal end of row: may have to pad  */
		while (length-- > 0)
			*row++ = WHITEBYTE;
		return c;
	
	case1(0x21):
	case6(0x22):
	case8(0x28):
		/* punctuation characters:  repeat byte given by two succeeding hex chars */
		if (length <= 0) {
			ungetc(c, file);
			return('\0');
		}
		repeatcount = c - OTHERZERO;
		InputState = RepeatPending;
		break;

	case8(0x30):
	case8(0x38):
		/* digit (or following punctuation)  -  hex digit */
		hexval = c - 0x30;
		goto hexdigit;
	case6(0x41):
		/* A ... F    -  hex digit */
		hexval = c - (0x41 - 0xA);
		goto hexdigit;
	case6(0x61):
		/* a ... f  - hex digit */
		hexval = c - (0x61 - 0xA);
		goto hexdigit;

	case8(0x67):
	case8(0x6F):
	case4(0x77):
		/* g ... z   -   multiple WHITE bytes */
		if (length <= 0) {
			ungetc(c, file);
			return('\0');
		}
		repeatcount = c - WHITEZERO;
		hexval = WHITEBYTE;
		goto store;
	case8(0x47):
	case8(0x4F):
	case4(0x57):
		/* G ... Z   -   multiple BLACK bytes */
		if (length <= 0) {
			ungetc(c, file);
			return('\0');
		}
		repeatcount = c - BLACKZERO;
		hexval = BLACKBYTE;
		goto store;

hexdigit:
		/* process a hex digit.  Use InputState to determine
			what to do with it. */
		if (length <= 0) {
			ungetc(c, file);
			return('\0');
		}
		switch(InputState) {
		case Ready:
			InputState = HexDigitPending;
			pendinghex = hexval << 4;
			break;
		case HexDigitPending:
			hexval |= pendinghex;
			repeatcount = 1;
			goto store;
		case RepeatPending:
			InputState = RepeatAndDigit;
			pendinghex = hexval << 4;
			break;
		case RepeatAndDigit:
			hexval |= pendinghex;
			goto store;
		}
		break;

store:
		/* generate byte(s) into the output row 
			Use repeatcount, depending on state.  */
		if (length < repeatcount) 
			/* reduce repeat count if it would exceed
				available space */
			repeatcount = length;
		length -= repeatcount;	/* do this before repeatcount-- */
		while (repeatcount-- > 0)
				*row++ = hexval;
		InputState = Ready;
		break;

	} /* end of while( - )switch( - ) */
	return EOF;
}
#undef case1
#undef case4
#undef case6
#undef case8

