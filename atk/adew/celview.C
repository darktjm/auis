/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

#define UNSET 0
#define FUDGE 2
#define celview_ChangeWidth 1
#define celview_ChangeHeight 2
#define STRING 1
#define LONG 2
#define FLOAT 3
#define INITNOW 100
#define VALUE 10
#define ChildMenus 1
#define ClientMenus 2
#include <andrewos.h>
ATK_IMPL("celview.H")

#include <ctype.h>
#include <bind.H>
#include <view.H>
#include <cel.H>
#include <menulist.H>
#include <cursor.H>
#include <graphic.H>
#include <im.H>
#include <keymap.H>
#include <keystate.H>
#include <atom.H>
#include <atomlist.H>
#include <rm.H>
#include <valueview.H>
#include <text.H>
#include <message.H>
#include <arbiterview.H>
#include <arbcon.H>
#include <filetype.H>
#include <dataobject.H>
#include <completion.H>
#include <proctable.H>
#include <celview.H>
#include <arbiter.H>
/* #define DEBUG */

#define DataObject(A) (A->dataobject)
#define Parent(A) (A->parent)
#define Data(A) ((class cel *)DataObject(A) )
#define VALUE 10
static class atom *UNSETFLAG;
#define RESIZING FALSE
#define DRAWING FALSE
static class menulist *celviewMenus;
#define DataObject(A) (A->dataobject)
#define Cel(A) ((class cel *) DataObject(A))
#define NameSet(V) (((class view *)V)->name_explicitly_set)
struct overlay  {
class dataobject *object;
class view *view;
struct rectangle rect;
long flags;
struct overlay *next;
};
#define SCALEWID
#define DOINDENT(SELF) (SELF->drawing  || SELF->resizing)
#define celview_COVERCHILD 1

ATKdefineRegistry(celview, view, celview::InitializeClass);
static void scaleoverlay(class celview  *self,struct overlay  *ov,struct rectangle  *Or);
static class view *PopOverlay(class celview  *self,class view  *v);
static void UpdateCursors(class celview  *self);
static void UpdateDrawing(class celview  *self);
static void initchild(class celview  *self);
static char * trunc(char  *c);
static void celview_ReadFile(class celview  *self,FILE  *thisFile,char  *iname);
static void celview_Paste(class celview  *self);
static void celview_PromptForFile(class celview  *self);
static void InitNow(class celview  *self);
static void drawshadow(class celview  *self,struct rectangle  *r);
static void SetVisible(class celview  *self);
static void SetInvisible(class celview  *self);
static boolean objecttest(class celview   *self,const char  *name,const char  *desiredname);
static int lookuptype(char  *ty);
static char * atomlisttostring(class atomlist  *al);
static void appendresourceList( class celview  * self, struct resourceList  * resources);
static void editresourceList( class celview  * self, struct resourceList  * resources,int  askres,int  maxcount );
static boolean StringToResourceList(struct resourceList  *rl,char  *str);
static void GetParameters(class celview  *self);
static void PostParameters(class celview  *self);


static void scaleoverlay(class celview  *self,struct overlay  *ov,struct rectangle  *Or)
{
    struct rectangle *nr = &(self->enclosingRect);
    struct rectangle *sr = &(ov->rect);
/*     if(ov->flags & celview_COVERCHILD) {
	*sr = *Or;
	return;
    } */
    if(Or->width > 5 && Or->height > 5 && nr->width > 5 && nr->height > 5)
    {
	if(Or->height != nr->height){
	    sr->top = (sr->top * nr->height) / Or->height;
	    sr->height = (sr->height * nr->height) / Or->height;
	}
	if(Or->width != nr->width){
	    sr->left = (sr->left * nr->width) / Or->width;
	    sr->width = (sr->width * nr->width) / Or->width;
	}
    }
}
static class view *PopOverlay(class celview  *self,class view  *v)
{
    if(self->olist){
	struct overlay *o = NULL,*dm;
	dm = self->olist;
	if(v != NULL) {
	    for(;dm != NULL;dm = dm->next){
		if(dm->view == v) break;
		o = dm;
	    }
	    if(dm == NULL) return NULL;
	}
	v = dm->view;
	if(dm != self->olist) o->next = dm->next;
	else{
	    o = dm->next;
	    if((self->olist = o) == NULL){
		self->child  = self->safec ;
		self->truechild = self->safetc;
	    }
	    else self->child = self->truechild = self->olist->view;
	}
	(v)->UnlinkTree();
	self->mode = celview_UpdateView;
	(self)->WantUpdate(self);
	free(dm);
	return v;
    }
    return NULL;
}
static void UpdateCursors(class celview  *self)
{
    switch(self->Moving){
	case celview_ChangeWidth:
	    im::SetProcessCursor(self->widthcursor);
	    break;
	case celview_ChangeHeight:
	     im::SetProcessCursor(self->heightcursor);
	    break;
	default:
	    if(self->WasMoving){
		im::SetProcessCursor(NULL);
		self->WasMoving = FALSE;
	    }
	    else if(self->resizing){
		struct rectangle sb;	
		long width  = (self)->GetLogicalWidth() ;
		long height = (self)->GetLogicalHeight() ;

		if(width <= FUDGE || height <= FUDGE) break;
		sb.top = 0; sb.left = width - FUDGE + 1;
		sb.height = height - FUDGE; sb.width = FUDGE -1 ;
		(self)->PostCursor(&sb,self->widthcursor);
		sb.top = height - FUDGE + 1; sb.left =0;
		sb.height = FUDGE - 1 ; sb.width = width - FUDGE;
		(self)->PostCursor(&sb,self->heightcursor);
	    }
	    else if(self->WasResizing){
		(self)->RetractViewCursors(self);
		self->WasResizing = FALSE;
	    }
	    break;
    }
}
void celview::Print(FILE  *file, const char  *processor, const char  *finalFormat, boolean  topLevel)
{
    if(this->truechild) 
	(this->truechild)->Print(file, processor, finalFormat, topLevel);
}
view_DSattributes celview::DesiredSize(long  width, long  height, enum view_DSpass  pass, long  *dWidth, long  *dHeight)
{
    view_DSattributes val;
    long pwidth , pheight ,offset;
    this->sizepending = FALSE;
    if(this->vismode == cel_INVISIBLE) {
	*dWidth = 0;
	*dHeight = 0;
	return(view_Fixed);
    }
    if((Cel(this))->GetReadCount() == 0 &&
	((Cel(this))->GetDefaultStream() != NULL ||
	 (Cel(this))->GetInitFile() != NULL)){
	(Cel(this))->InitDefault();
	(this)->makeview(Cel(this));
    }
/*
    if(self->child && self->mode == celview_FirstUpdate){
	view_LinkTree(self->child,self);
	self->mode = celview_HasView;
    }
*/
    if(this->child && this->desw == UNSET && this->desh == UNSET) {
	if(DOINDENT(this)){
	    val = (this->child)->DesiredSize( width -2 , height -2 , pass, dWidth, dHeight);
	    *dWidth += 2;
	    *dHeight += 2;
	}
	else val = (this->child)->DesiredSize( width , height , pass, dWidth, dHeight);
	return val;
    }
    offset = DOINDENT(this)? 2: 0;
    pheight = (this->desh != UNSET) ? this->desh : height - offset;
    pwidth = (this->desw != UNSET) ? this->desw : width - offset;
    switch(pass){
	case view_HeightSet:
	    pheight = height -offset;
	    if(this->desw != UNSET){
		*dWidth = this->desw;
		return view_Fixed;
	    }
	    break;
	case view_WidthSet:
	    pwidth = width - offset;
	    if(this->desh != UNSET ){
		*dHeight = this->desh;
		return view_Fixed;
	    }
	    break;
	case view_NoSet:	
	    if(this->desh != UNSET && this->desw != UNSET){
		*dHeight = this->desh;
		*dWidth = this->desw;
		return view_Fixed;
	    }
	    if(this->desh != UNSET) pass = view_HeightSet;
	    else if(this->desw != UNSET) pass = view_WidthSet;
    }
    if(this->child )val = (this->child)->DesiredSize( pwidth , pheight , pass, dWidth, dHeight);
    else if( (Data(this))->GetDefaultStream() || (Data(this))->GetInitFile() ){
	val = (view_DSattributes)(view_HeightFlexible | view_WidthFlexible);
	 *dWidth = width;
	 *dHeight = height;
	 return val;
    }
    else{
	val = (view_DSattributes)(view_HeightFlexible | view_WidthFlexible);
/*
	*dHeight =(self->desh != UNSET) ? self->desh :  height - offset; 
	if(*dHeight  > 1024) *dHeight = 200;
	*dWidth =(self->desw != UNSET) ? self->desw : width - offset;
*/
	*dWidth = 100; *dHeight = 100;
    }
    if(this->desh == UNSET) *dHeight += offset;
    else  *dHeight = this->desh;
    if(this->desw == UNSET) *dWidth += offset;
    else *dWidth = this->desw;
    if(*dWidth > width) *dWidth = width;
    if(*dHeight > height ) *dHeight = height;
    return val;
}
void celview::ReceiveInputFocus()
    {
    if(this->truechild) (this->truechild)->WantInputFocus(this->truechild);
    else {
	(this)->PostMenus(NULL);
	this->HasFocus = TRUE;
    }
   (this)->WantUpdate(this);
}

void celview::LoseInputFocus()
    {
    if(this->child) (this->child)->LoseInputFocus();
    this->HasFocus = FALSE;
    (this)->WantUpdate(this);
}


static void UpdateDrawing(class celview  *self)
{
    if(self->OldMode != self->drawing || self->child == NULL){
	if(self->child == NULL){
	    (self)->SetTransferMode(graphic_WHITE);
	    (self)->EraseVisualRect();
	}
	(self)->SetTransferMode(graphic_INVERT);
	(self)->DrawRect(&(self->enclosingRect));
	if(self->child == NULL && self->HasFocus){
	    (self)->MoveTo(0,0);
	    (self)->DrawLineTo(self->enclosingRect.width,self->enclosingRect.height);
	    (self)->MoveTo(0,self->enclosingRect.height);
	    (self)->DrawLineTo(self->enclosingRect.width,0);
	}
	self->OldMode = self->drawing;
    }
}
class view *celview::makeview(class cel  *ls)
{
    /*    if(ls->application == VALUE && ls->valuename == NULL){
	if(ls->dispatcher == NULL) celview_GetDispatcher(self); */

    class view *truechild;
    const char *lv = ls->viewType;
    this->NeedsRemade = FALSE;
    if(this->child == NULL || this->viewatm != (ls)->GetViewAtom() || this->dataatm != (ls)->GetObjectAtom()){
	this->viewatm = (ls)->GetViewAtom();
	this->dataatm = (ls)->GetObjectAtom();
	if(ls->dataObject && ATK::IsTypeByName(ls->dataObject->GetTypeName(), "unknown")) {
	    if(!ATK::IsTypeByName(lv, "unknownv")) {
		lv="unknownv";
	    }
	}
	if(lv != NULL && *lv != '\0' /* && ls->dataObject != NULL */ && (truechild = (class view *) ATK::NewObject(lv)) )  {
	    /*	celview_RetractCursor(self,self->cursorp); */
	    if(ls->application == cel_VALUE && ls->script == NULL) 
		this->promptforparameters = TRUE;
	    if(ls->dataType &&  *ls->dataType && (truechild)->CanView(ls->dataType) == FALSE){
		/* given bad view, so use default */
		(truechild)->Destroy();
		(ls)->SetViewName(NULL,TRUE);
		truechild = (class view *) ATK::NewObject(ls->viewType);
	    }
	    if(truechild){
		class dataobject *dob;
		this->application = ls->application + 1; /* kicks in code below */
		this->refatm = NULL;
		this->mode = celview_NoUpdate;
		if(ls->dataObject == NULL){
		    if(ls->linkname != NULL && (dob = arbiterview::GetNamedObject(this,ls->linkname)) != NULL &&
		       strcmp((dob)->GetTypeName(),ls->dataType) == 0){
			ls->dataObject = dob;
			(dob)->Reference();
		    }
		    else{
			if(ls->linkname){
			    char buf[128];
			    sprintf(buf,"can't link to %s",ls->linkname);
			    (ls)->SetLinkName(NULL);
			}
			ls->dataObject = (class dataobject *)ATK::NewObject(ls->dataType);
		    }
		} 
		if(ls && ls->dataObject != NULL) 
		    (truechild)->SetDataObject(ls->dataObject);
		(truechild)->LinkTree(this);	
		(truechild)->AddObserver(this);
		this->truechild = truechild;
		(this->menus)->SetMask(0);
		this->child = NULL;
		this->mode = celview_FirstUpdate; 
	    }
	    else{
		this->child = this->truechild = NULL;
	    }
	}
	else{
	    this->child = this->truechild = NULL;
	}
	
    }
    if((this->refatm != (ls)->GetRefAtom()) && ls->refname){
	this->refatm =  (ls)->GetRefAtom();
	if(this->truechild  && *ls->refname)
	    (this->truechild)->SetName(atomlist::StringToAtomlist(ls->refname));
    }
    if(this->application != ls->application ){
	this->application = ls->application;
	if(this->truechild){
	    if(ls->application == cel_APPLICATION || ls->application == cel_VALUE || this->AddAppLayer){
		if((this->child = (this->truechild)->GetApplicationLayer()) == NULL)
		    this->child = this->truechild;
		else (this->child)->LinkTree(this);
	    }
	    else{
		this->child = this->truechild;
	    }
	}
    }
    if(this->child){
	if(this->promptforparameters) {
#ifdef DEBUG
	    printf("calling GetParameters\n");
#endif /* DEBUG */
	    GetParameters(this);
	}
	if(ls->application == cel_VALUE) 
	    /*PostParameters(self);*/
	    this->NeedsPost = TRUE;
	if(this->HasFocus)
	    (this->truechild)->WantInputFocus(this->truechild);
    }
    else this->mode = 0;
    return this->child;
}
static void initchild(class celview  *self)
{
    class cel *vr = Cel(self);
    if(vr->viewType == NULL) return;
    (self)->makeview(vr);
}
static char * trunc(char  *c)
{
    char *cp;
    if((cp = strrchr(c,'/')) != NULL && *(++cp) != '\0')
	return cp;
    return c;
}
static void celview_ReadFile(class celview  *self,FILE  *thisFile,char  *iname)
{
    long objectID;
    const char *objectName;
    class cel *ls = Data(self);
    objectName = filetype::Lookup(thisFile, iname, &objectID, NULL); /* For now, ignore attributes. */
    if(objectName == NULL) objectName = "text";
/*    if(class_IsTypeByName("cel",objectName)){ */
    if(strcmp((ls)->GetTypeName(),objectName) == 0){
	(ls)->Read(thisFile,objectID);
	if(self->arb) (self->arb)->InitCell(self);
    }
    else{
	if(objecttest(self,objectName,"dataobject") && (ls)->SetObjectByName(objectName) && ls->dataObject != NULL){
	    const char *nm = NULL;
	    class arbiter *arb = NULL;
	    (ls)->SetViewName(NULL,TRUE);
	    (ls)->SetRefName("");
	    (ls->dataObject)->Read(thisFile,objectID);
	    if(strcmp(objectName,"arbiter") == 0){
		arb = (class arbiter *) (ls)->GetObject();
		if((nm = (arb)->GetRefName()) == NULL || *nm == '\0'){
		    if(iname && *iname) (arb)->SetRefName(trunc(iname));
		    else (arb)->SetRefName(objectName);
		}
		(ls)->SetRefName("arbcel");
	    }
	    else{
		if(iname && *iname) (ls)->SetRefName(trunc(iname));
		else (ls)->SetRefName(objectName);
	    }
	}
    }
    (ls)->NotifyObservers(0);
}
static void celview_Paste(class celview  *self)
{
    FILE *pasteFile;
    if(self->child ) return;
    pasteFile = ((self)->GetIM())->FromCutBuffer();
    celview_ReadFile(self,pasteFile,NULL);
    ((self)->GetIM())->CloseFromCutBuffer( pasteFile);

}
static void celview_PromptForFile(class celview  *self)
{
    char frs[1024];
    FILE *thisFile;
    if(completion::GetFilename(self,"File to insert: ",NULL,frs,1024,FALSE,TRUE) == -1)
	return;
    if ((thisFile = fopen(frs, "r")) == NULL){
            message::DisplayString(self, 0,"can't open file");
	    return;
    }
    celview_ReadFile(self,thisFile,frs);
    fclose(thisFile);
}
static void InitNow(class celview  *self)
{
    if(self->child) return;
    self->mode = INITNOW;
    (Cel(self))->SetRefName("");
    self->NeedsRemade  = self->NeedsReinit = TRUE;
    (self)->WantUpdate(self);
}
void celview::Update()
{
    class cel *vr = Cel(this);
    if(this->mode == celview_NoUpdate) return;
    if(this->mode == celview_DoFull ){
	this->mode = celview_HasView;
	(this)->FullUpdate(view_FullRedraw,0,0,0,0);
/*	view_InsertView(self->child, self, self->olist->rect);
	view_FullUpdate(self->child,view_FullRedraw,0,0,0,0);
	view_WantInputFocus(self->child,self->child);
	UpdateCursors(self);
	UpdateDrawing(self); */
	return;
    }
    if(this->mode == celview_UpdateView){
	this->mode = celview_HasView;
	(this)->FullUpdate(view_FullRedraw,0,0,0,0);
	return;
    }
/*    if(self->mode == INITNOW){ */
    if(this->NeedsRemade  && this->mode == INITNOW  ){
	this->mode = celview_NoUpdate;
	if((this->arb == NULL && this->TopLevel == FALSE )|| this->NeedsReinit){ 
	    arbcon::InitCel(this,this->arb);
	}
#ifdef DEBUG
	printf(" vt = %s (%d)\n",vr->viewType,vr->dataObject);
#endif /* DEBUG */
 	if( (this)->PromptForInfo(this->arb,TRUE,FALSE)== -1){
	    this->NeedsReinit = TRUE;
/*	    if(Parent(self)) view_WantInputFocus(Parent(self),Parent(self)); */
	    return; 
	}
#ifdef DEBUG
	printf(" In UPDATE vr->application === %d (%d)\n",vr->application,VALUE);
#endif /* DEBUG */

	if(vr->application == VALUE) this->promptforparameters = 1;
    }
    if(this->child == NULL || this->NeedsRemade) {
	if((this)->makeview(vr) )  
	    (vr)->NotifyObservers(0);
 	if(this->child != NULL) {
	    (this)->SetTransferMode(graphic_WHITE);
	    (this)->ClearClippingRect();
	    (this)->SetTransferMode(graphic_BLACK);
	    (this)->view::WantNewSize(this); 
	    this->sizepending = TRUE;
	    (this)->FullUpdate(view_FullRedraw,0,0,0,0); /* ??? */
	    this->NeedsRemade = FALSE;
	}
	else {
	    UpdateDrawing(this);
	}
	return;

    }
    if(Cel(this) && (this->desw != Cel(this)->desw || this->desh != Cel(this)->desh)){
	this->desw = Cel(this)->desw ;
	this->desh = Cel(this)->desh ;
	if(!this->sizepending) {
	    (this)->view::WantNewSize(this); 
	    this->sizepending = TRUE;
	}
    }
    if(this->vismode == cel_INVISIBLE) return;
    if(this->child){
	if(this->NeedsPost){
	    if(this->NeedsPost){ 
		::PostParameters(this);
		((class valueview *)this->truechild)->LookupParameters();
	    }
	    (this->child)->FullUpdate(view_FullRedraw,0,0,0,0);
	}
	else (this->child)->Update();
    }
    UpdateCursors(this);
    UpdateDrawing(this);

}
#define OFFSET 5
static void drawshadow(class celview  *self,struct rectangle  *r)
{
    (self)->SetTransferMode(graphic_INVERT);
    (self)->FillRectSize(r->left + OFFSET,r->top + r->height,r->width,OFFSET,(self)->GrayPattern(8,16));
    (self)->FillRectSize(r->left + r->width,r->top + OFFSET,OFFSET,r->height - OFFSET,(self)->GrayPattern(8,16));
    (self)->SetTransferMode(graphic_BLACK);
    (self)->DrawRect(r);
}
void celview::FullUpdate(enum view_UpdateType  type,long  left,long  top,long  width,long  height)
{
    struct rectangle currec,tmprec;
    struct overlay *ov,*lastov;
    currec = this->enclosingRect;
    if(this->mode == celview_NoUpdate) return;
    if((Cel(this))->GetReadCount() == 0 &&
	((Cel(this))->GetDefaultStream() != NULL ||
	 (Cel(this))->GetInitFile() != NULL)){
	(Cel(this))->InitDefault();
    }
    if(this->arb == NULL && !this->TopLevel &&
	(this->arb = arbiterview::FindArb(this->parent)) != NULL){
	(this->arb)->InitCell(this);
    }
    if(type == view_FullRedraw && Cel(this) && (this->desw != Cel(this)->desw || this->desh != Cel(this)->desh)){
	this->desw = Cel(this)->desw ;
	this->desh = Cel(this)->desh ;
	if(!this->sizepending) {
	    (this)->view::WantNewSize(this); 
	    this->sizepending = TRUE;
	}
	return; /* Might as well, since our parent will be redrawing us again */
    }
    this->enclosingRect.top = 0; this->enclosingRect.left = 0;
    this->enclosingRect.width  = (this)->GetLogicalWidth() ;
    this->enclosingRect.height = (this)->GetLogicalHeight() ;
/*    if(DOINDENT(self)){ */
	(this->enclosingRect.width)--;
	(this->enclosingRect.height)--; 
/*    } */
	if(type != view_Remove) {
	    UpdateCursors(this);
	    if(type != view_MoveNoRedraw){
		(this)->SetTransferMode(graphic_WHITE);
		(this)->EraseRect(&(this->enclosingRect));
		(this)->SetTransferMode(graphic_INVERT);

		if(this->drawing){ 
		    (this)->DrawRect(&(this->enclosingRect));
		} 
	    }
	}
 /*   UpdateDrawing(self); */
    this->OldMode = this->drawing;
/*    self->childRect = self->enclosingRect; */
    (this)->GetLogicalBounds(&(this->childRect));
    if(DOINDENT(this)){
	this->childRect.top++; this->childRect.left++;
	this->childRect.width--  ;
	this->childRect.height-- ;
	this->childRect.width--  ;
	this->childRect.height-- ;
	top--; left--;
    }
    if(this->child) {
	class view *child;
	child = (this->olist != NULL) ? this->safec : this->child;
	(child)->InsertView( this, &this->childRect);
	(this)->RetractViewCursors(child);
	if(this->vismode != cel_INVISIBLE && (this->olist == NULL || !(this->olist->flags & celview_COVERCHILD))){
	    if(this->NeedsPost) ::PostParameters(this);
	    if(this->mode == celview_FirstUpdate) 
		this->mode = celview_HasView;
	    (this->child)->FullUpdate(type,left,top,width,height);
	}
    }
    else (this)->WantUpdate(this);
    ov = this->olist;
    lastov = NULL;
    while(ov != NULL){
	for(ov = this->olist;  ov->next != lastov ; ov = ov->next);
	scaleoverlay(this,ov,&currec);
	(ov->view)->InsertView( this, &ov->rect);
	(this)->RetractViewCursors(ov->view);
	(ov->view)->FullUpdate(view_FullRedraw,0,0,0,0);
	(this)->SetClippingRect(&(this->enclosingRect));
	tmprec = ov->rect;
	if(tmprec.top < 0) tmprec.top--;
	if(tmprec.left < 0) tmprec.left--;
	tmprec.height++; tmprec.width++;
	tmprec.height++; tmprec.width++;
	drawshadow(this,&tmprec);
	if(ov == this->olist) break;
	lastov = ov;
    }
    if(ov && ov->view){
	(ov->view)->WantInputFocus(ov->view);
    }
#ifdef DEBUG
    fprintf(stdout,"Out of FullUpdate, NP = %s self->script = %d %d\n",this->NeedsPost?"TRUE":"FALSE",this->script,(this->script)?(this->script)->GetLength():0);fflush(stdout);
#endif
}


void celview::SetDataObject(class dataobject  *dd)
{
    this->desw = ((class cel *)dd)->desw;
    this->desh = ((class cel *)dd)->desh;
    this->vismode = ((class cel *)dd)->mode;
    (this)->view::SetDataObject(dd);
    this->NeedsRemade = TRUE;
    if(((class cel *)dd)->application == cel_VALUE) this->NeedsPost = TRUE;
}
celview::celview()
{
	ATKinit;

    this->widthcursor = cursor::Create(this);
    this->heightcursor = cursor::Create(this);
    (this->heightcursor)->SetStandard(Cursor_HorizontalBars);
    (this->widthcursor)->SetStandard(Cursor_VerticalBars);
    this->Moving = 0; 
    this->WasMoving = 0;
    this->resizing = RESIZING;  
    this->WasResizing = 0;
    this->child = this->truechild = NULL;
    this->desw = this->desh = UNSET;
    this->menus = (celviewMenus)->DuplicateML( this);
    this->drawing = this->OldMode = DRAWING;
    this->sizepending = TRUE;
    this->arb = NULL;
    this->promptforparameters = 0;
    this->NeedsPost = FALSE;
    this->vismode = cel_VISIBLE;
    this->NeedsRemade = FALSE;
    this->viewatm = this->dataatm = this->refatm = NULL;
    this->application = -100;
    this->NeedsReinit = FALSE;
    this->hitfunc = NULL;
    this->hitrock = 0;
    this->menulistp = NULL;
    this->keymapp = NULL;
    this->keystatep = NULL;
    this->HasFocus = FALSE;
    this->TopLevel = FALSE;
    this->olist = NULL;
    this->rarb = NULL;
    this->AddAppLayer = FALSE;
    this->script = NULL;
    this->level = 0;
    (this->menus)->SetMask(1);
    THROWONFAILURE( TRUE);
}
void celview::SetResizing(long  key)
{
    if(this->resizing == 0){
	this->resizing = 1;
    }
    else {
	this->resizing = 0;
	this->WasResizing = 1;
    }
    if(this->mode == celview_HasView)
	this->mode = celview_DoFull;
    (this)->WantUpdate(this);
}
void celview::PostMenus(class menulist  *menulistp)
{
    (this->menus)->UnchainML(ChildMenus);
    if(menulistp != NULL && menulistp != this->menus)
	(this->menus)->ChainAfterML(menulistp,ChildMenus);
    (this)->view::PostMenus( this->menus);
}
void celview::PostKeyState(class keystate  *keystatep)
{
    class keystate *cur = keystatep;
    if(this->keystatep != NULL && this->keystatep->orgMap != this->keymapp){
	/* (this->keystatep)->Reset(); */
	delete this->keystatep;
	this->keystatep = NULL;
    }
    if(this->keymapp != NULL && this->keystatep == NULL)
	this->keystatep = keystate::Create(this,this->keymapp);

    if(this->keystatep){
	/* this->keystate->next = NULL;
	 (this->keystatep)->Reset();*/
	cur = (this->keystatep)->AddBefore( cur);
    }
    (this)->view::PostKeyState( cur);
}
void celview::WantNewSize(class view  *requestor)
    {
    if(Cel(this)){
	if(this->vismode == cel_INVISIBLE && requestor != (class view *)this) return;
	if((Cel(this)->desw != UNSET ||  Cel(this)->desh != UNSET)){
	    Cel(this)->desw = Cel(this)->desh = UNSET;
	    this->desw = this->desh = UNSET;
	    (Cel(this))->NotifyObservers(0);
	}
    }
    if(!this->sizepending) {
	(this)->view::WantNewSize(this);
	this->sizepending = TRUE;
    }
}

static struct bind_Description celviewBindings[]={
    {"celview-Init-Child",NULL,0,"celview,Init Child",0,1,(proctable_fptr)InitNow,"Initialize Child" },
    {"celview-Paste",NULL,0,"celview,Paste",0,1,(proctable_fptr)celview_Paste,"Paste Child" },
    {"celview-InsertFile",NULL,0,"celview,Insert File",0,1,(proctable_fptr)celview_PromptForFile,"Read Child" },
NULL
};
static void SetVisible(class celview  *self)
{
    if(Cel(self))
	(Cel(self))->SetVisible();
}
static void SetInvisible(class celview  *self)
{
    if(Cel(self))
	(Cel(self))->SetInvisible();
}

boolean celview::InitializeClass()
    {
    celviewMenus = new menulist;
    bind::BindList(celviewBindings, NULL , celviewMenus, &celview_ATKregistry_ );
    proctable::DefineProc("celview-set-visible", (proctable_fptr)SetVisible,&celview_ATKregistry_ ,NULL, "Make celview visible");
    proctable::DefineProc("celview-set-invisible", (proctable_fptr)SetInvisible,&celview_ATKregistry_ ,NULL, "Make celview invisible");
   UNSETFLAG = atom::Intern("XXXUNDEFINEDXXX");
    return TRUE;
}

static boolean objecttest(class celview   *self,const char  *name,const char  *desiredname)
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

static int lookuptype(char  *ty)
{
    const struct types *tp;
    for(tp = typearray;tp->val != 0; tp++)
	if(*ty == *tp->str && strcmp(ty,tp->str) == 0)
	    return tp->val;
    return 0;
}

static char * atomlisttostring(class atomlist  *al)
/* should be an atomlist call */
{
    static char buf[512];
    char *cp,*alcp;
    struct atoms *at;
    cp = buf;
    for(at = (al)->TraversalStart(); at != NULL; at = (al)->TraversalNext(at)){
#ifdef DEBUG
printf("--> %s\n",at->atom->name);
#endif /* DEBUG */
	for(alcp = at->atom->name; *alcp; alcp++)
	    *cp++ = *alcp;
	*cp++ = '.';
    }
    if(cp > buf) cp--;
    *cp = '\0';
#ifdef DEBUG
printf("returning> %s\n",buf);
#endif /* DEBUG */
    return buf;
}   
int celview::ResourceListToString(char  *str,struct resourceList  *rl)
{
	ATKinit;

#ifdef DEBUG
printf("In RLtoS found = %d %s\n",rl->found,rl->type->name);
#endif /* DEBUG */
    if(!rl->found){
	char *res;
	res = atomlisttostring(rl->name);
#ifdef DEBUG
printf("In RLtoS Not found = %d %s %s\n",rl->found,rl->type->name,res);
#endif /* DEBUG */

	sprintf(str,"[%s] <%s> ()",rl->type->name,res);
#ifdef DEBUG
printf("out of sprintf\n");
#endif /* DEBUG */
    }
    else switch(lookuptype(rl->type->name)){
	case STRING:
	    sprintf(str,"[%s] <%s> (%s)",rl->type->name,atomlisttostring(rl->name),
		    (char *) rl->data);
	    break;
	case FLOAT:
	    sprintf(str,"[%s] <%s> (%f)",rl->type->name,atomlisttostring(rl->name),
		    (float) rl->data);
	    break;
	case LONG:
	    sprintf(str,"[%s] <%s> (%ld)",rl->type->name,atomlisttostring(rl->name),
		    rl->data);
	    break;
	default:   
	    sprintf(str,"Unknown Type %s", rl->type->name);
	    return FALSE;
    }
#ifdef DEBUG
printf("leaving RLtoS\n");
#endif /* DEBUG */
return TRUE;
}
void celview::GetManyParameters( struct resourceList  * resources, class atomlist  * name, class atomlist  * class_c )
{
    int x;
    (this)->view::GetManyParameters( resources, name, class_c ); 
    for (x = 0; resources[x].name != NULL; ++x){
	if(this->NeedsPost || resources[x].data == (long) UNSETFLAG){
	    resources[x].found = FALSE;
	}
    }
#ifdef DEBUG
	fprintf(stdout,"get-many mis match %d %d (%d)\n",this->level,x,this->script);fflush(stdout);
#endif

    if(this->script != NULL && this->level <  x ){
/*	fprintf(stdout,"get-many mis match %d %d \n",self->level,x);fflush(stdout);*/
	/*  # of RESOURCES requested != # of resources stored in cel */
	appendresourceList( this, resources);
    }
}
static void appendresourceList( class celview  * self, struct resourceList  * resources)
{   /* append new entries onto the cels text */
    char *buf,tbuf[1024],*cp,*el;
    struct resourceList rl;
    long len,x;
    boolean fnd[64];
    for(x = 0;x < 64;  x++) fnd[x] = FALSE;
    if(self->script != NULL ){
	len = (self->script)->GetLength();
	if(len > 0){
	    buf = (char *)malloc(len + 1);
	    (self->script)->CopySubString(0,len,buf,FALSE);
	    for(cp = buf; cp < (buf + len ); cp = el + 1){
		if((el = strchr(cp,'\n')) == NULL){
		    break;
		}
		*el = '\0';
		if(el == cp){
		    continue;
		}
		if((StringToResourceList(&rl,cp)) == TRUE){
		    strcpy(tbuf,atomlisttostring(rl.name));
		    for (x = 0; resources[x].name != NULL; ++x){
			/* not a very efficient test, but this code is rarely called */
			if(strcmp(tbuf,atomlisttostring(resources[x].name)) == 0){
			   /* fprintf(stdout,"found %s\n", tbuf);fflush(stdout);*/
			    fnd[x] = TRUE;
			    break;
			}
		    }
		}
	    }
	    for (x = 0; resources[x].name != NULL; ++x){
		if(fnd[x] == FALSE){
/*		    fprintf(stdout,"adding  %s\n", atomlisttostring(resources[x].name));fflush(stdout);*/
		    celview::ResourceListToString(tbuf,&resources[x]);
		    cp = strrchr(tbuf,'(');
		    sprintf(cp,"()\n");
		    self->level++;
		    (self->script)->InsertCharacters(0,tbuf,strlen(tbuf));
		}
	    }
	    free(buf);
	}
    }	

}
static void editresourceList( class celview  * self, struct resourceList  * resources,int  askres,int  maxcount )
{
    struct resourceList *rl;
    char buf[1024],iname[512],*cp;
    int pf;
/*fprintf(stdout,"In edit res %d\n",maxcount);fflush(stdout);*/
    for(rl = resources; (rl->name != NULL) &&( maxcount-- >= 0); rl++){
	celview::ResourceListToString(buf,rl);
#ifdef DEBUG
	printf("RLTS returned %s\n",buf);
#endif /* DEBUG */
	if(askres){
	    pf = message::AskForString(NULL, 0, buf, 0, iname, sizeof(iname));
	    if (pf < 0 ) continue;
	    if(strlen(iname ) > 0 ){
		cp = strrchr(buf,'(');
		sprintf(cp,"(%s)\n",iname);
	    }
	    else strcat(buf,"\n");
	}
	else {
	    cp = strrchr(buf,'(');
	    sprintf(cp,"()\n");
/*	    strcat(buf,"\n"); */
	}
#ifdef DEBUG
	printf(">> inserting %s",buf);
#endif /* DEBUG */
	(self->script)->InsertCharacters(0,buf,strlen(buf));
    }
}

#define SKIPTO(SRC,C,PT) for(PT = SRC; *PT != C ; PT++) if(*PT == '\0') return FALSE
#define SKIPTOSET(SRC,C,PT,BB) for(PT = SRC; *PT != C ; *BB++ = *PT++) if(*PT == '\0') return FALSE
#define PULLOUT(S1,S2,C1,C2,BUF)     tmp = BUF; *tmp = '\0';\
    SKIPTO(S2,C1,S1); \
    S1++;\
    SKIPTOSET(S1,C2,S2,tmp);\
    S2++ ; *tmp = '\0'

static boolean StringToResourceList(struct resourceList  *rl,char  *str)
{
    char buf[512], *tmp;
    char *start,*end;
    end = str;
    PULLOUT(start,end,'[',']',buf);
    rl->type = atom::Intern(buf);
    PULLOUT(start,end,'<','>',buf);
    rl->name = atomlist::StringToAtomlist(buf);
    PULLOUT(start,end,'(',')',buf);

    if(*buf == '\0') {
	rl->data = (long) UNSETFLAG;
	return TRUE;
    }
    
    switch(lookuptype((rl->type)->Name())){
	case LONG:
	    for(tmp = buf; !isdigit(*tmp); tmp++)
		if(*tmp == '\0' || *tmp == '-')break;
	    if(*tmp == '\0') rl->data =  (long) UNSETFLAG;
	    else rl->data = atol(tmp);
	    break;
	case FLOAT:
	    for(tmp = buf; !isdigit(*tmp); tmp++)
		if(*tmp == '\0'|| *tmp == '-' || *tmp == '.')break;
	    if(*tmp == '\0') rl->data =  (long) UNSETFLAG;
	    else rl->data = (long) atof(tmp);
	    break;
	case STRING:
	    rl->data = (long) (atom::Intern(buf))->Name();
	    break;
	default:
	    return FALSE;
    }
    return TRUE;
}
static void GetParameters(class celview  *self)
{
    class valueview *wv = (class valueview *)self->truechild;
    struct resourceList *resources;
    class cel *ls = Data(self);
    if(ls->script != NULL){
	self->script = ls->script;
	(self->script)->Clear();
    }
    else self->script = new text;
    self->promptforparameters = 2;
#ifdef DEBUG
printf("entering lookup\n");
#endif /* DEBUG */
    resources = (wv)->GetDesiredParameters();
if(self->arb){
    editresourceList( self, resources,FALSE,100 );
}
    (wv)->LookupParameters(); /* This should call GetManyParameters above */
#ifdef DEBUG
printf("exiting lookup\n");
#endif /* DEBUG */
    self->promptforparameters = 0;
    if((self->script)->GetLength() == 0){
	if(self->script != ls->script) (self->script)->Destroy();
	self->script = NULL;
#ifdef DEBUG
printf("script is EMPTY\n");
#endif /* DEBUG */
	return;
    }
    ls->script = self->script;
    if(self->arb){
	arbcon::SetCurrentCelview(self);
	arbcon::EditCurrentCelview();
    }
#ifdef DEBUG
printf("setting ls->script %d\n",(ls->script)->GetLength());
#endif /* DEBUG */

}
void celview::PostParameters()
{
this->NeedsPost = TRUE;
(this)->WantUpdate(this);
}
/* #define DEBUG  */
static void PostParameters(class celview  *self)
{
    int len;
    struct resourceList rl;
    char *buf,*cp,*el;
    class cel *ls = Data(self);
    if((self)->WantHandler("message") == NULL){
	/* tree node linked yet */
	return;
    }
#ifdef DEBUG
printf("In Postparametes %d\n",(ls->script == NULL)? -1:(ls->script)->GetLength());
#endif /* DEBUG */
    if(ls->script != NULL ){
	self->level = 0;
	len = (ls->script)->GetLength();
	if(len > 0){
	    buf = (char *)malloc(len + 1);
	    (ls->script)->CopySubString(0,len,buf,FALSE);
	    for(cp = buf; cp < (buf + len ); cp = el + 1){
		if((el = strchr(cp,'\n')) == NULL){
		    break;
		}
		*el = '\0';
		if(el == cp){
		    continue;
		}
		self->level++;
#ifdef DEBUG
		printf("Calling STRL w/ %s\n",cp);
#endif /* DEBUG */
		if((StringToResourceList(&rl,cp)) == TRUE){
		    (self->truechild)->PostResource(rl.name,rl.type,rl.data);
#ifdef SHOULDNOTDEF
fprintf(stdout,"Posting something \n"); fflush(stdout);
		    (self)->PostResource(rl.name,rl.type,rl.data); 
#endif
		}
	    }
	    free(buf);
	    self->script = ls->script;
	}
    }	
    self->NeedsPost = 0;
}
int celview::PromptForInfo(class arbiterview  *arb,boolean  promptForViewName,boolean  changeRefName)
{

    char iname[100],qz[64];
    const char *prompt;
    char viewname[200],refname[256];
    long pf;	
    class cel *ls = Cel(this);
    if(ls == NULL){
	printf("Cel Has No Dataobject\n");
	return -1;
    }
 /*   im_ForceUpdate(); */
    viewname[0] = '\0'; iname[0] = '\0';
    if(ls->dataObject == NULL){
	pf = message::AskForString(NULL, 0, "Data object to insert here: ", 0, iname, sizeof(iname));
	if (pf < 0) {
	    return -1 ;
	}
	if(strlen(iname)==0)  {
	    promptForViewName = TRUE;
	} 
	else if (strcmp(iname,"value") == 0){
	    promptForViewName =	TRUE;	
	}
	else if(objecttest(this,iname,"dataobject") == FALSE) return -1;
	(ls)->SetObjectByName(iname);
    }
    prompt = "View to place here (return for default)? ";
    while((ls->viewType == NULL || *(ls->viewType) == '\0') &&  promptForViewName){
	if( message::AskForString (NULL, 0, prompt, 0, viewname, 200) < 0)
	    {
	     return -1;
	}
	if(strlen(viewname) == 0)
	    (ls)->SetViewName(NULL,TRUE);
	else if(objecttest(this,viewname,"view")) 
	    (ls)->SetViewName(viewname,FALSE);
	prompt = "Invalid View! View to place here (return for default)? ";
    }
    *refname = '\0';
    if((((ls->refname == NULL || *(ls->refname) == '\0') && this->TopLevel == FALSE)|| changeRefName)) {
	class atomlist *atm;
#ifdef DEBUG
printf("calling for name , arb = %d\n",arb);
#endif /* DEBUG */
	if( message::AskForString (NULL, 0, "Name for reference ", 0, refname, 200) < 0){
	    return -1;
	}
#ifdef DEBUG
printf("name is %s\n",refname);
#endif /* DEBUG */
	if(arb){
	    while(((arb)->registername(this,refname)) == FALSE){
		*qz = 'y';
		if( message::AskForString (NULL, 0, "That name is taken.Do you wish another view on that object?[y] ", 0,qz , 64) < 0){
		    return -1;
		}
		if(*qz == 'n' ||  *qz == 'N') {
		    if( message::AskForString (NULL, 0, "New name for reference ", 0, refname, 200) < 0){
			  return -1;
		    }
		}
		else break;
	    }
	}
	else (ls)->SetRefName(refname);
	atm = atomlist::StringToAtomlist(refname);
	(this)->SetName(atm); 
    }
    if(strcmp(ls->dataType,"value") == 0) 
	(ls)->SetApplication(cel_VALUE);
    else if(ls->application == cel_NOTSET){
	*refname = '\0';
	if( message::AskForString (NULL, 0, "Add Application Layer (no)", 0, refname, 200) < 0){
	    return -1;
	}
	if(*refname == 'y' || *refname == 'Y') (ls)->SetApplication(cel_APPLICATION);
    }
    return 1;
}
void celview::LinkTree(class view  *parent)
{
    class arbiterview *ab;
    int named = FALSE;
    (this)->view::LinkTree( parent);
    if(this->child){
	(this->child)->LinkTree(this);
    }
    // Note: this->arb may be an arbiterview being deleted, currently
    // executing ~celview, which means that it is no longer an
    // arbiterview.  Thus a dummy DeleteCell() is needed in cellview
    // to avoid crash here or below (the actual functionality of DeleteCall()
    // has already been executed in ~arbiterview as needed)
    if(parent == NULL){
	if(this->arb != NULL) {
	    (this->arb)->DeleteCell(this);
	    this->arb = NULL;
	}
	return;
    }
    if(this->TopLevel || strcmp((parent)->GetTypeName(),"frame") == 0){
	ab = NULL;
	this->TopLevel = TRUE;
	if(Cel(this)) {
	    (Cel(this))->SetApplication(cel_APPLICATION);
	    (Cel(this))->SetRefName("");
	    (Cel(this))->SetViewName(NULL,TRUE);
	}
    }
    else{
	if(this->rarb) ab = this->rarb;
	else ab =arbiterview::FindArb(parent);
    }

    if(Cel(this)  != NULL && Cel(this)->refname != NULL) named = TRUE;
    if(this->arb == NULL){
	if((this->arb = ab) != NULL)
	    (this->arb)->InitCell(this);
    }
    else {
	if(this->arb == ab) /* arbiterview_ReInitCell(self->arb,self) Necessary ? */ ;
	else {
	    (this->arb)->DeleteCell(this);
	    this->arb = ab;
	    if(this->arb) (this->arb)->InitCell(this);
	}
    }
    if(this->child == NULL && Cel(this) != NULL ){
	if( named ){
	    initchild(this); 
	}
    }
}
void celview::Copy ()
    {
    FILE *cutFile;
    cutFile = ((this)->GetIM())->ToCutBuffer();
    (Cel(this))->Write(cutFile,im::GetWriteID(),0);

    ((this)->GetIM())->CloseToCutBuffer( cutFile);
}


celview::~celview()
{
	ATKinit;

    if(this->olist){
	struct overlay *ov; 
	class view *vw[128];
	int i;
	for(i = 0,ov = this->olist;  ov != NULL ; ov = ov->next){
	    vw[i++] = ov->view;
	    if(i > 128) break;
	}
	while(i--){
	    (vw[i])->RemoveObserver(this);
	    ::PopOverlay(this,vw[i]);
	}
    }
    if(this->truechild){
	class view *child;
	child = this->truechild;
	(child)->UnlinkTree();
	this->child = NULL;
	(child)->RemoveObserver(this);
	(child)->Destroy();
    }
    if(this->menus) delete this->menus;
}
void celview::InitChildren()
{
    if(this->child == NULL || this->NeedsRemade) {
	(this)->makeview(Cel(this));
       }
    if(this->child) 
	(this->child)->InitChildren();
	
}
boolean celview::CanView(const char  *TypeName)
{
    return ATK::IsTypeByName(TypeName,"cel");
}
void celview::SetHitfunc(celview_hitfptr hitfunc,long  hitrock)
{
    this->hitfunc = hitfunc;
    this->hitrock = hitrock;
}
void celview::SetKeymap(class keymap  *km)
{
    this->keymapp = km;
 }

void celview::SetMenulist(class menulist  *ml)
{
    (this->menus)->UnchainML(ClientMenus);
    if(ml != NULL){
	(this->menus)->ChainBeforeML(ml,ClientMenus);
	(ml)->SetView(this);
    }
}

void celview::Repost()
{
    /* question, how to force child to repost its menus and keystate 
       so that we can add ours?
        answer: If the child does not have the input focus, don't worry
      about it since the posting will happen when it gets it.
      If the child does have the input focus, we request it and then 
      (in celview_ReceiveInputFocus) we request it back for the child,
	  thus getting the post requests we want.
	  */
    class im *im = (this)->GetIM();
    class view *foc;
    if(this->child && im){
	foc = (im)->GetInputFocus();
	if(this->truechild == foc || this->child == foc )
	    (this)->WantInputFocus(this);
    }
}
void celview::PushOverlay(class view  *view,struct rectangle  *rect,long  flags)
{
    struct overlay *ov;
    ov = (struct overlay *) malloc(sizeof(struct overlay));
    ov->view = view;
    if(rect) ov->rect = *rect;
    ov->flags = flags;
    ov->next = this->olist;
    if(this->olist == NULL){
	this->safec = this->child ;
	this->safetc = this->truechild ;
    }
    this->child = this->truechild = view;
    this->olist = ov;
    this->mode = celview_DoFull;
    (view)->AddObserver(this); 
    (this)->WantUpdate(this);
}
class view *celview::PopOverlay(class view  *view)
{
if((view = ::PopOverlay(this,view)) != NULL) (view)->RemoveObserver(this);
return view;
}
void celview::WantUpdate(class view  *requestor)
{
    class view *view;
    if(this->olist == NULL || requestor == (class view *)this){
	(this)->view::WantUpdate(requestor);
	return;
    }
    /* don't process updates for children that don't belong to the current overlay */
    for(view = requestor; (view != (class view *) this) && view != NULL; view = view->parent)
	if(view == this->child)
	    (this)->view::WantUpdate(requestor);

}
void celview::PostCursor(struct rectangle  *rec,class cursor  *c)
{
    class view *view = c->view;
    if(this->olist == NULL || view == (class view *)this){
	(this)->view::PostCursor(rec,c);
	return;
    }
    /* don't process posts for children that don't belong to the current overlay */
    for(; (view != (class view *) this) && view != NULL; view = view->parent)
	if(view == this->child)
	    (this)->view::PostCursor(rec,c);

}
void celview::ObservedChanged(class observable  *changed, long  value)
{
    if(changed == (class observable *)Cel(this)) 
    {
	class cel *c = Cel(this);
	class view *parent = ((class view *) this)->parent;
	if (value == observable_OBJECTDESTROYED){
/* NO LONGER assumes the parent is also observing the cel and will destroy the celview */
	    if(parent == NULL) this->truechild = NULL;
	    else (this)->UnlinkTree();
	    (this)->view::ObservedChanged(changed,value); /* sets dataobject to NULL, so remove observer 
							won't be called when the view is destroyed */
	    (this)->Destroy();
	    return;
	}
	else if(parent == NULL)
	    return;
	else if(value == cel_NeedsRepost){
	    this->NeedsPost = TRUE;
	    (this)->WantUpdate(this);
	}
	else if(this->viewatm != (c)->GetViewAtom() || this->dataatm != (c)->GetObjectAtom() || this->refatm != (c)->GetRefAtom() || this->application != (c)->GetApplication()){
	    this->NeedsRemade = TRUE;
	    (this)->WantUpdate(this);
	}
	else if(this->vismode != Cel(this)->mode) {
	    this->vismode = Cel(this)->mode;
	    (this)->WantNewSize(this);
	}
	else if (Cel(this) && (this->desw != Cel(this)->desw || this->desh != Cel(this)->desh))
	    (this)->WantUpdate(this);

    }
    if(value ==  observable_OBJECTDESTROYED ){
	if(this->olist){
	    if(changed == (class observable *) ::PopOverlay(this,(class view *)changed))
		  return;
	}
	else{
	    if(changed == (class observable *)this->child ||
	       changed == (class observable *)this->truechild ||
	       changed == (class observable *)this->safec ||
	       changed == (class observable *)this->safetc
	       ){
		this->child = NULL;
		this->truechild = NULL;
		(this)->UnlinkTree();
		(this)->Destroy(); 
	    }
	}
    }
}

class view *celview::Hit(enum view_MouseAction  action,long  mousex ,long  mousey ,long  numberOfClicks) 
{
    class view *vw;
    static boolean lasthit;
    lasthit = TRUE;
    if(Cel(this) == NULL )
	return (class view *)this;
    if(this->olist && ((this->child)->EnclosedXToLocalX( mousex) < 0 || 
	(this->child)->EnclosedYToLocalY( mousey) < 0)){
	if((vw = ::PopOverlay(this,NULL)) != NULL){
	    (vw)->RemoveObserver(this);
	    (vw)->Destroy();
	}
	return (class view *) this;
    }

    if( this->vismode == cel_INVISIBLE){
	if ((this->arb)->GetCelMode())
	    arbcon::SetCurrentCelview(this);
	return (class view *)this;
    }
    if((this->child && !this->resizing) )	{
	vw = NULL;
	if(this->hitfunc){
	    vw = (*(this->hitfunc))
	      (this,action,
	       (this->child)->EnclosedXToLocalX( mousex),
	       (this->child)->EnclosedYToLocalY( mousey),
	       numberOfClicks,this->hitrock);
	}
	if(vw == NULL)
	    vw = (this->child)->Hit(action,(this->child)->EnclosedXToLocalX( mousex), (this->child)->EnclosedYToLocalY( mousey),numberOfClicks);
	if(this->arb && lasthit ){
	    lasthit = FALSE;
	    if((this->arb)->GetCopyMode()){ 
		char buf[256];
		arbcon::SetCurrentCelview(this);
		(this)->Copy();
		sprintf(buf,"copying %s",(Cel(this))->GetRefName());
		message::DisplayString(this,0,buf);
	    }
	    else if ((this->arb)->GetCelMode())
		arbcon::SetCurrentCelview(this);
	}
	return vw;
    }
    if(this->child == NULL) {
	(this)->WantInputFocus(this);
    }
    switch(action){
	case view_LeftDown:
	case view_RightDown:
	    {
	    long width = (this)->GetLogicalRight();
	    long height = (this)->GetLogicalBottom();
	    if(width - FUDGE < mousex) this->Moving = celview_ChangeWidth;
	    else if(height - FUDGE < mousey) this->Moving = celview_ChangeHeight;
#if 0
	    else if(this->child) {
		vw = (this->child)->Hit(action,(this->child)->EnclosedXToLocalX( mousex), (this->child)->EnclosedYToLocalY( mousey),numberOfClicks);
		if(this->arb && (vw == this->truechild || vw == this->child ))
		    arbcon::SetCurrentCelview(this);
		return vw;
	    }
#endif /* 0 */
	    else break;
	    UpdateCursors(this);
	    return (class view *) this;
	    }
	case view_RightUp:
	case view_LeftUp:
	    if(this->Moving){
		int move = this->Moving;
		this->Moving = 0;
		this->WasMoving = 1;
		UpdateCursors(this);
		if(move == celview_ChangeWidth && mousex > 0) {
			Cel(this)->desw = mousex + 1;
			(Cel(this))->NotifyObservers(0);
		}
		else if(move == celview_ChangeHeight && mousey > 0) {
			Cel(this)->desh = mousey + 1;
			(Cel(this))->NotifyObservers(0);
		}
		return (class view *) this;
	    }
	default:
	    break;
    }
    if(this->Moving || this->child == NULL) return (class view *) this;
    return NULL;
}

ATK  * celview::WantHandler(const char  *handlerName)
{
    if(strcmp(handlerName,"arbiterview") == 0 && this->arb) return (ATK  *)this->arb;
    return (this)->view::WantHandler( handlerName);
}

class celview *celview::GetCelviewFromView(class view  *v)
{
	ATKinit;

    if(v == NULL) return NULL;
    for( v = v->parent;v != NULL; v = v->parent){
	if(ATK::IsTypeByName((v)->GetTypeName(),"celview"))
	    return((class celview *) v);
    }
    return NULL;
}

void celview::GetOrigin(long  width, long  height, long  *originX, long  *originY)
                    {
    if(this->child) (this->child)->GetOrigin( width, height, originX, originY);
    else (this)->view::GetOrigin( width, height, originX, originY);
}

boolean celview::RecSearch(struct SearchPattern *pat, boolean toplevel)
{
    boolean res;

    this->InitChildren();
    if (this->child) {
	res = this->child->RecSearch(pat, toplevel);
	return res;
    }

    return FALSE;
}

boolean celview::RecSrchResume(struct SearchPattern *pat)
{
    boolean res;

    this->InitChildren();
    if (this->child) {
	res = this->child->RecSrchResume(pat);
	return res;
    }
    return FALSE;
}

boolean celview::RecSrchReplace(class dataobject *text, long pos, long len)
{
    boolean res;

    this->InitChildren();
    if (this->child) {
	res = this->child->RecSrchReplace(text, pos, len);
	return res;
    }
    return FALSE;
}

void celview::RecSrchExpose(const struct rectangle &logical, struct rectangle &hit)
{
    if (this->child) {
	struct rectangle crect=logical;
	if(DOINDENT(this)){
	    crect.top++; crect.left++;
	    crect.width--  ;
	    crect.height-- ;
	    crect.width--  ;
	    crect.height-- ;
	}
	child->LinkTree(this);
	struct rectangle oldrect;
	child->GetEnclosedBounds(&oldrect);
	child->InsertView(this, &oldrect);
	this->child->RecSrchExpose(crect, hit);
    }
}

