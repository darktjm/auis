/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/config/COPYRITE'     *
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

#ifndef NORCSID
#define NORCSID
static UNUSED const char rcsid[]="$Header: /afs/cs.cmu.edu/project/atk-src-C++/atk/console/lib/RCS/notop.C,v 1.1 1993/05/25 21:29:58 gk5g Stab74 $";
#endif


 

/* 
 ****************************************************************
 * notop.c -- Routines for instrument console that are NOT
 * shared by the operator's console.
 ****************************************************************
 */


#include <consoleClass.H>
#include <console.h>
#include <errno.h>
#include <signal.h>

/* the following char* values get used in:
  1.) ../lib/input.c
  2.) ../lib/setup.c
*/
char *RealProgramName = "Console";
char EXTENSION[] = "con";
char EXTENSION1[] = "console";
char EXTENSION2[] = "Console";



#ifndef NORCSID
#endif
void ChooseColumns(int  numcol);
void ChooseMachines(class consoleClass  *self, char  *machinelist);
void ConfigureMachines(class consoleClass  *self, int  *Rows , int  *Columns , int  *Machines, boolean  Initialize);
struct datum *BuildDatum(char  *keyword, int  machine);
void OneTimeRemoteInit(class consoleClass  *self);
void InitializeGetStats(class consoleClass  *self);
void InitializeInstruments(class consoleClass  *self);
struct RegionLog *WhichErrorLog(int  i);
void ReInitializeRemoteInstruments();


void ChooseColumns(int  numcol)
    {
    mydbg(("entering: ChooseColumns\n"));
}

void ChooseMachines(class consoleClass  *self, char  *machinelist)
    {
    mydbg(("entering: ChooseMachines\n"));
}

void ConfigureMachines(class consoleClass  *self, int  *Rows , int  *Columns , int  *Machines, boolean  Initialize)
        {
    mydbg(("entering: ConfigureMachines\n"));
    *Rows = *Columns = *Machines = 1;
}

struct datum *BuildDatum(char  *keyword, int  machine)
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
    if (DoVenusMonitoring && ! DidInitVenus) {
	InitializeMariner(self);
	DidInitVenus = TRUE;
    }
    if (! DidInitVenus) {
/*	VenusIn = winin; */
	VenusIn = NULL; /* ??? */
    }
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
