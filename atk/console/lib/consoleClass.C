/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

/* 
 * console -- User-configurable instrument console for Andrew
 * -originally written for use under the Andrew Window Manager
 *
 * -This program collects all the monitoring information you might
 * -ever want, and lets you configure it in a single instrument
 * -console in a very flexible way.  It is intended to supersede
 * -clock, gvmstat, wdf, ttyscript, and probably a bunch
 * -of other stuff as well.
 *
 */

#include <andrewos.h>
ATK_IMPL("consoleClass.H")
#include <consoleClass.H>
#include <menulist.H>
#include <event.H>
#include <im.H>
#include <environ.H>
#include <style.H>
#include <fontdesc.H>
#include <scroll.H>
#include <graphic.H>
#include <cursor.H>
#include <text.H>
#include <console.h>
#include <signal.h>





ATKdefineRegistry(consoleClass, view, consoleClass::InitializeClass);
struct ATKregistryEntry  *consoleClass_GetInfoHack();
void SetLogFont(class text  *textObj);


struct ATKregistryEntry  *consoleClass_GetInfoHack()  {
    return &consoleClass_ATKregistry_ ;
}
static class menulist *stdMenulist;

static SIGNAL_RETURN_TYPE SignalDoQuit(int sig)
{
    DoQuit(NULL, NULL);
}

consoleClass::consoleClass()
{
	ATKinit;

    mydbg(("entering: consoleClass__InitializeObject"));
    this->stdMenulist = (::stdMenulist)->DuplicateML( this);
#if 0
    this->altMenulist = NULL;
#endif /* 0 */
    this->haveInputFocus = FALSE;
    this->interactive = FALSE;
    this->menuMask = 0;
    is_visible = TRUE;	/* assume visible unless we detect otherwise */
    signal(SIGTERM, SignalDoQuit);
    THROWONFAILURE( TRUE);
}

void consoleClass::FullUpdate(enum view_UpdateType  type, long  left, long  top, long  width, long  height)
                        {
    static boolean firstTime = TRUE;

    mydbg(("entering: consoleClass__FullUpdate\n"));
    is_visible = TRUE;
    (this)->SetTransferMode(graphic::BLACK);
    if (firstTime)  {
	((this)->GetIM())->SetTitle( TitleFromFile (ConFile, TRUE));
	firstTime = FALSE;
	InitDisplay(this);
	InitErrorMonitoring(this,TRUE); 
	InitializeInstruments(this);
	WakeUp(this);
    }
    if (!PauseEnqueuedEvents && !RingingAlarm){
	RedrawDisplays(this);
    }
    else{
	RedrawPrompt(this);
    }
}

void consoleClass::WantUpdate(class view  *requestor)
{
    mydbg(("entering: consoleClass__WantUpdate\n"));
    if (!PauseEnqueuedEvents && !RingingAlarm){
	(this)->view::WantUpdate( requestor);
    }
}  

void consoleClass::Update()
{
    struct display *mydisp;
    const char *window_visibility = NULL;

    mydbg(("entering: consoleClass::Update\n"));
    if ((window_visibility = WantInformation("im_visible")) &&
	(*window_visibility == 'o' || *window_visibility == 'i')) {	/* window obscured */
      is_visible = FALSE;
    } else if (!is_visible) {
	/* Window wasn't visible, but now visible again.  Better fullupdate... */
	GetIM()->RedrawWindow();
	is_visible = TRUE;
    } else {
	for (mydisp = VeryFirstDisplay; mydisp; mydisp = mydisp->NextOfAllDisplays) {
	    if (mydisp->WhatToDisplay->NeedsUpdate) {
		UpdateDisplay(this, mydisp);
	    }
	}
	for (mydisp = VeryFirstDisplay; mydisp; mydisp = mydisp->NextOfAllDisplays) {
	    mydisp->WhatToDisplay->NeedsUpdate = FALSE;
	}
    }
}

void consoleClass::PostMenus(class menulist  *menu)
        {
    mydbg(("entering: consoleClass__PostMenus\n"));
    (this->stdMenulist)->UnchainML( 0);
    if (menu != this->stdMenulist)
	(this->stdMenulist)->ChainBeforeML( menu, 0);
    (this)->view::PostMenus( this->stdMenulist);
}

 
 

class view *consoleClass::Hit(enum view_MouseAction  action, long  x, long  y, long  numberOfClicks)
{
    class view *ret = NULL;
    struct RegionLog   *whichlog;
    struct display *mydisp;
    static int    MovingX = -1;
    static long LastX,LastY;

    mydbg(("entering: consoleClass__Hit\n"));
    if (((mydisp = FindInstrument(this, x, y)) != NULL)&& (mydisp->AssociatedLogView != NULL) && (MovingX == -1)){
	long newx = x - mydisp->Xmin;
	long newy = y - mydisp->Ymin;
	ret = (mydisp->ScrollLogView)->Hit( action, newx, newy, numberOfClicks);
    }
    else{
	if(! this->haveInputFocus){
	    (this)->WantInputFocus( this);
	}
	switch (action) {
	    case view_LeftDown:
		if ((mydisp = FindInstrument(this, x, y)) == NULL) {
		    return((class view *) this);
		}
		if (mydisp->ClickWhenInvisible || (mydisp->WhatToDisplay->Value <= mydisp->Ceiling && mydisp->WhatToDisplay->Value >= mydisp->Threshhold)) {
		    whichlog = mydisp->AssociatedLog;
		    AddToLog(this, mydisp, TRUE, whichlog, FALSE);
		}
		break;
	    case view_LeftUp: 
		break;
	    case view_RightDown: 
		MovingX = CheckMovingX(this, x, y);
		LastX = x;
		LastY = y;
		break;
	    case view_RightUp:
		if (MovingX != -1) {
		    ResizeDisplay(this,x, y, LastX, LastY, MovingX);
		    MovingX = -1;
		}
		SetStandardCursor(this, Cursor_Arrow);
		break;
	    case view_LeftMovement:
	    case view_RightMovement:
		break;
	    default: 
//		ReportInternalError(this, 
//			"console: Unrecognized mouse action"); 
//		return(NULL);  
;

	}
	ret = (class view *) this;
    }
    return( ret );
}


void consoleClass::ReceiveInputFocus()
    {
    mydbg(("entering: consoleClass__ReceiveInputFocus\n"));
    (this)->view::ReceiveInputFocus();
    this->haveInputFocus = TRUE;
    (this->stdMenulist)->ChainBeforeML( this->userMenulist, (long)this->userMenulist);
    (this)->PostMenus( this->stdMenulist);
    (this)->PostKeyState( NULL);
}

void consoleClass::LoseInputFocus()
    {
    mydbg(("entering: consoleClass__LoseInputFocus\n"));
    this->haveInputFocus = FALSE;
}

void SetLogFont(class text  *textObj)
{
    char FontBuffer[50];
    long FontSize, FontStyle;
    static class style *globalStyle = NULL;

    if (globalStyle == NULL){
	const char *s = NULL;
	if (PromptFont == NULL){
	    s = environ::GetProfile("bodyfont");
	    if (!s || !*s) s = PromptFontName;
	}
	fontdesc::ExplodeFontName(s, FontBuffer, sizeof(FontBuffer), &FontStyle, &FontSize);
	PromptFont = fontdesc::Create(FontBuffer, FontStyle, FontSize);
	globalStyle = new style;
	(globalStyle)->SetFontFamily( FontBuffer);
	(globalStyle)->SetFontSize( style_ConstantFontSize, FontSize);
	(globalStyle)->AddNewFontFace( FontStyle);
    }
    (textObj)->SetGlobalStyle( globalStyle);
}



boolean consoleClass::InitializeClass()
    {
    mydbg(("entering: consoleClass__InitializeClass\n"));
    PrepareStdMenus(TRUE, &::stdMenulist, &consoleClass_ATKregistry_ );
    RegionLogs[ERRORREGIONLOG].TextLog = new text;
    SetLogFont(RegionLogs[ERRORREGIONLOG].TextLog);
    (RegionLogs[ERRORREGIONLOG].TextLog )->SetExportEnvironments( FALSE);
    RegionLogs[ERRORREGIONLOG].WhichDatum = NULL;
    RegionLogs[ERRORREGIONLOG].ScrollReverse = FALSE;
    RegionLogs[REPORTREGIONLOG].TextLog = new text;
    SetLogFont(RegionLogs[REPORTREGIONLOG].TextLog);
    (RegionLogs[REPORTREGIONLOG].TextLog )->SetExportEnvironments( FALSE);
    RegionLogs[REPORTREGIONLOG].WhichDatum = NULL;
    RegionLogs[REPORTREGIONLOG].ScrollReverse = FALSE;
    RegionLogs[USERREGIONLOG].TextLog = new text;
    SetLogFont(RegionLogs[USERREGIONLOG].TextLog);
    (RegionLogs[USERREGIONLOG].TextLog )->SetExportEnvironments( FALSE);
    RegionLogs[USERREGIONLOG].WhichDatum = NULL;
    RegionLogs[USERREGIONLOG].ScrollReverse = FALSE;
    RegionLogs[SILLYREGIONLOG].TextLog = new text;
    SetLogFont(RegionLogs[SILLYREGIONLOG].TextLog);
    (RegionLogs[SILLYREGIONLOG].TextLog )->SetExportEnvironments( FALSE);
    RegionLogs[SILLYREGIONLOG].WhichDatum = NULL;
    RegionLogs[SILLYREGIONLOG].ScrollReverse = FALSE;
    return TRUE;
}

