#ifndef _WORKPROC_H_
#define _WORKPROC_H_
ATK_INTER
/* Copyright 1996 Carnegie Mellon University All rights reserved. */
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
