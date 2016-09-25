/* Generic Image I/O (i.e., supports many file formats) */
/* FreeImage (http://freeimage.sourceforge.net/) implementation */
/* tested with 3.15.4. */
/* skeleton of sorts copied from jpeg.C.  Bite me, IBM.  */
#include <andrewos.h>
ATK_IMPL("freeimage.H")
#include <image.H>
#include <imageio.H>
#include <FreeImage.h>

/* ugh - this is technically an internal thing, but why do I have to duplicate it here? */
extern void SetDefaultIO(FreeImageIO *io);

static void errhandler(FREE_IMAGE_FORMAT fif, const char *message)
{
    const char *fmt = FreeImage_GetFormatFromFIF(fif);
    if(!fmt)
	fmt = "";
    fprintf(stdout, "FreeImage %s error: %s\n", fmt, message);
}

static boolean freeimage_init()
{
    /* may already done by .so _init */
    FreeImage_Initialise(FALSE);
    fprintf(stdout, "%s\n", FreeImage_GetCopyrightMessage());
    FreeImage_SetOutputMessage(errhandler);
    return TRUE;
}

ATKdefineRegistry(imageio, image, freeimage_init);

static void rb_swap(byte *data, unsigned long len)
{
    while(len > 0) {
	data[0] ^= data[2];
	data[2] ^= data[0];
	data[0] ^= data[2];
	data += 3;
	--len;
    }
}

long imageio::WriteNative(FILE  *file, const char  *filename)
{
    FIBITMAP *bmp;
    unsigned int w = this->Width(), h = this->Height();
    int opt;
    int ret;
    FREE_IMAGE_FORMAT fif;

    if(!filename || (fif = FreeImage_GetFIFFromFilename(filename)) == FIF_UNKNOWN) {
	const char *deftype = this->SaveFormatString();
	char fname[strlen(deftype) + 3];
	sprintf(fname, "x.%s", deftype);
	fif = FreeImage_GetFIFFromFilename(fname);
	if(fif == FIF_UNKNOWN)
	    return -1;
    }

    /* convert to format Image can handle? */
    switch(this->Type()) {
      case IBITMAP:
	bmp = FreeImage_ConvertFromRawBits(this->Data(), w, h, (w + 7) / 8, 1,
					   1, 1, 1, TRUE);
	break;
      case IGREYSCALE:
	bmp = FreeImage_ConvertFromRawBits(this->Data(), w, h, w, 8,
					   0xff, 0xff, 0xff, TRUE);
	break;
      case IRGB:
	this->Expand();
      case ITRUE:
	/* piece of crap ignores the RGB mask and just uses internal BGR */
	rb_swap(this->Data(), h * w);
	bmp = FreeImage_ConvertFromRawBits(this->Data(), w, h, w * 3, 24,
					   0xff0000, 0xff00, 0xff, TRUE);
	break;
      default:
	return -1;
    }
    if(!bmp)
	return -1;
    if(this->saveQuality > 75)
	opt = JPEG_QUALITYSUPERB;
    else if(this->saveQuality > 50)
	opt = JPEG_QUALITYGOOD;
    else if(this->saveQuality > 25)
	opt = JPEG_QUALITYNORMAL;
    else if(this->saveQuality > 10)
	opt = JPEG_QUALITYAVERAGE;
    else
	opt = JPEG_QUALITYBAD;
    if(file) {
	FreeImageIO io;
	SetDefaultIO(&io);
	ret = FreeImage_SaveToHandle(fif, bmp, &io, file, opt);
    } else {
	ret = FreeImage_Save(fif, bmp, filename, opt);
    }
    FreeImage_Unload(bmp);
    return -!ret;
}


int imageio::Load(const char *fname, FILE *f)
{
    FIBITMAP *bmp, *tbmp;
    unsigned int w, h;

    if(f) {
	FreeImageIO io;
	SetDefaultIO(&io);
	FREE_IMAGE_FORMAT fif = FreeImage_GetFileTypeFromHandle(&io, f);
	bmp = FreeImage_LoadFromHandle(fif, &io, f);
    } else {
	FREE_IMAGE_FORMAT fif = FreeImage_GetFileType(fname);
	bmp = FreeImage_Load(fif, fname);
    }
    if(!bmp)
	return -1;
    w = FreeImage_GetWidth(bmp);
    h = FreeImage_GetHeight(bmp);
    FreeImage_SetTransparent(bmp, FALSE); /* not supported by ATK */
    switch(FreeImage_GetColorType(bmp)) {
      case FIC_MINISWHITE: /* grayscale, inverted */
	tbmp = FreeImage_ConvertToGreyscale(bmp);
	FreeImage_Unload(bmp);
	if(!(bmp = tbmp))
	    return -1;
	/* fall through */
      case FIC_MINISBLACK: /* grayscale */
	if(FreeImage_GetBPP(bmp) == 1) {
	    this->newBitImage(w, h);
	    FreeImage_ConvertToRawBits(this->Data(), bmp, (w + 7)/8, 1,
				       1, 1, 1, TRUE);
	    break;
	}
	/* maybe atk supports >8 bits, but I'm not willing to bet */
	if(FreeImage_GetBPP(bmp) != 8) {
	    tbmp = FreeImage_ConvertToGreyscale(bmp);
	    FreeImage_Unload(bmp);
	    if(!(bmp = tbmp))
		return -1;
	}
	this->newGreyImage(w, h, 8);
	FreeImage_ConvertToRawBits(this->Data(), bmp, w, 8,
				   0xff, 0xff, 0xff, TRUE);
	break;
      case FIC_CMYK:
	/* Ugh - yet another internal (and internally unused!) function */
	extern BOOL ConvertCMYKtoRGBA(FIBITMAP *);
	ConvertCMYKtoRGBA(bmp);
      case FIC_PALETTE: /* RGB w/ palette */
	/* automatically converted, if 8-bit palette */
      case FIC_RGB:
      case FIC_RGBALPHA: /* shouldn't happen */
	this->newTrueImage(w, h);
	/* A hidden feature of ConvertToRawBits is that it does conversions */
	/* however, the conversions it does are simplistic and wrong sometimes */
	FreeImage_ConvertToRawBits(this->Data(), bmp, w * 3, 24,
				   0xff0000, 0xff00, 0xff, TRUE);
	/* piece of crap ignores the RGB mask and just uses internal BGR */
	rb_swap(this->Data(), h * w);
	break;
    }
    FreeImage_Unload(bmp);
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
    /* test load - expensive */
    /* only other way may be to just return TRUE if fif != UNKNOWN */
    FREE_IMAGE_FORMAT fif = FreeImage_GetFileType(fullname);
    FIBITMAP *bmp = FreeImage_Load(fif, fullname);
    if(!bmp)
	return FALSE;
    FreeImage_Unload(bmp);
    return TRUE;
}

imageio::imageio()
{
    ATKinit;
    this->saveQuality = 75;
    THROWONFAILURE((TRUE));
}
