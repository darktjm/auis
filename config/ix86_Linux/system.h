/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

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

#define STDC_HEADERS 1

/* presite.h should be used to select a compiler if the system.h file has support for multiple compilers. */
#include <presite.h>

#include <allsys.h>

#define	OPSYSNAME	"ix86_Linux" /* the full name of this operating system */
#define	sys_ix86_Linux	1	/* sys_<system-type-name> should be defined to 1 */
#define	SYS_NAME	"ix86_Linux" /* the short name of this operating system, should be the same as the system-type-name */

#ifdef In_Imake
#undef FAILEXIT
#define FAILEXIT test $$? -eq 0 || exit 1
#endif

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

/* Here follow the overrides for this system. */
#undef  SY_U53
#define SY_U53  1 /* This system is most like SVR3 */
#undef SY_U5x
#define SY_U5x 1


/* The following section includes #defines, #includes and any other hackery which is needed
 in C and/or C++ source files. (and which should NOT be included for Imakefiles)*/
#ifndef In_Imake
/* Get the definitions of BEGINCPLUSPLUSPROTOS and ENDCPLUSPLUSPROTOS.
 These macros should be used to delimit any sequence of data types or function prototypes for
 C functions or datastructures.*/
#include <atkproto.h>
#include <errno.h>

#undef ATK_INTER
#undef ATK_IMPLPRAG
#undef ATK_IMPL
#define ATK_INTER /* obsolescent */
#define ATK_IMPLPRAG /* obsolescent */
#define ATK_IMPL(x)

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
#include <string.h>
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
#define DIRENT_ILLEGAL_ACCESS 1
#include <dirent.h>
#undef DIRENT_NAMELEN
#define DIRENT_NAMELEN(x) (strlen((x)->d_name))
#undef DIRENT_ILLEGAL_ACCESS

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
#include <sys/time.h>
#include <sys/times.h>
#include <time.h>

/* Get utime, and utimbuf, the POSIX replacement for utimes */
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

/*  these header files are NOT specified by POSIX.
 POSIX specifies an fcntl interface to locking.
 
#include <sys/lockf.h>
#include <sys/flock.h>

 osi_ExclusiveLockNoBlock tries to get a lock, and returns immediately
 if it is unable to get the lock. */
#define osi_ExclusiveLockNoBlock(fid)	lockf((fid), F_TLOCK, 0)
/* osi_UnLock unlocks the file. */
#define osi_UnLock(fid)			lockf((fid), F_ULOCK, 0)

#undef NEED_LOCKF

/* osi_O_READLOCK is the mode needed in the open flags in order
 for locking to be done. */
#define osi_O_READLOCK			O_RDWR
/* osi_F_READLOCK is like O_READLOCK, except for fopen. */
#define osi_F_READLOCK			"r+"

/* handle (BSD) vfork for (AIX) which only knows fork
 osi_vfork should be the fastest available fork, on some systems
 the child created by osi_vfork will suspend its parent until
 an exec call and will share the address space of the parent until the exec. */
#define	osi_vfork()			fork()

/* if your system requires any special handling of setjmp and longjmp modify
 osi_setjmp and osi_longjmp, you will probably have to update some of
 the sources as well as modifying these macros. */
#define	osi_setjmp  setjmp
#define	osi_longjmp longjmp

/* Make a time standard, see the ATK Coding Standards document */
struct osi_Times {unsigned long int Secs; unsigned long int USecs;};
/* Set one of the above with a call to osi_GetTimes(&foo) */
#define osi_GetSecs() time((long int *) 0)
#define osi_SetZone() tzset()
#define osi_ZoneNames tzname
#define osi_SecondsWest timezone
#define osi_IsEverDaylight daylight

#define HAS_GETTIMEOFDAY 1
BEGINCPLUSPLUSPROTOS
extern int osi_GetTimes(struct osi_Times *p); /* in libutil.a */
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

#define SCANDIRSELFUNC(x) (( int (*)(const struct dirent*))x)
#define QSORTFUNC(x) ( ( int (*)(const void *, const void *))x)

#undef RANDOM
#undef SRANDOM
#undef INITSTATE
#define RANDOM() lrand48()
#define SRANDOM(x) srand48(x)
#define INITSTATE(x,y,z) srand48(x)

#define bcopy(src, dst, length) memmove(dst, src, length)
#define bzero(b, length) memset(b, 0, length)
#define bcmp(region1, region2, length) memcmp(region1, region2, length)
#define killpg(pgid, signal) kill(-(pgid), signal)
#define index(str, ch) strchr(str, ch)
#define rindex(str, ch) strrchr(str, ch)
#ifndef IN_ATKOS_LIB
#include <atkos.h>
#endif


#undef FILE_NEEDS_FLUSH
#define FILE_NEEDS_FLUSH(f) (1)

#undef SIGSET_TYPE
#define SIGSET_TYPE sigset_t

#undef MANDIR
#define MANDIR "/usr/share/man"

#undef TMACMANFILE
#define TMACMANFILE "/usr/share/groff/current/tmac/an.tmac"
#undef TMACPREFIX
#define TMACPREFIX "/usr/share/groff/current/tmac/"
#undef TMACSUFFIX
#define TMACSUFFIX ".tmac"

#undef FILE_HAS_IO
#define FILE_HAS_IO(f) ((f)->_IO_read_end - (f)->_IO_read_ptr)

#define NDEBUG			/* some places use asserts()'s, but */
				/* linux doesn't have ___eprintf() in */
				/* the shared libs for some reason. So */
				/* we just turn them off. */

#ifndef USG
#define USG 1
#endif /* USG */

#define getwd(pathname) getcwd(pathname, MAXPATHLEN)

#ifndef FNDELAY
#define FNDELAY O_NONBLOCK
#endif
#endif /* !In_Imake */

#ifndef FLEX_ENV
#define FLEX_ENV 1
#endif

#ifndef POSIX_ENV
#define POSIX_ENV       1
#endif /* POSIX_ENV */

#define USE_VARARGS
#define ANSI_COMPILER 1

#ifdef __cplusplus
#include <new>
#endif
#undef NULL
#define NULL 0UL

/* GETDOMAIN_ENV should be defined as 1 if the getdomain() function is defined and can be concatenated with the result of gethostname to make a fully qualified hostname. */
#define GETDOMAIN_ENV 1

/* Now follow the site-specific customizations. */
 
#include <site.h>

#define HAVE_DYNAMIC_LOADING 1
#define HAVE_DYNAMIC_INTERLINKING 1
#define HAVE_SHARED_LIBRARIES 1
#define ATKDYNOBJS_ARE_LIBS 1
#ifdef In_Imake
#define EXTRA_LIBPATH_DEFAULTS $(XLIBDIR):

#undef SetLibPath
#define SetLibPath(x) LD_LIBRARY_PATH=x:$${LD_LIBRARY_PATH-EXTRA_LIBPATH_DEFAULTS/usr/lib:/lib};export LIBPATH

#define DynamicPreLinkCommand(dobj,extraclasses,objs,libs,syslibs) $(BASEDIR)/etc/alibexp dobj $(BASEDIR) $(BASEDIR)/etc/relativize --- objs libs syslibs;
#define InstallATKLibrary(name, dest)			@@\
InstallFiles(Concat(lib,name.$(ATKLIBEXTM)),$(INSTLIBFLAGS), dest)				@@\
InstallFiles(name.lt name.rf, $(INSTLIBFLAGS) , dest)  @@\
InstallATKExportsTarget(install.time,name,dest)
#endif
#undef ATKLIBEXT
#undef ATKDYNEXT
#undef ATKDYNPREFIX
#define ATKLIBEXT "so"
#define ATKDYNEXT "so"
#define ATKDYNPREFIX "lib"

#endif	/* SYSTEM_H */
 
