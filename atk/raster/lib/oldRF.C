/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

/*  oldrf.c

	oldRF package

	Routines for reading and writing rasters in the form 
	defined by rastfile.h, long ago.

	There is a 14 or 16 byte header followed by the bits.
	Some files are even 18 bytes longer than required.  These seem 
	to have a 16 byte header.

 */

#include <andrewos.h> /* sys/types.h */
ATK_IMPL("oldRF.H")
#include <stdio.h>
#include <sys/stat.h>
#include <netinet/in.h>

#include <rastfile.h>


#include <oldRF.H>
#include <pixelimage.H>	
#include <dataobject.H>

#define DEBUG(x)  {printf x; fflush(stdout);}

#define SWAB(v) ( ((v>>8) & 0xFF) | ((v&0xFF)<<8) )
#define SWAL(v) ( (SWAB(v)<<16)  |  SWAB(v>>16) )

static unsigned char masks[] = {0xFF, 0x80, 0xC0, 0xE0, 0xF0, 0xF8, 0xFC, 0xFE};



/* oldRF__WriteRow(file, row, length)  
		Writes to 'file' the encoded version of the 'length' bytes
		beginning at byte with address 'row'.
*/
	
ATKdefineRegistryNoInit(oldRF, ATK);


void
oldRF::WriteRow(FILE  *file, unsigned char *row, long  length)
				{
	unsigned char buf[BUFBITS>>3];	/* temp for inversion */
	unsigned char *sx;			/* where to get bytes */
	unsigned char *dx;			/* where to put them */
	long W = (length+7)>>3;	/* total number of bytes */

	/* convert colors */
	for (sx = row, dx = buf; sx < row+W; )
		*dx++ = ~*sx++;
	
	fwrite(buf, W, 1, file);
}


/* oldRF__ReadRow(file, row, length) 
	Copies one row of bits from 'file' to 'pix'
	returns 0 for success.  -1 for failure
*/
	long
oldRF::ReadRow(FILE  *file			/* where to get bytes from */, unsigned char *row		/* where to put them */, long  length		/* how many bits in row must be filled */)
				{
	long W = (length+7)>>3;/* number of bytes */
	unsigned char savebyte;

	savebyte = *(row+W-1);		/* save last byte */
	if (fread(row, W, 1, file) < 1) /* read the bytes */
		return dataobject::PREMATUREEOF;  /* report error, if any */

	/* convert colors */
	if (((unsigned long)row) & 3) {
		/* do first partial word */
		unsigned char *x;
		for (x = row; ((unsigned long)x)&3; x++)
			*x = ~*x;
	}
	{
		/* do full words */
		unsigned long *lx;
		for (lx = (unsigned long *)(((unsigned long)(row+W-4))&(~3)); 
				lx >= (unsigned long *)row; lx --)
			*lx = ~*lx;
	}
	if (((unsigned long)(row+W))&3) {
		unsigned char *x;
		for (x = row+W; ((unsigned long)(x--))&3; )
			*x = ~*x;
	}

	if (length & 0x7) {
		/* fix the last byte if length is not a multiple of 8 bits */
		long mask = masks[length & 0x7];
		unsigned char *loc = row+W-1;
		*loc = (*loc & mask) | (savebyte & ~mask);
	}
	return dataobject::NOREADERROR;
}



/* oldRF__ReadImage(file, pix) 
	Read a raster image from 'file' and put it in 'pix' 
		return error code
*/
	long
oldRF::ReadImage(FILE  *file		/* where to get bits from */, class pixelimage  *pix/* where to put them */)
			{
	struct stat st;			/* buffer to read stat info */
	struct RasterHeader hdr;	/* buffer to read file header */
	unsigned char *where;		/* where to store next row */
	long row, W;		/* count rows;  byte length of row */

	if (fread(&hdr, 14, 1, file) < 1)   /* read the header */
		  return dataobject::PREMATUREEOF;

	if (hdr.Magic == RasterMagic) {
		/* it is a proper raster file */
	}
	else if (hdr.Magic == SWAL(RasterMagic)) {
		/* it is a raster file with swapped bytes */
		hdr.width = SWAL(hdr.width);
		hdr.height = SWAL(hdr.height);
	}
	else {
		fprintf(stderr, "File starts w/ F1, but magic # is 0x%lx\n",
			hdr.Magic);
		fflush(stderr);
		return dataobject::BADFORMAT;
	}

	W = (hdr.width+7)>>3;

	/* kludge to test for files written with a sixteen byte header   XXX */
	if (fstat(fileno(file), &st) < 0)  return -2;
	if (st.st_size != 14 + hdr.height * W)
		/* assume it is a 16 byte header
		     read and discard 2 bytes */
		fread(&hdr.depth, 2, 1, file);

	(pix)->Resize( hdr.width, hdr.height);
	where = (pix)->GetBitsPtr();
	W = (W+1) & ~1;		/* round up to a multiple of 16 bits */
	for (row = 0; row < hdr.height; row ++, where += W)
		oldRF::ReadRow(file, where, hdr.width);
	(pix)->NotifyObservers( pixelimage_DATACHANGED);
	return dataobject::NOREADERROR;
}

/* oldRF__WriteImage(file, pix, sub) 
	Write a raster image from 'pix' to 'file'
*/
	void
oldRF::WriteImage(FILE  *file		/* where to put bits  */, class pixelimage  *pix/* where to get them from */, struct rectangle  *sub)
				{
	long left, top, width, height;
	long buf[BUFBITS>>5];
	struct RasterHeader hdr;	/* buffer to write file header */
	long row;

	rectangle_GetRectSize(sub, &left, &top, &width, &height);

	hdr.Magic = htonl(RasterMagic);
	hdr.width = htonl(width);
	hdr.height = htonl(height);
	hdr.depth = htons(1);

	fwrite(&hdr, 14, 1, file);

	for (row = top;  row < top + height; row++) {
		(pix)->GetRow( left, row, width, (unsigned short *)buf);
		oldRF::WriteRow(file, (unsigned char *)buf, width);
	}
}

