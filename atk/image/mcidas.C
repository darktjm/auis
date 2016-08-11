/* mcidas.ch - class description for interface from MCIDAS format to image */
/*
	Copyright Carnegie Mellon University 1992 - All rights reserved
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

/*
 * McIDAS areafile support.  contributed by Glenn P. Davis
 * (davis@unidata.ucar.edu).
 */

#include <andrewos.h>
ATK_IMPL("mcidas.H")
#include <image.H>
/*#include <mcidas.h>*/
/* Argh, looks like the numbers are in vax byte order */

#define TYPELEN		4 /* Short strings used as identifiers */
#define COMMENTLEN	32	/* longer strings */
/*
 * McIdas AREA DIRECTORY, based on documentation dated 5/87 by R. Dengal
 */
struct area_dir {
/*  1 */ unsigned long	status ;
/*  2 */ unsigned long	type ;
/*  3 */ unsigned long	satid ;
/*  4 */ unsigned long	ndate ; /* YYDDD */
/*  5 */ unsigned long	ntime ; /* HHMMSS */
/*  6 */ unsigned long	lcor ;
/*  7 */ unsigned long	ecor ;
/*  8 */ unsigned long	zcor ;
/*  9 */ unsigned long	lsiz ;
/* 10 */ unsigned long	esiz ;
/* 11 */ unsigned long	zsiz ;
/* 12 */ unsigned long	lres ;
/* 13 */ unsigned long	eres ;
/* 14 */ unsigned long	bands ;
/* 15 */ unsigned long	yzprefix ;
/* 16 */ unsigned long	projnum ;
/* 17 */ unsigned long	cdate ;
/* 18 */ unsigned long	ctime ;
/* 19 */ unsigned long	filtmap ;
/* 20 */ unsigned long	imageid ;
/* 21 */ unsigned long	resvid[4] ;
#define AREA_COMMENTS 24
/* 25 */ char	comments[COMMENTLEN] ;
#define AREA_CALKEY 32
/* 33 */ unsigned long	calkey ;
/* 34 */ unsigned long	navkey ;
/* 35 */ unsigned long	navkey2 ;
/* 36 */ unsigned long	lprefix ;
/* 37 */ unsigned long	pdl[8] ;
/* 45 */ unsigned long	band8 ;
/* 46 */ unsigned long idate ;
/* 47 */ unsigned long itime ;
/* 48 */ unsigned long	startscan ;
/* 49 */ unsigned long	doclen ;
/* 50 */ unsigned long	callen ;
/* 51 */ unsigned long	levlen ;
#define AREA_STYPE 51
/* 52 */ char	stype[TYPELEN] ;
/* 53 */ char	ctype[TYPELEN] ;
/* 54 */ unsigned long	reserved[11] ;
} ;

/*
 * McIdas NAVIGATION CODICIL, based on documentation dated 5/87 by D. Santek
 *  Only type 'GOES' used here currently
 */
struct navigation {
/*   1 */ char	type[TYPELEN] ;
#define NAV_DATA 1
/*   2 */ unsigned long iddate ;
/*   3 */ unsigned long itime ;
/*   4 */ unsigned long fill[37] ; /* expand this later, if needed */
#define NAV_RESERVED 40
/*  41 */ unsigned long	reserved[80] ;
/* 121 */ char	memo[COMMENTLEN] ;
} ;

struct mc_area {
	struct area_dir *dir ;
	struct navigation *nav ;
	unsigned char *image ;
	unsigned char *priv ; /* conveninence pointer */
} ;

#include <mcidas.H>

/*
 * convert from little endian to big endian four byte object
 */

ATKdefineRegistry(mcidas, image, NULL);
static unsigned long vhtonl(unsigned long  lend );


static unsigned long
vhtonl(unsigned long  lend )
{
	unsigned long bend ;
	unsigned char *lp, *bp ;

	lp = ((unsigned char *)&lend) + 3 ;
	bp = (unsigned char *) &bend ;

	*bp++ = *lp-- ;
	*bp++ = *lp-- ;
	*bp++ = *lp-- ;
	*bp = *lp ;

	return(bend) ;
}


/* ARGSUSED */
int 
mcidas::Ident( char  *fullname )
        { FILE          *f;
  struct area_dir dir ;
  int             r;

  if(!(f = fopen(fullname,"r"))) {
    perror("mcidasIdent");
    return(0);
  }
  switch (fread((byte *)&dir, sizeof(struct area_dir), 1, f)) {
      case -1:
	  r = 0;
	  break;

      case 1:
	  if (dir.type != 4 && dir.type != 67108864) {
	      r = 0;
	      break;
	  }
	  r = 1;
	  break;

      default:
	  r = 0;
	  break;
  }
  fclose(f);
  return(r);
}


int
mcidas::Load( char  *fullname, FILE  *fp )
            { FILE          *f;
  struct area_dir  dir;
  struct navigation  nav;
  unsigned int    y;
  int doswap = 0 ;

  if((f = fp) == 0) {
      if (! (f = fopen(fullname, "r"))) {
	  fprintf(stderr, "Couldn't open mcidas file %s.\n", fullname);
	  return(-1);
      }
  }
  switch (fread((byte *)&dir, sizeof(struct area_dir), 1, f)) {
      case -1:
	  perror("mcidasLoad");
	  fclose(f);
	  return(-1);

      case 1:
	  if(dir.type != 4) {
	      if(dir.type != 67108864) {
		  fclose(f);
		  return(-1);
	      } else {
		  doswap = 1;
	      }
	  }
	  break;

      default:
	  fclose(f);
	  return(-1);
  }

  if(doswap) {
    unsigned long *begin; 
    unsigned long *ulp;
    begin = (unsigned long*) &dir;
    for(ulp = begin; ulp < &begin[AREA_COMMENTS]; ulp++)
       *ulp = vhtonl(*ulp);
     for(ulp = &begin[AREA_CALKEY]; ulp < &begin[AREA_STYPE]; ulp++)
        *ulp = vhtonl(*ulp);
   }

  /* skip the nav */
  if (fread((byte*) &nav, sizeof(struct navigation), 1, f) != 1) {
      fclose(f);
      return(-1);
  }

  /* get an image to put the data in
   */

   (this)->newRGBImage(
		      dir.esiz,
		      dir.lsiz,
		      8 * dir.bands);

  /* set up the colormap, linear grey scale
   */

    for (y = 0; y < 255; y++) {
	(this)->RedPixel( y) = 
	  (this)->RedPixel( y) =
	  (this)->RedPixel( y)  = y * 257;
    }
    (this)->RGBUsed() = 255;

  fread((this)->Data(), sizeof(byte), dir.esiz * dir.lsiz, f);

  fclose(f);
  return(0);
}

long
mcidas::Read( FILE  *file, long  id )
            {
    if((this)->Load( NULL, file) == 0)
	return(dataobject_NOREADERROR);
    else
	return(dataobject_BADFORMAT);
}

long
mcidas::Write( FILE  *file, long  writeID, int  level )
                {
    return((this)->image::Write( file, writeID, level));
}

long
mcidas::WriteNative( FILE  *file, char  *filename )
            {
return(0);
}
