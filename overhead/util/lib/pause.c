/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

/*
		pause.c -- test and set the Computer Science pause-on-disk-full facility

	SetPauseOff() -- turn off disk-full-pausing facility
	SetPauseOn() -- turn on disk-full-pausing facility
	PauseP() -- return non-zero if the disk-full-pausing facility is enabled.
*/

#include <andrewos.h>
#include <errno.h>
#include <stdio.h>
#include <sys/stat.h>
#include <signal.h>
/* #include <sys.h> DMT commented out */

static int DoCall(Action, OldP)
int Action, *OldP;
{/* Do the Computer Science Department's ``rpause'' system call for ENOSPC. */
#define	RPAUSE_SAME	0		/* leave state unchanged */
#define	RPAUSE_DISABLE	1	/* disable pause on error */
#define	RPAUSE_ENABLE	2	/* enable pause on error */

#define	RPAUSE_ALL	0x7fffffff		/* all error number types */
#if !defined(CMUCS)
    return 0;
#else /* !defined(CMUCS) */

#ifdef POSIX_ENV
    struct sigaction OldS, NewS;
#else
    struct sigvec OldS, NewS;
#endif
    int Err;

#ifdef POSIX_ENV
    NewS.sa_handler = SIG_IGN;
    sigemptyset(&NewS.sa_mask);
    NewS.sa_flags = 0;
    if (sigaction(SIGSYS, &NewS, &OldS) != 0) return -1;
#else
    NewS.sv_handler = SIG_IGN;
    NewS.sv_mask = 0;
    NewS.sv_onstack = 0;
    if (sigvec(SIGSYS, &NewS, &OldS) != 0) return -1;
#endif

    *OldP = syscall(-5, ENOSPC, RPAUSE_ALL, Action);
    Err = errno;

#ifdef POSIX_ENV
    sigaction(SIGSYS, &OldS, NULL);
#else
    sigvec(SIGSYS, &OldS, NULL);	/* Reset old; don't want prior state */
#endif

    errno = Err;
    return (*OldP >= 0);
#endif /* !defined(CMUCS) */
}

int SetPauseOff()
{/* Set the pause facility off. */
    int Dum;
    return DoCall(RPAUSE_DISABLE, &Dum);
}

int SetPauseOn()
{/* Set the pause facility on. */
    int Dum;
    return DoCall(RPAUSE_ENABLE, &Dum);
}

int PauseP()
{/* Tell if the pause facility is on. */
    int Old;

    Old = 0;	/* default: wasn't on. */
    DoCall(RPAUSE_SAME, &Old);
    return (Old == RPAUSE_ENABLE ? 1 : 0);
}

#ifdef TESTINGONLYTESTING
main ()
{
    printf("PauseP() returns %d/%d\n", PauseP(), errno);
    printf("SetPauseOn() returns %d/%d\n", SetPauseOn(), errno);
    printf("PauseP() returns %d/%d\n", PauseP(), errno);
    printf("SetPauseOff() returns %d/%d\n", SetPauseOff(), errno);
    printf("PauseP() returns %d/%d\n", PauseP(), errno);
}
#endif /* TESTINGONLYTESTING */
