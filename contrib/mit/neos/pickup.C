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
 * pickup.c
 *
 * This does the work for pickup within the EOS applications.
*/

/* ************************************************************
 *  Copyright (C) 1989, 1990, 1991
 *  by the Massachusetts Institute of Technology
 *  For full copyright information see:'mit-copyright.h'     *
 *************************************************************/

#include <mit-copyright.h>

#include <andrewos.h>
#include <atom.H>
#include <bind.H>
#include <blank.H>
#include <cursor.H>
#include <environ.H>
#include <eos.h>

/* sys/types.h in AIX PS2 defines "struct label",  causing a type name clash.
  Avoid this by temporarily redefining "label" to be something else
      BEFORE the first use of struct label. */
#define label gezornenplatz
#include <eosfx.H>
/* eosfx.ih includes sys/types.h */
#undef label

#include <eos.H>   /* eos.ih uses struct label */
#include <fontdesc.H>
#include <im.H>
#include <label.H>
#include <labelview.H>
#include <lpair.H>
#include <menulist.H>
#include <message.H>
#include <newbuttonview.H>
#include <pushbutton.H>
#include <scroll.H>
#include <text.H>
#include <textview.H>
#include <pickup.H>	/* pickup.eh uses struct label */
#include <view.H>

static class cursor *clockcursor;
static class menulist *menus;
#define pickupReadyToGo 1 /* menu flag indicating it's okay to show the continue option */


ATKdefineRegistry(pickup, view, pickup::InitializeClass);
void CancelOperation(class pickup  *self, FX  **fxp);
static void Hide( class pickup  *self, class observable  *triggerer, long  rock);


void CancelOperation(class pickup  *self, FX  **fxp)
{
    eosfx::Close(fxp);
    (self)->WantInputFocus( self->textv);
    StopWaitCursor();
}

static void Hide( class pickup  *self, class observable  *triggerer, long  rock)
{
    if (!self) return;
    self->IDoNotExist = TRUE;
    /* "If I close my eyes, then you can't see me!" */
    ((self)->GetIM())->VanishWindow();
}

void pickup::DoPickUp()
{
/*
  open course;
  read list of GRADED pickup;
  display total;
  if (total = 0) then exit;
  foreach paper in list, until list exhausted {
      receive file;
      change status of paper to PICKEDUP
    display message;
  }
  display finished message;
  exit;
*/
    int i;
    Paper criterion, thispaper, tmppaper;
    Paperlist node;
    FX *fxp;
    class im *thisIM;
    char *errormsg;
    char string[128];
    char filename[128];
    char openerr[256];
    boolean AskUser, pickup;
    char c;

    thisIM = (this)->GetIM();
    (this->textobj)->SetReadOnly( FALSE);
    (this->textobj)->ClearCompletely();
    textput("Reading course...\n");
    (this)->WantUpdate( this->textv);
    im::ForceUpdate();

    errormsg = eosfx::OpenCourse(this->course, &fxp);
    /* (fxp == NULL && errormsg) or (fxp is valid for reads&writes) */
    /* or (fxp is valid for reads, invalid for writes && errormsg) */
    if (!fxp) {
	/* We have no chance of continuing */
	textput(errormsg);
        textput("Press CONTINUE to close window");
        (this->okv)->Enable( TRUE);
	this->menuflags = pickupReadyToGo;
        CancelOperation(this, NULL);
        return;
    } 
    /* Note: At this point, we may have a valid FXP, but not be able
     * to write to the server beacuse of some other error.
     * We continue anyway, doing reads only, to find out if there is
     * anything to read. If there isn't then we just keep quiet about the
     * error.
     * If there are things to pick up, we then start complaining to the user
     */
    if (errormsg)
	strcpy(openerr, errormsg); /* Keep a copy for later */
    else
	openerr[0] = '\0';  /* set it to the null string */

/* Make sure the list is 'clean' before we get a new list */
    eosfx::ListDestroy(&this->list);
    eosfx::DestroyPositions(&this->Positions);
 
/* Set the criterion - totally clean apart from type.
 * i.e. We want all GRADED papers belonging to this person.
 */
    eosfx::PaperClear(&criterion);
    criterion.type = GRADED;
    criterion.author = fxp->owner;

    if (errormsg = eosfx::List(fxp, &criterion, &this->list)) {
	/* If we get an error here, and we got one when we opened the course,
	 * we display the openCourse error, as that is probably more significant
	 */
	if (strcmp (openerr, ""))
	    textput(openerr);
	else
	    textput(errormsg);
        textput("\nPress CONTINUE to close window");
        (this->okv)->Enable( TRUE);
	this->menuflags = pickupReadyToGo;
        CancelOperation(this, &fxp);
        return;
    }

/* Count the papers and place them into a paperPositions list */
    i = 0;
    for (node = this->list->Paperlist_res_u.list; node !=NULL; node = node->next) {
        eosfx::AddPaperText(&this->Positions, node, 0, 0);
        i++;
    }

    if (!i) {
        textput("No assignments have been returned.\n");
        textput("Press CONTINUE to close window");
    } else {
        if (i != 1)
	    sprintf(string, "There are %d assignments ready to pickup\n", i);
        else 
	    sprintf(string, "There is one paper to pick up.\n");
        textput(string);

	if (strcmp(openerr, "")) {
	    /* We can continue to pickup, but cannot tell the server about
	     * the collection
	     */
	    textput("\nHowever, there is a problem with the EOS server:\n");
	    textput(openerr);
	    textput("If you continue, the system will not know that you have collected these assignments, and next time you pickup, you will get them again.\n");
	    textput("Do you want to continue? (press 'y' or 'n') [y] >");
	    (this)->WantUpdate( this->textv);
	    im::ForceUpdate();
	    c = (thisIM)->GetCharacter();
	    if (c == 'y' || c == 'Y' || c == '\r' || c == '\n') {
		textput("Yes.\n");
		textput("Continuing...\n\n");
	    } else {
		textput("No.\n");
		textput("Cancelled.\n");
		textput("Press CONTINUE to close window");
		(this->okv)->Enable( TRUE);
		this->menuflags = pickupReadyToGo;
		CancelOperation(this, &fxp);
		return;
	    }
	}
	AskUser = environ::GetProfileSwitch("InteractivePickup", FALSE);
	pickup  = TRUE;
        textput("Collecting...\n\n");
        (this)->WantUpdate( this->textv);
        im::ForceUpdate();
        while (this->Positions != NULL) {
            node = this->Positions->paper;
            this->Positions = this->Positions->next;
            thispaper = node->p;
	    if (AskUser) {
		StopWaitCursor();
		sprintf(string, "%s (#%d) - Collect? (press 'y' or 'n') >", thispaper.filename, thispaper.assignment);
		textput(string);
		(this)->WantUpdate( this->textv);
		im::ForceUpdate();
		c = (thisIM)->GetCharacter();
		if (c == 'y' || c == 'Y' || c == '\r' || c == '\n') {
		    pickup = TRUE;
		    textput("Yes\n");
		    (this)->WantUpdate( this->textv);
		    im::ForceUpdate();
		} else {
		    pickup = FALSE;
		    textput("No\n");
		}
		StartWaitCursor();
	    }
	    if (pickup) {
		strcpy(filename, eosfx::LocalUnique(thispaper.filename));
		if (errormsg = eosfx::RetrieveFile(fxp, &(thispaper), filename)) {
		    textput(errormsg);
		    textput("Pick Up aborted. Press CONTINUE to close window");
		    (this->okv)->Enable( TRUE);
		    this->menuflags = pickupReadyToGo;
		    CancelOperation(this, &fxp);
		    return;
		}
		eosfx::PaperCopy(&thispaper, &tmppaper);
		tmppaper.type = PICKEDUP;
		if (errormsg = eosfx::Move (fxp, &thispaper, &tmppaper)) {
		    textput(errormsg);
		    textput("Pick up aborted. Press the CONTINUE button to close this window.");
		    (this->okv)->Enable( TRUE);
		    this->menuflags = pickupReadyToGo;
		    CancelOperation(this, &fxp);
		    return;
		}
		(this->daddy)->SetBuffer( filename, 0);
		sprintf(string, "Picked up assignment #%d as '%s'.\n", node->p.assignment, filename);
		textput(string);
	    }
            (this)->WantUpdate( this->textv);
            im::ForceUpdate();
        }
        textput("\nCollection completed. Press CONTINUE to close window");
    } 

    (this->okv)->Enable( TRUE);
    this->menuflags = pickupReadyToGo;
    CancelOperation(this, &fxp);
    return;
}

static struct bind_Description pickupBindings[] = {
    {"pickup-continue", NULL, 0, "Continue...", 0, pickupReadyToGo, Hide, NULL},
    NULL
};


boolean pickup::InitializeClass()
{
    ::menus = new class menulist;
    bind::BindList(pickupBindings, NULL, ::menus, &pickup_ATKregistry_ );
    clockcursor = cursor::Create(NULL);
    (clockcursor)->SetStandard( Cursor_Wait);
    return TRUE;
}

void pickup::SetTitle(const char  *title)
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


pickup::pickup()
{
	ATKinit;

    int style;
    class blank *blank;
    class lpair *b1, *b2;

    this->menuflags  = 0;
    this->Positions  = NULL;
    this->daddy      = NULL;
    this->list       = NULL;
    this->title	     = new class label;
    this->titleV     = new class labelview;
    this->course     = (char *) malloc(33);
    if (!this->title || !this->titleV) THROWONFAILURE( FALSE);

    this->menus = (::menus)->DuplicateML( this);

/* Create the title */
    (this->titleV)->SetDataObject( this->title);
    (this->title)->SetFlags( label_CENTERED);
    (this->title)->SetText( "Pick Up");
    (this->title)->SetFont( "helvetica", fontdesc_Plain, 24);

/* Create the text junk */
    this->textobj = new class text;
    this->textv   = new class textview;
    if (!this->textobj || !this->textv) THROWONFAILURE( FALSE);
    (this->textv)->SetDataObject( this->textobj);
    this->scroll  = scroll::Create(this->textv, scroll_LEFT);
    if (!this->scroll)
	THROWONFAILURE( FALSE);

/* Create the button */
    style = environ::GetProfileInt("buttonstyle", 2);
    this->okb = new class pushbutton;
    this->okv = new class newbuttonview;
    if (!this->okb || !this->okv) THROWONFAILURE( FALSE);
    (this->okb)->SetStyle( style);
    (this->okb)->SetText( "CONTINUE");
    (this->okv)->SetDataObject( this->okb);
    (this->okv)->AddRecipient( atom::Intern("buttonpushed"), (class view *) this, Hide, 0L);
    (this->okv)->Enable( FALSE);

    b1 = new class lpair;
    b2 = new class lpair;
    blank = new class blank;
    (b1)->HFixed( blank, this->okv, 100, FALSE);
    blank = new class blank;
    (b2)->HSplit( b1, blank, 40, FALSE);

/* Create the view itself */
    this->topscreen = new class lpair;
    this->whole     = new class lpair;
    if (!this->whole || !this->topscreen) THROWONFAILURE( FALSE);
    (this->topscreen)->VSplit( this->titleV, this->scroll, 85, FALSE);
    (this->topscreen)->SetMovable( FALSE);
    (this->whole)->VSplit( this->topscreen, b2, 20, FALSE);
    (this->whole)->LinkTree( this);

    THROWONFAILURE( TRUE);
}

pickup::~pickup()
{
	ATKinit;

    /* Does nothing for now */
}


void
pickup::PostMenus(class menulist  *ml)
{
    (this->menus)->SetMask( this->menuflags);
    (this)->view::PostMenus( this->menus);
}

void pickup::LinkTree(class view  *parent)
{
    (this)->view::LinkTree( parent);

    if (this->whole)
        (this->whole)->LinkTree( this);
}

void pickup::FullUpdate(enum view_UpdateType  type, long  left , long  top , long  width , long  height)
{
    struct rectangle childRect;

    if (this->IDoNotExist) this->IDoNotExist = FALSE;

    (this)->GetLogicalBounds( &childRect);

    (this->whole)->InsertView( this, &childRect);
    (this->whole)->FullUpdate( type, left, top, width, height);
}

class view *pickup::Hit(enum view_MouseAction  action, long  x , long  y , long  clicks)
{
    return (this->whole)->Hit( action, x, y, clicks);
}

void pickup::ReceiveInputFocus()
{
    (this)->WantInputFocus( this->textv);
}
