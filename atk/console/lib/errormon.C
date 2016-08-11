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

#ifndef NORCSID
#define NORCSID
static char rcsid[]="$Header: /afs/cs.cmu.edu/project/atk-src-C++/atk/console/lib/RCS/errormon.C,v 1.9 1996/02/09 19:37:26 susan Stab74 $";
#endif


 

/* 
 ***************************************************************
 * Routines for monitoring messages on console service
 * and (on some machines) /dev/console.
 * Some routines swiped from ttyscript.
 ****************************************************************
 */


#include <andrewos.h> /* sys/types.h sys/time.h sys/file.h */
#include <im.H>
#include <consoleClass.H>
#include <environ.H>
#include <console.h>
#include <sitevars.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/socket.h>	/* for gethostname() */
#include <signal.h>
/* #include <sys/tty.h> */
#include <fcntl.h>
#include <util.h>

#if SY_AIX221
#include <sys/devinfo.h>
#include <sys/pty.h>
#include <sys/tty.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/termio.h>
#endif /* #if SY_AIX221 */

#ifdef RCH_ENV
// Consoled stuff
#include <netdb.h>
#include <netinet/in.h>
#endif

#ifdef ibm032
/* 
 * Should do the following but include files are wrong
 * #include </sys/cacons/screen_conf.h>
 * So instead we do the following hack:
 */
#define TIOCSCONS _IO(t, 104)
#endif /* ibm032 */

#ifdef M_UNIX
#include <sys/eio.h>

#endif

extern int ForceErrorMonitoring, InhibitErrorMonitoring;

  


int ConsoleSocket;
int SubChannel;
char ptyname[30];
char ThisHostName[256]="";
u_long ThisHostAddr;

#define MAXERRORRESTARTS 2

#ifndef NORCSID
#endif
#if SY_AIX221
#endif /* #if SY_AIX221 */
#ifdef ibm032
#endif /* ibm032 */
#ifdef M_UNIX
#endif
void RestartErrorMonitoring(class consoleClass  *self, char  *msg);
int InitErrorMonitoring(class consoleClass  *self,boolean  FirstTime);
extern "C" int OpenConsoleSocket();
void ReportInternalError(class consoleClass  *self, char  *string);
#if defined(M_UNIX) || defined(__hpux) || defined(hpux)
int ConsolePipe(class consoleClass  *self);
#endif /* M_UNIX */

void RestartErrorMonitoring(class consoleClass  *self, char  *msg)
        {
    char ErrTxt[256];
    static int ErrorRestarts = 0;

    mydbg(("entering: RestartErrorMonitoring\n"));
    if (++ErrorRestarts > MAXERRORRESTARTS) {
        sprintf(ErrTxt, "console: Error monitoring terminated: %s", msg);
        ReportInternalError(self, ErrTxt);
	ErrorsIn = NULL;
        return;
    } else {
        sprintf(ErrTxt,"console: An error message may have been lost: %s", msg);
        ReportInternalError(self, ErrTxt);
        fclose(ErrorsIn);
        close(SubChannel);
        InitErrorMonitoring(self, FALSE);
    }
}

int RealOpenConsoleSocket(class consoleClass *self)
{
    int flags;
    ConsoleSocket=OpenConsoleSocket();
    if(ConsoleSocket<0)
	switch(ConsoleSocket) {
	    case -1:
		if (self){
		    ReportInternalError(self, "console:<OpenConsoleSocket> gethostname failed");
		}
		else{
		    arrgh(("console:<OpenConsoleSocket> gethostname failed"));
		}
		break;
	    case -2:
		if (self){
		    ReportInternalError(self, "console:<OpenConsoleSocket> gethostbyname failed");
		}
		else{
		    arrgh(("console:<OpenConsoleSocket> gethostbyname failed"));
		}
		break;
	    case -3:
		if (self){
		    ReportInternalError(self, "console:<OpenConsoleSocket> Incomplete error monitoring: bind to ConsoleSocket failed");
		}
		else{
		    arrgh(("console:<OpenConsoleSocket> Incomplete error monitoring: bind to ConsoleSocket failed"));
		}
		break;
	    case -4:
		if (self){
		    ReportInternalError(self, "console:<OpenConsoleSocket> fcntl on ConsoleSocket failed");
		}
		else {
		    arrgh(("console:<OpenConsoleSocket> fcntl on ConsoleSocket failed"));
		}
		break;
	    case -5:
			/* obsolete. */
		break;
	    case -6:
		ReportInternalError(self, "console:<make_socket> socket failed");
		break;
	    case -7:

		ReportInternalError(self, "console:<make_socket> bind failed");
		break;
	    default: ReportInternalError(self, "console:Unkown failure in creating the console socket.");
	}
    if(ConsoleSocket<0) return ConsoleSocket;
    ConsoleIn = fdopen(ConsoleSocket, "r");
    if (ConsoleIn == NULL){
	if (self){
	    ReportInternalError(self, "console:<OpenConsoleSocket> fdopen of ConsoleIn failed");
	}
	else{
	    arrgh(("console:<OpenConsoleSocket> fdopen of ConsoleIn failed"));
	}
	return -5;
    }
    im::AddFileHandler(ConsoleIn, (im_filefptr)CheckConsoleSocket, (char *)self, 3);
    return 0;
}

static int IsRemoteDisplay( char *s )
{
    char buf[256], *c;

    if (!s) return 0; /* no DISPLAY */
    if (*s == ':') return 0; /* R5 local is :X.X */

    if (!(c = strchr(s, ':'))) return 0; /* DISPLAY bad */
    if (c-s == 4 && !ULstrncmp(s, "unix", c-s)) return 0; /* unix:X.X */
    if (c-s == 9 && !ULstrncmp(s, "localhost", c-s)) return 0; /* localhost:X.X */

    if (gethostname(buf, sizeof(buf))) return 0; /* no net ... */

    if (!ULstrncmp(s, buf, c-s) && (!buf[c-s] || buf[c-s] == '.' )) {
	return 0; /* XXX:Y.Z XXX is a possibly-abbreviated form
		   * of the hostname - this is somewhat simplistic
		   */
    }

    return 1;
}

int InitErrorMonitoring(class consoleClass  *self, boolean  FirstTime)
{
    int tempfd;
    int PtyNumber = 0;
    int flags;
    char ptyname[64];
    
    mydbg(("entering: InitErrorMonitoring\n"));
    if ((InhibitErrorMonitoring || IsRemoteDisplay(environ::Get("DISPLAY")) || environ::Get("WMHOST")) && !ForceErrorMonitoring) {
        ReportInternalError(self, "console: Not monitoring errors on remote machine");
        return(-1);
    }
    if (FirstTime) RealOpenConsoleSocket(self);	// continue even if console socket fails.
#if defined(M_UNIX) || defined (__hpux)
    return ConsolePipe(self);
#else /* M_UNIX || __hpux */
#ifdef TIOCNOTTY
    SubChannel = open (_SITE_DEV_TTY, 2);
    if (SubChannel >= 0) {
        ioctl (SubChannel, TIOCNOTTY, 0);
        close (SubChannel);
    }
#endif /* TIOCNOTTY */

    if (! GetPtyandName(&SubChannel, &tempfd, ptyname, sizeof(ptyname)))  {
        sprintf(ErrTxt, "console: Incomplete error monitoring: Can't open pty %s %s", ptyname, sys_errlist[errno]);
        ReportInternalError(self, ErrTxt);
	return(-1);
    }
#if defined(TIOCCONS) && !defined(_AIX)
    {
	int ON = 1;
	if (ioctl (tempfd, TIOCCONS, (char *) &ON) < 0) {
            ReportInternalError(self, "console: Incomplete error monitoring: ioctl (TIOCSCONS) failed");
	    return(-1);
	}
    }
#else /* TIOCCONS */
#if (defined(vax) || defined(MIPSEL))
      if ((SubChannel = open(_SITE_INTERCEPT, O_RDONLY)) < 0) {
	  if ((SubChannel = open("/dev/smscreen", O_RDONLY)) < 0){
	      sprintf(ErrTxt,"console: Incomplete error monitoring (open '%s' (and '/dev/smscreen') failed)", _SITE_INTERCEPT);
	      ReportInternalError(self, ErrTxt);
	      return(-1);
	  }
      }
#else /* vax || MIPSEL */
      {
#if SY_AIX221
	  int ON = REMOTE;
	  if (ioctl (SubChannel, PTYSETM, &ON) < 0) {
	      ReportInternalError(self, "console: Incomplete error monitoring: ioctl() failed on PTYSETM");
	      return(-1);
	  }
	  switch (fork()) {
	      case -1:
		  ReportInternalError(self, "console: Incomplete error monitoring: fork of AIX console logger failed.");
		  return(-1);
	      case 0:
		  /* child */
		  break;
	      default:
		  /* parent */
		  {
		  int con;
		  int msgqid, msgsize;
		  char iobuf[100];
		  struct tlog tl;
		  struct {
		      mtyp_t	mtype;		/* message type */
		      char	mtext[4096];	/* message text */
		  } msg;

		  dup2(tempfd, 1);
		  dup2(tempfd, 2);
		  setvbuf(stdout, iobuf, _IOLBF, sizeof(iobuf));
		  setbuf(stderr, NULL);
		  if ((con = open("/dev/console", O_RDWR)) < 0) {
		      perror("console: Incomplete error monitoring: open /dev/console failed.");
		      exit(1);
		  }
		  /* Set logging to our message queue. */
		  if ((msgqid = msgget(IPC_PRIVATE, IPC_CREAT|S_IRUSR|S_IWUSR)) < 0) {
		      perror("console: Incomplete error monitoring:  no msg queue.");
		      exit(1);
		  }
		  tl.tl_flags = TLOG_ON|TLOG_QID;
		  tl.tl_msgqid = msgqid;
		  if (ioctl(con, TCLOG, &tl) < 0) {
		      perror("console:  Incomplete error monitoring: msg queueing failed.");
		      exit(1);
		  }
		  while (1) {
		      if ((msgsize = msgrcv(msgqid, &msg, sizeof(msg.mtext), 0, MSG_NOERROR)) < 0) {
			  perror("console:  Incomplete error monitoring: msg receive error.");
			  exit(1);
		      }
		      msg.mtext[msgsize] = '\0';
		      printf("%s", msg.mtext);
		  }
		  }
	  }
#else /* #if SY_AIX221 */
#ifndef SOLARIS
#ifndef RCH_ENV
	  int ON = 1;
	  if (ioctl (SubChannel, TIOCREMOTE, &ON) < 0) {
	      ReportInternalError(self, "console: Incomplete error monitoring: ioctl (TIOCREMOTE) failed on TIOCREMOTE");
	      return(-1);
	  }
#endif
#endif
#endif /* #if SY_AIX221 */
      }
#endif /* vax */
#endif /* TIOCCONS */
#ifdef RCH_ENV
      /* Connect to consoled. */
      {
	  struct servent *sp;
	  struct sockaddr_in saddr;

	  if ((sp = getservbyname("consoled", "tcp")) == NULL) {
	      goto skipit;
	  }
	  if ((SubChannel = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
	      goto skipit;
	  }

	  memset(&saddr, 0, sizeof(saddr));
	  saddr.sin_family = AF_INET;
	  saddr.sin_addr.s_addr = getaddr();
	  saddr.sin_port = sp->s_port;

	  if (connect(SubChannel, (struct sockaddr *)&saddr, sizeof(saddr)) < 0) {
	      goto skipit;
	  }
      }
skipit:
#endif
	ErrorsIn = fdopen(SubChannel, "r");
        flags = fcntl(fileno(ErrorsIn), F_GETFL, 0);
#if POSIX_ENV
	fcntl(fileno(ErrorsIn), F_SETFL, flags | O_NONBLOCK);
#else
	fcntl(fileno(ErrorsIn), F_SETFL, flags | FNDELAY);
#endif
    im::AddFileHandler(ErrorsIn, CheckErrorsIn, (char *)self, 2);
    return(0);
#endif /* M_UNIX || __hpux */
}


void ReportInternalError(class consoleClass  *self, char  *string)
        {
    mydbg(("entering: ReportInternalError\n"));
    AnotherError(self,string, TRUE);
}

#if defined(M_UNIX) || defined(__hpux)
int ConsolePipe(class consoleClass  *self)
    {
    int i; 
    int flags;
    int mask, *newchild;
    int pfds[2];
    extern int children[];

    mydbg(("entering: ConsolePipe\n"));
    pipe(pfds);

    mask = sigblock(1 << (SIGCHLD - 1));
    /* loop up until the end of children, the last element is a -1
        added a bounds check here  --MKM */
    i = 0;
    for (newchild = children; *newchild > 0 && i < MAXCHILDPIDS; newchild++) {
	i++;
    }
/* why is the first element always set to a -1? This looks likea kludge to me.--MKM*/
/*    if (*newchild == -1){
	newchild[1] = -1;
    } */
    /* if we're forking getconsole */
    if ((*newchild = osi_vfork()) == 0) {
	NEWPGRP();
	/* 1 == stdout */
	if (dup2 (pfds[1], 1) == -1){ 
	    perror("console: dup2 failed while forking console mon\n");
	    _exit(-1);
	}
	if (close (pfds[0]) == -1){
	    perror("console:<child> close of pfds[0] failed\n");
	}
	if (close (pfds[1]) == -1){
	    perror("console: close of pfds[1] failed\n");
	}
	for (i=FDTABLESIZE()-1; i>2; i--) {
	    (void) close(i);
	}
#if defined(M_UNIX) || defined(__hpux) || defined(hpux)
	execl("/bin/cat", "cat", "-u", "/dev/error",0);
	/* it's an error if execl returns */
	perror("execl failed");
#else
        execlp("getconsole", "getconsole", 0);
	/* it's an error if execlp returns */
	perror("execlp failed");
#endif
	_exit(-1);
    } else{
	/* set the child mask; assume that console stats are not forked */
	if (*newchild == -1){
	    *newchild = 0;
	    sigsetmask(mask);
	    ReportInternalError(self, "ERROR: console: fork failed - console error messages will not be monitored");
	    return(-1);
	}
    }
    sigsetmask(mask);
    /* im_AddZombieHandler(pid, getkmemexit, self); */

    if (close (pfds[1]) == -1){
	ReportInternalError(self, "ERROR: console:<parent> close of pfds[1] failed - no Disk or GVMStats will be monitored");
    }
    ErrorsIn = fdopen(pfds[0], "r");
    if (!ErrorsIn){
	ReportInternalError(self, "ERROR: console: fdopen of pfds[0] failed - console error messages will not be monitored");
	return(-1);
    }
    flags = fcntl(fileno(ErrorsIn), F_GETFL, 0);
#if POSIX_ENV
    fcntl(fileno(ErrorsIn), F_SETFL, flags | O_NONBLOCK);
#else
    fcntl(fileno(ErrorsIn), F_SETFL, flags | FNDELAY);
#endif
    im::AddFileHandler(ErrorsIn, CheckErrorsIn, (char *)self, 2);
    return 0;
}
#endif /* M_UNIX || __hpux */
