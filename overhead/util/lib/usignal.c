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
