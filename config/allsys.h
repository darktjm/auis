/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *	Copyright Carnegie Mellon University, 1992, 1996 - All Rights Reserved
 *        For full copyright information see:'andrew/config/COPYRITE'     *
\* ********************************************************************** */

/*
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
#include <aconcat.h>

/* 
	Sites wishing to customize this file should do so by 
	copying "site.h.tpl" to site.h and editing the latter. 
*/

/* Define to be the default ``ANDREWDIR'' value, 
	from which users will access the Andrew installation. */
/*
#define DEFAULT_ANDREWDIR_ENV /usr/andrew
*/

/* Defined to be the default ``LOCALDIR'' value, a directory where */
/* some site-specific customizations may be installed */
#define DEFAULT_LOCALDIR_ENV /usr/local

/* These configuration flags determine how the Andrew
  software is built. Defining any one of these will cause that
  part of the system to be built (if the sources are available),
  and/or introduce dependencies on that software in other
  parts of the system.
  */

/* Defined for building for X11 */
#define X11_ENV	1

/* We build, by default, for the current release of X11.  If you're planning to use AUIS with an earlier version of MIT X11, undefine this token. */
#define FONTS_TO_PCF_ENV 1

/* Define if you're building against X11R3 or earlier */
/* #define PRE_X11R4_ENV	1 */

/* Define this if you want to build the Andrew fonts for the OpenWindows Environment. */
/* #define OPENWINDOWS_ENV 1 */

/* use direct-to-PostScript printing */
#define PSPRINTING_ENV 1

/* Choose packages to build */
#define MK_BLD_BLKS 1
#define MK_BASIC_UTILS	1 
#define MK_BASIC_INSETS	1
#define MK_HELP_APP 1
#define MK_TEXT_EXT 1
#define MK_AUTHORING 1 
#define MK_AUX_UTILS 1
#define MK_HTML 1

/* Define WWW_ENV if you want the web browser. */
#define WWW_ENV 1

/* Define this if you want to build the contributed software
   (in ./contrib). */
/* #define CONTRIB_ENV 1 */

/* these are only effective if CONTRIB_ENV is defined */
#define MK_DEMOS 1
#define MK_TIME 1
#define MK_BDFFONT 1
#define MK_GOFIG 1
#define MK_MIT 1

/* If you have either libjpeg.a or libtiff.a, #undef appropriately in site.h */
#define MK_JPEG 1
#define MK_TIFF 1

/* if this is defined, a runapp including all of ATK will be built */
/* #define MEGARUNAPP_ENV 1 */

/* Defined if you have ditroff and are using troff printing*/
/* #define DITROFF_ENV 1 */

/* Define this if you have the ndbm(3) package */
/* #define NDBM_ENV 1 */

/* Defined if you run the Internet domain name resolver */
#define RESOLVER_ENV	1

/* Defined if building for use with Snap (remote messageservers) */
/* #define SNAP_ENV    1 */

/* ____________________________________________
	below here are platform dependent options */

/* Define this if your libc.a provides getdomainname() */
/* #define GETDOMAIN_ENV 1 */

/* Defined to be where various font species live off of the 
 ANDREWDIR.  DO NOT PUT extra whitepace between the
 macro name and the value.  If you do the build will DIE. */
#define FONTDEST_X X11fonts
#define FONTDEST_OPENWIN Xnewsfonts

/* Defined if you want to build the andrew version of install */
/* Normally set if your system install does not work with our distribution */
/* In particular if your install cannot install a file as another file */
/* but only a file into a directory */
#define BUILDANDREWINSTALL_ENV 1

/* If you have an old version of Ultrix that doesn't handle disabling of 
	ECHO for pty's then define OLD_ULTRIX_ENV */
/* #define OLD_ULTRIX_ENV 1 */

/* This needs to be on unless you are using 
			the Andrew versions of the Adobe fonts */
#define	ISO80_FONTS_ENV	1

/* Put this here so that it gets into andrewos.h */
#ifndef TRUE
#define TRUE 1
#define FALSE 0
#endif

/* The short form of the copyright notice, to be displayed at the start
			of the build and on application startup. */
#define AUIS_SHORT_COPYRIGHT  \
	"AUIS Copyright 1989,96 CMU, IBM, et al.  All rights reserved."

/* Override the following in the platform system.h 
		if that platform uses the GNU streambuf FILE */
/* Currently i386_Linux and i386_bsd override this macro. */
#undef FILE_HAS_IO
#define FILE_HAS_IO(f) ((f)->_cnt)

#undef MANDIR
#define MANDIR "/usr/man"

#undef TMACMANFILE
#define TMACMANFILE "/usr/lib/tmac/tmac.an"

#undef TMACPREFIX
#define TMACPREFIX "/usr/lib/tmac/tmac."

#undef SIGSET_TYPE
#define SIGSET_TYPE sigset_t

/* The following section is stuff that will end up in 
	each source file that #include's <andrewos.h>. 
	Most of these are platform dependent and
	will be modified in system.h for the platform.
*/
#ifndef In_Imake

#ifndef MIN
#define MIN(x,y) ((x<y)?(x):(y))
#endif

#ifndef MAX
#define MAX(x,y) ((x>y)?(x):(y))
#endif

#ifndef ABS
#define ABS(a) ((a)<0?-(a):(a))
#endif

typedef int boolean;
typedef void *pointer;
typedef int (*procedure)();

#if defined(__STDC__) || defined(__cplusplus) || defined(c_plusplus)
#define ANSI_COMPILER 1
#endif
#if defined(__STDC__)
#define ANSI_CPP 1
#define ANSI_IMAKECPP 1
#endif

#define POSIX_ENV 1

/* these defaults are appropriate for POSIX_ENV systems */
#define SIGNAL_RETURN_TYPE void
#define WAIT_STATUS_TYPE int
#define WAIT_EXIT_STATUS(x) WEXITSTATUS(x)
#define DIRENT_TYPE struct dirent
#define DIRENT_NAMELEN(d) ((d)->d_namlen)

#define UTIME_TYPE struct utimbuf
#define UTIME_SET(buf,anaccesstime,amodtime) ((buf).actime=(anaccesstime), (buf).modtime=amodtime)
#define UTIME(thefile,thetimes) utime(thefile, thetimes)

#define NEWPGRP() setpgid(0,0)

#define QSORTFUNC(x) x
#define SCANDIRSELARG_TYPE const DIRENT_TYPE
#define SCANDIRCOMPFUNC(x) x
#define SCANDIRSELFUNC(x) x
#define SIGVECHANDLERFUNC(x) x
#define SIGACTIONHANDLERFUNC(x) x

#define FDTABLESIZE() sysconf(_SC_OPEN_MAX)
#define KILLPG(pgid, signal) kill(-(pgid), signal)
#define GETPGRP() getpgrp()

/* The random function specified by ANSI and POSIX is very weak,
 many platforms have random so we default to it.  The RANDOM
 macro should return a uniformly distributed sequence of random
 numbers (in the range 0 to 2^31-1 preferably). SRANDOM and
 INITSTATE should initialize the random number generator using
 the first argument as a seed.  The buffer given in the INITSTATE
 macro may be used by the random number generator.  */
#define RANDOM() random()
#define SRANDOM(x) srandom(x)
#define INITSTATE(x,y,z) initstate(x,y,z)
    
#define HAVE_ANSI_LIBC 1
#ifndef FILE_NEEDS_FLUSH
/* Override the following in the platform system.h if that platform uses some other
 * stdio structure */
#define FILE_NEEDS_FLUSH(f) ((f)->_base)
#endif

#define HAVE_FORK 1
#define HAVE_POSIX_SIGNALS 1
#define HAVE_WAITPID 1

#define ATK_INTER 
#define ATK_IMPL(x)
#endif /* In_Imake */

#define ATK_SHARED_LIB_DIR(x) $(DASHL)x

#define ATKDLIBDIR "/lib/atk"
#define ATKDYNEXT "+"
#define ATKLIBEXT "a"
#undef ATKDYNPREFIX
#define HAVE_SYMLINKS 1

/* The system.h file will override definitions in this file, as appropriate. */
/* OPSYSNAME, sys_xxxxxx, SYS_NAME, UNQUOT_SYSNAME, and
   some XXX_ENV should be defined, also. */

/* the SY_ variables are always defined 
	and have value 0 if not valid and 1 if valid.  
	In general the SY_ variables are set in the system.h file 
	and need not be modified in site.h */
/* The only currently supported systems are bsd4.2 and bsd4.3 */
#define	SY_B42	0 /* define for bsd 4.2 */
#define	SY_B43	0 /* define for bsd 4.3 */
/* These are here for System V support. */
#define	SY_U51	0 /* define for SysVR1 */
#define	SY_U52	0 /* define for SysVR2 */
#define	SY_U53	0 /* define for SysVR3 */
/* These are here for AIX support. */
#define	SY_AIX11	0	/* define for AIX 1.1 (e.g. on PS/2) */
#define	SY_AIX12	0	/* define for AIX 1.2 (e.g. on PS/2) */
#define	SY_AIX221	0	/* define for AIX 2.2.1 */
#define	SY_AIX3		0	/* defined for AIX 3 (e.g. on RS/6000) */
#define	SY_AIX4		0	/* defined for AIX 4 (e.g. on RS/6000) */

#define SY_OS2		0	/* defined for OS/2 Warp */

/* Generic bsd vs SysV defines */
#define	SY_B4x	(SY_B42 || SY_B43)
#define	SY_U5x	(SY_U51 || SY_U52 || SY_U53)
#define	SY_AIXx	(SY_AIX11 || SY_AIX12 || SY_AIX221 || SY_AIX3 || SY_AIX4)

#ifdef __GNUC__
#define UNUSED __attribute__((__unused__))
#else
#define UNUSED /* UNUSED */
#endif
