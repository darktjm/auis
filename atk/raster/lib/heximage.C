/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

/*  heximage.c

	heximage package

	Routines for reading and writing rasters in hexadecimal
	as used in PostScript.

 */

#include <andrewos.h>
ATK_IMPL("heximage.H")
#include <stdio.h>
#include <math.h>


#include <heximage.H>
#include <pixelimage.H>


static unsigned char masks[] = {0xFF, 0x80, 0xC0, 0xE0, 0xF0, 0xF8, 0xFC, 0xFE};


static unsigned char    hex[16] = {
	'0', '1', '2', '3', '4', '5', '6', '7',
	'8', '9', 'a', 'b', 'c', 'd', 'e', 'f'
};


static unsigned char invhex[16] = {
	'f', 'e', 'd', 'c', 'b', 'a', '9', '8',
	'7', '6', '5', '4', '3', '2', '1', '0'
};

static unsigned char unhex[8][17] = {
	"\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0",
	"\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0",
	"\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0",
	"\0\1\2\3\4\5\6\7\10\11\0\0\0\0\0\0",
	"\0\12\13\14\15\16\17\0\0\0\0\0\0\0\0\0",
	"\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0",
	"\0\12\13\14\15\16\17\0\0\0\0\0\0\0\0\0",
	"\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0",
};


/* heximage__WriteRow(file, byteaddr, nbits, invert)  
	Writes to 'file' the encoded version of the 'nbits' bits
	beginning at byte with address 'byteaddr'.
	Invert the image if 'invert' is TRUE.
	The output stream has spaces every sixteen bytes, 
	and a newline at the end of the bytes for the row.
*/
	
ATKdefineRegistry(heximage, ATK, NULL);


void
heximage::WriteRow(FILE  *file, unsigned char *byteaddr, long  nbits, boolean  invert)
					{
	unsigned char *tbl = ((invert) ? invhex : hex);
	int widthbytes = (nbits+7) >> 3;
	int colfit = 8;

	while (widthbytes--) {
	    fputc(*(tbl + ((*byteaddr) / 16)), file);    /* write high order nibble */
	    fputc(*(tbl + ((*byteaddr) & 15)), file);    /* then low */
	    byteaddr += 1;				/* move to next byte of data */
	    if (--colfit == 0) {
		colfit = 8;
		fputc(' ', file);
	    }
	}
	fputc('\n', file);
}


/* heximage__ReadRow(file, row, length) 
	Copies one row of bits from 'file' to 'pix'
	returns 0 for success.  -1 for failure
*/
	long
heximage::ReadRow(FILE  *file		/* where to get bytes from */, unsigned char *row	/* where to put them to */, long  length		/* how many bits in row must be filled */)
				{
	long W = (length+7)>>3;/* number of bytes */
	unsigned char savebyte = *(row+W-1);	/* save last byte */
	unsigned char *where = row;
	long n = W;
	unsigned char *tbl = (unsigned char *)unhex;

	while (n--) {
		int c = getc(file);
		int c2 = getc(file);
		if (c == EOF ||  c2 == EOF) return -1;
		*where++ = (*(tbl+c)<<4) | *(tbl+c2);
	}

	if (length & 0x7) {
		/* fix the last byte if length is not a multiple of 8 bits */
		unsigned long mask = masks[length & 0x7];
		unsigned char *loc = row+W-1;
		*loc = (*loc & mask) | (savebyte & ~mask);
	}
	return 0;
}



/* heximage__ReadImage(file, pix, width, height) 
	Read a raster image from 'file' and put it in 'pix' 
		The width and height must be supplied 
		and will be stored in the pix.
		return error code 
*/
	long
heximage::ReadImage(FILE  *file		/* where to get bits from */, class pixelimage  *pix/* where to put them */, long  width , long  height)
				{
	unsigned char *where;		/* where to store next row */
	long row, W;		/* count rows;  byte length of row */
	
	W = (width+7)>>3;

	(pix)->Resize( width, height);
	where = (pix)->GetBitsPtr();
	W = (W+1) & ~1;			/* round up to a multiple of 16 bits */
	for (row = 0; row < height; row ++, where += W)
		heximage::ReadRow(file, where, width);
	(pix)->NotifyObservers( pixelimage_DATACHANGED);
	return 0;
}


/* heximage__WriteImage(file, pix, sub) 
	Write the raster image from 'pix' to 'file'
*/
	void
heximage::WriteImage(FILE  *file		/* where to put bits  */, class pixelimage  *pix	/* where to get them from */, struct rectangle  *sub)
				{
	long row;
	long top, left, width, height;
	short buf[BUFBITS>>4];

	rectangle_GetRectSize(sub, &left, &top, &width, &height);

	for (row = top;  row < top + height; row++) {
		(pix)->GetRow( left, row, width, (unsigned short *)buf); 
		heximage::WriteRow(file, (unsigned char *)buf, width, FALSE);
	}
}

/* Headers for postscript file.  The order here is reflected in the 'hx' stmts below. */
static const char * const PSheader[] = {
/* 1*/	"%%!PS-Adobe-2.0 EPSF-1.2\n",
/* 2*/	"%%%%BoundingBox: %d %d %d %d\n",
/* 3*/	"%%  This file written by the Andrew Toolkit's heximage object.\n",
/* 4*/	"%%  Bits are stored left-to-right, bottom-to-top;  1=white, 0=black. \n",
/* 5*/	"%%  The image is %0.4f\" wide and %0.4f\" high.\n",
/* 6*/	"  /width %ld def  /height %ld def		%% dimensions in pixels\n",
/* 7*/	"     /xScale %0.4f def /yScale %0.4f def	%% scaling factors\n",
/* 8*/	"     width xScale mul height yScale mul scale	%% set scaling\n",
/* not used anymore   "     0.03 0.015 translate                      %% create a small margin\n", */
/* 9*/	"     /picstr width 7 add 8 idiv string def  	%% define place to read lines\n",
/*10*/	"	%% the  \"image\" operator has five parameters:\n",
/*11*/	"	width height 1			%% dimensions (x, y, z)\n",
/*12*/"	[width    0    0    height    0    0]	%% transform mat<rix\n",
/*13*/	"	{currentfile picstr readhexstring pop}	%% bits source\n",
/*14*/	"     image					%% PRINT IT\n",
/*15*/	NULL
};

static const char * const PStrailer[] = {
/* 1*/	"\nshowpage	%% cause image to be printed\n",
/* 2*/	"%%  End of image written by Andrew Toolkit heximage object.\n",
	NULL
};

/* heximage__WritePostscript(file, pix, sub, xfrac, yfrac) 
	Write the raster image from 'pix' to 'file' as input suitable for postscript.
	Write only the portion selected by the subraster.

	    This routine assumes that each pixel in the image is 1/72 of an inch in size.
	    Unfortunately, this information is not kept with the raster data so we must
	    make assumptions.  72 pixels/inch is close to the size on many large screens.
*/
	void
heximage::WritePostscript(FILE  *file		/* where to put bits  */, class pixelimage  *pix/* where to get them from */, struct rectangle  *sub, double  xfrac , double  yfrac)
					{
	long row;
	long left, top, width, height;
	const char * const *hx;
	short buf[BUFBITS>>4];

	rectangle_GetRectSize(sub, &left, &top, &width, &height);

	/* write the header */

	hx = PSheader;
/* 1*/	fprintf(file, *hx++);
/* 2*/	fprintf(file, *hx++,
		0, 
		0, 
		(int) ceil((double) width * xfrac),
		(int) ceil((double) height * yfrac));
/* 3*/	fprintf(file, *hx++);
/* 4*/	fprintf(file, *hx++);
/* 5*/	fprintf(file, *hx++, width * xfrac / 72.0, height * yfrac / 72.0);
/* 6*/	fprintf(file, *hx++, width, height);
/* 7*/	fprintf(file, *hx++, xfrac, yfrac);
/* 8+*/	for (; *hx; hx++)
		fprintf(file, *hx);

	/* write the data */

	for (row = top+height-1;  row >= top; row--) {
		(pix)->GetRow( left, row, width, (unsigned short *)buf); 
		heximage::WriteRow(file, (unsigned char *)buf, width, TRUE);
	}


	/* write the trailer */

	hx = PStrailer;
	for (; *hx; hx++)
		fprintf(file, *hx);
}

