/* cmuwm.c - class description for ATK raster interface to image */
/*
	Copyright Carnegie Mellon University 1992 - All rights reserved
*/

#include <andrewos.h>
ATK_IMPL("cmuwm.H")
#include <image.H>
#include <dataobject.H>
#include <filetype.H>
#include <raster.H>
#include <rasterimage.H>

#include <cmuwm.H>


ATKdefineRegistry(cmuwm, image, NULL);

	int 
cmuwm::Ident( const char  *fullname ) {
    FILE *f;
    int r = 0;
    long objectID;
    char *objectName;

    if (!(f = fopen(fullname, "r")))
	return(0);

    if((objectName = filetype::Lookup(f, fullname, &objectID, NULL)))
	r = !(strcmp(objectName, "raster"));
    fclose(f);
    return r;
}

	int
cmuwm::Load( const char  *fullname, FILE  *fp ) {
    FILE *f;
    class raster *rasterp = new raster;
    long objectID;
    char *objectName;

    if(rasterp==NULL) {
	fprintf(stderr, "Couldn't create raster to load image from.\n");
	return -1;
    }
    
    if((f = fp) == 0) {
	if (! (f = fopen(fullname, "r"))) {
	    fprintf(stderr, "Couldn't open file %s for reading.\n", fullname);
	    return(-1);
	}
    }

    if((objectName = filetype::Lookup(f, fullname, &objectID, NULL))) {
	long status = 0;
	if((status = (rasterp)->Read( f, objectID)) == dataobject::NOREADERROR && (rasterp)->GetPix()!=NULL) {
	    int i;
	    int w = (rasterp)->GetWidth(), h = (rasterp)->GetHeight();
	    int widthBytes = ((w)/8) + ((w)%8 ? 1 : 0);
	    unsigned short *buf=(unsigned short *)malloc(widthBytes+(widthBytes&1));
	    unsigned char *ibuf;
	    if(buf==NULL) {
		(rasterp)->Destroy();
		return -1;
	    }
	    memset(buf,0, widthBytes+(widthBytes&1));
	    (this)->newBitImage( w, h);
	    ibuf=(this)->Data();
	    for(i=0;i<h;i++) {
		((rasterp)->GetPix())->GetRow( 0, i, w, buf);
		memmove(ibuf, buf, widthBytes);
		ibuf+=widthBytes;
	    }
	    (rasterp)->Destroy();
	    free(buf);
	}
	else {
	    fprintf(stderr, "cmuwm: Read error (%ld)\n", status);
	    if(!fp) fclose(f);
	    return(-1);
	}
    }
    else {
	fprintf(stderr, "objectName: %s\n", objectName);
	if(!fp) fclose(f);
	return(-1);
    }
    if (fp == NULL) /* If fp is NULL, this means we opened filename and so must also close it. */
	fclose(f);
    return(0);
}

	long
cmuwm::Read( FILE  *file, long  id ) {
    if((this)->Load( NULL, file) == 0)
	return(dataobject::NOREADERROR);
    else return(dataobject::BADFORMAT);
}

	long
cmuwm::Write( FILE  *file, long  writeID, int  level )  {
    return((this)->image::Write( file, writeID, level));
}

	long
cmuwm::WriteNative( FILE  *file, const char  *filename ) {
    class raster *ras;
    FILE *f;
    int w = (this)->Width(), h = (this)->Height();
    long status;

    if((f = file) == 0) {
	if (! (f = fopen(filename, "w"))) {
	    fprintf(stderr, "Couldn't open file %s for writing.\n", filename);
	    return(-1);
	}
    }

    if ((this)->Type() != IBITMAP) /* If the image, which isn't the original because we duplicated the original to self, isn't of type IBITMAP, dither it down. */
	(this)->Dither();

    if((ras = raster::Create(w, h)) && (ras)->GetPix()) {
	int widthBytes = w/8 + (w%8 ? 1 : 0);
	int i;
	unsigned char *ibuf=(this)->Data();
	unsigned short *buf=(unsigned short *)malloc(widthBytes+(widthBytes&1));
	if(buf==NULL) {
	    if(file==NULL) fclose(f);
	    (ras)->Destroy();
	    return -1;
	}
	for(i=0;i<h;i++) {
	    memmove(buf, ibuf, widthBytes);
	    ((ras)->GetPix())->SetRow( 0, i, w, buf);
	    ibuf+=widthBytes;
	}
	if((status = (ras)->Write( f, (ras)->UniqueID(), -1)) != (ras)->UniqueID()) {
	    if(file==NULL) fclose(f);
	    fprintf(stderr, "raster: Write error (%ld)\n", status);
	    return(-1);
	}
	(ras)->Destroy();
	if (file == NULL) /* this means we opened filename; so we must also close it. */
	    fclose(f);
    }
    return(0);
}
