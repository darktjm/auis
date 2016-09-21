ATK_INTER
#ifndef _event_H_
#define _event_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

/** \addtogroup libbasics
 * @{ */
#define event_VERSION 1

#include <ATK.H>

    typedef void (*event_fptr)(void *pd, long time);

class event : public ATK   {
public:
	virtual ATKregistryEntry *ATKregistry();


    virtual void Cancel();
    event();
    virtual ~event();
    static class event * Enqueue(long time, event_fptr proc, const void *procdata)  ;
    static void ForceNext();
    static long FirstTime(long currentTime)  ;
    static void StartTimer();
    static long HandleTimer(long currentTime)  ;
    static long Now()  ;
    static class event * Allocate()  ;
    static void Deallocate(class event *self);

    long t;		/* time of evenet in microsec >> 6 */
    event_fptr proc;	/* procedure to call for event */
    const void *procdata;	/* data to be passed to proc */
    class event *next;
};

#define event_TUtoSEC(x)  ((x)/(1000000>>6))
#define event_TUtoUSEC(x) ((x)<<6)
#define event_TUtoMSEC(x) (((x)<<3)/(1000>>3))
#define event_SECtoTU(x)  ((x)*(1000000>>6))
#define event_USECtoTU(x) ((x)>>6)
#define event_MSECtoTU(x) (((x)*(1000>>3))>>3)
#define event_ENDOFTIME 2000000000
/** @} */
#endif /* _event_H_ */
