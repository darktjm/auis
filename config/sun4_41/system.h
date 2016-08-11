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

/* This first section contains definitions and includes which should be present for both
 C, C++ source files and for Imakefiles.
All #includes should be prsent exactly as they are in this file, unless otherwise
 noted. */

/* The default definitions of ANSI_COMPILER, ANSI_CPP and ANSI_IMAKECPP may be overridden here and/or in site.h */
#undef ANSI_COMPILER
#define ANSI_COMPILER 1
#undef ANSI_CPP
#define ANSI_CPP 1
#undef ANSI_IMAKECPP

/* presite.h should be used to select a compiler if the system.h file has support for multiple compilers. */
#include <presite.h>

#include <allsys.h>

#undef POSIX_ENV
#undef HAVE_POSIX_SIGNALS
#undef HAVE_WAITPID

#define	OPSYSNAME	"SUNOS_41" /* the full name of this operating system */
#define	sys_sun_41	1	/* sys_<system-type-name> should be defined to 1 */
#define	SYS_NAME	"sun_41" /* the short name of this operating system, should be the same as the system-type-name */


/* any necessary machine specific #defines may be set here */

#undef ATK_INTER
#undef ATK_IMPLPRAG
#undef ATK_IMPL
#define ATK_INTER #pragma interface
#define ATK_IMPLPRAG #pragma implementation
#define ATK_IMPL(x) ATK_IMPLPRAG x

/* If the system provides shared libraries and you have implemented a means to build
 ATK libraries as shared (by implementing mkatkshlib), then HAVE_SHARED_LIBRARIES
 should be defined and INSTALL_SHLIB_SUPPORT should be defined as an Imakefile
 fragment which will install any files needed to support shared libraries, these files
 should be kept in the source tree in config/system-type-name.  (In particular at least
 mkatkshlib should be installed.) */
/* 
#define USE_SHARED_LIBS 1
*/

#define HAVE_DYNAMIC_LOADING 1
#ifdef USE_SHARED_LIBS
#define HAVE_SHARED_LIBRARIES 1
#ifdef In_Imake
#define InstallATKLibrary(name, dest)			@@\
InstallFiles(Concat(lib,name.$(ATKLIBEXTM)),$(INSTLIBFLAGS), dest)				@@\
InstallFiles(name.lt name.rf, $(INSTLIBFLAGS) , dest)  @@\
InstallLink(Concat(lib,name).$(ATKLIBEXTM), dest/Concat(lib,name).$(ATKLIBEXTM).1.0) @@\
InstallATKExportsTarget(install.time,name,dest)
#endif
#endif

#undef PreDynFlags
#define PreDynFlags(dobj,extraclasses,objs,libs,syslibs)  $(PREDYNFLAGS)

#undef InstalLATKExportsTarget
#define InstallATKExportsTarget(target,name,dest) @@\
target:: name.exp @@\
	$(INSTALL) $(INSTINCFLAGS) name.exp dest/name.exp

#undef ATKExportsLibrary
#define ATKExportsLibrary(dobj,objs,libs,syslibs,extraargs) ATKLibrary(dobj,objs,libs,syslibs,extraargs)


#undef InstallATKExportsLibrary
#define InstallATKExportsLibrary(dobj,dest) InstallATKLibrary(dobj,dest)

/* The SY_ macros should hopefully become obsolete fairly soon, in the mean time you should
 uncomment one of the macros below which most closely describes your system.  In general
 these should be generic OS descriptions like SVR4, or BSD4.4, not specific OSes like AIX... */
 /* You should add a corresponding #undef SY_* line before the line you uncomment.
  (To avoid warnings from CPP about redefined macros.)
  */
 /* The only currently supported systems are bsd4.2 and bsd4.3 */
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

#undef SY_B43
#define SY_B43 1

/* The following section includes #defines, #includes and any other hackery which is needed
 in C and/or C++ source files. (and which should NOT be included for Imakefiles)*/
#ifndef In_Imake
/* Get the definitions of BEGINCPLUSPLUSPROTOS and ENDCPLUSPLUSPROTOS.
 These macros should be used to delimit any sequence of data types or function prototypes for
 C functions or datastructures.*/
#include <atkproto.h>
#include <errno.h>
extern int sys_nerr;
extern char *sys_errlist[];

#ifdef NEED_ALLOCA
/* any special hacks for files using alloca should go here. */
#endif /* NEED_ALLOCA */

/* The following list of #includes may be modified as necessary to bring in the required
 types, macros and functions. */
/* the following types should be included.  (usually from sys/types.h)
 dev_t	device identifiers
 gid_t	group ids
 ino_t	file inode numbers
 mode_t	file attributes
 nlink_t	link count
 off_t	file size
 pid_t	process identifiers
 size_t	(the C standard type)
 ssize_t	a count of bytes which uses a negative value for an error condition
 uid_t	a numeric user identifier
 */
#include <sys/types.h>
/* The following macros, types and functions should be included. (usually from sys/stat.h)

 Data Structures:
 struct stat with members:

 st_atime may not be meaningful for files on a distributed file system.
 st_atime	last access time
 st_ctime	last file status change time
 st_mtime	last file modification time

st_dev and st_ino should generally not be used, as their meaning is unclear in the context
 of a distributed file system.
 st_dev	device identifier
 st_ino	file inode number

it should be noted that st_gid and st_uid may not correctly reflect whether
 the current user owns a file (in the context of a distributed file system)
 st_gid	group identifier
 st_uid	owner of the file

note that with a distributed file system (such as AFS) the st_mode flags for group
 and other access may be ignored.
 st_mode	file attributes
 st_nlink	number of links to the file
 st_size	size of the file

 MACROS:
 (m is taken from the st_mode member of a stat structure.)
 S_ISDIR(m)		true iff the file is a directory.
 S_ISCHR(m)	true iff the file is a character special file
 S_ISBLK(m)	true iff the file is a block special file
 S_ISREG(m)	true iff the file is a regular file
 S_ISFIFO(m)	true iff the file is a pipe or FIFO

 S_IRWXU	Read, Write and Search/Execute  permission mask for st_mode. (for the owner)
 S_IRWXG	Read, Write, and Search/Execute  permission mask for st_mode. (for the group)
 S_IRWXO	Read, Write, and Search/Execute permission mask for st_mode. (for others)

 S_ISUID	Set user id.
 S_ISGID	Set group id.

 Functions:
 int chmod(const char *path, mode_t mode);
 int mkdir(const char *path);
 int stat(const char *path, struct stat *buf);
 int fstat(int fd, struct stat *buf);
 int mkfifo(const char *path, mode_t mode);
 int umask(mode_t cmask);
 */
#include <sys/stat.h>
/*The following macros, types and functions should be included. (usually from stdlib.h)
 Macros:
 EXIT_FAILURE
 EXIT_SUCCESS
 MB_CUR_MAX
 NULL
 RAND_MAX
 
Types:
 div_t	the structure returned from div()   (the remainder and quotient of two ints)
 ldiv_t	the structure returned from ldiv()  (the remainder and quotient of two longs)
 size_t	a count of bytes, unsigned.
 whcar_t	a wide character

 Functions:
 abort	abs
 atexit	atof
 atol	atoi
 bsearch	calloc
 div	exit
 free	getenv
 labs	ldiv
 malloc	mblen
 mbstowcs	mbtowc
 qsort	rand
 realloc	srand
 strtod	strtol
 strtoul	wcstombs
 wctomb 
 */
#include <stdlib.h>
/* strings.h commonly has the old-style string functions:
 bcopy
 bzero
 index
 rindex
 if strings.h is not present or does not include appropriate definitions of these
 functions other steps should be taken to define them.
#include <strings.h>
     */
/* string.h is mandated by posix and contains:
 Macros:
 NULL

 Types:
 size_t

 Functions:
 memchr	memcmp	memcpy	memmove	memset
 strcat	strchr	strcmp	strcoll	strcpy
 strcspn	strerror	strlen	strncat	strncmp
 strncpy	strpbrk	strrchr	strspn	strstr
 strtok	strxfrm
*/
#define strlen __hidden_strlen
#include <string.h>
#undef strlen
/* unistd.h is mandated by posix:
 Macros:
 F_OK		NULL
 R_OK		SEEK_CUR
 SEEK_END		SEEK_SET
 STDERR_FILENO	W_OK
 X_OK		*/
 
#include <unistd.h>
/* stdarg.h is part of the ANSI standard: (note that varargs cannot be substituted, a stdarg interface
 is required.
 Macros:
 va_arg
 va_end
 va_list
 va_start
 */
#include <stdarg.h>
/* stdio.h is specified by posix and ANSI:

 Macros:
 BUFSIZ		EOF
 FILENAME_MAX	L_ctermid
 L_cuserid		L_tmpnam
NULL		SEEK_CUR
SEEK_END		SEEK_SET
STREAM_MAX	TMP_MAX
stderr		stdin
stdout		_IOFBF
 _IOLBF		_IONBF

 Types:
 fpos_t	size_t
 FILE

 Functions:
 clearerr	fclose
 fdopen	feof
ferror	fflush
 fgetc	fgetpos
 fgets	fileno
 fopen	fprintf
 fputc	fputs
 fread	freopen
 fscanf	fseek
 fsetpos	ftell
 fwrite	getc
 getchar	gets
 perror	printf
 putc	putchar
 puts	remove
 rename	rewind
 scanf	setbuf
 setvbuf	sprintf
 sscanf	tmpfile
 tmpnam	ungetc
 vfprintf	vprintf
 vsprintf
 */
#include <stdio.h>
/* #ifdef __cplusplus
extern "C" {

}
#endif */
/* dirent.h is specified by POSIX, on BSD systems this header is sys/dir.h and
 struct dirent is struct direct.  (The macro DIRENT_TYPE will need to
 be changed if your system uses struct direct instead of struct dirent. See the
 last part of this file.)
 Types:
 DIR
 struct dirent with member char * d_name.
 Functions:
 closedir	opendir
 readdir	rewinddir
 */
#include <sys/dir.h>
#undef DIRENT_TYPE
#define DIRENT_TYPE struct direct

/* sys/wait.h is specified by POSIX.
 Macros:
 WEXITSTATUS	WIFSIGNALED
 WNOHANG		WTERMSIG
 WIFEXITED	WIFSTOPPED
 WSTOPSIG		WUNTRACED
Functions:
 wait	waitpid
 */
#include <sys/wait.h>
 
/* Get open(2) constants
 This should only be needed on BSD systems.
#include <sys/file.h>
*/
#include <sys/file.h>

/* Get struct timeval

 sys/time.h usually has struct timeval, but it is not  specified by posix.

 sys/times.h is specified by posix:
 Types:
 clock_t
struct tms with members: tms_ctime, tms_cutime, tms_stime, tms_utime
Functions:
 times
 
 time.h is specified by posix:
 Macros:
 CLK_TCK	CLOCKS_PER_SEC NULL

 Types:
 clock_t		size_t
 time_t
 struct tm with members tm_hour, tm_mday, tm_mon, tm_wday, tm_year,
 tm_isdst, tm_min, tm_sec, tmyday
Functions:
 asctime	ctime
 gmtime	mktime
 time	clock
 difftime	localtime	strftime
 tzzet
 Variable:
char *tzname
 */
#define select __hide_select
#include <sys/time.h>
#undef select
#include <sys/times.h>
#include <time.h>

/* Get utime, and utimbuf, the POSIX replacement for utimes. */
#include <utime.h>
 
/*
 sys/param.h
 MAXPATHLEN	maximum length of file path names.
 */
#include <sys/param.h>

/* include path for syslog.h

      int syslog(int priority, const char *message, int parameters, ...);

      int openlog(const char *ident, int logopt, int facility);

      int closelog(void);

      int setlogmask(int maskpri);
*/
#include <syslog.h>

/* if no other header file declares select, you will need to find a header file
 which declares it and #include it here, or simply put a declaration of select here:

 e.g. : extern int select(int numfds, fd_set *readset, fd_set *writeset, fd_set *exceptionset, struct timeval *timeout);

 If your system doesn't use fd_set's substitute the appropriate type.  (usually int, maybe long sometimes).
 */
BEGINCPLUSPLUSPROTOS
extern int select(int numfds, fd_set *readset, fd_set *writeset, fd_set *exceptionset, struct timeval *timeout);
ENDCPLUSPLUSPROTOS

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

/* ctype.h:
 Functions:
 isalnum		isdigit
 islower		ispunct
 isupper		tolower
 isalpha		isgraph
 isprint		isspace
 isxdigit		toupper
 iscntrl
 */
#include <ctype.h>

#define OSI_HAS_SYMLINKS 1
/* If OSI_HAS_SYMLINKS is not defined, osi_readlink is present in libutil. */
#define osi_readlink(PATH,BUF,SIZE) readlink((PATH),(BUF),(SIZE))

/* fcntl is specified by posix:
Macros:
 FD_CLOEXEC	F_DUPFD
 F_GETFD		F_GETFL
 F_GETLK		F_RDLCK
 F_SETFD		F_SETFL
 F_SETLK		F_SETLKW
 F_UNLCK		F_WRLCK
 O_ACCMODE	O_APPEND
 O_CREAT		O_EXCL
 O_NOCTTY		O_NONBLCOK
 O_RDONLY		O_RDWR
 O_TRUNC		O_WRONLY

 Types:
 flock with members: l_len, l_pid, l_start, l_type, l_whence

 Functions:
 creat	fcntl
 open
 */
#include <fcntl.h>
#ifdef __cplusplus
extern "C" int flock(int,int);
#endif
/*  these header files are NOT specified by POSIX.
 POSIX specifies an fcntl interface to locking.
 
#include <sys/lockf.h>
#include <sys/flock.h>

 osi_ExclusiveLockNoBlock tries to get a lock, and returns immediately
 if it is unable to get the lock.
#define osi_ExclusiveLockNoBlock(fid)	lockf((fid), F_TLOCK, 0)
osi_UnLock unlocks the file.
#define osi_UnLock(fid)			lockf((fid), F_ULOCK, 0)
*/
#define osi_ExclusiveLockNoBlock(fid)	flock((fid), LOCK_EX | LOCK_NB)
#define osi_UnLock(fid)			flock((fid), LOCK_UN)
#define osi_O_READLOCK			O_RDONLY
#define osi_F_READLOCK			"r"

/* handle (BSD) vfork
 osi_vfork should be the fastest available fork, on some systems
 the child created by osi_vfork will suspend it's parent until
 an exec call and will share the address space of the parent until the exec. */
#define	osi_vfork()			vfork()

/* if your system requires any special handling of setjmp and longjmp modify
 osi_setjmp and osi_longjmp, you will probably have to update some of
 the sources as well as modifying these macros. */
#define	osi_setjmp  setjmp
#define	osi_longjmp longjmp

/* Make a time standard. */
struct osi_Times {unsigned long int Secs; unsigned long int USecs;};
/* Set one of the above with a call to osi_GetTimes(&foo) */
#define osi_GetSecs() time((long int *) 0)
extern void osi_SetZone();
extern char *osi_ZoneNames[];
extern long int osi_SecondsWest;
extern int osi_IsEverDaylight;

BEGINCPLUSPLUSPROTOS
extern int osi_GetTimes(struct osi_Times *p);
ENDCPLUSPLUSPROTOS

/* More BSD-isms,  if your system has setlinebuf in libc you can remove this line. */
#define setlinebuf(file) setvbuf(file, NULL, _IOLBF, BUFSIZ)

/*
 * Put system-specific definitions here
 */
/* HAS_SYSEXITS should be one if the file /usr/include/sysexits.h exists */
#define HAS_SYSEXITS 1
/* BIT_ZERO_ON_LEFT should be one if Most Significant Bit is on the left,
 if the byte order is not simply Most to Least significant bits from left to
 right or right to left code may need to be updated. */
#define BIT_ZERO_ON_LEFT 1

#undef SCANDIRSELFUNC
#undef QSORTFUNC
#undef SCANDIRSELARG_TYPE
#undef SCANDIRCOMPFUNC
#define SCANDIRSELARG_TYPE struct direct
#define SCANDIRCOMPFUNC(x)  ((int (*)(struct direct **,struct direct **))x)
#define SCANDIRSELFUNC(x) (( int (*)(struct direct*))x)
#define QSORTFUNC(x) ( ( int (*)(const void *, const void *))x)

#undef SIGNAL_RETURN_TYPE
#define SIGNAL_RETURN_TYPE void
#undef WAIT_STATUS_TYPE
#define WAIT_STATUS_TYPE union wait

#undef SIGVECHANDLERFUNC
#define SIGVECHANDLERFUNC(x)  ((void (*)(...))x)
BEGINCPLUSPLUSPROTOS
extern void bcopy(const void *src, void *dest, int n);
extern void bzero(void *dest, int n);
extern int bcmp(const void *s1, const void *s2, int n);
extern char *fcvt(double value, int ndigit, int *decpt, int *sign);
struct rusage;
extern int wait3(union wait *statusp, int options, struct rusage *rusage);
extern     int ioctl(int fd, int request, void *arg);
extern     int sigblock(int);
extern     int sigsetmask(int);
extern int sigvec(int sig, struct sigvec *vec, struct sigvec *ovec);
extern int scandir(const char *dirname, struct direct ***namelist, int (*select)(struct direct *f), int (*compar)(struct direct **f1, struct direct **f2));
extern int alphasort(struct direct **d1, struct direct **d2);
extern char *getwd(char *pbuf);
extern int vfork();
extern int readlink(const char *path, char *buf, int bufsiz);
extern int lstat(const char *path, struct stat *buf);
extern long lrand48();
void srand48(long seed);
int gettimeofday(struct timeval *tp, struct timezone *tzp);
int settimeofday(struct timeval *tp, struct timezone *tzp);
ENDCPLUSPLUSPROTOS
 
#undef RANDOM
#undef SRANDOM
#undef INITSTATE
#define RANDOM() lrand48()
#define SRANDOM(x) srand48(x)
#define INITSTATE(x,y,z) srand48(x)

#define memmove(dst,src,length) bcopy(src,dst,length)

#ifdef KILLPG
#undef KILLPG
#endif
#define KILLPG(pgid, signal) killpg(pgid, signal)

#define index(str, ch) strchr(str, ch)
#define rindex(str, ch) strrchr(str, ch)

#include <atkos.h>
#endif /* !In_Imake */

/* GETDOMAIN_ENV should be defined as 1 if the getdomain() function is defined and can be concatenated with the result of gethostname to make a fully qualified hostname. */
#define GETDOMAIN_ENV 1

/* Now follow the site-specific customizations. */

#ifdef USE_SHARED_LIBS
#undef ATKLIBEXT
#define ATKLIBEXT "so"
#endif

#include <site.h>

#endif	/* SYSTEM_H */
 

