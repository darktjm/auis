/*********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/config/COPYRITE'     *
\* ********************************************************************** */

/*
	$Disclaimer: 
// Permission to use, copy, modify, and distribute this software and its 
// documentation for any purpose and without fee is hereby granted, provided 
// that the above copyright notice appear in all copies and that both that 
// copyright notice and this permission notice appear in supporting 
// documentation, and that the name of IBM not be used in advertising or 
// publicity pertaining to distribution of the software without specific, 
// written prior permission. 
//                         
// THE COPYRIGHT HOLDERS DISCLAIM ALL WARRANTIES WITH REGARD 
// TO THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF 
// MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL ANY COPYRIGHT 
// HOLDER BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL 
// DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, 
// DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE 
// OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION 
// WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
// 
//  $
*/

#ifndef NORCSID
#define NORCSID
static char rcsid[]="$Header: /afs/cs.cmu.edu/project/atk-src-C++/atk/basics/common/RCS/event.C,v 3.4 1994/12/13 20:35:03 rr2b Stab74 $";
#endif


 

#include <andrewos.h> /* sys/time.h */
ATK_IMPL("event.H")

#include <event.H>

static class event *timerQueue = NULL;
static long currSec;			/* seconds since last StartTimer call */
static long tuBase;
static boolean timeInited = FALSE;
static long MSEC10 = event_MSECtoTU(10);

static class event *freeList;


ATKdefineRegistry(event, ATK, NULL);

event::event()
{
#ifdef EVENT_DEBUG
    printf("new event:0x%x\n", this);
#endif /* EVENT_DEBUG */
    this->t = event_ENDOFTIME;
    this->proc = NULL;
    this->procdata = NULL;
    this->next = NULL;
    THROWONFAILURE( TRUE);
}

event::~event()
{
#ifdef EVENT_DEBUG
    printf("delete event:0x%x\n",this);
#endif /* EVENT_DEBUG */
}

class event *event::Allocate()
    {
    register class event *e;

    if (freeList == NULL)
	e = (class event *) malloc(sizeof(class event));
    else  {
	e = freeList;
	freeList = freeList->next;
    }
    return e;
}

void event::Deallocate(class event  *self)
        {
    self->next = freeList;
    freeList = self;
}

void event::Cancel()
    {
    register class event *prev = NULL;
    register class event *x;

    for (x = timerQueue; x != NULL && this != x; x=x->next) 
	prev=x;
    if (x == NULL) return;
    if (prev)
	prev->next = this->next;
    else
	timerQueue = this->next;
    delete this;
}

class event *event::Enqueue(long  time, event_fptr proc, void  *procdata)
                {
    register class event *e;

    e = new event;
    e->t = time;
    e->proc = proc;
    e->procdata = procdata;
    
    /* enqueue in sequence by time of happening */

    if (timerQueue == NULL) {
	timerQueue = e;
    }
    else {
	register class event *prev = NULL,
			*x = timerQueue;

	for (; x != NULL && time > x->t; prev=x, x=x->next) ;
	if (prev) {
	    e->next = prev->next;
	    prev->next=e;
	}
	else {
	    e->next = timerQueue;
	    timerQueue = e;
	}
    }
    return e;
}

void event::ForceNext()
    {
    /* set time so next event will occur
    this routine changes the value that will be
    returned by the next call on now()  */

    if (timerQueue == NULL) return;
    tuBase = timerQueue->t;
}

long event::FirstTime(long  currentTime)
        {
    /* returns the time remaining to first event on queue */

    return ((timerQueue == NULL) ? event_ENDOFTIME : timerQueue->t - currentTime);
}

void event::StartTimer()
    {
    /* start timer for elapsed time
    units are   microseconds >>6  (max of 64000 sec) */

    struct osi_Times tp;
    register class event *e = timerQueue;

    osi_GetTimes(&tp);
    if (timerQueue) 
	if (timeInited) {
	    /* reduce every time by 'now' */

	    register long oldNow, deltaSec;

	    deltaSec = tp.Secs - currSec;
	    currSec += deltaSec;
	    tuBase += event_SECtoTU(deltaSec);
	    oldNow = tuBase + event_USECtoTU(tp.USecs);
	    for (; e; e=e->next)
		e->t -= oldNow;
	} else {
	    /* queue is bogus, clear all times */

	    for ( ; e; e = e->next)
		e->t = 0;
	}
    currSec = tp.Secs;
    tuBase = - event_USECtoTU(tp.USecs);
    timeInited = TRUE;
}

long event::HandleTimer(long  currentTime)
        {
    /* there are elements on timer queue.  process first if it is
    time (or if it will be time within 10 msec).
    return time to wait before next event  */

    register long twait;
    register class event *e = timerQueue;

    if (timerQueue == NULL)
	return event_ENDOFTIME;

    twait = e->t - currentTime;		/* time to wait for next event */
    if (twait > MSEC10)
	return (twait);

    /* handle first event on queue */

    timerQueue = e->next;
    (*e->proc)(e->procdata, currentTime);
    delete e;

    if (timerQueue == NULL) return event_ENDOFTIME;
    twait = timerQueue->t - currentTime - MSEC10;
    return (twait>0 ? twait : 0);
}

long event::Now()
    {
    /* returns time relative to last StartTimer 
    units are   microseconds >>6  (max of 64000 sec) 
    This routine returns timeintervals good for about a day;
    for longer execution times (as in a server) there
    ought to be an ocassional call on StartTimer().
    If times are only used for EnqueueEvent, StartTimer
    can be called at any time, the queue will be updated. */

    struct osi_Times tp;
    register long deltaSec;

    if (! timeInited)
	event::StartTimer(); 
    osi_GetTimes(&tp);
    if ((deltaSec=tp.Secs-currSec) )  {
	/* second hand has skipped to next second
	   record new second hand position and update 
	   the current time value. */

	currSec += deltaSec;
	if (deltaSec>1) 
	    tuBase += event_SECtoTU(deltaSec);
	else tuBase += (1000000>>6);
    }
    return (tuBase + event_USECtoTU(tp.USecs));

}
