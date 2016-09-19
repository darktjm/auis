/* rle.c - class description for interface from RLE format to image */
/*
	Copyright Carnegie Mellon University 1992 - All rights reserved
*/

/* 
 * rle - read in a Utah RLE Toolkit type image.
 * 
 * Author:	Graeme Gill
 * Date: 	30/5/90
 * 
 * Bugs - doesn't free up memory used by rle functions.
 *
 */

#include <andrewos.h>
ATK_IMPL("rle.H")
#include <rle.H>
#include <math.h>
#include <stdio.h>
#include <image.H>
/*#include <rle.h>*/
/* 
 * svfb_global.h - externally visible variables for svfb.
 * 
 * Author:	Todd W. Fuqua
 * 		Computer Science Dept.
 * 		University of Utah
 * Date:	Sun Jul 29 1984
 * Copyright (c) 1984 Todd W. Fuqua
 * 
 * Added optimised dither square size globals
 * 88/07/13 Graeme W. Gill
 */

enum sv_dispatch {
    RUN_DISPATCH
};

/* some compilers have problems converting unsigned bytes to float */
#define BYTEBUG

/* On BIGENDIAN machines swap the bytes. Everything but vax's and
 * pdp-11's (not sure if it's pdp11 or PDP11 ??)
 * are considered BIGENDIAN machines.
 */

#define SWAB(val) (val= memToValLSB((byte *)&val, sizeof(val)))

/* ****************************************************************
 * TAG( rle_pixel rle_map )
 *
 * Typedef for 8-bit (or less) pixel data.
 *
 * Typedef for 16-bit color map data.
 */
typedef unsigned char rle_pixel;
typedef unsigned short rle_map;

/*
 * Defines for traditional channel numbers
 */
#define	SV_RED	    0		/* red channel traditionally here */
#define SV_GREEN    1		/* green channel traditionally here */
#define	SV_BLUE	    2		/* blue channel traditionally here */
#define SV_ALPHA    -1		/* Alpha channel here */

/*
 * Return values from rle_get_setup
 */
#define	RLE_SUCCESS	0
#define	RLE_NOT_RLE	-1
#define	RLE_NO_SPACE	-2
#define	RLE_EMPTY	-3
#define	RLE_EOF		-4

/*
 * TAG( sv_globals )
 *
 * Definition of "globals" structure used by RLE routines
 */

extern struct sv_globals {
    enum sv_dispatch sv_dispatch; /* type of file to create */
    int	    sv_ncolors,		/* number of color channels */
	  * sv_bg_color,	/* pointer to bg color vector */
	    sv_alpha,		/* if !0, save alpha channel */
	    sv_background,	/* (background) 0->just save pixels, */
				/* 1->overlay, 2->clear to bg first */
	    sv_xmin,		/* lower X bound (left) */
	    sv_xmax,		/* upper X bound (right) */
	    sv_ymin,		/* lower Y bound (bottom) */
	    sv_ymax,		/* upper Y bound (top) */
	    sv_ncmap,		/* number of color channels in color map */
				/* map only saved if != 0 */
	    sv_cmaplen;		/* log2 of color map length */
    rle_map * sv_cmap;	/* pointer to color map array */
    char    ** sv_comments;	/* pointer to array of pointers to comments */
    FILE  * svfb_fd;		/* output file */
    /* 
     * Bit map of channels to read/save.  Indexed by (channel mod 256).
     * Alpha channel sets bit 255.
     * 
     * Indexing (0 <= c <= 255):
     *	    sv_bits[c/8] & (1 << (c%8))
     */
#define SV_SET_BIT(glob,bit) \
     ((glob).sv_bits[((bit)&0xff)/8] |= (1<<((bit)&0x7)))
#define SV_CLR_BIT(glob,bit) \
	((glob).sv_bits[((bit)&0xff)/8] &= ~(1<<((bit)&0x7)))
#define SV_BIT(glob,bit) \
	((glob).sv_bits[((bit)&0xff)/8] & (1<<((bit)&0x7)))
    char    sv_bits[256/8];
    /* 
     * Local storage for rle_getrow & sv_putrow.
     * rle_getrow has
     *	    scan_y	int	    current Y scanline
     *	    vert_skip	int	    number of lines to skip
     * sv_putrow has
     *	    nblank	int	    number of blank lines
     *	    brun	short(*)[2] Array of background runs.
     *	    fileptr	long	    Position in output file.
     */
     union {
	struct {
	    int	scan_y,
		vert_skip;
	    char is_eof,	/* Set when EOF or EofOp encountered */
		is_seek;	/* If true, can seek input file */
	} get;
	struct {
	    int	nblank;
	    short (*brun)[2];
	    long fileptr;
	} put;
     } sv_private;
} sv_globals;


/* 
 * buildmap - build a more usable colormap from data in globals struct.
 */
extern rle_pixel **
buildmap();
/* ( globals, minmap, gamma )
 * struct sv_globals * globals;
 * int minmap;
 * double gamma;
 */

/*
 * rle_getcom - get a specific comment from the image comments.
 */
extern char *
rle_getcom();
/* ( name, globals )
 * char * name;
 * struct sv_globals * globals;
 */

/*
 * rle_putcom - put (or replace) a comment into the image comments.
 */
extern char *
rle_putcom();
/* ( value, globals )
 * char * value;
 * struct sv_globals * globals;
 */

/*
 * rle_delcom - delete a specific comment from the image comments.
 */
extern char *
rle_delcom();
/* ( name, globals )
 * char * name;
 * struct sv_globals * globals;
 */

/*
 * dither globals
 */

extern int dith_levels;	/* target effective number of levels, default = 128 */
extern int dith_np2;	/* set non-zero to use non-power_of_2 matrix size */
extern int dith_size;	/* effective size of the dither matrix chosen */



#undef  DEBUG

#ifdef DEBUG
# define debug(xx)	fprintf(stderr,xx)
#else
# define debug(xx)
#endif

/* 
 * Opcode definitions
 */

#define     LONG                0x40
#define	    RSkipLinesOp	1
#define	    RSetColorOp		2
#define	    RSkipPixelsOp	3
#define	    RByteDataOp		5
#define	    RRunDataOp		6
#define	    REOFOp		7

#define     H_CLEARFIRST        0x1	/* clear framebuffer flag */
#define	    H_NO_BACKGROUND	0x2	/* if set, no bg color supplied */
#define	    H_ALPHA		0x4   /* if set, alpha channel (-1) present */
#define	    H_COMMENT		0x8	/* if set, comments present */

struct XtndRsetup
{
    short   h_xpos,
            h_ypos,
            h_xlen,
            h_ylen;
    char    h_flags,
            h_ncolors,
	    h_pixelbits,
	    h_ncmap,
	    h_cmaplen;
};
#define	    SETUPSIZE	((4*2)+5)

/* "Old" RLE format magic numbers */
#define	    RMAGIC	('R' << 8)	/* top half of magic number */
#define	    WMAGIC	('W' << 8)	/* black&white rle image */

#define	    XtndRMAGIC	((short)0xcc52)	/* RLE file magic number */

#ifndef MIN
#define MIN(a,b) ( (a)<(b) ? (a) : (b))
#endif

/* input file stuff */
static int ptype;				/* picture type : */
#define BW_NM	0		/* black and white, no map */
#define BW_M	1		/* black and white, and a map */
#define SC_M	2		/* single colour channel and colour map */
#define C_NM	3		/* full colour, no maps */
#define C_M		4		/* full colour with colour maps */
static rle_pixel **fmaps;		/* file colour maps from buildmap() */
static unsigned char **scan;	/* buffer for input data */
static int x_min;		/* copy of picture x_min */
static int y_min;		/* copy of picture y_min */

/* option stuff (Not functional) */
static float disp_gam = 1.0;	/* default display gamma correction factor */
static int iflag=0;			/* user suplied image gamma */
static float img_gam = 1.0;			/* image gamma */
static int bwflag = 0;			/* black and white flag */
static int gammamap[256];
static int background[3] = {0,0,0};	/* our background colour */

/* stuff for dithering colour pictures */
int colmap[216][3];		/* for dither map */
int magic[16][16];
int modN[256];
int divN[256];

static int sv_bg_color[3] = { 0, 0, 0 };

struct sv_Globals sv_globals = {
    RUN_DISPATCH,		/* dispatch value */
    3,				/* 3 colors */
    sv_bg_color,		/* background color */
    0,				/* (alpha) if 1, save alpha channel */
    2,				/* (background) 0->just save pixels, */
				/* 1->overlay, 2->clear to bg first */
    0, 511,			/* (xmin, xmax) X bounds to save */
    0, 479,			/* (ymin, ymax) Y bounds to save */
    0,				/* ncmap (if != 0, save color map) */
    8,				/* cmaplen (log2 of length of color map) */
    NULL,			/* pointer to color map */
    NULL,			/* pointer to comment strings */
    NULL,			/* output file */
    { 7 }			/* RGB channels only */
    /* Can't initialize the union */
};

typedef struct sv_Globals *sv_globals_type;

ATKdefineRegistry(rle, image, NULL);
void bw_m_line(unsigned char *dp,int  number);
void c_m_line(unsigned char *dp,int  number,int  line);
int rle_get_setup( sv_globals_type globals );
int rle_getrow( sv_globals_type globals, rle_pixel  *scanline[] );
static void bfill( char  *s, int  n , int  c );
static char * match( char  *n, char  *v );
char * rle_getcom( char  *name, sv_globals_type globals );
rle_pixel ** buildmap( sv_globals_type  globals, int  minmap, double  gamma );
void dithermap( int levels, double  gamma, int  rgbmap[][3], int  divN[256], int  modN[256], int  magic[16][16] );
void make_square( double  N, int  divN[256], int  modN[256], int  magic[16][16]  );
void make_magic( int  size, int  magic[16][16] );
void make_gamma( double  gamma, int  gammamap[256] );


int 
rle::Ident( const char  *fullname )
        {
  FILE *rlefile;
  int x_len,y_len;
  int rv;

  debug("rleIdent called\n");
  rlefile = fopen(fullname, "r");
  if(!rlefile) {
      perror("rleIdent: Unable to open file");
      return(0);
  }
  debug("rleIdent: Opened file ok\n");
  sv_globals.svfb_fd = rlefile;
  rv = rle_get_setup(&sv_globals);
  debug("rleIdent: got setup ok\n");
  fclose(rlefile);
  debug("rleIdent: closed file ok\n");
  switch(rv) {
    case -1:
      return 0;		/* Not rle format */
    case 0:
      /* now figure out the picture type */
      switch(sv_globals.sv_ncolors) {
	case 0:
	  perror("rleIdent: no colour channels to display");
	  return(0);
	case 1:
	  x_len = sv_globals.sv_xmax - sv_globals.sv_xmin + 1;
	  y_len = sv_globals.sv_ymax - sv_globals.sv_ymin + 1;
	  printf("%s is a %dx%d", fullname, x_len, y_len);
	  switch(sv_globals.sv_ncmap) {
	    case 0:
	      /* black and white, no map */
	      printf(" 8 bit grey scale RLE image with no map\n"); 
	      break;
	    case 1:
	      /* black and white with a map */
	      printf(" 8 bit grey scale RLE image with map\n"); 
	      break;
	    case 3:
	      /* single channel encoded colour with decoding map */
	      printf(" 8 bit color RLE image with color map\n");
	      break;
	    default:
	      perror(" 8 bit RLE image with an illegal color map\n");
	      return 0;
	    }
	  break;
	case 3:
	  x_len = sv_globals.sv_xmax - sv_globals.sv_xmin + 1;
	  y_len = sv_globals.sv_ymax - sv_globals.sv_ymin + 1;
	  printf("%s is a %dx%d", fullname, x_len, y_len);
	  switch(sv_globals.sv_ncmap) {
	    case 0:
	      printf(" 24 bit color RLE image with no map\n");
	      break;
	    case 3:
	      printf(" 24 bit color RLE image with colour map\n");
	      break;
	    default:
	      printf(" 24 bit color RLE image with an illegal color map\n");
	      return 0;
	    }
	  break;
	default:
	  x_len = sv_globals.sv_xmax - sv_globals.sv_xmin + 1;
	  y_len = sv_globals.sv_ymax - sv_globals.sv_ymin + 1;
	  printf("%s is a %dx%d", fullname, x_len, y_len);
	  printf(" RLE image with an illegal number of color planes\n");
	  return 0;
	}
      return 1;
    default:			/* Some sort of error */
      /*			perror("rleIdent");*/
      return 0;
    }
}

int
rle::Load( const char  *fullname, FILE  *fp )
            {
  int x_len, y_len;
  int i,j;
  FILE *rlefile;
  int ncol;			/* number of colors */
  int depth;
  unsigned char *bufp;
  unsigned char *buf;
  
  dith_levels = 256;	/* aim for 128 levels of each colour */
  
  debug("rleLoad called\n");
  if((rlefile = fp) == 0) {
      if (! (rlefile = fopen(fullname, "r"))) {
	  fprintf(stderr, "Couldn't open rle file %s.\n", fullname);
	  return(-1);
      }
  }
  sv_globals.svfb_fd = rlefile;
  debug("rleLoad: About to call get_setup\n");
  if(rle_get_setup( &sv_globals )) {
    if(!fp) fclose(rlefile);
    return(-1);	
  }
  
  debug("rleLoad: get_setup called ok\n");
  if(iflag == 1)	/* -i flag */
    img_gam = 1.0/img_gam;		/* convert to display gamma */
  
  /* If no image gamma on command line, check comments in file */
  if (!iflag)
    {
      char * v;
      if ( (v = rle_getcom( "image_gamma", &sv_globals )) != NULL )
	{	
	  img_gam = atof( v );
	  /* Protect against bogus information */
	  if ( img_gam == 0.0 )
	    img_gam = 1.0;
	  else
	    img_gam = 1.0 / img_gam;	/* convert to display gamma */
	}
      else if ( (v = rle_getcom( "display_gamma", &sv_globals )) != NULL )
	{
	  img_gam = atof( v );
	  /* Protect */
	  if ( img_gam == 0.0 )
	    img_gam = 1.0;
	}
    }
  
  x_len = sv_globals.sv_xmax - sv_globals.sv_xmin + 1;
  y_len = sv_globals.sv_ymax - sv_globals.sv_ymin + 1;
  
  x_min = sv_globals.sv_xmin;
  y_min = sv_globals.sv_ymin;
  
  /* fix this so that we don't waste space */
  sv_globals.sv_xmax -= sv_globals.sv_xmin;
  sv_globals.sv_xmin = 0;
  
  /* turn off the alpha channel (don't waste time and space)*/
  sv_globals.sv_alpha = 0;
  SV_CLR_BIT(sv_globals,SV_ALPHA);
  
  /* for now, force background clear */
  if(sv_globals.sv_background ==1)	/* danger setting */
    {
      debug("Forcing clear of background\n");
      sv_globals.sv_background = 2;
      if(sv_globals.sv_bg_color==0)	/* if none allocated */
	sv_globals.sv_bg_color = background;	/* use this one */
    }
  
  /* now figure out the picture type */
  switch(sv_globals.sv_ncolors)
    {
    case 0:
      perror("rleLoad: no colour channels to display");
      if(!fp) fclose(rlefile);
      return(-1);
    case 1:
      switch(sv_globals.sv_ncmap)
	{
	case 0:
	  ptype = BW_NM;	/* black and white, no map */
	  break;
	case 1:
	  ptype = BW_M;	/* black and white with a map */
	  break;
	case 3:
	  ptype = SC_M;	/* single channel encoded colour with decoding map */
	  break;
	default:
	  if(!fp) fclose(rlefile);
	  perror("rleLoad: Illegal number of maps for one colour channel");
	  return(-1);
	}
      break;
    case 3:
      switch(sv_globals.sv_ncmap)
	{
	case 0:
	  ptype = C_NM;	/* colour, no map */
	  break;
	case 3:
	  ptype = C_M;	/* colour with maps */
	  break;
	default:
	  perror("rleLoad: Illegal number of maps for colour picture");
	  if(!fp) fclose(rlefile);
	  return(-1);
	}
      break;
    default:
      perror("rleLoad: Illegal number of colour channels");
      if(!fp) fclose(rlefile);
      return(-1);
    }
  
  if(ptype==SC_M)
    {	/* don't mess with the image, but change their colour map a bit if needed */
      disp_gam /= img_gam;	/* amount to change their map */
      img_gam = 1.0;		/*  not to the image coming in */
    }
  
  /* get hold of the colour maps, and set to undo their images gamma */
  fmaps = buildmap(&sv_globals,sv_globals.sv_ncolors,img_gam);
  
  /* now we had better sort out the picture data */
  debug("done colour map\n");
  
  /* rle stufff */
  /* Get space for a full colour scan line */
  scan = (unsigned char **) malloc( (sv_globals.sv_ncolors +
				      sv_globals.sv_alpha) *
				    sizeof( unsigned char * ) );
  for ( i = 0; i < sv_globals.sv_ncolors + sv_globals.sv_alpha; i++ )
    scan[i] = (unsigned char *)malloc(x_len);
  if ( sv_globals.sv_alpha )
    scan++;
  debug("got space for get_row\n");
  
  depth = 8;		/* We always supply 8bit images */
  (this)->newRGBImage( x_len,y_len,depth);
  debug("got image structure\n");
  
  buf = (this)->Data();
  
  /* If we are going to dither - then create the dither matrix. */
  if(!bwflag && ptype!=SC_M && ptype != BW_NM && ptype != BW_M)
    {
      dith_np2 = 1;		/* allow non-power of 2 dither map size */
      dithermap( 6, disp_gam, colmap, divN, modN, magic );
    }
  
  debug("About to read image in\n");
  bufp = buf + (y_len-1) * x_len;
  for(j=y_len;j>0;j--,bufp -= x_len)
    {
      rle_getrow(&sv_globals,scan);
      switch(ptype)
	{
	case SC_M:
	  memcpy(bufp,&scan[0][0],x_len);
	  break;
	case BW_NM:
	case BW_M:
	  bw_m_line(bufp,x_len);
	  break;
	case C_NM:
	case C_M:
	  c_m_line(bufp,x_len,j);
	  break;
	}
    }
  debug("Image Read in\n");
  
  /* Deal with colour maps */
  /* set up our gamma correction */
  make_gamma(disp_gam,gammamap);	/* we'll need it */
  
  debug("Creating color map\n");
  /* now load an appropriate colour map */
  if(!bwflag && ptype==SC_M) {
      /* use their maps  & correct their gamma */
      ncol = 1<<sv_globals.sv_cmaplen;	/* number of entries */
      for(i = 0; i < ncol; i++) {
	  (this)->RedPixel( i) = gammamap[fmaps[0][i]]<<8;
	  (this)->GreenPixel( i) = gammamap[fmaps[1][i]]<<8;
	  (this)->BluePixel( i) = gammamap[fmaps[2][i]]<<8;
	}
    }
  else if(bwflag || ptype == BW_NM || ptype == BW_M) {
      /* load a black and white map (gamma corrected for this display)*/
      ncol = 256;			/* don't know whats been used */
      for(i = 0; i < ncol; i++) {
	  (this)->RedPixel( i) = 
	    (this)->GreenPixel( i) = 
	    (this)->BluePixel( i) = gammamap[i]<<8;
	}
    }
  else {
      /* must be colour, so use dither map (gamma corrected for this display)*/
      ncol = 6*6*6;
      /* Dither map has already been created above */
      for(i = 0; i < ncol; i++) {
	  (this)->RedPixel( i) = colmap[i][0]<<8;
	  (this)->GreenPixel( i) = colmap[i][1]<<8;
	  (this)->BluePixel( i) = colmap[i][2]<<8;
	}
  }
  (this)->RGBUsed() = ncol;

  if(!fp) fclose(rlefile);
  debug("finished\n");
  return(0);
}

#define DMAP(v,x,y)	(modN[v]>magic[x][y] ? divN[v] + 1 : divN[v])

/* run the black and white through its map */
void bw_m_line(unsigned char *dp,int  number)
          {
  unsigned char *r;
  int i;
  
  for(i=number,r= &scan[0][0];i>0;i--,r++,dp++)
    {
      *dp = fmaps[0][*r];
    }
}

/* convert a colour line with map to 8 bits per pixel */
void c_m_line(unsigned char *dp,int  number,int  line)
          {
  unsigned char *r, *g, *b;
  int i, col, row;
  
  if(!bwflag)
    {		
      for ( row = line % dith_size, col = x_min % dith_size, i = number, r = &scan[0][0]
	   ,g= &scan[1][0], b= &scan[2][0];
	   i > 0; i--, r++, g++, b++, dp++, col = ((col + 1) % dith_size) )
	{
	  *dp = DMAP(fmaps[0][*r], col, row) +
	    DMAP(fmaps[1][*g], col, row) * 6 +
	      DMAP(fmaps[2][*b], col, row) * 36;
	}
    }
  else
    {
      int red,green,blue;
      for (i = number, r= &scan[0][0], g= &scan[1][0]
	   ,b= &scan[2][0]; i>0;i--,r++,g++,b++,dp++)
	{
	  red = fmaps[0][*r];green=fmaps[1][*g];blue = fmaps[2][*b];
	  *dp = 0.35* red + 0.55* green + 0.1* blue;
	}
    }
}

/*
 * Automatically define LITTLE_ENDIAN on vax and pdp11 machines
 */
#if defined(vax) || defined(pdp11)
#define	LITTLE_ENDIAN
#endif

struct inst {
  unsigned opcode:8, datum:8;
};

#define BREAD(type, var, len)\
	    fread((byte *)&var, 1, len, infile)
#define OPCODE(inst) (inst.opcode & ~LONG)
#define LONGP(inst) (inst.opcode & LONG)
#define DATUM(inst) (0x00ff & inst.datum)

/*****************************************************************
 * TAG( rle_get_setup )
 * 
 * Read the initialization information from an RLE file.
 * Inputs:
 * 	globals:    Contains pointer to the input file.
 * Outputs:
 * 	globals:    Initialized with information from the
 *		    input file.
 *	Returns 0 on success, -1 if the file is not an RLE file,
 *	-2 if malloc of the color map failed, -3 if an immediate EOF
 *	is hit (empty input file), and -4 if an EOF is encountered reading
 *	the setup information.
 * Assumptions:
 * 	infile points to the "magic" number in an RLE file (usually
 * byte 0 in the file).
 * Algorithm:
 * 	Read in the setup info and fill in sv_globals.
 */
int rle_get_setup( sv_globals_type  globals )
{
    struct XtndRsetup setup;
    short magic;			/* assume 16 bits */
    FILE *infile = globals->svfb_fd;
    rle_pixel * bg_color;
    int i;
    char * comment_buf;

    BREAD( short, magic, sizeof magic );
    SWAB(magic);
    if ( feof( infile ) )
	return -3;
    if ( magic != XtndRMAGIC )
	return -1;
    BREAD( struct XtndRsetup, setup, SETUPSIZE );  /* assume VAX packing */
    if ( feof( infile ) )
	return -4;
    SWAB( setup.h_xpos );
    SWAB( setup.h_ypos );
    SWAB( setup.h_xlen );
    SWAB( setup.h_ylen );

    /* Extract information from setup */
    globals->sv_ncolors = setup.h_ncolors;
    for ( i = 0; i < globals->sv_ncolors; i++ )
	SV_SET_BIT( *globals, i );

    if ( !(setup.h_flags & H_NO_BACKGROUND) )
    {
	globals->sv_bg_color = (int *)malloc(
	    (unsigned)(sizeof(int) * setup.h_ncolors) );
	bg_color = (rle_pixel *)malloc(
	    (unsigned)(1 + (setup.h_ncolors / 2) * 2) );
	fread((byte *)bg_color, 1, 1 + (setup.h_ncolors / 2) * 2, infile);
	for ( i = 0; i < setup.h_ncolors; i++ )
	    globals->sv_bg_color[i] = bg_color[i];
	free( bg_color );
    }
    else
	fgetc( infile );			/* skip filler byte */

    if ( setup.h_flags & H_NO_BACKGROUND )
	globals->sv_background = 0;
    else if ( setup.h_flags & H_CLEARFIRST )
	globals->sv_background = 2;
    else
	globals->sv_background = 1;
    if ( setup.h_flags & H_ALPHA )
    {
	globals->sv_alpha = 1;
	SV_SET_BIT( *globals, SV_ALPHA );
    }
    else
	globals->sv_alpha = 0;

    globals->sv_xmin = setup.h_xpos;
    globals->sv_ymin = setup.h_ypos;
    globals->sv_xmax = globals->sv_xmin + setup.h_xlen - 1;
    globals->sv_ymax = globals->sv_ymin + setup.h_ylen - 1;

    globals->sv_ncmap = setup.h_ncmap;
    globals->sv_cmaplen = setup.h_cmaplen;
    if ( globals->sv_ncmap > 0 )
    {
	int maplen =
		     globals->sv_ncmap * (1 << globals->sv_cmaplen);
	globals->sv_cmap = (rle_map *)malloc(
	    (unsigned)(sizeof(rle_map) * maplen) );
	if ( globals->sv_cmap == NULL )
	{
	    fprintf( stderr,
		"Malloc failed for color map of size %d*%d in rle_get_setup\n",
		globals->sv_ncmap, (1 << globals->sv_cmaplen) );
	    return -2;
	}
	fread((byte *)globals->sv_cmap, 1, sizeof(short) * maplen, infile);
#ifndef LITTLE_ENDIAN
    	/* Swap bytes on bigendian machines
	 */
    	for ( i = 0; i < maplen; i++ )
    	    SWAB( globals->sv_cmap[i] );
#endif
    }

    /* Check for comments */
    if ( setup.h_flags & H_COMMENT )
    {
	short comlen, evenlen;
	char * cp;

	BREAD( short, comlen, sizeof comlen );	/* get comment length */
	SWAB( comlen );
	evenlen = (comlen + 1) & ~1;	/* make it even */
	comment_buf = (char *)malloc( (unsigned) evenlen );
	if ( comment_buf == NULL )
	{
	    fprintf( stderr,
		     "Malloc failed for comment buffer of size %d in rle_get_setup\n",
		     comlen );
	    return -2;
	}
	fread((byte *)comment_buf, 1, evenlen,  infile);
	/* Count the comments */
	for ( i = 0, cp = comment_buf; cp < comment_buf + comlen; cp++ )
	    if ( *cp == 0 )
		i++;
	i++;			/* extra for NULL pointer at end */
	/* Get space to put pointers to comments */
	globals->sv_comments =
	    (char **)malloc( (unsigned)(i * sizeof(char *)) );
	if ( globals->sv_comments == NULL )
	{
	    fprintf( stderr,
		    "Malloc failed for %d comment pointers in rle_get_setup\n",
		     i );
	    return -2;
	}
	/* Get pointers to the comments */
	*globals->sv_comments = comment_buf;
	for ( i = 1, cp = comment_buf + 1; cp < comment_buf + comlen; cp++ )
	    if ( *(cp - 1) == 0 )
		globals->sv_comments[i++] = cp;
	globals->sv_comments[i] = NULL;
    }
    else
	globals->sv_comments = NULL;

    /* Initialize state for rle_getrow */
    globals->sv_private.get.scan_y = globals->sv_ymin;
    globals->sv_private.get.vert_skip = 0;
    globals->sv_private.get.is_eof = 0;
    globals->sv_private.get.is_seek = 0;	/* Can't do seek on file */

    if ( !feof( infile ) )
	return 0;			/* success! */
    else
    {
	globals->sv_private.get.is_eof = 1;
	return -4;
    }
}

/*****************************************************************
 * TAG( rle_getrow )
 * 
 * Get a scanline from the input file.
 * Inputs:
 *	globals:    sv_globals structure containing information about 
 *		    the input file.
 * Outputs:
 * 	scanline:   an array of pointers to the individual color
 *		    scanlines.  Scanline is assumed to have
 *		    globals->sv_ncolors pointers to arrays of rle_pixel,
 *		    each of which is at least globals->sv_xmax+1 long.
 *	Returns the current scanline number.
 * Assumptions:
 * 	rle_get_setup has already been called.
 * Algorithm:
 * 	If a vertical skip is being executed, and clear-to-background is
 *	specified (globals->sv_background is true), just set the
 *	scanlines to the background color.  If clear-to-background is
 *	not set, just increment the scanline number and return.
 * 
 *	Otherwise, read input until a vertical skip is encountered,
 *	decoding the instructions into scanline data.
 */

int rle_getrow( sv_globals_type  globals, rle_pixel  *scanline[] )
{
    rle_pixel * scanc;
    int nc;
    FILE *infile = globals->svfb_fd;
    int scan_x = globals->sv_xmin,	/* current X position */
	   channel = 0;			/* current color channel */
    short word, long_data;
    struct inst inst;

    /* Clear to background if specified */
    if ( globals->sv_background == 2 )
    {
	if ( globals->sv_alpha && SV_BIT( *globals, -1 ) )
	    bfill( (char *)scanline[-1], globals->sv_xmax + 1, 0 );
	for ( nc = 0; nc < globals->sv_ncolors; nc++ )
	    if ( SV_BIT( *globals, nc ) )
		bfill( (char *)scanline[nc], globals->sv_xmax+1,
			globals->sv_bg_color[nc] );
    }

    /* If skipping, then just return */
    if ( globals->sv_private.get.vert_skip > 0 )
    {
	globals->sv_private.get.vert_skip--;
	globals->sv_private.get.scan_y++;
	if ( globals->sv_private.get.vert_skip > 0 )
	    return globals->sv_private.get.scan_y;
    }

    /* If EOF has been encountered, return also */
    if ( globals->sv_private.get.is_eof )
	return ++globals->sv_private.get.scan_y;

    /* Otherwise, read and interpret instructions until a skipLines
     * instruction is encountered.
     */
    if ( SV_BIT( *globals, channel ) )
	scanc = scanline[channel] + scan_x;
    else
	scanc = NULL;
    for (;;)
    {
        BREAD(struct inst, inst, 2 );
	if ( feof(infile) )
	{
	    globals->sv_private.get.is_eof = 1;
	    break;		/* <--- one of the exits */
	}

	switch( OPCODE(inst) )
	{
	case RSkipLinesOp:
	    if ( LONGP(inst) )
	    {
	        BREAD( short, long_data, sizeof long_data );
		SWAB( long_data );
		globals->sv_private.get.vert_skip = long_data;
	    }
	    else
		globals->sv_private.get.vert_skip = DATUM(inst);

	    break;			/* need to break for() here, too */

	case RSetColorOp:
	    channel = DATUM(inst);	/* select color channel */
	    if ( channel == 255 )
		channel = -1;
	    scan_x = globals->sv_xmin;
	    if ( SV_BIT( *globals, channel ) )
		scanc = scanline[channel]+scan_x;
	    break;

	case RSkipPixelsOp:
	    if ( LONGP(inst) )
	    {
	        BREAD( short, long_data, sizeof long_data );
		SWAB( long_data );
		scan_x += long_data;
		scanc += long_data;
			 
	    }
	    else
	    {
		scan_x += DATUM(inst);
		scanc += DATUM(inst);
	    }
	    break;

	case RByteDataOp:
	    if ( LONGP(inst) )
	    {
	        BREAD( short, long_data, sizeof long_data );
		SWAB( long_data );
		nc = (int)long_data;
	    }
	    else
		nc = DATUM(inst);
	    nc++;
	    if ( SV_BIT( *globals, channel ) )
	    {
		fread((byte *)scanc, 1, nc, infile);
		if ( nc & 1 )
		    (void)fgetc( infile );	/* throw away odd byte */
	    }
	    else
		{		/* Emulate a forward fseek */
		    int ii;
		    for ( ii = ((nc + 1) / 2) * 2; ii > 0; ii-- )
			(void) fgetc( infile );	/* discard it */
		}

	    scanc += nc;
	    scan_x += nc;
	    break;

	case RRunDataOp:
	    if ( LONGP(inst) )
	    {
	        BREAD( short, long_data, sizeof long_data );
		SWAB( long_data );
		nc = long_data;
	    }
	    else
		nc = DATUM(inst);
	    scan_x += nc + 1;

	    BREAD( short, word, sizeof(short) );
	    SWAB( word );
	    if ( SV_BIT( *globals, channel ) )
	    {
		if ( nc >= 10 )		/* break point for 785, anyway */
		{
		    bfill( (char *)scanc, nc + 1, word );
		    scanc += nc + 1;
		}
		else
		    for ( ; nc >= 0; nc--, scanc++ )
			*scanc = word;
	    }
	    break;

	case REOFOp:
	    globals->sv_private.get.is_eof = 1;
	    break;

	default:
	    fprintf( stderr,
		     "rle_getrow: Unrecognized opcode: %d\n", inst.opcode );
	    exit(1);
	}
	if ( OPCODE(inst) == RSkipLinesOp || OPCODE(inst) == REOFOp )
	    break;			/* <--- the other loop exit */
    }

    return globals->sv_private.get.scan_y;
}


/* Fill buffer at s with n copies of character c.  N must be <= 65535*/
/* ARGSUSED */
static void bfill( char  *s, int  n , int  c )
{
#ifdef vax
    asm_c("   movc5   $0,*4(ap),12(ap),8(ap),*4(ap)");
#else
    while ( n-- > 0 )
	*s++ = c;
#endif
}

/*****************************************************************
 * TAG( match )
 * 
 * Match a name against a test string for "name=value" or "name".
 * If it matches name=value, return pointer to value part, if just
 * name, return pointer to NUL at end of string.  If no match, return NULL.
 *
 * Inputs:
 * 	n:	Name to match.  May also be "name=value" to make it easier
 *		to replace comments.
 *	v:	Test string.
 * Outputs:
 * 	Returns pointer as above.
 * Assumptions:
 *	[None]
 * Algorithm:
 *	[None]
 */
static char *
match( char  *n, char  *v )
{
    for ( ; *n != '\0' && *n != '=' && *n == *v; n++, v++ )
	;
    if (*n == '\0' || *n == '=')
	if ( *v == '\0' )
	    return v;
	else if ( *v == '=' )
	    return ++v;

    return NULL;
}

/*****************************************************************
 * TAG( rle_getcom )
 * 
 * Return a pointer to the value part of a name=value pair in the comments.
 * Inputs:
 * 	name:		Name part of the comment to search for.
 *	globals:	sv_globals structure.
 * Outputs:
 * 	Returns pointer to value part of comment or NULL if no match.
 * Assumptions:
 *	[None]
 * Algorithm:
 *	[None]
 */
char *
rle_getcom( char  *name, sv_globals_type  globals )
{
    char ** cp;
    char * v;

    if ( globals->sv_comments == NULL )
	return NULL;

    for ( cp = globals->sv_comments; *cp; cp++ )
	if ( (v = match( name, *cp )) != NULL )
	    return v;

    return NULL;
}

/* 
 * buildmap.c - Build a color map from the RLE file color map.
 * 
 * Author:	Spencer W. Thomas
 * 		Computer Science Dept.
 * 		University of Utah
 * Date:	Sat Jan 24 1987
 * Copyright (c) 1987, University of Utah
 */

/*****************************************************************
 * TAG( buildmap )
 * 
 * Returns a color map that can easily be used to map the pixel values in
 * an RLE file.  Map is built from the color map in the input file.
 * Inputs:
 * 	globals:	sv_globals structure containing color map.
 *	minmap:		Minimum number of channels in output map.
 *	gamma:		Adjust color map for this image gamma value
 *			(1.0 means no adjustment).
 * Outputs:
 * 	Returns an array of pointers to arrays of rle_pixels.  The array
 *	of pointers contains max(sv_ncolors, sv_ncmap) elements, each 
 *	array of pixels contains 2^sv_cmaplen elements.  The pixel arrays
 *	should be considered read-only.
 * Assumptions:
 * 	[None]
 * Algorithm:
 *	Ensure that there are at least sv_ncolors rows in the map, and
 *	that each has at least 256 elements in it (largest map that can
 *	be addressed by an rle_pixel).
 */
rle_pixel **
buildmap( sv_globals_type  globals, int  minmap, double  gamma )
{
    rle_pixel ** cmap, * gammap;
    int i, j;
    int maplen, cmaplen, ncmap, nmap;

    if ( globals->sv_ncmap == 0 )	/* make identity map */
    {
	nmap = (minmap < globals->sv_ncolors) ? globals->sv_ncolors : minmap;
	cmap = (rle_pixel **)malloc( nmap * sizeof(rle_pixel *) );
	cmap[0] = (rle_pixel *)malloc( 256 * sizeof(rle_pixel) );
	for ( i = 0; i < 256; i++ )
	    cmap[0][i] = i;
	for ( i = 1; i < nmap; i++ )
	    cmap[i] = cmap[0];
	maplen = 256;
	ncmap = 1;		/* number of unique rows */
    }
    else			/* make map from globals */
    {
	/* Map is at least 256 long */
	cmaplen = (1 << globals->sv_cmaplen);
	if ( cmaplen < 256 )
	    maplen = 256;
	else
	    maplen = cmaplen;

	if ( globals->sv_ncmap == 1 )	/* make "b&w" map */
	{
	    nmap = (minmap < globals->sv_ncolors) ?
		globals->sv_ncolors : minmap;
	    cmap = (rle_pixel **)malloc( nmap * sizeof(rle_pixel *) );
	    cmap[0] = (rle_pixel *)malloc( maplen * sizeof(rle_pixel) );
	    for ( i = 0; i < maplen; i++ )
		if ( i < cmaplen )
		    cmap[0][i] = globals->sv_cmap[i] >> 8;
		else
		    cmap[0][i] = i;
	    for ( i = 1; i < nmap; i++ )
		cmap[i] = cmap[0];
	    ncmap = 1;
	}
	else if ( globals->sv_ncolors <= globals->sv_ncmap )
	{
	    nmap = (minmap < globals->sv_ncmap) ? globals->sv_ncmap : minmap;
	    cmap = (rle_pixel **)malloc( nmap * sizeof(rle_pixel *) );
	    for ( j = 0; j < globals->sv_ncmap; j++ )
	    {
		cmap[j] = (rle_pixel *)malloc( maplen * sizeof(rle_pixel) );
		for ( i = 0; i < maplen; i++ )
		    if ( i < cmaplen )
			cmap[j][i] = globals->sv_cmap[j*cmaplen + i] >> 8;
		    else
			cmap[j][i] = i;
	    }
	    for ( i = j, j--; i < nmap; i++ )
		cmap[i] = cmap[j];
	    ncmap = globals->sv_ncmap;
	}
	else			/* ncolors > ncmap */
	{
	    nmap = (minmap < globals->sv_ncolors) ?
		globals->sv_ncolors : minmap;
	    cmap = (rle_pixel **)malloc( nmap * sizeof(rle_pixel *) );
	    for ( j = 0; j < globals->sv_ncmap; j++ )
	    {
		cmap[j] = (rle_pixel *)malloc( maplen * sizeof(rle_pixel) );
		for ( i = 0; i < maplen; i++ )
		    if ( i < cmaplen )
			cmap[j][i] = globals->sv_cmap[j*cmaplen + i] >> 8;
		    else
			cmap[j][i] = i;
	    }
	    for( i = j, j--; i < nmap; i++ )
		cmap[i] = cmap[j];
	    ncmap = globals->sv_ncmap;
	}
    }
	    
    /* Gamma compensate if requested */
    if ( gamma != 1.0 )
    {
	gammap = (rle_pixel *)malloc( 256 * sizeof(rle_pixel) );
	for ( i = 0; i < 256; i++ )
		{
#ifdef BYTEBUG
		int byteb1;
		byteb1 = (int)(0.5 + 255.0 * pow( i / 255.0, gamma ));
		gammap[i] = byteb1;
#else
	    gammap[i] = (int)(0.5 + 255.0 * pow( i / 255.0, gamma ));
#endif
		}
	for ( i = 0; i < ncmap; i++ )
	    for ( j = 0; j < maplen; j++ )
		cmap[i][j] = gammap[cmap[i][j]];
    }

    return cmap;
}

/* dither globals */
int dith_levels = 128;
int dith_np2 = 0;
int dith_size = 16;

/* basic dithering macro */
#define DMAP(v,x,y)	(modN[v]>magic[x][y] ? divN[v] + 1 : divN[v])

/*****************************************************************
 * TAG( dithermap )
 * 
 * Create a color dithering map with a specified number of intensity levels.
 * Inputs:
 * 	levels:		Intensity levels per primary.
 *	gamma:		Display gamma value.
 * Outputs:
 * 	rgbmap:		Generated color map.
 *	divN:		"div" function for dithering.
 *	modN:		"mod" function for dithering.
 * Assumptions:
 * 	rgbmap will hold levels^3 entries.
 * Algorithm:
 *	Compute gamma compensation map.
 *	N = 255.0 / (levels - 1) is number of pixel values per level.
 *	Compute rgbmap with red ramping fastest, green slower, and blue
 *	slowest (treat it as if it were rgbmap[levels][levels][levels][3]).
 *	Call make_square to get divN, modN, and magic
 *
 * Note:
 *	Call dithergb( x, y, r, g, b, levels, divN, modN, magic ) to get index
 *	into rgbmap for a given color/location pair, or use
 *	    row = y % 16; col = x % 16;
 *	    DMAP(v,col,row) =def (divN[v] + (modN[v]>magic[col][row] ? 1 : 0))
 *	    DMAP(r,col,row) + DMAP(g,col,row)*levels + DMAP(b,col,row)*levels^2
 *	if you don't want function call overhead.
 */
void
dithermap( int levels, double  gamma, int  rgbMap[][3], int  divN[256], int  modN[256], int  magic[16][16] )
{
    double N;
    int i;
    int levelsq, levelsc;
    int gammamap[256];
    
    make_gamma(gamma, gammamap);

    levelsq = levels*levels;	/* squared */
    levelsc = levels*levelsq;	/* and cubed */

    N = 255.0 / (levels - 1);    /* Get size of each step */

    /* 
     * Set up the color map entries.
     */
    for(i = 0; i < levelsc; i++) {
	rgbMap[i][0] = gammamap[(int)(0.5 + (i%levels) * N)];
	rgbMap[i][1] = gammamap[(int)(0.5 + ((i/levels)%levels) * N)];
	rgbMap[i][2] = gammamap[(int)(0.5 + ((i/levelsq)%levels) * N)];
    }

    make_square( N, divN, modN, magic );
}

/*****************************************************************
 * TAG( make_square )
 * 
 * Build the magic square for a given number of levels.
 * Inputs:
 * 	N:		Pixel values per level (255.0 / (levels-1)).
 * (global) dith_levels = 128 (default) - number of effective levels to aim for 
 * (global) dith_np2 = 0 (default) - non-zero if non power of two size is permissable.
 * Outputs:
 * 	divN:		Integer value of pixval / N
 *	modN:		Integer remainder between pixval and divN[pixval]*N
 *	magic:		Magic square for dithering to N sublevels.
 * (global) dith_size = magic square size chosen.
 * Assumptions:
 * 	
 * Algorithm:
 *	divN[pixval] = (int)(pixval / N) maps pixval to its appropriate level.
 *	modN[pixval] = pixval - (int)(N * divN[pixval]) maps pixval to
 *	its sublevel, and is used in the dithering computation.
 */
void
make_square( double  N, int  divN[256], int  modN[256], int  magic[16][16]  )
{
    int i, j, k, l;
    double magicfact;

    for ( i = 0; i < 256; i++ )
    {
	divN[i] = (int)(i / N);
	modN[i] = i - (int)(N * divN[i]);
    }
    modN[255] = 0;		/* always */

	/* figure out how big a square will give */
	/* the desired number of levels */
	if(dith_np2)
		for(dith_size= 2;((dith_size * dith_size)+1)<((N*dith_levels)/256);dith_size++ );
	else
		for(dith_size= 2;((dith_size * dith_size)+1)<((N*dith_levels)/256);dith_size *=2);

	/* make the basic square up */
	/* ( will have numbers 0 - size * size ) */
	make_magic(dith_size,magic);
	
	/* divN gives 0 - levels-1 */
	/* modN gives 0 - N-1 */
	/* dither is if(modN(pix) > magic[][] so */
	/* scale magic it to have levels 0 to N-2 */
	/* (ie takes account of magic square size allows size * size +1 levels */

	magicfact = (N-2)/((double)((dith_size * dith_size)-1));
	for(i=0;i<dith_size;i++)
	{
		for(j=0;j<dith_size;j++)
		{
			magic[i][j] = (int)(0.5 + magic[i][j] * magicfact);
		}
	}

	if(!dith_np2)	/* if we have power of 2 */
	{
		/* now replicate the size square we've chosen */  
		/* (and use a brick pattern) */
		for(k=0;k<16;k += dith_size)
		{
			for(l=k>0?0:dith_size;l<16;l += dith_size)
			{
				for(i=0;i<dith_size;i++)
				{
					for(j=0;j<dith_size;j++)
					{
						magic[k+i][((l+k/2)+j)%16] = magic[i][j];
					}
				}
			}
		}
	}
}

int magic16x16[16][16] = 
	{
		{0,128,32,160,8,136,40,168,2,130,34,162,10,138,42,170},
		{192,64,224,96,200,72,232,104,194,66,226,98,202,74,234,106},
		{48,176,16,144,56,184,24,152,50,178,18,146,58,186,26,154},
		{240,112,208,80,248,120,216,88,242,114,210,82,250,122,218,90},
		{12,140,44,172,4,132,36,164,14,142,46,174,6,134,38,166},
		{204,76,236,108,196,68,228,100,206,78,238,110,198,70,230,102},
		{60,188,28,156,52,180,20,148,62,190,30,158,54,182,22,150},
		{252,124,220,92,244,116,212,84,254,126,222,94,246,118,214,86},
		{3,131,35,163,11,139,43,171,1,129,33,161,9,137,41,169},
		{195,67,227,99,203,75,235,107,193,65,225,97,201,73,233,105},
		{51,179,19,147,59,187,27,155,49,177,17,145,57,185,25,153},
		{243,115,211,83,251,123,219,91,241,113,209,81,249,121,217,89},
		{15,143,47,175,7,135,39,167,13,141,45,173,5,133,37,165},
		{207,79,239,111,199,71,231,103,205,77,237,109,197,69,229,101},
		{63,191,31,159,55,183,23,151,61,189,29,157,53,181,21,149},
		{255,127,223,95,247,119,215,87,253,125,221,93,245,117,213,85}
	};


/*****************************************************************
 * TAG( make_magic )
     * 
 * Create the magic square.
 * Inputs:
 * 	size:		Order of the square
 *  magic:		Address of 16 x 16 magic square.
 * Outputs:
 * 	Fills in the 16 x 16 magic square.
 * Assumptions:
 * 	size is between 2 and 16
 * Algorithm:
 * 	Chose sub cell of 16 by 16 magic square
     */
void make_magic( int  size, int  magic[16][16] )
{
	int j,i,li,bi,bx,by;
	int xx,yy;
	int total;

	total = size * size;

	i = 0;
	li = -1;
	for(j=0;j<total;j++)	
	{
		bi = 256;

		for(xx=0;xx<size;xx++)
		{
			for(yy=0;yy<size;yy++)
			{
				if(magic16x16[xx][yy] >li && magic16x16[xx][yy] < bi)
				{
					bx = xx;
					by = yy;
					bi = magic16x16[xx][yy];
				}
			}
		}
		magic[bx][by] = i;
		i++;
		li = bi;
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
void make_gamma( double  gamma, int  gammamap[256] )
{
	int i;

    for ( i = 0; i < 256; i++ )
		{
#ifdef BYTEBUG
		int byteb1;
		
		byteb1 = (int)(0.5 + 255 * pow( i / 255.0, 1.0/gamma ));
		gammamap[i] = byteb1;
#else
		gammamap[i] = (int)(0.5 + 255 * pow( i / 255.0, 1.0/gamma ));
#endif
		}
}

long
rle::Read( FILE  *file, long  id )
            {
    if((this)->Load( NULL, file) == 0)
	return(dataobject_NOREADERROR);
    else
	return(dataobject_BADFORMAT);
}

long
rle::Write( FILE  *file, long  writeID, int  level )
                {
    return((this)->image::Write( file, writeID, level));
}

long
rle::WriteNative( FILE  *file, const char  *filename )
            {
return(0);
}
