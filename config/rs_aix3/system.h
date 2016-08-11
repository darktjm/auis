/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
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
#ifndef	SYSTEM_H
#define	SYSTEM_H

/* Get common definitions
 *
 * NOTE:
 * define CFRONT_ENV or IBMCSET_ENV in presite.h for the AT&T C++ compiler
 * or the IBM CSet++ (xlC++) compiler, respectively.  The default is g++.
 */
#include <presite.h>

#if defined(__xlC__) && !defined(GNU_ENV)
/* This really won't work unless imake also defines __xlC__, otherwise the
 * generated makefile will attempt to use g++.  Put a #define IBMCSET_ENV 1
 * into presite.h to force it to work.
 */
#define IBMCSET_ENV 1
#endif

#if !defined(In_Imake) && defined(IBMCSET_ENV)
/* We run the CSet++ compiler in "extended" mode to allow non-standard C++
 * code to compile.  Thus, __STDC__ is not defined.  So we manually set
 * ANSI_CPP so things defined in aconcat.h and such will still work.
 */
#define ANSI_CPP 1
#endif

#include <allsys.h>

#ifdef GNU_ENV
/* sigh, this triggers a bug in gcc 2.5.8... */
#undef ATK_INTER
#undef ATK_IMPL
#define ATK_INTER #pragma interface
#define ATK_IMPLPRAG #pragma implementation
#define ATK_IMPL(x) ATK_IMPLPRAG x
#endif

#define	OPSYSNAME	"risc_aix3"
#define	sys_rt_aix3	1
#define	SYS_NAME	"rs_aix3"

#define ANSI_COMPILER 1

#ifndef _IBMR2
#define _IBMR2 1
#endif /* _IBMR2 */ 

#define EXTRA_LIBPATH_DEFAULTS $(XLIBDIR):

#undef SetLibPath
#define SetLibPath(x) LIBPATH=x:$${LIBPATH-EXTRA_LIBPATH_DEFAULTS/usr/lib:/lib};export LIBPATH

#define ATK_RELATIVIZE_FUNCTION(arg,dot,slash) ATKUseExportsFiles(arg,dot,slash)

#define HAVE_DYNAMIC_LOADING 1
#define HAVE_DYNAMIC_INTERLINKING 1
#define HAVE_SHARED_LIBRARIES 1

/* Try to detect AIX 4.1 here.  We cleverly include limits.h, but we must
 * be careful when in Imake because we can't declare C structs :-)!
 * We look for the presence of OFF_MAX which is in 4.1 and not in 3.2.  Other
 * symbols will probably work, too.
 */
#ifdef In_Imake
#define _H_FLOAT	/* don't let limits.h include float.h */
#endif
#include <limits.h>

#ifdef OFF_MAX		/* Defined in limits.h on AIX 4.1 */
#undef SY_AIX4
#define SY_AIX4 1
#else
#undef SY_AIX3
#define SY_AIX3 1
#endif

#ifndef IBMCSET_ENV
#if !SY_AIX4
#define NEED_ATKINIFINI 1
#endif
#endif

#ifdef In_Imake
#ifndef IBMCSET_ENV
#undef InstallATKExportsTarget
#define InstallATKExportsTarget(target,name,dest) @@\
target:: @@\
	$(INSTALL) $(INSTLIBFLAGS) name.exp dest/name.exp
#undef InstallATKExportsLibrary
#define InstallATKExportsLibrary(dobj,dest) @@\
install.time:: @@\
	$(INSTALL) $(INSTLIBFLAGS) dobj.exp dest/dobj.exp
#endif
#endif
#define DynamicPreLinkCommand(dobj,extraclasses,objs,libs,syslibs) $(BASEDIR)/etc/exp.csh -d dobj.exp $(BASEDIR) objs;
#define PreDynFlags(dobj,extraclasses,objs,libs,syslibs) -Wl,-e,main -Wl,-bM:SRE -Wl,-bE:dobj.exp

#undef POSIX_ENV
#define POSIX_ENV 1	/* This is a Posix system. */

#ifndef In_Imake
#include <atkproto.h>
extern int sys_nerr;
extern char *sys_errlist[];
#include <errno.h>

#if defined(__GNUG__) || defined(__GNUC__)
/* shut up gcc about rusage declared in a parameter list of a g++ header file */
struct rusage;
#endif

/* I hate G++ */
#ifdef __GNUG__
#include <stddef.h>
#if defined(__cplusplus) || defined(c_plusplus) || defined(CPP_COMPILER)
#undef NULL
#define NULL 0
#endif
#endif

#ifdef CFRONT_ENV
#define vsprintf __vsprintf 
#define wait3 __wait3 
#define setpgrp __setpgrp 
#define ecvt __ecvt 
#define fcvt __fcvt 
#define gcvt __gcvt 
#include <stdlib.h> 
#undef ecvt 
#undef fcvt 
#undef gcvt
BEGINCPLUSPLUSPROTOS
char *ecvt(double Value,  int NumberOfDigits, int *DecimalPointer, int
*Sign); 
char *fcvt(double Value,  int NumberOfDigits, int *DecimalPointer, int
*Sign); 
char *gcvt (double Value, int NumberOfDigits, char *Buffer);
ENDCPLUSPLUSPROTOS
#endif /* CFRONT_ENV */
 
#ifdef IBMCSET_ENV
/* IBM CSet++ compiler (tested with versions 2.1 and 3.1) */

#undef QSORTFUNC
#undef SCANDIRCOMPFUNC
#undef SCANDIRSELFUNC
#undef SCANDIRSELARG_TYPE
#define QSORTFUNC(x) (int (*)(const void *,const void *))(x)
#if SY_AIX4
#define SCANDIRCOMPFUNC(x) ((int(*)())x)
#define SCANDIRSELFUNC(x) ((int(*)())x)
#else
#define SCANDIRCOMPFUNC(x) ((int (*)(struct dirent **, struct dirent **))x)
#define SCANDIRSELFUNC(x) ((int (*)(struct dirent *))x)
#endif
#define SCANDIRSELARG_TYPE struct dirent

#include <stdlib.h> 
#if !SY_AIX4
/* Use mem* routines to avoid pointer conv problems. */
#define bzero(p, n) memset(p, 0, n)
#define bcopy(s, d, n) memcpy(d, s, n)
#define bcmp(s1, s2, n) memcmp(s1, s2, n)
#endif
#endif /* IBMCSET_ENV */

#include <sys/types.h>
#if SY_AIX4
#include <sys/stat.h>
#else
#define fchmod fchmod_hidden_
#include <sys/stat.h>
#undef fchmod
#endif
#include <sys/access.h>
#include <stdlib.h>
#include <strings.h>
#include <string.h>
#include <unistd.h>
#if defined(__STDC__) || defined(__cplusplus) || defined(c_plusplus) || defined(ANSI_COMPILER)
#include <stdarg.h>
#else
#include <varargs.h>
#endif
#include <stdio.h>
#include <dirent.h>


#include <sys/wait.h>
 
/* Get open(2) constants */
#include <sys/file.h>


/* Get struct timeval */
#include <sys/time.h>
#include <sys/times.h>
#include <time.h>
/* Get utime, and utimbuf, the POSIX replacement for utimes. */
#include <utime.h>

 /* include path for syslog.h */
#include <sys/syslog.h>

#ifdef __GNUG__
/* Hack around SignalHandler typedef provided by
 GNU for Interviews compatibility, it conflicts
 with a method called SignalHandler. */
#define SignalHandler G_SignalHandler
#include <signal.h>
#undef SignalIgnore
#undef SignalBad
#undef SignalDefault
#define SignalDefault ((G_SignalHandler)0)
#define SignalIgnore ((G_SignalHandler)1)
#define SignalBad ((G_SignalHandler)-1)
#undef SignalHandler
#else
#include <signal.h>
#endif

#include <sys/select.h>

#include <ctype.h>

#define OSI_HAS_SYMLINKS 1
/* If OSI_HAS_SYMLINKS is not defined, osi_readlink is present in libutil. */
#define osi_readlink(PATH,BUF,SIZE) readlink((PATH),(BUF),(SIZE))
#include <sys/lockf.h>
#include <sys/flock.h>
#define osi_ExclusiveLockNoBlock(fid)	lockf((fid), F_TLOCK, 0)
#define osi_UnLock(fid)			lockf((fid), F_ULOCK, 0)

#define osi_O_READLOCK			O_RDWR
#define osi_F_READLOCK			"r+"

/* handle (BSD) vfork for (AIX) which only knows fork */
#define	osi_vfork()			fork()

/* Handle the absence of _setjmp and _longjmp on AIX. */
#define	osi_setjmp  setjmp
#define	osi_longjmp longjmp

/* Make a time standard. */
struct osi_Times {unsigned long int Secs; unsigned long int USecs;};
/* Set one of the above with a call to osi_GetTimes(&foo) */
#define osi_GetSecs() time((long int *) 0)
#define osi_SetZone() tzset()
#define osi_ZoneNames tzname
#define osi_SecondsWest timezone
#define osi_IsEverDaylight daylight

#if 1
/* More BSD-isms */
#define setlinebuf(file) setvbuf(file, NULL, _IOLBF, BUFSIZ)
#endif

/*
 * Put system-specific definitions here
 */
#define HAS_SYSEXITS 1
#define BIT_ZERO_ON_LEFT 1

#define setreuid(r,e) atk_setuid(r)

#ifndef IBMCSET_ENV
#undef QSORTFUNC
#undef SCANDIRCOMPFUNC
#undef SCANDIRSELFUNC
#undef SCANDIRSELARG_TYPE
#define QSORTFUNC(x) (int (*)(void *,void *))(x)
#define SCANDIRCOMPFUNC(x) ((int (*)(struct dirent **, struct dirent **))x)
#define SCANDIRSELFUNC(x) ((int (*)(struct dirent *))x)
#define SCANDIRSELARG_TYPE struct dirent

#define bzero(x,y) memset(x, 0, y)
#define bcopy(x,y,z) memmove(y,x,z)
#endif


BEGINCPLUSPLUSPROTOS
 

/* Prototypes missing from the system header files. */
#if !SY_AIX4
extern int  fchmod(int fd, int mode);
extern int scandir (char *DirectoryName, struct dirent *(*NameList[]), int (*Select)(struct dirent *), int (*Compare)(struct dirent **, struct dirent **));
int alphasort(struct dirent **, struct dirent **);
extern int killpg(int pid, int sig);
struct sockaddr;
extern int recvfrom(int fd, char *buf, int length, int flags, struct sockaddr *from, int *flen);
extern void srandom(int Seed);
extern long random(); 
extern char *initstate(unsigned int Seed, char *State, int Number);
int gethostname (char *, int);
int ioctl(int fd, int cmd, void *arg);
int gettimeofday(struct timeval *tp, struct timezone *tzp);
#endif

#ifndef IBMCSET_ENV
extern char  *getwd(char *foo);
extern int lockf(int fd, int req, off_t size);
struct sockaddr;

#ifdef CFRONT_ENV
extern int sigvec(int sig, struct sigvec *,struct sigvec *);
extern char *mktemp(char *name); 
extern int ftruncate(int fd, size_t len); 
extern int readlink(const char *p, char *buf, size_t len); 
extern int setsockopt(int fd, int opttype, int opt, char *buf, size_t bufsize);
#undef wait3
#undef setpgrp
#undef vsprintf
extern int vsprintf(char *buf, const char *msg, va_list va);
struct rusage;
extern int wait3(int *status, int flags, struct rusage *r); 
extern int setpgrp();
#endif /* CFRONT_ENV */
#endif /* !IBMCSET_ENV */

/* End of prototypes missing from the system header files. */

extern int osi_GetTimes(struct osi_Times *p);

ENDCPLUSPLUSPROTOS

#define NEED_ISINF_USING_FINITE 1
#include <atkos.h>
#endif /* !In_Imake */

#define GETDOMAIN_ENV 1

/* Now follow the site-specific customizations. */
 
#include <site.h>

#endif	/* SYSTEM_H */
 
