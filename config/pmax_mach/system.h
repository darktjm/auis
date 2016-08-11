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
#define	OPSYSNAME	"PMAX_MACH"
#define	sys_pmax_mach	1
#define	SYS_NAME	"pmax_mach"
#define	PMAX_ENV

/* Here follow the overrides for this system. */
#undef	SY_B43
#define	SY_B43	1 /* This system is most like bsd 4.3 */

#ifndef In_Imake

#undef WAIT_STATUS_TYPE
#undef WAIT_EXIT_STATUS
#undef DIRENT_TYPE
#define WAIT_STATUS_TYPE union wait
#define WAIT_EXIT_STATUS(x) ((x).w_retcode)
#define DIRENT_TYPE struct direct

#include <atkproto.h>

/* Get major data types (esp. caddr_t) */
#include <sys/types.h>

#undef MIN
#undef MAX
#include <sys/param.h>

#include <stdio.h>
#define strlen __strlen_hidden
#include <strings.h>
#include <string.h>
#undef strlen

/* Get open(2) constants */
#include <sys/file.h>

/* Get struct timeval */
#ifdef SYSV
#include <time.h>
# ifdef mips
# include <bsd/sys/time.h>
# endif /* mips */
#else /* else not SYSV */
#include <sys/time.h>
#endif /* SYSV */

/* include path for syslog.h BSD vs SYSV */
#include <syslog.h>

#include <unistd.h>
#include <stdlib.h>
#include <stdarg.h>

#include <ctype.h>

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

#include <fcntl.h>

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

BEGINCPLUSPLUSPROTOS
/* Make a time standard. */
struct osi_Times {unsigned long int Secs; unsigned long int USecs;};
/* Set one of the above with a call to osi_GetTimes(&foo) */
#define osi_GetSecs() time((long int *) 0)
extern void osi_SetZone();
extern char *osi_ZoneNames[];
extern long int osi_SecondsWest;
extern int osi_IsEverDaylight;
extern int osi_GetTimes(struct osi_Times *p);
ENDCPLUSPLUSPROTOS
/*
 * Put system-specific definitions here
 */

BEGINCPLUSPLUSPROTOS
/* functions missing from the G++ and system header files */
extern char  *getwd(char *foo);
extern int  fchmod(int fd, int mode);
struct direct;
extern int scandir (char *DirectoryName, struct direct *(*NameList[]), int (*Select)(struct direct *), int (*Compare)(struct direct **, struct direct **));
#define scandir __hidden_scandir_
#include <sys/dir.h>
#undef scandir
extern int alphasort(struct direct **, struct direct **);
ENDCPLUSPLUSPROTOS

#ifdef mips
# ifdef SYSTYPE_SYSV
# include <bsd/sys/ioctl.h>
# include <bsd/sys/file.h>
# endif /* SYSTYPE_SYSV */
#endif /* mips */

#define HAS_SYSEXITS 1

#undef QSORTFUNC
#undef SCANDIRCOMPFUNC
#undef SCANDIRSELFUNC
#define QSORTFUNC(x) ((int (*)(const void *, const void *))(x))
#define SCANDIRCOMPFUNC(x) ((int (*)(struct direct **, struct direct **))x)
#define SCANDIRSELFUNC(x) ((int (*)(struct direct *))x)


#ifndef NULL
#define NULL 0
#endif

#endif /* !In_Imake */

/* Now follow the site-specific customizations. */
#include <site.h>

#endif	/* SYSTEM_H */
