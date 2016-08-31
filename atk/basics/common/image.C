/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/config/COPYRITE'     *
\* ********************************************************************** */

/*
	$Disclaimer: 
// Permission to use, copy, modify, and distribute this software and its 
// documentation for any purpose and without fee is hereby granted, provided 
// that the above copyright notice appear in all copies and that both that 
// copyright notice and this permission notice appear in supporting 
// documentation, and that the name of IBM not be used in advertising or 
// publicity pertaining to distribution of the software without specific, 
// written prior permission. 
//                         
// THE COPYRIGHT HOLDERS DISCLAIM ALL WARRANTIES WITH REGARD 
// TO THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF 
// MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL ANY COPYRIGHT 
// HOLDER BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL 
// DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, 
// DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE 
// OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION 
// WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
// 
//  $
*/

/*
 * Copyright 1989, 1990, 1991 Jim Frost
 *
 * Permission to use, copy, modify, distribute, and sell this software
 * and its documentation for any purpose is hereby granted without fee,
 * provided that the above copyright notice appear in all copies and
 * that both that copyright notice and this permission notice appear
 * in supporting documentation.  The author makes no representations
 * about the suitability of this software for any purpose.  It is
 * provided "as is" without express or implied warranty.
 *
 * THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN
 * NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS
 * OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE
 * OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE
 * USE OR PERFORMANCE OF THIS SOFTWARE.
 */
/* ppmquant.c - quantize the colors in a pixmap down to a specified number
**
** Copyright (C) 1989, 1991 by Jef Poskanzer.
**
** Permission to use, copy, modify, and distribute this software and its
** documentation for any purpose and without fee is hereby granted, provided
** that the above copyright notice appear in all copies and that both that
** copyright notice and this permission notice appear in supporting
** documentation.  This software is provided "as is" without express or
** implied warranty.
*/

/* image.c
 */
#include <andrewos.h>

ATK_IMPL("image.H")

#include <ctype.h>
#include <netinet/in.h>	/* Get the definition for ntohl. */

/* for maxpathlen */
#include <sys/param.h>
	
#include <attribs.h>
#include <environ.H>
#include <jpeg.H>
#include <gif.H>
#include <image.H>
#include <util.h>
#include <math.h>
#include <ahash.H>

#define MAXFILELINE 255
#define DEFAULT_SAVE_QUALITY (75)

static const char * const imageTypeLabels[NUM_IMAGE_TYPES] = {
    "IBITMAP",
    "IGREYSCALE",
    "IRGB",
    "ITRUE"
};

static const unsigned short RedIntensity[256]= {
      0,    76,   153,   230,   307,   384,   460,   537,
    614,   691,   768,   844,   921,   998,  1075,  1152,
   1228,  1305,  1382,  1459,  1536,  1612,  1689,  1766,
   1843,  1920,  1996,  2073,  2150,  2227,  2304,  2380,
   2457,  2534,  2611,  2688,  2764,  2841,  2918,  2995,
   3072,  3148,  3225,  3302,  3379,  3456,  3532,  3609,
   3686,  3763,  3840,  3916,  3993,  4070,  4147,  4224,
   4300,  4377,  4454,  4531,  4608,  4684,  4761,  4838,
   4915,  4992,  5068,  5145,  5222,  5299,  5376,  5452,
   5529,  5606,  5683,  5760,  5836,  5913,  5990,  6067,
   6144,  6220,  6297,  6374,  6451,  6528,  6604,  6681,
   6758,  6835,  6912,  6988,  7065,  7142,  7219,  7296,
   7372,  7449,  7526,  7603,  7680,  7756,  7833,  7910,
   7987,  8064,  8140,  8217,  8294,  8371,  8448,  8524,
   8601,  8678,  8755,  8832,  8908,  8985,  9062,  9139,
   9216,  9292,  9369,  9446,  9523,  9600,  9676,  9753,
   9830,  9907,  9984, 10060, 10137, 10214, 10291, 10368,
  10444, 10521, 10598, 10675, 10752, 10828, 10905, 10982,
  11059, 11136, 11212, 11289, 11366, 11443, 11520, 11596,
  11673, 11750, 11827, 11904, 11980, 12057, 12134, 12211,
  12288, 12364, 12441, 12518, 12595, 12672, 12748, 12825,
  12902, 12979, 13056, 13132, 13209, 13286, 13363, 13440,
  13516, 13593, 13670, 13747, 13824, 13900, 13977, 14054,
  14131, 14208, 14284, 14361, 14438, 14515, 14592, 14668,
  14745, 14822, 14899, 14976, 15052, 15129, 15206, 15283,
  15360, 15436, 15513, 15590, 15667, 15744, 15820, 15897,
  15974, 16051, 16128, 16204, 16281, 16358, 16435, 16512,
  16588, 16665, 16742, 16819, 16896, 16972, 17049, 17126,
  17203, 17280, 17356, 17433, 17510, 17587, 17664, 17740,
  17817, 17894, 17971, 18048, 18124, 18201, 18278, 18355,
  18432, 18508, 18585, 18662, 18739, 18816, 18892, 18969,
  19046, 19123, 19200, 19276, 19353, 19430, 19507, 19584
};

static const unsigned short GreenIntensity[256]= {
     0,  151,  302,  453,  604,  755,  906, 1057,
  1208, 1359, 1510, 1661, 1812, 1963, 2114, 2265,
  2416, 2567, 2718, 2869, 3020, 3171, 3322, 3473,
  3624, 3776, 3927, 4078, 4229, 4380, 4531, 4682,
  4833, 4984, 5135, 5286, 5437, 5588, 5739, 5890,
  6041, 6192, 6343, 6494, 6645, 6796, 6947, 7098,
  7249, 7400, 7552, 7703, 7854, 8005, 8156, 8307,
  8458, 8609, 8760, 8911, 9062, 9213, 9364, 9515,
  9666, 9817, 9968,10119,10270,10421,10572,10723,
 10874,11025,11176,11328,11479,11630,11781,11932,
 12083,12234,12385,12536,12687,12838,12989,13140,
 13291,13442,13593,13744,13895,14046,14197,14348,
 14499,14650,14801,14952,15104,15255,15406,15557,
 15708,15859,16010,16161,16312,16463,16614,16765,
 16916,17067,17218,17369,17520,17671,17822,17973,
 18124,18275,18426,18577,18728,18880,19031,19182,
 19333,19484,19635,19786,19937,20088,20239,20390,
 20541,20692,20843,20994,21145,21296,21447,21598,
 21749,21900,22051,22202,22353,22504,22656,22807,
 22958,23109,23260,23411,23562,23713,23864,24015,
 24166,24317,24468,24619,24770,24921,25072,25223,
 25374,25525,25676,25827,25978,26129,26280,26432,
 26583,26734,26885,27036,27187,27338,27489,27640,
 27791,27942,28093,28244,28395,28546,28697,28848,
 28999,29150,29301,29452,29603,29754,29905,30056,
 30208,30359,30510,30661,30812,30963,31114,31265,
 31416,31567,31718,31869,32020,32171,32322,32473,
 32624,32775,32926,33077,33228,33379,33530,33681,
 33832,33984,34135,34286,34437,34588,34739,34890,
 35041,35192,35343,35494,35645,35796,35947,36098,
 36249,36400,36551,36702,36853,37004,37155,37306,
 37457,37608,37760,37911,38062,38213,38364,38515
};

static const unsigned short BlueIntensity[256]= {
     0,   28,   56,   84,  112,  140,  168,  197,
   225,  253,  281,  309,  337,  366,  394,  422,
   450,  478,  506,  535,  563,  591,  619,  647,
   675,  704,  732,  760,  788,  816,  844,  872,
   901,  929,  957,  985, 1013, 1041, 1070, 1098,
  1126, 1154, 1182, 1210, 1239, 1267, 1295, 1323,
  1351, 1379, 1408, 1436, 1464, 1492, 1520, 1548,
  1576, 1605, 1633, 1661, 1689, 1717, 1745, 1774,
  1802, 1830, 1858, 1886, 1914, 1943, 1971, 1999,
  2027, 2055, 2083, 2112, 2140, 2168, 2196, 2224,
  2252, 2280, 2309, 2337, 2365, 2393, 2421, 2449,
  2478, 2506, 2534, 2562, 2590, 2618, 2647, 2675,
  2703, 2731, 2759, 2787, 2816, 2844, 2872, 2900,
  2928, 2956, 2984, 3013, 3041, 3069, 3097, 3125,
  3153, 3182, 3210, 3238, 3266, 3294, 3322, 3351,
  3379, 3407, 3435, 3463, 3491, 3520, 3548, 3576,
  3604, 3632, 3660, 3688, 3717, 3745, 3773, 3801,
  3829, 3857, 3886, 3914, 3942, 3970, 3998, 4026,
  4055, 4083, 4111, 4139, 4167, 4195, 4224, 4252,
  4280, 4308, 4336, 4364, 4392, 4421, 4449, 4477,
  4505, 4533, 4561, 4590, 4618, 4646, 4674, 4702,
  4730, 4759, 4787, 4815, 4843, 4871, 4899, 4928,
  4956, 4984, 5012, 5040, 5068, 5096, 5125, 5153,
  5181, 5209, 5237, 5265, 5294, 5322, 5350, 5378,
  5406, 5434, 5463, 5491, 5519, 5547, 5575, 5603,
  5632, 5660, 5688, 5716, 5744, 5772, 5800, 5829,
  5857, 5885, 5913, 5941, 5969, 5998, 6026, 6054,
  6082, 6110, 6138, 6167, 6195, 6223, 6251, 6279,
  6307, 6336, 6364, 6392, 6420, 6448, 6476, 6504,
  6533, 6561, 6589, 6617, 6645, 6673, 6702, 6730,
  6758, 6786, 6814, 6842, 6871, 6899, 6927, 6955,
  6983, 7011, 7040, 7068, 7096, 7124, 7152, 7180
};

/* 4x4 arrays used for dithering, arranged by nybble
 */

#define GRAYS    17 /* ((4 * 4) + 1) patterns for a good dither */
#define GRAYSTEP ((unsigned long)(65536 / GRAYS))

static byte DitherBits[GRAYS][4] = {
  0xf, 0xf, 0xf, 0xf,
  0xe, 0xf, 0xf, 0xf,
  0xe, 0xf, 0xb, 0xf,
  0xa, 0xf, 0xb, 0xf,
  0xa, 0xf, 0xa, 0xf,
  0xa, 0xd, 0xa, 0xf,
  0xa, 0xd, 0xa, 0x7,
  0xa, 0x5, 0xa, 0x7,
  0xa, 0x5, 0xa, 0x5,
  0x8, 0x5, 0xa, 0x5,
  0x8, 0x5, 0x2, 0x5,
  0x0, 0x5, 0x2, 0x5,
  0x0, 0x5, 0x0, 0x5,
  0x0, 0x4, 0x0, 0x5,
  0x0, 0x4, 0x0, 0x1,
  0x0, 0x0, 0x0, 0x1,
  0x0, 0x0, 0x0, 0x0
};

static unsigned long DepthToColorsTable[] = {
  /*  0 */ 1,
  /*  1 */ 2,
  /*  2 */ 4,
  /*  3 */ 8,
  /*  4 */ 16,
  /*  5 */ 32,
  /*  6 */ 64,
  /*  7 */ 128,
  /*  8 */ 256,
  /*  9 */ 512,
  /* 10 */ 1024,
  /* 11 */ 2048,
  /* 12 */ 4096,
  /* 13 */ 8192,
  /* 14 */ 16384,
  /* 15 */ 32768,
  /* 16 */ 65536,
  /* 17 */ 131072,
  /* 18 */ 262144,
  /* 19 */ 524288,
  /* 20 */ 1048576,
  /* 21 */ 2097152,
  /* 22 */ 4194304,
  /* 23 */ 8388608,
  /* 24 */ 16777216,
  /* 25 */ 33554432,
  /* 26 */ 67108864,
  /* 27 */ 134217728,
  /* 28 */ 268435456,
  /* 29 */ 536870912,
  /* 30 */ 1073741824,
  /* 31 */ 2147483648,
  /* 32 */ 2147483648 /* bigger than unsigned int; this is good enough */
};

#define DEPTHTOCOLORS(n) DepthToColorsTable[((n) < 32 ? (n) : 32)]


ATKdefineRegistry(image, dataobject, image::InitializeClass);

static long WriteImageToTempFile( class image  *self, FILE  *file );
static unsigned long  colorsToDepth(unsigned long  ncolors);
static void  newRGBMapData( RGBMap  *rgb, unsigned int size );
static  void make_gamma( double  gamma, int  gammamap[256] );
static void  setupNormalizationArray( unsigned int min , unsigned int max, byte  *array );
static unsigned int * buildZoomIndex( unsigned int width, unsigned int zoom, unsigned int *rwidth );
static int sortRGB(unsigned short  *p1 , unsigned short  *p2);
static int sortRBG(unsigned short  *p1 , unsigned short  *p2);
static int sortGRB(unsigned short  *p1 , unsigned short  *p2);
static int sortGBR(unsigned short  *p1 , unsigned short  *p2);
static int sortBRG(unsigned short  *p1 , unsigned short  *p2);
static int sortBGR(unsigned short  *p1 , unsigned short  *p2);
static  void insertColorArea(unsigned long  *pixel_counts, struct color_area  **rlargest , struct color_area  **rsmallest , struct color_area  *area);
static unsigned int  tone_scale_adjust(unsigned int val);
static void  LeftToRight(int  *curr, int  *next, int   width);
static void  RightToLeft(int  *curr, int  *next, int   width);


boolean
image::InitializeClass( )
{
  return(TRUE);
}

image::image( )
{
    ATKinit;
    const char *saveformat;

    (this)->Type() = 0;
    if(!(this->rgb = (RGBMap *) calloc(1, sizeof(RGBMap))) )
	THROWONFAILURE((FALSE));
    (this)->Width() = 0;
    (this)->Height() = 0;
    (this)->Depth() = 0;
    (this)->Pixlen() = 0;
    (this)->Data() = NULL;
    this->inited = FALSE;
    this->jpegSaveQuality = environ::GetProfileInt("imagesavequality", DEFAULT_SAVE_QUALITY);
    this->origData = NULL;
    this->origDataSize = 0;
    this->lastModified = (this)->GetModified();
    if(!(saveformat = environ::GetProfile("imagesaveformat")))
	saveformat = "gif";
    this->saveformatstring = (char *) malloc(strlen(saveformat) + 1);
    strcpy(this->saveformatstring, saveformat);
    THROWONFAILURE((TRUE));
}

void
image::Duplicate( class image  *target )
{ unsigned int i;
  int size = 0;

    (target)->Type() = (this)->Type();
    switch((this)->Type()) {
	case IRGB:
	case IGREYSCALE:
	    size = ((this)->Width() * (this)->Height()) * pixlen;
	    break;
	case ITRUE:
	    size = ((this)->Width() * (this)->Height() * 3);
	    break;
	case IBITMAP:
	    size = ((((this)->Width() / 8) + ((this)->Width() % 8 ? 1 : 0)) * (this)->Height());
	    break;
    }

    if (!TRUEP(target))
	(target)->newRGBMapData( (this)->RGBSize()); 
    for(i = 0; i < (this)->RGBUsed(); i++) {
	(target)->RedPixel( i) = (this)->RedPixel( i);
	(target)->GreenPixel( i) = (this)->GreenPixel( i);
	(target)->BluePixel( i) = (this)->BluePixel( i);
      }
    (target)->RGBUsed() = (this)->RGBUsed();
    target->rgb->compressed = this->rgb->compressed;

    (target)->Width() = (this)->Width();
    (target)->Height() = (this)->Height();
    (target)->Pixlen() = (this)->Pixlen();
    (target)->Depth() = (this)->Depth();
    target->jpegSaveQuality = (this)->GetJPEGSaveQuality();

    if((target)->Data())
	free((target)->Data());
    if(size>0) (target)->Data() = (unsigned char*) malloc(size);
    else (target)->Data()=NULL;

    if((target)->Data()!=NULL && (this)->Data()==NULL) memset((target)->Data(), 0, size);
    if((this)->Data() && (target)->Data()) memmove((target)->Data(), (this)->Data(), size);
    target->inited = FALSE;
    target->lastModified = this->lastModified;
}

image::~image( )
{
    ATKinit;

    (this)->freeImageData();
    if(rgb) free(rgb);
    if(this->origData) {
	free(this->origData);
	this->origData = NULL;
	this->origDataSize = 0;
    }
    if(this->saveformatstring)
	free(this->saveformatstring);
}

long
image::GetBeginData(FILE  *file, long  id)
{
    int tc;
    if(file == NULL) 
	return(dataobject_PREMATUREEOF);
    ungetc(tc = getc(file), file);
    if(tc == '\\') {
	long discardid;
	if (fscanf(file, "\\begindata{image,%ld", &discardid) != 1
	    || getc(file) != '}' || getc(file) != '\n') {
	    printf("NotATKDatastream\n");
	    return(dataobject_NOTATKDATASTREAM);
    }
    }
    return(dataobject_NOREADERROR);
}

long
image::GetImageData(FILE  *file)
{
    char format[64];
    FILE *tmpFile1, *tmpFile2; 
    char buf[BUFSIZ];

    *format = (char)0;
    if(fscanf(file, "format: %s", format) == 0) { /* empty image? Better be! */
	while(fgets(buf, sizeof(buf), file))
	    if(!strncmp(buf, "\\enddata", 8)) break;
	return(dataobject_NOREADERROR);
    }

    if(*format == (char)0) {
	fprintf(stderr, "image: No save-format found in image data\n");
	return(dataobject_BADFORMAT);
    }
    else {
	if(this->saveformatstring)
	    free(this->saveformatstring);
	this->saveformatstring = (char *) malloc(strlen(format) + 1);
	strcpy(this->saveformatstring, format);
    }

    /* open temp files */
    if( (tmpFile1 = tmpfile()) == NULL || 
       (tmpFile2 = tmpfile()) == NULL ) { /* Error */	
	if(tmpFile1)
	    fclose(tmpFile1);
	perror("image: couldn't open temporary file for writing.");
	return(dataobject_OBJECTCREATIONFAILED);
    }

    /* Write image bytes to tmpFile1 */
    while(fgets(buf, sizeof(buf), file)) {
	if(!strncmp(buf, "\\enddata", 8))
	    break;
	if(fputs(buf, tmpFile1) < 0) {
	    perror("image: error writing out image data");
	    fclose(tmpFile1);
	    fclose(tmpFile2);
	    return(dataobject_OBJECTCREATIONFAILED);
	}
    }
    if(fflush(tmpFile1) || fseek(tmpFile1, 0L, SEEK_SET) < 0) {
	perror("image: error writing out image data");
	fclose(tmpFile1);
	fclose(tmpFile2);
	return(dataobject_OBJECTCREATIONFAILED);
    }

    /* decode base64 data into temp2 */
    from64(tmpFile1, tmpFile2);
    fclose(tmpFile1);
    if(fflush(tmpFile2) || fseek(tmpFile2, 0L, SEEK_SET) < 0) {
	perror("image: error decoding image data");
	fclose(tmpFile2);
	return(dataobject_OBJECTCREATIONFAILED);
    }

    /* decompress/load file */
    {
	class image *savedimage;
	if(!(savedimage = (class image *) ATK::NewObject(format))) {
	    fclose(tmpFile2);
	    fprintf(stderr, "image: couldn't get new object of type %s.\n", format);
	    return(-1);
	}
	(savedimage)->Type() = (this)->Type();
	if((savedimage)->Load( NULL, tmpFile2) < 0) {
	    fclose(tmpFile2);
	    (savedimage)->Destroy();
	    fprintf(stderr, "image: couldn't load image of type %s.\n", format);
	    return(-1);
	}

	/* reset self and copy in data */
	(this)->Reset();
	(savedimage)->Duplicate( this);
	(savedimage)->Destroy();
    }
    fclose(tmpFile2);
    return(dataobject_NOREADERROR);
}

long
image::GetEndData(FILE  *file, long  id)
{
/* This is a noop because GetImageData can deal with or without the enddata */
    return(dataobject_NOREADERROR);
}

static long
WriteImageToTempFile( class image  *self, FILE  *file )
{
    char buf[BUFSIZ];
    FILE *f;
    int size = 0;
    long retval= dataobject_NOREADERROR;
    

    if(self->origData) {
	free(self->origData);
	self->origDataSize = 0;
    }
    if((f = tmpfile())) {
	int savepos = ftell(file), cnt;

	while(fgets(buf, sizeof(buf), file)) {
	    char *tmp;
	    if(!strncmp(buf, "\\enddata", 8)) break;
	    if((tmp =  strchr(buf, '\n')) != NULL) {
		cnt =  (tmp - buf) + 1;
		size += cnt;
		fwrite(buf, sizeof(char), cnt, f);
	    } else {
		/* this is some big nasty binary file -RSK*/
		fclose(f);
		return dataobject_NOTATKDATASTREAM;
	    }
	}
	if((self->origData = (char*) malloc(size))) {
	    if(!fflush(f) && !fseek(f, 0, SEEK_SET)) {
		if((cnt = fread(self->origData, 1, size, f)) != size) {
		    fprintf(stderr, "image: short read on image data.\n");
		    retval= dataobject_PREMATUREEOF; /*RSK*/
		    free(self->origData);
		    self->origData = NULL;
		    self->origDataSize = 0;
		}
		else {
		    self->origDataSize = size;
		}
	    }
	    else {
		fprintf(stderr, "image: couldn't open temp file for writing.\n");
		retval= dataobject_OBJECTCREATIONFAILED; /* not quite true, but close enough -RSK*/
		self->origData = NULL;
		self->origDataSize = 0;
	    }
	}
	fclose(f);
	fseek(file, savepos, SEEK_SET);
    }
    else {
	fprintf(stderr, "image: couldn't open temp file for writing.\n");
	retval= dataobject_OBJECTCREATIONFAILED; /* not quite true, but close enough -RSK*/
	self->origData = NULL;
	self->origDataSize = 0;
    }
    return retval;
}

long
image::Read( FILE  *file, long  id )
{
    long status;

    if((status=WriteImageToTempFile(this, file))==0 &&
       (status = (this)->GetBeginData( file, id)) == 0 &&
       (status = (this)->GetImageData( file)) == 0 &&
       (status = (this)->GetEndData( file, id)) == 0) {
	if(!this->rgb->compressed )
	    (this)->Compress();
	(this)->NotifyObservers( image_NEW);
    }
    return(status);
}

long
image::SendBeginData(FILE  *file, long  writeID, int  level)
{
    long id = (this)->UniqueID();
    this->writeID = writeID;
    fprintf(file, "\\begindata{image,%ld}\n", id);
    if(ferror(file))
	return(-1);
    else
	return(id);
}

long
image::SendImageData(FILE  *file)
{
    if(this->Data()) {

	if((this)->GetModified() > this->lastModified || this->origData == NULL) {
	    class image *savedimage = (class image *) ATK::NewObject(this->saveformatstring);
	    FILE *tmpFile; 

	    if((tmpFile = tmpfile())) {

	/* duplicate self into savedimage and write native format data into tmp file */
		(this)->Duplicate( savedimage);
		if(strcmp(this->saveformatstring, "jpeg") == 0)
		    ((struct jpeg*) savedimage)->SetSaveQuality( (this)->GetJPEGSaveQuality());
		(savedimage)->WriteNative( tmpFile, NULL);
		(savedimage)->Destroy();

	/* open tmp file and encode base64 into file */
		if(fflush(tmpFile) || fseek(tmpFile, 0, SEEK_SET)) {
		    printf("image: failed to open temp file for reading.");
		    fclose(tmpFile);
		    return(-1);
		}
		fprintf(file,"format: %s\n", this->saveformatstring);
		to64(tmpFile, file);
		fclose(tmpFile);
	    }
	    else {
		if(savedimage)
		    (savedimage)->Destroy();
		fprintf(stderr, "image: failed to open temp file for writing.");
		return(-1);
	    }
	}
	else if(this->origData) {
	    fwrite(this->origData, this->origDataSize, sizeof(char), file);
	}
    }
    else { /* important for reading an empty image datastream */
	fprintf(file, "\n");
    }
    this->lastModified = (this)->GetModified();
    return(0);
}    

long
image::SendEndData(FILE  *file, long  writeID, int  id)
{
    (this)->SetWriteID( writeID);
    (this)->SetID( id);
    fprintf(file, "\\enddata{image, %d}\n", id);
    if(ferror(file))
	return(-1);
    return(0);
}

long
image::Write( FILE  *file, long  writeID, int  level )
{   long id = (this)->SendBeginData( file, writeID, level);
    long status = -1;
    if( (id > 0) &&
       (status = (this)->SendImageData( file)) == 0 &&
       (status = (this)->SendEndData( file, writeID, id)) == 0)
	return(id);
    else
	return(status);
}

const char *
image::ViewName( )
{
  return("imagev");
}

void
image::Reset( )
{ 
  (this)->freeImageData();
  (this)->Type() = 0;
  (this)->Width() = 0;
  (this)->Height() = 0;
  (this)->Depth() = 0;
  (this)->Pixlen() = 0;
  this->inited = FALSE;
  this->jpegSaveQuality = environ::GetProfileInt("imagesavequality", DEFAULT_SAVE_QUALITY);
 }

static unsigned long 
colorsToDepth(unsigned long  ncolors)
{ unsigned long a;

  for(a = 0; (a < 32) && (DepthToColorsTable[a] < ncolors); a++);
  return(a);
}

static void 
newRGBMapData( RGBMap  *rgb, unsigned int size )
{ 
  rgb->used = 0;
  rgb->size = size;
  rgb->compressed = 0;
  rgb->red = (Intensity *)malloc(sizeof(Intensity) * size);
  rgb->green = (Intensity *)malloc(sizeof(Intensity) * size);
  rgb->blue = (Intensity *)malloc(sizeof(Intensity) * size);
}

void 
image::newRGBMapData( unsigned int size )
{
    if(this->rgb) {
	freeRGBMapData();
	::newRGBMapData(this->rgb, size);
    }
}

void 
image::freeRGBMapData( )
{
  if(this->rgb) {
      if((this)->RedMap()) {
	  free((byte*) (this)->RedMap());
	  (this)->RedMap() = NULL;
      }
      if((this)->GreenMap()) {
	  free((byte*) (this)->GreenMap());
	  (this)->GreenMap() = NULL;
      }
      if((this)->BlueMap()) {
	  free((byte*) (this)->BlueMap());
	  (this)->BlueMap() = NULL;
      }
      (this)->RGBUsed() = (this)->RGBSize() = 0;
      this->rgb->compressed = 0;
  }
}

void
image::newBitImage( unsigned int width, unsigned int height )
{ unsigned int linelen;

  (this)->Type() = IBITMAP;
  (this)->newRGBMapData( (unsigned int)2);
  (this)->RedPixel( 0) = (this)->GreenPixel( 0) = (this)->BluePixel( 0) = 65535;
  (this)->RedPixel( 1) = (this)->GreenPixel( 1) = (this)->BluePixel( 1) = 0;
  (this)->RGBUsed() = 2;
  (this)->Width() = width;
  (this)->Height() = height;
  (this)->Depth() = 1;
  (this)->Pixlen() = 1;
  linelen = (width / 8) + (width % 8 ? 1 : 0); /* thanx johnh@amcc.com */
  (this)->Data() = (unsigned char *) calloc(linelen, height);
}

void
image::newGreyImage( unsigned int width, unsigned int height, unsigned int depth )
{
  (this)->Type() = IGREYSCALE;
  (this)->newRGBMapData( DEPTHTOCOLORS(depth));
  (this)->Width() = width;
  (this)->Height() = height;
  (this)->Depth() = depth;
  (this)->Pixlen() = 1; /* in bytes */
  (this)->Data() = (unsigned char *) malloc(width * height);
}

void
image::newRGBImage( unsigned int width, unsigned int height, unsigned int depth )
{ unsigned int pixlen, numcolors;

  pixlen = depth / 8 + (depth %	8 ? 1 :	0); /* in bytes */
  if (pixlen == 0) /* special case for `zero' depth image, which is */
    pixlen = 1;     /* sometimes interpreted as `one color' */
  numcolors = DEPTHTOCOLORS(depth);
  (this)->Type() = IRGB;
  (this)->newRGBMapData( numcolors);
  (this)->Width() = width;
  (this)->Height() = height;
  (this)->Depth() = depth;
  (this)->Pixlen() = pixlen;
  (this)->Data() = (unsigned char *) malloc(width * height * pixlen);
}

void
image::newTrueImage( unsigned int width , unsigned int height )
{
  (this)->Type() = ITRUE;
  (this)->RGBUsed() = (this)->RGBSize() = 0;
  (this)->Width() = width;
  (this)->Height() = height;
  (this)->Depth() = 24;
  (this)->Pixlen() = 3;
  (this)->Data() = (unsigned char *) malloc(width * height * 3);
}

void 
image::freeImageData( )
{
  if (!TRUEP(this) && this->rgb) {
    (this)->freeRGBMapData();
  }
  if((this)->Data())
      free((this)->Data());
  (this)->Data() = NULL;
}

/* alter an image's brightness by a given percentage
 */

void 
image::Brighten( unsigned int percent )
{ unsigned int a;
  unsigned int newrgb;
  float        fperc;
  unsigned int size;
  byte        *destptr;

  if (BITMAPP(this)) /* we're AT&T */
    return;

  fperc = (float)percent / 100.0;

  switch ((this)->Type()) {
      case IGREYSCALE:
	  for (a = 0; a < (this)->RGBUsed(); a++) {
	      newrgb = (unsigned int)((this)->RedPixel( a) * fperc);
	      if (newrgb > 65535)
		  newrgb = 65535;
	      (this)->RedPixel( a) = 
		(this)->GreenPixel( a) = 
		(this)->BluePixel( a) = newrgb;
	  }
	  break;
      case IRGB:
	  for (a = 0; a < (this)->RGBUsed(); a++) {
	      newrgb = (unsigned int)((this)->RedPixel( a) * fperc);
	      if (newrgb > 65535)
		  newrgb = 65535;
	      (this)->RedPixel( a) = newrgb;
	      newrgb = (unsigned int)((this)->GreenPixel( a) * fperc);
	      if (newrgb > 65535)
		  newrgb = 65535;
	      (this)->GreenPixel( a) = newrgb;
	      newrgb = (unsigned int)((this)->BluePixel( a) * fperc);
	      if (newrgb > 65535)
		  newrgb = 65535;
	      (this)->BluePixel( a) = newrgb;
	  }
	  break;
      case ITRUE:
	  size = (this)->Width() * (this)->Height() * 3;
	  destptr = (this)->Data();
	  for (a = 0; a < size; a++) {
	      newrgb = (unsigned int)(*destptr * fperc);
	      if (newrgb > 255)
		  newrgb = 255;
	      *(destptr++) = newrgb;
	  }
	  break;
  } 
}

/*****************************************************************
 * TAG( make_gamma )
 * 
 * Makes a gamma compenstation map.
 * Inputs:
 *  gamma:			desired gamma
 * 	gammamap:		gamma mapping array
 * Outputs:
 *  Changes gamma array entries.
 */
static void
make_gamma( double  gamma, int  gammamap[256] )
{   int i;

    for (i = 0; i < 256; i++ ) {
#ifdef BYTEBUG
		int byteb1;
		
		byteb1 = (int)(0.5 + 255 * pow( i / 255.0, 1.0/gamma ));
		gammamap[i] = byteb1;
#else
		gammamap[i] = (int)(0.5 + 255 * pow( i / 255.0, 1.0/gamma ));
#endif
    }
}

void 
image::GammaCorrect( float   disp_gam )
{ unsigned int a;
  int gammamap[256];
  unsigned int size;
  byte *destptr;

  if (BITMAPP(this)) /* we're AT&T */
    return;

  make_gamma(disp_gam,gammamap);

  switch ((this)->Type()) {
      case IGREYSCALE:
	  for (a = 0; a < (this)->RGBUsed(); a++) {
	      (this)->RedPixel( a) = 
		(this)->GreenPixel( a) = 
		(this)->BluePixel( a) = 
		gammamap[(this)->RedPixel( a) >> 8] << 8;
	  }
	  break;
      case IRGB:
	  for (a = 0; a < (this)->RGBUsed(); a++) {
	      (this)->RedPixel( a) = gammamap[(this)->RedPixel(a) >> 8] << 8;
	      (this)->GreenPixel( a) = gammamap[(this)->GreenPixel( a) >> 8] << 8;
	      (this)->BluePixel( a) = gammamap[(this)->BluePixel( a) >> 8] << 8;
	  }
	  break;
      case ITRUE:
	  size = (this)->Width() * (this)->Height() * 3;
	  destptr = (this)->Data();
	  for (a = 0; a < size; a++) {
	      *destptr = gammamap[*destptr];
	      destptr++;
	  }
	  break;
  }
}

/* this initializes a lookup table for doing normalization
 */

static void 
setupNormalizationArray( unsigned int min , unsigned int max, byte  *array )
{ unsigned int a;
  unsigned int new_c;
  float factor;

  factor = 256.0 / (max - min);
  for (a = min; a <= max; a++) {
    new_c = (unsigned int)((float)(a - min) * factor);
    array[a] = (new_c > 255 ? 255 : new_c);
  }
}

/* normalize an image.
 */

class image *
image::Normalize( )
{ unsigned int  a, x, y;
  unsigned int  min, max;
  Pixel         pixval;
  class image *newimage;
  byte         *srcptr, *destptr;
  byte          array[256];

  if (BITMAPP(this))
    return(this);

  switch ((this)->Type()) {
      case IGREYSCALE:
      case IRGB:
	  min = 256;
	  max = 0;
	  for (a = 0; a < (this)->RGBUsed(); a++) {
	      byte red, green, blue;

	      red = (this)->RedPixel( a) >> 8;
	      green = (this)->GreenPixel( a) >> 8;
	      blue = (this)->BluePixel( a) >> 8;
	      if (red < min)
		  min = red;
	      if (red > max)
		  max = red;
	      if (green < min)
		  min = green;
	      if (green > max)
		  max = green;
	      if (blue < min)
		  min = blue;
	      if (blue > max)
		  max = blue;
	  }
	  setupNormalizationArray(min, max, array);

	  (newimage = new image)->newTrueImage( (this)->Width(), (this)->Height());
	  newimage->jpegSaveQuality = (this)->GetJPEGSaveQuality();
	  srcptr = (this)->Data();
	  destptr = (newimage)->Data();
	  for (y = 0; y < (this)->Height(); y++)
	      for (x = 0; x < (this)->Width(); x++) {
		  pixval = memToVal(srcptr, (this)->Pixlen());
		  *destptr = array[(this)->RedPixel( pixval) >> 8];
		  destptr++;
		  *destptr = array[(this)->GreenPixel( pixval) >> 8];
		  destptr++;
		  *destptr = array[(this)->BluePixel( pixval) >> 8];
		  destptr++;
		  srcptr += (this)->Pixlen();
	      }
	  (newimage)->Duplicate( this);
	  (newimage)->Destroy();
	  break;

      case ITRUE:
	  srcptr = (this)->Data();
	  min = 255;
	  max = 0;
	  for	(y = 0; y < (this)->Height(); y++)
	      for (x = 0; x < (this)->Width(); x++) {
		  if (*srcptr < min)
		      min = *srcptr;
		  if (*srcptr > max)
		      max = *srcptr;
		  srcptr++;
		  if (*srcptr < min)
		      min = *srcptr;
		  if (*srcptr > max)
		      max = *srcptr;
		  srcptr++;
		  if (*srcptr < min)
		      min = *srcptr;
		  if (*srcptr > max)
		      max = *srcptr;
		  srcptr++;
	      }
	  setupNormalizationArray(min, max, array);

	  srcptr = (this)->Data();
	  for (y = 0; y < (this)->Height(); y++)
	      for (x = 0; x < (this)->Width(); x++) {
		  *srcptr = array[*srcptr];
		  srcptr++;
		  *srcptr = array[*srcptr];
		  srcptr++;
		  *srcptr = array[*srcptr];
		  srcptr++;
	      }
  }
  return(this);
}

/* convert to grayscale
 */

void 
image::Gray( )
{ unsigned int a;
  unsigned int size;
  Intensity intensity, red, green, blue;
  byte *destptr;

  if (BITMAPP(this) || GREYSCALEP(this))
    return;

  switch ((this)->Type()) {
      case IRGB:
	  for (a = 0; a < (this)->RGBUsed(); a++) {
	      intensity = colorIntensity((this)->RedPixel( a),
					 (this)->GreenPixel( a),
					 (this)->BluePixel( a));
	      (this)->RedPixel( a) = intensity;
	      (this)->GreenPixel( a) = intensity;
	      (this)->BluePixel( a) = intensity;
	  }
	  break;

      case ITRUE:
	  size = (this)->Width() * (this)->Height();
	  destptr = (this)->Data();
	  for (a = 0; a < size; a++) {
	      red = *destptr << 8;
	      green = *(destptr + 1) << 8;
	      blue = *(destptr + 2) << 8;
	      intensity = colorIntensity(red, green, blue) >> 8;
	      *(destptr++) = intensity; /* red */
	      *(destptr++) = intensity; /* green */
	      *(destptr++) = intensity; /* blue */
	  }
	  break;
  }
}

#define TLA_TO_15BIT(TABLE,PIXEL)           \
  ((((TABLE)->red[PIXEL] & 0xf800) >> 1) |   \
   (((TABLE)->green[PIXEL] & 0xf800) >> 6) | \
   (((TABLE)->blue[PIXEL] & 0xf800) >> 11))

/* this converts a 24-bit true color pixel into a 15-bit true color pixel
 */

#define TRUE_TO_15BIT(PIXEL)     \
  ((((PIXEL) & 0xf80000) >> 9) | \
   (((PIXEL) & 0x00f800) >> 6) | \
   (((PIXEL) & 0x0000f8) >> 3))

/* these macros extract color intensities from a 15-bit true color pixel
 */

#define RED_INTENSITY(P)   (((P) & 0x7c00) >> 10)
#define GREEN_INTENSITY(P) (((P) & 0x03e0) >> 5)
#define BLUE_INTENSITY(P)   ((P) & 0x001f)

#define NIL_PIXEL 0xffffffff

struct compressrgb {
    unsigned int pixel;
    unsigned short r, g, b;
};

static int compresssort(const void *a, const void *b) {
    compressrgb *ca=(compressrgb *)a;
    compressrgb *cb=(compressrgb *)b;
    if(ca->r!=cb->r) return ca->r-cb->r;
    else if(ca->g!=cb->g) return ca->g-cb->g;
    else return ca->b-cb->b;
}

static void OldCompress(image *self) {
    Pixel         hash_table[32768];
    Pixel        *pixel_table;
    Pixel        *pixel_map;
    Pixel         index, oldpixval, newpixval;
    byte         *pixptr;
    unsigned int  x, y, badcount, dupcount;
 /* initialize hash table and allocate new RGB intensity tables
 */

    RGBMap       *rgb;
    for (x = 0; x < 32768; x++)
	hash_table[x] = NIL_PIXEL;
    rgb = (RGBMap*) calloc(1, sizeof(RGBMap));
    ::newRGBMapData(rgb, (self)->RGBUsed());
    rgb->size = (self)->RGBUsed();
    rgb->used = 0;
    pixel_table = (Pixel *)malloc(sizeof(Pixel) * (self)->RGBUsed());
    pixel_map = (Pixel *)malloc(sizeof(Pixel) * (self)->RGBUsed());
    for (x = 0; x < (self)->RGBUsed(); x++)
	pixel_map[x] = NIL_PIXEL;

    pixptr = (self)->Data();
    dupcount= badcount= 0;
    for (y = 0; y < (self)->Height(); y++)
	for (x = 0; x < (self)->Width(); x++) {
	    oldpixval = memToVal(pixptr, (self)->Pixlen());
	    if (oldpixval > (self)->RGBUsed()) {
		badcount++;
		oldpixval= 0;
	    }

 /* if we don't already know what value the new pixel will have,
 * look for a similar pixel in hash table.
 */
	    if (pixel_map[oldpixval] == NIL_PIXEL) {
		index = TLA_TO_15BIT(self->rgb, oldpixval);

 /* nothing similar
 */

		if (hash_table[index] == NIL_PIXEL) {
		    newpixval = rgb->used++;
		    hash_table[index] = newpixval;
		}

 /* we've seen one like self before; try to find out if it's an
 * exact match
 */

		else {
		    newpixval = hash_table[index];
		    for (;;) {

 /* if the color is the same as another color we've seen,
 * use the pixel that the other color is using
 */

			if ((rgb->red[newpixval] == (self)->RedPixel( oldpixval)) &&
			    (rgb->green[newpixval] == (self)->GreenPixel( oldpixval)) &&
			    (rgb->blue[newpixval] == (self)->BluePixel( oldpixval))) {
			    pixel_map[oldpixval] = newpixval; /* same color */
			    dupcount++;
			    goto move_pixel;
			}

 /* if we're at the end of the chain, we're the first pixel
 * of self color
 */

			if (pixel_table[newpixval] == NIL_PIXEL) /* end of the chain */
			    break;
			newpixval = pixel_table[newpixval];
		    }
		    pixel_table[newpixval] = rgb->used;
		    newpixval = rgb->used++;
		}
		pixel_map[oldpixval] = newpixval;
		pixel_table[newpixval] = NIL_PIXEL;
		rgb->red[newpixval] = (self)->RedPixel( oldpixval);
		rgb->green[newpixval] = (self)->GreenPixel( oldpixval);
		rgb->blue[newpixval] = (self)->BluePixel( oldpixval);
	    }

 /* change the pixel
 */

	    move_pixel:
	      valToMem(pixel_map[oldpixval], pixptr, (self)->Pixlen());
	    pixptr += (self)->Pixlen();
	}
    free(pixel_table);
    free(pixel_map);

 /* image is converted; now fix up its colormap
 */

    (self)->freeRGBMapData();
    free(self->rgb);
    self->rgb = rgb;
    self->rgb->compressed = 1;

}

void 
image::Compress( ) {
    if (!RGBP(this) || this->rgb->compressed || this->rgb->used<2) /* we're AT&T */
	return;
    if(rgb->used>65536) {
	OldCompress(this);
	return;
    }
    unsigned int oused=rgb->used;
    byte *srcptr = data;
    Pixel pixval;
    unsigned int x, y, nc;
    compressrgb *c=(compressrgb *)malloc(sizeof(struct compressrgb)*rgb->used);
    if(c==NULL) return;
    unsigned int *map=(unsigned int *)malloc(sizeof(unsigned int)*rgb->used);
    if(map==NULL) {
	free(c);
	return;
    }
    char *occ=(char *)malloc(sizeof(char)*rgb->used);
    if(occ==NULL) {
	free(c);
	free(map);
	return;
    }
    memset(occ, 0, rgb->used);
    for (y = 0; y < height; y++)
	for (x = 0; x < width; x++) {
	    pixval = memToVal(srcptr, pixlen);
	    if(pixval<rgb->used) occ[pixval]=1;
	    srcptr += pixlen;
	}
    srcptr = data;
    nc=0;
    for(x=0;x<rgb->used;x++) {
	if(occ[x]) {
	    c[nc].pixel=x;
	    c[nc].r=rgb->red[x];
	    c[nc].g=rgb->green[x];
	    c[nc].b=rgb->blue[x];
	    nc++;
	}
    }
    rgb->used=nc;
    qsort(c, rgb->used, sizeof(struct compressrgb),
	  QSORTFUNC(compresssort));
    nc=(-1);
    for(x=0;x<rgb->used;x++) {
	if(x>0 && c[x].r==c[x-1].r && c[x].g==c[x-1].g && c[x].b==c[x-1].b) {
	} else {
	    nc++;
	    rgb->red[nc]=c[x].r;
	    rgb->green[nc]=c[x].g;
	    rgb->blue[nc]=c[x].b;
	}
	map[c[x].pixel]=nc;
    }
    rgb->used=nc+1;
    for (y = 0; y < height; y++)
	for (x = 0; x < width; x++) {
	    pixval = memToVal(srcptr, pixlen);
	    if(pixval<oused) valToMem(map[pixval], srcptr, pixlen);
	    else valToMem(map[0], srcptr, pixlen);
	    srcptr += pixlen;
	}
    free(c);
    free(map);
    free(occ);
    rgb->compressed = 1;
}

static unsigned int *
buildZoomIndex( unsigned int width, unsigned int zoom, unsigned int *rwidth )
{ float         fzoom;
  unsigned int *index;
  unsigned int  a;

  if (!zoom) {
    fzoom = 100.0;
    *rwidth = width;
  }
  else {
    fzoom = (float)zoom / 100.0;
    *rwidth = (unsigned int)(fzoom * width);
  }
  index = (unsigned int *)malloc(sizeof(unsigned int) * *rwidth);
  for (a = 0; a < *rwidth; a++)
    if (zoom)
      *(index + a) = (unsigned int)((float)a / fzoom);
    else
      *(index + a) = a;
  return(index);
}

class image *
image::Zoom( unsigned int xzoom , unsigned int yzoom )
{ class image *newimage=NULL;
  unsigned int *xindex, *yindex;
  unsigned int  xwidth, ywidth;
  unsigned int  x, y, xsrc, ysrc;
  unsigned int  pixlen;
  unsigned int  srclinelen;
  unsigned int  destlinelen;
  byte         *srcline, *srcptr;
  byte         *destline, *destptr;
  byte          srcmask, destmask, bit;
  Pixel         value;

  if (!xzoom && !yzoom) /* stupid user */
    return(this);

  xindex = buildZoomIndex((this)->Width(), xzoom, &xwidth);
  yindex = buildZoomIndex((this)->Height(), yzoom, &ywidth);

  switch ((this)->Type()) {
      case IBITMAP:
	  newimage = new image;
	  (newimage)->newBitImage( xwidth, ywidth);
	  for (x = 0; x < (this)->RGBUsed(); x++) {
	      (newimage)->RedPixel( x) = (this)->RedPixel( x);
	      (newimage)->GreenPixel( x) = (this)->GreenPixel( x);
	      (newimage)->BluePixel( x) = (this)->BluePixel( x);
	  }
	  (newimage)->RGBUsed() = (this)->RGBUsed();
	  destline = (newimage)->Data();
	  destlinelen = (xwidth / 8) + (xwidth % 8 ? 1 : 0);
	  srcline = (this)->Data();
	  srclinelen = ((this)->Width() / 8) + ((this)->Width() % 8 ? 1 : 0);
	  for (y = 0, ysrc = *(yindex + y); y < ywidth; y++) {
	      while (ysrc != *(yindex + y)) {
		  ysrc++;
		  srcline += srclinelen;
	      }
	      srcptr = srcline;
	      destptr = destline;
	      srcmask = 0x80;
	      destmask = 0x80;
	      bit= srcmask & *srcptr;
	      for (x = 0, xsrc = *(xindex + x); x < xwidth; x++) {
		  if (xsrc != *(xindex + x)) {
		      do {
			  xsrc++;
			  if (!(srcmask >>= 1)) {
			      srcmask = 0x80;
			      srcptr++;
			  }
		      } while (xsrc != *(xindex + x));
		      bit = srcmask & *srcptr;
		  }
		  if (bit)
		      *destptr |= destmask;
		  if (!(destmask >>= 1)) {
		      destmask = 0x80;
		      destptr++;
		  }
	      }
	      destline += destlinelen;
	  }
	  break;
      case IGREYSCALE:
      case IRGB:
	  newimage = new image;
	  (newimage)->newRGBImage( xwidth, ywidth, (this)->Depth());
	  for (x = 0; x < (this)->RGBUsed(); x++) {
	      (newimage)->RedPixel( x) = (this)->RedPixel( x);
	      (newimage)->GreenPixel( x) = (this)->GreenPixel( x);
	      (newimage)->BluePixel( x) = (this)->BluePixel( x);
	  }
	  (newimage)->RGBUsed() = (this)->RGBUsed();
	  /* FALLTHRU */

      case ITRUE:
	  if (newimage==NULL) {
	      newimage = new image;
	      (newimage)->newTrueImage( xwidth, ywidth);
	  }
	  pixlen = (this)->Pixlen();
	  destptr = (newimage)->Data();
	  srcline = (this)->Data();
	  srclinelen = (this)->Width() * pixlen;
	  for (y = 0, ysrc = *(yindex + y); y < ywidth; y++) {
	      while (ysrc != *(yindex + y)) {
		  ysrc++;
		  srcline += srclinelen;
	      }

	      srcptr = srcline;
	      value = memToVal(srcptr, pixlen);
	      for (x = 0, xsrc = *(xindex + x); x < xwidth; x++) {
		  if (xsrc != *(xindex + x)) {
		      do {
			  xsrc++;
			  srcptr += (newimage)->Pixlen();
		      } while (xsrc != *(xindex + x));
		      value = memToVal(srcptr, pixlen);
		  }
		  valToMem(value, destptr, pixlen);
		  destptr += pixlen;
	      }
	  }
	  break;
  }

  free((byte *)xindex);
  free((byte *)yindex);

  newimage->jpegSaveQuality = (this)->GetJPEGSaveQuality();
  (newimage)->SetModified();
  return(newimage);
}

/* this structure defines a color area which is made up of an array of pixel
 * values and a count of the total number of image pixels represented by
 * the area.  color areas are kept in a list sorted by the number of image
 * pixels they represent.
 */

typedef int (*sortfptr)(const void  *p1 , const void *  *p2);
struct color_area {
    unsigned short    *pixels;       /* array of pixel values in this area */
    unsigned short     num_pixels;   /* size of above array */
    sortfptr sort_func; /* predicate func to sort with before
				      * splitting */
    unsigned long      pixel_count;  /* # of image pixels we represent */
    struct color_area *prev, *next;
};

/* predicate functions for qsort */

static int
sortRGB(unsigned short  *p1 , unsigned short  *p2)
{ unsigned int red1, green1, blue1, red2, green2, blue2;

  red1 = RED_INTENSITY(*p1);
  green1 = GREEN_INTENSITY(*p1);
  blue1 = BLUE_INTENSITY(*p1);
  red2 = RED_INTENSITY(*p2);
  green2 = GREEN_INTENSITY(*p2);
  blue2 = BLUE_INTENSITY(*p2);

  if (red1 == red2)
    if (green1 == green2)
      if (blue1 < blue2)
	return(-1);
      else
	return(1);
    else if (green1 < green2)
      return(-1);
    else
      return(1);
  else if (red1 < red2)
    return(-1);
  else
    return(1);
}

static int
sortRBG(unsigned short  *p1 , unsigned short  *p2)
{ unsigned int red1, green1, blue1, red2, green2, blue2;

  red1 = RED_INTENSITY(*p1);
  green1 = GREEN_INTENSITY(*p1);
  blue1 = BLUE_INTENSITY(*p1);
  red2 = RED_INTENSITY(*p2);
  green2 = GREEN_INTENSITY(*p2);
  blue2 = BLUE_INTENSITY(*p2);

  if (red1 == red2)
    if (blue1 == blue2)
      if (green1 < green2)
	return(-1);
      else
	return(1);
    else if (blue1 < blue2)
      return(-1);
    else
      return(1);
  else if (red1 < red2)
    return(-1);
  else
    return(1);
}

static int
sortGRB(unsigned short  *p1 , unsigned short  *p2)
{ unsigned int red1, green1, blue1, red2, green2, blue2;

  red1 = RED_INTENSITY(*p1);
  green1 = GREEN_INTENSITY(*p1);
  blue1 = BLUE_INTENSITY(*p1);
  red2 = RED_INTENSITY(*p2);
  green2 = GREEN_INTENSITY(*p2);
  blue2 = BLUE_INTENSITY(*p2);

  if (green1 == green2)
    if (red1 == red2)
      if (blue1 < blue2)
	return(-1);
      else
	return(1);
    else if (red1 < red2)
      return(-1);
    else
      return(1);
  else if (green1 < green2)
    return(-1);
  else
    return(1);
}

static int
sortGBR(unsigned short  *p1 , unsigned short  *p2)
{ unsigned int red1, green1, blue1, red2, green2, blue2;

  red1 = RED_INTENSITY(*p1);
  green1 = GREEN_INTENSITY(*p1);
  blue1 = BLUE_INTENSITY(*p1);
  red2 = RED_INTENSITY(*p2);
  green2 = GREEN_INTENSITY(*p2);
  blue2 = BLUE_INTENSITY(*p2);

  if (green1 == green2)
    if (blue1 == blue2)
      if (red1 < red2)
	return(-1);
      else
	return(1);
    else if (blue1 < blue2)
      return(-1);
    else
      return(1);
  else if (green1 < green2)
    return(-1);
  else
    return(1);
}

static int
sortBRG(unsigned short  *p1 , unsigned short  *p2)
{ unsigned int red1, green1, blue1, red2, green2, blue2;

  red1 = RED_INTENSITY(*p1);
  green1 = GREEN_INTENSITY(*p1);
  blue1 = BLUE_INTENSITY(*p1);
  red2 = RED_INTENSITY(*p2);
  green2 = GREEN_INTENSITY(*p2);
  blue2 = BLUE_INTENSITY(*p2);

  if (blue1 == blue2)
    if (red1 == red2)
      if (green1 < green2)
	return(-1);
      else
	return(1);
    else if (red1 < red2)
      return(-1);
    else
      return(1);
  else if (blue1 < blue2)
    return(-1);
  else
    return(1);
}

static int
sortBGR(unsigned short  *p1 , unsigned short  *p2)
{ unsigned int red1, green1, blue1, red2, green2, blue2;

  red1 = RED_INTENSITY(*p1);
  green1 = GREEN_INTENSITY(*p1);
  blue1 = BLUE_INTENSITY(*p1);
  red2 = RED_INTENSITY(*p2);
  green2 = GREEN_INTENSITY(*p2);
  blue2 = BLUE_INTENSITY(*p2);

  if (blue1 == blue2)
    if (green1 == green2)
      if (red1 < red2)
	return(-1);
      else
	return(1);
    else if (green1 < green2)
      return(-1);
    else
      return(1);
  else if (blue1 < blue2)
    return(-1);
  else
    return(1);
}

/* this does calculations on a color area following a split and inserts
 * the color area in the list of color areas.
 */

static void
insertColorArea(unsigned long  *pixel_counts, struct color_area  **rlargest , struct color_area  **rsmallest , struct color_area  *area)
{ int a;
  unsigned int red, green, blue;
  unsigned int min_red, min_green, min_blue;
  unsigned int max_red, max_green, max_blue= 0;
  struct color_area *largest, *smallest, *tmp_area;

  min_red = min_green = min_blue= 31;
  max_red = max_green = max_blue= 0;

/* update pixel count for this area and find RGB intensity widths
 */

  area->pixel_count = 0;
  for (a = 0; a < area->num_pixels; a++) {
    area->pixel_count += pixel_counts[area->pixels[a]];
    red = RED_INTENSITY(area->pixels[a]);
    green = GREEN_INTENSITY(area->pixels[a]);
    blue = BLUE_INTENSITY(area->pixels[a]);
    if (red < min_red)
      min_red = red;
    if (red > max_red)
      max_red = red;
    if (green < min_green)
      min_green = green;
    if (green > max_green)
      max_green = green;
    if (blue < min_blue)
      min_blue = blue;
    if (blue > max_blue)
      max_blue = blue;
  }

  /* calculate widths and determine which predicate function to use based
   * on the result
   */

  red = max_red - min_red;
  green = max_green - min_green;
  blue = max_blue - min_blue;

  if (red > green)
    if (green > blue)
      area->sort_func = (sortfptr)sortRGB;
    else if (red > blue)
      area->sort_func = (sortfptr)sortRBG;
    else
      area->sort_func = (sortfptr)sortBRG;
  else if (green > blue)
    if (red > blue)
      area->sort_func = (sortfptr)sortGRB;
    else
      area->sort_func = (sortfptr)sortGBR;
  else
    area->sort_func = (sortfptr)sortBGR;

  /* insert color area in color area list sorted by number of pixels that
   * the area represents
   */

  largest = *rlargest;
  smallest = *rsmallest;

  if (!largest) {
    largest = smallest = area;
    area->prev = area->next = (struct color_area *)NULL;
  }

  /* if we only have one element, our pixel count is immaterial so we get
   * stuck on the end of the list.
   */

  else if (area->num_pixels < 2) {
    smallest->next = area;
    area->prev = smallest;
    area->next = (struct color_area *)NULL;
    smallest = area;
  }

  /* insert node into list
   */

  else {
    for (tmp_area = largest; tmp_area; tmp_area = tmp_area->next)
      if ((area->pixel_count > tmp_area->pixel_count) ||
	  (tmp_area->num_pixels < 2)) {
	area->prev = tmp_area->prev;
	area->next = tmp_area;
	tmp_area->prev = area;
	if (area->prev)
	  area->prev->next = area;
	else
	  largest = area;
	break;
      }
    if (!tmp_area) {
      area->prev = smallest;
      area->next = (struct color_area *)NULL;
      smallest->next = area;
      smallest = area;
    }
  }
  *rlargest = largest;
  *rsmallest = smallest;
}

/* Reduce an image to n colors: also 24 --> 8 if necessary */

class image *
image::Reduce( unsigned int n )
{ unsigned long pixel_counts[32768]; /* pixel occurrance histogram */
  unsigned short pixel_array[32768];
  unsigned long count, midpoint;
  unsigned int x, y, num_pixels, allocated, depth;
  byte *pixel, *dpixel;
  struct color_area *areas, *largest_area, *smallest_area;
  struct color_area *new_area, *old_area;
  class image *new_image;

  if (n > 32768) /* max # of colors we can handle */
    n = 32768;

  /* create a histogram of particular pixel occurrances
   */

  memset(pixel_counts, 0, 32768 * sizeof(unsigned long));
  switch ((this)->Type()) {
  case IBITMAP:
      return(this);

  case IGREYSCALE:
  case IRGB:
    if ((this)->RGBUsed() <= n)
      return(this);
    pixel = (this)->Data();
    for (y = 0; y < (this)->Height(); y++)
      for (x = 0; x < (this)->Width(); x++) {
	pixel_counts[TLA_TO_15BIT(this->rgb,
				  memToVal(pixel, (this)->Pixlen()))]++;
	pixel += (this)->Pixlen();
      }
    break;

  case ITRUE:
    if ((this)->Pixlen() != 3) {
      fprintf(stderr, "reduce: true color image has strange pixel length?\n");
      return(this);
    }

    pixel = (this)->Data();
    for (y= 0; y < (this)->Height(); y++)
      for (x= 0; x < (this)->Width(); x++) {
	pixel_counts[TRUE_TO_15BIT(memToVal(pixel, 3))]++;
	pixel += 3;
      }
    break;

  default:
      return(this); /* not something we can reduce, thank you anyway */
  }

  /* create array of 15-bit pixel values that actually occur in the image
   */

  num_pixels = 0;
  for (x = 0; x < 32768; x++)
    if (pixel_counts[x] > 0)
      pixel_array[num_pixels++] = (short)x;

  /* create color area array and initialize first element
   */

  areas = (struct color_area *)malloc(n * sizeof(struct color_area));
  areas[0].pixels = pixel_array;
  areas[0].num_pixels = num_pixels;
  largest_area = smallest_area = (struct color_area *)NULL;
  insertColorArea(pixel_counts, &largest_area, &smallest_area, areas);
  allocated = 1;

  /* keep splitting the color area until we have as many color areas as we
   * need
   */

  while (allocated < n) {

    /* if our largest area can't be broken down, we can't even get the
     * number of colors they asked us to
     */

    if (largest_area->num_pixels < 2)
      break;

    /* find midpoint of largest area and do split
     */

    qsort(largest_area->pixels, largest_area->num_pixels, sizeof(short),
	  QSORTFUNC(largest_area->sort_func));
    count = 0;
    midpoint = largest_area->pixel_count / 2;
    for (x = 0; x < largest_area->num_pixels; x++) {
      count += pixel_counts[largest_area->pixels[x]];
      if (count > midpoint)
	break;
    }
    if (x == 0) /* degenerate case; divide in half */
      x = 1;
    new_area = areas + allocated;
    new_area->pixels = largest_area->pixels + x;
    new_area->num_pixels = largest_area->num_pixels - x;
    largest_area->num_pixels = x;
    old_area = largest_area;
    largest_area = largest_area->next;
    if (largest_area)
      largest_area->prev = (struct color_area *)NULL;
    else
      smallest_area = (struct color_area *)NULL;

    /* recalculate for each area of split and insert in the area list
     */

    insertColorArea(pixel_counts, &largest_area, &smallest_area, old_area);
    insertColorArea(pixel_counts, &largest_area, &smallest_area, new_area);

    allocated++;
  }

  /* get destination image
   */

  depth = ::colorsToDepth(allocated);
  new_image = new image;
  (new_image)->newRGBImage( (this)->Width(), (this)->Height(), depth);

  /* calculate RGB table from each color area.  this should really calculate
   * a new color by weighting the intensities by the number of pixels, but
   * it's a pain to scale so this just averages all the intensities.  it
   * works pretty well regardless.
   */

  for (x = 0; x < allocated; x++) {
    long red, green, blue, pixel;

    red = green = blue = 0;
    for (y = 0; y < areas[x].num_pixels; y++) {
      pixel = areas[x].pixels[y];
      red += RED_INTENSITY(pixel);
      green += GREEN_INTENSITY(pixel);
      blue += BLUE_INTENSITY(pixel);
      pixel_counts[pixel] = x;
    }
    red /= areas[x].num_pixels;
    green /= areas[x].num_pixels;
    blue /= areas[x].num_pixels;
    (new_image)->RedPixel( x) = (unsigned short)(red << 11);
    (new_image)->GreenPixel( x) = (unsigned short)(green << 11);
    (new_image)->BluePixel( x) = (unsigned short)(blue << 11);
  };
  (new_image)->RGBUsed() = allocated;
  new_image->rgb->compressed = 1;

  free(areas);

  /* copy old image into new image
   */

  pixel = (this)->Data();
  dpixel = (new_image)->Data();

  switch((this)->Type()) {
      case IGREYSCALE:	
      case IRGB:
	  for (y = 0; y < (this)->Height(); y++)
	      for (x = 0; x < (this)->Width(); x++) {
		  valToMem(pixel_counts[TLA_TO_15BIT(this->rgb,	    memToVal(pixel, (this)->Pixlen()))], dpixel,  (new_image)->Pixlen());
		  pixel += (this)->Pixlen();
		  dpixel += (new_image)->Pixlen();
	      }
	  break;

      case ITRUE:
	  for (y = 0; y < (this)->Height(); y++)
	      for (x = 0; x < (this)->Width(); x++) {
		  valToMem(pixel_counts[TRUE_TO_15BIT(memToVal(pixel, 3))],
			   dpixel, (new_image)->Pixlen());
		  pixel += 3;
		  dpixel += (new_image)->Pixlen();
	      }
	  break;
  }
  new_image->jpegSaveQuality = (this)->GetJPEGSaveQuality();
  (this)->Reset();
  (new_image)->Duplicate( this);
  (new_image)->Destroy();
  return(this);
}

/* expand an image into a true color image
 */

class image *
image::Expand( )
{
  class image *new_image;
  unsigned int x, y;
  Pixel spixval;
  byte *spixel, *dpixel, *line;
  unsigned int linelen;
  byte mask;

  if(TRUEP(this))
    return(this);

  new_image = new image;
  (new_image)->newTrueImage( (this)->Width(), (this)->Height());

  switch ((this)->Type()) {
      case IBITMAP:
	  line = (this)->Data();
	  dpixel = (new_image)->Data();
	  linelen = (this)->Width() / 8 + ((this)->Width() % 8 ? 1 : 0);
	  for (y = 0; y < (this)->Height(); y++) {
	      spixel = line;
	      mask = 0x80;
	      for (x = 0; x < (this)->Width(); x++) {
		  valToMem((mask & *spixel ? 0L : 0xffffff), dpixel, 3);
		  mask >>= 1;
		  if (!mask) {
		      mask = 0x80;
		      spixel++;
		  }
		  dpixel += (new_image)->Pixlen();
	      }
	      line += linelen;
	  }
	  break;
      case IGREYSCALE:
      case IRGB:
	  spixel = (this)->Data();
	  dpixel = (new_image)->Data();
	  for (y = 0; y < (this)->Height(); y++)
	      for (x = 0; x < (this)->Width(); x++) {
		  spixval= memToVal(spixel, (this)->Pixlen());
		  valToMem(RGB_TO_TRUE((this)->RedPixel( spixval),
				       (this)->GreenPixel( spixval),
				       (this)->BluePixel( spixval)),
			   dpixel, (new_image)->Pixlen());
		  spixel += (this)->Pixlen();
		  dpixel += (new_image)->Pixlen();
	      }
	  break;
  }
  new_image->jpegSaveQuality = (this)->GetJPEGSaveQuality();
  (this)->Reset();
  (new_image)->Duplicate( this);
  (new_image)->Destroy();
  return(this);
}

class image *
image::Bit2Grey( )
{
  class image *new_image;
  unsigned int x, y;
  byte *spixel, *dpixel, *line;
  unsigned int linelen;
  byte mask;

  if(TRUEP(this) || RGBP(this) || GREYSCALEP(this))
    return(this);

  new_image = new image;
  (new_image)->newGreyImage( (this)->Width(), (this)->Height(), 1);
  (new_image)->RedPixel( 0) = (new_image)->GreenPixel( 0) = (new_image)->BluePixel( 0) = 0;
  (new_image)->RedPixel( 1) = (new_image)->GreenPixel( 1) = (new_image)->BluePixel( 1) = 65535;
  (new_image)->RGBUsed() = 2;

  line = (this)->Data();
  dpixel = (new_image)->Data();
  linelen = ((this)->Width() / 8) + ((this)->Width() % 8 ? 1 : 0);
  for (y = 0; y < (this)->Height(); y++) {
      spixel = line;
      mask = 0x80;
      for (x = 0; x < (this)->Width(); x++) {
	  valToMem((mask & *spixel ? 0L : 1L), dpixel, (new_image)->Pixlen());
	  mask >>= 1;
	  if (!mask) {
	      mask = 0x80;
	      spixel++;
	  }
	  dpixel += (new_image)->Pixlen();
      }
      line += linelen;
  }
  new_image->jpegSaveQuality = (this)->GetJPEGSaveQuality();
  (this)->Reset();
  (new_image)->Duplicate( this);
  (new_image)->Destroy();
  return(this);
}

int
image::depthToColors( int  n )
{
  ATKinit;
  return(DEPTHTOCOLORS(n));
}

int
image::colorsToDepth( int  n )
{
  ATKinit;
  return(::colorsToDepth(n));
}

#define MaxIntensity  65536	/* maximum possible Intensity */
#define MaxGrey       32768	/* limits on the grey levels used */
#define Threshold     16384	/* in the dithering process */
#define MinGrey           0

 



/*
 * simple floyd-steinberg dither with serpentine raster processing
 */

class image *
image::Dither( )
{
  class image   *imagep;	/* destination image */
  unsigned int   *grey;		/* grey map for source image */
  unsigned int    spl;		/* source pixel length in bytes */
  unsigned int    dll;		/* destination line length in bytes */
  unsigned char  *src;		/* source data */
  unsigned char  *dst;		/* destination data */
  int            *curr;		/* current line buffer */
  int            *next;		/* next line buffer */
  int            *swap;		/* for swapping line buffers */
  Pixel           color;	/* pixel color */
  unsigned int    level;	/* grey level */
  unsigned int    i, j;		/* loop counters */

  /*
   * check the source image
   */
  if (BITMAPP(this))
    return(this);

  /*
   * allocate destination image
   */
  imagep = new image;
  (imagep)->newBitImage( (this)->Width(), (this)->Height());

  /*
   * if the number of entries in the colormap isn't too large, compute
   * the grey level for each entry and store it in grey[]. else the
   * grey levels will be computed on the fly.
   */
  if (RGBP(this) && ((this)->Depth() <= 16)) {
    grey = (unsigned int *)malloc(sizeof(unsigned int) * (this)->RGBUsed());
    for (i = 0; i < (this)->RGBUsed(); i++)
      grey[i] =
	(colorIntensity((this)->RedPixel( i),
			(this)->GreenPixel( i),
			(this)->BluePixel( i)) >> 1);

    for (i = 0; i < (this)->RGBUsed(); i++)
      grey[i] = tone_scale_adjust(grey[i]);
  }
  else
  {
    grey = NULL;
  }

  /*
   * dither setup
   */
  spl = (this)->Pixlen();
  dll = ((imagep)->Width() / 8) + ((imagep)->Width() % 8 ? 1 : 0);
  src = (this)->Data();
  dst = (imagep)->Data();

  curr = (int *) malloc(sizeof(int) * ((this)->Width() + 2));
  next = (int *) malloc(sizeof(int) * ((this)->Width() + 2));
  curr += 1;
  next += 1;
  for (j = 0; j < (this)->Width(); j++) {
    curr[j] = 0;
    next[j] = 0;
  }

  /*
   * primary dither loop
   */
  for (i = 0; i < (this)->Height(); i++) {
    /* copy the row into the current line */
    for (j = 0; j < (this)->Width(); j++) {
      color = memToVal(src, spl);
      src += spl;
      
      if (RGBP(this)) {
	if (grey == NULL)
	  level =
	    tone_scale_adjust(colorIntensity((this)->RedPixel( color),
					     (this)->GreenPixel( color),
					     (this)->BluePixel( color)) >> 1);
	else
	  level = grey[color];
      }
      else {
	level =
	  tone_scale_adjust(colorIntensity((TRUE_RED(color) << 8),
					   (TRUE_GREEN(color) << 8),
					   (TRUE_BLUE(color) << 8)) >> 1);
      }
      curr[j] += level;
    }

    /* dither the current line */
    if (i & 0x01)
      RightToLeft(curr, next, (this)->Width());
    else
      LeftToRight(curr, next, (this)->Width());

    /* copy the dithered line to the destination image */
    for (j = 0; j < (this)->Width(); j++)
      if (curr[j] < Threshold)
	dst[j / 8] |= 1 << (7 - (j & 7));
    dst += dll;
    
    /* circulate the line buffers */
    swap = curr;
    curr = next;
    next = swap;
    for (j = 0; j < (this)->Width(); j++)
      next[j] = 0;
  }

  /*
   * clean up
   */
  free((byte *)grey);
  free((byte *)(curr-1));
  free((byte *)(next-1));

  imagep->jpegSaveQuality = (this)->GetJPEGSaveQuality();
  (this)->Reset();
  (imagep)->Duplicate( this);
  (imagep)->Destroy();
  return(this);
}


/*
 * a _very_ simple tone scale adjustment routine. provides a piecewise
 * linear mapping according to the following:
 *
 *      input:          output:
 *     0 (MinGrey)    0 (MinGrey)
 *     Threshold      Threshold/2
 *     MaxGrey        MaxGrey
 * 
 * this should help things look a bit better on most displays.
 */
static unsigned int 
tone_scale_adjust(unsigned int val)
{
  unsigned int rslt;
  
  if (val < Threshold)
    rslt = val / 2;
  else
    rslt = (((val - Threshold) * (MaxGrey-(Threshold/2))) /
	    (MaxGrey-Threshold)) + (Threshold/2);

  return rslt;
}


/*
 * dither a line from left to right
 */
static void 
LeftToRight(int  *curr, int  *next, int   width)
{
  int idx;
  int error;
  int output;

  for (idx = 0; idx < width; idx++) {
    output       = (curr[idx] > Threshold) ? MaxGrey : MinGrey;
    error        = curr[idx] - output;
    curr[idx]    = output;
    next[idx-1] += error * 3 / 16;
    next[idx]   += error * 5 / 16;
    next[idx+1] += error * 1 / 16;
    curr[idx+1] += error * 7 / 16;
  }
}


/*
 * dither a line from right to left
 */
static void 
RightToLeft(int  *curr, int  *next, int   width)
{
  int idx;
  int error;
  int output;

  for (idx = (width-1); idx >= 0; idx--) {
    output       = (curr[idx] > Threshold) ? MaxGrey : MinGrey;
    error        = curr[idx] - output;
    curr[idx]    = output;
    next[idx+1] += error * 3 / 16;
    next[idx]   += error * 5 / 16;
    next[idx-1] += error * 1 / 16;
    curr[idx-1] += error * 7 / 16;
  }
}

/* simple dithering algorithm, really optimized for the 4x4 array
 */

class image *
image::Halftone( )
{ class image  *imagep;
  unsigned char *sp, *dp, *dp2; /* data pointers */
  unsigned int   dindex;        /* index into dither array */
  unsigned int   spl;           /* source pixel length in bytes */
  unsigned int   dll;           /* destination line length in bytes */
  Pixel          color;         /* pixel color */
  unsigned int  *index;         /* index into dither array for a given pixel */
  unsigned int   a, x, y;       /* random counters */

  if (BITMAPP(this))
    return(this);

  /* set up
   */

  imagep = new image;
  (imagep)->newBitImage( (this)->Width() * 4, (this)->Height() * 4);
  spl = (this)->Pixlen();
  dll = ((imagep)->Width() / 8) + ((imagep)->Width() % 8 ? 1 : 0);

  /* if the number of possible pixels isn't very large, build an array
   * which we index by the pixel value to find the dither array index
   * by color brightness.  we do this in advance so we don't have to do
   * it for each pixel.  things will break if a pixel value is greater
   * than (1 << depth), which is bogus anyway.  this calculation is done
   * on a per-pixel basis if the colormap is too big.
   */

  if (RGBP(this) && ((this)->Depth() <= 16)) {
    index = (unsigned int *)malloc(sizeof(unsigned int) * (this)->RGBUsed());
    for (x = 0; x < (this)->RGBUsed(); x++) {
      *(index + x) =
	((unsigned long)colorIntensity((this)->RedPixel( x),
				       (this)->GreenPixel( x),
				       (this)->BluePixel( x))) / GRAYSTEP;
      if (*(index + x) >= GRAYS) /* rounding errors can do this */
	*(index + x) = GRAYS - 1;
    }
  }
  else
    index = NULL;

  /* dither each pixel
   */

  sp = (this)->Data();
  dp = (imagep)->Data();
  for (y = 0; y < (this)->Height(); y++) {
    for (x = 0; x < (this)->Width(); x++) {
      dp2 = dp + (x >> 1);
      color = memToVal(sp, spl);
      if (RGBP(this)) {
	if (index)
	  dindex = *(index + color);
	else {
	  dindex = 
	    ((unsigned long)colorIntensity((this)->RedPixel( color),
					   (this)->GreenPixel( color),
					   (this)->BluePixel( color))) / GRAYSTEP;
	}
      }
      else {
	dindex =
	  ((unsigned long)colorIntensity((TRUE_RED(color) << 8),
					 (TRUE_GREEN(color) << 8),
					 (TRUE_BLUE(color) << 8))) / GRAYSTEP;
      }
      if (dindex >= GRAYS) /* rounding errors can do this */
	dindex = GRAYS - 1;

      /* loop for the four Y bits in the dither pattern, putting all
       * four X bits in at once.  if you think this would be hard to
       * change to be an NxN dithering array, you're right, since we're
       * banking on the fact that we need only shift the mask based on
       * whether x is odd or not.  an 8x8 array wouldn't even need that,
       * but blowing an image up by 64x is probably not a feature.
       */

      if (x & 1)
	for (a = 0; a < 4; a++, dp2 += dll)
	  *dp2 |= DitherBits[dindex][a];
      else
	for (a = 0; a < 4; a++, dp2 += dll)
	  *dp2 |= (DitherBits[dindex][a] << 4);
      sp += spl;
    }
    dp += (dll << 2); /* (dll * 4) but I like shifts */
  }

  imagep->jpegSaveQuality = (this)->GetJPEGSaveQuality();
  (this)->Reset();
  (imagep)->Duplicate( this);
  (imagep)->Destroy();
  return(this);
}

static int tmpfilectr = 0;

long image::WriteOtherFormat(FILE  *file, long  writeID, int  level, int  usagetype, char  *boundary)
{
    FILE *tmpfp;
    char Fnam[1000];
    class gif *gifp = new gif;

    if(this->writeID == writeID)  return(this->id);
    this->writeID = writeID;

    fprintf(file, "\n--%s\nContent-type: image/gif\nContent-Transfer-Encoding: base64\n\n", boundary);
    sprintf(Fnam, "/tmp/imagegif.%d.%d", getpid(), tmpfilectr++);
    if(!(tmpfp = fopen(Fnam, "w"))) {
	(gifp)->Destroy();
	return(0);
    }
    if((this)->Data()) (this)->Duplicate( (class image *) gifp);
    else {
	gifp->newRGBImage(1, 1, 8);
	gifp->RedPixel(0)=gifp->GreenPixel(0)=gifp->BluePixel(0)=65535;
	gifp->RedPixel(1)=gifp->GreenPixel(1)=gifp->BluePixel(1)=0;
	gifp->RGBUsed()=2;
	if(gifp->Data()) *(gifp->Data())=0;
    }
    gifp->WriteNative( tmpfp, NULL);
    fclose(tmpfp);
    (gifp)->Destroy();
    if(!(tmpfp = fopen(Fnam, "r"))) {
	unlink(Fnam);
	return(0);
    }
    to64(tmpfp, file);
    fclose(tmpfp);
    unlink(Fnam);
    return(this->id);
}

boolean
image::ReadOtherFormat(FILE  *file, char  *fmt, char  *encoding, char  *desc)
{
    char TmpFile[250];
    FILE *tmpfp = NULL;
    int code;

    if (strcmp(fmt, "image/gif")
	&& strcmp(fmt, "image/x-gif")
	&& strcmp(fmt, "image/pbm")
	&& strcmp(fmt, "image/pbm")
	&& strcmp(fmt, "image/ppm")
	&& strcmp(fmt, "image/pgm")
	&& strcmp(fmt, "image/jpeg") ) return(FALSE);

    /* Need to decode base64 or q-p here */
    if (!strncmp(encoding, "base64", 6)
	 || !strncmp(encoding, "quoted-printable", 16)) {
	sprintf(TmpFile, "/tmp/imagegif.%d.%d", getpid(), tmpfilectr++);
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
    } else return FALSE;

    code = (this)->Read( file, -1);
    if (tmpfp) {
	fclose(tmpfp);
	unlink(TmpFile); 
    }
    if (code == dataobject_NOREADERROR) {
	return(TRUE);
    } else {
	return (FALSE);
    }
}

long
image::WriteNative( FILE  *file, const char  *filename )
{
    printf("image_WriteNative\n");
    return -1;
}

int
image::Load( const char  *fullname, FILE  *fp )
{
/* This method should be overridden by subclasses of image */
  return(0);
}

void
image::SetSaveFormatString( const char  *format )
{
    if(this->saveformatstring)
	free(this->saveformatstring);
    this->saveformatstring = NULL;
    if(format && *format != (char) 0) {
	const char *src;
	char *dest;
	this->saveformatstring = (char *) malloc(strlen(format) + 1);
	for( src = format, dest = this->saveformatstring; *src ;)
	    *dest++ = tolower(*src++);
	*dest = (char)0;
	(this)->SetModified(); /* .. so it will write out new data in SendImageData */
    }
}

static inline unsigned int ColorDistance(unsigned short r, unsigned short g, unsigned short b, unsigned short cred, unsigned short cgreen, unsigned short cblue) {
    int dr, dg, db;
    dr = (r - cred);	
    dg = (g - cgreen);	
    db = (b - cblue);	
    dr = abs(dr)>>2;
    dg = abs(dg)>>2;
    db = abs(db)>>2;
    return dr*dr + dg*dg + db*db;
}

image *image::ColorDither(RGBMap *map) {
    image *result=new image;
    if(type==IBITMAP) {
	fprintf(stderr, "image: Attempted to ColorDither a bitmap!\n");
	Duplicate(result);
	return result;
    }
    if(map->used<2) {
	fprintf(stderr, "image: Attemped to ColorDither to less than 2 colors!\n");
	Duplicate(result);
	return result;
    }
    (result)->newRGBImage(Width(), Height(), colorsToDepth(map->used));
    result->freeRGBMapData();
    result->rgb=map;
    long* thisrerr;
    long* nextrerr;
    long* thisgerr;
    long* nextgerr;
    long* thisberr;
    long* nextberr;
    long* temperr;
    long sr, sg, sb, err;
    unsigned int row, col, limitcol;
#define FS_SCALE 1024
    int fs_direction=1;
    Intensity R = 0, G = 0, B = 0;
    Pixel pixval;
    Pixel hash[32768];
    memset(hash, 0xff, sizeof(hash));
    thisrerr = (long*) malloc((Width()+2)*sizeof(long));
    nextrerr = (long*) malloc((Width()+2)*sizeof(long));
    thisgerr = (long*) malloc((Width()+2)*sizeof(long));
    nextgerr = (long*) malloc((Width()+2)*sizeof(long));
    thisberr = (long*) malloc((Width()+2)*sizeof(long));
    nextberr = (long*) malloc((Width()+2)*sizeof(long));
    SRANDOM((int)time(0));
    for ( col = 0; col < Width() + 2; ++col )
    {
	thisrerr[col] = RANDOM( ) % ( FS_SCALE * 2 ) - FS_SCALE;
	thisgerr[col] = RANDOM( ) % ( FS_SCALE * 2 ) - FS_SCALE;
	thisberr[col] = RANDOM( ) % ( FS_SCALE * 2 ) - FS_SCALE;
	/* (random errors in [-1 .. 1]) */
    }
    unsigned char *data=Data();
    unsigned char *pP;
    unsigned char *destdata=result->Data();
    unsigned char *dest;
#define RGB_TO_15BIT(R, G, B)     \
  ((((R) & 0xf800) >> 1) | \
   (((G) & 0xf800) >> 6) | \
   (((B) & 0xf800) >> 11))
    {
	unsigned int i;
	for ( i = 0; i < map->used; ++i )
	{
	    hash[RGB_TO_15BIT(map->red[i], map->green[i], map->blue[i])]=i;
	}
    }
    for(row=0;row<Height();++row) {
	memset(nextrerr,0,(Width()+2)*sizeof(long));
	memset(nextgerr,0,(Width()+2)*sizeof(long));
	memset(nextberr,0,(Width()+2)*sizeof(long));
	if(fs_direction) {
	    col=0;
	    limitcol=Width();
	    pP=data+row*(pixlen*Width());
	    dest=destdata+row*(result->pixlen*Width());
	} else {
	    col = Width() -1;
	    limitcol = -1;
	    pP=data+row*(pixlen*Width())+ col*pixlen;
	    dest=destdata+row*(result->pixlen*Width())+ col*result->pixlen;
	}
	do {
	    pixval=memToVal(pP, pixlen);
	    unsigned short h;
	    switch(type) {
		case IGREYSCALE:
		case IRGB:
		    R=RedPixel(pixval);
		    G=GreenPixel(pixval);
		    B=BluePixel(pixval);
		    break;
		case ITRUE:
		    R=TRUE_RED(pixval);
		    G=TRUE_GREEN(pixval);
		    B=TRUE_BLUE(pixval);
		    break;
	    }
	    sr = R + (thisrerr[col + 1]/FS_SCALE);
	    sg = G + (thisgerr[col + 1]/FS_SCALE);
	    sb = B + (thisberr[col + 1]/FS_SCALE);

	    if ( sr < 0 ) sr = 0;
	    else if ( sr > 65535 ) sr = 65535;
	    if ( sg < 0 ) sg = 0;
	    else if ( sg > 65535 ) sg = 65535;
	    if ( sb < 0 ) sb = 0;
	    else if ( sb > 65535 ) sb = 65535;
	    R=sr;
	    G=sg;
	    B=sb;
	    h=RGB_TO_15BIT(R, G, B);
	    Pixel index=hash[h];
	    if(index==NIL_PIXEL) { /* No; search colormap for closest match. */
		unsigned int i;
		unsigned long dist, newdist;
		dist = ~0;
		for ( i = 0; i < map->used; ++i )
		{
		    newdist = ColorDistance(R, G, B, map->red[i], map->green[i], map->blue[i]);
		    if ( newdist < dist )
		    {
			index = i;
			dist = newdist;
			if(dist==0) break;
		    }
		}
		if(index<map->used) {
		    hash[h]=index;
		} else index=0;
	    }
	    /* Propagate Floyd-Steinberg error terms. */
	    if ( fs_direction )
	    {
		err = ( sr - (long) map->red[index] ) * FS_SCALE;
		thisrerr[col + 2] += ( err * 7 ) / 16;
		nextrerr[col    ] += ( err * 3 ) / 16;
		nextrerr[col + 1] += ( err * 5 ) / 16;
		nextrerr[col + 2] += ( err     ) / 16;
		err = ( sg - (long) map->green[index] ) * FS_SCALE;
		thisgerr[col + 2] += ( err * 7 ) / 16;
		nextgerr[col    ] += ( err * 3 ) / 16;
		nextgerr[col + 1] += ( err * 5 ) / 16;
		nextgerr[col + 2] += ( err     ) / 16;
		err = ( sb - (long) map->blue[index] ) * FS_SCALE;
		thisberr[col + 2] += ( err * 7 ) / 16;
		nextberr[col    ] += ( err * 3 ) / 16;
		nextberr[col + 1] += ( err * 5 ) / 16;
		nextberr[col + 2] += ( err     ) / 16;
	    }
	    else
	    {
		err = ( sr - (long) map->red[index] ) * FS_SCALE;
		thisrerr[col    ] += ( err * 7 ) / 16;
		nextrerr[col + 2] += ( err * 3 ) / 16;
		nextrerr[col + 1] += ( err * 5 ) / 16;
		nextrerr[col    ] += ( err     ) / 16;
		err = ( sg - (long) map->green[index] ) * FS_SCALE;
		thisgerr[col    ] += ( err * 7 ) / 16;
		nextgerr[col + 2] += ( err * 3 ) / 16;
		nextgerr[col + 1] += ( err * 5 ) / 16;
		nextgerr[col    ] += ( err     ) / 16;
		err = ( sb - (long) map->blue[index] ) * FS_SCALE;
		thisberr[col    ] += ( err * 7 ) / 16;
		nextberr[col + 2] += ( err * 3 ) / 16;
		nextberr[col + 1] += ( err * 5 ) / 16;
		nextberr[col    ] += ( err     ) / 16;

	    }
	    valToMem(index, dest, result->pixlen);
	    if(fs_direction) {
		++col;
		pP+=pixlen;
		dest+=result->pixlen;
	    } else {
		--col;
		pP-=pixlen;
		dest-=result->pixlen;
	    }
	} while (col != limitcol);

	temperr = thisrerr;
	thisrerr = nextrerr;
	nextrerr = temperr;
	temperr = thisgerr;
	thisgerr = nextgerr;
	nextgerr = temperr;
	temperr = thisberr;
	thisberr = nextberr;
	nextberr = temperr;
	fs_direction = ! fs_direction;
    }
    free(thisrerr);
    free(nextrerr);
    free(thisgerr);
    free(nextgerr);
    free(thisberr);
    free(nextberr);
    return result;
}

  

image *image::ColorDitherToCube(int reds, int greens, int blues) {
    image *result=new image;
    if(type==IBITMAP) {
	fprintf(stderr, "image: Attempted to ColorDither a bitmap!\n");
	Duplicate(result);
	return result;
    }
    if(reds*greens*blues<8) {
	fprintf(stderr, "image: Attemped to ColorDither to less than 8 colors!\n");
	Duplicate(result);
	return result;
    }
    int depth=colorsToDepth(reds*greens*blues);
    depth=((depth+7)/8)*8;
    (result)->newRGBImage(Width(), Height(), depth);
    RGBMap *map=result->rgb;
    map->used=reds*greens*blues;
    int i, j, k;
    for(i=0;i<reds;i++) {
	for(j=0;j<greens;j++) {
	    for(k=0;k<blues;k++) {
		int indx=i*greens*blues+j*blues+k;
		result->rgb->red[indx]=(i*65535)/(reds-1);
		result->rgb->green[indx]=(j*65535)/(greens-1);
		result->rgb->blue[indx]=(k*65535)/(blues-1);
	    }
	}
    }
    long* thisrerr;
    long* nextrerr;
    long* thisgerr;
    long* nextgerr;
    long* thisberr;
    long* nextberr;
    long* temperr;
    long sr, sg, sb, err;
    unsigned int row, col, limitcol;
    int fs_direction=1;
    Intensity R = 0, G = 0, B = 0;
    Pixel pixval;
    thisrerr = (long*) malloc((Width()+2)*sizeof(long));
    nextrerr = (long*) malloc((Width()+2)*sizeof(long));
    thisgerr = (long*) malloc((Width()+2)*sizeof(long));
    nextgerr = (long*) malloc((Width()+2)*sizeof(long));
    thisberr = (long*) malloc((Width()+2)*sizeof(long));
    nextberr = (long*) malloc((Width()+2)*sizeof(long));
    SRANDOM(0);
    for ( col = 0; col < Width() + 2; ++col )
    {
	thisrerr[col] = RANDOM( ) % ( FS_SCALE * 2 ) - FS_SCALE;
	thisgerr[col] = RANDOM( ) % ( FS_SCALE * 2 ) - FS_SCALE;
	thisberr[col] = RANDOM( ) % ( FS_SCALE * 2 ) - FS_SCALE;
	/* (random errors in [-1 .. 1]) */
    }
    unsigned char *data=Data();
    unsigned char *pP;
    unsigned char *destdata=result->Data();
    unsigned char *dest;

    for(row=0;row<Height();++row) {
	memset(nextrerr,0,(Width()+2)*sizeof(long));
	memset(nextgerr,0,(Width()+2)*sizeof(long));
	memset(nextberr,0,(Width()+2)*sizeof(long));
	if(fs_direction) {
	    col=0;
	    limitcol=Width();
	    pP=data+row*(pixlen*Width());
	    dest=destdata+row*(result->pixlen*Width());
	} else {
	    col = Width() -1;
	    limitcol = -1;
	    pP=data+row*(pixlen*Width())+ col*pixlen;
	    dest=destdata+row*(result->pixlen*Width())+ col*result->pixlen;
	}
	do {
	    pixval=memToVal(pP, pixlen);
	    switch(type) {
		case IGREYSCALE:
		case IRGB:
		    R=RedPixel(pixval);
		    G=GreenPixel(pixval);
		    B=BluePixel(pixval);
		    break;
		case ITRUE:
		    R=TRUE_RED(pixval);
		    G=TRUE_GREEN(pixval);
		    B=TRUE_BLUE(pixval);
		    break;
	    }
	    sr = R + (thisrerr[col + 1]/FS_SCALE);
	    sg = G + (thisgerr[col + 1]/FS_SCALE);
	    sb = B + (thisberr[col + 1]/FS_SCALE);

	    if ( sr < 0 ) sr = 0;
	    else if ( sr > 65535 ) sr = 65535;
	    if ( sg < 0 ) sg = 0;
	    else if ( sg > 65535 ) sg = 65535;
	    if ( sb < 0 ) sb = 0;
	    else if ( sb > 65535 ) sb = 65535;
	    Pixel index=((sr*(reds-1))/65535)*greens*blues + ((sg*(greens-1))/65535)*blues + (sb*(blues-1))/65535;
	    /* Propagate Floyd-Steinberg error terms. */
	    if ( fs_direction )
	    {
		err = ( sr - (long) map->red[index] ) * FS_SCALE;
		thisrerr[col + 2] += ( err * 7 ) / 16;
		nextrerr[col    ] += ( err * 3 ) / 16;
		nextrerr[col + 1] += ( err * 5 ) / 16;
		nextrerr[col + 2] += ( err     ) / 16;
		err = ( sg - (long) map->green[index] ) * FS_SCALE;
		thisgerr[col + 2] += ( err * 7 ) / 16;
		nextgerr[col    ] += ( err * 3 ) / 16;
		nextgerr[col + 1] += ( err * 5 ) / 16;
		nextgerr[col + 2] += ( err     ) / 16;
		err = ( sb - (long) map->blue[index] ) * FS_SCALE;
		thisberr[col + 2] += ( err * 7 ) / 16;
		nextberr[col    ] += ( err * 3 ) / 16;
		nextberr[col + 1] += ( err * 5 ) / 16;
		nextberr[col + 2] += ( err     ) / 16;
	    }
	    else
	    {
		err = ( sr - (long) map->red[index] ) * FS_SCALE;
		thisrerr[col    ] += ( err * 7 ) / 16;
		nextrerr[col + 2] += ( err * 3 ) / 16;
		nextrerr[col + 1] += ( err * 5 ) / 16;
		nextrerr[col    ] += ( err     ) / 16;
		err = ( sg - (long) map->green[index] ) * FS_SCALE;
		thisgerr[col    ] += ( err * 7 ) / 16;
		nextgerr[col + 2] += ( err * 3 ) / 16;
		nextgerr[col + 1] += ( err * 5 ) / 16;
		nextgerr[col    ] += ( err     ) / 16;
		err = ( sb - (long) map->blue[index] ) * FS_SCALE;
		thisberr[col    ] += ( err * 7 ) / 16;
		nextberr[col + 2] += ( err * 3 ) / 16;
		nextberr[col + 1] += ( err * 5 ) / 16;
		nextberr[col    ] += ( err     ) / 16;

	    }
	    valToMem(index, dest, result->pixlen);
	    if(fs_direction) {
		++col;
		pP+=pixlen;
		dest+=result->pixlen;
	    } else {
		--col;
		pP-=pixlen;
		dest-=result->pixlen;
	    }
	} while (col != limitcol);

	temperr = thisrerr;
	thisrerr = nextrerr;
	nextrerr = temperr;
	temperr = thisgerr;
	thisgerr = nextgerr;
	nextgerr = temperr;
	temperr = thisberr;
	thisberr = nextberr;
	nextberr = temperr;
	fs_direction = ! fs_direction;
    }
    free(thisrerr);
    free(nextrerr);
    free(thisgerr);
    free(nextgerr);
    free(thisberr);
    free(nextberr);
    return result;
}

 
