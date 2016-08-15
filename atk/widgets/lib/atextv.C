/* Copyright 1996 Carnegie Mellon University All rights reserved.
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
ATK_IMPL("atextv.H")

#ifndef NORCSID
static UNUSED const char rcsid[] = "$Header: /afs/cs.cmu.edu/project/atk-src-C++/atk/widgets/lib/RCS/atextv.C,v 1.9 1996/11/12 00:27:20 robr Exp $";
#endif

#include <atextv.H>
#include <proctable.H>
#include <atext.H>
#include <rect.h>
#include <message.H>
#include <astring.H>
#include <textview.H>
#include <print.H>
#include <text.H>
#include <keymap.H>
#include <keystate.H>
#include <physical.h>

static keymap *ATextvKeymap;

static boolean Init();
ATKdefineRegistry(ATextv, AWidgetView, Init);
ATKdefineRegistryNoInit(ATextSizeFormula,AFormulaViewAct);

ASlot_NAME(password);
void ATextvStateFormula::Evaluate() {
    atv->dataViewFormula.Validate();
    AText *txt=(AText *)atv->GetDataObject();
    text *src=txt->Source();
    AFORMULA_BEGIN;
    {
        if(atv->tv) {
            atv->tv->SetPassword(txt->Get(slot_password, abool(FALSE)));
        }
    }
    AFORMULA_END;
    AFORMULA_BEGIN;
    {
        if(atv->tv) {
            atv->tv->SetDotPosition(txt->CursorPosition());
            atv->tv->SetDotLength(0);
        }
    }
    AFORMULA_END;
    AFORMULA_BEGIN;
    {
        if(atv->tv) {
            atv->tv->SetTopPosition( src->GetBeginningOfLine( txt->TopCharacter()));
        }
    }
    AFORMULA_END;
    AFORMULA_BEGIN;
    {
        if(src && atv->tv) {
            boolean wrap=txt->WordWrap();
            style *s=atv->tv->GetDefaultStyle();
            if(s) {
                if(wrap) s->RemoveNoWrap();
                else s->AddNoWrap();
                src->RegionModified(0, src->GetLength());
                src->NotifyObservers( observable_OBJECTCHANGED);
            }
        }
    }
    AFORMULA_END;
    AFORMULA_BEGIN;
    {
        if(src) {
            fontdesc *fd=txt->Font();
            style *s=atv->tv->GetDefaultStyle();
            if(s && fd) {
                s->SetFontFamily(fd->GetFontFamily());
                s->SetFontSize(style_ConstantFontSize, fd->GetFontSize());
                s->ClearNewFontFaces();
                s->ClearOldFontFaces();
                s->AddNewFontFace(fd->GetFontStyle());
                src->RegionModified(0, src->GetLength());
                src->NotifyObservers( observable_OBJECTCHANGED);
            }
        }
    }
    AFORMULA_END;
}

START_ATEXTV_CALLBACK_METHOD(WantUpdateMethod) {
    if(GetIM()) WantUpdate(this);
}
END_ATEXTV_CALLBACK_METHOD();

START_ATEXTV_CALLBACK_METHOD(WantNewSizeMethod) {
    if(GetIM()) WantNewSize(this);
}
END_ATEXTV_CALLBACK_METHOD();

START_ATEXTV_MOUSE_METHOD(HitMethod) {
    dataViewFormula.Validate();
    if(child) {
        view *nf=NULL;
        if(mouseFocus) {
            graphic *drawable=GetDrawable();
            long mx=physical_LogicalXToGlobalX(drawable, ToPixX(x));
            long my=physical_LogicalYToGlobalY(drawable, ToPixY(y));
            nf=(mouseFocus)->Hit( action,                          physical_GlobalXToLogicalX( (mouseFocus)->GetDrawable(), mx),                          physical_GlobalYToLogicalY( (mouseFocus)->GetDrawable(), my),                          num_clicks);
            if(action==view_LeftUp || action==view_RightUp || action==view_UpMovement) nf=NULL;
        } else nf=child->Hit(action, child->EnclosedXToLocalX(ToPixX(x)), child->EnclosedYToLocalY(ToPixY(y)), num_clicks);
        if(nf!=mouseFocus && (action==view_LeftDown || action==view_RightDown || nf==NULL)) {
            if(mouseFocus) mouseFocus->RemoveObserver(this);
            if(nf) nf->AddObserver(this);
            mouseFocus=nf;
        }
    }
}
END_ATEXTV_MOUSE_METHOD();


AACTION_METHOD(ActivateMethod,ATextv) {
    AText *at=(AText *)GetDataObject();
    aaction *doit = at->ActivateCallback();
    if (doit) {
        avalueflex foo;
        (*doit)(at, avalueflex()|this, foo);
    }
}

static boolean Init() {
    ENTER_AACTION_METHOD(HitMethod, ATextv, "atext-hit", 0L, "Passes a mouse hit to the text.");
    ENTER_AACTION_METHOD(ActivateMethod, ATextv, "atext-activate", 0L, "Invokes the widget's activate callback.");
    ATextvKeymap=new keymap;
    ATextvKeymap->BindToKey("\n", proctable::Lookup("atext-activate"), 0);
    ATextvKeymap->BindToKey("\r", proctable::Lookup("atext-activate"), 0);
    return TRUE;
}


void ATextv::Init() {
    ATKinit;
    
}

static textview *NewView(const char *c) {
    ATK_CLASS(textview);
    ATKregistryEntry *ent=ATK::LoadClass(c);
    if(ent->IsType(class_textview)) return (textview *)ATK::NewObject(c);
    return NULL;
}

START_ATEXTV_CALLBACK_METHOD(DataViewMethod) {
    AFORMULA_BEGIN;
    if(dself->Source()==NULL) {
       dself->SetSource(new text);
    }
    if(dself->Source()!=lastobs) {
        if(lastobs) lastobs->RemoveObserver(this);
        lastobs=dself->Source();
        lastobs->AddObserver(this);
    }
    AFORMULA_END;
    AFORMULA_BEGIN_NAMED(viewpart) {
        const char *vc=dself->textViewClass;
        boolean haveapp=(child && tv && child!=tv);
        if(haveapp) {
            tv->DeleteApplicationLayer(child);
        }
        if(tv) tv->Destroy();
        if(*vc!='\0') tv=NewView(vc);
        if(tv==NULL) {
            vc=dself->Source()->ViewName();
            tv=NewView(vc);
        }
        child=tv;
        if(tv) {
            tv->SetDataObject(dself->Source());
        }
    }
    AFORMULA_END;
    AFORMULA_BEGIN {
        dataViewFormula.Validate(viewpart);
        GetApplicationLayer();
    }
    AFORMULA_END;
    AFORMULA_BEGIN_NAMED(linkpart) {
        dataViewFormula.Validate(viewpart);
        if(child) child->LinkTree(this);
    }
    AFORMULA_END;
    forcelink=linkpart;
    
    AFORMULA_BEGIN;
    AShadow *asb=GetBorder();
    if((boolean)(long)asb) {
        if((boolean)dself->scrolled) asb->UseBorder(0);
        else asb->UseBorder(AShadow_Plain);
    }
    AFORMULA_END;
}
END_ATEXTV_CALLBACK_METHOD();

START_ATEXTV_CALLBACK_METHOD(SizeMethod) {
    AText *b = (AText *)GetDataObject();
    dataViewFormula.Validate();
    struct rectangle linterior, exterior;
    long below, oy;
    linterior.left=0;
    linterior.top=0;
    linterior.width=0;
    linterior.height=0;
    long lrm=(long)(((long)b->marginWidth*2) * ((double)b->scaleWidth) / uxscale);
    long tbm=(long)(((long)b->marginHeight*2) * ((double)b->scaleHeight) / uyscale);
    TextDimensions(linterior.width, linterior.height, lrm, tbm);
    if(border) border->ExteriorRect(&linterior, &exterior);
    else exterior=linterior;
    size.SetDesired(
                    (long)(exterior.width),
                    (long)(exterior.height));
    if(tv) {
        long ox;
        tv->SetEmbeddedBorder(lrm, tbm);
        tv->SetBorder(lrm, tbm);
        child->GetOrigin(linterior.width, linterior.height, &ox, &oy);
    }
    if(border) {
        long dtx, ty;
        border->TotalUpperBorderWidthHeight(dtx, ty);
        oy+=ty+(tv?0:tbm);
    } else oy+=(tv?0:tbm);
    size.SetOrigin(0, oy);
}
END_ATEXTV_CALLBACK_METHOD();

START_ATEXTV_CALLBACK_METHOD(TextMethod) {
    // should use a formula to set vself->interior
    struct rectangle bounds;
    GetLogicalBounds(&bounds);
    if(border) {
	border->InteriorRect(&bounds, &interior);
    } else interior=bounds;

    dataViewFormula.Validate();

    child->InsertView(this, &interior);
    child->FullUpdate(view_FullRedraw, interior.left, interior.top, interior.width, interior.top);
}
END_ATEXTV_CALLBACK_METHOD();

START_ATEXTV_CALLBACK_METHOD(BorderMethod) {
    struct rectangle bounds;
    GetLogicalBounds(&bounds);
    AShadow *asb=GetBorder();
    AFORMULA_BEGIN;
    if((boolean)(long)asb) {
        if((boolean)dself->scrolled) if(asb) asb->UseBorder(0);
        else if(asb) asb->UseBorder(AShadow_Plain);
    }
    AFORMULA_END;
    if(asb) {
        asb->DrawRectBorder(this, &bounds);
    }
}
END_ATEXTV_CALLBACK_METHOD();


START_ATEXTV_CALLBACK_METHOD(ScaleMethod) {
    if(GetBorder())
        GetBorder()->SetScale(
                         uxscale / ((double)dself->scaleWidth),
                         uyscale / ((double)dself->scaleHeight));
}
END_ATEXTV_CALLBACK_METHOD();

START_ATEXTV_CALLBACK_METHOD(KeyStateMethod) {
    dataViewFormula.Validate();
    if((long)dself->editMode==AText_SingleLine) {
        postKeyState=TRUE;
    } else postKeyState=FALSE;
}
END_ATEXTV_CALLBACK_METHOD();

START_ATEXTV_CALLBACK_METHOD(WantKeyStateMethod) {
    dataViewFormula.Validate();
    if(tv && HasInputFocus) tv->PostKeyState(NULL);
}
END_ATEXTV_CALLBACK_METHOD();

static ATextvMethodAct lact(&ATextv::TextMethod,0L);
static ATextvMethodAct bact(&ATextv::BorderMethod,0L);
static ATextvMethodAct sizeact(&ATextv::SizeMethod,0L);
static ATextvMethodAct updateact(&ATextv::WantUpdateMethod, 0L);
static ATextvMethodAct newsizeact(&ATextv::WantNewSizeMethod, 0L);
static ATextvMethodAct scaleact(&ATextv::ScaleMethod, 0L);
static ATextvMethodAct viewdataact(&ATextv::DataViewMethod, 0L);
static ATextvMethodAct keystateact(&ATextv::KeyStateMethod, 0L);
static ATextvMethodAct wantkeystateact(&ATextv::WantKeyStateMethod, 0L);

ATextv::ATextv() :
lformula(this, &lact, &updateact),
bformula(this, &bact, &updateact),
size(this, &sizeact, &newsizeact),
scaleFormula(this, &scaleact),
dataViewFormula(this, &viewdataact, &updateact),
keyStateFormula(this, &keystateact, &wantkeystateact),
stateFormula(this)
{
    ATKinit;
    tv=NULL;
    child=NULL;
    lastobs=NULL;
    keyState=keystate::Create(this, ATextvKeymap);
    mouseFocus=NULL;
    forcelink=0;
    swidth=sheight=-1;
    pass=view_NoSet;
}

ATextv::~ATextv() {
    if(lastobs) lastobs->RemoveObserver(this);
    if(mouseFocus) mouseFocus->RemoveObserver(this);
    delete keyState;
}


void ATextv::PostKeyState(keystate  *ks)
{
    AText *at=(AText *)GetDataObject();
    keyStateFormula.Validate();
    if(keyState && postKeyState) {
        keystate *pks=(keyState)->AddBefore( ks);
        (this)->AWidgetView::PostKeyState( pks);
    } else (this)->AWidgetView::PostKeyState( ks);
}

void ATextv::InitChildren() {
    if(forcelink>0) ForceLink();
    dataViewFormula.Validate();
}

void ATextv::LinkTree(class view *parent) {
    AWidgetView::LinkTree(parent);
    if(parent && parent->GetIM()) {
        ForceLink();
        dataViewFormula.Validate();
        stateFormula.Validate();
    } else {
        ForceLink();
        child->LinkTree(this);
    }
}


void ATextv::TextDimensions(long &w, long &h, long lrm, long tbm) {
    dataViewFormula.Validate();
    AText *b=(AText *)GetDataObject();
    struct rectangle exterior, interior;
    exterior.left=0;
    exterior.top=0;
    exterior.width=swidth;
    exterior.height=sheight;
    if(border) border->InteriorRect(&exterior, &interior);
    else interior=exterior;
    tv->SetBorder(lrm, tbm);
    tv->SetEmbeddedBorder(lrm, tbm);
    tv->SetDesiredRowsAndColumns((long)b->rows, (long)b->columns);
    if(tv) {
        child->DesiredSize(interior.width, interior.height, pass, &w, &h);
    }
}

void ATextv::FullUpdate(enum view_UpdateType type, long /* left */, long /* top */, long /* width */, long /* height */) {
    AText *b=(AText *)GetDataObject();
    struct rectangle bounds;
    GetLogicalBounds(&bounds);
    if(border) {
        border->InteriorRect(&bounds, &interior);
    } else interior=bounds;
    if(type==view_Remove) {
        child->FullUpdate(view_Remove, 0, 0, 0, 0);
        child->InsertViewSize(this, 0, 0, 0, 0);
        return;
    }
    if(type==view_MoveNoRedraw) {
        if(child) child->InsertView(this, &interior);
        return;
    }
    
    if(type==view_PartialRedraw) return;

    if(border) border->SetHighlighted(HasInputFocus);

    updateRequested=FALSE;
    bformula.Invalidate();
    lformula.Invalidate();
    lformula.Validate();
    bformula.Validate();
}

void ATextv::Update() {
    updateRequested=FALSE;
    lformula.Validate();
    bformula.Validate();
}

void ATextv::SetDataObject(class dataobject *d) {
    AWidgetView::SetDataObject(d);
    AText *ab=(AText *)d;
    if(border) {
        border->SetResources(ab);
        border->SetScaleFormula(&scaleFormula);
    }
    lformula.Invalidate();
    bformula.Invalidate();
    dataViewFormula.Invalidate();
}

view_DSattributes ATextv::DesiredSize(long  width, long  height, enum view_DSpass  p, long  *desired_width, long  *desired_height)
{
    boolean force=FALSE;
    if(swidth<0 || swidth!=width || sheight!=height || pass!=p) {
        force=TRUE;
        swidth=width;
        sheight=height;
        pass=p;
    }
    if(force) {
        // XXX sigh... hack so that we don't get an unwanted WantNewSize call... :-( robr
        unsigned char mode=size.Mode();
        size.SetMode(AFORMULAACCESS);
        size.Invalidate();
        size.SetMode(mode);
    }
    size.Validate();
    size.Desired(desired_width, desired_height);
    return view_Fixed;
}

void ATextv::ObservedChanged(class observable  *changed, long  status)  {
    AWidgetView::ObservedChanged(changed, status);
    if (status == observable_OBJECTDESTROYED) 
    {
        if(changed==lastobs) lastobs=NULL;
    }
    else {
        AText *at=(AText *)GetDataObject();
        if(changed==at->Source()) {
            aaction *doit = at->ValueChangedCallback();
            if (doit) {
                avalueflex foo;
                (*doit)(at, avalueflex()|this, foo);
            }
        }
    }
}

void ATextv::WantUpdate(view *req) {
    if(GetIM()) AWidgetView::WantUpdate(req);
}


void ATextv::GetOrigin(long  width, long height, long *originX, long *originY) {
    boolean force=FALSE;
    if(swidth!=width || sheight!=height) {
        force=TRUE;
        swidth=width;
        sheight=height;
    }
    if(force) {
        // XXX sigh... hack so that we don't get an unwanted WantNewSize call... :-( robr
        unsigned char mode=size.Mode();
        size.SetMode(AFORMULAACCESS);
        size.Invalidate();
        size.SetMode(mode);
    }
    size.Validate();
    size.Origin(originX, originY);
}

boolean ATextv::RecSearch(struct SearchPattern *pat, boolean ) {
    if(child) return child->RecSearch(pat, FALSE);
    return FALSE;
}

boolean ATextv::RecSrchResume(struct SearchPattern *pat) {
    if(child) return child->RecSrchResume(pat);
    return FALSE;
}

boolean ATextv::RecSrchReplace(class dataobject *txt, long pos, long srclen) {
    if(child) return child->RecSrchReplace(txt, pos, srclen);
    return FALSE;
}

void ATextv::RecSrchExpose(const struct rectangle &logical, struct rectangle &hit) {
    AText *b = (AText *)GetDataObject();
    if(GetIM()==NULL || GetDrawable()==NULL) return;
    struct rectangle interior;
    if(border) border->InteriorRect((rectangle *)&logical, &interior);
    else interior=logical;

    interior.left+=(long)((long)b->marginWidth * ((double)b->scaleWidth)/uxscale);
    
    interior.width-=(long)((long)b->marginWidth*2 * ((double)b->scaleHeight)/uyscale);
    
    interior.top+=(long)((long)b->marginHeight * ((double)b->scaleHeight)/uyscale);
    
    interior.height-=(long)((long)b->marginHeight*2 * ((double)b->scaleHeight)/uyscale);
    
    
    if(child) {
	child->RecSrchExpose(interior, hit);
	return;
    }
}


void ATextv::DesiredPrintSize(long width, long height, enum view_DSpass pass, long *desiredwidth, long *desiredheight) 
{
    AText *dobj = (AText *)(this->GetDataObject());
    dataViewFormula.Validate();
    if(child) {
        child->DesiredPrintSize(width, height, pass, desiredwidth, desiredheight);
    }
    
}

void *ATextv::GetPSPrintInterface(char *printtype)
{
    if (!strcmp(printtype, "generic")) {
	return (void *)this;
    }

    return NULL;

}

void ATextv::GetPrintOrigin(long w, long h, long *xoff, long *yoff) {
    dataViewFormula.Validate();
    AText *dobj = (AText *)(this->GetDataObject());
    if(child) {
        child->GetPrintOrigin(w, h, xoff, yoff);
    }
}

void ATextv::PrintPSRect(FILE *outfile, long logwidth, long logheight, struct rectangle *visrect)
{
    dataViewFormula.Validate();
    AText *dobj = (AText *)(this->GetDataObject());
    if(child) child->PrintPSRect(outfile, logwidth, logheight, visrect);
}

view *ATextv::GetApplicationLayer() {
    if(tv==NULL) return this;
    
    AText *at=(AText *)GetDataObject();

    if(at->Scrolled()) {
        long scrollpos=0;
        if(at->ScrollVertical()) {
            if(at->ScrollLeftSide()) scrollpos=scroll_LEFT;
            else scrollpos=scroll_RIGHT;
        }
        if(at->ScrollHorizontal()) {
            if(at->ScrollTopSide()) scrollpos|=scroll_TOP;
            else scrollpos|=scroll_BOTTOM;
        }

        if(child && tv && child!=tv) {
            tv->DeleteApplicationLayer(child);
            child=NULL;
        }

        if(scrollpos!=0) {
            child=scroll::CreateScroller(tv, scrollpos);
        }
    }
    if(child==NULL) child=tv;
    
    return this;
}

void ATextv::ReceiveInputFocus() {
    AText *at=(AText *)GetDataObject();

    aaction *doit = at->FocusCallback();
    if (doit) {
        avalueflex foo;
        (*doit)(at, avalueflex()|this, foo);
    }
    AWidgetView::ReceiveInputFocus();
}


void ATextv::LoseInputFocus() {
    AText *at=(AText *)GetDataObject();

    aaction *doit = at->LosingFocusCallback();
    if (doit) {
        avalueflex foo;
        (*doit)(at, avalueflex()|this, foo);
    }
    AWidgetView::LoseInputFocus();
}

/* 
 * $Log: atextv.C,v $
 * Revision 1.9  1996/11/12  00:27:20  robr
 * fixed atk_impl.
 *
 * Revision 1.8  1996/11/07  00:12:17  robr
 * Fixed to avoid calling WantUpdate when GetIM()==NULL.
 * (in particular this was happening while the tree was being unlinked.)
 * BUG
 *
 * Revision 1.7  1996/11/05  17:47:02  robr
 * Updated to use the 'password' resource and feature of textview
 * to prevent echoing of input.
 * FEATURE
 *
 * Revision 1.6  1996/10/29  00:03:18  robr
 * Fixed recsearch functions to always return a value.
 * Added casts on uses of ASlotBool in if statements.
 * Ensured that when the ATextv is destroyed it removes it's
 * observation status in the mousefocus and or source objects.
 * Initialized swidth, sheight,pass and forcelink with reasonable
 * initial values.
 * BUG
 *
 * Revision 1.5  1996/10/22  16:39:28  robr
 * Make sure to do an InsertView when we are removed from the screen.
 * BUG
 *
 * Revision 1.4  1996/10/17  21:44:51  robr
 * made some items protected for use by the list widget.
 * Fixed the topCharacterPosition resource so it will display the
 * line containing the requested character.
 * (instead of starting the display with that character.)
 * BUG
 *
 * Revision 1.3  1996/10/10  20:01:03  robr
 * Fixed so that mouse events are passed along properly.
 * (Have to deal with old substrates like scroll which aren't designed
 * to pass along all mouse events themselves. sigh.)
 * BUG
 *
 * Revision 1.2  1996/10/09  19:26:38  robr
 * Added support for wordWrap and font.
 * Added scrolling and callbacks support.
 * Added 'activate' keybinding for use in
 * singleline mode.
 * FEATURE
 *
 * Revision 1.1  1996/10/04  15:51:47  robr
 * Initial revision
  */
