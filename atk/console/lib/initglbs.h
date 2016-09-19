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
 * global initialization needed for dynamically loaded console
 */

/* Below is taken from original console.c -- ghoti */

NO_DLL_EXPORT ConLibType conlib[MAXCONFILES];
NO_DLL_EXPORT char *libpaths[MAXLIBS];

NO_DLL_EXPORT long libnum = 0;
NO_DLL_EXPORT long connum = 0;
NO_DLL_EXPORT long BasicLib = -1;
NO_DLL_EXPORT long LocalLib = -1;
NO_DLL_EXPORT long UserLib = -1;

NO_DLL_EXPORT char ConFile[1024] = "";
NO_DLL_EXPORT int MYDEBUGGING = FALSE;
NO_DLL_EXPORT struct fontdesc *FontsAvail[MAXNUMFONTS];
NO_DLL_EXPORT const char *AvailFontNames[MAXNUMFONTS] = { "" };
NO_DLL_EXPORT int AvailFontPts[MAXNUMFONTS];
NO_DLL_EXPORT int FontCount = 0;
NO_DLL_EXPORT int ExternalsInUse = 0;
NO_DLL_EXPORT FILE *ErrorsIn, *ConsoleIn;
NO_DLL_EXPORT char ErrTxt[256] = "";
NO_DLL_EXPORT char Nullity[] = "";
NO_DLL_EXPORT char *StatusServer = NULL; /* for vopcon server machine */
NO_DLL_EXPORT int ScaleFactor = 100;
NO_DLL_EXPORT int VMPollCt = 1,
    DiskPollCt = 1,
    MailPollCt = 1,
    PrintPollCt = 1,
    DirPollCt = 1,
    WindowPollCt = 1,
    NetPollCt = 1,
    ClockPollCt = 1,
    FPAPollCt = 1;
NO_DLL_EXPORT boolean ErrorInputPending = FALSE;
NO_DLL_EXPORT boolean ConsoleSocketInputPending = FALSE;
NO_DLL_EXPORT boolean WindowInputPending = FALSE;
NO_DLL_EXPORT int LastMailMod= 0;
NO_DLL_EXPORT boolean DidInitGVM = FALSE,
    DidInitPrinting = FALSE,
    DidInitNet = FALSE,
    DidInitErrLog = FALSE;
NO_DLL_EXPORT char *envmail = NULL;
NO_DLL_EXPORT char FontFamily[20] = "andysans";
NO_DLL_EXPORT struct RegionLog RegionLogs[NUMREGIONLOGS + 1] = {NULL};
NO_DLL_EXPORT struct InternalVariable IntrnlVars[NUMINTERNALVARIABLES + 1];
NO_DLL_EXPORT struct datum Numbers[DisplayTypeCount + 1];
NO_DLL_EXPORT struct display *VeryFirstDisplay;
NO_DLL_EXPORT struct fontdesc *PromptFont;
NO_DLL_EXPORT struct fontdesc *EventFont;
NO_DLL_EXPORT char PromptFontName[50] = "andysans10";


NO_DLL_EXPORT display_pdffptr prefunctions[] = {
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    (display_pdffptr)PreLogError,
    (display_pdffptr)PreLogReport,
    (display_pdffptr)PreLogUser,
    (display_pdffptr)PreLogSilly,
    NULL,
    NULL,
    NULL
};



NO_DLL_EXPORT display_dffptr functions[] = {
    (display_dffptr)DrawDebug,
    (display_dffptr)DrawDebug,
    (display_dffptr)DrawEKGGraph,
    (display_dffptr)DrawGauge,
    (display_dffptr)DrawDial,
    (display_dffptr)DrawIndicator,
    (display_dffptr)DrawBarGraph,
    (display_dffptr)RingAlarm,
    (display_dffptr)SignalTrouble,
    (display_dffptr)LogError,
    (display_dffptr)LogReport,
    (display_dffptr)LogUser,
    (display_dffptr)LogSilly,
    (display_dffptr)DrawLog,
    (display_dffptr)DrawTitle,
    (display_dffptr)DrawNothing
};

NO_DLL_EXPORT int DefaultErrorPriority = 4,
    HighestPriority = 4,
    Period = 1,
    MinHeight = 500,
    MaxHeight = 500,
    MinWidth = 500,
    MaxWidth = 500,
    VMPollFreq = 2,
    DiskPollFreq = 60,
    MailPollFreq = 60,
    PrintPollFreq = 60,
    DirPollFreq = 60,
    WindowPollFreq = 60,
    NetPollFreq = 60,
    ClockPollFreq = 2,
    FPAPollFreq = 3600;
NO_DLL_EXPORT boolean DoTroubleChecking = FALSE,
    DoVMStat = FALSE,
    DoDiskFreeStat = FALSE,
    DoMailChecking = FALSE,
    DoPrintChecking = FALSE,
    DoDirChecking = FALSE,
    DoCheckClock = FALSE,
    DoWindows = FALSE,
    DoLOAD = FALSE,
    DoCPU = FALSE,
    DoVM = FALSE,
    DoPAGING = FALSE,
    DoMEM = FALSE,
    DoQUEUE = FALSE,
    DoINTS = FALSE,
    DoNDSTAT = FALSE,
    DoNetChecking = FALSE,
    DoPROCESSES = FALSE,
    PauseEnqueuedEvents = FALSE,
    FPAchecked = FALSE,
    RingingAlarm = FALSE;

/* added to handle expanded path names  --MKM */
NO_DLL_EXPORT char *xPaths[MAXCONSOLEPATHS] = { (char *) NULL };
NO_DLL_EXPORT char *xNames[MAXCONSOLEPATHS] = { (char *) NULL };
NO_DLL_EXPORT char *ConLib[MAXCONSOLES] = { NULL };
NO_DLL_EXPORT int ConNum = 0;
NO_DLL_EXPORT int IntVarCount[NUMINTERNALVARIABLES + 1];
