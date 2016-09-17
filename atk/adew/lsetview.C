/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'     *
\* ********************************************************************** */

/*
	$Disclaimer: 
// Permission to use, copy, modify, and distribute this software and its 
// documentation for any purpose and without fee is hereby granted, provided 
// that the above copyright notice appear in all copies and that both that 
// copyright notice and this permission notice appear in supporting 
// documentation, and that the name of IBM not be used in advertising or 
// publicity pertaining to distribution of the software without specific, 
// written prior permission. 
//                         
// THE COPYRIGHT HOLDERS DISCLAIM ALL WARRANTIES WITH REGARD 
// TO THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF 
// MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL ANY COPYRIGHT 
// HOLDER BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL 
// DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, 
// DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE 
// OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION 
// WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
// 
//  $
*/

#include <andrewos.h> /* strings.h */
ATK_IMPL("lsetview.H")
#include <lsetview.H>
#include <lset.H>
#include <view.H>
#include <cursor.H>
#include <keystate.H>
#include <keymap.H>
#include <message.H>
#include <im.H>
#include <graphic.H>
#include <proctable.H>
#include <menulist.H>
#include <filetype.H>
#include <dataobject.H>
#include <atom.H>
#include <atomlist.H>
#include <rm.H>
#include <valueview.H>
#include <text.H>

static class keymap *newKeymap;
static class menulist *newMenus;
static class lsetview *DeleteMode = NULL;

#define DataObject(A) (A->dataobject)
#define Data(A) ((class lset *)DataObject(A) )
#define Islinked(self) (((struct view *)self)->parent != NULL && self->GetIM())
#define lsetview_NeedLink 10
#define VALUE 10
#define CEL 5
struct types {
    const char *str;
    int val;
};
static const struct types typearray[] = {
    {"string",STRING},
    {"char *",STRING},
    {"int",LONG},
    {"long",LONG},
    {"float",FLOAT},
    {"",0}
};


ATKdefineRegistry(lsetview, lpair, lsetview::InitializeClass);
static class view *makeview(class lsetview  *self,class lset  *ls);
void lsetview_SetMode(class lsetview  *self,int  mode);
static void initkids(class lsetview  *self,class lset  *ls);
static void dolink(class lsetview  *self);
static boolean objecttest(class lsetview   *self,const char  *name,const char  *desiredname);
static void lsetview_PlaceApplication(class lsetview  *self);
static void lsetview_PlaceCel(class lsetview  *self);
static void lsetview_PlaceValue(class lsetview  *self);
static void lsetview_DestroyView(class lsetview  *self);
static int lsetview_PlaceView(class lsetview  *self);
void lsetview_DeleteMode(class lsetview  *self);
void lsetview_UnsplitParent(class lsetview  *self);
int lsetview_ReadView(class lsetview  *self);
void lsetview_Paste(class lsetview  *self);
static void lsetview_SplitVert(class lsetview  *self);
static void lsetview_SplitHorz(class lsetview  *self);


static class view *makeview(class lsetview  *self,class lset  *ls)
{

    const char *lv=ls->viewname;
    if(ls->dobj && ATK::IsTypeByName(ls->dobj->GetTypeName(), "unknown")) {
	if(lv==NULL || !ATK::IsTypeByName(lv, "unknownv")) {
	    lv="unknownv";
	}
    }
    if(*lv != '\0' /* && ls->dobj != NULL */ && (self->child = (class view *) ATK::NewObject(lv)))  {
	(self->child)->AddObserver(self);
	(self)->RetractCursor(self->cursorp);
	self->mode = lsetview_NoUpdate;
	if(ls && ls->dobj != NULL) 
	    (self->child)->SetDataObject(ls->dobj);
	if(ls->refname && *ls->refname)
	    (self->child)->SetName(atomlist::StringToAtomlist(ls->refname));


	if(ls->application == TRUE){
	    self->app = (self->child)->GetApplicationLayer();
	}
	if(self->app == NULL) self->app = self->child;
	(self->app)->LinkTree(self);
	self->mode = lsetview_FirstUpdate;
	return self->child;
    }
    return NULL;
}
void lsetview_SetMode(class lsetview  *self,int  mode)
{
self->mode = mode;
}
class lsetview *lsetview::Create(int  level,class lset  *d,class view  *parent)
    {
	ATKinit;

    class lsetview *lv;
    if(d == NULL) d = new lset;
    lv = new lsetview;
    (lv)->SetDataObject(d);
    (lv)->LinkTree(parent);
   lv->level = level;
    return(lv);
}
static void initkids(class lsetview  *self,class lset  *ls)
{
	class lsetview *v1,*v2;
	v1 = lsetview::Create(self->level+1,(class lset *) ls->left,(class view *)self);
	v2 = lsetview::Create(self->level+1,(class lset *)ls->right,(class view *)self);
	if(ls->type == lsetview_MakeHorz)
	    (self)->HSplit(v1,v2,ls->pct,TRUE);
	else
	    (self)->VSplit(v1,v2,ls->pct,TRUE);
	self->mode = lsetview_IsSplit;
	(self)->WantUpdate(self);
    }	
static void dolink(class lsetview  *self)
{
    class lset *ls;
    ls = Data(self);
    if(Islinked(self)){
	self->mode = lsetview_UnInitialized;
	if(ls->left && ls->right)
	    initkids(self,ls);
	else if(makeview(self,ls) )
	    (self)->WantUpdate(self);
    }
}

void lsetview::SetDataObject(class dataobject  *ls)
{
    this->revision = ((class lset *)ls)->revision;
   if(Data(this) != ls) (this)->lpair::SetDataObject(ls);
   this->mode = lsetview_NeedLink;
}

class view *lsetview::Hit(enum view_MouseAction  action,long  x ,long  y ,long  numberOfClicks)
{
long size;
int pct;
class lsetview *v1,*v2;
class lset *ls;
class view *vw;
    switch(this->mode){
	case lsetview_NeedLink:
	    return (class view *)this;	   
        case lsetview_IsSplit:
            vw = (this)->lpair::Hit(action,x,y,numberOfClicks);
	    if(DeleteMode && vw == (class view *) this && !(DeleteMode)->IsAncestor(this)){
		lsetview_DestroyView(this);
		DeleteMode = NULL;
		return (class view *)this;
	    }
	    return vw;

        case lsetview_MakeHorz:
            size = (this)->GetLogicalWidth();
            pct = 100 - (x * 100 )/ size;
            break;
        case lsetview_MakeVert:
            size = (this)->GetLogicalHeight();
            pct = 100 - (y * 100 )/ size;
            break;
	case lsetview_HasView:
	    if(DeleteMode && !(DeleteMode)->IsAncestor(this)){
		lsetview_DestroyView(this);
		DeleteMode = NULL;
		return (class view *)this;
	    }
	    return((this->app)->Hit(action,x,y,numberOfClicks));
        default:
	    if(DeleteMode ){
		message::DisplayString(NULL,0,"Delete Mode Canceled");
		DeleteMode = NULL;
	    }
            if(!(this->HasFocus)) (this)->WantInputFocus(this);
            return((class view *)this);
    }
    if(action != view_LeftDown) return((class view *)this);
    v1 = lsetview::Create(this->level+1,NULL,(class view *)this);
    v2 = lsetview::Create(this->level+1,NULL,(class view *)this);
    /* update dataobject */

    ls = Data(this);
    ls->left = DataObject(v1);
    ls->right =  DataObject(v2);
    ls->pct = pct;
    ls->type = this->mode;
    if(this->mode == lsetview_MakeHorz)
        (this)->HSplit(v1,v2,pct,TRUE);
    else
        (this)->VSplit(v1,v2,pct,TRUE);
    this->mode = lsetview_IsSplit;
/*    lsetview_WantUpdate(self,self); */
    (ls)->NotifyObservers(0);

    return((class view *)this);
}
static boolean objecttest(class lsetview   *self,const char  *name,const char  *desiredname)
{
    if(ATK::LoadClass(name) == NULL){
        char foo[640];
        sprintf(foo,"Can't load %s",name);
         message::DisplayString(self, 0, foo);
        return(FALSE);
    }
    if(! ATK::IsTypeByName(name,desiredname)){
        char foo[640];
        sprintf(foo,"%s is not a %s",name,desiredname);
         message::DisplayString(self, 0, foo);
        return(FALSE);
    }
    return(TRUE);
}
static void lsetview_PlaceApplication(class lsetview  *self)
{
    if(self->child || self->mode == lsetview_IsSplit) return;
    Data(self)->application = TRUE;
    lsetview_PlaceView(self);
}
static void lsetview_PlaceCel(class lsetview  *self)
{
    if(self->child || self->mode == lsetview_IsSplit) return;
    Data(self)->application = CEL;
    lsetview_PlaceView(self);
}

static void lsetview_PlaceValue(class lsetview  *self)
{
    if(self->child || self->mode == lsetview_IsSplit) return;
    Data(self)->application = VALUE;
    self->promptforparameters = 1;
    lsetview_PlaceView(self);
}
static void lsetview_DestroyView(class lsetview  *self)
{
    class lset *ls = Data(self);
/*    struct lpair *lp = (struct lpair *) self; */
/*
    lset_RemoveObserver(ls,self);
    lset_Finalize(ls);
    lset_Initialize(ls);
*/
/*    lset::InitializeObject(ls); XXX, huh? what's the purpose? -rr2b */
    ls->revision = self->revision;
    if(self->app){
	(self->app)->UnlinkTree(); 
	(self->app)->Destroy();
    }
    self->HasFocus = 0;
    self->mode = lsetview_UnInitialized;
    self->child = NULL;
    self->app = NULL;
    self->promptforparameters = 0;
    self->pdoc = NULL;
    dolink(self);
    (self)->WantInputFocus(self);
/*    if (lp->obj[0] != NULL)
        view_UnlinkTree(lp->obj[0]);
    if (lp->obj[1] != NULL)
        view_UnlinkTree(lp->obj[1]);
*/
}
static int lsetview_PlaceView(class lsetview  *self)
{
/* if(self->level == 0) return; */
/* prompt for dataobject */
/* create object and view */

    char iname[100];
    char viewname[200];
    long pf; 
    class lset *ls = Data(self);
    boolean promptforname = self->GetIM()->ArgProvided();
    (self->imPtr)->ClearArg();    
    if(self->child || self->mode == lsetview_IsSplit) return -1;
   viewname[0] = '\0';
#ifdef DEBUG
printf("in Place View %d\n",ls->application );
#endif /* DEBUG */
    if(ls->application == CEL){
	strcpy(iname,"cel");
	promptforname = FALSE;
    }
    else {
	pf = message::AskForString(self, 0, "Data object to insert here??: ", 0, iname, sizeof(iname));
	if (pf < 0) return -1;
    }
#ifdef DEBUG
printf("Still in Place View\n");
#endif /* DEBUG */
    if(strlen(iname)==0) {
	promptforname = TRUE;
    }
    else if(ls->application == VALUE){
	    sprintf(viewname,"%sV",iname);
	    if(!ATK::IsTypeByName(iname,"value")) 
		strcpy(iname,"value");
	    if(!ATK::IsTypeByName(viewname,"valueview")) 
		promptforname = TRUE ;    
	}
    else if(objecttest(self,iname,"dataobject") == FALSE) return -1;
    if(promptforname){
	if( message::AskForString (self, 0, "View to place here ", 0, viewname, 200) < 0)
	    return  -1;
	if(objecttest(self,viewname,"view") == FALSE) 
	    return -1;
    }

    (ls)->InsertObject( iname,viewname);
/*    if(makeview(self,ls) )  */
    (ls)->NotifyObservers(0);
    return 0;
}
void lsetview_DeleteMode(class lsetview  *self)
{
  self->mode = lsetview_Initialized;
    (self)->WantUpdate(self);
    message::DisplayString(self, 0, "Click on lset to delete");
    DeleteMode = self;
}
void lsetview_UnsplitParent(class lsetview  *self)
{
    if(self->child || self->mode == lsetview_IsSplit) return;
    if(self->parent && (self)->IsType(self->parent)){
	class lsetview *parent = (class lsetview *) self->parent;
	(parent)->Unsplit(self);
    }
}

int lsetview_ReadView(class lsetview  *self)
{
/* prompt for dataobject */

    char iname[100];
    long pf;
    char realName[1024];
    FILE *thisFile;
    (self->imPtr)->ClearArg();    
    pf = message::AskForString(self, 0, "File to insert here: ", 0, iname, sizeof(iname));
    if (pf < 0) return -1;
    if(strlen(iname)==0){
	message::DisplayString(self, 0, "No file specified");
	return -1;
    }
    filetype::CanonicalizeFilename(realName, iname, sizeof(realName) - 1);

    if ((thisFile = fopen(realName, "r")) == NULL){
            message::DisplayString(self, 0,"can't open file");
	    return -1;
    }
    (self)->ReadFile(thisFile,iname);
    fclose(thisFile);
    return 0;
}
void lsetview::ReadFile(FILE  *thisFile,const char  *iname)
{
    long objectID;
    const char *objectName;
    class lset *ls = Data(this);
    objectName = filetype::Lookup(thisFile, iname, &objectID, NULL); /* For now, ignore attributes. */
    if(objectName == NULL) objectName = "text";
    if(objecttest(this,objectName,"dataobject") == FALSE) return;
    (ls)->InsertObject( objectName,NULL);
    if(ls->dobj != NULL) (ls->dobj)->Read(thisFile,objectID);
    (ls)->NotifyObservers(0);
}
void lsetview_Paste(class lsetview  *self)
{
    FILE *pasteFile;
    if(self->child || self->mode == lsetview_IsSplit) return;
    pasteFile = ((self)->GetIM())->FromCutBuffer();
    (self)->ReadFile(pasteFile,NULL);
    ((self)->GetIM())->CloseFromCutBuffer( pasteFile);

}

void lsetview::Update()
{
 /*   lsetview_RestoreGraphicsState(self); */
    class lset *ls = Data(this);
    switch(this->mode){
	case lsetview_NoUpdate:
	    return ;
	case lsetview_FirstUpdate:
            (this->cursorp)->SetStandard(Cursor_Arrow);
	case lsetview_UpdateView:
	    /* child view needs reinserted */
	    {
	    struct rectangle rr;
	    long foo1,foo2;
	    (this)->GetVisualBounds(&rr);
	    (this->app)->InsertView( this, &rr);
	    (this->app)->RetractViewCursors(this->app);
	    (this->app)->DesiredSize(rr.width,rr.height,view_NoSet,&foo1,&foo2);
	    (this->app)->FullUpdate( view_FullRedraw,0,0,0,0);

	    if(this->mode == lsetview_FirstUpdate && ls->application == CEL &&
	       this->HasFocus)
		(this->app)->WantInputFocus(this->app);

	    this->mode = lsetview_HasView;
	    return;
	    }
	case lsetview_HasView:
	    (this->app)->Update();
	    return;
	case lsetview_IsSplit:
            if((this->cursorp)->IsPosted())
                (this)->RetractCursor(this->cursorp);
	    if(this->objsize[1] > 0 &&   ls->pct != this->objsize[1]){
		ls->pct = this->objsize[1];
		(ls)->NotifyObservers(0);
	    }
            (this)->lpair::Update(); return ;
        case lsetview_MakeVert:
            (this->cursorp)->SetStandard(Cursor_HorizontalBars);
            break;
        case lsetview_MakeHorz:
            (this->cursorp)->SetStandard(Cursor_VerticalBars);
            break;
        case lsetview_Initialized:
            (this->cursorp)->SetStandard(Cursor_Arrow);
            break;
    }
    if(ls->dobj || ls->left){
	dolink(this);
	return;
    }
    if(this->HasFocus){
        (this)->SetTransferMode(graphic_BLACK);
    }
    else
        (this)->SetTransferMode(graphic_WHITE);
    (this)->EraseVisualRect();
    (this)->SetTransferMode(graphic_INVERT);
   if(!(this->cursorp)->IsPosted()){
       struct rectangle tr;
       (this)->GetVisualBounds(&tr);
       (this)->PostCursor(&tr,this->cursorp);
   }
}
void lsetview::FullUpdate(enum view_UpdateType  type,long  left,long  top,long  width,long  height)
{
    if(this->mode == lsetview_NeedLink)
	dolink(this);
    if(this->mode == lsetview_HasView){
	struct rectangle rr;
	long foo1,foo2;
/*	lsetview_RestoreGraphicsState(self); */
/*	lsetview_GetVisualBounds(self,&rr); */
	(this)->GetLogicalBounds(&rr);
	(this->app)->InsertView( this, &rr);
	(this->app)->RetractViewCursors(this->app);
	(this->app)->DesiredSize(width,height,view_NoSet,&foo1,&foo2);
	(this->app)->FullUpdate(type,left,top,width,height);
    }
    else if (this->mode == lsetview_IsSplit){
        (this)->lpair::FullUpdate(type,left,top,width,height);
    }
    else (this)->Update();
}
void lsetview::ObservedChanged(class observable  *changed, long  value)
                {
	class lset *ls = Data(this);
	class lpair *lp = (class lpair *) this;
	class view *v;
	if(value ==  observable_OBJECTDESTROYED ){
	    if(changed == (class observable *)this->child ||
	       changed == (class observable *) DataObject(this) ){
		this->child = NULL;this->app = NULL;
		lsetview_DestroyView(this);
		/*		self->child = NULL;self->app = NULL;
		lsetview_WantUpdate(self,self); */
	    }
	}
	else if(changed == (class observable *) DataObject(this)){
	    if(this->revision != ls->revision){
		if(this->mode == lsetview_IsSplit){
		    if(lp->obj[0]){
			v = lp->obj[0];
			lp->obj[0] = NULL;
			(v)->UnlinkTree();
			(v)->Destroy();
		    }
		    if(lp->obj[1]){
			v = lp->obj[1];
			lp->obj[1] = NULL;
			(v)->UnlinkTree();
			(v)->Destroy();
		    }
		}
		if(this->app){
		    (this->app)->UnlinkTree();
		    (this->app)->RemoveObserver(this);
		    (this->app)->Destroy();
		}
		this->HasFocus = 0;
		this->mode = lsetview_UnInitialized;
		this->child = NULL;
		this->app = NULL;
		this->promptforparameters = 0;
		this->pdoc = NULL;
		this->level = 0;
		DeleteMode = NULL;
		(this->cursorp)->SetStandard(Cursor_Arrow);
		dolink(this);
	    }
	    else if(Data(this)->pct != this->objsize[1]){
		this->objsize[1] =  Data(this)->pct ;
		this->needsfull = 1;
		(this)->WantUpdate(this);
	    }

	    (this)->lpair::ObservedChanged( changed, value);
	}
    }
boolean lsetview::Unsplit(class lsetview  *who)
{
    class lpair *lp ;
    class view *saved;
    class lset *ll,*oldll;
    lp = (class lpair *) this;
    if(lp->obj[0] == (class view *) who)
	saved =  lp->obj[1];
    else if (lp->obj[1] == (class view *) who)
	saved =  lp->obj[0];
    else return FALSE;
    if(!(saved)->IsType(this)) return FALSE;
    ll = Data(saved);
    oldll = Data(this);

    oldll->type = ll->type;
    oldll->pct = ll->pct;
    strcpy(oldll->dataname,ll->dataname);
    strcpy(oldll->viewname,ll->viewname);
    strcpy(oldll->refname,ll->refname);
    oldll->dobj = ll->dobj;
    oldll->left = ll->left;
    oldll->right = ll->right;
    oldll->application = ll->application;
    oldll->pdoc = ll->pdoc;
    (oldll->revision)++;
    (oldll)->NotifyObservers(0);
   /*
    savedlp = (struct lpair *) saved;
    view_UnlinkTree(lp->obj[0]);
    view_UnlinkTree(lp->obj[1]);
    lp->obj[0] = lp->obj[1] = NULL;
    self->mode = saved->mode;
    self->level = saved->level;
    self->child = saved->child;
    self->app = seved->app;
    self->promptforparameters = saved->promptforparameters;
    if(self->mode == lsetview_IsSplit){
	v1 = savedlp->obj[0];
	v2 = savedlp->obj[1];
	savedlp->obj[0] = savedlp->obj[1] = NULL;
	if(ll->type == lsetview_MakeHorz)
	    lsetview_HSplit(self,v1,v2,lp->pct,TRUE);
	else
	    lsetview_VSplit(self,v1,v2,lp->pct,TRUE);
    }
    else if(self->child){
	view_UnlinkTree(self->child);
	view_LinkTree(self->child,self);
    }
    lsetview_Destroy(saved);
    lsetview_Destroy(who);
    lsetview_WantUpdate(self,self);
    */
   return TRUE;
}
void lsetview::ReceiveInputFocus()
{
    if(this->child){
	(this->child)->WantInputFocus(this->child);
	return;
    }
    if(this->mode == lsetview_IsSplit){
	class lpair *lp = (class lpair *) this;
	(lp->obj[0])->WantInputFocus(lp->obj[0]);
	return;
    }
    this->HasFocus = 1;
    this->keystatep->next = NULL;
    (this)->PostKeyState( this->keystatep); 
    (this)->PostMenus(this->menulistp);
    (this)->WantUpdate(this);

}
void lsetview::LoseInputFocus()
{
this->HasFocus = 0;
if(this->mode == lsetview_MakeVert || this->mode == lsetview_MakeHorz)
    this->mode = lsetview_Initialized;
(this)->WantUpdate(this);
}
lsetview::lsetview()
    
{
	ATKinit;

    this->HasFocus = 0;
    this->mode = lsetview_UnInitialized;
    this->child = NULL;
    this->app = NULL;
    this->keystatep = keystate::Create(this,newKeymap);
    this->menulistp = (newMenus)->DuplicateML(this);
    this->cursorp = cursor::Create(this);
    this->promptforparameters = 0;
    this->pdoc = NULL;
    this->level = 0;
    DeleteMode = NULL;
    THROWONFAILURE( TRUE);
}
static void lsetview_SplitVert(class lsetview  *self)
{
if(self->child || self->mode == lsetview_IsSplit) return;
self->mode = lsetview_MakeHorz;
(self)->WantUpdate(self);
}
static void lsetview_SplitHorz(class lsetview  *self)
{
if(self->child || self->mode == lsetview_IsSplit) return;
self->mode = lsetview_MakeVert;
(self)->WantUpdate(self);
}

void lsetview::LinkTree(class view  *parent)
{
    (this)->lpair::LinkTree( parent);
    if(this->child) 
	(this->child)->LinkTree(this);
    if(this->mode == lsetview_NeedLink) dolink(this);
}
void lsetview::WantNewSize(class view  *requestor)
        {
	this->mode = lsetview_UpdateView;
	(this)->WantUpdate(this);
}
boolean lsetview::InitializeClass()
{
    const char *cmdString;
    struct proctable_Entry *tempProc;
    newMenus = new menulist;
    newKeymap = new keymap;
    tempProc = proctable::DefineProc(cmdString = "lsetview-Insert-Object", (proctable_fptr)lsetview_PlaceView,&lsetview_ATKregistry_ ,NULL, "Set the object of a child view");
/*    keymap_BindToKey(newKeymap,cmdString,NULL,"\033\t"); */
/*    menulist_AddToML(newMenus,"lset,Set Object~31",tempProc,NULL,0); */
    tempProc = proctable::DefineProc("lsetview-Destroy-Object", (proctable_fptr)lsetview_DestroyView,&lsetview_ATKregistry_ ,NULL, "Destroy the child view");
    (newKeymap)->BindToKey("\033d",tempProc,0);


    tempProc = proctable::DefineProc(cmdString = "lsetview-Insert-value", (proctable_fptr)lsetview_PlaceValue,&lsetview_ATKregistry_ ,NULL, "Set the value of a child view");
/*    keymap_BindToKey(newKeymap,cmdString,NULL,"\033\t"); */
/*    menulist_AddToML(newMenus,"lset,Set value~34",tempProc,NULL,0); */

    tempProc = proctable::DefineProc(cmdString = "lsetview-Insert-application", (proctable_fptr)lsetview_PlaceApplication,&lsetview_ATKregistry_ ,NULL, "Set the application of a child view");
    (newMenus)->AddToML("lset,Set application~32",tempProc,0,0);
    tempProc = proctable::DefineProc(cmdString = "lsetview-Insert-Cel", (proctable_fptr)lsetview_PlaceCel,&lsetview_ATKregistry_ ,NULL, "Set the Cel of a child view");
    (newMenus)->AddToML("lset,Set Cel~30",tempProc,0,0);

    tempProc = proctable::DefineProc(cmdString = "lsetview-Paste", (proctable_fptr)lsetview_Paste,&lsetview_ATKregistry_ ,NULL, "Fill lset with contents of cut buffer");
    (newMenus)->AddToML("lset,Paste~36",tempProc,0,0);


/*
    tempProc = proctable_DefineProc(cmdString = "lsetview-Delete-Mode", lsetview_DeleteMode,&lsetview_classinfo,NULL, "sets delete mode");
*/
    tempProc = proctable::DefineProc(cmdString = "lsetview-Unsplit", (proctable_fptr)lsetview_UnsplitParent,&lsetview_ATKregistry_ ,NULL, "Unsplits parent lset");
    (newMenus)->AddToML("lset,Unsplit Lset~40",tempProc,0,0);/*    keymap_BindToKey(newKeymap,cmdString,NULL,"\033t"); */
    tempProc = proctable::DefineProc(cmdString = "lsetview-Read-File", (proctable_fptr)lsetview_ReadView,&lsetview_ATKregistry_ ,NULL, "read a file into a child view");
/*    keymap_BindToKey(newKeymap,cmdString,NULL,"\030\t"); */
    (newMenus)->AddToML("lset,Insert File~35",tempProc,0,0);

    tempProc = proctable::DefineProc(cmdString = "lsetview-Split-Horz", (proctable_fptr)lsetview_SplitHorz,&lsetview_ATKregistry_ ,NULL, "split the lpair Horizontally");
/*    keymap_BindToKey(newKeymap,cmdString,NULL,"\033\t"); */
    (newMenus)->AddToML("lset,Split Horizontal~11",tempProc,0,0);
    tempProc = proctable::DefineProc(cmdString = "lsetview-Split-Vert", (proctable_fptr)lsetview_SplitVert,&lsetview_ATKregistry_ ,NULL, "split the lpair Vertically");
/*    keymap_BindToKey(newKeymap,cmdString,NULL,"\033\t"); */
    (newMenus)->AddToML("lset,Split Vertically~10",tempProc,0,0);
    return TRUE;
}
void lsetview::InitChildren()
{
    if(this->child) 
	(this->child)->InitChildren();
    else     (this)->lpair::InitChildren();

}
boolean lsetview::CanView(const char  *TypeName)
{
    return ATK::IsTypeByName(TypeName,"lset");
}
void lsetview::Print(FILE  *file, const char  *processor, const char  *finalFormat, boolean  topLevel)
{
    class lpair *lself = (class lpair *) this;
    if(this->child) 
	(this->child)->Print(file, processor, finalFormat, topLevel);
    else {
	if (lself->obj[0] != NULL)
	    (lself->obj[0] )->Print(file, processor, finalFormat, topLevel);
	if (lself->obj[1] != NULL)
	    (lself->obj[1] )->Print(file, processor, finalFormat, topLevel);
    }
}
lsetview::~lsetview()
{
	ATKinit;

    if(this->menulistp) delete this->menulistp;
    if(this->app){
	class view *child = this->child;
	(child)->UnlinkTree();
	this->child = NULL;
	(child)->RemoveObserver(this);
	(child)->Destroy();
    }
}
