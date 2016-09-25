/* Generic Image I/O (i.e., supports many file formats) */
/* DevIL (http://openil.sourceforge.net/) implementation */
/* tested with 1.7.8. */
/* skeleton of sorts copied from jpeg.C.  Bite me, IBM.  */
#include <andrewos.h>
ATK_IMPL("imageio.H")
#include <image.H>
#include <imageio.H>
#include <IL/il.h>
#include <IL/ilu.h>

static boolean imageio_init()
{
    ilInit();
    iluInit();
    // ilutRenderer(ILUT_OPENGL);
    ilEnable(IL_ORIGIN_SET);
    ilOriginFunc(IL_ORIGIN_UPPER_LEFT);
    return TRUE;
}

ATKdefineRegistry(imageio, image, imageio_init);

static void print_il_error(bool print)
{
    ILenum err;
    while((err = ilGetError()) != IL_NO_ERROR)
	if(print)
	    fprintf(stdout, "devil error: %s\n", iluErrorString(err));
}

long imageio::WriteNative(FILE  *file, const char  *filename)
{
    int bpp = 0, fmt = 0;
    ILuint image;
    bool ret;

    ilGenImages(1, &image);
    ilBindImage(image);
    switch(this->Type()) {
      case IBITMAP:
	this->Bit2Grey(); /* il doesn't do sub-byte pixel widths */
      case IGREYSCALE:
	bpp = 1;
	fmt = IL_LUMINANCE;
	break;
      case IRGB:
	// fmt = IL_COLOUR_INDEX;  /* not sure how to pass cmap in */
	// bpp = 1;
	// break;
	this->Expand();
      case ITRUE:
	fmt = IL_RGB;
	bpp = 3;
	break;
    }
    ilTexImage(this->Width(), this->Height(), 1, bpp, fmt, IL_UNSIGNED_BYTE, this->Data());
    // passing in Data() directly makes image upside-down
    // as does using ilSetixels
    // but calling this for some reason fails
    iluFlipImage();
    ilSetInteger(IL_JPG_QUALITY, this->saveQuality);
    if(file)
	ret = ilSaveF(type, file);
    else {
	remove(filename);
	ret = ilSaveImage(filename);
    }
    if(!ret) {
	print_il_error(1);
	ilDeleteImages(1, &image);
	return -1;
    }
    ilDeleteImages(1, &image);
    return 0;
}

int imageio::Load(const char *fname, FILE *f)
{
    int ret;
    int w, h, fmt;
    ILuint image;

    ilGenImages(1, &image);
    ilBindImage(image);
    if(f)
	ret = ilLoadF(IL_TYPE_UNKNOWN, f);
    else
	ret = ilLoadImage(fname);
    if(!ret) {
	print_il_error(1);
	ilDeleteImages(1, &image);
	return -1;
    }
    w = ilGetInteger(IL_IMAGE_WIDTH);
    h = ilGetInteger(IL_IMAGE_HEIGHT);
    fmt = ilGetInteger(IL_IMAGE_FORMAT);
    if(fmt == IL_LUMINANCE) {
	this->newGreyImage( w, h, 8);
	ilCopyPixels(0, 0, 0, w, h, 1, IL_RGB, IL_UNSIGNED_BYTE, this->Data());
    } else {
	this->newTrueImage( w, h);
	ilCopyPixels(0, 0, 0, w, h, 1, IL_RGB, IL_UNSIGNED_BYTE, this->Data());
    }
    ilDeleteImages(1, &image);
    return 0;
}


long imageio::Read( FILE  *file, long  id )
{
    if(this->Load( NULL, file) == 0)
	return(dataobject_NOREADERROR);
    else
	return(dataobject_BADFORMAT);
}

int  imageio::Ident( const char  *fullname )
{
    /* There is no ident function, so just try loading (expensive!) */
    ILuint image;
    ilGenImages(1, &image);
    int ret = ilLoadImage(fullname);
    ilDeleteImages(1, &image);
    print_il_error(0);
    return ret;
}

imageio::imageio( )
{
    ATKinit;
    this->saveQuality = 75;
    THROWONFAILURE((TRUE));
}
