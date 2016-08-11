static char *hgghview_rcsid = "$Header: /afs/cs.cmu.edu/project/atk-src-C++/contrib/atkbook/framed/RCS/hgghview.C,v 1.2 1994/08/11 03:01:26 rr2b Stab74 $";

/* **************************************************** *\
Copyright 1989 Nathaniel S. Borenstein
Permission to use, copy, modify, and distribute this software and its
documentation for any purpose and without fee is hereby granted,
provided that the above copyright notice appear in all copies and
that both that copyright notice and this permission notice appear in
supporting documentation, and that the name of 
Nathaniel S. Borenstein not be used in
advertising or publicity pertaining to distribution of the software
without specific, written prior permission. 

Nathaniel S. Borenstein DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING ALL
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
Nathaniel S. Borenstein BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY
DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER
IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING
OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
\* ***************************************************** */
#include <andrewos.h>
#include <hgghview.H>
#include <im.H>
#include <view.H>
#include <text.H>
#include <textview.H>
#include <lpair.H>
#include <butt.H>
#include <buttview.H>
#include <proctable.H>
#include <keymap.H>
#include <keystate.H>
#include <menulist.H>
#include <message.H>
#include <hggh.H>

  

static class keymap *hgghview_keymap = NULL;
static class menulist *hgghview_menulist = NULL;


ATKdefineRegistry(hgghview, lpair, hgghview::InitializeClass);
static void ChangeButton(class hgghview  *self, long  param);
static void ToggleProc(class hgghview  *self, long  param);
static void ToggleLpairViews(class hgghview  *self, long  ignored, class butt  *b, enum view_MouseAction  action);


boolean hgghview::InitializeClass()
{
    struct proctable_Entry *proc = NULL;

    hgghview_keymap = new keymap;

    proc = proctable::DefineProc("hgghview-toggle", (proctable_fptr)ToggleProc, &hgghview_ATKregistry_ , NULL, "toggles the two parts of the hgghview.");
    (hgghview_keymap)->BindToKey( "!", proc, 0);

    hgghview_menulist = new menulist;
    (hgghview_menulist)->AddToML( "Toggle Me!", proc,
		      NULL, 0);
    proc = proctable::DefineProc("hgghview-change-button", (proctable_fptr)ChangeButton, &hgghview_ATKregistry_ , NULL, "Changes the text in the toggling button.");
    (hgghview_menulist)->AddToML("HGGH,Change Button Text", proc, NULL, 0);

    return(TRUE);
}

hgghview::hgghview()
{
	ATKinit;

    class text *t1 = new text;
    class text *t2 = new text;
    class textview *tv1 = new textview;
    class textview *tv2 = new textview;

    this->bv = new buttview;
    this->b = new butt;
    this->lp = new lpair;
    (tv1)->SetDataObject( t1);
    (tv2)->SetDataObject( t2);
    (this->bv)->SetDataObject( this->b);
    (t1)->InsertCharacters( 0, "Hello, world!", 13);
    (t2)->InsertCharacters( 0, "Goodbye, world!", 15);
    (this->b)->SetText( "Toggle");
    (this->bv)->SetHitFunction( (buttview_hit_fptr)ToggleLpairViews);
    (this->bv)->SetRocks( this, NULL);
    (this->lp)->VSplit( tv1, tv2, 50, 1);
    (this)->VTFixed( this->bv, this->lp, 25, 1);
    (tv1)->WantInputFocus( tv1);
    this->ks = keystate::Create(this, hgghview_keymap);
    this->ml = (hgghview_menulist)->DuplicateML( this);
    THROWONFAILURE((TRUE));
}

hgghview::~hgghview()
{
	ATKinit;

    (this->lp)->Destroy();
    (this->b)->Destroy();
    (this->bv)->Destroy();
    delete this->ks;
    delete this->ml;
}

void
hgghview::PostKeyState(class keystate  *ks)
{
    this->ks->next = NULL;
    (this->ks)->AddBefore( ks);
    (this)->lpair::PostKeyState( this->ks);
}

void hgghview::PostMenus(class menulist  *ml)
{
    (this->ml)->ClearChain();
    if (ml) (this->ml)->ChainAfterML( ml, (long)ml);
    (this)->lpair::PostMenus( this->ml);
}

static void ChangeButton(class hgghview  *self, long  param)
{
    char Buf[250];

    if (message::AskForString(self, 50,
		"Enter new text for button: ",
		NULL, Buf, sizeof(Buf)) >= 0) {
	(self->b)->SetText( Buf);
	(self->b)->NotifyObservers(
		observable_OBJECTCHANGED);
	message::DisplayString(self, 10,
		"Changed the button text as requested.");
    }
}

static void ToggleProc(class hgghview  *self, long  param)
{
    class view *v1, *v2;

    v1 = (self->lp)->GetNth( 0);
    v2 = (self->lp)->GetNth( 1);
    (self->lp)->SetNth( 0, v2);
    (self->lp)->SetNth( 1, v1);
    (self->lp)->WantUpdate( self->lp);
    (v2)->WantInputFocus(v2);
}

static void ToggleLpairViews(class hgghview  *self, long  ignored, class butt  *b, enum view_MouseAction  action)
{
    if (action == view_LeftDown || action == view_RightDown) {
	ToggleProc(self, 0);
    }
}


