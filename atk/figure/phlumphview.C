/*
	Copyright Carnegie Mellon University 1994 - All rights reserved
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

#include <andrewos.h>
ATK_IMPL("phlumphview.H")
#include <phlumphview.H>

#include <graphic.H>
#include <fontdesc.H>
#include <menulist.H>
#include <keymap.H>
#include <keystate.H>
#include <proctable.H>
#include <buffer.H>
#include <print.H>
#include <frame.H>
#include <im.H>
#include <textview.H>
#include <textflow.H>
#include <textflowview.H>
#include <phlumph.H>

/* IconMenus flags */
#define	ML_Open		(1)
#define	ML_Closed	(2)

/* RealMenus flags */
#define ML_AnyPages	(1)
#define ML_NotFirstPage	(2)
#define ML_NotLastPage	(4)
#define ML_RepeatSet	(8)

ATKdefineRegistry(phlumphview, view, phlumphview::InitializeClass);

static class menulist *IconMenus;
static class keymap *IconKeymap;
static class menulist *RealMenus;
static class keymap *RealKeymap;
static class fontdesc *IconFontDesc;

static void RepostMenus(class phlumphview  *self, boolean force);
static void DrawAll(class phlumphview *self);
static void AdjustToCurnum(class phlumphview  *self);

static void OpenProc(class phlumphview  *self, long  val);
static void CloseProc(class phlumphview  *self, long  val);

static void SuicideProc(class phlumphview  *self, long  val);
static void NextPageProc(class phlumphview  *self, long  val);
static void PreviousPageProc(class phlumphview  *self, long  val);
static void InsertPageProc(class phlumphview  *self, long  val);
static void DeletePageProc(class phlumphview  *self, long  val);
static void SetRepeatProc(class phlumphview  *self, long  val);
static void phlumphview_DestroyPagev(class textflowview *pagv);

boolean phlumphview::InitializeClass()
{
    struct proctable_Entry *proc = NULL;

    IconFontDesc = fontdesc::Create("andysans", fontdesc_Bold, 12);

    /* --- proctable for iconified view --- */

    IconMenus = new menulist;
    IconKeymap = new keymap;

    proc = proctable::DefineProc("phlumph-open", (proctable_fptr)OpenProc, &phlumphview_ATKregistry_ , NULL, "Open window containing page layouts.");
    (IconMenus)->AddToML( "Phlumph~5,Open~10", proc, 0, ML_Closed);

    proc = proctable::DefineProc("phlumph-close", (proctable_fptr)CloseProc, &phlumphview_ATKregistry_ , NULL, "Close window containing page layouts.");
    (IconMenus)->AddToML( "Phlumph~5,Close~11", proc, 0, ML_Open);

    /* --- proctable for real view --- */

    RealMenus = new menulist;
    RealKeymap = new keymap;

    proc = proctable::DefineProc("phlumph-toolset-suicide", (proctable_fptr)SuicideProc, &phlumphview_ATKregistry_ , NULL, "Deletes this window.");
    (RealKeymap)->BindToKey( "\030\004", proc, 0);	/* ^X^D */
    (RealKeymap)->BindToKey( "\030\003", proc, 0);	/* ^X^C */
    (RealMenus)->AddToML( "Quit~99", proc, 0, 0);
    
    proc = proctable::DefineProc("phlumph-next-page", (proctable_fptr)NextPageProc, &phlumphview_ATKregistry_ , NULL, "Go to next page of page list.");
    (RealKeymap)->BindToKey( "\030\016", proc, 0);	/* ^X^N */
    (RealMenus)->AddToML( "Phlumph~6,Next Page~21", proc, 0, ML_NotLastPage);
    
    proc = proctable::DefineProc("phlumph-previous-page", (proctable_fptr)PreviousPageProc, &phlumphview_ATKregistry_ , NULL, "Go to previous page of page list.");
    (RealKeymap)->BindToKey( "\030\020", proc, 0);	/* ^X^P */
    (RealMenus)->AddToML( "Phlumph~6,Previous Page~20", proc, 0, ML_NotFirstPage);
    
    proc = proctable::DefineProc("phlumph-insert-page", (proctable_fptr)InsertPageProc, &phlumphview_ATKregistry_ , NULL, "Insert new page before the current one or at the end.");
    (RealKeymap)->BindToKey( "\030a", proc, 0);	/* ^Xa */
    (RealMenus)->AddToML( "Phlumph~6,Add Page~10", proc, 0, 0);
    (RealKeymap)->BindToKey( "\030i", proc, 1);	/* ^Xi */
    (RealMenus)->AddToML( "Phlumph~6,Insert Page Here~11", proc, 1, ML_AnyPages);
    
    proc = proctable::DefineProc("phlumph-delete-page", (proctable_fptr)DeletePageProc, &phlumphview_ATKregistry_ , NULL, "Insert new page before the current one.");
    (RealKeymap)->BindToKey( "\030d", proc, 0);	/* ^Xd */
    (RealMenus)->AddToML( "Phlumph~6,Delete This Page~12", proc, 0, ML_AnyPages);
    
    proc = proctable::DefineProc("phlumph-set-repeat", (proctable_fptr)SetRepeatProc, &phlumphview_ATKregistry_ , NULL, "Insert new page before the current one.");
    (RealMenus)->AddToML( "Phlumph~6,Set Page Repeat~30", proc, 0, ML_AnyPages);
    (RealMenus)->AddToML( "Phlumph~6,Clear Page Repeat~31", proc, 1, ML_AnyPages|ML_RepeatSet);
    
    return TRUE;
}

/* call with iconmode==TRUE if you want it iconified. Call with iconmode==FALSE if you want a full phlumph inset with figures and everything. Call with no arguments if you want the default behavior (ie, if you want whatever the user would get if he his esc-tab phlumph in text. */
phlumphview::phlumphview(boolean iconmode)
{
    struct proctable_Entry *proc = NULL;
    ATKinit;

    this->IconMode = iconmode;

    if (this->IconMode) {
	this->Menus = (IconMenus)->DuplicateML( this);
	this->Keystate = keystate::Create(this, IconKeymap);
    }
    else {
	this->Menus = (RealMenus)->DuplicateML( this);
	this->Keystate = keystate::Create(this, RealKeymap);
    }

    this->HasInputFocus = FALSE;
    this->toolset = NULL;
    this->curview = NULL;
    this->curnum = 0;
    this->lastwidth = 0;
    this->lastheight = 0;

    THROWONFAILURE( TRUE);
}

phlumphview::~phlumphview()
{
    ATKinit;

    delete this->Menus;
    delete this->Keystate;
}

static void DrawAll(class phlumphview *self)
{
}

static void DrawIcon(class phlumphview *self)
{
    struct rectangle box;
    class phlumph *dat = (class phlumph *)self->GetDataObject();
    char buf[32];

    self->GetLogicalBounds(&box);
    box.width--;
    box.height--;
    self->DrawRect(&box);
    box.left++;
    box.top++;
    box.width--;
    box.height--;
    self->EraseRect(&box);

    sprintf(buf, "%d page%s", dat->GetNumPages(), (dat->GetNumPages()==1 ? "" : "s"));
    self->SetFont(IconFontDesc);
    self->MoveTo(5, 5);
    self->DrawString(buf, graphic_ATLEFT|graphic_ATTOP);
}

void phlumphview::FullUpdate(enum view_UpdateType type, long left, long top, long width, long height)
{
    if (this->IconMode)
	DrawIcon(this);
    else {
	struct rectangle rec;
	this->GetLogicalBounds(&rec);
	if (this->lastwidth != rec.width || this->lastheight != rec.height) {
	    this->lastwidth = rec.width;
	    this->lastheight = rec.height;
	    if (this->curview) {
		this->curview->LinkTree(this);
		this->curview->InsertView(this, &rec);
	    }
	}
	DrawAll(this);
	if (this->curview) {
	    this->curview->FullUpdate(type, left, top, width, height);
	}
    }
}

void phlumphview::Update()
{
    if (this->IconMode)
	DrawIcon(this);
    else {
	DrawAll(this);
	if (this->curview) {
	    this->curview->Update();
	}
    }
}

/*void phlumphview::ReceiveInputFocus()
{
    printf("### phlumphview: ReceiveInputFocus\n");
    this->HasInputFocus = TRUE;
    (this)->PostKeyState( NULL);
    RepostMenus(this, TRUE);
}

void phlumphview::LoseInputFocus()
{
    printf("### phlumphview: LoseInputFocus\n");
    this->HasInputFocus = FALSE;
 }*/

void phlumphview::WantInputFocus(class view *requestor)
{
    if (this==requestor && this->curview) { (this->curview)->view::WantInputFocus(this->curview);
    }
    else {
	this->view::WantInputFocus(requestor);
    }
}

void phlumphview::ReceiveInputFocus()
{
    this->HasInputFocus = TRUE;
    if (this->curview) {
	(this->curview)->ReceiveInputFocus();
    }
    else {
	(this)->PostKeyState( NULL);
	RepostMenus(this, TRUE);
    }
    /*this->WantUpdate(this);*/
}

void phlumphview::LoseInputFocus()
{
    this->HasInputFocus = FALSE;
    if (this->curview)
	(this->curview)->LoseInputFocus();
    /*this->WantUpdate(this);*/
}

/* doesn't assume self is input focus */
static void RepostMenus(class phlumphview  *self, boolean force)
{
    long menumask = 0;
    long val;

    if (self->IconMode) {
	if (!self->toolset) 
	    menumask |= ML_Closed;
	else
	    menumask |= ML_Open;
    }
    else {
	class phlumph *dat;
	dat = (class phlumph *)self->GetDataObject();
	if (dat->GetNumPages() != 0) {
	    menumask |= ML_AnyPages;
	    if (self->curnum+1 < dat->GetNumPages())
		menumask |= ML_NotLastPage;
	    if (self->curnum > 0)
		menumask |= ML_NotFirstPage;
	}
	if (dat->GetRepeat())
	    menumask |= ML_RepeatSet;
    }

    if ((self->Menus)->SetMask( menumask) || force) {
	if (self->HasInputFocus)
	    self->PostMenus( NULL);
	else if (self->curview)
	    self->curview->PostMenus(NULL);
    }
}

void phlumphview::PostMenus(class menulist  *ml)
{
    /* Enable the menus for this object. */
    (this->Menus)->UnchainML( (long)this);
    if (ml)
	(this->Menus)->ChainBeforeML( ml, (long)this);

    (this)->view::PostMenus( this->Menus);
}

void phlumphview::PostKeyState(class keystate  *ks)
{
    /* Enable the keys for this object. */
    class keystate *newch;

    newch = (this->Keystate)->AddAfter( ks);
    (this)->view::PostKeyState( newch);
}

view_DSattributes phlumphview::DesiredSize(long  width, long  height, enum view_DSpass pass, long  *dwidth, long  *dheight)
{
    if (this->IconMode) {
	*dwidth = 80;
	*dheight = 32;
	return view_Fixed;
    }
    else {
	return this->view::DesiredSize(width, height, pass, dwidth, dheight);
    }
}

void phlumphview::ObservedChanged(class observable  *observed, long  status)
{
    if (this->toolset && observed == this->toolset) {
	if (status==observable_OBJECTDESTROYED) {
	    CloseProc(this, 1);
	}
    }
    else { /* observed == dataobject, we hope */
	if (status==observable_OBJECTDESTROYED) {
	}
	else if (status==phlumph_PAGESCHANGED) {
	    class phlumph *dat = (class phlumph *)observed;
	    if (this->IconMode) {
		this->WantUpdate(this);
	    }
	    else {
		AdjustToCurnum(this);
	    }
	}
	else {
	    /* don't need anything drastic */
	    RepostMenus(this, FALSE);
	}
    }
}

void phlumphview::SetDataObject(class dataobject  *f)
{
    class phlumph *dat = (class phlumph *)f;
    (this)->view::SetDataObject(dat);
    if (!this->IconMode) {
	this->curnum = 0;
	AdjustToCurnum(this);
    }
}

void phlumphview::LinkTree( class view  *parent )
{
    this->view::LinkTree( parent);
    if (parent && this->GetIM()) {
	/* call child->LinkTree(this) on all children */
	if (!this->IconMode && this->curview)
	    this->curview->LinkTree(this);
    }
}

class view *phlumphview::Hit(enum view_MouseAction action, long x, long y, long numclicks)
{
    class view *res;
    if (this->IconMode) {
	if (!this->HasInputFocus) {
	    (this)->WantInputFocus( this);
	}
	return (class view *)this;
    }
    else {
	if (!this->curview) {
	    if (!this->HasInputFocus) {
		(this)->WantInputFocus( this);
	    }
	    return (class view *)this;
	}
	else {
	    res = this->curview->Hit(action, x, y, numclicks);
	    return res;
	}
    }
}

static void OpenProc(class phlumphview  *self, long  val)
{
    class phlumph *dat = (class phlumph *)(self)->GetDataObject();
    class im *im;
    class frame *fr;
    class phlumphview *tv;
    boolean res;

    if (!self->IconMode){
	message::DisplayString(self, 40, "This procedure only applies to iconified phlumphviews.");
	return;
    }

    if (self->toolset)  {
	message::DisplayString(self, 40, "There is already a toolset window.");
	return;
    }

    if (!dat) {
	message::DisplayString(self, 40, "This view has no data!");
	return;
    }

    im = im::Create(NULL);
    if (!im) {
	message::DisplayString(self, 40, "Unable to create new window.");
	return;
    }

    fr = new frame;
    if (!fr) {
	message::DisplayString(self, 40, "Unable to create window frame.");
	return;
    }

    /*frame_SetCommandEnable(fr, TRUE);*/
    (fr)->PostDefaultHandler( "message", (fr)->WantHandler( "message"));

    tv = new phlumphview(FALSE);
    if (!tv) {
	message::DisplayString(self, 40, "Unable to create toolset.");
	return;
    }

    tv->SetDataObject(dat);

    /*res = (tv)->SetPrimaryView( self);
    if (!res) {
	message::DisplayString(self, 40, "Unable to initialize toolset (this shouldn't happen).");
	return;
    }*/

    self->toolset = tv;
    (self->toolset)->AddObserver( self);
    (fr)->SetView( tv);
    (fr)->SetQuitWindowFlag( TRUE);
    (im)->SetView( fr);
    {   
	class buffer *buf = buffer::FindBufferByData(dat); 
	char *nm;
	if (buf) {
	    nm = (buf)->GetFilename();
	    if (nm)
		(im)->SetTitle( nm); /* ### */
	}
    }
    (tv)->WantInputFocus( tv);
    RepostMenus(self, FALSE);
}

/* if val==1, don't actually destroy the toolset -- someone else is about to. */
static void CloseProc(class phlumphview  *self, long  val)
{
    class im *toolim = NULL;
    class frame *toolfr = NULL;

    if (!self->IconMode){
	message::DisplayString(self, 40, "This procedure only applies to iconified phlumphviews.");
	return;
    }

    if (!self->toolset) {
	message::DisplayString(self, 40, "There is no toolset window.");
	return;
    }

    toolim = (self->toolset)->GetIM();
    if (toolim)
	toolfr = (class frame *)toolim->topLevel;

    (self->toolset)->RemoveObserver( self);

    if (toolim) {
	(toolim)->SetView( NULL);
    }

    if (toolfr) {
	(toolfr)->SetView( NULL);
    }

    if (!val) {
	(self->toolset)->Destroy();
    }
    self->toolset = NULL;

    if (toolim) {
	(toolim)->Destroy(); 
    }
    if (toolfr) {
	(toolfr)->Destroy();
    }

    /*(self)->WantUpdate( self);*/
    RepostMenus(self, FALSE); 
}

static void SuicideProc(class phlumphview  *self, long  val)
{
    if (self->IconMode){
	message::DisplayString(self, 40, "This procedure does not apply to iconified phlumphviews.");
	return;
    }

    self->Destroy();
}

static void AdjustToCurnum(class phlumphview  *self)
{
    class phlumph *dat;
    char buf[64];
    class textflowview *oldcurview;
    dat = (class phlumph *)self->GetDataObject();

    if (self->curnum < 0 || self->curnum >= dat->GetNumPages()) {
	/* get rid of inset */
	if (self->curview) {
	    oldcurview = self->curview;
	    self->curview = NULL;
	    self->WantInputFocus(self);
	    oldcurview->SetDataObject(NULL);
	    oldcurview->Destroy();
	}
    }
    else {
	/* add inset */
	class textflow *pag = dat->GetPage(self->curnum);
	if (self->curview) {
	    struct rectangle currec;
	    self->GetLogicalBounds(&currec);
	    self->curview->SetDataObject(pag);
	    self->curview->WantInputFocus(self->curview);
	    self->curview->FullUpdate(view_FullRedraw, 0, 0, currec.width, currec.height);
	}
	else {
	    struct rectangle currec;
	    self->GetLogicalBounds(&currec);
	    self->curview = new textflowview;
	    self->curview->SetDataObject(pag);
	    self->curview->LinkTree( self);
	    self->curview->InsertView( self, &currec);
	    self->curview->WantInputFocus(self->curview);
	    self->curview->FullUpdate(view_FullRedraw, 0, 0, currec.width, currec.height);
	}
    }

    RepostMenus(self, FALSE);
    if (dat->GetNumPages())
	sprintf(buf, "Now viewing page %d", self->curnum+1);
    else
	strcpy(buf, "There are no pages in this phlumph.");
    message::DisplayString(self, 40, buf);
}

static void NextPageProc(class phlumphview  *self, long  val)
{
    class phlumph *dat;

    if (self->IconMode){
	message::DisplayString(self, 40, "This procedure does not apply to iconified phlumphviews.");
	return;
    }

    dat = (class phlumph *)self->GetDataObject();
    if (dat->GetNumPages() == 0) {
	message::DisplayString(self, 40, "There are no pages.");
	return;
    }
    if (self->curnum+1 == dat->GetNumPages()) {
	if (self->curnum == 0)
	    message::DisplayString(self, 40, "This is the only page.");
	else
	    message::DisplayString(self, 40, "This is the last page.");
	return;
    }

    self->curnum++;
    AdjustToCurnum(self);
}

static void PreviousPageProc(class phlumphview  *self, long  val)
{
    class phlumph *dat;

    if (self->IconMode){
	message::DisplayString(self, 40, "This procedure does not apply to iconified phlumphviews.");
	return;
    }

    dat = (class phlumph *)self->GetDataObject();
    if (dat->GetNumPages() == 0) {
	message::DisplayString(self, 40, "There are no pages.");
	return;
    }
    if (self->curnum == 0) {
	if (dat->GetNumPages() == 1)
	    message::DisplayString(self, 40, "This is the only page.");
	else
	    message::DisplayString(self, 40, "This is the first page.");
	return;
    }

    self->curnum--;
    AdjustToCurnum(self);
}

/* val==0 to insert at end, 1 to insert before curnum */
static void InsertPageProc(class phlumphview  *self, long  val)
{
    class phlumph *dat;
    int pos;
    boolean res;

    if (self->IconMode){
	message::DisplayString(self, 40, "This procedure does not apply to iconified phlumphviews.");
	return;
    }

    dat = (class phlumph *)self->GetDataObject();
    if (!val)
	pos = dat->GetNumPages();
    else
	pos = self->curnum;

    if (pos < 0)
	pos = 0;
    if (pos > dat->GetNumPages())
	pos = dat->GetNumPages();

    res = dat->InsertPage(pos);
    if (!res) {
	message::DisplayString(self, 40, "Unable to create new page.");
	return;
    }

    self->curnum = pos;
    dat->NotifyObservers( phlumph_PAGESCHANGED);
}

static void DeletePageProc(class phlumphview  *self, long  val)
{
    class phlumph *dat;
    int pos;

    if (self->IconMode){
	message::DisplayString(self, 40, "This procedure does not apply to iconified phlumphviews.");
	return;
    }

    dat = (class phlumph *)self->GetDataObject();
    dat->DeletePage(self->curnum);

    if (self->curnum > 0)
	self->curnum--;
    dat->NotifyObservers( phlumph_PAGESCHANGED);
}

static void SetRepeatProc(class phlumphview  *self, long  val)
{
    class phlumph *dobj = (class phlumph *)self->GetDataObject();
    int newval;

    if (val==1) {
	dobj->SetRepeat(0);
	dobj->NotifyObservers( phlumph_DATACHANGED);
	message::DisplayString(self, 10, "Repeat value cleared.");
    }
    else {
	char msgbuf[256];
	char buffer[64], buffer2[32];
	int res;

	strcpy(msgbuf, "Enter the number of pages to repeat in this phlumph:  ");
	if (dobj->GetRepeat())
	    sprintf(buffer2, "%d", dobj->GetRepeat());
	else
	    strcpy(buffer2, "");
	res = message::AskForString(self, 40, msgbuf, buffer2, buffer, 30); 
	if (res<0 || strlen(buffer)==0) {
	    message::DisplayString(self, 10, "Cancelled.");
	    return;
	}

	newval = atoi(buffer);
	if (newval <= 0) {
	    message::DisplayString(self, 10, "Repeat value must be positive.");
	    return;
	}

	dobj->SetRepeat(newval);
	dobj->NotifyObservers( phlumph_DATACHANGED);
	sprintf(msgbuf, "Repeat value set to %d.", dobj->GetRepeat());
	message::DisplayString(self, 10, msgbuf);
    }
}

static void phlumphview_DestroyPagev(class textflowview *pagv)
{
    pagv->SetDataObject(NULL);
    pagv->Destroy();
}

static struct textps_layout_plan *PrintLump(void *rock, long pagewidth, long pageheight)
{
    class phlumphview *self = (class phlumphview *)rock;
    class phlumph *dat;
    class textflow *pag;
    class textflowview *pagv;
    struct textps_layout_plan *plan;
    int ix;
    int startrepeatpage;
    long startrepeatcol, lx;

    /*class phlumph *fig = (class phlumph *)self->dataobject;
    struct printlump lump;*/

    dat = (class phlumph *)self->GetDataObject();

    plan = new textps_layout_plan;
    plan->planid = textview::GetUniquePlanId();
    plan->numcols = 0;
    plan->repeat = 0;
    plan->cols = NULL;

    startrepeatcol = (-1);
    if (dat->GetRepeat())
	startrepeatpage = dat->GetNumPages() - dat->GetRepeat();
    else
	startrepeatpage = (-1);

    for (ix=0; ix<dat->GetNumPages(); ix++) {
	pag = dat->GetPage(ix);
	pagv = new textflowview;
	pagv->SetDataObject(pag);
	lx = pagv->BuildPlan(&plan, pagewidth, pageheight);
	print::PSRegisterCleanup((print_cleanup_fptr)phlumphview_DestroyPagev, pagv);
	if (ix == startrepeatpage) {
	    startrepeatcol = lx;
	}
    }

    if (startrepeatcol != (-1)) {
	plan->repeat = plan->numcols - startrepeatcol;
    }

    return plan;
}

void *phlumphview::GetPSPrintInterface(const char *printtype)
{
    static struct textview_insetdata dat;

    if (!strcmp(printtype, "text")) {
	dat.type = textview_Layout;
	dat.u.Layout.rock = (void *)this;
	dat.u.Layout.func = PrintLump;
	return (void *)(&dat);
    }

    return NULL;
}

