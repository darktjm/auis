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
#include <flexd.H>
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
	"flex~96,Insert Object", proc, 0, 0);

    proc = proctable::DefineProc("flexview-alter-pair", (proctable_fptr)AlterPair, &flexview_ATKregistry_ , NULL, "Alters some of the settings of the flex.");
    (flexview_keymap_views)->BindToKey( "^X%",
		      proc, 0);
    (flexview_menulist_views)->AddToML(
	"flex~96,Alter Pair", proc, 0, 0);

    proc = proctable::DefineProc("flexview-delete-objects", (proctable_fptr)DeleteObjects, &flexview_ATKregistry_ , NULL, "Delete the children of of the flex.");
    (flexview_keymap_views)->BindToKey( "^X&", proc, 0);
    (flexview_menulist_views)->AddToML(
	"flex~96,Delete Objects", proc, 0, 0);

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
    class flexd *flexd = (class flexd *)
      (this)->GetDataObject();

    if (flexd && flexd->right) {
	if ((this->rightview != NULL) || CreateViews(this)) {
	    if ((this->oldleftdata != flexd->left)
		|| (this->oldrightdata != flexd->right)) {
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
    class flexd *flexd = (class flexd *)
      (this)->GetDataObject();

    if (flexd && flexd->right) {
	if (this->rightview || CreateViews(this)) {
	    int porf, vorh, movable;

	    if ((this->oldleftdata != flexd->left)
		|| (this->oldrightdata != flexd->right)) {
		ResetViews(this);
	    }
	    (this)->GetLPState( &porf,
				    &vorh, &movable);
	    if ((porf != flexd->porf) || (vorh != flexd->vorh)
		|| (movable != flexd->movable)) {
		(this)->SetLPState( flexd->porf,
			flexd->vorh, flexd->movable);
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
    class flexd *flexd = (class flexd *)
      (this)->GetDataObject();

    if (flexd && this->rightview) {
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
	    (flexd)->SetDisplayParams( porf, vorh,
				      movable, pct);
	    (flexd)->NotifyObservers(
			observable::OBJECTCHANGED);
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

view_DSattributes flexview::DesiredSize(long  width , long  height, enum view_DSpass  pass, long  *desiredwidth , long  *desiredheight)
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
    class flexd *flexd = (class flexd *)
      (self)->GetDataObject();

    self->leftview = (class view *)
      ATK::NewObject(flexd->lvname);
    if (self->leftview == NULL) return(FALSE);
    (self->leftview)->SetDataObject( flexd->left);
    self->rightview = (class view *)
      ATK::NewObject(flexd->rvname);
    if (self->rightview == NULL) return(FALSE);
    (self->rightview)->SetDataObject( flexd->right);
    (self)->SetUp( self->leftview, self->rightview,
	flexd->pct, flexd->porf, flexd->vorh, flexd->movable);
    (self)->SetLPState( flexd->porf, flexd->vorh,
	flexd->movable);
    (self->leftview)->WantInputFocus( self->leftview);
    self->oldleftdata = flexd->left;
    self->oldrightdata = flexd->right;
    return(TRUE);
}

static void ResetViews(class flexview  *self)
{
    class flexd *flexd = (class flexd *)
      (self)->GetDataObject();
    class view *oldfocus;

    if (self->oldleftdata == flexd->right) {
	oldfocus = ((self)->GetIM())->GetInputFocus();
	(self)->SetNth( 0, self->leftview);
	(self)->SetNth( 1, self->rightview);
	self->oldleftdata = flexd->left;
	self->oldrightdata = flexd->right;
	(self)->WantUpdate( self);
	if (oldfocus != NULL) {
	    (oldfocus)->WantInputFocus( oldfocus);
	}
    }
}

static void InsertObject(class flexview  *self)
{
    char objname[250], viewname[250];
    const char *defaultviewname;
    class dataobject *d;
    class flexd *flexd;

    if (self->rightview) {
	message::DisplayString(self, 10,
		"You already have objects here!");
	return;
    }
    if (message::AskForString(self, 10,
		"Data object to insert: ", "flexd",
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
    flexd = (class flexd *) (self)->GetDataObject();
    if (!(flexd)->InsertObject( d, viewname)) {
	message::DisplayString(self, 10,
		"Could not insert object -- sorry.");
	return;
    }
    (flexd)->NotifyObservers( observable::OBJECTCHANGED);
}

static void DeleteObjects(class flexview  *self)
{
    char Prompt[256];
    long result;
    static const char * const Choices[] = {
	"Delete both sub-objects", "Cancel", NULL};
    class flexd *flexd;

    if (!self->rightview) {
	message::DisplayString(self, 10,
		"There is nothing to delete!");
	return;
    }
    flexd = (class flexd *)
      (self)->GetDataObject();
    sprintf(Prompt,
	 "This pair contains a %s on a %s and a %s on a %s.",
	 (self->leftview)->GetTypeName(),
	 (flexd->left)->GetTypeName(),
	 (self->rightview)->GetTypeName(),
	 (flexd->right)->GetTypeName());
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
    (flexd)->DeleteObjects();
    (flexd)->NotifyObservers( observable::OBJECTCHANGED);
    (self)->WantUpdate( self);
    (self)->WantInputFocus( self);
}

static void AlterPair(class flexview  *self)
{
    class view *v;
    const char *QVec[8];
    long result;
    class flexd *flexd = (class flexd *)
      (self)->GetDataObject();

    if (!flexd || !self->rightview) {
	message::DisplayString(self, 10,
		"You don't have anything to alter here yet.");
	return;
    }
    QVec[0] = "Nothing";
    QVec[1] = "Toggle two views";
    if (flexd->vorh == lpair_HORIZONTAL) {
	QVec[2] = "Make split vertical";
    } else {
	QVec[2] = "Make split horizontal";
    }
    if (flexd->porf == lpair_PERCENTAGE) {
	QVec[3] = "Keep sizes split by percentages";
    } else {
	QVec[3] = "Make sizes split by percentages";
    }
    if (flexd->porf == lpair_TOPFIXED) {
	QVec[4] = "Keep top part fixed size";
    } else {
	QVec[4] = "Make top part fixed size";
    }
    if (flexd->porf == lpair_BOTTOMFIXED) {
	QVec[5] = "Keep bottom part fixed size";
    } else {
	QVec[5] = "Make bottom part fixed size";
    }
    if (flexd->movable) {
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
	    (flexd)->ToggleParts();
	    (flexd)->NotifyObservers(
			observable::OBJECTCHANGED);
	    v = self->leftview;
	    self->leftview = self->rightview;
	    self->rightview = v;
	    break;
	case 2:
	    flexd->vorh = !flexd->vorh;
	    break;
	case 3:
	    flexd->porf = lpair_PERCENTAGE;
	    break;
	case 4:
	    flexd->porf = lpair_TOPFIXED;
	    break;
	case 5:
	    flexd->porf = lpair_BOTTOMFIXED;
	    break;
	case 6:
	    flexd->movable = !flexd->movable;
	    break;
    }
    (flexd)->NotifyObservers( observable::OBJECTCHANGED);
}

