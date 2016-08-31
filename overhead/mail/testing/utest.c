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

#include <andrewos.h>
#include <stdio.h>
#include <signal.h>
#include <system.h>
#ifdef HAS_SYSEXITS
#include <sysexits.h>
#endif /* HAS_SYSEXITS */
#include <mail.h>
#include <util.h>

/* Test the UnixError, UnixSignal, and UnixSysExits routines. */
main()
{
    int Val;
    extern int EX_Nerr;

    for (Val = 0; Val < 115; Val++) {
	printf("UnixError(%d) is ``%s''", Val, UnixError(Val));
	if (vdown(Val)) printf(" (Vice down)");
	else if (tfail(Val)) printf(" (temp fail)");
	printf("\n");
    }
    printf("\n");
    for (Val = 0; Val < NSIG+1; Val++)
	printf("UnixSignal(%d) is ``%s''\n", Val, UnixSignal(Val));
    printf("\n");
    printf("UnixSysExits(%d) is ``%s''\n", EX_OK, UnixSysExits(EX_OK));
    for (Val = EX__BASE; Val <= (EX__BASE + EX_Nerr); Val++)
	printf("UnixSysExits(%d) is ``%s''\n", Val, UnixSysExits(Val));
}
