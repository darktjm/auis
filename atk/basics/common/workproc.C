/* Copyright 1996 Carnegie Mellon University All rights reserved. */

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
    if(change==observable::OBJECTDESTROYED) {
	RemoveObserver(changed);
	CancelSchedule();
	Stop();
    }
}
