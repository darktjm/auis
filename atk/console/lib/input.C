/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see: andrew/config/
******************************************************************* */

/* 
 ***************************************************************
 * Routines for handling kb & mouse input in Instrument Console
 **************************************************************
 */

/* 
 * By using conclass.eh - I am effectively including the entire
 * consoleClass object within this code - all procedures called via the
 * menuproc will autmatically have the first argument be the object
 * "self"
 */

#include <andrewos.h> /* sys/types.h sys/time.h */
#define AUXMODULE 1
#include <consoleClass.H>

#include <menulist.H>
#include <im.H>
#include <view.H>
#include <fontdesc.H>
#include <proctable.H>
#include <cursor.H>
#include <graphic.H>
#include <filetype.H> /* for ~ expansion */
#include <rect.h>
#include <console.h>
#include <convers.h>
#include <sitevars.h>
#include <sys/param.h>

#include <util.h>





extern int Pposx, Pposy;
extern char Pstring1[256], Pstring2[256], Pstring3[256], Pstring4[MAXPATHLEN];





/* EXTENSION(simple, 1, and 2) get set for:
    console files in: ../cmd/notop.c
*/

#define MAXPATHLENGTH 1024
#define MAXMENUCARD 6

/* | mask = turn on, & ~mask = turn off */
#define	STD_MASK	0   /* standard menulist */
#define	ALR_OFF_MASK	1   /* Alarm -is- off, Set Alarm menu -is- showing */
#define	ALR_ON_MASK	2   /* Alarm -is- set, Turn Off Alarm -is- showing */
#define	SHR_MASK	4   /* Menus -are- shrunk, Expand Menus -is- showing */
#define	EXP_MASK	8   /* Menus -are- expanded, Shrink Menus -is- showing */
#define	DEB_OFF_MASK   16   /* Debugging -is- off, Turn On Debugging -is- showing */
#define	DEB_ON_MASK    32   /* Debugging -is- on, Turn Off Debugging -is- showing */
#define	GETSTAT_MASK   64   /* Getstats -is- dead, Restart Getstats -is- showing */

#define MAXMATCHES 2
#define MAXEXTLTH 7

#ifdef NOTUSED
static char *PriorityTexts[] = { 
    "Urgent",
    "Error",
    "Error",
    "Error",
    "Error",
    "Warning",
    "Warning",
    "Debug",
    "Debug",
    "Debug",
    0
};
#endif /* NOTUSED */



void DebugMenu(class ATK *self, char  *rock);
void DoQuit(class ATK *self, char  *rock);
char *TitleFromFile(char  *fname, boolean  IncludeVersion);
void ReadNewConsoleFile(class ATK *self, char  *rock);
int adjmon(int  *mday , int  max , int  *mon);
void SetAlarm(class ATK *self, char  *rock);
void TurnOffAlarm(class ATK *self, char  *rock);
struct display *FindInstrument(class consoleClass  *self, int  x, int  y);
int CheckMovingX(class consoleClass  *self, int  x , int  y);
void ResizeDisplay(class consoleClass  *self, int  x , int  y , int  LastX , int  LastY , int  MovingX);
void TogVar(class ATK *self, long  rock);
void ExpandMenu(class ATK *self, char  *rock);
void ShrinkMenu(class ATK *self);
void GetStdItems(class menulist  *tempMenulist);
void GetStdConsoles(class menulist  *tempMenulist);
char *GetUserPaths();
void GetExtraConsoles(class menulist  *tempMenulist, const char  *conpath, const char  *cardTitle);
void SetStartUpConsole(char  *path, char  *ConFile);
void FindStartUpConsole(char  *ConFile, boolean  IsStartUp);
void PrepareStdMenus(boolean  IsStartup, class menulist  **stdMenulist, struct ATKregistryEntry   *ClassInfo);
class menulist *PrepareUserMenus(class consoleClass  *self, struct ATKregistryEntry   *ClassInfo);


void DebugMenu(class ATK *atkarg_self, char  *rock)
{

	class consoleClass *self=(class consoleClass *)atkarg_self;
    mydbg(("entering: DebugMenu\n"));
    ToggleDebugging(self, NULL);
    if(MYDEBUGGING){
	self->menuMask &= ~DEB_ON_MASK;
	self->menuMask |= DEB_OFF_MASK;
	(self->stdMenulist)->SetMask( self->menuMask);
    }
    else{
	self->menuMask &= ~DEB_OFF_MASK;
	self->menuMask |= DEB_ON_MASK;
	(self->stdMenulist)->SetMask( self->menuMask);
    }
    (self)->PostMenus( self->stdMenulist);
    (self)->WantUpdate( self);
}


void DoQuit(class ATK *atkarg_self, char  *rock)
{
    mydbg(("entering: DoQuit\n"));
    KillInitExecProcesses(TRUE);
    exit(0);
}


char *TitleFromFile(char  *fname, boolean  IncludeVersion)
{
    char   *s,
        *t;
    int     i;
    static char Title[40];

    mydbg(("entering: TitleFromFile\n"));
    t = NULL;
    s = fname;
    while (*s != '\0') {
        if (*s == '/')
            t = s;
        ++s;
    }
    if (t == NULL) {
        t = fname;
    }
    else {
        ++t;
    }
    s = t;
    while (*s != '\0' && *s != '.') {
        ++s;
    }
    if (*s == '.') {
        i = s - t;
    }
    else {
        i = strlen(t);
    }
    strncpy(Title, t, i);
    Title[i] = '\0';
    if (IncludeVersion) {
        strcat(Title, CONSOLE_VERSION);
    }
    return(Title);
}


extern const char RealProgramName[];
/* consoleName gets referenced in setup.c, in  SetupFromConsole...
    I did this to avoid changing the calling parameters to the proc in setup. */
NO_DLL_EXPORT char *consoleName;

void ReadNewConsoleFile(class ATK *atkarg_self, char  *rock)
{

	class consoleClass *self=(class consoleClass *)atkarg_self;
    consoleName = rock;

    mydbg(("entering: ReadNewConsoleFile\n"));
    if (consoleName == NULL) {
	if(GetConsoleFileFromTypeIn(self, FALSE) == FALSE) {
	    fprintf(stderr,"FATAL error getting the console file type-in!\n");
	    fflush(stderr);
	    exit(-1);
	}
    } else {
	ClearWindow(self);
	strcpy(ConFile, consoleName);
    }
    SetStandardCursor(self, Cursor_Wait);
    /* this is a jump in the control flow, we may never get back from this if there's a failure */
    SetupFromConsoleFile(self, FALSE);
    self->userMenulist = PrepareUserMenus(self, ATK::LoadClass("consoleClass"));
    InitializeInstruments(self);
    if (Numbers[ERRORS].RawText != Nullity && Numbers[ERRORS].RawText)
        Numbers[ERRORS].RawText[0] = '\0';
    im::SetProgramName(RealProgramName);
    ((self)->GetIM())->SetTitle( TitleFromFile (ConFile, TRUE));
    (self->stdMenulist)->ChainBeforeML( self->userMenulist, (long)self->userMenulist);
    (self)->PostMenus( self->stdMenulist);
    RedrawDisplays(self);
    SetStandardCursor(self, Cursor_Arrow);
    (self)->WantUpdate( self);
}



int adjmon(int  *mday , int  max , int  *mon)
{
    if (*mday > max){
	*mon += 1;
	*mday -= max;
	return(FALSE);
    }
    return(TRUE);
}


void SetAlarm(class ATK *atkarg_self, char  *rock)
{

	class consoleClass *self=(class consoleClass *)atkarg_self;
    boolean isPM, isAM;
    int     hr, min, day, mday, wday, mon, year;
    static int TFHC = -1;
    char    buf2[80], *s, eventbuf[120];
    static const char * const DayOfWeek[] = {
	"Sunday", "Monday", "Tuesday", "Wednesday", 
	"Thursday", "Friday", "Saturday", 0};
    time_t    now;
    struct tm  *t;

    mydbg(("entering: SetAlarm\n"));
    PauseEnqueuedEvents = TRUE;

    isPM = isAM = FALSE;

    if (TFHC == -1){/* only do this once */
	if (!strchr(Numbers[CLOCKALL].RawText, 'A') && !strchr(Numbers[CLOCKALL].RawText, 'P')){
	    TFHC = TRUE;
	}
	else{
	    TFHC = FALSE;
	}
    }
    buf2[0] = '\0';
    InitPstrings();
    sprintf(Pstring1, "Alarm Time:");
    sprintf(Pstring2, (TFHC) ? "[12:00]" : "[12:00 Noon]");
    sprintf(Pstring3, "==>> ");
    PromptToWindow(self);
    GetStringFromWindow(self, sizeof(buf2));
    if (Pstring4[0] == '\0'){
	sprintf(Pstring4, (TFHC) ? "12:00" : "12:00 PM");
    }
    now = time(0);
    t = localtime(&now);
    day = t->tm_yday;
    mday = t->tm_mday;
    wday = t->tm_wday;
    mon = t->tm_mon;
    year = t->tm_year;

    hr = min = 0;

    sscanf(Pstring4, "%d:%d%s", &hr, &min, buf2);

    /* if no minutes are given the resulting value may not be what they think they entered */
    
    /* if value entered for hour is greater than 24 assume a millitary like time entry - PM and AM declarations are ignored */

    if (!TFHC){
	if (hr <= 12 && hr > 0) {
	    for (s = buf2; *s; ++s) {
		if (*s == 'P' || *s == 'p')  {
		    isPM = TRUE;
		    break;
		}
		if (*s == 'A' || *s == 'a') {
		    isAM = TRUE;
		    break;
		}
	    }
	    if (isAM || isPM){
		if (hr == 12)  {
		    hr = 0;
		}
		if (isPM)  {
		    hr += 12;
		}
	    }
	    else  {

		/* User did not specify whether it is AM or PM.  Must find the next proper time */

		if (hr == 12){ /* generally the trickiest case */
		    if ((t->tm_hour == 12) && (min <= t->tm_min)){
			hr += 12; /* bump 12 to 24 - next available 12 */
		    }
		    if (t->tm_hour == 0){
			hr += (min <= t->tm_min) ? 24 : 12; /* ? next afternoon : this morning */
		    }
		    if (t->tm_hour > 12){ /* assume less than 24 ? */
			hr += 12; 
		    }
		}
		else  {
		    while ((hr < t->tm_hour) || (hr == t->tm_hour && min <= t->tm_min))  {
			hr += 12;
		    }
		}
	    }
	}
    }
    /* We now have the right time but need to know whether we need to move to the next day */

    if (hr < t->tm_hour || (hr == t->tm_hour && min <= t->tm_min))  {
	hr += 24;
    }

    /* Now adjust for the number of extra days  */

    while (hr >= 24)  { /* 24 should be reduced to 0 - it will be bumbped to 12 later */
	day++;
	wday++;
	mday++;
	hr -= 24;
    }

    wday %= 7;

    if (day > 365){
	int tmpday = day;
	while (tmpday > 365){
	    year++;
	    tmpday -= 365;
	}
    }
    if ((mday > 28 && mon == 1) || mday > 30){
	/* rough elimination so we don't have to go through this every time */
	int done = FALSE;
	mon++; /* 1 - 12 instead of 0 - 11 */
	while (!done){
	    switch(mon){
		case 1: case 3: case 5: case 7: case 8: case 10: case 12:
		    done = adjmon(&mday, 31, &mon);
		    break;
		case 4: case 6: case 9: case 11:
		    done = adjmon(&mday, 30, &mon);
		    break;
		case 2:
		    /* not perfect - but for now will do */
		    done = adjmon(&mday, ((year % 4) == 0) ? 29 : 28, &mon);
		    break;
	    }
	}
    }
    while (mon > 12) {
	mon -= 12;
	year++;
    }
    

    eventbuf[0]='\0';
    InitPstrings();
    sprintf(Pstring1, "Event:");
    sprintf(Pstring2, "Wake Up!!");
    sprintf(Pstring3, "==>> ");
    PromptToWindow(self);
    GetStringFromWindow(self, 120);
    if (Pstring4[0] == '\0'){
        strcpy(eventbuf,"Wake Up!!");
    }
    else{
	strcpy(eventbuf, Pstring4);
    }
    if (eventbuf[strlen(eventbuf) - 1] == '.'){
	eventbuf[strlen(eventbuf) - 1] = '\0';
    }
    PauseEnqueuedEvents = FALSE;

    VeryFirstDisplay->Threshhold = day * 24 * 3600 + hr * 3600 + min * 60;

    if (!TFHC){
	if (hr == 12){
	    isPM = TRUE;
	}
	if (hr > 12)   {
	    hr -= 12;
	    isPM = TRUE;
	}

	if (hr == 0){
	    hr += 12;
	    isPM = FALSE;
	}
    }
/*    consoleClass_WantUpdate(self, self);*/
    self->menuMask &= ~ALR_OFF_MASK;
    self->menuMask |= ALR_ON_MASK;
    (self->stdMenulist)->SetMask( self->menuMask);
    Numbers[ALARM].Value = TRUE;
    if (!TFHC){
	sprintf(Numbers[ALARM].RawText, "Alarm: %d:%02d %s - %s %d/%d/%d: %s.", hr, min, (isPM) ? "PM" : "AM", DayOfWeek[wday], mon + 1, mday,  year, eventbuf);
    }
    else{
	sprintf(Numbers[ALARM].RawText, "Alarm: %02d:%02d - %s %d/%d/%d: %s.", hr, min, DayOfWeek[wday], mon + 1, mday,  year, eventbuf);
    }
    (self)->PostMenus( self->stdMenulist);
    RedrawDisplays(self);
    (self)->FlushGraphics();
    (self)->ReceiveInputFocus();
    (self)->WantUpdate( self);
}

void TurnOffAlarm(class ATK *atkarg_self, char  *rock)
{

	class consoleClass *self=(class consoleClass *)atkarg_self;
    mydbg(("entering: TurnOffAlarm\n"));
    VeryFirstDisplay->Threshhold = 36500000;
    Numbers[ALARM].Value=FALSE;
    strcpy(Numbers[ALARM].RawText, "The alarm clock is not set.") ;
    self->menuMask &= ~ALR_ON_MASK;
    self->menuMask |= ALR_OFF_MASK;
    (self->stdMenulist)->SetMask( self->menuMask);
    (self)->PostMenus( self->stdMenulist);
    PauseEnqueuedEvents = FALSE;
    RingingAlarm = FALSE;
    RedrawDisplays(self);
    (self)->FlushGraphics();
    (self)->ReceiveInputFocus();
    (self)->WantUpdate( self);
}


struct display *FindInstrument(class consoleClass  *self, int  x, int  y)
{
    struct display *mydisp;

    mydbg(("entering: FindInstrument\n"));
    for (mydisp = VeryFirstDisplay; mydisp; mydisp = mydisp->NextOfAllDisplays) {
        if (mydisp->Xmin <= x && mydisp->Xmax >= x && mydisp->Ymin <= y && mydisp->Ymax >= y && (!mydisp->DependentUponVariables || IntrnlVars[mydisp->WhichVariable].Value == mydisp->AppearIfTrue)){
            break;
        }
    }
    if (!mydisp) {
	InvertWindow(self);
	InvertWindow(self);
        (self)->SetTransferMode( graphic::BLACK);
        return(NULL);
    }
    mydisp->LastClickValue = mydisp->WhatToDisplay->Value;
    return(mydisp);
}



#define ABSVALDIFF(a,b) (((a) > (b)) ? ((a) - (b)) : ((b) - (a)))
#define CLOSEFIT 4

int CheckMovingX(class consoleClass  *self, int  x , int  y)
{
    struct display *mydisp;

    mydbg(("entering: CheckMovingX\n"));
    for (mydisp = VeryFirstDisplay; mydisp; mydisp = mydisp->NextOfAllDisplays) {
        if (mydisp->DependentUponVariables && IntrnlVars[mydisp->WhichVariable].Value != mydisp->AppearIfTrue) {
            continue;
        }
        if ((((ABSVALDIFF(mydisp->Xmin, x)) < CLOSEFIT) || ((ABSVALDIFF(mydisp->Xmax, x)) < CLOSEFIT)) && (mydisp->Ymin <= y) && (mydisp->Ymax >= y)){
            SetStandardCursor(self, Cursor_VerticalBars);
            return(1);
        }
        if ((((ABSVALDIFF(mydisp->Ymin, y)) < CLOSEFIT) || ((ABSVALDIFF(mydisp->Ymax, y)) < CLOSEFIT)) && (mydisp->Xmin <= x) && (mydisp->Xmax >= x)){
            SetStandardCursor(self, Cursor_HorizontalBars);
            return(0);
        }
    }
    SetStandardCursor(self, Cursor_Cross);
    return(-1);
}


/* 
 * MovingX is 1 if X boundary is being moved, 0 if y is
 * being moved, and -1 if the right click was not near
 * any boundary.
 */

void ResizeDisplay(class consoleClass  *self, int  x , int  y , int  LastX , int  LastY , int  MovingX)
{
    struct display *mydisp;
    int     old, new_c, this_c;

    mydbg(("entering: ResizeDisplay\n"));
    /* Normalize coordinates to relative scales */
    x = x * ScaleFactor / (self)->GetLogicalWidth();
    y = y * ScaleFactor / (self)->GetLogicalHeight();
    LastX = LastX * ScaleFactor / (self)->GetLogicalWidth();
    LastY = LastY * ScaleFactor / (self)->GetLogicalHeight();

    old = MovingX ? LastX : LastY;
    new_c = MovingX ? x : y;
    if (ABSVALDIFF(old, new_c) < CLOSEFIT) {
        return;
    }
    for (mydisp = VeryFirstDisplay; mydisp; mydisp = mydisp->NextOfAllDisplays) {
        this_c = MovingX ? mydisp->RelXmin : mydisp->RelYmin;
        if (this_c < old) {
            this_c = ((this_c * 1000 * new_c) / old) / 1000;
        }
        else {
            this_c = ScaleFactor - ((((ScaleFactor - this_c) * (ScaleFactor - new_c) * 1000) / (ScaleFactor - old)) / 1000);
        }
        if (MovingX) {
            mydisp->RelXmin = this_c;
        }
        else {
            mydisp->RelYmin = this_c;
        }
        this_c = MovingX ? mydisp->RelXmax : mydisp->RelYmax;
        if (this_c < old) {
            this_c = ((this_c * 1000 * new_c) / old) / 1000;
        }
        else {
            this_c = ScaleFactor - ((((ScaleFactor - this_c) * (ScaleFactor - new_c) * 1000) / (ScaleFactor - old)) / 1000);
        }
        if (MovingX) {
            mydisp->RelXmax = this_c;
        }
        else {
            mydisp->RelYmax = this_c;
        }
    }
    RedrawDisplays(self);
    (self)->WantUpdate( self);
}

/* To make things a little easier to read ... for me */
#define IntVarOn(x)    IntrnlVars[(x)].turnon
#define IntVarOff(x)   IntrnlVars[(x)].turnoff

void TogVar(class ATK *atkarg_self, long  rock)
        {

	class consoleClass *self=(class consoleClass *)atkarg_self;
    int whichvar = rock;
    struct proctable_Entry *menuProc;

    mydbg(("entering: TogVar\n"));
    if (whichvar < 0 || whichvar > NUMINTERNALVARIABLES) {
	sprintf(ErrTxt, "console: Variable %d does not exist", whichvar);
        ReportInternalError(self, ErrTxt);
    }
    else {
        if (IntrnlVars[whichvar].Value) {
            if (whichvar == 0) {
                Numbers[ERRORS].Value = 0;
                Numbers[ERRORS].RawText[0] = '\0';
            }
            IntrnlVars[whichvar].Value = FALSE;
            if (IntVarOn(whichvar)) {
                (self->userMenulist)->DeleteFromML( IntVarOff(whichvar));
		menuProc = proctable::DefineProc("console-toggle-var", (proctable_fptr)TogVar, ATK::LoadClass("consoleClass"), NULL, "dummy.");
                (self->userMenulist)->AddToML( IntVarOn(whichvar), menuProc, whichvar, 0);
            }
        }
        else {
            IntrnlVars[whichvar].Value = TRUE;
            if (IntrnlVars[whichvar].turnon) {
                (self->userMenulist)->DeleteFromML( IntVarOn(whichvar));
	    menuProc = proctable::DefineProc("console-toggle-var", (proctable_fptr)TogVar, ATK::LoadClass("consoleClass"), NULL, "dummy.");
                (self->userMenulist)->AddToML( IntVarOff(whichvar), menuProc, whichvar, 0);
            }
        }
    }
    (self)->PostMenus( self->stdMenulist);
    RedrawDisplays(self);
    (self)->ReceiveInputFocus();
    (self)->WantUpdate( self);

}
    

void ExpandMenu(class ATK *atkarg_self, char  *rock)
{

	class consoleClass *self=(class consoleClass *)atkarg_self;
    mydbg(("entering: ExpandMenu\n"));
    self->menuMask &= ~SHR_MASK;
    self->menuMask |= EXP_MASK;
    (self->stdMenulist)->SetMask( self->menuMask);
    if (MYDEBUGGING){
	self->menuMask &= ~DEB_OFF_MASK;
	self->menuMask |= DEB_ON_MASK;
	(self->stdMenulist)->SetMask( self->menuMask);
    }
    else{
	self->menuMask &= ~DEB_ON_MASK;
	self->menuMask |= DEB_OFF_MASK;
	(self->stdMenulist)->SetMask( self->menuMask);
    }
    (self)->PostMenus( self->stdMenulist);
    (self)->WantUpdate( self);
}

void ShrinkMenu(class ATK *atkarg_self)
{

	class consoleClass *self=(class consoleClass *)atkarg_self;
    mydbg(("entering: ShrinkMenu\n"));
    self->menuMask &= ~EXP_MASK;
    self->menuMask &= ~DEB_OFF_MASK;
    self->menuMask &= ~DEB_ON_MASK;
    self->menuMask |= SHR_MASK;
    (self->stdMenulist)->SetMask( self->menuMask);
    (self)->PostMenus( self->stdMenulist);
    (self)->WantUpdate( self);
}





static struct ATKregistryEntry *are=NULL;

void GetStdItems(class menulist  *tempMenulist)
{
    struct proctable_Entry *menuProc;

    if(are==NULL) are=ATK::LoadClass("consoleClass");
    
    sprintf(ErrTxt, "Display~2, Restart %s", conlib[0].confile?conlib[0].confile:"");
    menuProc = proctable::DefineProc("console-read-new-console-file", (proctable_fptr)ReadNewConsoleFile, are, 0, "dummye");
    (tempMenulist)->AddToML( ErrTxt, menuProc, (long)conlib[0].path, 0);
    menuProc = proctable::DefineProc("console-quit", (proctable_fptr)DoQuit, are, 0, "dummy.");
    (tempMenulist)->AddToML( "Quit~99", menuProc, 0, 0);
    menuProc = proctable::DefineProc("console-read-new-console-file", (proctable_fptr)ReadNewConsoleFile, are, 0, "dummye");
    (tempMenulist)->AddToML( "Display~2, Read New Console File~24", menuProc, 0, 0);
    menuProc = proctable::DefineProc("console-expand-menu", (proctable_fptr)ExpandMenu, are, 0, "dummy.");
    (tempMenulist)->AddToML( "Display~2, Expand Menu~52", menuProc, 0, SHR_MASK);
    menuProc = proctable::DefineProc("console-shrink-menu", (proctable_fptr)ShrinkMenu, are, 0, "dummy.");
    (tempMenulist)->AddToML( "Display~2, Shrink Menu~52", menuProc, 0, EXP_MASK);
    menuProc = proctable::DefineProc("console-set-alarm", (proctable_fptr)SetAlarm, are, 0, "dummy.");
    (tempMenulist)->AddToML( "Set Alarm~5", menuProc, 0, ALR_OFF_MASK);
    menuProc = proctable::DefineProc("console-turn-off-alarm", (proctable_fptr)TurnOffAlarm, are, 0, "dummy.");
    (tempMenulist)->AddToML( "Turn Off Alarm~5", menuProc, 0, ALR_ON_MASK);
    menuProc = proctable::DefineProc("console-write-log-file", (proctable_fptr)WriteMonsterLog, are, 0, "dummy.");
    (tempMenulist)->AddToML( "Write Log File~40", menuProc, 0, 0);
    menuProc = proctable::DefineProc("console-clear-logs", (proctable_fptr)ClearAllLogs, are, 0, "dummy.");
    (tempMenulist)->AddToML( "Clear All Logs~45", menuProc, 0, 0);
    menuProc = proctable::DefineProc("console-getstats", (proctable_fptr)RestartStats, are, 0, "dummy.");
    (tempMenulist)->AddToML( "Restart Getstats~60", menuProc, 0, GETSTAT_MASK);
    menuProc = proctable::DefineProc("console-debug", (proctable_fptr)DebugMenu, are, 0, "dummy.");
    (tempMenulist)->AddToML( "Expert Options~90, Turn Off Debugging~90", menuProc, 0, DEB_ON_MASK);
    (tempMenulist)->AddToML( "Expert Options~90, Turn On Debugging~90", menuProc, 0, DEB_OFF_MASK);
    menuProc = proctable::DefineProc("console-debug", (proctable_fptr)DebugMenu, are , 0, "dummy.");
    (tempMenulist)->AddToML( "Expert Options~90, Turn Off Debugging~90", menuProc, 0, DEB_ON_MASK);
    (tempMenulist)->AddToML( "Expert Options~90, Turn On Debugging~90", menuProc, 0, DEB_OFF_MASK);

}


void GetStdConsoles(class menulist  *tempMenulist)
{
    char TmpBuf[MAXPATHLEN];
    int len, len2, i, j;
    char *nm, *tail;
    DIRENT_TYPE *dp;
    DIR *dirp;
    struct proctable_Entry *menuProc;
    unsigned char loopCtr;
    char copyOfEXTENSION[1 + MAXEXTLTH];

    mydbg(("Entering: GetStdConsoles\n"));
    connum = 1;

    for (loopCtr = 0; loopCtr <= MAXMATCHES; ++loopCtr) {
	dirp = opendir(libpaths[BasicLib]);
	if (dirp == NULL) {
	    arrgh(("console: Can't open Console Library '%s'\n", libpaths[BasicLib]));
	    if(loopCtr == MAXMATCHES) {
		mydbg(("Leaving: GetStdConsoles(0)\n"));
	    }
	    return;
	}
	memset(TmpBuf, 0, MAXPATHLEN);
	memset(copyOfEXTENSION, 0, MAXEXTLTH);
	switch(loopCtr) {
	    case 0:
		strcpy(copyOfEXTENSION, EXTENSION);
		break;
	    case 1:
		strcpy(copyOfEXTENSION, EXTENSION1);
		break;
	    case 2:
		strcpy(copyOfEXTENSION, EXTENSION2);
		break;
	    default:
		strcpy(copyOfEXTENSION, EXTENSION);
		break;
	}
	sprintf(TmpBuf, ".%s", copyOfEXTENSION);
	len = strlen(TmpBuf);
	/* load up conlib with appropriate filenames */
	while (((dp = readdir(dirp)) != NULL) && connum < MAXCONFILES) {
	    if ((int)DIRENT_NAMELEN(dp) > len) {
		nm  = dp->d_name;
		len2 = strlen(nm);
		tail = nm + len2 - len;
		if(!strcmp(tail, TmpBuf)){
		    *tail = '\0';
		    conlib[connum].path = (char *) malloc(strlen(libpaths[0]) + len2 + 
							  strlen(copyOfEXTENSION) + 3);
		    sprintf(conlib[connum].path, "%s/%s.%s", libpaths[0], nm, copyOfEXTENSION);
		    conlib[connum].confile = (char *) malloc(len2 + 1);
		    strcpy(conlib[connum].confile, nm);
		    mydbg(("conlib[%ld]: %s\n", connum, conlib[connum].confile));
		    ++connum;
		}
	    }
	}
	closedir(dirp);
    }

    /* sort conlib.  0 is reserved for startup console */
    for (i = 1; i < connum; i++) {
	for (j = 1; j < i; j++) {
	    char *s, *p;
	    if (lc_strcmp(conlib[i].confile, conlib[j].confile) < 0) {
		s = conlib[i].confile;
		p = conlib[i].path;
		conlib[i].confile = conlib[j].confile;
		conlib[i].path = conlib[j].path;
		conlib[j].confile = s;
		conlib[j].path = p;
	    }
	}
    }
    if(are==NULL) are=ATK::LoadClass("consoleClass");
    /* stick into menus */
    for (i = 1; i < connum; i++) {
	sprintf(ErrTxt,"Consoles~5, %s~%d", conlib[i].confile, i);
	menuProc = proctable::DefineProc("console-read-new-console-file", (proctable_fptr)ReadNewConsoleFile, are, NULL, "read in a console file");
	mydbg(("ExtraMenus: %s\n", ErrTxt));
	(tempMenulist)->AddToML( ErrTxt, menuProc, (long)conlib[i].path, 0);
    }
    mydbg(("Leaving: GetStdConsoles(0)\n"));
}



char *GetUserPaths()
{
    static char *paths = NULL, *start = NULL;
    char *end = NULL;
    static boolean alldone = FALSE;
    static char TmpBuf[MAXPATHLEN];
    int len;

    memset(TmpBuf, 0, MAXPATHLEN);
    mydbg(("Entering GetUserPaths\n"));

    if (alldone) return(NULL);
    
    if (paths == NULL || *paths == '\0'){
	if ((paths = getenv("CONSOLELIB")) == NULL) return(NULL);
    }
    if (start == NULL || *start == '\0') start = paths;
    if ((end = strchr(start, ':')) == 0){
	alldone = TRUE;
	strcpy(TmpBuf, start);
	return(TmpBuf);
    }
    len = end - start;
    strncpy(TmpBuf, start, len);
    TmpBuf[len] = '\0';
    start = end;
    start++;
    mydbg(("Leaving GetUserPaths\n"));
    return(TmpBuf);
}


void GetExtraConsoles(class menulist  *tempMenulist, const char  *conpath, const char  *cardTitle)
{
    char TmpBuf[MAXPATHLEN];
    int len, len2, i, j;
    char *nm, *tail;
    DIRENT_TYPE *dp;
    DIR *dirp;
    long start;
    struct proctable_Entry *menuProc;
    unsigned char loopCtr;
    char copyOfEXTENSION[1 + MAXEXTLTH];

    mydbg(("Entering: GetExtraConsoles\n"));
    start = connum;

    for (loopCtr = 0; loopCtr <= MAXMATCHES; ++loopCtr) {
	dirp = opendir(conpath);
	if (dirp == NULL) {
	    arrgh(("console: Can't open Console Library '%s'\n", conpath));
	    if (loopCtr == MAXMATCHES) {
		mydbg(("Leaving: GetExtraConsoles(0)\n"));
	    }
	    return;
	}
	memset(TmpBuf, 0, MAXPATHLEN);
	memset(copyOfEXTENSION, 0, MAXEXTLTH);
	switch(loopCtr) {
	    case 0:
		strcpy(copyOfEXTENSION, EXTENSION);
		break;
	    case 1:
		strcpy(copyOfEXTENSION, EXTENSION1);
		break;
	    case 2:
		strcpy(copyOfEXTENSION, EXTENSION2);
		break;
	    default:
		strcpy(copyOfEXTENSION, EXTENSION);
		break;
	}
	sprintf(TmpBuf, ".%s", copyOfEXTENSION);
	len = strlen(TmpBuf);

	/* load conlib with appropriate files */
	while (((dp = readdir(dirp)) != NULL) && connum < MAXCONFILES){
	    if ((int)DIRENT_NAMELEN(dp) > len) {
		nm = dp->d_name;
		len2 = strlen(nm);
		tail = nm + len2 - len;
		if (!strcmp(tail, TmpBuf)) {
		    *tail = '\0';
		    conlib[connum].path = (char *) malloc(strlen(conpath) + len2 + strlen(copyOfEXTENSION) + 3);
		    sprintf(conlib[connum].path, "%s/%s.%s", conpath, nm, copyOfEXTENSION);
		    conlib[connum].confile = strdup(nm);
		    mydbg(("conlib[%ld]: %s\n", connum, conlib[connum].confile));
		    ++connum;
		}
	    }
	}
	closedir(dirp);
    }
    
    /* sort conlib */
    for (i = start; i < connum; i++){
	for (j = start; ((j < i) && (j < connum)); j++){
	    char *s, *p;
	    if (lc_strcmp(conlib[i].confile, conlib[j].confile) < 0){
		s = conlib[i].confile;
		p = conlib[i].path;
		conlib[i].confile = conlib[j].confile;
		conlib[i].path = conlib[j].path;
		conlib[j].confile = s;
		conlib[j].path = p;
	    }
	}
    }

    if(are==NULL) are=ATK::LoadClass("consoleClass");
    /* put into menus @ 6 per menucard*/
    {
	int cardnum = 1, pos = 0;
	static int cardloc = 5 + 1; /* 5 is position of Std Console Menu Card */

	for (i = start; i < connum; i++){
	    sprintf(ErrTxt, "%s(%d)~%d,%s~%d", cardTitle, cardnum, cardloc, conlib[i].confile, (pos % 6));
	    pos++;
	    if ((pos % 6) == 0){
		cardnum++;
		cardloc++;
	    }
	    menuProc = proctable::DefineProc("console-read-new-console-file", (proctable_fptr)ReadNewConsoleFile, are, NULL, "read new console file");
	    mydbg(("STD: Menu: %s\n", ErrTxt));
	    (tempMenulist)->AddToML( ErrTxt, menuProc, (long)conlib[i].path, EXP_MASK);
	}
	cardloc++; /* bump for next invocation */
    }
    mydbg(("Leaving: GetExtraConsoles(1)\n"));
}

void SetStartUpConsole(char  *path, char  *ConFile)
{
    mydbg(("Entering: SetStartUpConsole(%s, %s)\n", path, ConFile));
    /* These mallocs could be space leaks since there's no checking, 
         need to check for zeroth element initialization (is it NULL),
         i.e. can we compare the path to NULL safely? */
    conlib[0].path = strdup(path);
    conlib[0].confile = strdup(ConFile);
    mydbg(("Leaving: SetStartUpConsole(%s, %s)\n", path, ConFile));
}

#ifndef getwd

#endif /* getwd */

void FindStartUpConsole(char  *ConFile, boolean  IsStartUp)
{
    FILE *pfd = NULL;
    int i;
    char *pwd= NULL;
    char tmpbuf[MAXPATHLEN];
    boolean haspath = FALSE;
    char *confile = NULL;
    char *e = NULL;
    char tmpStr[MAXPATHLEN];
    unsigned char loopCtr;
    char copyOfEXTENSION[1 + MAXEXTLTH];

    mydbg(("Entering: FindStartUpConsole(%s, %d)\n", ConFile, IsStartUp));
    
    if ((e = strchr(ConFile, '/')) != NULL){ /* we can be certain of the index behaviour */
	haspath = TRUE;
	e = strrchr(ConFile, '/');
	e++;
	confile = strdup(e);
	if (confile == NULL) {
	    fprintf(stderr, "FATAL ERROR: cannot allocate memory for confile; in input.FindStartUpConsole...\n");
	    fflush(stderr);
	    exit(-1);
	}
    } else {
	confile = strdup(ConFile);
	if (confile == NULL) {
	    fprintf(stderr, "FATAL ERROR: cannot allocate memory for confile; in input.FindStartUpConsole...\n");
	    fflush(stderr);
	    exit(-1);
	}
    }

    if (haspath){
	if ((pfd = fopen(ConFile, "r")) != NULL){
	    /* use ConFile for full path and add the path to the path list in conlib */
	    if (IsStartUp) {
		SetStartUpConsole(ConFile, confile);
	    }
	    mydbg(("Leaving: FindStartUpConsole(0)\n"));
	    if (confile != NULL) {
		free(confile);
	    }
	    fclose(pfd);
	    return;
	}
	for (loopCtr = 0; loopCtr <= MAXMATCHES; ++loopCtr) {
	    memset(tmpbuf, 0, MAXPATHLEN);
	    memset(copyOfEXTENSION, 0, MAXEXTLTH);
	    switch(loopCtr) {
		case 0: 
		    strcpy(copyOfEXTENSION, EXTENSION);
		    break;
		case 1:
		    strcpy(copyOfEXTENSION, EXTENSION1);
		    break;
		case 2:
		    strcpy(copyOfEXTENSION, EXTENSION2);
		    break;
		default: 
		    strcpy(copyOfEXTENSION, EXTENSION);
		    break;
	    }
	    sprintf(tmpbuf, "%s.%s", ConFile, copyOfEXTENSION);
	    if ((pfd = fopen(tmpbuf, "r")) != NULL){
		if (IsStartUp) {
		    SetStartUpConsole(tmpbuf, confile);
		}
		mydbg(("Leaving: FindStartUpConsole(1)\n"));
		if (confile != NULL) {
		    free(confile);
		}
		fclose(pfd);
		return;
	    }
	}
    }

    /* else no path was specified */
    pwd = (char *)malloc(MAXPATHLEN);
    if (pwd == NULL) {
	fprintf(stderr, "FATAL ERROR: cannot allocate memory for pwd in input.FindStartUpConsole...\n");
	fflush(stderr);
	exit(-1);
    }
    memset(tmpStr, 0, MAXPATHLEN);
    getwd(tmpStr);
    strcpy(pwd, tmpStr);
    sprintf(tmpbuf, "%s/%s", pwd, confile);

    /* first, try it with an extension, otherwise add the extension and try again */
    if ((pfd = fopen(tmpbuf, "r")) != NULL){
	if (IsStartUp) {
	    SetStartUpConsole(tmpbuf, confile);
	}
	mydbg(("Leaving: FindStartUpConsole(2)\n"));
	if (confile != NULL) {
	    free(confile);
	}
	free(pwd);
	fclose(pfd);
	return;
    }
    for (loopCtr = 0; loopCtr <= MAXMATCHES; ++loopCtr) {
	memset(tmpbuf, 0, MAXPATHLEN);
	memset(copyOfEXTENSION, 0, MAXEXTLTH);
	switch (loopCtr) {
	    case 0:
		strcpy(copyOfEXTENSION, EXTENSION);
		break;
	    case 1:
		strcpy(copyOfEXTENSION, EXTENSION1);
		break;
	    case 2:
		strcpy(copyOfEXTENSION, EXTENSION2);
		break;
	    default:
		strcpy(copyOfEXTENSION, EXTENSION);
		break;
	}
	sprintf(tmpbuf, "%s/%s.%s", pwd, confile, copyOfEXTENSION);
	if ((pfd = fopen(tmpbuf, "r")) != NULL){
	    if (IsStartUp) {
		SetStartUpConsole(tmpbuf, confile);
	    }
	    mydbg(("Leaving: FindStartUpConsole(3)\n"));
	    if (confile != NULL) {
		free(confile);
	    }
	    if (pwd != NULL) {
		free(pwd);
	    }
	    fclose(pfd);
	    return;
	}
    }

    /* the path failed for the pwd, try the paths in the library */
    for (i = 0; i < libnum && libpaths[i] != NULL; i++){
	/* first, try it with an extension, otherwise add the extension and try again */
	memset(tmpbuf, 0, MAXPATHLEN);
	sprintf(tmpbuf, "%s/%s", libpaths[i], confile);
	if ((pfd = fopen(tmpbuf, "r")) != NULL){
	    if (IsStartUp) {
		SetStartUpConsole(tmpbuf, confile);
	    }
	    mydbg(("Leaving: FindStartUpConsole(4)\n"));
	    if (confile != NULL) {
		free(confile);
	    }
	    if (pwd != NULL) {
		free(pwd);
	    }
	    fclose(pfd);
	    return;
	}
	for (loopCtr = 0; loopCtr <= MAXMATCHES; ++loopCtr) {
	    memset(tmpbuf, 0, MAXPATHLEN);
	    memset(copyOfEXTENSION, 0, MAXEXTLTH);
	    switch (loopCtr) {
		case 0:
		    strcpy(copyOfEXTENSION, EXTENSION);
		    break;
		case 1:
		    strcpy(copyOfEXTENSION, EXTENSION1);
		    break;
		case 2:
		    strcpy(copyOfEXTENSION, EXTENSION2);
		    break;
		default:
		    strcpy(copyOfEXTENSION, EXTENSION);
		    break;
	    }
	    sprintf(tmpbuf, "%s/%s.%s", libpaths[i], confile, copyOfEXTENSION);
	    if ((pfd = fopen(tmpbuf, "r")) != NULL){
		if (IsStartUp) {
		    SetStartUpConsole(tmpbuf, confile);
		}
		mydbg(("Leaving: FindStartUpConsole(5)\n"));
		if (confile != NULL) {
		    free(confile);
		}
		if (pwd != NULL) {
		    free(pwd);
		}
		fclose(pfd);
		return;
	    }
	}
    }
    mydbg(("Leaving: FindStartUpConsole(6)\n"));
    if (confile != NULL) {
	free(confile);
    }
    if (pwd != NULL) {
	free(pwd);
    }
    /* perhaps this should be boolean - but if the file can't be opened in setup.c then it will prompt the user */
}


void PrepareStdMenus(boolean  IsStartup, class menulist  **stdMenulist, struct ATKregistryEntry   *ClassInfo)
{
    class menulist *tempMenulist;
    int num;

    mydbg(("Entering: PrepareStdMenus\n"));
    if (!IsStartup) {
        return;
    }
    SetConsoleLib();
    FindStartUpConsole(ConFile, IsStartup);

    tempMenulist = new menulist;

    GetStdItems(tempMenulist);
    if (BasicLib >= 0) GetStdConsoles(tempMenulist);
    if (LocalLib >= 0) GetExtraConsoles(tempMenulist, libpaths[LocalLib], "Local Consoles");
    for (num = UserLib; num < libnum; ++num) {
	char tmp[25];
	sprintf(tmp, "CONSOLELIB-%ld", num - UserLib + 1);
	GetExtraConsoles(tempMenulist, libpaths[num], tmp);
    }
    *stdMenulist = tempMenulist;
    mydbg(("Leaving: PrepareStdMenus\n"));
}



class menulist *PrepareUserMenus(class consoleClass  *self, struct ATKregistryEntry   *ClassInfo)
{
    class menulist *tempMenulist;
    struct proctable_Entry *menuProc;
    int     i;

    mydbg(("entering: PrepareUserMenus\n"));
    tempMenulist = menulist::Create(self);
    if (IntrnlVars[0].InUse && IntrnlVars[0].turnon) {
        if (IntrnlVars[0].Value) {
            sprintf(ErrTxt, "%s", IntrnlVars[0].turnoff);
            menuProc = proctable::DefineProc("console-toggle-var", (proctable_fptr)TogVar, ClassInfo, NULL, "dummy.");
            (tempMenulist)->AddToML( ErrTxt, menuProc, 0, 0);
        }
        else {
            sprintf(ErrTxt, "%s", IntrnlVars[0].turnon);
            menuProc = proctable::DefineProc("console-toggle-var", (proctable_fptr)TogVar, ClassInfo, NULL, "dummy.");
            (tempMenulist)->AddToML( ErrTxt, menuProc, 0, 0);
        }
    }
    for (i = 1; i <= NUMINTERNALVARIABLES; ++i) {
        IntVarCount[i] = i; 
        if (IntrnlVars[i].InUse && IntrnlVars[i].turnon) {
            sprintf(ErrTxt, "%s", IntrnlVars[i].turnon);
            menuProc = proctable::DefineProc("console-toggle-var", (proctable_fptr)TogVar, ClassInfo, NULL, "dummy.");
            (tempMenulist)->AddToML( ErrTxt, menuProc, IntVarCount[i], 0);
        }
    }
    return(tempMenulist);
}
