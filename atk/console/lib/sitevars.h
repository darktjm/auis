/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

/* this is a set of defines for directories that are used within the console program - these should be tailored to the specific site that plans to use console */

#define _SITE_CONSOLELIB "/usr/andrew/lib/consoles"
#define _SITE_QUEUEMAIL "/usr/andrew/etc/queuemail"
#define _SITE_PRINT "/usr/andrew/bin/print"

#ifdef hpux
#define _SITE_NON_ANDREW_MAIL "/usr/mail/"
#else /* hpux */
#define _SITE_NON_ANDREW_MAIL "/usr/spool/mail/"
#endif /* hpux */
#define _SITE_MAILBOX "Mailbox"
#ifdef hpux
#define _SITE_NON_ANDREW_PRINTDIR "/usr/spool/lp/request"
#else /* hpux */
#ifdef M_UNIX
#define _SITE_NON_ANDREW_PRINTDIR "/usr/spool/lp/requests"
#else /* M_UNIX */
#define _SITE_NON_ANDREW_PRINTDIR "/usr/spool/lpd"
#endif /* M_UNIX */
#endif /* hpux */
#define _SITE_PRINTDIR "PrintDir"
#define _SITE_LOGFILE "/tmp/ConsoleLog"

#ifdef hpux
#define _SITE_MTAB "/etc/mnttab"
#define _SITE_FSTAB "/etc/checklist"
#else /* hpux */
#define _SITE_MTAB "/etc/mtab"
#define _SITE_FSTAB "/etc/fstab"
#endif /* hpux */

#define _SITE_BIN_SH "/bin/sh"

#define _SITE_DEV_TTY "/dev/tty"
#define _SITE_DEV_PTYP "/dev/ptyp"
#define _SITE_DEV_CONSOLE "/dev/console"
#define _SITE_DEV_KMEM "/dev/kmem"

#ifdef hpux
#define _SITE_VMUNIX "/hp-ux"
#else /* hpux */
#ifdef _IBMR2
#define _SITE_VMUNIX "/unix"
#else /* ! _IBMR2 */
#define _SITE_VMUNIX "/vmunix"
#endif /* _IBMR2 */
#endif /* hpux */

#define _SITE_CONSOLE_SOCKET 2018 /* udp */

#define _SITE_SCM "cluster1.fs.andrew.cmu.edu"
/* above for vopcon */

#if (defined(vax) || defined(MIPSEL))
#define _SITE_INTERCEPT "/dev/xcons"
/* used to be /dev/smscreen */
#endif /* vax || MIPSEL */

