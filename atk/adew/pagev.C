/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

/* This code is taken , in part, from the switcher inset of N. Borenstein's
Andrew Toolkit Book . It has been modified and used with the permission
of the author */

#include <andrewos.h>
ATK_IMPL("pagev.H")
#include <pagev.H>
#include <dataobject.H>
#include <view.H>
#include <keymap.H>
#include <keystate.H>
#include <menulist.H>
#include <proctable.H>
#include <message.H>
#include <completion.H>
#include <filetype.H>
#include <page.H>
#include <im.H>


 
   
  

static class keymap *pagev_keymap = NULL;
static class menulist *pagev_menulist = NULL;
static struct proctable_Entry *switchobjproc = NULL;


ATKdefineRegistry(pagev, view, pagev::InitializeClass);
static void SetCurrentView(class pagev  *self,char  *name);
static boolean CheckRightSwitchee(class pagev  *self, boolean  *NeedFullRedraw,struct page_switchee  *cp);
static void NextSwitchee(class pagev  *self);
static void SwitchObject(class pagev  *self, struct page_switchee  *swin /* really a long */);
#if 0
static void AddSwitchee(class pagev  *self);
static void AddSwitcheeFromFile(class pagev  *self);
#endif
static void PasteSwitchee(class pagev  *self);


boolean pagev::InitializeClass()
{
    struct proctable_Entry *proc = NULL;

    pagev_keymap = new keymap;
    pagev_menulist = new menulist;

    proc = proctable::DefineProc("pagev-next-page",
	(proctable_fptr)NextSwitchee, &pagev_ATKregistry_ , NULL,
	"Changes the page to look at the next object.");
    (pagev_keymap)->BindToKey( "^X^N", proc, 0);
    (pagev_menulist)->AddToML(
	"Flip~95,Flip Page~90", proc, 0, 0);
#if 0
    proc = proctable_DefineProc("pagev-add-page",
	AddSwitchee, &pagev_classinfo, NULL,
	"Adds a new thing for the page to switch to.");
    keymap_BindToKey(pagev_keymap, "^X^A", proc, 0);
    menulist_AddToML(pagev_menulist,
	"page~95,Add Page~91", proc, NULL, 0);

    proc = proctable_DefineProc("pagev-add-from-file",
	AddSwitcheeFromFile, &pagev_classinfo, NULL,
	"Adds the contents of a file as a new switchee.");
    keymap_BindToKey(pagev_keymap, "^X5", proc, 0);
    menulist_AddToML(pagev_menulist,
	"page~95,Insert File~92", proc, NULL, 0);
#endif
    proc = proctable::DefineProc("pagev-paste",
	(proctable_fptr)PasteSwitchee, &pagev_ATKregistry_ , NULL,
	"Pastes a switchee from the cut-buffer");
    (pagev_keymap)->BindToKey( "^X5", proc, 0);
    (pagev_menulist)->AddToML(
	"Flip~95,Paste~80", proc, 0, 0);

    proctable::DefineProc("pagev-SetCurrentView",
	(proctable_fptr)SetCurrentView, &pagev_ATKregistry_ , NULL,
	"Takes a string argument and calls page_SetNowPlayingByName");

    switchobjproc = proctable::DefineProc(
	"pagev-switch-object", (proctable_fptr)SwitchObject,
	&pagev_ATKregistry_ , NULL,
	"Switches to a given object.");
    return(TRUE);

}
static void SetCurrentView(class pagev  *self,char  *name)
{
    class page *page = (class page *)
      (self)->GetDataObject();
    (page)->SetNowPlayingByName(name);
}

pagev::pagev()
{
	ATKinit;

    this->ks = keystate::Create(this, pagev_keymap);
    this->ml = (pagev_menulist)->DuplicateML(
				     this);
    if (!this->ks || !this->ml) THROWONFAILURE((FALSE));
    this->Firstswitcheroo = NULL;
    this->NowPlaying = NULL;
    THROWONFAILURE((TRUE));
}

pagev::~pagev()
{
	ATKinit;

    struct pagev_switcheroo *this_c, *next;

    if (this->ks) {
	delete this->ks;
	this->ks = NULL;
    }
    if (this->ml) {
	delete this->ml;
	this->ml = NULL;
    }
    this_c = this->Firstswitcheroo;
    while (this_c) {
	this_c->v->RemoveObserver(this);
	if(this_c->v->GetDataObject()) this_c->v->GetDataObject()->RemoveObserver(this);
	(this_c->v)->Destroy();
	next = this_c->next;
	free(this_c);
	this_c = next;
    }
}

static boolean CheckRightSwitchee(class pagev  *self, boolean  *NeedFullRedraw,struct page_switchee  *cp)
{
    class page *page = (class page *)
      (self)->GetDataObject();
    struct pagev_switcheroo *swtmp;
    boolean pagein;

    if(cp == NULL){
      cp = page->NowPlaying;
      pagein = TRUE;
    }
    else pagein = FALSE;
    *NeedFullRedraw = FALSE;
    if (!cp) {
	self->NowPlaying = NULL;
	*NeedFullRedraw = TRUE;
	return(TRUE);
    }
    if (self->NowPlaying) {
	if (self->NowPlaying->switchee == cp) {
	    return(TRUE);
	}
	for (swtmp = self->Firstswitcheroo;
	     swtmp != NULL;
	     swtmp = swtmp->next) {
	    if (swtmp->switchee == cp) {
		if (self->NowPlaying && pagein) {
		    /* notify of removal */
		    (self->NowPlaying->v)->UnlinkTree();  
		    /* relink so that name stays on arbiterview list */
		    (self->NowPlaying->v)->LinkTree( self);  
		}
		self->NowPlaying = swtmp;
		*NeedFullRedraw = TRUE;
		if(pagein) (self->NowPlaying->v)->WantInputFocus(
				    self->NowPlaying->v);
		return(TRUE);
	    }
	}
    }
    swtmp = (struct pagev_switcheroo *)
      malloc(sizeof(struct pagev_switcheroo));
    if (swtmp) {
	const char *cpv=cp->viewname;
	if(cp->d && ATK::IsTypeByName(cp->d->GetTypeName(), "unknown")) {
	    if(!ATK::IsTypeByName(cpv, "unknownv")) cpv="unknownv";
	}
	swtmp->v = (class view *)
	  ATK::NewObject(cpv);
	if (swtmp->v) {
	    (swtmp->v)->SetDataObject(
			cp->d);
	    swtmp->next = self->Firstswitcheroo;
	    swtmp->switchee = cp;
	    self->Firstswitcheroo = swtmp;
	    if (self->NowPlaying && pagein) {
		/* notify of removal */
		(self->NowPlaying->v)->UnlinkTree();  
		/* relink so that name stays on arbiterview list */
		(self->NowPlaying->v)->LinkTree( self);  
	    }
	    self->NowPlaying = swtmp;
	    (self->NowPlaying->v)->LinkTree( self);
	    *NeedFullRedraw = TRUE;
	    if(pagein) (self->NowPlaying->v)->WantInputFocus(
				self->NowPlaying->v);
	    (self->NowPlaying->v)->AddObserver(self);
	    return(TRUE);
	}
    }
    return(FALSE);
}

void pagev::FullUpdate(enum view_UpdateType  type, long  left, long  top, long  width, long  height)
{
    struct rectangle Rect;
    boolean NeedFull; /* ignored */

    if (!CheckRightSwitchee(this, &NeedFull, NULL)) return;
    (this)->GetVisualBounds( &Rect);
    if (!this->NowPlaying) {
	(this)->MoveTo( (Rect.left + Rect.width) / 2,
			    (Rect.top + Rect.height) / 2);
	(this)->DrawString( "<No objects>",
	    graphic::BETWEENLEFTANDRIGHT
	    | graphic::BETWEENTOPANDBASELINE);
	return;
    }
    (this->NowPlaying->v)->InsertView( this, &Rect);
    (this->NowPlaying->v)->FullUpdate( type, left,
		     top, width, height);
}

void pagev::Update()
{
    boolean NeedFullRedraw;

    if (!CheckRightSwitchee(this, &NeedFullRedraw, NULL)) return;
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

class view *pagev::Hit(enum view_MouseAction  action, long	 x, long	 y, long	 numberOfClicks)
{
    if (!this->NowPlaying) {
	(this)->WantInputFocus( this);
	return((class view *) this);
    }
    return((this->NowPlaying->v)->Hit( action,
		     x, y, numberOfClicks));
}

void pagev::PostKeyState(class keystate  *ks)
{
    this->ks->next = NULL;
    (this->ks)->AddBefore( ks);
    (this)->view::PostKeyState( this->ks);
}

void pagev::PostMenus(class menulist  *ml)
{
    struct page_switchee *sw;
    class page *page = (class page *)
      (this)->GetDataObject();
    char MenuBuf[200];
    if((page)->GetPostMenus() == FALSE){
	(this)->view::PostMenus(ml);
	return;
    }
    (this->ml)->ClearChain();
    for (sw = page->FirstSwitchee; sw; sw = sw->next) {
	sprintf(MenuBuf, "Flip~95,%s", (page)->GetSwitcheeName(sw));
	if (this->NowPlaying && (page->NowPlaying == sw)) {
	    (this->ml)->DeleteFromML( MenuBuf);
	} else {
	    (this->ml)->AddToML( MenuBuf,
			     switchobjproc, (long) sw, 0);
	}
    }
    if (ml) (this->ml)->ChainBeforeML( ml, (long)ml);
    (this)->view::PostMenus( this->ml);
}

void pagev::LinkTree(class view  *parent)
{
    (this)->view::LinkTree( parent);
    if (this->NowPlaying) {
	(this->NowPlaying->v)->LinkTree( this);
    }
}

void pagev::WantInputFocus(class view  *v)
{
    if (this->NowPlaying && (v == (class view *) this)) {
	v = this->NowPlaying->v;
    }
    (this)->view::WantInputFocus( v);
}

#if 0
static void AddSwitchee(class pagev  *self)
{
    char ObjName[150], ViewName[150], Label[150];
    class dataobject *d;
    class page *sw;

    if (message::AskForString(self, 10, "Object to insert: ", 
			      NULL, ObjName,
			      sizeof(ObjName)) < 0) {
	message::DisplayString(self, 10, "Cancelled.");
	return;
    }
    d = (class dataobject *) ATK::NewObject(ObjName);
    if (d == NULL) {
	message::DisplayString(self, 10,
			"Could not create new object; sorry.");
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
    sw = (class page *) (self)->GetDataObject();
    if (!(sw)->AddObject( d, Label, ViewName,page_AFTERCURRENT)) {
	message::DisplayString(self, 10,
		"Object creation failed; sorry!");
	return;
    }
    (sw)->SetNowPlaying( d);
 /*   NextSwitchee(self);
    page_NotifyObservers(sw, observable::OBJECTCHANGED); */
}
#endif

static void NextSwitchee(class pagev  *self)
{
    class page *page = (class page *)
      (self)->GetDataObject();
    (page)->SetNowPlayingByPosition(page_AFTERCURRENT);
}

static void SwitchObject(class pagev  *self, struct page_switchee  *swin /* really a long */)
{
    class page *page = (class page *)
      (self)->GetDataObject();
    struct page_switchee *sw;

    for (sw= page->FirstSwitchee; sw; sw = sw->next) {
	if (sw == swin) {
	    (page)->SetNowPlaying( sw->d);
	    return;
	}
    }
    message::DisplayString(self, 10,
	"SwitchObject called for nonexistent object.");
}

#if 0
static void AddSwitcheeFromFile(class pagev  *self)
{
    char FileName[150], ViewName[150], Label[150], *ObjName;
    class dataobject *d;
    FILE *fp;
    long ID;
    class page *sw;

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
		"Could not create new object; sorry.");
	return;
    }
    if ((d)->Read( fp, ID) != dataobject::NOREADERROR) {
	fclose(fp);
	message::DisplayString(self, 10,
		"Read operation failed; sorry.");
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
    sw = (class page *) (self)->GetDataObject();
    if (!(sw)->AddObject( d, Label, ViewName,page_AFTERCURRENT)) {
	message::DisplayString(self, 10,
		"Object creation failed; sorry!");
	return;
    }
    (sw)->SetNowPlaying( d);
}
#endif
static void PasteSwitchee(class pagev  *self)
{
    char FileName[150], ViewName[150], Label[150], *ObjName;
    class dataobject *d;
    FILE *fp;
    long ID;
    class page *sw;

    fp = ((self)->GetIM())->FromCutBuffer();

    if (!fp) {
	message::DisplayString(self, 10, "Could not open file.");
	return;
    }
    ObjName = filetype::Lookup(fp, FileName, &ID, NULL);
    d = (class dataobject *) ATK::NewObject(ObjName);
    if (!d) {
	((self)->GetIM())->CloseFromCutBuffer( fp);
	message::DisplayString(self, 10,
		"Could not create new object; sorry.");
	return;
    }
    if ((d)->Read( fp, ID) != dataobject::NOREADERROR) {
	((self)->GetIM())->CloseFromCutBuffer( fp);
	message::DisplayString(self, 10,
		"Read operation failed; sorry.");
	return;
    }
    ((self)->GetIM())->CloseFromCutBuffer( fp);
    strcpy(ViewName,(d)->ViewName());
    if(ATK::IsTypeByName((d)->GetTypeName(),"cel")) *Label = '\0';
    else if (message::AskForString(self, 10,
		"Label for this object: ", NULL, Label,
		sizeof(Label)) < 0) {
	message::DisplayString(self, 10, "Cancelled.");
	(d)->Destroy();
	return;
    }
    sw = (class page *) (self)->GetDataObject();
    if (!(sw)->AddObject( d, Label, ViewName,page_AFTERCURRENT)) {
	message::DisplayString(self, 10,
		"Object creation failed; sorry!");
	return;
    }
    (sw)->SetNowPlaying( d);
}

view_DSattributes pagev::DesiredSize(long  width, long  height, enum view_DSpass  pass, long  *dWidth, long  *dHeight)
{
    if(this->NowPlaying && this->NowPlaying->v){
	return (this->NowPlaying->v)->DesiredSize( width  , height  , pass, dWidth, dHeight);
    }
    return (this)->view::DesiredSize( width, height, pass, dWidth, dHeight);
}

void pagev::InitChildren()
{
    struct page_switchee *sw;
    struct pagev_switcheroo *safe;
    int NeedFullRedraw;
    class page *page = (class page *)
      (this)->GetDataObject();

    safe = this->NowPlaying;
    for (sw = page->FirstSwitchee; sw; sw = sw->next) {
	CheckRightSwitchee(this, &NeedFullRedraw, sw);
/*	if(self->NowPlaying != safe) 
	    view_UnlinkTree(self->NowPlaying->v); */
    }
    this->NowPlaying = safe;
}

void pagev::Print(FILE  *file, const char  *processor, const char  *finalFormat, boolean  topLevel)
{
    struct page_switchee *sw;
    struct pagev_switcheroo *safe;
    int NeedFullRedraw;
    class page *page = (class page *)
      (this)->GetDataObject();

    safe = this->NowPlaying;
    for (sw = page->FirstSwitchee; sw; sw = sw->next) {
	if(safe != NULL && safe->switchee == sw){
	    (safe->v)->Print(file, processor, finalFormat, topLevel);
	    if(strcmp(processor,"troff") == 0  && (sw->next != NULL)) fprintf(file,".bp\n");
	    continue;
	}
	CheckRightSwitchee(this, &NeedFullRedraw, sw);
	if(this->NowPlaying && this->NowPlaying->v){
	    (this->NowPlaying->v)->Print(file, processor, finalFormat, topLevel);
	    if(strcmp(processor,"troff") == 0 && (sw->next != NULL)) 
		fprintf(file,".bp\n");
	}
/*	view_UnlinkTree(self->NowPlaying->v); */
    }
    this->NowPlaying = safe;
}
void pagev::ObservedChanged(class observable  *changed, long  value)
{
    struct pagev_switcheroo *swtmp,*last=NULL;

    if(value == observable::OBJECTDESTROYED){
	if(changed == (class observable *) (this)->GetDataObject()) 
	{
	    (this)->UnlinkTree();
	    (this)->Destroy();
	    return;
	}
	for (swtmp = this->Firstswitcheroo;
	     swtmp != NULL;
	     swtmp = swtmp->next) {
	    if (swtmp->v == (class view *) changed) {
		if(this->Firstswitcheroo == swtmp)
		    this->Firstswitcheroo = swtmp->next;
		else if(last)
		    last->next = swtmp->next;
		if (this->NowPlaying == swtmp) {
		    this->NowPlaying = NULL;
		}
		((class page *) (this)->GetDataObject())->DeleteObject(swtmp->switchee->d);
		free(swtmp);
		(this)->PostMenus( NULL);
		return;
	    }
	    last = swtmp;
	}

    }
    (this)->view::ObservedChanged( changed, value);
}
