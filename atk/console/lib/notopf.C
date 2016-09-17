/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'     *
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

