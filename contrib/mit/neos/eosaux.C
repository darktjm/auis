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
 * eosaux.c
 *
 * This is the overflow from the eos.c module.
*/

/* ************************************************************
 *  Copyright (C) 1989, 1990, 1991
 *  by the Massachusetts Institute of Technology
 *   For full copyright information see:'mit-copyright.h'     *
 ************************************************************ */

#include <mit-copyright.h>

#include <andrewos.h>	/* andrewos.h includes sys/types.h */
#include <atom.H>
#include <buffer.H>
#include <dataobject.H>
#include <environ.H>
#include <eosbutton.H>
#include <eos.h>
#include <eframe.H>
#include <fontdesc.H>
#include <frame.H>
#include <im.H>
#include <keystate.H>
#include <lpair.H>
#include <menulist.H>
#include <pushbutton.H>
#include <rect.h>
#include <view.H>

/* sys/types.h in AIX PS2 defines "struct label",  causing a type name clash.
  Avoid this by temporarily redefining "label" to be something else. */
#define label gezornenplatz
#include <eosfx.H> /* eosfx.ih includes sys/types.h */
#undef label


#include <label.H>
#include <labelview.H>

#define AUXMODULE 1
#include <eos.H>   /* eos.eh uses struct label */










extern class menulist *eosMenu;
extern class keymap *eosKeys;

static const char * const studentbuttonNames[]=
  { "Pick Up", "Turn In", "Handouts", "Exchange", "Guide", "Help", NULL};
static const void (*studentbuttonFuncs[])()=
  { eos_PickUp, eos_TurnIn, eos_Handouts, eos_Exchange, eos_StyleGuide, eos_Help, NULL };

static const char * const instructbuttonNames[]=
  { "Grade","Return", "Handouts", "Exchange", "Guide", "Help", NULL };
static const void (*instructbuttonFuncs[])()=
  { eos_Grade, eos_Return, eos_Handouts, eos_Exchange, eos_StyleGuide, eos_Help, NULL };

void eos_NewWindow(class eos  *self);


eos::eos()
/* The view is made up: eos[screen[head[buttons, title], frame[editregion]]] */
{
	ATKinit;

    int i;
    char **buttonNames;
    void (**buttonFuncs)();
    struct lplist *lplist;
    struct buttonList  *blist;
    char *t; /* temporary string */
    char filename[128];
    long version; 

    lplist         = NULL;
    blist          = NULL;
    this->handouts = NULL;
    this->grades   = NULL;
    this->pickups  = NULL;
    this->exchanges= NULL;
    this->turnins  = NULL;
    this->displaywindow = NULL;
    this->course   = (char *) malloc(33);
    this->course[0] = '\0'; /* No course defined. njw 9/14/90 */
    this->program  = (char *) malloc(33);
    strcpy(this->program, "eos"); /* This should get overridden by application */
    this->dialogpri = environ::GetProfileInt("DialogPriority", 0);

    /* The editing buffer */
    this->editregion = buffer::FindBufferByName("Scratch");
    strcpy(filename, eosfx::LocalUnique("Scratch"));
    if (!this->editregion) this->editregion = buffer::Create("Scratch", filename, NULL, NULL);
    (this->editregion)->SetScratch( FALSE);

    version = ((this->editregion)->GetData())->GetModified();

    (this->editregion)->SetCkpClock( 0);
    (this->editregion)->SetCkpVersion( version);
    (this->editregion)->SetWriteVersion( version);

    this->frame = new class frame;
    (this->frame)->SetCommandEnable( TRUE);
    (this->frame)->SetBuffer( this->editregion, TRUE);

    /* The title bar */
    this->title = new class label;
    this->titleV = new class labelview;
    (this->titleV)->SetDataObject( this->title);
    (this->title)->SetFlags( label_CENTERED);
    (this->title)->SetFont( "helvetica", fontdesc_Plain, 42);

    t = environ::Get("EOSTYPE");
    if (t && strcmp(t, "grading") == 0) {
	(this->title)->SetText( "GRADE: Editor");
	this->gradingflag = TRUE;
        buttonNames = instructbuttonNames;
	buttonFuncs = instructbuttonFuncs;
	this->paperatom = atom::Intern ("eos_paper");
    } else {
	(this->title)->SetText( "EOS: Editor");
	this->gradingflag = FALSE;
        buttonNames = studentbuttonNames;
	buttonFuncs = studentbuttonFuncs;
	this->paperatom = NULL;
    }

    /* Make the buttons */
    for ( i=0 ; buttonNames[i] != NULL ; i++ ) {
        blist = eosbutton::MakeButton(blist, buttonNames[i], buttonFuncs[i], (class view *) this);
        if (buttonNames[i+1] != NULL) lplist = eosbutton::MakeLpair(lplist);
    } /* for - initializing buttons */

/*
 * The below lines glue the buttons into their lpairs, using the
 * linked lists blist and lplist
 * The lpairs are split so as each button is the same size
 * Making all buttons the same size is done by the code in the for loop, making
 * the n'th button be 1/n fraction of the size of the panel. 
 * i.e. The lpair_HSplit splits it 1/(n+1) to button and n/(n+1) to the previous
 * lpair.
 * The entire panel is finally placed into self->buttons
 */
    (lplist->lp)->HSplit( blist->next->buttv, blist->buttv, 50, FALSE);

    blist  = blist->next->next;
    i = 2;
    while (blist != NULL) {
        (lplist->next->lp)->HSplit( blist->buttv, lplist->lp, (long int) 100*i/(i+1), FALSE);
        lplist = lplist->next;
        blist  = blist->next;
        i++;
    }

    this->buttons = lplist->lp;
    this->head = lpair::Create(this->buttons, this->titleV, -40);

    /* We want the buttons to remain the same size all the time.
      and also, the title bar */
    this->screen = new class lpair;
    (this->screen)->VTFixed( this->head, this->frame, 75, 0);

    (this->head)->SetMovable( 0);
    (this->screen)->SetMovable( 0);

    /* Menus */
    this->menuflags = MENUS_general;
    if (this->gradingflag) this->menuflags |= MENUS_instructor;
    this->menus = (eosMenu)->DuplicateML( this);

    this->keys = keystate::Create(this, eosKeys);

   /* Place the end result into the view-tree */
    (this->screen)->LinkTree( this);
   
    THROWONFAILURE( TRUE);
}

eos::~eos()
{
	ATKinit;

    if (!this) return;

    if (this->screen) (this->screen)->Destroy();
    if (this->menus) delete this->menus;
    if (this->course) free(this->course);
    if (this->program) free(this->program);
    if (this->keys) delete this->keys;
}

void eos::LinkTree(class view  *parent)
{
    (this)->view::LinkTree( parent);

    if (this->screen)
        (this->screen)->LinkTree( this);
}

void eos::FullUpdate(enum view_UpdateType  type, long  left , long  top , long  width , long  height)
/* Replace the 'eos' view with the view wanted: self->screen */
{
    struct rectangle childRect;

    (this)->GetLogicalBounds( &childRect);
    (this->screen)->InsertView( this, &childRect);
    (this->screen)->FullUpdate( type, left, top, width, height);

}

class view *eos::Hit(enum view_MouseAction  action, long  x , long  y , long  clicks)
/*
  All we want to do is to pass the hit into the sub-views
 */
{
    return (this->screen)->Hit( action, x, y, clicks);
}

void eos::PostKeyState(class keystate  *ks)
/* Want to add our own keybindings into the chain that gets passed to us */
{
    if (!ks) return;

    this->keys->next = NULL;
    (this->keys)->AddBefore( ks); 
    (this)->view::PostKeyState( this->keys);
}

void eos::SetTitle(char  *title)
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

void eos_NewWindow(class eos  *self)
    {
    class eframe *new_c;
    register class buffer *buffer;
    class im *window;
    new_c = new class eframe;

    window = im::Create(NULL);
    (window)->SetView( new_c);

    buffer = (self->frame)->GetBuffer();
    (new_c->frame)->SetBuffer( buffer, TRUE);
    (new_c)->WantInputFocus( new_c);
}

