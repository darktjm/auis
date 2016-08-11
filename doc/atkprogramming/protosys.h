/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1994 - All Rights Reserved      *
 *        For full copyright information see:'andrew/config/COPYRITE'     *
\* ********************************************************************** */
The copyright notice may be dropped or replaced with your copyright notice.  Any copyright notice should be in the MIT style giving unrestricted modification/distribution rights.

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

#include <allsys.h>


#undef ANSI_COMPILER
#define ANSI_COMPILER 1

#undef ANSI_CPP
#define ANSI_CPP 1

#undef ANSI_IMAKECPP


#include <presite.h>

#undef MAX
#undef MIN

#define	OPSYSNAME	"HPUX_700/90"
#define	sys_hp_hpux_90 	1
#define	SYS_NAME	"hp_hpux_90"

/* any necessary machine specific #defines may be set here */

/* 
#define HAVE_SHARED_LIBRARIES 1 
#ifdef In_Imake
#define INSTALL_SHLIB_SUPPORT() @@\
InstallFileToFile(hp700_90/mkatkshlib, $(INSTPROGFLAGS), $(DESTDIR)/etc)
#endif
*/
/* #define HAVE_DYNAMIC_LOADING 1 */

 
/* #define	SY_B42	1 */ /* define for bsd 4.2 */
/* #define	SY_B43	1 */ /* define for bsd 4.3 */
/* These are here for System V support. */
/* #define	SY_U51	1 */ /* define for SysVR1 */
/* #define	SY_U52	1 */ /* define for SysVR2 */
/* #define	SY_U53	1 */ /* define for SysVR3 */
/* These are here for AIX support. */
/* #define	SY_AIX11	1 */	/* define for AIX 1.1 (e.g. on PS/2) */
/* #define	SY_AIX12  1 */	/* define for AIX 1.2 (e.g. on PS/2) */
/* #define	SY_AIX221	1 */	/* define for AIX 2.2.1 */
/* #define	SY_AIX3		1 */	/* defined for AIX 3 (e.g. on RS/6000) */

#undef SY_U52
#define SY_U52 1
 
#ifndef SYSV
#define SYSV 1
#endif

#ifndef In_Imake
#include <atkproto.h>

#ifdef NEED_ALLOCA
#endif /* NEED_ALLOCA */


#include <sys/types.h>

#include <sys/stat.h>

#include <stdlib.h>

#include <string.h>

#include <unistd.h>

#include <stdio.h>

#include <dirent.h>
 
/* #include <sys/file.h> */

/* #include <sys/time.h> */

#include <time.h> 

#include <sys/param.h>

#include <syslog.h>


/* 
BEGINCPLUSPLUSPROTOS
 extern int select(int numfds, fd_set *readset, fd_set *writeset, fd_set *exceptionset, struct timeval *timeout);
BEGINCPLUSPLUSPROTOS
*/

#ifdef __GNUG__
/* Additional hackery may have to be added here on some platforms when using
 G++. */
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

#include <ctype.h>

#define OSI_HAS_SYMLINKS 1

#define osi_readlink(PATH,BUF,SIZE) readlink((PATH),(BUF),(SIZE))

#include <fcntl.h>

/*
#include <sys/lockf.h>
#include <sys/flock.h>
*/

#define osi_ExclusiveLockNoBlock(fid)	lockf((fid), F_TLOCK, 0)

#define osi_UnLock(fid)			lockf((fid), F_ULOCK, 0)

#define osi_O_READLOCK			O_RDWR

#define osi_F_READLOCK			"r+"

#define	osi_vfork()			fork()

#define	osi_setjmp  setjmp
#define	osi_longjmp longjmp 

struct osi_Times {unsigned long int Secs; unsigned long int USecs;};

#define osi_GetSecs() time((long int *) 0)
#define osi_SetZone() tzset()
#define osi_ZoneNames tzname
#define osi_SecondsWest timezone
#define osi_IsEverDaylight daylight

BEGINCPLUSPLUSPROTOS
extern int osi_GetTimes(struct osi_Times *p);
ENDCPLUSPLUSPROTOS

#define setlinebuf(file) setvbuf(file, NULL, _IOLBF, BUFSIZ)

#define HAS_SYSEXITS 1

#define bcopy(src, dst, length) memmove(dst, src, length)
#define bzero(b, length) memset(b, 0, length)
#define bcmp(region1, region2, length) memcmp(region1, region2, length)
#define index(str, ch) strchr(str, ch)
#define rindex(str, ch) strrchr(str, ch)

#define killpg(pgid, signal) kill(-(pgid), signal)

#endif /* !In_Imake */

#define GETDOMAIN_ENV 1

/* Now follow the site-specific customizations. */
 
#include <site.h>

#define NEED_LOCKF 1

#endif	/* SYSTEM_H */
 
