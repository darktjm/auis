#ifndef _WORKPROC_H_
#define _WORKPROC_H_
ATK_INTER
/* Copyright 1996 Carnegie Mellon University All rights reserved.
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
#include <observable.H>
class event;

enum WorkProc_Status {
    WorkProc_Working=1,
    WorkProc_Suspended,
    WorkProc_Done,
    WorkProc_Exited,
    WorkProc_Stopped
};

class WorkProc : public observable {
    event *scheduled;
    WorkProc_Status status;
    int exitstatus;
    long interval;
    void CancelSchedule();
    void Schedule();
    static void DoWork(void *pd, long time);
  public:
    ATKregistryEntry *ATKregistry();
    WorkProc(long workinterval=0);
    virtual ~WorkProc();
    virtual void Start();
    virtual WorkProc_Status Stop();
    virtual void Suspend();
    virtual void Resume();
    virtual WorkProc_Status FinishNow();
    virtual WorkProc_Status Process();
    virtual WorkProc_Status OneStep();
    virtual void SetInterval(long workinterval=0);
    virtual void ObservedChanged(observable *changed, long change);
    inline WorkProc_Status WorkProcStatus() {
	return status;
    }
    inline int ExitStatus() {
      return exitstatus;
    }
};

#endif
