/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

/*
	usignal.c -- Return a static string describing a signal value.
*/

#include <andrewos.h>
#include <util.h>
 
#include <errno.h>
#include <signal.h>

const char *UnixSignal(int signalNumber)
{
/* Returns a pointer to a static buffer containing English text describing the same signal condition that signalNumber describes (interpreted as a Unix signal number).  The text has no newlines in it.  We contend that this is what ``psignal'' should have been returning all along. */
    static char SigBuff[40];
#if SY_B4x
    extern char *sys_siglist[];

    if (signalNumber < NSIG && signalNumber > 0) return sys_siglist[signalNumber];
#endif /* ! SYSV */
    sprintf(SigBuff, "Signal number %d", signalNumber);
    return SigBuff;
}
