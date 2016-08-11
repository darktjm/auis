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


 

/* 
 * global initialization needed for dynamically loaded console
 */

/* Below is taken from original console.c -- ghoti */

ConLibType conlib[MAXCONFILES];
char *libpaths[MAXLIBS];

long libnum = 0;
long connum = 0;
long BasicLib = -1;
long LocalLib = -1;
long UserLib = -1;

char ConFile[1024] = "";
int MYDEBUGGING = FALSE;
struct fontdesc *FontsAvail[MAXNUMFONTS];
char *AvailFontNames[MAXNUMFONTS] = { "" };
int AvailFontPts[MAXNUMFONTS];
int FontCount = 0;
int ExternalsInUse = 0;
FILE *VenusIn, *ErrorsIn, *ConsoleIn;
char ErrTxt[256] = "";
char *Nullity = "";
char *StatusServer = NULL; /* for vopcon server machine */
int ScaleFactor = 100;
int VMPollCt = 1,
    DiskPollCt = 1,
    MailPollCt = 1,
    PrintPollCt = 1,
    DirPollCt = 1,
    WindowPollCt = 1,
    NetPollCt = 1,
    VenusPollCt = 1,
    ClockPollCt = 1,
    FPAPollCt = 1;
boolean ErrorInputPending = FALSE;
boolean ConsoleSocketInputPending = FALSE;
boolean VenusInputPending = FALSE;
boolean WindowInputPending = FALSE;
int LastMailMod= 0;
boolean DidInitGVM = FALSE,
    DidInitVenus = FALSE,
    DidInitPrinting = FALSE,
    DidInitNet = FALSE,
    DidInitErrLog = FALSE;
char *envmail = NULL;
char FontFamily[20] = "andysans";
struct RegionLog RegionLogs[NUMREGIONLOGS + 1] = {NULL};
struct InternalVariable IntrnlVars[NUMINTERNALVARIABLES + 1];
struct datum Numbers[DisplayTypeCount + 1];
struct display *VeryFirstDisplay;
struct fontdesc *PromptFont;
struct fontdesc *EventFont;
char PromptFontName[50] = "andysans10";


display_pdffptr prefunctions[] = {
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



display_dffptr functions[] = {
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

int DefaultErrorPriority = 4,
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
    VenusPollFreq = 30,
    ClockPollFreq = 2,
    FPAPollFreq = 3600;
boolean DoTroubleChecking = FALSE,
    DoVenusChecking = FALSE,
    DoVMStat = FALSE,
    DoDiskFreeStat = FALSE,
    DoVenusMonitoring = FALSE,
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
char *xPaths[MAXCONSOLEPATHS] = { (char *) NULL };
char *xNames[MAXCONSOLEPATHS] = { (char *) NULL };
char *ConLib[MAXCONSOLES] = { NULL };
int ConNum = 0;
int IntVarCount[NUMINTERNALVARIABLES + 1];

