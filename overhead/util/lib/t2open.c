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

#include <andrewos.h>		/* sys/time.h */
#include <util.h>
#include <stdio.h>
#include <signal.h>
#include <setjmp.h>
#include <errno.h>
static int *popen_pid = NULL;
static int dtablesize = -1;

typedef SIGNAL_RETURN_TYPE SignalReturnType;

#ifdef __STDC__
static SignalReturnType (*oldfunc)(int);
#else
static SignalReturnType (*oldfunc)();
#endif

int t2open(const char *name, char * const argv[], FILE **r, FILE **w)
{
#ifndef HAVE_FORK
    printf("t2open in libutil not implemented on this platform.\n");
    return -1;
#else
    int p1[2], p2[2], pgrp;

    if (popen_pid == NULL) {
	dtablesize = FDTABLESIZE();
	if (dtablesize > 0) {
	    popen_pid = (int *) malloc(dtablesize * sizeof(int));
	}
    }
    if (popen_pid == NULL) {errno = ENOMEM; return -1;}

    /* Set up to ignore broken pipe signals */
    oldfunc = signal(SIGPIPE, SIG_IGN);

    if(pipe(p1) < 0 || pipe(p2) < 0) return -1;
    /* p1 is for writing from child to parent, p2 vice versa */

    pgrp = osi_vfork();
    if(pgrp == 0) {
	int fd;

	NEWPGRP();

	/* Set up my write side */
	close(p1[0]);		/* I don't read from p1 */
	dup2(p1[1], 1);		/* My stdout is now p1 */
	close(p1[1]);

	/* Set up my read side */
	close(p2[1]);
	dup2(p2[0], 0);
	close(p2[0]);

	for (fd = dtablesize; fd > 2; --fd) close(fd);
	execv(name, argv);
	_exit(0377);
    }
    if (pgrp == -1) return -1;	/* VFORK failed */

    /* I am parent */
    popen_pid[p2[1]] = pgrp;	/* Index by write descriptor */

    /* Set up my write side */
    close(p2[0]);
    *w = fdopen(p2[1], "w");
    if (*w == NULL) return -1;

    /* Set up read side */
    close(p1[1]);
    *r = fdopen(p1[0], "r");
    if (*r == NULL) return -1;
    return pgrp;
#endif
}

static jmp_buf env;

static 
#if defined(__STDC__)
void lclalarm(int i)
#else
lclalarm()
#endif
{
    longjmp(env, 1);
}

int t2close(FILE *ptr, int seconds, int *timedout)
{
#ifndef HAVE_FORK
    printf("t2close in libutil not implemented on this platform.\n");
    return -1;
#else
	int status;
	SIGSET_TYPE omask, nmask;
    register int f, r;
    struct itimerval val;

#ifdef __STDC__
    SignalReturnType (*old)(int);
#else
    SignalReturnType (*old)();
#endif

    f = fileno(ptr);
    fclose(ptr);
#ifdef POSIX_ENV
    (void) sigemptyset(&nmask);
    (void) sigaddset(&nmask, SIGINT);
    (void) sigaddset(&nmask, SIGQUIT);
    (void) sigaddset(&nmask, SIGHUP);
    (void) sigaddset(&nmask, SIGCHLD);
    if (sigprocmask(SIG_BLOCK, &nmask, &omask) < 0)
	perror("sigprocmask");
#else
#define	mask(s)	(1<<((s)-1))
    omask = sigblock(mask(SIGINT)|mask(SIGQUIT)|mask(SIGHUP)|mask(SIGCHLD));
#endif

    if (timedout != 0) {
	/* Enable signal handler */
	old = signal(SIGALRM, lclalarm);

	/* Set timer */
	val.it_interval.tv_sec = 0;
	val.it_interval.tv_usec = 0;
	val.it_value.tv_sec = seconds;
	val.it_value.tv_usec = 0;
	setitimer(ITIMER_REAL, &val, 0);

	if (setjmp(env) == 0) {
	    while((r = wait(&status)) != popen_pid[f] && r != -1) ;

	    /* Turn off timer */
	    val.it_value.tv_sec = 0;
	    val.it_value.tv_usec = 0;
	    setitimer(ITIMER_REAL, &val, 0);
	    *timedout = 0;
	} else {
	    /* Timer expired */
	    status = -1;
	    *timedout = 1;
	}
	(void) signal(SIGALRM, old);	/* Restore old value */
    } else {
	while ((r=wait(&status)) != popen_pid[f] && r != -1) ;
    }

#ifdef POSIX_ENV
    (void) sigprocmask(SIG_SETMASK, &omask, (sigset_t *)NULL);
#else
    sigsetmask(omask);
#endif


    /* Reenable old SIGPIPE action */
    (void) signal(SIGPIPE, oldfunc);

    return status;
#endif
}
