/*LIBS: -lbasics -lclass -lmalloc -lerrors -lutil
*/

/*
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

#ifndef NORCSID
#define NORCSID
static char rcsid[]="$Header: /afs/cs.cmu.edu/project/atk-src-C++/atk/raster/convert/RCS/convrast.C,v 1.4 1994/08/11 19:06:08 rr2b Stab74 $";
#endif

/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/config/COPYRITE'     *
\* ********************************************************************** */

 

/* convrast.c - program to convert raster formats

    convertraster [-nulr] [-c(x,y,w,h)] [-pscale] [-intype=type] [-outtype=type] 
		[-infile=filename] [-outfile=filename]

    Convertraster reads the infile, generates an internal representation of the raster, 
    processes it as described by the switches, and writes the outfile.  

    If infile is omitted, standard input is read.  
    If outfile is omitted, statndard output is written.

    Switch definitions:

	-c(x,y,w,h)  Crop the output to the subimage of size (w, h) at (x,y).
        -pscale	Scale postscript output by this factor (1.0 by default)
	-n	Negative image.
	-u	Reflect across horizontal axis. (Swap Up and down). 
	-l	Reflect across vertical axis.  (Swap Left and right).
	-r	Rotate 90 degrees clockwise.  Multiple r's rotate 180, 270, . . .

    Cropping is done before the other processing.
    Postscript scaling is done after all other operations.

    The 'type's supported are: 

	MacPaint  - the format produced by MacPaint.
	RF - the old ITC RasterFile format
	raster - the BE2 .raster format
	PostScript - print data stream;  not supported for intype
	Xwd - X Window Dump
	bitmap - for X bitmap

    If the output type is not specified, a .raster file is generated.  
    If the input type is not specified, the program will examine 
    the first byte and use RF for 0xF1 and raster for '\'.  If the first
    byte doesn't help, the program halts.
*/


#include <andrewos.h> /* strings.h */
#include <ctype.h>
#include <rect.h>
#include <ATK.H>
#include <observable.H>
#include <proctable.H>
#include <dataobject.H>
#include <pixelimage.H>
#include <heximage.H>
#include <paint.H>
#include <oldRF.H>
#include <rasterio.H>
#include <xwdio.H>
#include <xbm.H>


char inname[1025], outname[1025];

enum rasterType {
	typeUnknown,
	typeMacPaint,
	typeRF,
	typeraster,
	typePostscript,
	typexwd,
	typexbm
}  inType, outType;

char opSwitches[20];	/* -runl */
struct rectangle crop;	/* -c(l,t,w,h) */
float PSscale;		/* -p */


	static void
ProcessPix(struct pixelimage *pix)
{
	struct pixelimage *tix;
	char *sx;
	struct rectangle r;

	if ( ! rectangle_IsEmptyRect(&crop)) {
		/* do the cropping by replacing the bits area of the pix  XXX */
		long buf[1000];
		register long row;

		tix = new pixelimage;
		(tix)->Resize(crop.width, crop.height);
		/* copy sub image to 'tix' */
		for (row = crop.top; row < crop.top+crop.height; row++) {
			(pix)->GetRow(crop.left, row, crop.width, (unsigned short *)buf);
			(tix)->SetRow(0, row-crop.top, crop.width, (unsigned short *)buf);
		}
		/* remove the bits from 'pix' */
		free((pix)->GetBitsPtr());
		(pix)->SetBitsPtr(NULL);
		/* resize 'pix' and give it the bits from 'tix' */
		(pix)->Resize(crop.width, crop.height);
		free((pix)->GetBitsPtr());
		(pix)->SetBitsPtr((tix)->GetBitsPtr());
		/* get rid of 'tix' */
		(tix)->SetBitsPtr(NULL);
		(tix)->Destroy();
	}

	for (sx = opSwitches; *sx; sx++) {
		rectangle_SetRectSize(&r, 0, 0, 
				(pix)->GetWidth(),
				(pix)->GetHeight());
		switch (*sx) {
		case 'n':	(pix)->InvertSubraster(&r); break;
		case 'u':	(pix)->MirrorUDSubraster(&r); break;
		case 'l':	(pix)->MirrorLRSubraster(&r); break;
		case 'r':
			tix = new pixelimage();	
			(pix)->GetRotatedSubraster(&r, tix);
			/* remove the bits from 'pix' */
			free((pix)->GetBitsPtr());
			(pix)->SetBitsPtr(NULL);
			/* resize 'pix' and give it the bits from 'tix' */
			(pix)->Resize(r.height, r.width);
			free((pix)->GetBitsPtr());
			(pix)->SetBitsPtr((tix)->GetBitsPtr());
			/* get rid of 'tix' */
			(tix)->SetBitsPtr(NULL);
			(tix)->Destroy();
			break;
		}
	}
}

	static long
ReadInputFile(FILE * InputFile, struct pixelimage *pix)
{
	switch (inType) {
	case typePostscript: 
	case typeUnknown: 
		return dataobject_BADFORMAT;
	case typeraster: 
	    return rasterio::ReadImage(InputFile, pix);
	case typeRF: 
	    return oldRF::ReadImage(InputFile, pix);
	case typeMacPaint: 
	    return paint::ReadImage(InputFile, pix);
	case typexwd: 
	    return xwdio::ReadImage(InputFile, pix);
	case typexbm: 
	    return xbm::ReadImage(InputFile, pix);
	}
}

	static void
WriteOutputFile(FILE * OutputFile, struct pixelimage *pix)
{
	struct rectangle r;
	rectangle_SetRectSize(&r, 0, 0, (pix)->GetWidth(), (pix)->GetHeight());

	switch (outType) {
	case typeUnknown: 
		return;
	case typeraster: 
	    rasterio::WriteImage(OutputFile, pix, &r);
		return;
	case typeRF: 
	    oldRF::WriteImage(OutputFile, pix, &r);
		return;
	case typeMacPaint: 
	    paint::WriteImage(OutputFile, pix, &r);
		return;
	case typePostscript: 
	    heximage::WritePostscript(OutputFile, pix, &r, (double) PSscale, (double) PSscale);
		return;
	case typexwd: 
	    xwdio::WriteImage(OutputFile, pix, &r);
		return;
	case typexbm: 
	    xbm::WriteImage(OutputFile, pix, &r);
		return;
	}
}




	static void
fail(char *msg)
{
	fprintf(stderr, "%s\n", msg);
	exit(1);
}


	static void
usage()
{
	fprintf(stderr, "usage: convertraster [-ruln] [-c(x,y,w,h)] [-p[scale]]");
		fprintf(stderr, " [intype=type] [infile=file] [outtype=type] [outfile=file]\n");
	fprintf(stderr, "usage:    Types are MacPaint, RF, raster, Xwindow, Xbitmap and Postscript\n");
			     exit(2);
}


struct symentry {	
	char *symbol;		/* string to recognize */
	void (*f)(char *sx, struct symentry *sym); /* function to call */
	char *target;		/* where the function assigns value */
	enum rasterType v;	/* value the function assigns */
};

static struct symentry *FindSym(char *buf);


	static void
storename(char *arg, struct symentry *sym)
{
	strcpy(sym->target, arg+1);
	if (strlen(sym->target) > 1023)
		fail ("file name too long");
}
	static void
storetype(char *arg, struct symentry *sym)
{
	struct symentry *typesym;
	char buf[20];
	strcpy(buf, arg+1);
	if (strlen(buf) > 18)
		fail ("type name too long");
	typesym = FindSym(buf);
	if (typesym == NULL)
		fail ("unknown type");
	*((enum rasterType *)sym->target) = typesym->v;			/* ! store a type value */
	
}

#define NUMFORMATS 18

	static struct symentry 
SymTable[NUMFORMATS] = {
	"infile", storename, inname, typeUnknown,
	"outfile", storename, outname, typeUnknown,
	"intype", storetype, (char *)&inType, typeUnknown,
	"outtype", storetype, (char *)&outType, typeUnknown,
	"macpaint", NULL, NULL, typeMacPaint,
	"macp", NULL, NULL, typeMacPaint,
	"mp", NULL, NULL, typeMacPaint,
	"rf", NULL, NULL, typeRF,
	"ras", NULL, NULL, typeRF,
	"rasterfile", NULL, NULL, typeRF,
	"raster", NULL, NULL, typeraster,
	"be2", NULL, NULL, typeraster,
	"ps", NULL, NULL, typePostscript,
	"postscript", NULL, NULL, typePostscript,
	"xwd", NULL, NULL, typexwd,
	"xwindow", NULL, NULL, typexwd,
	"xbitmap", NULL, NULL, typexbm,
	"", NULL, NULL, typeUnknown
};

	static struct symentry *
FindSym(char *s)
{
	char buf[12];
	char *bx = buf;
	struct symentry *tx;
	for ( ; *s && bx - buf < 10; s++)
		if (islower(*s)) *bx++ = *s;
		else if (isupper(*s)) *bx++ = tolower(*s);
	*bx++ = '\0';
	for (tx = SymTable; *tx->symbol; tx++)
		if (strcmp(tx->symbol, buf) == 0) 
			return tx;
	return NULL;
}


	static void
ParseSwitches(int argc, char **argv)
{
	struct symentry *sym;
	char buf[20];
	char *bx, *sx;
	char *arg;

	while (--argc > 0)  {
		arg = *++argv;
		if (*arg == '-')  arg++;		/* '-' is optional  !!! */
		switch (*arg) {
		case 'c': {
			long left, top, width, height;
			if (sscanf(arg+1, "(%d,%d,%d,%d)", 
					&left, &top, &width, &height) != 4)
				fail("crop with  -c(left,top,width,height)");
			rectangle_SetRectSize(&crop, left, top, width, height);
			} break;
		case 'n':
		case 'r':
		case 'u':
		case 'l':
			strcat(opSwitches, arg);
			if (strlen(opSwitches) > 18) 
				fail ("Too many n,l,r,u operations");
			break;
		case 'p':
			if (sscanf(arg+1, "%f", &PSscale) != 1)
			    fail("PS scale factor -pscale");
			break;
		case 'i':
		case 'o':
			for (bx = buf, sx = arg; 
				isalpha(*sx) && bx - buf < 18; 
				*bx++ = *sx++) {}
			*bx++ = '\0';
			sym = FindSym(buf);
			if (sym != NULL && sym->f != NULL)
				(sym->f)(sx, sym);
			else usage();
			break;	

		default: 
			usage();
		}
	}
}


	static FILE *
OpenInputFile()
{
	FILE *infile;
	register int c;

	if ( ! *inname)  
		infile = stdin;
	else
		infile = fopen(inname, "r");
	if (infile == NULL) 
		return NULL;

	/* check first byte and possibly change inType */
	ungetc((c=getc(infile)), infile);
	if (c == 0xF1)  inType = typeRF;
	else if (c == '\\') inType = typeraster;

	return infile;
}

	static FILE *
OpenOutputFile()
{
	char *ext = strrchr(outname, '.');
	if (*outname) {
		if (ext == NULL)
			fail("Output filename must have an extension");
		else switch (outType) {
		case typeraster:
			if (strcmp(ext, ".raster") != 0)
				fail("Output file extension for raster files must be \".raster\"");
			break;
		case typeRF:
			if (strcmp(ext, ".ras") != 0)
				fail("Output file extension for old RasterFile format must be \".ras\"");
			break;
		case typeMacPaint:
			if (strcmp(ext, ".mp") != 0)
				fail("Output file extension for MacPaint files must be \".mp\"");
			break;
		case typePostscript:
			if (strcmp(ext, ".ps") != 0)
				fail("Output file extension for Postscript files must be \".ps\"");
		        break;
		case typexwd:
			if (strcmp(ext, ".xwd") != 0)
				fail("Output file extension for X Window Dump files must be \".xwd\"");
			break;
		case typexbm:
			if (strcmp(ext, ".bitmap") != 0)
				fail("Output file extension for X bitmap files must be \".bitmap\"");
			break;
		}
		return fopen(outname, "w");
	}
	else return stdout;
}


main(int argc, char **argv)
{
	FILE *infile, *outfile;
	long ret;
	struct pixelimage *pix;

	*inname = '\0';
	*outname = '\0';
	inType = typeUnknown;
	outType = typeraster;
	*opSwitches = '\0';
	rectangle_EmptyRect(&crop);
	PSscale	= 1.0;		/* 1 to 1 mapping by default */

	ParseSwitches(argc, argv);

	if (inType == typePostscript)
		fail("convertraster can write Postscript, but cannot read it.");
	infile = OpenInputFile();
	if (infile == NULL)
		fail("Cannot open infile");
	if (inType == typeUnknown)
		fail("Input file is not Raster or RasterFile.  Do you want 'intype=xwd' or 'intype=xbitmap'?");

	outfile = OpenOutputFile();
	if (outfile == NULL) 
		fail ("Cannot open outfile"); 

	fprintf(stderr, "%s->%s\n", 
		(*inname) ? inname : "stdin" , 
		(*outname) ? outname : "stdout" );
	if ( ! rectangle_IsEmptyRect(&crop))
		fprintf(stderr, "	Crop input to (%d, %d, %d,%d)\n",
				crop.left, crop.top, crop.width, crop.height);
	if (*opSwitches) 
		fprintf(stderr, "	Process with \"%s\"\n", opSwitches);

	pix = new pixelimage;

	ret = ReadInputFile(infile, pix);
	if (ret != dataobject_NOREADERROR) {
		fprintf (stderr, "Read of %s failed with code %d\n", inname, ret);
		exit(3);
	}
	fclose(infile);

	ProcessPix(pix);

	WriteOutputFile(outfile, pix);
	fclose(outfile);
}
