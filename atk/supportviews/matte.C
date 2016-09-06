 /* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/config/COPYRITE'     *
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

#define UNSET 0
#define FUDGE 2
#define BIGFUDGE 5	/* for bigger insets */
#define BIGSIZE 20		/* width/height of "big" inset */

#define matte_ChangeWidth 1
#define matte_ChangeHeight 2


#include <andrewos.h>
ATK_IMPL("matte.H")
#include <bind.H>
#include <view.H>
#include <viewref.H>
#include <menulist.H>
#include <cursor.H>
#include <graphic.H>
#include <im.H>
#include <keymap.H>
#include <dataobject.H>
#include <environment.H>
#include <style.H>
#include <environ.H>

#include <matte.H>

#define RESIZING TRUE
#define DRAWING FALSE

class menulist *matteMenus;
static class keymap *matteKeyMap;


ATKdefineRegistry(matte, view, matte::InitializeClass);

static void UpdateCursors(class matte  *self);
static void UpdateDrawing(class matte  *self);
static char *matte_PromptForViewName(class matte  *self);


static void UpdateCursors(class matte  *self)
{
    int fudge;
    switch(self->Moving){
	case matte_ChangeWidth:
	    im::SetProcessCursor(self->widthcursor);
	    break;
	case matte_ChangeHeight:
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

		if(width <= FUDGE || height <= FUDGE)
		    break;
		else if (width <= BIGSIZE || height <= BIGSIZE)
		    fudge = FUDGE;	/* normal fudge */
		else
		    fudge = BIGFUDGE;	/* for bigger insets (figures, tables, etc) */
		sb.top = 0; sb.left = width - fudge + 1;
		sb.height = height - fudge; sb.width = fudge -1 ;
		self->PostCursor(&sb,self->widthcursor);
		sb.top = height - fudge + 1; sb.left =0;
		sb.height = fudge - 1 ; sb.width = width - fudge;
		(self)->PostCursor(&sb,self->heightcursor);
	    }
	    else if(self->WasResizing){
		(self)->RetractViewCursors(self);
		self->WasResizing = FALSE;
	    }
	    break;
    }
}

void matte::Print(FILE  *file, const char  *processor, const char  *finalFormat, boolean  topLevel)
{
    if(this->child) 
	(this->child)->Print(file, processor, finalFormat, topLevel);
}

view_DSattributes matte::DesiredSize(long  width, long  height, enum view_DSpass  pass, long  *dWidth, long  *dHeight)
{
    view_DSattributes val;
    long pwidth , pheight ;
    int fudge;
    if (width <= BIGSIZE || height <= BIGSIZE)
	fudge = FUDGE;	/* normal fudge */
    else
	fudge = BIGFUDGE;	/* for bigger insets (figures, tables, etc) */
    this->sizepending = FALSE;
    if(this->child && this->desw == UNSET && this->desh == UNSET) {
	val = (this->child)->DesiredSize( width -fudge , height -fudge , pass, dWidth, dHeight);
	*dWidth += fudge;
	*dHeight += fudge;
	return val;
    }
    pheight = (this->desh != UNSET) ? this->desh : height - fudge;
    pwidth = (this->desw != UNSET) ? this->desw : width - fudge;
    switch(pass){
	case view_HeightSet:
	    pheight = height -fudge;
	    if(this->desw != UNSET){
		*dWidth = this->desw;
		return view_Fixed;
	    }
	    break;
	case view_WidthSet:
	    pwidth = width - fudge;
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
    else{
	val = (view_DSattributes)(view_HeightFlexible | view_WidthFlexible);
	*dHeight = height - fudge; 
	*dWidth = width - fudge;
    }
    if(this->desh == UNSET) *dHeight += fudge;
    else  *dHeight = this->desh;
    if(this->desw == UNSET) *dWidth += fudge;
    else *dWidth = this->desw;
    if(*dWidth > width) *dWidth = width;
    if(*dHeight > height ) *dHeight = height;
    return val;
}

void matte::GetOrigin(long  width, long  height, long  *originX, long  *originY)
                    {
    if (this->child != NULL) {
        if (width <= BIGSIZE || height <= BIGSIZE) {
            width-=FUDGE ;
            height-=FUDGE ;
        } else {
            width -= BIGFUDGE  ;
            height -= BIGFUDGE ;
        }
        (this->child)->GetOrigin( width, height, originX, originY);
        *originY+=1;
        *originX+=1;
    } else
	(this)->view::GetOrigin( width, height, originX, originY);
    
}

void matte::GetPrintOrigin(long width, long height, long *originX, long *originY) {
    if(child!=NULL) child->GetPrintOrigin(width, height, originX, originY);
}

void matte::WantInputFocus(class view *requestor)
{
    if(this==requestor && this->child) (this->child)->view::WantInputFocus(this->child);
    else (this)->view::WantInputFocus(requestor);
}

void matte::ReceiveInputFocus()
    {
    if(this->child) (this->child)->ReceiveInputFocus();
    (this)->WantUpdate(this);
}

void matte::LoseInputFocus()
    {
    if(this->child) (this->child)->LoseInputFocus();
    (this)->WantUpdate(this);
}

class view *matte::Hit(enum view_MouseAction  action,long  mousex ,long  mousey ,long  numberOfClicks) 
{
    int fudge;
    if((this->child && !this->resizing) || this->ref == NULL)	
	return (this->child)->Hit(action,(this->child)->EnclosedXToLocalX( mousex), (this->child)->EnclosedYToLocalY( mousey),numberOfClicks);

    switch(action){
	case view_LeftDown:
	case view_RightDown:
	    {
	    long width = (this)->GetLogicalRight();
	    long height = (this)->GetLogicalBottom();
	    if (width <= BIGSIZE || height <= BIGSIZE)
		fudge = FUDGE;	/* normal fudge */
	    else
		fudge = BIGFUDGE;	/* for bigger insets (figures, tables, etc) */
	    if(width - fudge < mousex) this->Moving = matte_ChangeWidth;
	    else if(height - fudge < mousey) this->Moving = matte_ChangeHeight;
	    else if(this->child) 
		return (this->child)->Hit(action,(this->child)->EnclosedXToLocalX( mousex), (this->child)->EnclosedYToLocalY( mousey),numberOfClicks);
	    else return FALSE;
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
		if(move == matte_ChangeWidth && mousex > 0) {
		    this->ref->desw = mousex + 1;
		    (this->ref)->NotifyObservers(0);
		}
		else if(move == matte_ChangeHeight && mousey > 0) {
		    this->ref->desh = mousey + 1;
		    (this->ref)->NotifyObservers(0);
		}
		return (class view *) this;
	    } else {
		long width = (this)->GetLogicalRight();
		long height = (this)->GetLogicalBottom();
		if (width <= BIGSIZE || height <= BIGSIZE)
		    fudge = FUDGE;	/* normal fudge */
		else
		    fudge = BIGFUDGE;	/* for bigger insets (figures, tables, etc) */
		if(width-fudge >= mousex && height-fudge >= mousey && child) 
		    return (this->child)->Hit(action,(this->child)->EnclosedXToLocalX( mousex), (this->child)->EnclosedYToLocalY( mousey),numberOfClicks);
	    }
	default:
	    break;
    }
    return (class view *) this;
}
static void UpdateDrawing(class matte  *self)
{
    if(self->OldMode != self->drawing){
	struct rectangle enclosingRect;
	(self)->SetTransferMode(graphic_INVERT);
	enclosingRect.top = 0; enclosingRect.left = 0;
	enclosingRect.width  = (self)->GetLogicalWidth() -1 ;
	enclosingRect.height = (self)->GetLogicalHeight() -1 ;
	(self)->DrawRect(&enclosingRect);
	self->OldMode = self->drawing;
    }
}
void matte::Update()
{
    if(this->ref && (this->desw != this->ref->desw || this->desh != this->ref->desh)){
	this->desw = this->ref->desw ;
	this->desh = this->ref->desh ;
	if(!this->sizepending) {
	    (this)->view::WantNewSize(this); 
	    this->sizepending = TRUE;
	}
	return; /* Might as well, since our parent will be redrawing us again */
    }
    if(this->child) (this->child)->Update();
    UpdateCursors(this);
    UpdateDrawing(this);
}
void matte::FullUpdate(enum view_UpdateType  type,long  left,long  top,long  width,long  height)
{
    struct rectangle enclosingRect;
    if(type == view_FullRedraw && this->ref && (this->desw != this->ref->desw || this->desh != this->ref->desh)){
	this->desw = this->ref->desw ;
	this->desh = this->ref->desh ;
	if(!this->sizepending) {
	    (this)->view::WantNewSize(this); 
	    this->sizepending = TRUE;
	}
	return; /* Might as well, since our parent will be redrawing us again */
    }
    enclosingRect.top = 0; enclosingRect.left = 0;
    enclosingRect.width  = (this)->GetLogicalWidth() -1 ;
    enclosingRect.height = (this)->GetLogicalHeight() -1 ;
    if(type != view_Remove){
	UpdateCursors(this);
	if(type != view_MoveNoRedraw){
	    (this)->SetTransferMode(graphic_WHITE);
	    (this)->DrawRect(&enclosingRect);
	    (this)->SetTransferMode(graphic_INVERT);
	    if(this->drawing){ 
		(this)->DrawRect(&enclosingRect);
	    }
	    this->OldMode = this->drawing;
	}
    }
    enclosingRect.top++; enclosingRect.left++;
    if (enclosingRect.width <= BIGSIZE || enclosingRect.height <= BIGSIZE) {
	enclosingRect.width-- ;
	enclosingRect.height-- ;
    } else {
	enclosingRect.width -= BIGFUDGE-1  ;
	enclosingRect.height -= BIGFUDGE-1 ;
    }

    if(this->child) {
	(this->child)->InsertView( this, &enclosingRect);
	(this)->RetractViewCursors(this->child);
	(this->child)->FullUpdate(type,left,top,width,height);
    }
}

static char *matte_PromptForViewName(class matte  *self)
{   /* MISSING FUNCTION */
    return NULL;
}

class matte *matte::Create(class viewref  *vr,class view  *parent)
{
    ATKinit;

    class matte *self;
    const char *viewT=vr->viewType;
    self = new matte;
    self->ref = vr;
    self->desh = self->ref->desh;
    self->desw = self->ref->desw;
    (self)->LinkTree(parent);

    if(vr->dataObject && ATK::IsTypeByName(vr->dataObject->GetTypeName(), "unknown")) {
	if(viewT==NULL || !ATK::IsTypeByName(viewT, "unknownv")) {
	    viewT="unknownv";
	}
    }
    if(viewT == NULL || !ATK::LoadClass(viewT) || !ATK::IsTypeByName(viewT,"view")){

	if(!matte_PromptForViewName(self)) {
	    (self)->Destroy();
	    return(NULL);
	}
    }
    if((self->child = (class view *)ATK::NewObject(viewT)) == NULL){
	(self)->Destroy();
	return(NULL);
    }
    if(vr->dataObject)
	(self->child )->SetDataObject(vr->dataObject);
    (self->child)->LinkTree((class view *)self);
    (vr)->AddObserver(self);
    (self->child)->AddObserver(self);
    return(self);
}

void matte::SetDataObject(class dataobject  *dd)
{
    if(this->child){
	(this->child)->SetDataObject(dd);
    }
}

matte::matte()
{
    ATKinit;
    this->widthcursor = cursor::Create(this);
    this->heightcursor = cursor::Create(this);
    (this->heightcursor)->SetStandard(Cursor_HorizontalBars);
    (this->widthcursor)->SetStandard(Cursor_VerticalBars);
    this->Moving = 0; 
    this->WasMoving = 0;
    this->resizing = environ::GetProfileSwitch("ResizeInset", RESIZING);
    this->WasResizing = 0;
    this->ref = NULL;
    this->child = NULL;
    this->desw = this->desh = UNSET;
    this->menus = (matteMenus)->DuplicateML( this);
    this->drawing = environ::GetProfileSwitch("DrawInsetBorder", DRAWING);
    this->sizepending = TRUE;
    this->childhasfocus = FALSE;
    THROWONFAILURE( TRUE);
}

static void ApplyEnvironmentsToStyle(environment *env, style *dest) {
    if(env==NULL) return;
    if(env->GetParent()!=NULL) ApplyEnvironmentsToStyle((environment *)env->GetParent(), dest);
    if(env->type!=environment_Style || env->data.style==NULL) return;
    env->data.style->MergeInto(dest);
}

const char *matte::WantInformation(const char *name) {
    if(strcmp(name, "style")==0) {
        // compute a style representing the effects of all the styles
        // effective at this matte's location in the text.  the caller
        // is responsible for freeing the style object.
        if(ref) {
            style *result=new style;
            const char *font;
            char bodyFont[256];
            long fontStyle, fontSize;
            if ((font = environ::GetProfile("bodyfont")) == NULL || ! fontdesc::ExplodeFontName(font, bodyFont, sizeof(bodyFont), &fontStyle, &fontSize)) {
                strcpy(bodyFont, "Andy");
                fontStyle = 0;
                fontSize = 12;		// needs to be the default bodyfont size XXX
            }
            boolean justify = environ::GetProfileSwitch("justified", TRUE);
            (result)->SetFontFamily( bodyFont);
            (result)->SetFontSize( style_ConstantFontSize, fontSize);
            (result)->AddNewFontFace( fontStyle);
            if (! justify)
                (result)->SetJustification( style_LeftJustified);
            ApplyEnvironmentsToStyle(ref->Environment(), result);
            return (char *)result;
        } else return NULL;
    }
    return view::WantInformation(name);
}

void matte::SetResizing(long  key)
{
    if(this->resizing == 0){
	this->resizing = 1;
    }
    else {
	this->resizing = 0;
	this->WasResizing = 1;
    }
    (this)->WantUpdate(this);
}

void matte::PostMenus(class menulist  *menulist)
{
    (this->menus)->ClearChain();
    (this->menus)->ChainBeforeML( menulist, (long) menulist);
    (this)->view::PostMenus( this->menus);
}

void matte::WantNewSize(class view  *requestor)
    {

    if(this->ref && (this->ref->desw != UNSET ||  this->ref->desh != UNSET)){
	this->ref->desw = this->ref->desh = UNSET;
      this->desw = this->desh = UNSET;
	(this->ref)->NotifyObservers(0);
    }
      if(!this->sizepending) {
	(this)->view::WantNewSize(this);
	  this->sizepending = TRUE;
      }
}

static long procSetResizing(ATK *v, long c)
{
    ((matte *)v)->SetResizing(c);
    return 0;
}

static void procSetDrawing(class matte  *self,long  key)
{
    self->drawing = !self->drawing;
    (self)->WantUpdate(self);
}

static struct bind_Description matteBindings[]={
    {"matte-allow-resizing", NULL,0,NULL,0,0, (proctable_fptr)procSetResizing, "Allow view to resize" },
    {"matte-allow-drawing", NULL,0,NULL,0,0, (proctable_fptr)procSetDrawing, "Draw the view's border" },
    NULL
};

boolean matte::InitializeClass()
    {
    matteMenus = new menulist;
    matteKeyMap =  new keymap;
    bind::BindList(matteBindings, matteKeyMap , matteMenus, &matte_ATKregistry_ );
    return TRUE;
}

void matte::ObservedChanged(class observable  *changed, long  value)
{
    if(changed == (class observable *)this->ref ) 
    {
	if (value == observable_OBJECTDESTROYED){
	    this->ref = NULL;
 /* assumes the parent is also observing the viewref and will destroy the matte */
/*	    matte_Destroy(self); */
	}
	else if (this->ref && (this->desw != this->ref->desw || this->desh != this->ref->desh))
	    (this)->WantUpdate(this);
	}
    if(changed == (class observable *)this->child && value ==  observable_OBJECTDESTROYED){
	this->child = NULL;
	if(this->ref) {
	    class viewref *vv = this->ref;
	    this->ref = NULL;
	    (vv)->RemoveObserver(this);
	    (vv)->Destroy();
	}
	else (this)->Destroy();
    }
}
void matte::LinkTree(class view  *parent)
{

    (this)->view::LinkTree( parent);
    if (this->child != NULL)
        (this->child)->LinkTree( this);
}
void matte::InitChildren()
{
    if (this->child != NULL)
        (this->child)->InitChildren();
}
matte::~matte()
{
	ATKinit;

    if(this->child){
	class view *child = this->child;
	this->child = NULL;
	(child)->RemoveObserver(this);
	(child)->Destroy();
    }
    if(this->ref){
	(this->ref)->RemoveObserver(this);
    }
    delete this->widthcursor;
    delete this->heightcursor;
    delete this->menus;
}

boolean matte::RecSearch(struct SearchPattern *pat, boolean toplevel)
{
    if (this->child)
	return this->child->RecSearch(pat, toplevel);
    return FALSE;
}

boolean matte::RecSrchResume(struct SearchPattern *pat)
{
    if (this->child)
	return this->child->RecSrchResume(pat);
    return FALSE;
}

boolean matte::RecSrchReplace(class dataobject *text, long pos, long len)
{
    if (this->child)
	return this->child->RecSrchReplace(text, pos, len);
    return FALSE;
}


void matte::PrintPSDoc(FILE *outfile, long pagew, long pageh)
{
    if (this->child)
	this->child->PrintPSDoc(outfile, pagew, pageh);
}
void *matte::GetPSPrintInterface(const char *printtype)
{
     if (this->child)
	return this->child->GetPSPrintInterface(printtype); 
    return NULL;
}
void matte::DesiredPrintSize(long width, long height, enum view_DSpass pass, long *desiredwidth, long *desiredheight)
{
    if (this->child)
	this->child->DesiredPrintSize(width, height, pass, desiredwidth, desiredheight);
}
void matte::PrintPSRect(FILE *outfile, long logwidth, long logheight, struct rectangle *visrect)
{
    if (this->child)
	this->child->PrintPSRect(outfile, logwidth, logheight, visrect);
}

void matte::ChildLosingInputFocus()
{
    this->childhasfocus = FALSE;
    this->drawing = FALSE;
    (this)->WantUpdate(this);
}

void matte::ChildReceivingInputFocus()
{
    this->childhasfocus = TRUE;
    this->drawing = TRUE;
    (this)->WantUpdate(this);
}

void matte::RecSrchExpose(const struct rectangle &logical, struct rectangle &hit) {
    if(child==NULL) hit=logical;
    else {
	struct rectangle bounds=logical;
	if(!child->GetIM()) child->LinkTree(this);
#if 0
	struct rectangle oldrect;
	child->GetEnclosedBounds(&oldrect);
	child->InsertView(this, &oldrect);
#endif
	bounds.left++;
	bounds.top++;
	bounds.width-=2;
	bounds.height-=2;
	child->RecSrchExpose(bounds,hit);
    }
}
