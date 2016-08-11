/* $Author: wjh $ */

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

#ifndef NORCSID
#define NORCSID
static char rcsid[]="$Header: /afs/cs.cmu.edu/project/atk-src-C++/contrib/mit/neos/RCS/papersaux.C,v 1.5 1996/06/11 01:28:57 wjh Exp $";
#endif


 
/*
 * papersaux.c
 *
 * This is the overflow from the papers.c module.
*/

/* ************************************************************
 *  Copyright (C) 1989, 1990, 1991
 *  by the Massachusetts Institute of Technology
 *  For full copyright information see:'mit-copyright.h'     *
 *************************************************************/

#include <andrewos.h>
#include <mit-copyright.h>

#include <cursor.H>
#include <eosbutton.H>
#include <eos.h>
#include <fontdesc.H>
#include <im.H>
#include <lpair.H>
#include <menulist.H>
#include <newbuttonview.H>
#include <pushbutton.H>
#include <scroll.H>
#include <text.H>
#include <textview.H>
#include <view.H>
#include <blank.H>

/* sys/types.h in AIX PS2 defines "struct label",  causing a type name clash.
  Avoid this by temporarily redefining "label" to be something else. */
#define label gezornenplatz
#include <eosfx.H> /* eosfx.ih includes sys/types.h */
#undef label

#include <label.H>
#include <labelview.H>

#define AUXMODULE 1
#include <papers.H>   /* papers.eh uses struct label */










extern	class menulist *papers_global_menus;

static char *buttonNames[]=
{ "EDIT", "KEEP", "HIDE", NULL };
static void (*buttonFuncs[])()=
{  papers_Edit, papers_Keep, papers_Hide, NULL };

static char *subdispbuttons[]=
{ "SUBMIT", "DISPLAY", "EDIT", "KEEP", "HIDE", NULL };
static void (*subdispFuncs[])()=
{ MakePaper, papers_Display, papers_Edit, papers_Keep, papers_Hide, NULL };

static char *altbuttons[]=
{ "OLD DOCS", "DISPLAY", "EDIT", "KEEP", "HIDE", NULL };
static void (*altFuncs[])()=
{ papers_GradingToggleAndList, papers_Display, papers_Edit, papers_Keep, papers_Hide, NULL };

#ifndef NORCSID
#endif


papers::papers()
/* This is getting a bit messy... */
/* Possible configurations of view:
    papers[screen[main[titleV, scroll[textv]], buttons]]
    papers[screen[main[titleV, scroll[textv]], altbuttons]]
    papers[submitscreen[main[titleV, scroll[textv]], xtrabuttons]]
 */
 /* A nicer way of doing this would be to abstract all the junk into a main papers object and then subclass the different views. The current code is blech 
   -njw 6/19/90 */
{
	ATKinit;

    int i, numbuttons;
    struct lplist     *lplist;
    struct buttonList *blist;
    struct lplist     *altlplist;
    struct buttonList *altblist;
    struct lplist     *sd_lplist;
    struct buttonList *sd_blist;
    struct buttonList *miscbuttons;
    class lpair      *tmpbuts;
    class blank      *blank;

    lplist    = NULL;
    blist     = NULL;
    altlplist = NULL;
    altblist  = NULL;
    sd_lplist = NULL;
    sd_blist  = NULL;

    this->list         = NULL;
    this->Positions    = NULL;
    this->wantbuttons  = papers_PLAIN;
    this->thiswindow   = papersNotKnown;
    this->daddy        = NULL;
    this->course       = (char *) malloc(33);
    if (!this->course) THROWONFAILURE( FALSE);
    strcpy(this->course, "no course");
    this->toggled      = papers_OLD;
    this->IDoNotExist  = FALSE;
    this->assignment = 0;
    this->student = NULL;

    /* Create and initialise the text */
    this->textv = new class textview;
    this->textobj = new class text;
    if (!this->textv | !this->textobj)
	THROWONFAILURE( FALSE);
    (this->textv)->SetDataObject( this->textobj);
    this->scroll = scroll::Create(this->textv, scroll_LEFT);
    if (!this->scroll)
	THROWONFAILURE( FALSE);

    /* Create & initialise the title bar */
    this->title = new class label;
    this->titleV = new class labelview;
    (this->titleV)->SetDataObject( this->title);
    (this->title)->SetFlags( label_CENTERED);
    (this)->SetTitle( "Available papers");
    (this->title)->SetFont( "helvetica", fontdesc_Plain, 24);

    /* Yucky code follows! Make the sets of buttons */
    for ( i=0 ; buttonNames[i] != NULL ; i++ ) {
	blist = eosbutton::MakeButton(blist, buttonNames[i], buttonFuncs[i], (class view *) this);
	if (buttonNames[i+1] != NULL) lplist = eosbutton::MakeLpair(lplist);
    } /* for - initializing buttons */

    for ( i=0 ; ::altbuttons[i] != NULL ; i++ ) {
	altblist = eosbutton::MakeButton(altblist, ::altbuttons[i], altFuncs[i], (class view *) this);
	if (!strcmp(::altbuttons[i], "OLD DOCS"))
	    this->toggle = altblist->buttv;
	if (::altbuttons[i+1] != NULL) altlplist = eosbutton::MakeLpair(altlplist);
    } /* for - initializing buttons */

    for ( i=0 ; ::subdispbuttons[i] != NULL ; i++ ) {
	sd_blist = eosbutton::MakeButton(sd_blist, ::subdispbuttons[i], subdispFuncs[i], (class view *) this);
	if (::subdispbuttons[i+1] != NULL) sd_lplist = eosbutton::MakeLpair(sd_lplist);
    } /* for - initializing buttons */

    /*
      * The below lines glue the buttons into their lpairs, using the
      * linked lists blist and lplist
      * The lpairs are split so as each button is the same size
      * Making all buttons the same size is done by the code in the while loop, making
	  * the n'th button be 1/n fraction of the size of the panel. 
  * i.e. The lpair_HSplit splits it 1/(n+1) to button and n/(n+1) to the previous lpair.
  * The entire panel is finally placed into self->buttons
  */

    (lplist->lp)->VSplit( blist->next->buttv, blist->buttv, 50, FALSE);
    (altlplist->lp)->VSplit( altblist->next->buttv, altblist->buttv, 50, FALSE);
    (sd_lplist->lp)->VSplit( sd_blist->next->buttv, sd_blist->buttv, 50, FALSE);

    blist  = blist->next->next;
    altblist = altblist->next->next;
    sd_blist = sd_blist->next->next;
    i = 2;
    while (altblist != NULL) {
	(altlplist->next->lp)->VSplit( altblist->buttv, altlplist->lp, (long int) 100*i/(i+1), FALSE);
	altlplist = altlplist->next;
	altblist  = altblist->next;
	i++;
    } 
    i = 2;
    while (sd_blist != NULL) {
	(sd_lplist->next->lp)->VSplit( sd_blist->buttv, sd_lplist->lp, (long int) 100*i/(i+1), FALSE);
	sd_lplist = sd_lplist->next;
	sd_blist  = sd_blist->next;
	i++;
    } 
    i = 2;
    while (blist != NULL) {
	(lplist->next->lp)->VSplit( blist->buttv, lplist->lp, (long int) 100*i/(i+1), FALSE);
	lplist = lplist->next;
	blist  = blist->next;
	i++;
    } 
    numbuttons = i;

    this->buttons = lplist->lp;
/*    self->buttons = lplist->lp; */


    miscbuttons = NULL;
    miscbuttons = eosbutton::MakeButton(miscbuttons, "SUBMIT", MakePaper, (class view *) this);

    this->xtrabuttons = new class lpair;
    this->xtrabuttons = (this->xtrabuttons)->VSplit( miscbuttons->buttv, this->buttons, (long int) 100*numbuttons/(numbuttons+1), FALSE);

    /* There are several different views possible:
      * So, the lpairs below are all the different configurations
      */
    this->main         = new class lpair;
    this->submitscreen = new class lpair;
    this->screen       = new class lpair;

    /* HTFixed is used so that resizes keep the buttons a constant size. */
    /* The blank is a simple view used to pad out the space for the buttons. */
    /* It just redraws itself with blank space */
    blank = new class blank;
    tmpbuts = new class lpair;
    (this->main)->VTFixed( this->titleV, this->scroll, 30, FALSE);
    (tmpbuts)->VTFixed( this->buttons, blank, 150, FALSE);
    (this->screen)->HFixed( this->main, tmpbuts, 100, FALSE);

    tmpbuts = new class lpair;
    blank = new class blank;

    (tmpbuts)->VTFixed( this->xtrabuttons, blank, 200, FALSE);
    (this->submitscreen)->HFixed( this->main, tmpbuts, 100, FALSE);

    tmpbuts = new class lpair;
    blank = new class blank;

    this->altbuttons = new class lpair;
    (this->altbuttons)->VTFixed( altlplist->lp, blank, 230, FALSE);

    tmpbuts = new class lpair;
    blank = new class blank;

    this->subdispbuttons = new class lpair;
    (this->subdispbuttons)->VTFixed( sd_lplist->lp, blank, 230, FALSE);

    /* we don't want the user altering the lpair boundaries... */
    (this->main)->SetMovable( 0);
    (this->screen)->SetMovable( 0);
    (this->submitscreen)->SetMovable( 0);

    this->menuflags = MENUS_general;
    this->menus = (papers_global_menus)->DuplicateML( this);

    SetSortOrder();

    this->display = this->screen;
    this->maincursor = cursor::Create(this->textv);
    (this->maincursor)->SetStandard( Cursor_LeftPointer);
    THROWONFAILURE( TRUE);
}


void papers::SetTitle(char  *title)
/* Set the text of the title bar. If course-in-title is TRUE, then add the name
   of the course into the text */
{
    char string[80];
    strcpy(string, title);
    if (environ::GetProfileSwitch("course-in-title", TRUE)) {
	strcat(string, ": ");
	strcat(string, this->course);
    }
    (this->title)->SetText( string);
}

void papers::SetDisplay(enum papers_DisplayType  displaytype, enum papers_Types  wintype)
/* Papers can take on different aspects - mainly, it can show different
   sets of buttons at the side. This routine decides which buttons to use.
 */
{
    this->thiswindow  = wintype;
    this->wantbuttons = displaytype;

    if (wintype == papersExchange) this->menuflags |= MENUS_instructor;

    switch (displaytype) {
	case papers_SIDE:
	    this->display = this->screen;
	    break;
	case papers_ALTSIDE:
	    (this->screen)->SetNth( 1, this->altbuttons);
	    this->display = this->screen;
	    break;
	case papers_SIDESUBMIT:
	    this->display = this->submitscreen;
	    break;
	case papers_SIDESUBDISP:
	    (this->screen)->SetNth( 1, this->subdispbuttons);
	    this->display = this->screen;
	    break;
	default:
	    this->display = this->main;
    }

    (this->display)->LinkTree( this);
    (this)->WantUpdate( this);
    im::ForceUpdate();
}

papers::~papers()
{
	ATKinit;

    /* does nothing for now */
}

void papers::LinkTree(class view  *parent)
{
    (this)->view::LinkTree( parent);

    if (this->display)
	(this->display)->LinkTree( this);
}

void papers::FullUpdate(enum view_UpdateType  type, long  left , long  top , long  width , long  height)
{
    struct rectangle childRect, crect;

    /* Ensure we exist! */
    if (this->IDoNotExist) this->IDoNotExist = FALSE;

    /* We want to replace the 'papers' view with self->display,
      which is the actual view wanted, so insert self->display into
      the logical area of self
      */
    (this)->GetLogicalBounds( &childRect);
    (this->textv)->GetLogicalBounds( &crect);
    (this->textv)->PostCursor( &crect, this->maincursor);
    (this->display)->InsertView( this, &childRect);
    (this->display)->FullUpdate( type, left, top, width, height);
}

class view *papers::Hit(enum view_MouseAction  action, long  x , long  y , long  clicks)
/*
  If the hit occurred within the textview, then we want to process it
  ourselves, otherwise, just pass it into the lpairs to let them deal with it...
  Also, if we notice the list of papers is empty, we don't even
      bother with
      our own processing when the mouse is in the textview
      this code is based on the code in the captions object, used by andrew messages
      */
{
    struct rectangle clickrect, labelrect;
    struct point *mouse;
    long textx, texty;

    mouse = point_CreatePoint(x, y);
    (this->textv)->GetEnclosedBounds( &clickrect);
    (this->titleV)->GetEnclosedBounds( &labelrect);
    clickrect.top += labelrect.height;
    /* Translate the (x,y) into local coordinates for the textview */
    textx = x - clickrect.left + (this->textv)->GetLogicalLeft();
    texty = y - clickrect.top  + (this->scroll)->GetLogicalTop();

    if (rectangle_IsPtInRect(mouse, &clickrect) && this->list != NULL) {
	long thisdot;

	switch (action) {
	    case view_LeftDown:
	    case view_RightDown:
		(this->textv)->Hit( view_LeftDown, textx, texty, 1);
		this->downdot = (this->textv)->GetDotPosition();
		break;
	    case view_LeftUp:
		(this->textv)->Hit( view_LeftUp, textx, texty, 1);
		SimulateClick(this, TRUE);
		break;
	    case view_RightUp:
		(this->textv)->Hit( view_LeftUp, textx, texty, 1);
		thisdot = (this->textv)->GetDotPosition();
		if (thisdot == this->downdot)
		    thisdot += (this->textv)->GetDotLength();
		if (thisdot != this->downdot) {
		    /* We have a range! */
		    struct paperPositions *start, *end;

		    start = eosfx::LocatePaper(this->Positions, this->downdot, NULL);
		    end   = eosfx::LocatePaper(this->Positions, thisdot, NULL);
		    if (start != end) {
			/* Yep, it's confirmed - many papers to select */
			struct paperPositions *node, *stopnode;
			if (end->textpos > start->textpos) {
			    node = end;
			    stopnode = start;
			} else {
			    node = start;
			    stopnode = end;
			}
			while (node != stopnode) {
			    if (!(node->flags & eos_PaperMarked) && !(node->flags & eos_PaperDeleted)) ToggleMark(this, node);
			    node = node->next;
			}
			(this->textv)->SetDotPosition( thisdot);
			(this->textv)->SetDotLength( 0);
		    } else SimulateClick(this, FALSE);
		} else SimulateClick(this, FALSE);
		break;
	    case view_LeftMovement:
	    case view_RightMovement:
		(this->textv)->Hit( view_LeftMovement, textx, texty, 1);
		break;
	}
	return (class view *) this;
    } else 
	return (this->display)->Hit( action, x, y, clicks);

}
