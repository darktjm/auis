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

#include <andrewos.h>
ATK_IMPL("bpv.H")
#include <view.H>
#include <fontdesc.H>
#include <observable.H>
#include <message.H>
#include <proctable.H>
#include <menulist.H>
#include <bp.H>
#include <bpv.H>
#include <textview.H>

#define	ML_pagenumset	    (1)

ATKdefineRegistry(bpv, view, bpv::InitializeClass);

static void SetPageNumProc(class bpv  *self, long  val);
static void ClearPageNumProc(class bpv  *self, long  val);
static void RepostMenus(class bpv  *self, boolean force);

static class menulist *EmbeddedMenus;
static class fontdesc *ClassFontDesc;

boolean bpv::InitializeClass()
{
    struct proctable_Entry *proc = NULL;

    ClassFontDesc = fontdesc::Create("andysans", fontdesc_Bold, 12);

    EmbeddedMenus = new menulist;

    proc = proctable::DefineProc("bpv-pagenum-set",(proctable_fptr) SetPageNumProc, &bpv_ATKregistry_ , NULL, "Set the new page number after this page break.");
    (EmbeddedMenus)->AddToML( "Pagebreak~5,Set Page Number~10", proc, 0, 0);
    proc = proctable::DefineProc("bpv-pagenum-clear",(proctable_fptr) ClearPageNumProc, &bpv_ATKregistry_ , NULL, "Clear the new page number after this page break.");
    (EmbeddedMenus)->AddToML( "Pagebreak~5,Clear Page Number~11", proc, 0, ML_pagenumset);

    return TRUE;
}

bpv::bpv()
{
    this->HasInputFocus = FALSE;
    this->Menus = (EmbeddedMenus)->DuplicateML( this);
    this->NowBig = FALSE;
    THROWONFAILURE( TRUE);
}

bpv::~bpv()
{
    delete this->Menus;
}

void bpv::SetDataObject(class dataobject *dobj)
{
    class bp *bpob = (class bp *)dobj;

    this->view::SetDataObject(dobj);
    if (bpob->haspagenum && !this->NowBig) {
	this->NowBig = TRUE;
	this->WantNewSize(this);
    }
    else if (!bpob->haspagenum && this->NowBig) {
	this->NowBig = FALSE;
	this->WantNewSize(this);
    }
}

view_DSattributes bpv::DesiredSize(long  width , long  height, enum view_DSpass  pass, long  *desiredwidth , long  *desiredheight)
{
    *desiredwidth = width;
    if (this->NowBig)
	*desiredheight = 28;
    else
	*desiredheight = 8;
    return (view_DSattributes)(view_HeightFlexible | view_WidthFlexible);
}

void bpv::Update()
{
    this->FullUpdate(view_FullRedraw, 0, 0, 0, 0);
}

void bpv::FullUpdate(enum view_UpdateType  type,long  left,long  top,long  width,long  height)
{
    struct rectangle enclosingRect;
    class bp *dobj = (class bp *)(this)->GetDataObject();
    int graylevel = (this->HasInputFocus) ? 8 : 4;
    char buf[48];

    enclosingRect.top = 0;
    enclosingRect.left = 0;
    enclosingRect.width  = (this)->GetLogicalWidth() - 1 ;
    enclosingRect.height = (this)->GetLogicalHeight() - 1 ;
    (this)->SetTransferMode(graphic_WHITE);
    (this)->EraseRect(&(enclosingRect));
    (this)->SetTransferMode(  graphic_COPY );
    (this)->FillRect(&(enclosingRect) ,(this)->GrayPattern(graylevel,16));

    if (dobj->haspagenum) {
	long wid, hgt;
	sprintf(buf, "Next page number: %ld", dobj->pagenum);
	ClassFontDesc->StringSize(this->GetDrawable(), buf,
					     &wid, &hgt);
	enclosingRect.left = 30;
	enclosingRect.top = 4;
	enclosingRect.width = wid+4;
	enclosingRect.height = 19;
	(this)->EraseRect(&(enclosingRect));
	(this)->SetFont( ClassFontDesc);
	(this)->MoveTo(32, 5);
	(this)->DrawString(buf, graphic_ATLEFT|graphic_ATTOP);
    }
}

void bpv::ObservedChanged(class observable  *obs, long  status)
{
    if (status == observable_OBJECTDESTROYED) {
    }
    else if (status == bp_DATACHANGED) {
	class bp *dobj = (class bp *)(this)->GetDataObject();
	if (dobj->haspagenum && !this->NowBig) {
	    this->NowBig = TRUE;
	    this->WantNewSize(this);
	}
	else if (!dobj->haspagenum && this->NowBig) {
	    this->NowBig = FALSE;
	    this->WantNewSize(this);
	}
	else {
	    (this)->WantUpdate( this);
	}
    }
}

void bpv::ReceiveInputFocus()
{
    this->HasInputFocus = TRUE;
    (this)->PostKeyState( NULL);
    RepostMenus(this, TRUE);
    this->WantUpdate(this);
}

void bpv::LoseInputFocus()
{
    this->HasInputFocus = FALSE;
    this->WantUpdate(this);
}

/* assumes self is input focus */
static void RepostMenus(class bpv  *self, boolean force)
{
    class bp *dobj = (class bp *)(self)->GetDataObject();
    long menumask = 0;

    if (dobj->haspagenum)
	menumask |= ML_pagenumset;

    if ((self->Menus)->SetMask( menumask) || force) {
	(self)->PostMenus( NULL);
    }
}

void bpv::PostMenus(class menulist  *ml)
{
    /* Enable the menus for this object. */
    (this->Menus)->UnchainML( (long)this);
    if (ml)
	(this->Menus)->ChainBeforeML( ml, (long)this);

    (this)->view::PostMenus( this->Menus);
}

class view *bpv::Hit(enum view_MouseAction  action, long  x , long  y , long  num_clicks)
{
    if (!this->HasInputFocus) {
	(this)->WantInputFocus( this);
    }
    return (class view *)this;
}

static void SetPageNumProc(class bpv  *self, long  val)
{
    class bp *dobj = (class bp *)(self)->GetDataObject();
    char msgbuf[256];
    char buffer[64], buffer2[32];
    int res;

    strcpy(msgbuf, "Enter the number of the next page after this break:  ");
    if (dobj->haspagenum)
	sprintf(buffer2, "%ld", dobj->pagenum);
    else
	strcpy(buffer2, "");
    res = message::AskForString(self, 40, msgbuf, buffer2, buffer, 30); 
    if (res<0 || strlen(buffer)==0) {
	message::DisplayString(self, 10, "Cancelled.");
	return;
    }

    val = atol(buffer);

    dobj->SetPageNum(val);
    dobj->NotifyObservers( bp_DATACHANGED);
    sprintf(msgbuf, "The next page after this break will be numbered %ld.", val);
    message::DisplayString(self, 10, msgbuf);
    RepostMenus(self, FALSE);
}

static void ClearPageNumProc(class bpv  *self, long  val)
{
    class bp *dobj = (class bp *)(self)->GetDataObject();
    dobj->ClearPageNum();
    dobj->NotifyObservers( bp_DATACHANGED);
    message::DisplayString(self, 10, "This break will no longer affect the numbering sequence.");
    RepostMenus(self, FALSE);
}

void bpv::Print(FILE  *f, const char  *process, const char  *final, boolean  toplevel)
{
    class bp *dobj = (class bp *)(this->GetDataObject());

    if (dobj && dobj->haspagenum)
	fprintf(f, ".OC\n.bp %ld\n", dobj->pagenum);
    else
	fputs(".OC\n.bp\n", f);
}

void *bpv::GetPSPrintInterface(const char *printtype)
{
    static struct textview_insetdata dat;

    if (!strcmp(printtype, "text")) {
	dat.type = textview_NewPage;
	dat.u.NewPage = this;
	return (void *)(&dat);
    }

    return NULL;
}

