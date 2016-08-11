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

/* Get common definitions */
#include <allsys.h>

#define ANSI_COMPILER 1
#define	OPSYSNAME	"ULTRIX_4.3bsd"
#define	sys_pmax_ul4	1
#undef SYS_NAME
#define	SYS_NAME	"pmax_ul4"
#define	PMAX_ENV

#define HAVE_DYNAMIC_LOADING 1
#define HAVE_DYNAMIC_INTERLINKING 1

#undef InstalLATKExportsTarget
#define InstallATKExportsTarget(target,name,dest) @@\
target:: name.exp @@\
	$(INSTALL) $(INSTINCFLAGS) name.exp dest/name.exp

#undef ATKExportsLibrary
#define ATKExportsLibrary(dobj,objs,libs,syslibs,extraargs) ATKLibrary(dobj,objs,libs,syslibs,extraargs)


#undef InstallATKExportsLibrary
#define InstallATKExportsLibrary(dobj,dest) InstallATKLibrary(dobj,dest)


/* Here follow the overrides for this system. */
#undef	SY_B43
#define	SY_B43	1 /* This system is most like bsd 4.3 */

#ifndef In_Imake
extern int errno;
extern int sys_nerr;
extern char *sys_errlist[];

#include <stdarg.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <time.h>
#include <utime.h>
#include <atkproto.h>

/* Get major data types (esp. caddr_t) */
#include <sys/types.h>

#undef MAX
#undef MIN
#include <sys/param.h>

#include <string.h>
#include <strings.h>

/* Get open(2) constants */
#include <sys/file.h>

#include <fcntl.h>

#include <sys/dir.h>

#include <sys/signal.h>

#include <sys/stat.h>

#include <sys/wait.h>

#define ntohl nthol_hidden_
#define	ntohs ntohs_hidden_
#define	htonl htonl_hidden_
#define	htons htons_hidden_
#include <netinet/in.h>
#undef ntohl
#undef ntohs
#undef htonl
#undef htons

BEGINCPLUSPLUSPROTOS
char *ecvt(double Value,  int NumberOfDigits, int *DecimalPointer, int
*Sign); 
char *fcvt(double Value,  int NumberOfDigits, int *DecimalPointer, int
*Sign); 
char *gcvt (double Value, int NumberOfDigits, char *Buffer);
int select(int n,fd_set *rd, fd_set *wr, fd_set *ex, struct timeval *tv);
extern u_short	ntohs(u_short), htons(u_short);
extern u_long	ntohl(u_long), htonl(u_long);
extern struct hostent *gethostbyname(char *);
extern struct hostent *gethostbyaddr(char *, int size, int net);
ENDCPLUSPLUSPROTOS

/* include path for syslog.h BSD vs SYSV */
#include <syslog.h>

#ifdef __GNUG__
 /* 3 */
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
#undef SIG_DFL
#undef SIG_IGN
#undef SIG_CATCH
#undef SIG_HOLD
#define SIG_DFL         ((void (*)(int))( 0))
#define SIG_IGN         ((void (*)(int))( 1))
#define SIG_CATCH       ((void (*)(int))( 2))
#define SIG_HOLD        ((void (*)(int))( 3))

BEGINCPLUSPLUSPROTOS
    int fork();
    int access(const char *path, int flags);
    char *getwd(char *buf);
    int fchmod(int fd, int mode);
    extern int scandir (char *DirectoryName, struct direct *(*NameList[]), int (*Select)(struct direct *), int (*Compare)(struct direct **, struct direct **));
    int alphasort(struct direct **, struct direct **); 
    extern int lockf(int fd, int req, off_t size);
    extern int flock(int f, int o); /* this is necessary -- without it, any C++ call to flock() is assumed to be to the constructor for struct flock, which is disastrous */
    extern int killpg(int pid, int sig);
ENDCPLUSPLUSPROTOS

/* VMUNIX vs. SY_B4x */
#ifndef VMUNIX
#define	VMUNIX	1
#endif VMUNIX

#define OSI_HAS_SYMLINKS 1
#define osi_readlink(PATH,BUF,SIZE) readlink((PATH),(BUF),(SIZE))

#define osi_ExclusiveLockNoBlock(fid)	flock((fid), LOCK_EX | LOCK_NB)
#define osi_UnLock(fid)			flock((fid), LOCK_UN)
#define osi_O_READLOCK			O_RDONLY
#define osi_F_READLOCK			"r"

#define	osi_vfork()			vfork()

#define	osi_setjmp  _setjmp
#define	osi_longjmp _longjmp

/* Make a time standard. */
struct osi_Times {unsigned long int Secs; unsigned long int USecs;};
/* Set one of the above with a call to osi_GetTimes(&foo) */
#define osi_GetSecs() time((time_t *) 0)
BEGINCPLUSPLUSPROTOS
extern void osi_SetZone();
extern int osi_GetTimes(struct osi_Times *);
ENDCPLUSPLUSPROTOS
extern char *osi_ZoneNames[];
extern long int osi_SecondsWest;
extern int osi_IsEverDaylight;

/*
 * Put system-specific definitions here
 */

#ifdef mips
# ifdef SYSTYPE_SYSV
# include <bsd/sys/ioctl.h>
# include <bsd/sys/file.h>
# endif /* SYSTYPE_SYSV */
#endif /* mips */

#define HAS_SYSEXITS 1

/* these defaults are appropriate for POSIX_ENV systems */

#undef SIGNAL_RETURN_TYPE
#undef WAIT_STATUS_TYPE
#undef WAIT_EXIT_STATUS
#undef DIRENT_TYPE
#undef SIGVECHANDLERFUNC
#undef SIGACTIONHANDLERFUNC

#undef QSORTFUNC
#undef SCANDIRCOMPFUNC
#undef SCANDIRSELFUNC

#define SIGNAL_RETURN_TYPE void
#define WAIT_STATUS_TYPE int
#define WAIT_EXIT_STATUS(x) WEXITSTATUS(x)
#define DIRENT_TYPE struct direct
#define DIRENT_NAMELEN(d) ((d)->d_namlen)

#define QSORTFUNC(x) (int (*)(const void *,const void *))(x)
#define SCANDIRCOMPFUNC(x) x
#define SCANDIRSELFUNC(x) x
#define SIGVECHANDLERFUNC(x) ((void (*)())(x))
#define SIGACTIONHANDLERFUNC(x) ((void (*)())(x))
#endif /* !In_Imake */

/* Now follow the site-specific customizations. */
#include <site.h>

#endif	/* SYSTEM_H */
