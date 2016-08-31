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
 * turninaux.c
 *
 * This is the overflow from the turnin.c module.
*/

/* ************************************************************
 *  Copyright (C) 1989, 1990, 1991
 *  by the Massachusetts Institute of Technology
 *  For full copyright information see:'mit-copyright.h'     *
 *************************************************************/

#include <mit-copyright.h>

#include <andrewos.h>	/* andrewos.h includes sys/types.h */
#include <atom.H>
#include <atomlist.H>
#include <blank.H>
#include <buffer.H>
#include <dataobject.H>
#include <environ.H>
#include <eosbutton.H>
#include <eos.h>
#include <fontdesc.H>
#include <frame.H>
#include <keymap.H>
#include <keystate.H>
#include <im.H>
#include <lpair.H>
#include <menulist.H>
#include <onoffV.H>
#include <pushbutton.H>
#include <rect.h>
#include <strinput.H>
#include <value.H>

/* sys/types.h in AIX PS2 defines "struct label",  causing a type name clash.
  Avoid this by temporarily redefining "label" to be something else. */
#define label gezornenplatz
#undef label

#include <label.H>
#include <labelview.H>

#define debug(x)  /* printf x ; fflush(stdin);  */
/* #include <stdio.h> */

#define AUXMODULE 1
#include <turnin.H>	/* turnin.eh uses struct label */

static char *ofont     = "Andy";
static char *otoplabel = "Use current edit buffer";
static char *obotlabel = "Use named file";




extern class menulist *turnin_menus;
extern class keymap *turnin_kmap;

turnin::turnin()
{
	ATKinit;

    int style;
    class blank *blank;
    class lpair *left, *right;

    this->turninfromfile = FALSE;
    this->daddy = NULL;
    this->kstate       = keystate::Create(this, turnin_kmap);
    this->title	       = new class label;
    this->titleV       = new class labelview;
    this->course       = (char *) malloc(33);

    this->menus        = (turnin_menus)->DuplicateML( this);

    /* Create the title */
    (this->titleV)->SetDataObject( this->title);
    (this->title)->SetFlags( label_CENTERED);
    (this->title)->SetText( "Turn In Assignment");
    (this->title)->SetFont( "helvetica", fontdesc_Plain, 24);

    /* Create the two buttons: OK and CANCEL */
    style = environ::GetProfileInt("buttonstyle", 2);
    this->okb = new class pushbutton;
    this->okv = new class newbuttonview;
    this->cancelb = new class pushbutton;
    this->cancelv = new class newbuttonview;
    (this->okb)->SetStyle( style);
    (this->cancelb)->SetStyle( style);
    (this->okb)->SetText( "OK");
    (this->cancelb)->SetText( "CANCEL");
    (this->okv)->SetDataObject( this->okb);
    (this->cancelv)->SetDataObject( this->cancelb);
    (this->okv)->AddRecipient( atom::Intern("buttonpushed"), (class view *) this, turnin_TurninGo, 0L);
    (this->cancelv)->AddRecipient( atom::Intern("buttonpushed"), (class view *) this, turnin_Hide, 0L);

    /* left and right are used to force the buttons to be a fixed size, with blank padding any extra space */
    left = new class lpair;
    blank = new class blank;
    (left)->HFixed( blank, this->okv, 100, FALSE);
    right = new class lpair;
    blank = new class blank;
    (right)->HTFixed( this->cancelv, blank, 100, FALSE);
    this->buttons = new class lpair;
    (this->buttons)->HSplit( left, right, 50, FALSE);
    (this->buttons)->SetMovable( FALSE);

    /* Create the questions */
    this->name   = new class strinput;
    this->number = new class strinput;
    (this->name)->SetPrompt( "Filename of assignment: ");
    (this->number)->SetPrompt( "Assignment number: ");
    this->questions = new class lpair;
    (this->questions)->VSplit( this->name, this->number, 50, FALSE);
    (this->questions)->SetMovable( FALSE);

    /* Create the on/off switch */

    /* Because of the way the value objects work, all the real */
    /* work must be postponed until after the views exist */
    /* We finish the job in turnin__FullUpdate where we set */
    /* ResourcesPosted to true so that we do the work only once. */

    this->onoff = new class value;
    this->onoffv = new class onoffV;
    this->ResourcesPosted = FALSE;

    this->userarea = new class lpair;
    (this->userarea)->VTFixed( this->onoffv, this->questions, 80, FALSE);

    /* Create the view itself */
    this->topscreen = new class lpair;
    (this->topscreen)->VSplit( this->titleV, this->userarea, 85, FALSE);
    (this->topscreen)->SetMovable( FALSE);
    this->whole = new class lpair;
    (this->whole)->VFixed( this->topscreen, this->buttons, 50, FALSE);
    (this->whole)->LinkTree( this);

    debug(("turnin object created\n"));

    THROWONFAILURE( TRUE);
}

turnin::~turnin()
{
	ATKinit;

    /* Does nothing for now */
}


/*
void turnin__LinkTree(self, parent)
struct turnin *self;
struct view *parent;
{
    super_LinkTree(self, parent);

    if (self->whole)
	lpair_LinkTree(self->whole, self);
}
*/

void turnin::SetTitle(char  *title)
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

void turnin::FullUpdate(enum view_UpdateType  type, long  left , long  top , long  width , long  height)
{
    struct rectangle childRect;

    if (this->IDoNotExist) this->IDoNotExist = FALSE;

    /* Now that the views exist, and view tree is established, post the */
    /* resources that customize our onoff switch, if they have not */
    /* already been posted. */

    if (!this->ResourcesPosted) {
	(this)->PostResource( atomlist::StringToAtomlist("bodyfont-size"), atom::Intern("long"), (long) 12);
	(this)->PostResource( atomlist::StringToAtomlist("bodyfont"), atom::Intern("string"), (long) ofont);
	(this)->PostResource( atomlist::StringToAtomlist("top label"),atom::Intern("string"), (long) otoplabel);
	(this)->PostResource( atomlist::StringToAtomlist("bottom label"), atom::Intern("string"), (long) obotlabel);
	(this->onoff)->SetValue( 1);
	/* Set the data object After the view exists */
	/* and after the value has been set! */
	(this->onoffv)->SetDataObject( this->onoff);
	this->ResourcesPosted = TRUE;
    }

    (this)->GetLogicalBounds( &childRect);

    (this->whole)->InsertView( this, &childRect);
    (this->whole)->FullUpdate( type, left, top, width, height);
}

class view *turnin::Hit(enum view_MouseAction  action, long  x , long  y , long  clicks)
{
    struct rectangle r, labelrect;
    struct point *mouse;
    class view *tmpv;

    mouse = point_CreatePoint(x, y);
    (this->onoffv)->GetEnclosedBounds( &r);
    (this->titleV)->GetEnclosedBounds( &labelrect);
    r.top += labelrect.height;

    tmpv = (this->whole)->Hit( action, x, y, clicks);
    if (rectangle_IsPtInRect(mouse, &r)) {
	(this)->WantInputFocus( this->name);
	return (class view *) this;
    }
    return tmpv;

}
