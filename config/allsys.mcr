/* Copyright IBM Corporation 1988,1991 - All Rights Reserved */
/* For full copyright information see:'andrew/doc/COPYRITE' */

XCOMM 
XCOMM  The following is from the allsys.mcr file, and may be over-ridden
XCOMM  by the platform-specific system.mcr file or the
XCOMM  site-specific site.mcr file.
XCOMM 

DASHL=-L

/* System-independent macros: included at the head of system.mcr. */


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

        SHELL = /bin/sh
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

/* The Lex library macro.  All systems should now use flex. */
LEXLIB = -lfl

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

INCLUDES =  -I${BASEDIR}/include/atk -I${BASEDIR}/include -I${XINCDIR}


        DEPENDSCRIPT = $(TOP)/config/depend
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

#ifdef GNU_ENV
ASMPP_CC = $(CPP)
AS_FLAGS =
#else		/* GNU_ENV */ /* DEFAULT */
ASMPP_CC = $(CC) -E
AS_FLAGS =
#endif		/* GNU_ENV */

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

#ifdef NEED_BSDMEMSTRFUNCS
OSSUPBCOPY=bcopy.o
OSSUPBZERO=bzero.o
OSSUPBCMP=bcmp.o
OSSUPINDEX=index.o
OSSUPRINDEX=rindex.o
#endif

        UTILLIB = $(BASEDIR)/lib/libutil.a
        PRSLIB =

ATKDLIBDIRM=ATKDLIBDIR
/* end changeable items */

XCOMM 
XCOMM  End of what comes from the allsys.mcr file.
XCOMM 
