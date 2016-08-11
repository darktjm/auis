/* Copyright IBM Corporation 1988,1991 - All Rights Reserved */

/*
	$Disclaimer: 
Permission to use, copy, modify, and distribute this software and its 
documentation for any purpose and without fee is hereby granted, provided 
that the above copyright notice appear in all copies and that both that 
copyright notice and this permission notice appear in supporting 
documentation, and that the name of IBM not be used in advertising or 
publicity pertaining to distribution of the software without specific, 
written prior permission. 
                        
THE COPYRIGHT HOLDERS DISCLAIM ALL WARRANTIES WITH REGARD 
TO THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF 
MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL ANY COPYRIGHT 
HOLDER BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL 
DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, 
DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE 
OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION 
WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

 $
*/

/* For full copyright information see:'andrew/config/COPYRITE' */

XCOMM 
XCOMM  The following is from the allsys.mcr file, and may be over-ridden
XCOMM  by the platform-specific system.mcr file or the
XCOMM  site-specific site.mcr file.
XCOMM 

DASHL=-L

/* System-independent macros: included at the head of system.mcr. */


#ifdef AFS_ENV
/* AFSBASEDIR should point to the top of an AFS installation, 
   such that these files both exist: 
 	$(AFSBASEDIR)/lib/afs
	$(AFSBASEDIR)/include/afs  */
/* if you need to change this setting, do so in site.mcr . */
        AFSBASEDIR = /
#endif /* AFS_ENV */

/* If your AFS protection server relies on the MIT Athena Kerberos */
/* library, set KRBLIB to name where the Kerberos library lives. */
/* You will also need to define this variable if you make the MIT neos
application */
        KRBLIB = 

/* set DESTDIR to where you want Andrew installed */
/* it is necessary that it be installed incrementally, while it is built */
/* DEFAULT_ANDREWDIR_ENV is set in the allsys.h file to the location where */
/* DESTDIR will be visible by users after installation.  If this is different */
/* from the location where it will be installed incrementally at build time, */
/* set BASEDIR to that latter location by redefining it in your site.h file. */
        BASEDIR = DEFAULT_ANDREWDIR_ENV
        DESTDIR = ${BASEDIR}

/* XUTILDIR is for X11 utility programs:  
		makedepend, imake, xmkfontdir, bdftopcf  
	If one or more of these is not in XUTILDIR,
	then individual locations can be set for them, see below
*/
        XUTILDIR = /usr/bin/X11
        XINCDIR = /usr/include
        XLIBDIR = /usr/lib

        XMAKEDEPEND = $(XUTILDIR)/makedepend
        IMAKE = $(XUTILDIR)/imake
        XMKFONTDIR = $(XUTILDIR)/mkfontdir
#ifdef FONTS_TO_PCF_ENV
        XFC =  $(XUTILDIR)/bdftopcf
#else
#ifdef FONTS_TO_BDF_ENV
        XFC =  cat
#else
        XFC =  $(XUTILDIR)/bdftosnf
#endif
#endif

/* CDEBUGFLAGS are passed to C compilations.  */
/* To generate debugger symbol tables, use -g instead of -O.  */
        CDEBUGFLAGS = -O

#if !SY_OS2
        SHELL = /bin/sh
#endif
        CSHELL = /bin/csh

        CC = gcc
        CPPC = g++
        LOCALDIR = DEFAULT_LOCALDIR_ENV
       ATKLIBLDARGS=$(CPPSTDLIB) $(GCCLIB)  -lc $(LOCAL_SHARED_LIB_PATH) $(SHARED_LIB_PATH)
        GENREFS = $(BASEDIR)/etc/grefs
        REFSNM = /bin/nm

/* Malloc will generate addresses divisible by MALLOCALIGNMENT,  */
/* which must be a multiple of 4 */
        MALLOCALIGNMENT = 4


/* Specify the default standard C library. */
        CRT0PATH = /lib/crt0.o
        CLIB = /lib/libc.a

/* Check and Set the RESOLVELIB path below */
/* There are at least two sources of libresolv.a */
/* 1) 4.3 distribution */
/* 2) bind distribution */
/* At CMU we use the bind distribution of libresolv.a */
#ifdef RESOLVER_ENV
/* if in a separate library: */
/*      RESOLVLIB = /usr/lib/libresolv.a */
/* if in /lib/libc.a: */
        RESOLVLIB =
#else /* RESOLVER_ENV */
        RESOLVLIB =
#endif /* RESOLVER_ENV */

/* the following variable is for any libraries
 * needed for networking support
 */
        NETLIBS =
/* this is to catch machine-specific libraries that contain
 * fixes for various functions
 */
        SUPLIBS =

/* AT&T SYSV shared library support library */
#if defined(LIBDL_ENV)
        DYN_LINK_LIB = -ldl
#endif 

/* The Lex library macro.  System that define FLEX_ENV now: i386_Linux, i386_BSD */
#ifdef FLEX_ENV
LEXLIB = -lfl
#else
LEXLIB = -ll
#endif

/* uncomment this if your make program has MAKEFLAGS but not MFLAGS */
/* #define ConstructMFLAGS */

        ADDALIASES = $(BASEDIR)/etc/addalias
        CLASS = ${BASEDIR}/bin/class

#ifdef HAVE_DYNAMIC_LOADING
DYNEXELIBS = $(BASEDIR)/lib/libATKDynLoad.a $(BASEDIR)/lib/libATKLink.a
#else
DYNEXELIBS =
#endif
DYNMAIN = ,DynM.$$$$
POSTDYNFLAGS = $(CPPSTDLIB)  $(GCCLIB) -lc
DYNPOSTPROCESS =@true
DYNLINKPROG = $(LINKPREFIX) $(CPPC)
MKDYNMAIN = $(BASEDIR)/etc/mkdynmain

#ifdef AFS_ENV
  INCLUDES =  -I${BASEDIR}/include/atk -I${BASEDIR}/include -I$(AFSBASEDIR)/include -I${XINCDIR}
#else /* AFS_ENV */
INCLUDES =  -I${BASEDIR}/include/atk -I${BASEDIR}/include -I${XINCDIR}
#endif /* AFS_ENV */


        DEPENDSCRIPT = $(TOP)/config/depend.csh
        IRULESRC = $(TOP)/config
        FDBWM = $(BASEDIR)/bin/fdbwm
        FDBBDF = $(BASEDIR)/bin/fdbbdf


        TOP = TOPDIR
        AS = as
        CPP = /lib/cpp
        LD = ld
        LINT = lint
#ifdef BUILDANDREWINSTALL_ENV
        INSTALL = $(BASEDIR)/etc/install
#else /* BUILDANDREWINSTALL_ENV */
        INSTALL = install
#endif /* BUILDANDREWINSTALL_ENV */
        TAGS = ctags
        RM = rm -f
        MV = mv
        CP = cp
        LN = ln -s
        RANLIB = ranlib
        AR = ar clq
        ARDEL = ar d
        CHMODW = chmod +w
        LS = ls
        AWK = awk
        SORT = sort
        TR = tr
        NM = nm
        MAKE = make
        SED = sed
        LEX = lex
        YACC = yacc
        SCRIBE = scribe
        LINTOPTS = -axz
        LINTLIBFLAG = -C
        STD_DEFINES =

XCOMM  This MATHLIB macro is a workaround for a bug in HPUX8.0 ld.
XCOMM  That loader has problems linking normal archive libraries
XCOMM  into a shared library.  [console/cmd, ness/objects]
XCOMM  This macro is made empty in the appropriate hp system.mcr
XCOMM  files.
        MATHLIB = -lm
XCOMM By default the same math library is used everywhere
XCOMM on some systems different libs may be needed, or
XCOMM the math library may need to be omitted in the
XCOMM dynamic objects and shared libraries. (e.g. HP)
        EXEMATHLIB = $(MATHLIB)
        DYNMATHLIB = $(MATHLIB)
        LIBMATHLIB = $(MATHLIB)

/* If there is already a JPEG distribution at your site, #undefine MK_JPEG and set JPEGLIBDIR to point to the library containing libjpeg.a */

JPEGLIBDIR=$(BASEDIR)/lib
        JPEGLIB =$(JPEGLIBDIR)/libjpeg.a

/* If there is already a TIFF distribution at your site, #undefine MK_TIFF and set TIFFLIBDIR to point to the library containing libtiff.a */

TIFFLIBDIR=$(BASEDIR)/lib
        TIFFLIB = $(TIFFLIBDIR)/libtiff.a

#ifdef mips 
ASMPP_CC = $(CC) -E
AS_FLAGS = -nocpp
#else		/* mips */
#ifdef SCOunix
ASMPP_CC = $(CPP)
AS_FLAGS =
#else		/* SCOunix */
#ifdef GNU_ENV
ASMPP_CC = $(CPP)
AS_FLAGS =
#else		/* GNU_ENV */ /* DEFAULT */
ASMPP_CC = $(CC) -E
AS_FLAGS =
#endif		/* GNU_ENV */
#endif		/* SCOunix */
#endif		/* mips */

/* obsolete
#define SelectATKLib(name) @@\
Concat(lib,name)=Concat(lib,name).a @@\
Concat(name,XXX)=$(Concat(lib,name)) @@\
Concat(OptionalLib,name)=install.time
#define DeSelectATKLib(name) @@\
Concat(lib,name) =  @@\
Concat(name,XXX) = Concat(name,XXX) @@\
Concat(OptionalLib,name)=Concat(install.lib,name)
#include <libs.mcr>
*/

/* the directories where the lists of packages and their location variables are stored */
LOCALPACKAGEDIRS=
STDPACKAGEDIRS=$(BASEDIR)/lib/packages $(BASEDIR)/lib/packages/atk

FULLATKDLIBDIR=$(DESTDIR)ATKDLIBDIR
ATKTMPDYNEXTM=+
ATKDYNEXTM=+
ATKLIBEXTM=a
ATKDYNPREFIXM=

SYS_DYNDIR= $(SYS_CONFDIR)

ATKSTDLIBS=
ATKLIBLIBS=
ATKDYNLIBS=
ATKEXELIBS=
LOCALSTDLIBS=
LOCALLIBLIBS=
LOCALEXELIBS=
LOCALDYNLIBS=

#ifdef GNU_ENV
GPPLIBS=\\1libstdc++.a
CPPSTDLIB=`$(CC) -v  2>&1 | sed -n -e 's@Reading specs from \\(.*\\)gcc-lib/\\(.*\\)specs@$(GPPLIBS) \\1gcc-lib/\\2libgcc.a@p' `
GCCLIB=`$(CC) -v  2>&1 | sed -n -e 's@Reading specs from \\(.*\\)gcc-lib/\\(.*\\)specs@\\1gcc-lib/\\2libgcc.a@p' `
CPPDEPINCLUDES="`$(CC) -v  2>&1 | sed -n -e 's@Reading specs from \\(.*\\)gcc-lib/\\(.*\\)specs@-I\\1g++-include\\ -I\\1gcc-lib/\\2include@p'`"
#endif

GNUEMACS=/usr/local/bin/gnu-emacs

#if defined(AFS32_ENV) || defined(AFS33_ENV)
REALAFSUTIL=  ${AFSBASEDIR}/lib/afs/util.a
AFSUTIL=$(REALAFSUTIL)
#endif

#ifdef AFS_ENV
        REALAFSUTILS = $(BASEDIR)/lib/libafs.a
#if defined(CMUCS) || defined(AFS32_ENV) || defined(AFS33_ENV)
        REALAFSUTILS = $(BASEDIR)/lib/libafs.a $(REALAFSUTIL)
#endif
AFSUTILS=$(REALAFSUTILS)

#ifdef AFS33_ENV
REALAFSAUDITLIB = $(AFSBASEDIR)/lib/afs/libaudit.a
#else
REALAFSAUDITLIB =
#endif
AFSAUDITLIB=$(REALAFSAUDITLIB)

#ifdef AFS30_ENV
#ifdef AFS31_ENV
        REALDESLIB = $(AFSBASEDIR)/lib/libdes.a
        REALPRSLIB = $(AFSBASEDIR)/lib/afs/libprot.a \
	$(AFSBASEDIR)/lib/libubik.a \
	$(AFSBASEDIR)/lib/afs/libauth.a \
	$(REALAFSAUDITLIB) \
	$(AFSBASEDIR)/lib/librxkad.a \
	$(AFSBASEDIR)/lib/librx.a \
	$(AFSBASEDIR)/lib/liblwp.a \
	$(AFSBASEDIR)/lib/afs/libcom_err.a \
	$(REALKRBLIB) $(REALDESLIB) $(REALAFSUTILS)
#else /* #ifdef AFS31_ENV */
        REALPRSLIB = $(REALAFSBASEDIR)/lib/afs/libprot.a \
      	$(AFSBASEDIR)/lib/libubik.a \
      	$(AFSBASEDIR)/lib/librx.a \
      	$(AFSBASEDIR)/lib/afs/libauth.a \
	$(REALAFSAUDITLIB) \
      	$(AFSBASEDIR)/lib/librxkad.a \
      	$(AFSBASEDIR)/lib/liblwp.a \
	$(AFSBASEDIR)/lib/libscrypt.a \
	$(AFSBASEDIR)/lib/afs/libcom_err.a \
	 $(REALAFSUTILS)
#endif /* #ifdef AFS31_ENV */
#else /* #ifdef AFS30_ENV */
        REALPRSLIB = 
#endif /* #ifdef AFS30_ENV */
#ifndef CMUCS
        REALAFSSYSLIB = $(AFSBASEDIR)/lib/afs/libsys.a
#else
        REALAFSSYSLIB = $(AFSBASEDIR)/lib/afs/libafssys.a
#endif
        DESLIB=$(REALDESLIB)
        PRSLIB=$(REALPRSLIB)
        AFSSYSLIB=$(REALAFSSYSLIB)
        REALRXLIBS=$(AFSBASEDIR)/lib/librx.a \
	$(AFSBASEDIR)/lib/librxkad.a \
	$(AFSBASEDIR)/lib/liblwp.a
        RXLIBS=$(REALRXLIBS)

        REALAFSAUTHLIB=	$(AFSBASEDIR)/lib/afs/libauth.a 
       AFSAUTHLIB=$(REALAFSAUTHLIB)

      ALLAFS=$(REALPRSLIB) $(REALAFSAUTHLIB) $(REALAFSAUDITLIB) $(REALAFSSYSLIB) $(REALRXLIBS) $(REALDESLIB) $(REALAFSUTILS)

#ifdef NEED_BSDMEMSTRFUNCS
OSSUPBCOPY=bcopy.o
OSSUPBZERO=bzero.o
OSSUPBCMP=bcmp.o
OSSUPINDEX=index.o
OSSUPRINDEX=rindex.o
#endif
#ifdef NEED_AFSSTRFUNCS
OSSUPUCSTRING=ucstring.o
OSSUPLCSTRING=lcstring.o
#endif

#ifdef NEED_AFSINATKOS
AFSAUTHLIB=
AFSAUDITLIB=
AFSSYSLIB=
RXLIBS=
DESLIB=
PRSLIB=
AFSUTILS=
ALLAFSOBJFILE=$(BASEDIR)/lib/allafs.o
#endif
        UTILLIB = $(BASEDIR)/lib/libutil.a \
	$(AFSAUTHLIB) \
	$(AFSAUDITLIB) \
	$(AFSSYSLIB) \
	$(RXLIBS) \
	$(DESLIB) $(AFSUTILS)
#else /* #ifdef AFS_ENV */
        UTILLIB = $(BASEDIR)/lib/libutil.a $(BASEDIR)/lib/libafs.a
        PRSLIB =
#endif /* AFS_ENV */

ATKDLIBDIRM=ATKDLIBDIR
/* end changeable items */

XCOMM 
XCOMM  End of what comes from the allsys.mcr file.
XCOMM 
