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
static char rcsid[]="$Header: /afs/cs.cmu.edu/project/atk-src-C++/overhead/util/lib/RCS/nicetime.c,v 2.12 1992/12/15 21:10:10 rr2b Stab74 $";
#endif


 

#include <stdio.h>
#include <andrewos.h>		/* sys/time.h */

char *NiceTime(Time)
long int Time;
{/* Like ctime, but do a more attractive job and don't end with a newline. */
    static char Res[50];
    static char *Mon[] = {	"Jan", "Feb", "Mar", "Apr", "May", "Jun",
    "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
    struct tm *This;

    This = localtime(&Time);
    osi_SetZone();
    sprintf(Res, "%d %s %04d at %d:%02d:%02d %s",
	     This->tm_mday,
	     Mon[This->tm_mon],
	     This->tm_year + 1900,
	     This->tm_hour,
	     This->tm_min,
	     This->tm_sec,
	     osi_ZoneNames[(osi_IsEverDaylight && This->tm_isdst ? 1 : 0)]);
    return Res;
}
