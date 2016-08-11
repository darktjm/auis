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


#ifndef SYSTEM_H
#define SYSTEM_H

/* Get common definitions */
#include <allsys.h>

#define OPSYSNAME "IRIX"
#define sys_sgi_4d 1
#define SYS_NAME "sgi_4d"
#define SGI_4D_ENV

/* Here follow the overrides for this system. */
#undef SY_U53
#define SY_U53  1   /* IRIX is most like SysVR3. */

#define BUILDANDREWINSTALL_ENV 1
#define USE_MLD_ENV 1

#ifndef In_Imake

#include <unistd.h> /* includes sys/types.h  */

#include <string.h>
#define index strchr
#define rindex strrchr

#include <fcntl.h>
#include <sys/file.h>

#include <sys/time.h>

#include <syslog.h>

#define getdtablesize()			_NFILE

#define OSI_HAS_SYMLINKS 1
#define osi_readlink(PATH,BUF,SIZE) readlink((PATH),(BUF),(SIZE))


#define osi_ExclusiveLockNoBlock(fid)	lockf((fid), F_TLOCK, 0)
#define osi_UnLock(fid)			lockf((fid), F_ULOCK, 0)
#define osi_O_READLOCK			O_RDWR
#define osi_F_READLOCK			"r+"

#define	osi_vfork()			fork()

#define	osi_setjmp  _setjmp
#define	osi_longjmp _longjmp

/* Make a time standard. */
struct osi_Times {unsigned long int Secs; unsigned long int USecs;};
/* Set one of the above with a call to osi_GetTimes(&foo) */
#define osi_GetSecs() time((long int *) 0)
#define osi_SetZone() tzset()
#define osi_ZoneNames tzname
#define osi_SecondsWest timezone
#define osi_IsEverDaylight daylight

#include <sys/ioctl.h>

#define HAS_SYSEXITS 1

#define NOMETAMAIL 1
#define POSIX_ENV 1

#endif /* !In_Imake */

/* Now follow the site-specific customizations. */
#include <site.h>

#endif /* SYSTEM_H */
