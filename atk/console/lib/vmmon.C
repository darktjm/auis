/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

#include <andrewos.h>
#include <im.H>
#include <consoleClass.H>
#include <console.h>

#include <fcntl.h>
#include <signal.h>



extern int children[]; /* Make getstats a bona-fide controlled child */
  /*
  * NOTE:  All references to the array of children are copied from code
  * in setup.c:SetupFromConsoleFile case FLAG_INITEXEC.
  * If the child management code changes, change this here too.
  * 12/29/88 wdc@athena.mit.edu
  */
 

/*
  * Call a small - secure setuid root/setgid kmem program to collect the stats
  * from /dev/kmem - and have them sent back through a pipe and
  * processed in ComputeStatistics - in vmmonf.c
  */

void getkmemexit(int  pid, class consoleClass  *self);


void getkmemexit(int  pid, class consoleClass  *self)
{
    mydbg(("entering: getkmemexit\n"));
    /* no-op -- do I really need this?  Can I just use NULL for the function pointer
	in the im_AddZombieHandler call? */
}    
extern int venusSocket;
int console_InitStats(class consoleClass  *self)
    {
    /* pfds = Pipe File DescriptorS */
    int pfds[2];
    /* added the following counter variable for bounds checking the array --MKM */
    int i; 
    FILE *vmstats;
    int flags;
    const char *myargv[5];
    char userid[50], freq1[50], freq2[50];
    int *newchild;
    SIGSET_TYPE nmask, omask;

    mydbg(("entering: InitStats\n"));
    pipe(pfds);
#ifdef POSIX_ENV
     (void) sigemptyset(&nmask);
     (void) sigaddset(&nmask, SIGCHLD);
    if (sigprocmask(SIG_BLOCK, &nmask, &omask) < 0)
	 perror("sigprocmask");
#else
    omask = sigblock(1 << (SIGCHLD - 1));
#endif
    /* loop up until the end of children, the last element is a -1
        added a bounds check here  --MKM */
    i = 0;
    for (newchild = children; *newchild > 0 && i < MAXCHILDPIDS; newchild++)
	i++;
/* why is the first element always set to a -1? This looks likea kludge to me.--MKM*/
/*    if (*newchild == -1){
	newchild[1] = -1;
    } */
    /* if we're forking getstats */
    if ((*newchild = osi_vfork()) == 0) {
	/* since the pid is 0, then all calls apply to this process group */
                theGetstatsPid = getpid();
		NEWPGRP();
	/* 1 == stdout */
	if (dup2 (pfds[1], 1) == -1){ 
	    perror("console: dup2 failed while forking getstats\n");
	    _exit(-1);
	}
	if (close (pfds[0]) == -1){
	    perror("console:<child> close of pfds[0] failed\n");
	}
	if (close (pfds[1]) == -1){
	    perror("console: close of pfds[1] failed\n");
	}
#ifdef AFS_ENV
	close(venusSocket);
#endif
	myargv[0] = "getstats";
	sprintf(userid, "%d", getuid());
	myargv[1] = userid;
	sprintf(freq1, "%d", VMPollFreq);
	sprintf(freq2, "%d", DiskPollFreq);
	myargv[2] = freq1;
	myargv[3] = freq2;
	myargv[4] = NULL;
	execvp("getstats", (char **)myargv);
	/* it's an error if execvp returns */
	perror("execvp failed");
	_exit(-1);
    } else{
	/* set the child mask; assume that console stats are not forked */
	if (*newchild == -1){
	    *newchild = 0;
#ifdef POSIX_ENV
	    (void) sigprocmask(SIG_SETMASK, &omask, (sigset_t *)NULL);
#else
	    sigsetmask(omask);
#endif
	    ReportInternalError(self, "ERROR: console: fork failed - no Disk or GVMStats will be monitored");
	    return(0);
	}
    }
#ifdef POSIX_ENV
    (void) sigprocmask(SIG_SETMASK, &omask, (sigset_t *)NULL);
#else
    sigsetmask(omask);
#endif
    /* im_AddZombieHandler(pid, getkmemexit, self); */

    if (close (pfds[1]) == -1){
	ReportInternalError(self, "ERROR: console:<parent> close of pfds[1] failed - no Disk or GVMStats will be monitored");
    }
    vmstats = fdopen(pfds[0], "r");
    if (!vmstats){
	ReportInternalError(self, "ERROR: console: fdopen of pfds[0] failed - no Disk or GVMStats will be monitored");
	return(0);
    }
    flags = fcntl(fileno(vmstats), F_GETFL, 0);
#if POSIX_ENV
    fcntl(fileno(vmstats), F_SETFL, flags | O_NONBLOCK);
#else
    fcntl(fileno(vmstats), F_SETFL, flags | FNDELAY);
#endif
    im::AddFileHandler(vmstats,(im_filefptr)ComputeStatistics, (char *)self, 1);
    return 0;
}
