/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1994 - All Rights Reserved      *
 *        For full copyright information see:'andrew/config/COPYRITE'     *
\* ********************************************************************** */

#ifndef	SYSTEM_H
#define	SYSTEM_H


#ifndef In_Imake
#define ANSI_CPP 1
#endif

#define LexStaticFile lexyy.c
#define LexStaticReplaceFile lexyy._c

#define FLEX_ENV 1

#include <allsys.h>
 
#define	OPSYSNAME	"ix86_OS2"
#define	SYS_NAME	"ix86_OS2"

#define ANSI_COMPILER 1

/* The following is for g++ to eliminate redundant vtables */
#ifdef USE_GCCPRAGMA
#undef ATK_INTER
#undef ATK_IMPL
#define ATK_INTER #pragma interface
#define ATK_IMPLPRAG #pragma implementation
#define ATK_IMPL(x) ATK_IMPLPRAG x
#endif

#define HAVE_DYNAMIC_LOADING 1
#define HAVE_DYNAMIC_INTERLINKING 1
#define HAVE_SHARED_LIBRARIES 1
#ifdef In_Imake
#undef InstallATKExportsTarget
#define InstallATKExportsTarget(target,name,dest) @@\
target:: Concat(lib,name).a @@\
	$(DESTDIR)/etc/installdll name $(DESTDIR)/dll @@\
	$(INSTALL) $(INSTINCFLAGS) name.map dest @@\
clean::	@@\
	$(RM) name.def name.map a-*.dll

#undef DynamicMultiObjectTarget
#define DynamicMultiObjectTarget(dobj, extraclasses, objs, libs, syslibs) @@\
all:: dobj.dll @@\
dobj.dll: objs libs @@\
	$(DESTDIR)/etc/mkatkshlib -D "dobj extraclasses" dobj $(BASEDIR) "$(LINKPREFIX)" objs libs syslibs $(MKATKLIBLIBS)
#undef InstallATKExportsLibrary
#define InstallATKExportsLibrary(dobj,dest) @@\
InstallFile(Concat(lib,dobj).a, $(INSTLIBFLAGS), dest)
#undef ATKExportsLibrary
#define ATKExportsLibrary(dobj,objs,libs,syslibs,extraargs) @@\
clean:: @@\
	$(RM) Concat(lib,dobj).a a-*.dll  dobj.def dobj.map
#undef InstallDynamicObject
#define InstallDynamicObject(dobj, extraclasses, dest) @@\
install.time:: dobj.dll @@\
	$(DESTDIR)/etc/dllalias $(ATKALIASES) a-dobj dobj extraclasses @@\
	$(DESTDIR)/etc/installdll dobj $(DESTDIR)/dll @@\
InstallFile(dobj.rf, $(INSTDOFLAGS), dest) @@\
InstallFile(dobj.lt, $(INSTDOFLAGS), dest) @@\
InstallFile(dobj.map, $(INSTDOFLAGS), dest) @@\
InstallATKExportsLibrary(dobj,dest)

#undef InstallDynamicObjectClass
#define InstallDynamicObjectClass(dobj,dest) @@\
install.time:: dobj.dll @@\
	$(DESTDIR)/etc/dllalias $(ATKALIASES) a-dobj dobj @@\
	$(DESTDIR)/etc/installdll dobj $(DESTDIR)/dll @@\
InstallFile(dobj.rf, $(INSTDOFLAGS), dest) @@\
InstallFile(dobj.lt, $(INSTDOFLAGS), dest) @@\
InstallFile(dobj.map, $(INSTDOFLAGS), dest) @@\
InstallATKExportsLibrary(dobj,dest)

#undef InstallDynamicObjectClassWithExports
#define InstallDynamicObjectClassWithExports(dobj,dest) @@\
install.time:: dobj.dll @@\
	$(DESTDIR)/etc/dllalias $(ATKALIASES) a-dobj dobj @@\
	$(DESTDIR)/etc/installdll dobj $(DESTDIR)/dll @@\
InstallFile(dobj.rf, $(INSTDOFLAGS), dest) @@\
InstallFile(dobj.lt, $(INSTDOFLAGS), dest) @@\
InstallFile(dobj.map, $(INSTDOFLAGS), dest) @@\
InstallATKExportsTarget(install.time,dobj,dest)

#define	InstallRunappAPP(app)						@@\
install.time::								@@\
	$(DESTDIR)/etc/installapp app $(DESTDIR)/bin

#endif

/* These are here for OS2 support. */
#undef SY_OS2
#define SY_OS2 1	/* define for OS2 */
#undef POSIX_ENV
#define POSIX_ENV 1	/* This is sort of a Posix system. */
#undef DITROFF_ENV

#ifndef In_Imake
#include <atkproto.h>

/* Stuff probably specific to compiling with EMX. */
#define osi_vfork() fork()
#define EFBIG 27
#define ENFILE 23
#define setitimer(a,b,c)
#undef FDTABLESIZE
#define FDTABLESIZE() 40
#undef FILE_HAS_IO
#define FILE_HAS_IO(f) ((f)->rcount)
/* These are to make gcc grok IBM TCP/IP header files */
#define _System

/* IBM CSet++ compiler (tested with version 2.1) */
#undef QSORTFUNC
#undef SCANDIRCOMPFUNC
#undef SCANDIRSELFUNC
#undef SCANDIRSELARG_TYPE
#define QSORTFUNC(x) (int (*)(__const__ void *,__const__ void *))(x)
#define SCANDIRCOMPFUNC(x) ((int (*)(struct dirent **, struct dirent **))x)
#define SCANDIRSELFUNC(x) ((int (*)(struct dirent *))x)
#define SCANDIRSELARG_TYPE struct dirent

#include <stdlib.h> 

#include <sys/types.h>
#include <sys/stat.h>
#include <strings.h>
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
BEGINCPLUSPLUSPROTOS
/* This is resolved to the TCP/IP header which isn't C++ safe... */
#include <sys/time.h>

/* This will be a duplicate declaration if IBM TCP/IP includes are used
 * ahead of EMX includes.  Use the EMX includes first, please.
 */
struct itimerval {
        struct timeval it_interval;
        struct timeval it_value;
};

ENDCPLUSPLUSPROTOS
#include <sys/times.h>
#include <time.h>

#include <signal.h>

#include <ctype.h>

#define osi_ExclusiveLockNoBlock(fid)	flock((fid), LOCK_EX | LOCK_NB)
#define osi_UnLock(fid)			flock((fid), LOCK_UN)

#define osi_O_READLOCK			O_RDWR
#define osi_F_READLOCK			"r+"

/* Make a time standard. */
struct osi_Times {unsigned long int Secs; unsigned long int USecs;};
/* Set one of the above with a call to osi_GetTimes(&foo) */
#define osi_GetSecs() time((long int *) 0)
#define osi_SetZone() tzset()
#define osi_ZoneNames tzname
#define osi_SecondsWest timezone
#define osi_IsEverDaylight daylight

/* More BSD-isms */
#define setlinebuf(file) setvbuf(file, NULL, _IOLBF, BUFSIZ)
#define lstat(p,s) stat((p),(s))

/*
 * Put system-specific definitions here
 */
/*overhead/mail/testing/utest.c can't find sysexits.h so I comment out its def and inserted some defines from it here*/
/*#define HAS_SYSEXITS 1*/
#define EX_OK 0
#define EX__BASE 64
#define EX_USAGE 64
#define EX_CANTCREAT 73
#define EX_IOERR 74
#define EX_TEMPFAIL 75
#define EX_SOFTWARE 70

#define BIT_ZERO_ON_LEFT 1

BEGINCPLUSPLUSPROTOS
 /* Prototypes missing from the system header files. */

int osi_GetTimes(struct osi_Times *p);
/* These are implemented in ossupport */
int scandir (char *DirectoryName, struct dirent *(*NameList[]), int (*Select)(struct dirent *), int (*Compare)(struct dirent **, struct dirent **));
int alphasort(struct dirent **, struct dirent **);
char *fcvt(double val, int num_dig, int *dec_p, int *sgn);

/* End of prototypes missing from the system header files. */
ENDCPLUSPLUSPROTOS

#endif /* !In_Imake */


/* Now follow the site-specific customizations. */
 
#include <site.h>

#endif	/* SYSTEM_H */
 


