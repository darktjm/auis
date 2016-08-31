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
#include <flexview.H>
#include <flex.H>
#include <dataobject.H>
#include <im.H>
#include <view.H>
#include <keymap.H>
#include <keystate.H>
#include <menulist.H>
#include <message.H>
#include <proctable.H>

/* forward declarations */

   

static class keymap *flexview_keymap_noviews = NULL,
    *flexview_keymap_views = NULL;
static class menulist *flexview_menulist_noviews = NULL,
    *flexview_menulist_views = NULL;


ATKdefineRegistry(flexview, lpair, flexview::InitializeClass);
static void DrawMe(class flexview  *self);
static boolean CreateViews(class flexview  *self);
static void ResetViews(class flexview  *self);
static void InsertObject(class flexview  *self);
static void DeleteObjects(class flexview  *self);
static void AlterPair(class flexview  *self);


boolean flexview::InitializeClass()
{
    struct proctable_Entry *proc = NULL;

    flexview_keymap_views = new keymap;
    flexview_keymap_noviews = new keymap;
    flexview_menulist_views = new menulist;
    flexview_menulist_noviews = new menulist;

    proc = proctable::DefineProc("flexview-insert-object", (proctable_fptr)InsertObject, &flexview_ATKregistry_ , NULL, "Inserts a new inset in the flex.");
    (flexview_keymap_noviews)->BindToKey( "^X!", proc, 0);
    (flexview_menulist_noviews)->AddToML(
	"flex~96,Insert Object", proc, NULL, 0);

    proc = proctable::DefineProc("flexview-alter-pair", (proctable_fptr)AlterPair, &flexview_ATKregistry_ , NULL, "Alters some of the settings of the flex.");
    (flexview_keymap_views)->BindToKey( "^X%",
		      proc, 0);
    (flexview_menulist_views)->AddToML(
	"flex~96,Alter Pair", proc, NULL, 0);

    proc = proctable::DefineProc("flexview-delete-objects", (proctable_fptr)DeleteObjects, &flexview_ATKregistry_ , NULL, "Delete the children of of the flex.");
    (flexview_keymap_views)->BindToKey( "^X&", proc, 0);
    (flexview_menulist_views)->AddToML(
	"flex~96,Delete Objects", proc, NULL, 0);

    return(TRUE);
}

flexview::flexview()
{
	ATKinit;

    this->KeystateWithViews =
      keystate::Create(this, flexview_keymap_views);
    this->MenulistWithViews =
      (flexview_menulist_views)->DuplicateML( this);
    this->KeystateWithNoViews =
      keystate::Create(this, flexview_keymap_noviews);
    this->MenulistWithNoViews =
      (flexview_menulist_noviews)->DuplicateML( this);
    this->oldleftdata = NULL;
    this->oldrightdata = NULL;
    this->leftview = NULL;
    this->rightview = NULL;
    THROWONFAILURE((TRUE));
}

flexview::~flexview()
{
	ATKinit;

    if (this->KeystateWithViews) {
	delete this->KeystateWithViews;
	this->KeystateWithViews = NULL;
    }
    if (this->KeystateWithNoViews) {
	delete this->KeystateWithNoViews;
	this->KeystateWithNoViews = NULL;
    }
    if (this->MenulistWithViews) {
	delete this->MenulistWithViews;
	this->MenulistWithViews = NULL;
    }
    if (this->MenulistWithNoViews) {
	delete this->MenulistWithNoViews;
	this->MenulistWithNoViews = NULL;
    }
    /* lpair_FinalizeObject should itself
      take care of the subviews */
}

void flexview::FullUpdate(enum view_UpdateType  type,long  left,long  top,long  width,long  height)
{
    class flex *flex = (class flex *)
      (this)->GetDataObject();

    if (flex && flex->right) {
	if ((this->rightview != NULL) || CreateViews(this)) {
	    if ((this->oldleftdata != flex->left)
		|| (this->oldrightdata != flex->right)) {
		ResetViews(this);
	    }
	    (this)->lpair::FullUpdate(type,left,
			     top,width,height);
	}
    } else {
	DrawMe(this);
    }
}

void flexview::Update()
{
    class flex *flex = (class flex *)
      (this)->GetDataObject();

    if (flex && flex->right) {
	if (this->rightview || CreateViews(this)) {
	    int porf, vorh, movable;

	    if ((this->oldleftdata != flex->left)
		|| (this->oldrightdata != flex->right)) {
		ResetViews(this);
	    }
	    (this)->GetLPState( &porf,
				    &vorh, &movable);
	    if ((porf != flex->porf) || (vorh != flex->vorh)
		|| (movable != flex->movable)) {
		(this)->SetLPState( flex->porf,
			flex->vorh, flex->movable);
	    }
	    (this)->lpair::Update();
	}
    } else {
	DrawMe(this);
    }
}

static void DrawMe(class flexview  *self)
{
    struct rectangle r;
    (self)->GetVisualBounds(&r);
    (self)->EraseRect( &r);
    (self)->MoveTo( (r.left + r.width) / 2,
			 (r.top + r.height) / 2);
    (self)->DrawString( "<No objects>",
	graphic_BETWEENLEFTANDRIGHT
	| graphic_BETWEENTOPANDBASELINE);
}

class view *flexview::Hit(enum view_MouseAction  action,long  x ,long  y ,long  numberOfClicks)
{
    class view *v;
    class flex *flex = (class flex *)
      (this)->GetDataObject();

    if (flex && this->rightview) {
	v = (this)->lpair::Hit(action,x,y,numberOfClicks);
	if (v == (class view *) this) {
	    int porf, vorh, movable, pct;
	    (this)->GetLPState( &porf,
				    &vorh, &movable);
	    if (porf == lpair_TOPFIXED) {
		pct = (this)->GetObjSize( 0);
	    } else {
		pct = (this)->GetObjSize( 1);
	    }
	    (flex)->SetDisplayParams( porf, vorh,
				      movable, pct);
	    (flex)->NotifyObservers(
			observable_OBJECTCHANGED);
	}
	return(v);
    }
    (this)->WantInputFocus( this);
    return((class view *) this);
}

void flexview::PostKeyState(class keystate  *ks)
{
    class keystate *myks, *tmp;

    if (this->rightview != NULL) {
	myks = this->KeystateWithViews;
    } else {
	myks = this->KeystateWithNoViews;
    }
    myks->next = NULL;
    tmp = (myks)->AddBefore( ks);
    (this)->lpair::PostKeyState( tmp);
}

void flexview::PostMenus(class menulist  *ml)
{
    class menulist *myml;

    if (this->rightview != NULL) {
	myml = this->MenulistWithViews;
    } else {
	myml = this->MenulistWithNoViews;
    }
    (myml)->ClearChain();
    if (ml) (myml)->ChainBeforeML( ml, (long)ml);
    (this)->lpair::PostMenus( myml);
}

enum view_DSattributes flexview::DesiredSize(long  width , long  height, enum view_DSpass  pass, long  *desiredwidth , long  *desiredheight)
{ 
    if (this->rightview) {
	return((this)->lpair::DesiredSize( width, height, pass,
			desiredwidth, desiredheight));
    }
    *desiredwidth = 200;
    *desiredheight = 100;
    return(view_WidthFlexible | view_HeightFlexible);
}

static boolean CreateViews(class flexview  *self)
{
    class flex *flex = (class flex *)
      (self)->GetDataObject();

    self->leftview = (class view *)
      ATK::NewObject(flex->lvname);
    if (self->leftview == NULL) return(FALSE);
    (self->leftview)->SetDataObject( flex->left);
    self->rightview = (class view *)
      ATK::NewObject(flex->rvname);
    if (self->rightview == NULL) return(FALSE);
    (self->rightview)->SetDataObject( flex->right);
    (self)->SetUp( self->leftview, self->rightview,
	flex->pct, flex->porf, flex->vorh, flex->movable);
    (self)->SetLPState( flex->porf, flex->vorh,
	flex->movable);
    (self->leftview)->WantInputFocus( self->leftview);
    self->oldleftdata = flex->left;
    self->oldrightdata = flex->right;
    return(TRUE);
}

static void ResetViews(class flexview  *self)
{
    class flex *flex = (class flex *)
      (self)->GetDataObject();
    class view *oldfocus;

    if (self->oldleftdata == flex->right) {
	oldfocus = ((self)->GetIM())->GetInputFocus();
	(self)->SetNth( 0, self->leftview);
	(self)->SetNth( 1, self->rightview);
	self->oldleftdata = flex->left;
	self->oldrightdata = flex->right;
	(self)->WantUpdate( self);
	if (oldfocus != NULL) {
	    (oldfocus)->WantInputFocus( oldfocus);
	}
    }
}

static void InsertObject(class flexview  *self)
{
    char objname[250], *defaultviewname, viewname[250];
    class dataobject *d;
    class flex *flex;

    if (self->rightview) {
	message::DisplayString(self, 10,
		"You already have objects here!");
	return;
    }
    if (message::AskForString(self, 10,
		"Data object to insert: ", "flex",
		objname, sizeof(objname)) < 0) {
	message::DisplayString(self, 10, "Cancelled.");
	return;
    }
    d = (class dataobject *) ATK::NewObject(objname);
    if (d == NULL) {
	message::DisplayString(self, 10,
		"Object creation failed.");
	return;
    }
    defaultviewname = (d)->ViewName();
    if (message::AskForString(self, 10, "View to use: ",
	defaultviewname, viewname, sizeof(viewname)) < 0) {
	return;
    }
    flex = (class flex *) (self)->GetDataObject();
    if (!(flex)->InsertObject( d, viewname)) {
	message::DisplayString(self, 10,
		"Could not insert object -- sorry.");
	return;
    }
    (flex)->NotifyObservers( observable_OBJECTCHANGED);
}

static void DeleteObjects(class flexview  *self)
{
    char Prompt[256];
    long result;
    static char *Choices[] = {
	"Delete both sub-objects", "Cancel", NULL};
    class flex *flex;

    if (!self->rightview) {
	message::DisplayString(self, 10,
		"There is nothing to delete!");
	return;
    }
    flex = (class flex *)
      (self)->GetDataObject();
    sprintf(Prompt,
	 "This pair contains a %s on a %s and a %s on a %s.",
	 (self->leftview)->GetTypeName(),
	 (flex->left)->GetTypeName(),
	 (self->rightview)->GetTypeName(),
	 (flex->right)->GetTypeName());
    if (message::MultipleChoiceQuestion(self, 50, Prompt, 1,
	 &result, Choices, NULL) < 0) return;
    if (result != 0 && result != 1) return;
    /* The following two lines will call
      view_UnlinkTree on self->leftview,
      so we don't need to do it here */
    (self)->SetNth( 0, NULL);
    (self)->SetNth( 1, NULL);
    (self->leftview)->Destroy();
    (self->rightview)->Destroy();
    self->leftview = self->rightview = NULL;
    self->oldleftdata = NULL;
    self->oldrightdata = NULL;
    (flex)->DeleteObjects();
    (flex)->NotifyObservers( observable_OBJECTCHANGED);
    (self)->WantUpdate( self);
    (self)->WantInputFocus( self);
}

static void AlterPair(class flexview  *self)
{
    class view *v;
    char *QVec[8];
    long result;
    class flex *flex = (class flex *)
      (self)->GetDataObject();

    if (!flex || !self->rightview) {
	message::DisplayString(self, 10,
		"You don't have anything to alter here yet.");
	return;
    }
    QVec[0] = "Nothing";
    QVec[1] = "Toggle two views";
    if (flex->vorh == lpair_HORIZONTAL) {
	QVec[2] = "Make split vertical";
    } else {
	QVec[2] = "Make split horizontal";
    }
    if (flex->porf == lpair_PERCENTAGE) {
	QVec[3] = "Keep sizes split by percentages";
    } else {
	QVec[3] = "Make sizes split by percentages";
    }
    if (flex->porf == lpair_TOPFIXED) {
	QVec[4] = "Keep top part fixed size";
    } else {
	QVec[4] = "Make top part fixed size";
    }
    if (flex->porf == lpair_BOTTOMFIXED) {
	QVec[5] = "Keep bottom part fixed size";
    } else {
	QVec[5] = "Make bottom part fixed size";
    }
    if (flex->movable) {
	QVec[6] = "Make split not movable";
    } else {
	QVec[6] = "Make split movable";
    }
    QVec[7] = NULL;
    if (message::MultipleChoiceQuestion(self, 50,
	"What do you want to change?", 0, &result,
	QVec, NULL) < 0) return;
    switch(result) {
	case 0:
	    return; /* Doing nothing */
	case 1:
	    (flex)->ToggleParts();
	    (flex)->NotifyObservers(
			observable_OBJECTCHANGED);
	    v = self->leftview;
	    self->leftview = self->rightview;
	    self->rightview = v;
	    break;
	case 2:
	    flex->vorh = !flex->vorh;
	    break;
	case 3:
	    flex->porf = lpair_PERCENTAGE;
	    break;
	case 4:
	    flex->porf = lpair_TOPFIXED;
	    break;
	case 5:
	    flex->porf = lpair_BOTTOMFIXED;
	    break;
	case 6:
	    flex->movable = !flex->movable;
	    break;
    }
    (flex)->NotifyObservers( observable_OBJECTCHANGED);
}

