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


 

#define event_VERSION 1

class event  {
methods:
    Cancel();
classprocedures:
    Enqueue(long time, procedure proc, char *procdata) returns struct event *;
    ForceNext();
    FirstTime(long currentTime) returns long;
    StartTimer();
    HandleTimer(long currentTime) returns long;
    Now() returns long;
    Allocate() returns struct event *;
    Deallocate(struct event *self);
data:
    long t;		/* time of evenet in microsec >> 6 */
    int (*proc)();	/* procedure to call for event */
    char *procdata;	/* data to be passed to proc */
    struct event *next;
};

#define event_TUtoSEC(x)  ((x)/(1000000>>6))
#define event_TUtoUSEC(x) ((x)<<6)
#define event_TUtoMSEC(x) (((x)<<3)/(1000>>3))
#define event_SECtoTU(x)  ((x)*(1000000>>6))
#define event_USECtoTU(x) ((x)>>6)
#define event_MSECtoTU(x) (((x)*(1000>>3))>>3)
#define event_ENDOFTIME 2000000000

