/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

#include <andrewos.h> /* sys/time.h */

/* 
 * notop.c -- Routines for instrument console that are
 * NOT shared by the operator's console.
 */


#include <consoleClass.H>
#include <im.H>
#include <event.H>
#include <console.h>
#include <errno.h>
#include <signal.h>


void WakeUp(class consoleClass  *self);


void WakeUp(class consoleClass  *self)
    {
    mydbg(("entering: WakeUp\n"));
    if (!PauseEnqueuedEvents){
	if (!RingingAlarm){
	    if (DoVMStat && ++VMPollCt > VMPollFreq) {
		VMPollCt = 1;
/*		ComputeStatistics(self);*//*handled directly by im_AddFileHandler in vmmon.c:InitStatistics ? */
	    }
	    if (DoDiskFreeStat && ++DiskPollCt > DiskPollFreq) {
		DiskPollCt = 1;
/*		FindFreeDiskSpace(self);*//*handled directly by im_AddFileHandler in vmmon.c:InitStatistics ? */
	    }
	    if (DoMailChecking && ++MailPollCt > MailPollFreq) {
		MailPollCt = 1;
		CheckMail(self, FALSE);
	    }
	    if (DoPrintChecking && ++PrintPollCt > PrintPollFreq) {
		PrintPollCt = 1;
		CheckPrint(self);	/* In mailmon.c, for no good reason */
	    }
	    if (DoDirChecking && ++DirPollCt > DirPollFreq) {
		DirPollCt = 1;
		CheckDirectories(self);	/* In mailmon.c */
	    }
	    if (DoCheckClock && ++ClockPollCt > ClockPollFreq) {
		ClockPollCt = 1;
		CheckClock(self);
	    }
	    if (++FPAPollCt > FPAPollFreq){
		FPAPollCt = 1;
		CheckFPA(self);
	    }
	}
	else
	    NewValue(self, &Numbers[CLOCKALL], Numbers[CLOCKALL].Value, NULL, TRUE);
	/* That last line is a hack so that the * alarm flashes as often as
	 possible */
	if (DoTroubleChecking) {
	    CheckTrouble(self);
	}
    }
    im::EnqueueEvent((event_fptr)WakeUp, (char *)self, event_SECtoTU(Period));
}

