/* g3.ch - class description for interface from G3 fax format to image */
/*
	Copyright Carnegie Mellon University 1992 - All rights reserved
	$Disclaimer: 
 * Permission to use, copy, modify, and distribute this software and its 
 * documentation for any purpose and without fee is hereby granted, provided 
 * that the above copyright notice appear in all copies and that both that 
 * copyright notice and this permission notice appear in supporting 
 * documentation, and that the name of IBM not be used in advertising or 
 * publicity pertaining to distribution of the software without specific, 
 * written prior permission. 
 *                         
 * THE COPYRIGHT HOLDERS DISCLAIM ALL WARRANTIES WITH REGARD 
 * TO THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF 
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL ANY COPYRIGHT 
 * HOLDER BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL 
 * DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, 
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE 
 * OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION 
 * WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 * 
 *  $
*/

/** g3.c - read a Group 3 FAX file and product a bitmap
 **
 ** Adapted from Paul Haeberli's <paul@manray.sgi.com> G3 to Portable Bitmap 
 ** code.
 **
 ** modified by jimf on 09.18.90 to fail on any load error.  this was done
 ** to cut down on the false positives caused by a lack of any read ID
 ** string.  the old errors are currently ifdef'ed out -- if you want 'em
 ** define ALLOW_G3_ERRORS.
 **/

/* g3topbm.c - read a Group 3 FAX file and produce a portable bitmap
**
** Copyright (C) 1989 by Paul Haeberli <paul@manray.sgi.com>.
**
** Permission to use, copy, modify, and distribute this software and its
** documentation for any purpose and without fee is hereby granted, provided
** that the above copyright notice appear in all copies and that both that
** copyright notice and this permission notice appear in supporting
** documentation.  This software is provided "as is" without express or
** implied warranty.
*/
/* Edit History

04/15/91   8 nazgul   Sanity check line widths
                      Doing a zclose on all failures
04/14/91   1 schulert add <sys/typesfor SYSV systems
04/13/91   6 nazgul   Handle reinvocation on the same file
04/13/91   5 nazgul   Bug fix to retry with bitreversed, and do not double allocate on multiple calls
04/12/91   4 nazgul   Spot faxes that do not have a 000 header
                      Handle faxes that have the bytes in the wrong order
07/03/90   2 nazgul   Added recovery for premature EOF
*/

#include <andrewos.h>
ATK_IMPL("g3.H")

/*#include <g3.h>*/
#define MAXCOLS 2550	/* Maximum image size is 8.5"x11" @ 300dpi */
#define MAXROWS 3300

#define TWTABLE		23
#define MWTABLE		24
#define TBTABLE		25
#define MBTABLE		26
#define EXTABLE		27
#define VRTABLE		28

#define WHASHA 3510
#define WHASHB 1178
#define BHASHA 293
#define BHASHB 2695
#define HASHSIZE 1021

#ifndef _G3_H_
#define _G3_H_

typedef unsigned char bit;

int	bmask[] = { 0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01 };

typedef struct tableentry {
    int tabid;
    int code;
    int length;
    int count;
    } tableentry;

struct tableentry twtable[] = {
    { TWTABLE, 0x35, 8, 0 },
    { TWTABLE, 0x7, 6, 1 },
    { TWTABLE, 0x7, 4, 2 },
    { TWTABLE, 0x8, 4, 3 },
    { TWTABLE, 0xb, 4, 4 },
    { TWTABLE, 0xc, 4, 5 },
    { TWTABLE, 0xe, 4, 6 },
    { TWTABLE, 0xf, 4, 7 },
    { TWTABLE, 0x13, 5, 8 },
    { TWTABLE, 0x14, 5, 9 },
    { TWTABLE, 0x7, 5, 10 },
    { TWTABLE, 0x8, 5, 11 },
    { TWTABLE, 0x8, 6, 12 },
    { TWTABLE, 0x3, 6, 13 },
    { TWTABLE, 0x34, 6, 14 },
    { TWTABLE, 0x35, 6, 15 },
    { TWTABLE, 0x2a, 6, 16 },
    { TWTABLE, 0x2b, 6, 17 },
    { TWTABLE, 0x27, 7, 18 },
    { TWTABLE, 0xc, 7, 19 },
    { TWTABLE, 0x8, 7, 20 },
    { TWTABLE, 0x17, 7, 21 },
    { TWTABLE, 0x3, 7, 22 },
    { TWTABLE, 0x4, 7, 23 },
    { TWTABLE, 0x28, 7, 24 },
    { TWTABLE, 0x2b, 7, 25 },
    { TWTABLE, 0x13, 7, 26 },
    { TWTABLE, 0x24, 7, 27 },
    { TWTABLE, 0x18, 7, 28 },
    { TWTABLE, 0x2, 8, 29 },
    { TWTABLE, 0x3, 8, 30 },
    { TWTABLE, 0x1a, 8, 31 },
    { TWTABLE, 0x1b, 8, 32 },
    { TWTABLE, 0x12, 8, 33 },
    { TWTABLE, 0x13, 8, 34 },
    { TWTABLE, 0x14, 8, 35 },
    { TWTABLE, 0x15, 8, 36 },
    { TWTABLE, 0x16, 8, 37 },
    { TWTABLE, 0x17, 8, 38 },
    { TWTABLE, 0x28, 8, 39 },
    { TWTABLE, 0x29, 8, 40 },
    { TWTABLE, 0x2a, 8, 41 },
    { TWTABLE, 0x2b, 8, 42 },
    { TWTABLE, 0x2c, 8, 43 },
    { TWTABLE, 0x2d, 8, 44 },
    { TWTABLE, 0x4, 8, 45 },
    { TWTABLE, 0x5, 8, 46 },
    { TWTABLE, 0xa, 8, 47 },
    { TWTABLE, 0xb, 8, 48 },
    { TWTABLE, 0x52, 8, 49 },
    { TWTABLE, 0x53, 8, 50 },
    { TWTABLE, 0x54, 8, 51 },
    { TWTABLE, 0x55, 8, 52 },
    { TWTABLE, 0x24, 8, 53 },
    { TWTABLE, 0x25, 8, 54 },
    { TWTABLE, 0x58, 8, 55 },
    { TWTABLE, 0x59, 8, 56 },
    { TWTABLE, 0x5a, 8, 57 },
    { TWTABLE, 0x5b, 8, 58 },
    { TWTABLE, 0x4a, 8, 59 },
    { TWTABLE, 0x4b, 8, 60 },
    { TWTABLE, 0x32, 8, 61 },
    { TWTABLE, 0x33, 8, 62 },
    { TWTABLE, 0x34, 8, 63 },
    };

struct tableentry mwtable[] = {
    { MWTABLE, 0x1b, 5, 64 },
    { MWTABLE, 0x12, 5, 128 },
    { MWTABLE, 0x17, 6, 192 },
    { MWTABLE, 0x37, 7, 256 },
    { MWTABLE, 0x36, 8, 320 },
    { MWTABLE, 0x37, 8, 384 },
    { MWTABLE, 0x64, 8, 448 },
    { MWTABLE, 0x65, 8, 512 },
    { MWTABLE, 0x68, 8, 576 },
    { MWTABLE, 0x67, 8, 640 },
    { MWTABLE, 0xcc, 9, 704 },
    { MWTABLE, 0xcd, 9, 768 },
    { MWTABLE, 0xd2, 9, 832 },
    { MWTABLE, 0xd3, 9, 896 },
    { MWTABLE, 0xd4, 9, 960 },
    { MWTABLE, 0xd5, 9, 1024 },
    { MWTABLE, 0xd6, 9, 1088 },
    { MWTABLE, 0xd7, 9, 1152 },
    { MWTABLE, 0xd8, 9, 1216 },
    { MWTABLE, 0xd9, 9, 1280 },
    { MWTABLE, 0xda, 9, 1344 },
    { MWTABLE, 0xdb, 9, 1408 },
    { MWTABLE, 0x98, 9, 1472 },
    { MWTABLE, 0x99, 9, 1536 },
    { MWTABLE, 0x9a, 9, 1600 },
    { MWTABLE, 0x18, 6, 1664 },
    { MWTABLE, 0x9b, 9, 1728 },
    };

struct tableentry tbtable[] = {
    { TBTABLE, 0x37, 10, 0 },
    { TBTABLE, 0x2, 3, 1 },
    { TBTABLE, 0x3, 2, 2 },
    { TBTABLE, 0x2, 2, 3 },
    { TBTABLE, 0x3, 3, 4 },
    { TBTABLE, 0x3, 4, 5 },
    { TBTABLE, 0x2, 4, 6 },
    { TBTABLE, 0x3, 5, 7 },
    { TBTABLE, 0x5, 6, 8 },
    { TBTABLE, 0x4, 6, 9 },
    { TBTABLE, 0x4, 7, 10 },
    { TBTABLE, 0x5, 7, 11 },
    { TBTABLE, 0x7, 7, 12 },
    { TBTABLE, 0x4, 8, 13 },
    { TBTABLE, 0x7, 8, 14 },
    { TBTABLE, 0x18, 9, 15 },
    { TBTABLE, 0x17, 10, 16 },
    { TBTABLE, 0x18, 10, 17 },
    { TBTABLE, 0x8, 10, 18 },
    { TBTABLE, 0x67, 11, 19 },
    { TBTABLE, 0x68, 11, 20 },
    { TBTABLE, 0x6c, 11, 21 },
    { TBTABLE, 0x37, 11, 22 },
    { TBTABLE, 0x28, 11, 23 },
    { TBTABLE, 0x17, 11, 24 },
    { TBTABLE, 0x18, 11, 25 },
    { TBTABLE, 0xca, 12, 26 },
    { TBTABLE, 0xcb, 12, 27 },
    { TBTABLE, 0xcc, 12, 28 },
    { TBTABLE, 0xcd, 12, 29 },
    { TBTABLE, 0x68, 12, 30 },
    { TBTABLE, 0x69, 12, 31 },
    { TBTABLE, 0x6a, 12, 32 },
    { TBTABLE, 0x6b, 12, 33 },
    { TBTABLE, 0xd2, 12, 34 },
    { TBTABLE, 0xd3, 12, 35 },
    { TBTABLE, 0xd4, 12, 36 },
    { TBTABLE, 0xd5, 12, 37 },
    { TBTABLE, 0xd6, 12, 38 },
    { TBTABLE, 0xd7, 12, 39 },
    { TBTABLE, 0x6c, 12, 40 },
    { TBTABLE, 0x6d, 12, 41 },
    { TBTABLE, 0xda, 12, 42 },
    { TBTABLE, 0xdb, 12, 43 },
    { TBTABLE, 0x54, 12, 44 },
    { TBTABLE, 0x55, 12, 45 },
    { TBTABLE, 0x56, 12, 46 },
    { TBTABLE, 0x57, 12, 47 },
    { TBTABLE, 0x64, 12, 48 },
    { TBTABLE, 0x65, 12, 49 },
    { TBTABLE, 0x52, 12, 50 },
    { TBTABLE, 0x53, 12, 51 },
    { TBTABLE, 0x24, 12, 52 },
    { TBTABLE, 0x37, 12, 53 },
    { TBTABLE, 0x38, 12, 54 },
    { TBTABLE, 0x27, 12, 55 },
    { TBTABLE, 0x28, 12, 56 },
    { TBTABLE, 0x58, 12, 57 },
    { TBTABLE, 0x59, 12, 58 },
    { TBTABLE, 0x2b, 12, 59 },
    { TBTABLE, 0x2c, 12, 60 },
    { TBTABLE, 0x5a, 12, 61 },
    { TBTABLE, 0x66, 12, 62 },
    { TBTABLE, 0x67, 12, 63 },
    };

struct tableentry mbtable[] = {
    { MBTABLE, 0xf, 10, 64 },
    { MBTABLE, 0xc8, 12, 128 },
    { MBTABLE, 0xc9, 12, 192 },
    { MBTABLE, 0x5b, 12, 256 },
    { MBTABLE, 0x33, 12, 320 },
    { MBTABLE, 0x34, 12, 384 },
    { MBTABLE, 0x35, 12, 448 },
    { MBTABLE, 0x6c, 13, 512 },
    { MBTABLE, 0x6d, 13, 576 },
    { MBTABLE, 0x4a, 13, 640 },
    { MBTABLE, 0x4b, 13, 704 },
    { MBTABLE, 0x4c, 13, 768 },
    { MBTABLE, 0x4d, 13, 832 },
    { MBTABLE, 0x72, 13, 896 },
    { MBTABLE, 0x73, 13, 960 },
    { MBTABLE, 0x74, 13, 1024 },
    { MBTABLE, 0x75, 13, 1088 },
    { MBTABLE, 0x76, 13, 1152 },
    { MBTABLE, 0x77, 13, 1216 },
    { MBTABLE, 0x52, 13, 1280 },
    { MBTABLE, 0x53, 13, 1344 },
    { MBTABLE, 0x54, 13, 1408 },
    { MBTABLE, 0x55, 13, 1472 },
    { MBTABLE, 0x5a, 13, 1536 },
    { MBTABLE, 0x5b, 13, 1600 },
    { MBTABLE, 0x64, 13, 1664 },
    { MBTABLE, 0x65, 13, 1728 },
    };

struct tableentry extable[] = {
    { EXTABLE, 0x8, 11, 1792 },
    { EXTABLE, 0xc, 11, 1856 },
    { EXTABLE, 0xd, 11, 1920 },
    { EXTABLE, 0x12, 12, 1984 },
    { EXTABLE, 0x13, 12, 2048 },
    { EXTABLE, 0x14, 12, 2112 },
    { EXTABLE, 0x15, 12, 2176 },
    { EXTABLE, 0x16, 12, 2240 },
    { EXTABLE, 0x17, 12, 2304 },
    { EXTABLE, 0x1c, 12, 2368 },
    { EXTABLE, 0x1d, 12, 2432 },
    { EXTABLE, 0x1e, 12, 2496 },
    { EXTABLE, 0x1f, 12, 2560 },
    };
#endif

#include <g3.H>
#include <sys/types.h>
#include <sys/file.h>

#include <image.H>

/* SUPPRESS 530 */
/* SUPPRESS 558 */
/* SUPPRESS 560 */

/****
 **
 ** Local defines
 **
 ****/

#define BITS_TO_BYTES(bits)	(bits/8)+((bits-((bits/8)*8)?1:0))
#define TABSIZE(tab) (sizeof(tab)/sizeof(struct tableentry))
#ifdef VMS
#define cols vmscols
#endif

/****
 **
 ** Local variables
 **
 ****/

static int g3_eof = 0;
static int g3_eols;
static int g3_rawzeros;
static int g3_Xrawzeros;
static int	maxlinelen;
static int	rows, cols;
static int g3_error = 0;
static int g3_verb;
static int curbit;

#define MAX_ERRORS	20

/****
 **
 ** Local tables
 **
 ****/

tableentry *whash[HASHSIZE];
tableentry *bhash[HASHSIZE];


ATKdefineRegistry(g3, image, NULL);
int	g3_addtohash(tableentry	 *hash[], tableentry	 *te, int	 n , int	 a , int	 b);
tableentry	*g3_hashfind(tableentry	 *hash[], int	 length , int	 code, int	 a , int	 b);
int	g3_getfaxrow(ZFILE	 *fd, byte	 *bitrow);
int	g3_skiptoeol(FILE	 *fd);
int	g3_rawgetbit(FILE	 *fd);
int	g3_bitson(bit	 *b, int	 c , int	 n);
int	g3_ident(FILE	 *fd);

int
g3_addtohash( tableentry *hash[], tableentry	 *te, int	 n , int	 a , int	 b)
			{
	unsigned int pos;

	while (n--) {
		pos = ((te->length+a)*(te->code+b))%HASHSIZE;
		if (hash[pos] != 0) {
#ifdef ALLOW_G3_ERRORS
			fprintf(stderr, "G3: Hash collision during initialization.\n");
			exit(1);
#else
			++g3_error;
			return(-1);
#endif
			}
		hash[pos] = te;
		te++;
	}
	return(0);
}

tableentry	*g3_hashfind(tableentry	 *hash[], int	 length , int	 code, int	 a , int	 b)
			{
	unsigned int pos;
	tableentry *te;

	pos = ((length+a)*(code+b))%HASHSIZE;
	if (pos >= HASHSIZE) {
#ifndef ALLOW_G3_ERRORS
		fprintf(stderr, "G3: Bad hash position, length %d code %d pos %d.\n", 
			length, code, pos);
		exit(2);
#else
		++g3_error;
		return(NULL);
#endif
		}
	te = hash[pos];
	return ((te && te->length == length && te->code == code) ? te : 0);
}

int	g3_getfaxrow(ZFILE	 *fd, byte	 *bitrow)
		{
        int col;
	int curlen, curcode, nextbit;
	int count, color;
	tableentry *te;

	/* First make the whole row white... */
	memset((char *) bitrow, 0, maxlinelen); /* was memset -- jimf 09.11.90 */

	col = 0;
	g3_rawzeros = 0;
	curlen = 0;
	curcode = 0;
	color = 1;
	count = 0;
	while (!g3_eof) {
		if (col >= MAXCOLS) {
#ifdef ALLOW_G3_ERRORS
			if (g3_verb) fprintf(stderr, "G3: Input row %d is too long, skipping to EOL.\n", rows);
			g3_skiptoeol(fd);
			++g3_error;
			return (col); 
#else
			return(-1);
#endif
			}
		do {
			if (g3_eof) return 0;
			if (g3_rawzeros >= 11) {
				nextbit = g3_rawgetbit(fd);
				if (nextbit) {
					if ( col == 0 )
						/* 6 consecutive EOLs mean end of document */
						g3_eof = (++g3_eols >= 5);
					else
						g3_eols = 0;

					return (col); 
					}
				}
			else
				nextbit = g3_rawgetbit(fd);

			curcode = (curcode<<1) + nextbit; 
			curlen++;
			} while (curcode <= 0);

		/* No codewords are greater than 13 bytes */
		if (curlen > 13) {
#ifdef ALLOW_G3_ERRORS
			if (g3_verb) fprintf(stderr, "G3: Bad code word at row %d, col %d (len %d code 0x%2.2x), skipping to EOL.\n", rows, col, curlen, curcode );
			g3_skiptoeol(fd);
			++g3_error;
			return (col);
#else
			return(-1);
#endif
			}
		if (color) {
			/* White codewords are at least 4 bits long */
			if (curlen < 4)
				continue;
			te = g3_hashfind(whash, curlen, curcode, WHASHA, WHASHB);
			}
		else {
			/* Black codewords are at least 2 bits long */
			if (curlen < 2)
				continue;
			te = g3_hashfind(bhash, curlen, curcode, BHASHA, BHASHB);
		}
		if (!te)
			continue;
		switch (te->tabid) {
			case TWTABLE:
			case TBTABLE:
				count += te->count;
				if (col+count > MAXCOLS) 
					count = MAXCOLS-col;
				if (count > 0) {
					if (color) {
						col += count;
						count = 0;
						}
					else
						g3_bitson(bitrow, col, count);
					}
				curcode = 0;
				curlen = 0;
				color = !color;
				break;
			case MWTABLE:
			case MBTABLE:
				count += te->count;
				curcode = 0;
				curlen = 0;
				break;
			case EXTABLE:
				count += te->count;
				curcode = 0;
				curlen = 0;
				break;
			default:
				fprintf(stderr, "G3: Bad table id from table entry.\n");
#ifndef ALLOW_G3_ERRORS
				exit(3);
#else
				++g3_error;
				return(-1);
#endif
			}
		}
	return (0);
}

int	g3_skiptoeol(FILE	 *fd)
	{
	while (g3_rawzeros<11 && !g3_eof)
		(void) g3_rawgetbit(fd);
	while(!g3_rawgetbit(fd) && !g3_eof);
	return(0);
}

int	g3_rawgetbit(FILE	 *fd)
	{
	int	b;
	static int	shdata;

	if (curbit >= 8) {
		shdata = fgetc(fd);
		if (shdata == EOF) {
#ifdef ALLOW_G3_ERRORS
		        if (g3_verb) fprintf(stderr, "G3: Premature EOF at line %d.\n", rows);
			g3_eols = 5;
			g3_eof = 1;
			++g3_error;
			return 0;
#else
			return(-1);
#endif
			}
		curbit = 0;
		}
	if (shdata & bmask[curbit]) {
		g3_Xrawzeros = g3_rawzeros;
		g3_rawzeros = 0;
		b = 1;
		}
	else {
		g3_rawzeros++;
		b = 0;
		}
	curbit++;
    return b;
}

int	g3_bitson(bit	 *b, int	 c , int	 n)
		{
	int	i, col;
	bit	*bP;
	static int	bitmask[] = { 0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01 };

	bP = b;
	col = c;
	bP+=(c/8);
	i = (c - ((c/8)*8));
	while(col <= (c+n)) { 
		for(;col <= (c+n) && i < 8; i++) {
			*bP |= bitmask[i];
			col++;
			}
		i = 0;
		bP++;
		}
	return(0);
}

/* All G3 images begin with a G3 EOL codeword which is eleven binary 0's
 * followed by one binary 1.  There could be up to 15 0' so that the image
 * starts on a char boundary.
 */
/*
 * They are all *supposed* to, but in fact some don't.  In fact pbmtog3 doesn't seem
 * to generate them.  So if that fails, we'll also try reading a line and seeing if
 * we get any errors.  Note that this means we had to move the call to g3_ident
 * to after the hash table init.  -nazgul
 */

int	g3_ident(FILE	 *fd)
	{

    int		ret = 0, col1, col2, i;
    byte	*tmpline;
    int		reverse = 0;
    struct cache	*dataptr;
    int			bufptr;
    
    g3_verb = 0;
    tmpline = (byte *) malloc(maxlinelen);

    /* In case this got reset by a previous pass through here */
    for (i = 0; i < 8; ++i) {
	bmask[7-i] = 1 << i;
    }

tryagain:
    curbit = 8;
    g3_Xrawzeros = 0;
    g3_eof = g3_eols = rows = cols = 0;
    
    /* If we have the zeros we're off to a good start, otherwise, skip some lines */
    for (g3_rawzeros = 0; !g3_rawgetbit(fd) && !g3_eof;);
    if (g3_Xrawzeros >= 11 && g3_Xrawzeros <= 15) {
	fd->dataptr = fd->data;
	fd->bufptr = 0;
	curbit = 8;
	g3_skiptoeol(fd);
	if (!g3_error) g3_skiptoeol(fd);
	if (!g3_error) g3_skiptoeol(fd);
	if (!g3_error) g3_skiptoeol(fd);
    } else ret = 1;

    /* Now get two lines and make sure they are the same length.  If not give up.
     * Note that it is possible for this to give false positives (value.o on a Sun IPC
     * did) but it's unlikely enough that I think we're okay.
     */
    
    dataptr = fd->dataptr;
    bufptr = fd->bufptr;
    if (!g3_error) col1 = g3_getfaxrow(fd, tmpline);
    if (!g3_error) col2 = g3_getfaxrow(fd, tmpline);
    if (!g3_error && col1 == col2 && col1 != 0) ret = 1;
    else ret = 0;
    /* if (ret) printf("%d = %d\n", col1, col2); */
    fd->dataptr = dataptr;
    fd->bufptr = bufptr;
    curbit = 8;

    /* This bogus hack is to accomodate some fax modems which apparently use a chip
     * with a different byte order.  We simply try again with the table reversed.
     */
    if (!ret && !reverse) {
	rows = cols = g3_error = 0;
	fd->dataptr = fd->data;
	fd->bufptr = 0;
	g3_Xrawzeros = 0;
	for (i = 0; i < 8; ++i) {
	    bmask[i] = 1 << i;
	}
	reverse = 1;
	goto tryagain;
    }
    g3_eols = rows = cols = 0;
    free(tmpline);

    return(ret);
}

int
g3::Load( const char  *fullname, FILE  *fp )
            {

	FILE	*fd;
	int i, col;
	byte	*currline;
	static int firstTime = 1;

	if((f = fp) == 0) {
	    if (! (f = fopen(fullname, "r"))) {
		fprintf(stderr, "Couldn't open g3 file %s.\n", fullname);
		return(-1);
	    }
	}

	if (firstTime) {
	    firstTime = 0;
	    
	    /* Initialize and load the hash tables */
	    for ( i = 0; i < HASHSIZE; ++i )
	      whash[i] = bhash[i] = (tableentry *) 0;
	    g3_addtohash(whash, twtable, TABSIZE(twtable), WHASHA, WHASHB);
	    g3_addtohash(whash, mwtable, TABSIZE(mwtable), WHASHA, WHASHB);
	    g3_addtohash(whash, extable, TABSIZE(extable), WHASHA, WHASHB);
	    g3_addtohash(bhash, tbtable, TABSIZE(tbtable), BHASHA, BHASHB);
	    g3_addtohash(bhash, mbtable, TABSIZE(mbtable), BHASHA, BHASHB);
	    g3_addtohash(bhash, extable, TABSIZE(extable), BHASHA, BHASHB);
	}
	
	g3_eof = g3_eols = 0;
	curbit = 8;	/* Reset on multiple reads */

	/* Calulate the number of bytes needed for maximum number of columns 
	 * (bits), create a temprary storage area for it.
	 */
	maxlinelen = BITS_TO_BYTES(MAXCOLS);

	if (!g3_ident(fd)) {
	    if(!fp) fclose(fd);
	    return(-1);
	}
	g3_verb = verbose;

	(this)->newBitImage(MAXCOLS, MAXROWS);

	currline = (this)->Data( );
	cols = 0;
	for (rows = 0; rows < MAXROWS; ++rows) {
		col = g3_getfaxrow(fd, currline);
#ifndef ALLOW_G3_ERRORS
		if (col < 0) {
		  (this)->Reset( );
		  if(!fp) fclose(fd);
		  return(-1);
		}
#else
		if (g3_error > MAX_ERRORS) {
		  (this)->Reset( );
		  if(!fp) fclose(fd);
		  return(-1);
	      }		    
#endif
		if (g3_eof)
			break;
		if (col > cols)
			cols = col;
		currline += BITS_TO_BYTES(cols);
		}

	if(!fp) fclose(fd);
	(this)->Width( ) = cols;
	(this)->Height( ) = rows;
	if (!(this)->Width( ) || !(this)->Height( )) { /* sanity check */
		(this)->Reset( );
		return(-1);
	}

    return(0);
}

/* originally this used only g3_ident to determine if it was a G3 image, but
 * it was always getting false positives so now it loads the whole image in
 * to see if it's reasonable.
 */
int	g3::Ident( const char	 *fullname )
	{ class g3 *g3 = new g3;

	g3_verb = 0;
	if ((g3)->Load(fullname, NULL) == 0) {
		(g3)->Destroy( );
		return(1);
	}
	return(0);
}

long
g3::Read( FILE  *file, long  id )
            {
    if((this)->Load( NULL, file ) == 0)
	return(dataobject_NOREADERROR);
    else
	return(dataobject_BADFORMAT);
}

long
g3::Write( FILE  *file, long  writeID, int  level )
                {
    return((this)->image::Write( file, writeID, level ));
}

long
g3::WriteNative( FILE  *file, const char  *filename )
            {
    return(0);
}
