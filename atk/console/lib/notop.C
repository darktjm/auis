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

/* 
 ****************************************************************
 * notop.c -- Routines for instrument console that are NOT
 * shared by the operator's console.
 ****************************************************************
 */

#include <andrewos.h> /* sys/time.h */
#include <consoleClass.H>
#include <console.h>
#include <errno.h>
#include <signal.h>

/* the following char* values get used in:
  1.) ../lib/input.c
  2.) ../lib/setup.c
*/
/* NO_DLL_EXPORT */ const char RealProgramName[] = "Console";
/* NO_DLL_EXPORT */ const char EXTENSION[] = "con";
/* NO_DLL_EXPORT */ const char EXTENSION1[] = "console";
/* NO_DLL_EXPORT */ const char EXTENSION2[] = "Console";


void ChooseColumns(int  numcol)
    {
    mydbg(("entering: ChooseColumns\n"));
}

void ChooseMachines(class consoleClass  *self, const char  *machinelist)
    {
    mydbg(("entering: ChooseMachines\n"));
}

void ConfigureMachines(class consoleClass  *self, int  *Rows , int  *Columns , int  *Machines, boolean  Initialize)
        {
    mydbg(("entering: ConfigureMachines\n"));
    *Rows = *Columns = *Machines = 1;
}

struct datum *BuildDatum(const char  *keyword, int  machine)
        {
    mydbg(("entering: BuildDatum\n"));
    return(&Numbers[ALWAYS]);
}

void OneTimeRemoteInit(class consoleClass  *self)
    {
    mydbg(("entering: OneTimeRemoteInit\n"));
    /* Does nothing if not operator console */
}

void InitializeGetStats(class consoleClass  *self)
    {
    static boolean DidInitDisks = FALSE;
    if ((DoVMStat && ! DidInitGVM) || (DoDiskFreeStat && !DidInitDisks)){
	console_InitStats(self);
	DidInitGVM = TRUE;
	DidInitDisks=TRUE;
    }

}

void InitializeInstruments(class consoleClass  *self)
    {
    static boolean DidInitMail = FALSE, DidInitDirs = FALSE, DidInitPrint = FALSE;

    mydbg(("entering: InitializeInstruments\n"));
    if (DoMailChecking && !DidInitMail) {
	InitMail(self);
	DidInitMail = TRUE;
    }
    if (DoPrintChecking && !DidInitPrint) {
	InitPrint(self);
	DidInitPrint = TRUE;
    }
    if (DoDirChecking && !DidInitDirs) {
	InitDirectories(); /* In mailmon.c */
	DidInitDirs = TRUE;
    }
}

struct RegionLog *WhichErrorLog(int  i)
    {
    mydbg(("entering: WhichErrorLog\n"));
    return &RegionLogs[ERRORREGIONLOG];
}

void ReInitializeRemoteInstruments() {}
