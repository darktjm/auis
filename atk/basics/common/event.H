ATK_INTER
#ifndef _event_H_
#define _event_H_
/* ********************************************************************** *\
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


 

#define event_VERSION 1

#include <ATK.H>

    typedef void (*event_fptr)(void *pd, long time);

class event : public ATK   {
public:
	virtual ATKregistryEntry *ATKregistry();


    virtual void Cancel();
    event();
    virtual ~event();
    static class event * Enqueue(long time, event_fptr proc, void *procdata)  ;
    static void ForceNext();
    static long FirstTime(long currentTime)  ;
    static void StartTimer();
    static long HandleTimer(long currentTime)  ;
    static long Now()  ;
    static class event * Allocate()  ;
    static void Deallocate(class event *self);

    long t;		/* time of evenet in microsec >> 6 */
    event_fptr proc;	/* procedure to call for event */
    void *procdata;	/* data to be passed to proc */
    class event *next;
};

#define event_TUtoSEC(x)  ((x)/(1000000>>6))
#define event_TUtoUSEC(x) ((x)<<6)
#define event_TUtoMSEC(x) (((x)<<3)/(1000>>3))
#define event_SECtoTU(x)  ((x)*(1000000>>6))
#define event_USECtoTU(x) ((x)>>6)
#define event_MSECtoTU(x) (((x)*(1000>>3))>>3)
#define event_ENDOFTIME 2000000000


#endif /* _event_H_ */
