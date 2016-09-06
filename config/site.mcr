/* 
	Copyright Carnegie Mellon, 1996 - All rights reserved
	$Disclaimer: 
# Permission to use, copy, modify, and distribute this software and its 
# documentation for any purpose and without fee is hereby granted, provided 
# that the above copyright notice appear in all copies and that both that 
# copyright notice and this permission notice appear in supporting 
# documentation, and that the name of IBM not be used in advertising or 
# publicity pertaining to distribution of the software without specific, 
# written prior permission. 
#                         
# THE COPYRIGHT HOLDERS DISCLAIM ALL WARRANTIES WITH REGARD 
# TO THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF 
# MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL ANY COPYRIGHT 
# HOLDER BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL 
# DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, 
# DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE 
# OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION 
# WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
# 
#  $


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
        CDEBUGFLAGS = -g3 -O2 -fPIC -D_XOPEN_SOURCE -D_DEFAULT_SOURCE -Wmissing-declarations -Wold-style-definition -Wextra -Wno-unused-parameter -Wmissing-declarations -Wno-unused-result -Wno-missing-field-initializers -Wwrite-strings -march=athlon64 -msse3 -msse3 -pipe -Wall

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


/* If there is already a JPEG distribution at your site, #undefine
 	MK_JPEG (in site.h) and set JPEGLIBDIR to point to the 
	library containing libjpeg.a */
JPEGLIBDIR=$(BASEDIR)/lib
        JPEGLIB =$(JPEGLIBDIR)/libjpeg.a

/* If there is already a TIFF distribution at your site, #undefine
	MK_TIFF in site.h and set TIFFLIBDIR to point to 
	the library containing libtiff.a */
TIFFLIBDIR=$(BASEDIR)/lib
        TIFFLIB = $(TIFFLIBDIR)/libtiff.a


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
