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
#include <switview.H>
#include <dataobject.H>
#include <view.H>
#include <keymap.H>
#include <keystate.H>
#include <menulist.H>
#include <proctable.H>
#include <message.H>
#include <completion.H>
#include <filetype.H>
#include <switcher.H>

/* A good exercise for advanced readers:
    Change switview to be a subclass of lpair,
    placing an appropriate toggling BUTTON in the top
    and the object being changed in the bottom.
    Even better exercise:  Make the object able to work
    either that way or the way it does in this implementation. */

 
   

static class keymap *switview_keymap = NULL;
static class menulist *switview_menulist = NULL;
static struct proctable_Entry *switchobjproc = NULL;


ATKdefineRegistry(switview, view, switview::InitializeClass);
static boolean CheckRightSwitchee(class switview  *self, boolean  *NeedFullRedraw);
static void AddSwitchee(class switview  *self);
static void NextSwitchee(class switview  *self);
static void SwitchObject(class switview  *self, struct switchee  *swin /* really a long */);
void AddSwitcheeFromFile(class switview  *self);


boolean switview::InitializeClass()
{
    struct proctable_Entry *proc = NULL;

    switview_keymap = new keymap;
    switview_menulist = new menulist;

    proc = proctable::DefineProc("switview-next-object",
	(proctable_fptr)NextSwitchee, &switview_ATKregistry_ , NULL,
	"Changes the switcher to look at the next object.");
    (switview_keymap)->BindToKey( "^X^N", proc, 0);
    (switview_menulist)->AddToML(
	"Switcher~95,Next Switchee~90", proc, 0, 0);

    proc = proctable::DefineProc("switview-add-switchee",
	(proctable_fptr)AddSwitchee, &switview_ATKregistry_ , NULL,
	"Adds a new thing for the switcher to switch to.");
    (switview_keymap)->BindToKey( "^X^A", proc, 0);
    (switview_menulist)->AddToML(
	"Switcher~95,Add Switchee~91", proc, 0, 0);

    proc = proctable::DefineProc("switview-add-from-file",
	(proctable_fptr)AddSwitcheeFromFile, &switview_ATKregistry_ , NULL,
	"Adds the contents of a file as a new switchee.");
    (switview_keymap)->BindToKey( "^X5", proc, 0);
    (switview_menulist)->AddToML(
	"Switcher~95,Insert File~92", proc, 0, 0);

    switchobjproc = proctable::DefineProc(
	"switview-switch-object", (proctable_fptr)SwitchObject,
	&switview_ATKregistry_ , NULL,
	"Switches to a given object.");
    return(TRUE);

}

switview::switview()
{
	ATKinit;

    this->ks = keystate::Create(this, switview_keymap);
    this->ml = (switview_menulist)->DuplicateML(
				     this);
    if (!this->ks || !this->ml) THROWONFAILURE((FALSE));
    this->FirstSwitcheroo = NULL;
    this->NowPlaying = NULL;
    THROWONFAILURE((TRUE));
}

switview::~switview()
{
	ATKinit;

    struct switcheroo *this_c, *next;

    if (this->ks) {
	delete this->ks;
	this->ks = NULL;
    }
    if (this->ml) {
	delete this->ml;
	this->ml = NULL;
    }
    this_c = this->FirstSwitcheroo;
    while (this_c) {
	(this_c->v)->Destroy();
	next = this_c->next;
	free(this_c);
	this_c = next;
    }
}

static boolean CheckRightSwitchee(class switview  *self, boolean  *NeedFullRedraw)
{
    class switcher *switcher = (class switcher *)
      (self)->GetDataObject();
    struct switcheroo *swtmp;

    *NeedFullRedraw = FALSE;
    if (!switcher->NowPlaying) {
	self->NowPlaying = NULL;
	return(TRUE);
    }
    if (self->NowPlaying) {
	if (self->NowPlaying->switchee == switcher->NowPlaying) {
	    return(TRUE);
	}
	for (swtmp = self->FirstSwitcheroo;
	     swtmp != NULL;
	     swtmp = swtmp->next) {
	    if (swtmp->switchee == switcher->NowPlaying) {
		if (self->NowPlaying) {
		    (self->NowPlaying->v)->UnlinkTree();
		}
		self->NowPlaying = swtmp;
		(self->NowPlaying->v)->LinkTree( self);
		*NeedFullRedraw = TRUE;
		(self->NowPlaying->v)->WantInputFocus(
				    self->NowPlaying->v);
		return(TRUE);
	    }
	}
    }
    swtmp = (struct switcheroo *)
      malloc(sizeof(struct switcheroo));
    if (swtmp) {
	swtmp->v = (class view *)
	  ATK::NewObject(switcher->NowPlaying->viewname);
	if (swtmp->v) {
	    (swtmp->v)->SetDataObject(
			switcher->NowPlaying->d);
	    swtmp->next = self->FirstSwitcheroo;
	    swtmp->switchee = switcher->NowPlaying;
	    self->FirstSwitcheroo = swtmp;
	    if (self->NowPlaying) {
		(self->NowPlaying->v)->UnlinkTree();
	    }
	    self->NowPlaying = swtmp;
	    (self->NowPlaying->v)->LinkTree( self);
	    *NeedFullRedraw = TRUE;
	    (self->NowPlaying->v)->WantInputFocus(
				self->NowPlaying->v);
	    return(TRUE);
	}
    }
    return(FALSE);
}

void switview::FullUpdate(enum view_UpdateType  type, long  left, long  top, long  width, long  height)
{
    struct rectangle Rect;
    boolean NeedFull; /* ignored */

    if (!CheckRightSwitchee(this, &NeedFull)) return;
    (this)->GetVisualBounds( &Rect);
    if (!this->NowPlaying) {
	(this)->MoveTo( (Rect.left + Rect.width) / 2,
			    (Rect.top + Rect.height) / 2);
	(this)->DrawString( "<No objects>",
	    graphic_BETWEENLEFTANDRIGHT
	    | graphic_BETWEENTOPANDBASELINE);
	return;
    }
    (this->NowPlaying->v)->InsertView( this, &Rect);
    (this->NowPlaying->v)->FullUpdate( type, left,
		     top, width, height);
}

void switview::Update()
{
    boolean NeedFullRedraw;

    if (!CheckRightSwitchee(this, &NeedFullRedraw)) return;
    if (NeedFullRedraw) {
	struct rectangle Rect;
	(this)->GetVisualBounds( &Rect);
	(this)->EraseRect( &Rect);
	(this)->FullUpdate( view_FullRedraw,
				Rect.left, Rect.top,
				Rect.width, Rect.height);
	return;
    }
    if (this->NowPlaying) (this->NowPlaying->v)->Update();
}

class view *switview::Hit(enum view_MouseAction  action, long	 x, long	 y, long	 numberOfClicks)
{
    if (!this->NowPlaying) {
	(this)->WantInputFocus( this);
	return((class view *) this);
    }
    return((this->NowPlaying->v)->Hit( action,
		     x, y, numberOfClicks));
}

void switview::PostKeyState(class keystate  *ks)
{
    class keystate  *tmp;

    this->ks->next = NULL;
    tmp = (this->ks)->AddBefore( ks);
    (this)->view::PostKeyState(tmp);
}

void switview::PostMenus(class menulist  *ml)
{
    struct switchee *sw;
    class switcher *switcher = (class switcher *)
      (this)->GetDataObject();
    char MenuBuf[200];

    (this->ml)->ClearChain();
    for (sw = switcher->FirstSwitchee; sw; sw = sw->next) {
	sprintf(MenuBuf, "Switcher~95,%s", sw->label);
	if (this->NowPlaying && (switcher->NowPlaying == sw)) {
	    (this->ml)->DeleteFromML( MenuBuf);
	} else {
	    (this->ml)->AddToML( MenuBuf,
			     switchobjproc, (long) sw, 0);
	}
    }
    if (ml) (this->ml)->ChainBeforeML( ml, (long)ml);
    (this)->view::PostMenus( this->ml);
}

void switview::LinkTree(class view  *parent)
{
    (this)->view::LinkTree( parent);
    if (this->NowPlaying) {
	(this->NowPlaying->v)->LinkTree( this);
    }
}

void switview::WantInputFocus(class view  *v)
{
    if (this->NowPlaying && (v == (class view *) this)) {
	v = this->NowPlaying->v;
    }
    (this)->view::WantInputFocus( v);
}

static void AddSwitchee(class switview  *self)
{
    char ObjName[150], ViewName[150], Label[150];
    class dataobject *d;
    class switcher *sw;

    if (message::AskForString(self, 10, "Object to insert: ", 
			      NULL, ObjName,
			      sizeof(ObjName)) < 0) {
	message::DisplayString(self, 10, "Cancelled.");
	return;
    }
    d = (class dataobject *) ATK::NewObject(ObjName);
    if (d == NULL) {
	message::DisplayString(self, 10,
			"Could not create new object, sorry.");
	return;
    }
    if (message::AskForString(self, 10, "View to use: ",
		(d)->ViewName(), ViewName,
		sizeof(ViewName)) < 0) {
	message::DisplayString(self, 10, "Cancelled.");
	(d)->Destroy();
	return;
    }
    if (message::AskForString(self, 10,
			      "Label for this object: ",
			      NULL, Label, sizeof(Label)) < 0) {
	message::DisplayString(self, 10, "Cancelled.");
	(d)->Destroy();
	return;
    }
    sw = (class switcher *) (self)->GetDataObject();
    if (!(sw)->AddObject( d, Label, ViewName)) {
	message::DisplayString(self, 10,
		"Object creation failed, sorry!");
	return;
    }
    NextSwitchee(self);
    (sw)->NotifyObservers( observable_OBJECTCHANGED);
}

static void NextSwitchee(class switview  *self)
{
    class switcher *switcher = (class switcher *)
      (self)->GetDataObject();
    struct switchee *sw = switcher->NowPlaying;

    if (sw && sw->next) {
	(switcher)->SetNowPlaying( sw->next->d);
    } else {
	(switcher)->SetNowPlaying(
			       switcher->FirstSwitchee->d);
    }
}

static void SwitchObject(class switview  *self, struct switchee  *swin /* really a long */)
{
    class switcher *switcher = (class switcher *)
      (self)->GetDataObject();
    struct switchee *sw;

    for (sw= switcher->FirstSwitchee; sw; sw = sw->next) {
	if (sw == swin) {
	    (switcher)->SetNowPlaying( sw->d);
	    return;
	}
    }
    message::DisplayString(self, 10,
	"SwitchObject called for nonexistent object.");
}

void AddSwitcheeFromFile(class switview  *self)
{
    char FileName[150], ViewName[150], Label[150], *ObjName;
    class dataobject *d;
    FILE *fp;
    long ID;
    class switcher *sw;

    if (completion::GetFilename(self,
		"File to insert as new switchee: ",
		NULL, FileName, sizeof(FileName),
		FALSE, TRUE) < 0) {
	message::DisplayString(self, 10, "Cancelled.");
	return;
    }
    fp = fopen(FileName, "r");
    if (!fp) {
	message::DisplayString(self, 10, "Could not open file.");
	return;
    }
    ObjName = filetype::Lookup(fp, FileName, &ID, NULL);
    d = (class dataobject *) ATK::NewObject(ObjName);
    if (!d) {
	fclose(fp);
	message::DisplayString(self, 10,
		"Could not create new object, sorry.");
	return;
    }
    if ((d)->Read( fp, ID) != dataobject_NOREADERROR) {
	fclose(fp);
	message::DisplayString(self, 10,
		"Read operation failed, sorry.");
	return;
    }
    fclose(fp);
    if (message::AskForString(self, 10,
		"View to use: ", (d)->ViewName(),
		ViewName, sizeof(ViewName)) < 0) {
	message::DisplayString(self, 10, "Cancelled.");
	(d)->Destroy();
	return;
    }
    if (message::AskForString(self, 10,
		"Label for this object: ", NULL, Label,
		sizeof(Label)) < 0) {
	message::DisplayString(self, 10, "Cancelled.");
	(d)->Destroy();
	return;
    }
    sw = (class switcher *) (self)->GetDataObject();
    if (!(sw)->AddObject( d, Label, ViewName)) {
	message::DisplayString(self, 10,
		"Object creation failed, sorry!");
	return;
    }
    NextSwitchee(self);
    (sw)->NotifyObservers( observable_OBJECTCHANGED);
}
