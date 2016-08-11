/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/config/COPYRITE'     *
\* ********************************************************************** */

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
static char rcsid[]="$Header: /afs/cs.cmu.edu/project/atk-src-C++/atk/raster/lib/RCS/raster.C,v 1.5 1996/10/18 19:43:38 wjh Exp $";
#endif


 

/* raster.c 

	raster Data-object

	Utilizes a rasterimage for actual storage of the image.


*/

#define RASTERVERSION	2

#define MAXFILELINE 255
#define BUFBYTES	600	/* enough for 4792 bits */

#include <andrewos.h>
ATK_IMPL("raster.H")
#include <stdio.h>
#include <sys/param.h> /* Defines MAXPATHLEN among other things */
#include <rect.h>
#include <util.h>

#include <attribs.h>
#include <raster.H>
#include <dataobject.H>
#include <rasterimage.H>
#include <pixelimage.H>	/* for ClipRange */
#include <rasterio.H>
#include <plusspace.H>
#include <oldRF.H>
#include <xwdio.H> /* for WriteOtherFormat */
#include <ctype.h>



ATKdefineRegistry(raster, dataobject, NULL);
#ifndef NORCSID
#endif
#ifdef NOTUSED
static void WriteV1Stream(class raster  *ras, FILE  *file, long  id);
#endif /* NOTUSED */
static long ReadRasterFile(register class raster   *self, FILE   *file, long  id);
static long ReadV1Raster(register class raster   *self, FILE   *file, long  id);
static long ReadV1SubRaster(class raster  *self, FILE  *file, register struct rectangle  *r);


raster::raster()
{
    this->pix = NULL;
    this->readOnly = FALSE;
    rectangle_EmptyRect(&this->subraster);
    /* default scaling is one half screen size */
    this->xScale = (raster_UNITSCALE * 12 + 12) / 25;
    this->yScale = (raster_UNITSCALE * 12 + 12) / 25;
    THROWONFAILURE( TRUE);
}

raster::~raster()
{
    (this)->SetPix( NULL);
}

class raster * raster::Create(register long  width , register long  height)
{
    register class raster *self = new raster;
    (self)->Resize( width, height);
    return self;
}

void raster::Resize(register long  width , register long  height)
{
    class rasterimage *pix = (this)->GetPix();
    if (pix == NULL) 
	(this)->SetPix( pix = rasterimage::Create(width, height));
    else {
	(pix)->Resize( width, height);
	rectangle_SetRectSize(&this->subraster, 0, 0, width, height);
	this->options = 0; }
}

void raster::ObservedChanged(class observable  *opix, long  status)
{
    class rasterimage *pix=(class rasterimage *)opix;
    if (status == observable_OBJECTDESTROYED) {
	/* the observed rasterimage is going away
	 we must not use raster_SetPix because
	 it will tinker the refcnt and try again to 
	 destroy the object */
	this->pix = NULL;
	return; }
    /* inform my own observers that the underlying rasterimage has changed */
    if ((pix)->GetResized()) {
	rectangle_SetRectSize(&this->subraster, 0, 0, 
			      (pix)->GetWidth(), (pix)->GetHeight());
	this->options = 0;
	this->xScale = raster_UNITSCALE / 2;
	this->yScale = raster_UNITSCALE / 2;
	/* pasieka's changing this to status. */
	(this)->NotifyObservers( raster_BOUNDSCHANGED);
	(this)->NotifyObservers( status); }
    else {
	/* notify observers of self only if the changed rectangle
	    of pix intersects with the subraster of self */
	struct rectangle R;
	R = this->subraster;
	rectangle_IntersectRect(&R, &R, (pix)->GetChanged());
	if ( ! rectangle_IsEmptyRect(&R))
	    /* pasieka's changing this to status. */
	    (this)->NotifyObservers( raster_BITSCHANGED);
	    (this)->NotifyObservers( status); }
}

void raster::SetPix(class rasterimage  *newpix)
{
    class rasterimage *pix = (this)->GetPix();
    if (newpix == pix) return;
    if (pix != NULL) 
	(pix)->RemoveObserver( this);
    this->pix = newpix;
    this->options = 0;
    if (newpix != NULL) {
	rectangle_SetRectSize(&this->subraster, 0, 0, 
			      (newpix)->GetWidth(),
			      (newpix)->GetHeight());
	(newpix)->AddObserver( this);
	/* XXX This is a kludge to kludge over the kludge in rasterview's RedrawRaster which creates a rasterimage data object when the view does not have a data object to view. */
	if (pix != NULL) {
	    (this)->NotifyObservers( raster_BITSCHANGED);
	    (this)->NotifyObservers( raster_BOUNDSCHANGED); } }
    else
	rectangle_EmptyRect(&this->subraster);
    /* XXX maybe we ought to notify observers of a BOUNDSCAHNGED
      even when newpix is NULL, but that value is used 
      when we are being Destroyed.  */
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - *
 *	WRITE routines
 *		entry points:
 *			raster__Write
 *			raster__WriteSubRaster
 *			raster__WriteShare   XXX
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
#ifdef NOTUSED
/*	WriteV1Stream(struct raster *ras, FILE *file, long id);
		write to 'file' the plusspace representation of 'ras' */
	static void
WriteV1Stream(class raster  *ras, FILE  *file, long  id)
			{
	register long nbytestofile;
	short buf[400];
	long x, y, w, h, yend;
	class pixelimage *pix = (class pixelimage *)(ras)->GetPix();

	rectangle_GetRectSize(&ras->subraster, &x, &y, &w, &h);
	fprintf(file, "\\begindata{%s,%d}\n", (ras)->GetTypeName(), id);
	fprintf(file, "%ld %u %d %d %d %d %ld %ld %d", 
			1, 0, 1, 1, 0, 0, w, h, 1);
		/* self->raster_version, self->raster_options, 
		self->raster_compression, self->raster_expansion, 
		self->raster_xoffset, self->raster_yoffset, 
		self->raster_width, self->raster_height, RASTERDEPTH  */

	/* write a "plusspace" version of the file, but only write the 
			selected subraster */
	nbytestofile = (w+7)>>3;
	yend = y + h;
	for ( ; y < yend; y++) {
		(pix)->GetRow( x, y, w, buf);
		plusspace::WriteRow(file, buf, nbytestofile);
	}

	fprintf(file, "\n\\enddata{%s,%d}\n", (ras)->GetTypeName(), id);
}
#endif /* NOTUSED */

/* raster__Write(self, file, writeID, level)
	Writes an ASCII data stream for 'self' to the designated 'file'
	Assumes the write has been done if WriteID int self->pix 
		has not changed from the last write.
	Assigns an object identifier for the data object, writes it in the header,
	Returns the assigned object identifier.
*/
	long
raster::Write(FILE  *file, long  writeID, int  level)
	 	 	 	 {
	register class rasterimage *pix = (this)->GetPix();
	long id = (this)->UniqueID();
	if (pix == NULL) {
		return 0;
	}
	if (this->writeID != writeID) {
		const char *name = (this)->GetTypeName();
		long x, y, width, height;	/* subraster parms */

		this->writeID = writeID;

/* XXX combine with options a bit saying this image is not shared
(so it doesn't need to be put in the dictionary and retained forever) */

		rectangle_GetRectSize(&this->subraster, &x, &y, &width, &height);
		fprintf(file, "\\begindata{%s,%d}\n", name, id);
		fprintf(file, "%ld %ld %ld %ld ", RASTERVERSION, 
				this->options, this->xScale, this->yScale);
		if ((pix)->GetWriteID() == writeID) {
			/* write a "refer" line */
			fprintf(file, "%ld %ld %ld %ld\n",
				 x, y, width, height);	/* subraster */
			fprintf(file, "refer %ld\n", (pix)->GetObjectID());
		}
		else if ((pix)->GetFileName()) {
			/* write a "file" line */
			char *path = (pix)->GetFilePath();
			if (path == NULL || *path == '\0')
				path = ".";	/* must write something non-white */
			fprintf(file, "%ld %ld %ld %ld\n",
				 x, y, width, height);	/* subraster */
			fprintf(file, "file %d %s %s \n", id, 
				(pix)->GetFileName(), path);

			(pix)->SetWriteID( writeID);
			(pix)->SetObjectID( id);
		}
		else if (pix->refcnt == 1) {
			/* write a "bits" version of the file, but only write the 
				selected subraster */
			register long nbytestofile = (width+7)>>3;
			short buf[400];
			register long yend = y + height;

			fprintf(file, "%ld %ld %ld %ld\n",
				 0, 0, width, height);	/* subraster */
			fprintf(file, "bits %ld %ld %ld\n", id, width, height);

			for ( ; y < yend; y++) {
				(pix)->GetRow( x, y, width, (unsigned short *)buf);
				rasterio::WriteRow(file, (unsigned char *)buf, nbytestofile);
			}

			(pix)->SetWriteID( writeID);
			(pix)->SetObjectID( id);
		}
		else {
			/* write a "bits" version of the file */
			register unsigned char *bits, *bitsend;
			register long W = (pix)->GetRowWidth();
			register long nbytestofile = ((pix)->GetWidth()+7)>>3;

			fprintf(file, "%ld %ld %ld %ld\n",
				 x, y, width, height);	/* subraster */
			fprintf(file, "bits %ld %ld %ld\n", id,
					(pix)->GetWidth(),
					(pix)->GetHeight());

			bits = (pix)->GetBitsPtr();
			bitsend = bits + W * (pix)->GetHeight();
			for ( ;  bits < bitsend; bits += W)
				rasterio::WriteRow(file, bits, nbytestofile);

			(pix)->SetWriteID( writeID);
			(pix)->SetObjectID( id);
		}
		fprintf(file, "\\enddata{%s, %d}\n", name, id);
	} /* end writeID != writeID */
	return(id);
}

/* raster__WriteSubRaster(self, file, objectid, sub)
		Write to 'file' the subraster 'sub'.  
		Use the object identifier 'objectid'.
		Returns the objectid.
*/
	long
raster::WriteSubRaster(register FILE  *file, long  objectid, struct rectangle  *sub)
				{
	register class rasterimage *pix = (this)->GetPix();
	const char *name = (this)->GetTypeName();
	struct rectangle R;
	long x, y, width, height;
	register long r;		/* count rows while putting */
	long nbytestofile;		/* bytes to output to for each row */
	unsigned short rowbits[BUFBYTES/2];

	if (pix == NULL) return 0;
	if (sub)
		rectangle_IntersectRect(&R, &this->subraster, sub);
	else R = this->subraster;
	rectangle_GetRectSize(&R, &x, &y, &width, &height);

	fprintf(file, "\\begindata{%s,%d}\n", name, objectid);
	fprintf(file, "%ld %ld %ld %ld %ld %ld %ld %ld\n", RASTERVERSION, 
			this->options, this->xScale, this->yScale,
			 0, 0, width, height);	/* subraster is the whole */
	fprintf(file, "bits %ld %ld %ld\n", objectid, width, height);

	nbytestofile = (width+7)>>3;
	for (r = 0; r < height; r++) {
		(pix)->GetRow( x, y+r, width, rowbits);
		rasterio::WriteRow(file, (unsigned char *)rowbits, nbytestofile);
	}
	fprintf(file, "\\enddata{%s, %d}\n", name, objectid);
	return objectid;
}

/* raster__WriteShare(self, file, sub)
		write a "share" record for the indicated 'subraster' of 'self'
*/
	void
raster::WriteShare(register FILE  *file, struct rectangle  *sub)
			{
	register class rasterimage *pix = (this)->GetPix();
	const char *name = (this)->GetTypeName();
	struct rectangle R;
	long x, y, width, height;

	if (pix == NULL) return;

	rectangle_IntersectRect(&R, &this->subraster, sub);
	rectangle_GetRectSize(&R, &x, &y, &width, &height);

	fprintf(file, "\\begindata{%s, %d}\n", name, 0);
	fprintf(file, "%ld %ld %ld %ld %ld %ld %ld\n", RASTERVERSION, 
			this->options, this->xScale, this->yScale,
			 0, 0, width, height);	/* subraster is the whole */

	fprintf(file, "share %d 0x%lx \n", getpid(), pix);

	fprintf(file, "\\enddata{%s, %d}\n", name, 0);
}


/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - *\
 *
 *	READ routines
 *		Major entry points:
 *			raster__Read 
 *			raster__ReadSubRaster
 *
 *	These routines recognize only the be2 raster formats,
 *	versions 1 and 2 and the original ITC RasterFile
\* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

/* ReadRasterFile(self, file, id)
	reads a file in RasterFile format
	the leading 0xF1 has already been read.

*/
	static long
ReadRasterFile(register class raster   *self, FILE   *file, long  id)
			{
	register class rasterimage *pix = (self)->GetPix();
	long retval;

	if (pix == NULL) 
		(self)->SetPix( pix = new rasterimage);

	retval = oldRF::ReadImage(file, pix);

	/* the next line will not change the pixelimage
		because it already has the right size.
		It clears out the options and subraster in the rasterimage */
	(self->pix)->SetRO(self->readOnly);
	(self)->Resize( (pix)->GetWidth(), 
				(pix)->GetHeight());
	(pix)->NotifyObservers( rasterimage_DATACHANGED);
	return retval;
}


/* ReadV1Raster(self, file)
	Read a version 1 raster image from stream 'file' and store in 'self'.
	The version field has already been read by the _Read routine above.
*/
/* Format of a compressed raster datastream:   (Version 1)

	 Offset is from AFTER datastream marker

	type	what it is
	----	---- -- --

	long	version
	long	options
	long	compression
	long	expansion
	long	xoffset
	long	yoffset
	long	width
	long	height
	long	depth

*/

/* bits of 'options' value */
#define  V1INVERT	(1<<1)
#define  V1FLIP		(1<<2)
#define  V1FLOP		(1<<3)
#define  V1ROTATE	(1<<4)

/* ReadV1Raster(self, file, id)
	Reads a version 1 BE2 raster image from file to self.

	The subraster, options, and expansion/contraction are set
	fromthe values in the record.
*/
	static long
ReadV1Raster(register class raster   *self, FILE   *file, long  id)
			{
	register class rasterimage *pix = (self)->GetPix();
	unsigned long options;
	long depth, compression, expansion;
	long xoffset, yoffset,  height, width;
	register long row, W;
	register unsigned char *byteaddr;
	register long nbytesfromfile;
	long retval = 0;

	char s[MAXFILELINE + 2];

	fscanf(file, " %u %ld %ld %ld %ld %ld %ld %hd",  
		&options, &compression, &expansion, &xoffset, 
		&yoffset, &width, &height, &depth);

	if (pix == NULL) {
		(self)->SetPix( pix = rasterimage::Create(width, height));
	}
	else (self)->Resize( width, height);

	rectangle_SetRectSize(&self->subraster, xoffset, yoffset, 
			width-xoffset, height-yoffset);
	self->options = options>>1;		/* XXX kludge the V1 options are
					shifted over one from the V2 options */

	/* XXX expansion/compression affect only the print size.  Is this right? */
	if (expansion > 1  && compression > 1) {
		long scale;
		scale = (expansion * raster_UNITSCALE / 2) / compression;
		self->xScale = self->yScale = scale;
	}

	W = (pix)->GetRowWidth();
	nbytesfromfile = (width+7)>>3;
	byteaddr = (pix)->GetBitsPtr();
	for (row = 0;   row < height;   row++, byteaddr += W) {
		if (plusspace::ReadRow(file, byteaddr, nbytesfromfile) 
				!= dataobject_NOREADERROR) {
			retval = dataobject_BADFORMAT;
			break;
		}
	}
	
	/* If plusspace_ReadRow ever gets to a backslash, it remains at the read point
		(via the magic of ungetc) */
	while ( ! feof(file) && getc(file) != '\\') {};	/* scan for \enddata */
	fgets(s, MAXFILELINE + 2, file);	/* discard \enddata{...}  */
	(pix)->SetRO(self->readOnly);
	(pix)->NotifyObservers( rasterimage_DATACHANGED);
	return retval;
}


/* ReadV1SubRaster(self, file, r)
	Reads from 'file' a data stream containing a V1 raster
	and stores it at upper left of subraster 'r' of 'self'

	NOT IMPLEMENTED     XXX
	It now just skips the image in the file 
*/
	static long
ReadV1SubRaster(class raster  *self, FILE  *file, register struct rectangle  *r)
			{
	char s[MAXFILELINE + 2];
	while (getc(file) != '\\') {}
	fgets(s, MAXFILELINE + 2, file);	/* discard \enddata{...}  */	

	return dataobject_OBJECTCREATIONFAILED;
}

/* raster__Read(self, file, id)
		Read from 'file' a data stream for a raster and
		store the raster in 'self'.  'id' is ignored.

		See file format in rasterspec.d.
		This routine reads the \begindata, if any. Its syntax is not checked.
		This routine reads the \enddata, if any. Its syntax is also not checked.
		If the version number read from the file is 1, 
		then the old Read routine is called.

	XXX should check for read errors
*/
	long
raster::Read(register FILE   *file, long   id			/* !0 if data stream, 0 if direct from file*/)
			{
	register class rasterimage *pix = (this)->GetPix();
	long version, width, height;
	register long row, W;
	register unsigned char *byteaddr;
	register nbytesfromfile;
	long options, xscale, yscale, xoffset, yoffset, subwidth, subheight;
	char keyword[6];
	long objectid;	/* id read for the incoming pixel image */
	char filename[256], path[1024];
	FILE *f2;		/* for "file" keyword */
	long result = dataobject_BADFORMAT;

	char s[MAXFILELINE + 2];
	long tc;

	this->xScale = this->yScale = raster_UNITSCALE / 2;

	if (file == NULL) return dataobject_PREMATUREEOF;

	/* check for RasterFile magic number */
	ungetc(tc=getc(file), file);
	if (tc == 0xF1 || tc==0) {
		if (pix != NULL) (pix)->Defile();
		return ReadRasterFile(this, file, id);
	}
	else if (tc == '\\') {
		/* accept "\begindata{raster,123123123}"
			even though it is a mistake */
		long discardid;
		if (fscanf(file, "\\begindata{raster,%ld", &discardid) != 1
				|| getc(file) != '}' || getc(file) != '\n') 
			return dataobject_NOTBE2DATASTREAM;
	}

	/* XXX check for "\begindata{raster," */

	/* check for version 1 */
	fscanf(file, " %ld ", &version);
	if (version == 1) {
		if (pix != NULL) (pix)->Defile();
		return ReadV1Raster(this, file, id);
	}

	/* it is a be2 version 2 raster image */

	/* read the rest of the first line of header */
	fscanf(file, " %u %ld %ld %ld %ld %ld %ld",  
		&options, &xscale, &yscale, &xoffset, 
		&yoffset, &subwidth, &subheight);

	/* scan to end of line in case this is actually something beyond V2 */
	while (((tc=getc(file)) != '\n') && (tc != '\\')
	       && (tc != EOF)) {}

	/* read the keyword */
	fscanf(file, " %5s", keyword);

	switch (*keyword) {
	case 'r':	{		/* "refer" type */
		class rasterimage *addr;
		fscanf(file, " %d ", &objectid);

		addr=NULL;/* XXX Lookup the objectid in a namespc.  set addr  */

		if (FALSE  /* XXX object is in namespace */ )  {
			(this)->SetPix( pix=addr);
			result = dataobject_NOREADERROR;
		}
		else result = dataobject_OBJECTCREATIONFAILED;
	}	break; 

	case 's':	{            /* "share" type */
		long pid;
		class rasterimage *addr;
		fscanf(file, " %d %x ", &pid, &addr);
		if (pid == getpid()  && strcmp((addr)->GetTypeName(), 
						"rasterimage")==0) {
			(this)->SetPix( pix=addr);
			result = dataobject_NOREADERROR;
		}
		else result = dataobject_OBJECTCREATIONFAILED;
	}	break;

	case 'f':	/* "file" type */
		/* XXX creating a pix here is bogus
			remove when fix the filename stuff */
		if (pix == NULL)
			(this)->SetPix( pix = new rasterimage);
		(pix)->Defile();
		fscanf(file, "%d %255s %1023s ", &objectid, filename, path);
		f2 = (pix)->FindFile( filename, path);
		result = (this)->Read( f2, 0);

		/* XXX BUG BUG   the raster_Read will Defile(self) */

		(pix)->SetRO(this->readOnly);
		/* XXX enter rasterimage into namespace with objectid as key
		unless the option bit says it is singly referenced */
	
		((this)->GetPix())->NotifyObservers(
				rasterimage_DATACHANGED);
		break;

	case 'b':	/* "bits" type */
		if (strcmp(keyword, "bits") != 0) {
			result = dataobject_BADFORMAT;
			break;
		}
		/* fix this to not Defile ??? XXX */
		if (pix) (pix)->Defile();
		fscanf(file, " %d %d %d ", &objectid, &width, &height);

		if (width < 1 || height < 1 || width > 1000000 || height > 1000000) {
			result = dataobject_BADFORMAT;
			break;
		}

		if (pix == NULL)
			(this)->SetPix( pix = rasterimage::Create(width, height));
		else (this)->Resize( width, height);
		W = (pix)->GetRowWidth();
		nbytesfromfile = (width+7)>>3;
		byteaddr = (pix)->GetBitsPtr();
		result = dataobject_NOREADERROR;
		for (row = 0;   row < height;   row++, byteaddr += W) {
			long c = rasterio::ReadRow(file, byteaddr, nbytesfromfile);
			if (c != '|') {
				result = (c == EOF) 
					? dataobject_PREMATUREEOF
					: dataobject_BADFORMAT;
				break;
			}
		}
	
		/* If ReadRow ever gets to a backslash, it remains at the read point
			(via the magic of ungetc) */
		while (! feof(file) && getc(file) != '\\') {};	/* scan for \enddata */
		fgets(s, MAXFILELINE + 2, file);	/* discard \enddata{...}  */
		if (result == dataobject_NOREADERROR &&
				strncmp(s, "enddata{raster,", 
					strlen("enddata{raster,")) != 0) 
			result = dataobject_MISSINGENDDATAMARKER;

		/* XXX enter rasterimage into dictionary with objectid as key 
		unless bit in options says it is singly referenced */
	
		(pix)->NotifyObservers( rasterimage_DATACHANGED);

		break;

	}  /* end of switch(*keyword) */

	if (result == dataobject_NOREADERROR) {
	rectangle_SetRectSize(&this->subraster, 
		xoffset, yoffset, subwidth, subheight);
		this->options = options;
		this->xScale = xscale;
		this->yScale = yscale;
		(pix)->SetRO(this->readOnly);
	}
	return result;
}


/* raster__ReadSubRaster(self, file, r)
		Read from 'file' into the upper left corner of subraster 'r' of 'self' 
		See other comments in raster_Read, above.
*/
	long
raster::ReadSubRaster(FILE  *file, struct rectangle  *r)
			{
	register class rasterimage *pix = (this)->GetPix();
	long version, width, height;
	register long nbytesfromfile;
	unsigned short buffer[BUFBYTES/2];
	register long ylim;
	long x, y, w, h;
	long options, xscale, yscale, xoffset, yoffset, subwidth, subheight;
	char rastertype [6];
	long objectid, fullwidth, fullheight;
	struct rectangle Dest;
	long result = dataobject_NOREADERROR;

	char s[MAXFILELINE + 2];
	
	fscanf(file, " %ld ", &version);
	if (version == 1) 
		return ReadV1SubRaster(this, file, r);

	fscanf(file, " %ld %ld %ld %ld %ld %ld %ld ",
		&options, &xscale, &yscale, &xoffset, &yoffset, &subwidth, &subheight);
	fscanf(file, " %5s %ld %ld %ld ", rastertype, &objectid, &fullwidth, &fullheight);

	if (strcmp(rastertype, "bits") != 0  
			||  fullwidth != subwidth  
			||  fullheight != subheight)
		/* XXX skip unimplemented operations */
		return 0;

	width = subwidth; 
	height = subheight;

	if (pix == NULL)
		(this)->SetPix( pix = rasterimage::Create(rectangle_Left(r)+width, 
					rectangle_Top(r)+height));
	
	rectangle_SetRectSize(&Dest, rectangle_Left(r), rectangle_Top(r), width, height);
	rectangle_IntersectRect(&Dest, &Dest, r);
	rectangle_IntersectRect(&Dest, &Dest, &this->subraster);
	rectangle_GetRectSize(&Dest, &x, &y, &w, &h);

	nbytesfromfile = (width+7)>>3;	/* amount to read */
	ylim = y + h;
	for ( ;   y < ylim;   y++) {
		long c = rasterio::ReadRow(file, (unsigned char *)buffer, nbytesfromfile);
		if (c != '|') {
			result = (c == EOF) 
				? dataobject_PREMATUREEOF
				: dataobject_BADFORMAT;
			break;
		}
		(pix)->SetRow( x, y, w, buffer);
	}
	
	/* If ReadRow ever gets to a backslash, it remains at the read point
		(via the magic of ungetc) */
	while ( ! feof(file) && getc(file) != '\\') {};	/* scan for \enddata */
	fgets(s, MAXFILELINE + 2, file);	/* discard \enddata{...}  */
	(pix)->SetRO(this->readOnly);
	(pix)->NotifyObservers( rasterimage_DATACHANGED);
	return result;
}

void 
raster::SetAttributes(struct attributes  *attributes)
    {
  class rasterimage *pix = (this)->GetPix();

  while(attributes) {
    if(!strcmp(attributes->key, "readonly")) {
      this->readOnly = attributes->value.integer;
      if(pix)
	  (pix)->SetRO(this->readOnly);
    }
    attributes = attributes->next;
  }
}

static int tmpfilectr = 0;

long raster::WriteOtherFormat(FILE  *file, long  writeID, int  level, int  usagetype, char  *boundary)
{
    FILE *tmpfp;
    char Fnam[1000];

    if (this->writeID == writeID)  return(this->id);
    this->writeID = writeID;
    
    fprintf(file, "\n--%s\nContent-type: image/x-xwd\nContent-Transfer-Encoding: base64\n\n", boundary);
    
    sprintf(Fnam, "/tmp/rastxwd.%d.%d", getpid(), tmpfilectr++);
    tmpfp = fopen(Fnam, "w");
    if (!tmpfp) return(0);
    (this->pix)->InvertSubraster( &this->subraster); /* ugh */
    xwdio::WriteImage(tmpfp, this->pix, &this->subraster);
    (this->pix)->InvertSubraster( &this->subraster); /* reverse the ugh */
    fclose(tmpfp);
    tmpfp = fopen(Fnam, "r");
    if (!tmpfp) return(0);
    to64(tmpfp, file);
    fclose(tmpfp);
    unlink(Fnam);
    return(this->id);
}

boolean raster::ReadOtherFormat(FILE  *file, char  *fmt, char  *encoding, char  *desc)
{
    char TmpFile[250];
    FILE *tmpfp = NULL;
    int code;
    class rasterimage *pix;

    if (strcmp(fmt, "image/xwd")
	 && strcmp(fmt, "image/x-xwd")) return(FALSE);
    /* Need to decode base64 or q-p here */
    if (!strncmp(encoding, "base64", 6)
	 || !strncmp(encoding, "quoted-printable", 16)) {
	sprintf(TmpFile, "/tmp/rastxwd.%d.%d", getpid(), tmpfilectr++);
	tmpfp = fopen(TmpFile, "w");
	if (!tmpfp) return(FALSE);
	if (!strncmp(encoding, "base64", 6)) {
	    from64(file, tmpfp);
	} else {
	    fromqp(file, tmpfp);
	}
	fclose(tmpfp);
	tmpfp = fopen(TmpFile, "r");
	if (!tmpfp) return(FALSE);
	file = tmpfp;
    }

    pix = (this)->GetPix();
    if (pix == NULL) 
	(this)->SetPix( pix = new rasterimage);
    code = xwdio::ReadImage(file, (this)->GetPix());
    if (tmpfp) {
	fclose(tmpfp);
	unlink(TmpFile); 
    }
    if (code == dataobject_NOREADERROR) {
	(this->pix)->InvertSubraster( &this->subraster);	
	return(TRUE);
    } else {
	return (FALSE);
    }
}