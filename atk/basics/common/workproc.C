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

#include <andrewos.h>
ATK_IMPL("workproc.H")
#include <workproc.H>
#include <event.H>
#include <im.H>

ATKdefineRegistryNoInit(WorkProc,observable);

WorkProc::WorkProc(long i) {
    status=WorkProc_Working;
    exitstatus=0;
    interval=i;
    Schedule();
}

void WorkProc::CancelSchedule() {
    if(scheduled) {
	scheduled->Cancel();
	scheduled=NULL;
    }
}
    
WorkProc::~WorkProc() {
    CancelSchedule();
}

WorkProc_Status WorkProc::OneStep() {
    WorkProc_Status newstatus=Process();
    if(newstatus!=status) NotifyObservers(status);
    status=newstatus;
    if(status==WorkProc_Done || status==WorkProc_Exited || status==WorkProc_Stopped) {
	Destroy();
    }
    return newstatus;
}

void WorkProc::DoWork(void *pd, long time) {
    WorkProc *self=(WorkProc *)pd;
    self->scheduled=NULL;
    WorkProc_Status status=self->OneStep();
    if(status!=WorkProc_Done && status!=WorkProc_Exited && status!=WorkProc_Stopped) self->Schedule();
}

void WorkProc::Schedule() { 
    scheduled=im::EnqueueEvent(DoWork, (char *)this, interval);
}

void WorkProc::Start() {
    Reference();  // the object owns itself again
    CancelSchedule();
    Schedule();
}
    
WorkProc_Status WorkProc::Stop() {
    WorkProc_Status oldstatus=status;
    if(status!=WorkProc_Stopped) {
	status=WorkProc_Stopped;
	CancelSchedule();
	NotifyObservers(WorkProc_Stopped);
	Destroy(); // the object no longer owns itself
    }
    return oldstatus;
}

void WorkProc::Suspend() {
    CancelSchedule();
}

void WorkProc::Resume() {
    CancelSchedule();
    Schedule();
}

WorkProc_Status WorkProc::FinishNow() {
    WorkProc_Status oldstatus=status;
    while(status!=WorkProc_Done && status!=WorkProc_Exited && status!=WorkProc_Stopped) {
	status=Process();
    }
    if(status!=oldstatus) NotifyObservers(status);
    Destroy();  // the object no longer owns itself.
    return status;
}

WorkProc_Status WorkProc::Process() {
    return WorkProc_Done;
}

void WorkProc::SetInterval(long i) {
    interval=i;
    if(scheduled) {
	CancelSchedule();
	Schedule();
    }
}

void WorkProc::ObservedChanged(observable *changed, long change) {
    if(change==observable_OBJECTDESTROYED) {
	RemoveObserver(changed);
	CancelSchedule();
	Stop();
    }
}
