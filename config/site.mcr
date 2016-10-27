/* 
	Copyright Carnegie Mellon, 1996 - All rights reserved

	Template for site.mcr file

	This file is included only in Makefiles, so variables
	defined here are accessible only from Makefiles.

	DO NOT PUT A TAB BEFORE THE VARIABLE NAME.

*/


/* XINCDIR must be the parent of the X11 include directory.
	That is $(XINCDIR)/X11/ contains X.h 
	and the other X include files. 
 XLIBDIR is the directory containing libX11.a 
 XUTILDIR is the directory containing imake, makedepend,
	xmkfontdir, bdftopcf, and bdftosnf.  If these are in different
	directories, set them individually below
 on Sun platforms, these are often found in /usr/openwin 
The values shown here are the defaults. */
/*
    XINCDIR = /usr/include
    XLIBDIR = /usr/lib
*/
    XUTILDIR = /usr/bin
/* if a utility is not in XUTILDIR, give its full pathname
	(or have it on your path 
	and set the variable to the utility name) 
	The values given are the defaults */
/*
    IMAKE = $(XUTILDIR)/imake
    XMAKEDEPEND = $(XUTILDIR)/makedepend
    XMKFONTDIR = $(XUTILDIR)/mkfontdir
*/
/* XFC:  allsys.mcr determines which font compiler to use based 
 on other variables.  The font compiler is assumed to be in 
 XUTILDIR as either bdftopcf or bdftosnf.
 If necessary, you can override the the choice by setting XFC */

/* CDEBUGFLAGS are passed to C compilations.  */
/* To generate debugger symbol tables, use -g instead of -O.  */
COPT = -O2
        CDEBUGFLAGS = -g3 $(COPT) -fPIC -D_XOPEN_SOURCE -D_DEFAULT_SOURCE -Wmissing-declarations -Wold-style-definition -Wextra -Wno-unused-parameter -Wmissing-declarations -Wno-unused-result -Wno-missing-field-initializers -Wwrite-strings -march=athlon64 -msse3 -msse3 -pipe -Wall

/* Check and Set the RESOLVELIB to the full path name
	of libresolv.a.  It may be in the 4.3 distribution, the
	bind distribution, or elsewhere.
	If the resolver library is in libc.a,  set RESOLVLIB to empty
*/
#ifdef RESOLVER_ENV
/* if in a separate library: */
    RESOLVLIB =  -lresolv
/* if in /lib/libc.a: */
/*     RESOLVLIB = */
#endif /* RESOLVER_ENV */


/*
 * Andrew used to do all image formats itself.  I can't maintain
 * that, so I've implemented image I/O using a helper library.
 * Unfortunately, all helper libraries suck in some way, so I went with
 * the one that was the most compatible with what I needed to do:
 * ImageMagick/MagickWand (http://www.imagemagick.org/).  It's rather
 * heavy, probably slow, but it supports most any format and doesn't barf
 * if it has to autocovert the input format.
 *
 * DevIL not only does not support writing GIF, it behaves badly with
 * single-bit and grayscale images.  In general it has very few formats
 * it supports.  It was my first "working" attempt at a generic converter,
 * but now it's gone.
 *
 * FreeImage has a weird license that makes it hard to tell if it's even
 * legal to use in this way.  Also, it can't write GIF from RGB, and
 * I don't feel like messing with trying to adapt, since it can't
 * distinguish between that issue and any other error writing the file.
 * It also has a confusing way of specifying internal image formats,
 * and required using undocumented/internal functions and swapping the
 * RGB order manually to get it to work.
 *
 * I was considering some other libraries available in gentoo at this time:
 *   imlib - doesn't build on gentoo
 *   imlib2 - poorly documented, hard to integrate
 *   AfterImage - probably worse than devil; not worth trying
 *   SDL[2]-image - too tied to SDL
 *   OpenImageIO - doesn't support reading/writing anything but named files
 *   ImageMagick/Magick++ - too C++-centric (hard to implement FILE * I/O)
 *   ImageMagick/MagickCore - don't want to invest that much time.
 */
/* Too much depends on this now to allow this to be turned off
#define USE_IMAGEIO
*/

#if 0
 Other variables we set in our local site.mcr files 
 (on one platform or another)

	/* programs at strange locations or with special flags */
     INSTALL = $(BASEDIR)/etc/install -ns
     MAKE = make -r
     YACC = bison -y
     GNUEMACS = /usr/local/bin/emacs
     CC = /usr/local/bin/gcc
     CPPC = /usr/local/bin/g++

     INCLUDES =  -I$(BASEDIR)/include \
         -I$(BASEDIR)/include/atk

     /* OR add an includes directory for the resolver */
     INCLUDES =  -I${BASEDIR}/include  \
        -I$(BASEDIR)/include/atk \
        -I$(XINCDIR)

#endif /* 0 */
