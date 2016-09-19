/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

#include <andrewos.h>
ATK_IMPL("logview.H")
#include <view.H>
#include <textview.H>
#include <proctable.H>
#include <menulist.H>
#include <logview.H>

static class menulist *logviewMenu;


ATKdefineRegistry(logview, textview, logview::InitializeClass);
static int logview_NoTextviewKey();


static int logview_NoTextviewKey()
{
    return(0);
}

logview::logview()
        {
	ATKinit;

    this->menu = (logviewMenu)->DuplicateML( this);
    THROWONFAILURE( TRUE);
}

logview::~logview()
{
	ATKinit;

    if (this->menu != NULL)
	delete this->menu;
}
	

void logview::LoseInputFocus()
    {
    (this)->SetDotLength( 0);
    (this)->PostMenus( NULL);
    this->hasInputFocus = FALSE;
    (this)->WantUpdate( this);
}

void logview::PostMenus(class menulist  *menulist)
        {
    if ((this)->GetDotLength() > 0)  {
	(this->menu)->SetMask( 1);
    }
    else {
	(this->menu)->SetMask( 0);
    }
    (this)->textview::PostMenus( this->menu);
}

void logview::FrameDot(long  pos)
        {
    struct linedesc *lastLine;
    long topPos;

    if (this->nLines > 0)  {
	lastLine = &(this->lines[this->nLines - 1]);
	if (lastLine->nChars != 0 && lastLine->y + 2 * lastLine->height > (this)->GetLogicalHeight())  {
	    topPos = (this)->GetTopPosition();
	    topPos = (this)->MoveBack( topPos, 0, textview_MoveByLines, 0, 0);
	    topPos = (this)->MoveForward( topPos, 1, textview_MoveByLines, 0, 0);
	    if (this->scroll == textview_ScrollBackward) this->scroll = textview_MultipleScroll;
	    (this)->SetTopPosition( topPos);
	}
    }
    (this)->textview::FrameDot( pos);
}

boolean logview::InitializeClass()
    {
    struct proctable_Entry *tempProc;
    struct ATKregistryEntry  *classInfo = &logview_ATKregistry_ ;
    proctable_fptr textview_CopyCmd;

    logviewMenu = new menulist;

    ATK::LoadClass("textview");

    if ((tempProc = proctable::Lookup("textview-copy-region")) != NULL)
        textview_CopyCmd = proctable::GetFunction(tempProc);
    else
        textview_CopyCmd = (proctable_fptr)logview_NoTextviewKey;

    tempProc = proctable::DefineProc("logview-copy-region", (proctable_fptr)textview_CopyCmd, classInfo, NULL, "Copy text from console log.");

    (logviewMenu)->AddToML( "Copy~12", tempProc, 0, 1);
    return TRUE;
}
