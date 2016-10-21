/* Generic Image I/O (i.e., supports many file formats) */
/* ImageMagick (http://www.imagemagick.org/) MagickWand implementation */
/* tested with 6.9.5.9. */
/* skeleton of sorts copied from jpeg.C.  Bite me, IBM.  */
#include <andrewos.h>
ATK_IMPL("magick.H")
#include <image.H>
#include <imageio.H>
/* C API is better because it's not quite so C++-centric */
/* and ATK is, after all, basically C */
#include <wand/MagickWand.h>

static boolean magick_init()
{
    MagickWandGenesis();
    return TRUE;
}

ATKdefineRegistry(imageio, image, magick_init);

static void magick_err(MagickWand *wand)
{
    ExceptionType sev;
    const char *msg = MagickGetException(wand, &sev);
    if(!msg)
	msg = "??";
    fprintf(stderr, "ImageMagick error: %s\n", msg);
    DestroyMagickWand(wand);
}

long imageio::WriteNative(FILE  *file, const char  *filename)
{
    MagickWand *wand = NewMagickWand();
    bool ret;
    unsigned long size = 0;

    if(!wand)
	return -1;
    /* simpler than using pixel iterators: xfer as a raw graphics "blob" */
    /* hard to say if it's faster or not, but it's probably good enough */
    switch(this->Type()) {
      case IBITMAP:
	MagickSetFormat(wand, "gray");
	MagickSetDepth(wand, 1);
	size = (this->Width() + 7) / 8 * this->Height();
	break;
      case IGREYSCALE:
	MagickSetFormat(wand, "gray");
	MagickSetDepth(wand, 8);
	size = this->Width() * this->Height();
	break;
      case IRGB:
	this->Expand();
      case ITRUE:
	MagickSetFormat(wand, "rgb");
	size = this->Width() * this->Height() * 3;
	MagickSetDepth(wand, 8);
	break;
    }
    MagickSetSize(wand, this->Width(), this->Height());
    if(!MagickReadImageBlob(wand, this->Data(), size)) {
	magick_err(wand);
	return -1;
    }
    if(this->Type() == IBITMAP)
	MagickNegateImage(wand, MagickTrue);
    /* Allow format to change based on file name */
    MagickSetFormat(wand, NULL);

    MagickSetImageFormat(wand, this->SaveFormatString());
    MagickSetImageCompressionQuality(wand, this->saveQuality);
    if(file)
	ret = MagickWriteImageFile(wand, file);
    else {
	const char *ext = strrchr(filename, '.');
	if(ext)
	    MagickSetImageFormat(wand, ext);
	ret = MagickWriteImage(wand, filename);
    }
    if(!ret) {
	magick_err(wand);
	return -1;
    }
    DestroyMagickWand(wand);
    return 0;
}


int imageio::Load(const char *fname, FILE *f)
{
    MagickBooleanType ret;
    MagickWand *wand = NewMagickWand();

    if(!wand)
	return -1;
    if(f) {
	ret = MagickReadImageFile(wand, f);
    } else {
	ret = MagickReadImage(wand, fname);
    }
    if(!ret) {
	magick_err(wand);
	return -1;
    }
    this->newTrueImage(MagickGetImageWidth(wand), MagickGetImageHeight(wand));
    /* there's no "WriteImageBlob", so use pixel iterators */
    MagickSetImageDepth(wand, 8);  // ensure quantum is 8-bit, I hope
    PixelIterator *iter = NewPixelIterator(wand);
    PixelWand **pix;
    size_t nw;
    byte *dest = this->Data();
    while((pix = PixelGetNextIteratorRow(iter, &nw))) {
	size_t i;
	for(i = 0; i < nw; i++) {
	    *dest++ = PixelGetRedQuantum(pix[i]);
	    *dest++ = PixelGetGreenQuantum(pix[i]);
	    *dest++ = PixelGetBlueQuantum(pix[i]);
	}
    }
    /* apparently, destroying the wand doesn't free the pixel iterator */
    DestroyPixelIterator(iter);
    DestroyMagickWand(wand);
    return 0;
}


long imageio::Read( FILE  *file, long  id )
{
    if(this->Load( NULL, file) == 0)
	return(dataobject::NOREADERROR);
    else
	return(dataobject::BADFORMAT);
}

int  imageio::Ident( const char  *fullname )
{
    /* test read of file - expensive */
    MagickBooleanType ret;
    MagickWand *wand = NewMagickWand();

    if(!wand)
	return FALSE;
    ret = MagickReadImage(wand, fullname);
    DestroyMagickWand(wand);
    return(ret);
}

imageio::imageio( )
{
    ATKinit;
    this->saveQuality = 75;
    THROWONFAILURE((TRUE));
}
