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
static char rcsid[]="$Header: /afs/cs.cmu.edu/project/atk-src-C++/atk/console/lib/RCS/netmon.C,v 1.2 1993/05/19 16:23:12 rr2b Stab74 $";
#endif


 

/*
	Network monitoring routines for Instrument Console.

 */

#include <andrewos.h>
#include <stdio.h>
#include <console.h>

#ifndef NORCSID
#endif
void CheckNet();
void InitNet();


void CheckNet()
{
    short i;

    mydbg(("entering: CheckNet\n"));
    i = NETRESPONSES;
    if (Numbers[i].IsDisplaying) {
#if 0
	NewValue(self, &Numbers[i], 5, NULL, FALSE);
#else
	printf("Network monitoring doesn't work.\n");
#endif
    }
}

void InitNet()
{
    mydbg(("entering: InitNet\n"));
    Numbers[NETRESPONSES].RawText = (char *)malloc(20);
    strcpy(Numbers[NETRESPONSES].RawText, "Foobar");
}
