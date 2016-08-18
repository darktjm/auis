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
ATK_IMPL("abuttonv.H")

#ifndef NORCSID
static UNUSED const char rcsid[] = "$Header: /afs/cs.cmu.edu/project/atk-src-C++/atk/widgets/lib/RCS/abuttonv.C,v 1.41 1996/11/07 00:12:17 robr Exp $";
#endif

#include <abuttonv.H>
#include <proctable.H>
#include <abutton.H>
#include <fontdesc.H>
#include <rect.h>
#include <simpletext.H>
#include <search.H>
#include <message.H>
#include <astring.H>
#include <textview.H>
#include <print.H>
#include <style.H>

static boolean Init();
ATKdefineRegistry(AButtonv, AWidgetView, Init);
ATKdefineRegistryNoInit(AButtonSizeFormula,AFormulaViewAct);

fontdesc *AButtonv::FindFont() {
    AButton *b=(AButton *)GetDataObject();
    fontdesc *fd=b->Font();
    fontdesc *dfd=b->designFont;
   
        // check the environments containing the abutton...
    style *result=(style *)WantInformation("style");
    if(result==NULL) {
        return fd;
    }   
    style *local=new style;
    local->SetFontFamily(fd->GetFontFamily());
    local->ClearOldFontFaces();
    local->ClearNewFontFaces();
    local->AddNewFontFace(fd->GetFontStyle());
    if(!(b->FontInheritanceFlags() & ABUTTON_INHERIT_FontFamily)) {
        result->SetFontFamily(NULL);
    }
    long newfaces=result->GetAddedFontFaces();
    newfaces&=b->FontInheritanceFlags();
    result->ClearNewFontFaces();
    result->AddNewFontFace(newfaces);
    long oldfaces=~result->GetRemovedFontFaces();
    oldfaces&=b->FontInheritanceFlags();
    result->ClearOldFontFaces();
    result->RemoveOldFontFace(oldfaces);
    result->MergeInto(local);
    long attrs = local->AddFontFaces & local->OutFontFaces;
    long fs=fd->GetFontSize();
    if((b->FontInheritanceFlags() & ABUTTON_INHERIT_FontSize) && dfd) {
        fs=(fs*result->FontSize.Operand) / dfd->GetFontSize();
    }
    fs=(long)(fs*(double)b->scaleHeight);
    // how and if to account for FontSize.SizeBasis?
    if(local->FontFamily) {
        fd=fontdesc::Create(local->FontFamily, attrs, fs);
        if(fd==NULL) fd=b->Font();
    }
    delete result;
    delete local;
    return fd;
}


void AButtonv::Init() {
    ATKinit;
}
// see abuttonv.H for the macros used below...
START_ABUTTONV_MOUSE_METHOD(Arm) {
    if((!Armed())!=(!within)) {
	SetArmed(within);
    }
    if((action==view_LeftDown || action==view_RightDown) && HasInputFocus) WantInputFocus(this);
}
END_ABUTTONV_MOUSE_METHOD();

START_ABUTTON_MOUSE_FUNC(Disarm) {
    vself->SetArmed(FALSE);
}
END_ABUTTON_MOUSE_FUNC();

START_ABUTTON_MOUSE_FUNC(Activate) {
    Disarm(self, aux, in, out);
    if(within) {
        // call activateCallback, if any
        if(vself->Label()) {
            view *label=vself->Label();
            aaction *doit = self->LabelActivateCallback();
            if(doit) {
                rectangle r, interior;
                vself->GetLogicalBounds( &r);
                if(vself->GetBorder()) vself->GetBorder()->InteriorRect(&r,&interior);
                else interior=r;
                long vx=vself->ToPixX(x)+interior.left;
                long vy=vself->ToPixY(y)+interior.top;
                vx=label->EnclosedXToLocalX(vx);
                vy=label->EnclosedYToLocalY(vy);
                avalueflex foo;
                (*doit)(self,
                        avalueflex()|
                        vself|
                        vx|vy, foo);
                return;
            }
        }
	aaction *doit = self->ActivateCallback();
	if (doit) {
	    avalueflex foo;
	    (*doit)(self, avalueflex()|vself, foo);
	}
    }
}
END_ABUTTON_MOUSE_FUNC();

START_ABUTTON_CALLBACK_FUNC(Toggle) {
    self->SetSet(!self->Set());
}
END_ABUTTON_CALLBACK_FUNC();

ASlot_NAME(sibling);
ASlot_NAME(set);
START_ABUTTON_CALLBACK_FUNC(RadioToggle) {
    if(self->Set()) return;
    self->SetSet(TRUE);
    AWidget *wgt=(AWidget *)self->GetATK(slot_sibling, class_AButton);
    while(wgt!=self && wgt) {
        wgt->Set(slot_set,abool(FALSE));
        wgt=(AWidget *)wgt->GetATK(slot_sibling, class_AButton);
    }
}
END_ABUTTON_CALLBACK_FUNC();

static boolean Init() {
    ENTER_AACTION_METHOD(Arm, AButtonv, "abutton-arm", 0L, "Arms or disarms the button, according to the mouse position.");
    ENTER_AACTION_FUNC(Disarm, "abutton-disarm", 0L, "Disarms the button.");
    ENTER_AACTION_FUNC(Activate, "abutton-activate", 0L, "Disarms the button, then executes the activate callback");
    ENTER_AACTION_FUNC(Toggle, "abutton-toggle", 0L, "Toggles the 'set' state of the button.");
    ENTER_AACTION_FUNC(RadioToggle, "abutton-radiotoggle", 0L, "Toggles the 'set' state of the button, ensures only one of the radio group buttons is 'set' at any one time.");
    return TRUE;
}

#define TEXTINMIDDLE (graphic_BETWEENLEFTANDRIGHT | graphic_BETWEENTOPANDBOTTOM)
#define AButtonv_IndicatorBorderType AShadow_Highlight|AShadow_Default
#define AButtonv_BorderType AShadow_Highlight|AShadow_Default|AShadow_Plain

START_ABUTTONV_CALLBACK_METHOD(WantUpdateMethod) {
    WantUpdate(this);
}
END_ABUTTONV_CALLBACK_METHOD();

START_ABUTTONV_CALLBACK_METHOD(SizeMethod) {
    long o_x, o_y, o_w, o_h;
    size.Desired(&o_w, &o_h);
    size.Origin(&o_x, &o_y);
    CheckIndicator();
    layout.Validate();
    AButton *b = (AButton *)GetDataObject();
    struct rectangle linterior, exterior;
    long below, oy;
    linterior.left=0;
    linterior.top=0;
    linterior.width=0;
    linterior.height=0;
    LabelDimensions(linterior.width, linterior.height, &below);
    if(indicator) {
        long iw;
        char ic=*(const char *)dself->indicatorType;
        if(ic=='p' || ic=='P') iw=linterior.height*3/2;
        else iw=linterior.height;
	linterior.width+=(long)(iw+((long)b->spacing) * ((double)b->scaleWidth)/uxscale);
    }
    if(border) border->ExteriorRect(&linterior, &exterior);
    else exterior=linterior;
    long d_w, d_h;
    
    size.SetDesired(d_w=(long)(exterior.width + ((long)b->marginWidth*2 +
		    (long)b->marginLeft +
		    (long)b->marginRight) * ((double)b->scaleWidth)/uxscale),
		   d_h = (long)(exterior.height + ((long)b->marginHeight*2 +
		    (long)b->marginTop + (long)b->marginBottom) * ((double)b->scaleHeight)/uyscale));
    if(border) {
        long dtx, ty;
        border->TotalLowerBorderWidthHeight(dtx, ty);
        ty=(long)(ty*((double)b->scaleHeight));
	oy=exterior.height + (long)(((long)b->marginTop + (long)b->marginHeight) * ((double)b->scaleHeight)/uyscale) - ty - below;
    } else oy=(long)(interior.height + ((long)b->marginTop + (long)b->marginHeight) * ((double)b->scaleHeight)/uyscale - below);
    size.SetOrigin(0, oy);
    if(o_w!=-1 && (o_y!=oy || d_w!=o_w || d_h!=o_h)) WantNewSize(this);
}
END_ABUTTONV_CALLBACK_METHOD();

START_ABUTTONV_CALLBACK_METHOD(LayoutMethod) {
    if((long)dself->indicatorOn) {
        char ic=*(const char *)dself->indicatorType;
        boolean option=(ic=='p' || ic=='P');
	if(!indicator) {
            if(border) border->UseBorder(option ? (long)dself->borderType:AButtonv_IndicatorBorderType);
	    indicator=new AShadow(this,dself);
	    if(indicator) indicator->UseBorder(AShadow_Plain);
	    if(border) border->SetIndicator(FALSE);
	}
    } else {
        if(border) {
            border->UseBorder((long)dself->borderType);
            if(indicator)
                border->SetIndicator(TRUE);
        }
	delete indicator;
	indicator=NULL;
    }
}
END_ABUTTONV_CALLBACK_METHOD();

START_ABUTTONV_CALLBACK_METHOD(LabelMethod) {
    layout.Validate();
    // should use a formula to set vself->interior
    struct rectangle bounds;
    GetLogicalBounds(&bounds);
    if(border) {
	border->InteriorRect(&bounds, &interior);
    } else interior=bounds;
    CheckLabelView();
    if(!dself->LabelData() || indicator) {
	// do this code only if the label is not an inset, or if the label
	// is an inset and an indicator is present.
	// known bug: due to this next bit of code, this gets called any time
	// the arming state has been changed even if only the indicator is
	// is showing selected/armed status.  (NEW: only happens if this code is ever run)
	if(GetBorder()) {
	    if(!indicator) GetBorder()->SetArmed(armed);
	    SetForegroundColor(&GetBorder()->Background());
	}
	FillRect(&interior, NULL);
    }
    struct rectangle linterior=interior;
    linterior.left+=(long)((long)dself->MarginWidth() * ((double)dself->scaleWidth)/uxscale);
    
    linterior.width-=(long)((long)dself->MarginWidth()*2 * ((double)dself->scaleWidth)/uxscale);
    
    linterior.top+=(long)((long)dself->MarginHeight() * ((double)dself->scaleHeight)/uyscale);
    
    linterior.height-=(long)((long)dself->MarginHeight()*2 * ((double)dself->scaleHeight)/uyscale);
    
    linterior.left+=(long)((long)dself->MarginLeft() * ((double)dself->scaleWidth)/uxscale);
    
    linterior.width-=(long)((long)dself->MarginRight() * ((double)dself->scaleWidth)/uxscale);
    
    linterior.top+=(long)((long)dself->MarginTop() * ((double)dself->scaleHeight)/uyscale);
    
    linterior.height-=(long)((long)dself->MarginBottom() * ((double)dself->scaleHeight)/uyscale);

    if(indicator) {
	long w=0, h=0;
	LabelDimensions(w, h);
	if(h>linterior.height) h=linterior.height;
        char ic=*(const char *)dself->indicatorType;
        boolean onright=(ic=='p' || ic=='P');
        if(onright) {
            struct rectangle grn;
            grn.left=grn.width=grn.top=0;
            grn.height=3;
            struct rectangle ir;
            indicator->ExteriorRect(&grn, &ir);
            h=ir.height;
            erect.left=linterior.left+linterior.width-h*2;
            erect.width=h*2;
            erect.height=h;
            erect.top=linterior.top+linterior.height/2-h/2;
            linterior.width-=h*2+(long) ((long)dself->Spacing() * ((double)dself->scaleWidth)/uxscale);
        } else {
            long added=h+(long)((long)dself->Spacing() * ((double)dself->scaleWidth)/uxscale);
            erect.left=linterior.left;
            erect.width=h;
            erect.height=h;
            erect.top=linterior.top+linterior.height/2-h/2;

            linterior.left+=added;
            linterior.width-=added;
        }
    }
    if(dself->LabelData()) {
	if(label) {
	    label->InsertView(this, &linterior);
	    label->FullUpdate(view_FullRedraw, linterior.left, linterior.top, linterior.width, linterior.top);
	}
    } else {
	ASlot_NAME(labelFG);
	if(GetBorder()) SetForegroundColor(&GetBorder()->LabelFG());
	else {
	    static color dlfg("black");
	    color &col=(color &)dself->Get(slot_labelFG,dlfg);
	    SetForegroundColor(&col);
        }
        fontdesc *fd=FindFont();
	SetFont(fd);
	DrawLabelText(fd, &linterior, dself->Text(), strlen(dself->Text()), TEXTINMIDDLE);
    }
}
END_ABUTTONV_CALLBACK_METHOD();

START_ABUTTONV_CALLBACK_METHOD(BorderMethod) {
    layout.Validate();
    struct rectangle bounds;
    GetLogicalBounds(&bounds);
    if(indicator) {
    } else {
	if(GetBorder()) {
	    GetBorder()->SetArmed(armed);
	}
    }
    if(GetBorder()) GetBorder()->DrawRectBorder(this, &bounds);
}
END_ABUTTONV_CALLBACK_METHOD();

START_ABUTTONV_CALLBACK_METHOD(IndicatorMethod) {
    lformula.Validate();
    if(indicator) {
	indicator->SetArmed(armed);
        char it=*(const char *)dself->IndicatorType();
        switch(it) {
            case 'N':
            case 'n':
            case 'P':
            case 'p':
                indicator->DrawRectBorder(this, &erect);
                indicator->FillRect(this, &erect);
                break;
            case 'O':
            case 'o':
                indicator->DrawDiamondBorder(this, &erect);
                indicator->FillDiamond(this, &erect);
                break;
	}
    }
}
END_ABUTTONV_CALLBACK_METHOD();

START_ABUTTONV_CALLBACK_METHOD(ScaleMethod) {
    layout.Validate();
    if(border)
        border->SetScale(
                         uxscale / ((double)dself->scaleWidth),
                         uyscale / ((double)dself->scaleHeight));
    if(indicator)
        indicator->SetScale(
                         uxscale / ((double)dself->scaleWidth),
                         uyscale / ((double)dself->scaleHeight));
}
END_ABUTTONV_CALLBACK_METHOD();
                                             
static AButtonvMethodAct lact(&AButtonv::LabelMethod,0L);
static AButtonvMethodAct bact(&AButtonv::BorderMethod,0L);
static AButtonvMethodAct iact(&AButtonv::IndicatorMethod,0L);
static AButtonvMethodAct layoutact(&AButtonv::LayoutMethod,0L);
static AButtonvMethodAct sizeact(&AButtonv::SizeMethod,0L);
static AButtonvMethodAct updateact(&AButtonv::WantUpdateMethod, 0L);
static AButtonvMethodAct scaleact(&AButtonv::ScaleMethod, 0L);

AButtonv::AButtonv() :
lformula(this, &lact, &updateact),
bformula(this, &bact, &updateact),
iformula(this, &iact, &updateact),
layout(this, &layoutact, &updateact),
size(this, &sizeact),
scaleFormula(this, &scaleact)
{
    ATKinit;
    highlighted=FALSE;
    armed=FALSE;
    indicator=NULL;
    label=NULL;
    rsvalid=FALSE;
    rsstart=0;
    rslen=0;
    layout.SetMode(AFORMULAACCESS);
    lastfont=NULL;
    swidth=sheight=-1;
    pass=view_NoSet;
    size.SetMode(AFORMULAIMMEDIATE);
}

AButtonv::~AButtonv() {
    delete indicator;
    if(label) label->Destroy();
}

void AButtonv::CheckIndicator(boolean force) {
    layout.Validate();
}


static view *NewView(const char *c) {
    ATK_CLASS(view);
    ATKregistryEntry *ent=ATK::LoadClass(c);
    if(ent->IsType(class_view)) return (view *)ATK::NewObject(c);
    return NULL;
}

void AButtonv::CheckLabelView(boolean link) {
    AButton *ab=(AButton *)GetDataObject();
    const char *vc=ab->labelViewClass;
    if(ab->LabelData()==NULL) return;
    if(label!=NULL) return;
    
    if(*vc!='\0') label=NewView(vc);
    if(label==NULL) {
	vc=ab->LabelData()->ViewName();
	label=NewView(vc);
    }
    if(label) {
	label->SetDataObject(ab->LabelData());
	if(link) label->LinkTree(this);
    }
}

void AButtonv::LinkTree(class view *parent) {
    AWidgetView::LinkTree(parent);
    if(parent && parent->GetIM()) {
        CheckLabelView(FALSE);
    }
    if(label) label->LinkTree(this);
}


void AButtonv::DoSelectionHighlight(char *textl, class fontdesc *my_fontdesc, long tx, const struct rectangle *r, struct rectangle *hit, boolean draw)
{
    if(HasInputFocus && my_fontdesc && rsvalid && GetIM()) {
	int width, height;
	int swidth, sheight;
	long start=rsstart;
	long slen=rslen;
	char k;
	my_fontdesc->StringBoundingBox(GetDrawable(), textl, &width, &height);
	tx-=width/2;
	k=textl[start];
	textl[start]='\0';
	my_fontdesc->StringBoundingBox(GetDrawable(), textl, &swidth, &sheight);
	textl[start]=k;
	tx+=swidth;
	width-=swidth;
	my_fontdesc->StringBoundingBox(GetDrawable(), textl+start+slen, &swidth, &sheight);
	width-=swidth;
	if(hit) {
	    hit->top=r->top;
	    hit->left=tx;
	    hit->width=width;
	    hit->height=r->height;
	}
	if(draw) {
	    SetTransferMode(graphic_INVERT);
	    FillRectSize(tx, r->top, width, r->height, NULL);
	    SetTransferMode(graphic_COPY);
	}
    }
}

void AButtonv::DrawLabelText(class fontdesc  *font, const struct rectangle *interior, const char  *text, int  len, long  flags)
{
    long  x=interior->left + interior->width / 2, y=interior->top + interior->height /2;
    if(flags==TEXTINMIDDLE && len==1 && font!=NULL) {
	long tx, ty;
	struct fontdesc_charInfo ci;
	(font)->CharSummary( GetDrawable(), *text, &ci);
	tx = ci.width + 4;
	ty = ci.height + 4;
	MoveTo( x - tx/2 + ci.xOriginOffset + 2, y - ty/2 + ci.yOriginOffset +2);
	DrawText( (char *)text, len, 0);
	if(HasInputFocus && rsvalid && rsstart==0 && rslen==1) {
	    SetTransferMode(graphic_INVERT);
	    FillRectSize(x - tx/2 + ci.xOriginOffset, y - ty/2 + ci.yOriginOffset, tx+4, ty+4, NULL);
	    SetTransferMode(graphic_COPY);
	}
    } else {
	MoveTo( x, y);
	DrawText( (char *)text, len, (short)flags);
	AString t(text);
	DoSelectionHighlight(t, font, x, interior, NULL, TRUE);
    }
}   

void AButtonv::LabelDimensions(long &w, long &h, long *below) {
    CheckLabelView();
    AButton *b=(AButton *)GetDataObject();
    if(label==NULL) {
	fontdesc *font=FindFont();
	const char *txt=b->Text();
	graphic *my_graphic=GetDrawable();
	struct fontdesc_charInfo ci;
	if(*txt && txt[1]=='\0') {
	    (font)->CharSummary( my_graphic, *txt, &ci);
            w=ci.width+(long)(4 * ((double)b->scaleWidth)/uxscale);
	    h=ci.height+(long)(4 * ((double)b->scaleHeight)/uyscale);
	} else {
	    struct FontSummary *my_FontSummary;
            (font)->StringSize( my_graphic, (char *)txt, &w, &h);
	    my_FontSummary =  (font)->FontSummary( my_graphic);
	    if(my_FontSummary) {
		h=my_FontSummary->maxHeight;
		if(below) *below=my_FontSummary->maxBelow;
	    }
	}
    } else {
	if(below) *below=0;
	struct rectangle exterior, interior;
	exterior.left=0;
	exterior.top=0;
	exterior.width=swidth;
	exterior.height=sheight;
	if(border) border->InteriorRect(&exterior, &interior);
	else interior=exterior;
	interior.width-=(long)(((long)b->marginWidth*2 +
	  (long)b->marginLeft +
	  (long)b->marginRight) * ((double)b->scaleWidth)/uxscale);
	interior.height-=(long)(((long)b->marginHeight*2 +
	  (long)b->marginTop +
	  (long)b->marginBottom) * ((double)b->scaleHeight)/uyscale);
	view_DSattributes result=label->DesiredSize(interior.width, interior.height, pass, &w, &h);
    }
}

void AButtonv::FullUpdate(enum view_UpdateType type, long /* left */, long /* top */, long /* width */, long /* height */) {
    CheckIndicator();
    AButton *b=(AButton *)GetDataObject();
    boolean drawing=TRUE;
    struct rectangle bounds;
    GetLogicalBounds(&bounds);
    fontdesc *fd=FindFont();
    if(lastfont!=NULL && fd!=lastfont) {
        size.Invalidate();
    }
    lastfont=fd;
    if(border) {
	border->InteriorRect(&bounds, &interior);
    } else interior=bounds;
    if(b->LabelData()) {
	CheckLabelView();
	if(label) {
	    if(type==view_Remove) {
                label->FullUpdate(view_Remove, 0, 0, 0, 0);
                label->InsertViewSize(this, 0, 0, 0, 0);
		return;
	    }
	    if(type==view_MoveNoRedraw) {
                struct rectangle linterior=interior;
                
                linterior.left+= (long)((long)b->MarginWidth() * ((double)b->scaleWidth)/uxscale);
                
                linterior.width-= (long)((long)b->MarginWidth()*2 * ((double)b->scaleWidth)/uxscale);
                
                linterior.top+= (long) ((long)b->MarginHeight() * ((double)b->scaleHeight)/uyscale);
                
                linterior.height-= (long) ((long)b->MarginHeight()*2 * ((double)b->scaleHeight)/uyscale);
                
                linterior.left+= (long) ((long)b->MarginLeft() * ((double)b->scaleWidth)/uxscale);
                
                linterior.width-= (long) ((long)b->MarginRight() * ((double)b->scaleWidth)/uxscale);
                
                linterior.top+= (long) ((long)b->MarginTop() * ((double)b->scaleHeight)/uyscale);
                
                linterior.height-= (long) ((long)b->MarginBottom() * ((double)b->scaleHeight)/uyscale);
                

                if(indicator) {
                    long w=0, h=0;
                    LabelDimensions(w, h);
                    if(h>linterior.height) h=linterior.height;
                    char ic=*(const char *)b->indicatorType;
                    boolean onright=(ic=='p' || ic=='P');
                    if(onright) {
                        struct rectangle grn;
                        grn.left=grn.width= grn.top=grn.height=0;
                        struct rectangle ir;
                        indicator->ExteriorRect(&grn, &ir);
                        linterior.width-=ir.height*2+(long) ((long)b->Spacing() * ((double)b->scaleWidth)/uxscale);
                    } else {
                        long added=h+(long)((long)b->Spacing() * ((double)b->scaleWidth)/uxscale);
                        linterior.left+=added;
                        linterior.width-=added;
                    }
                }
                label->InsertView(this, &linterior);
            }
	}
    }
    
    if(type==view_PartialRedraw || type==view_Remove || type==view_MoveNoRedraw) return;

    if(border) border->SetHighlighted(HasInputFocus);

    if(drawing) {
        // prevent any WantUpdate requests
	updateRequested=TRUE;
	bformula.Invalidate();
	lformula.Invalidate();
        iformula.Invalidate();
        // re-instate WantUpdate passing
        updateRequested=FALSE;
	lformula.Validate();
	iformula.Validate();
        bformula.Validate();
    }
}

void AButtonv::Update() {
    updateRequested=FALSE;
    // FullUpdate(view_FullRedraw, 0, 0, 0, 0);
    lformula.Validate();
    iformula.Validate();
    bformula.Validate();
}

void AButtonv::SetDataObject(class dataobject *d) {
    AWidgetView::SetDataObject(d);
    AButton *ab=(AButton *)d;
    delete indicator;
    indicator=NULL;
    if(border) {
        border->SetResources(ab);
        border->SetScaleFormula(&scaleFormula);
    }
    lformula.Invalidate();
    bformula.Invalidate();
    iformula.Invalidate();
    layout.Invalidate();
}

view_DSattributes AButtonv::DesiredSize(long  width, long  height, enum view_DSpass  p, long  *desired_width, long  *desired_height)
{
    boolean force=FALSE;
    fontdesc *fd=FindFont();
    if(lastfont!=NULL && fd!=lastfont) {
       force=TRUE;
    }
    lastfont=fd;
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


void AButtonv::GetOrigin(long  width, long height, long *originX, long *originY) {
    fontdesc *fd=FindFont();
    if(lastfont!=NULL && fd!=lastfont) {
        // XXX sigh... hack so that we don't get an unwanted WantNewSize call... :-( robr
        unsigned char mode=size.Mode();
        size.SetMode(AFORMULAACCESS);
        size.Invalidate();
        size.SetMode(mode);
    }
    lastfont=fd;
    size.Validate();
    size.Origin(originX, originY);
}

boolean AButtonv::RecSearch(struct SearchPattern *pat, boolean ) {
    if(label) return label->RecSearch(pat, FALSE);
    AButton *b=(AButton *)GetDataObject();
    if(b==NULL) {
	rsvalid=FALSE;
	return FALSE;
    }
    long start=search::MatchPatternStr((unsigned char *)b->Text(), 0, strlen(b->text), pat);
    if(start>=0) {
	rsvalid=TRUE;
	rsstart=start;
	rslen=search::GetMatchLength();
	return TRUE;
    }
    rsvalid=FALSE;
    return FALSE;
}

boolean AButtonv::RecSrchResume(struct SearchPattern *pat) {
    if(rsvalid) {
	if(label) return label->RecSrchResume(pat);
	AButton *b=(AButton *)GetDataObject();
	if(b==NULL) {
	    rsvalid=FALSE;
	    return FALSE;
	}
	rsstart++;
	rslen=0;
	long len=strlen(b->text);
	long start;
	if(len-rsstart>0) {
	    start=search::MatchPatternStr((unsigned char *)b->Text(), rsstart, len-rsstart, pat);
	    if(start>=0) {
		rsvalid=TRUE;
		rsstart=start;
		rslen=search::GetMatchLength();
		return TRUE;
	    }
	}
	rsvalid=FALSE;
	this->WantUpdate(this);
    }
    return FALSE;
}
boolean AButtonv::RecSrchReplace(class dataobject *txt, long pos, long srclen) {
    ATK_CLASS(simpletext);
    if(!rsvalid) return FALSE;
    if(label) return label->RecSrchReplace(txt, pos, srclen);

    AButton *b=(AButton *)GetDataObject();
    if(b==NULL) {
	rsvalid=FALSE;
	return FALSE;
    }
    char *buf;
    const char *ts;
    long start, len;
    simpletext *srctext;

    if(!txt->IsType(class_simpletext)) {
	message::DisplayString(this, 0, "Replacement object is not text.");
	return FALSE;
    }
    srctext=(simpletext *)txt;
    ts=b->text;
    start=rsstart;
    len=rslen;
    buf=(char *)malloc(strlen(ts)-rslen+srclen+2);
    strncpy(buf,ts,(size_t)rsstart);
    srctext->CopySubString(pos, srclen, buf+rsstart, FALSE);
    strcpy(buf + rsstart + srclen, ts + rsstart + rslen);
    rslen=srclen;
    b->SetText(buf);
    free(buf);
    return TRUE;
}

void AButtonv::RecSrchExpose(const struct rectangle &logical, struct rectangle &hit) {
    AButton *b = (AButton *)GetDataObject();
    if(GetIM()==NULL || GetDrawable()==NULL) return;
    struct rectangle interior;
    if(border) border->InteriorRect((rectangle *)&logical, &interior);
    else interior=logical;

    interior.left+=(long)((long)b->marginWidth * ((double)b->scaleWidth)/uxscale);
    
    interior.width-=(long)((long)b->marginWidth*2 * ((double)b->scaleHeight)/uyscale);
    
    interior.top+=(long)((long)b->marginHeight * ((double)b->scaleHeight)/uyscale);
    
    interior.height-=(long)((long)b->marginHeight*2 * ((double)b->scaleHeight)/uyscale);
    
    interior.left+=(long)((long)b->marginLeft * ((double)b->scaleWidth)/uxscale);
    
    interior.width-=(long)((long)b->marginRight * ((double)b->scaleWidth)/uxscale);
    
    interior.top+=(long)((long)b->marginTop * ((double)b->scaleHeight)/uyscale);
    
    interior.height-=(long)((long)b->marginBottom * ((double)b->scaleHeight)/uyscale);
    
    if(indicator) {
	long w=0, h=0;
	LabelDimensions(w, h);
	long added=h+(long)((long)b->spacing * ((double)b->scaleWidth)/uxscale);
	interior.left+=added;
	interior.width-=added;
    }
    if(label) {
	label->RecSrchExpose(interior, hit);
	return;
    }
    AString t(b->Text());
    DoSelectionHighlight(t, FindFont(), interior.left+interior.width/2, &interior, &hit, FALSE);
    this->WantInputFocus(this);
    this->WantUpdate(this);
}


#define FONT "andysans"
#define FONTTYPE fontdesc_Bold
#define FONTSIZE 12

void AButtonv::DesiredPrintSize(long width, long height, enum view_DSpass pass, long *desiredwidth, long *desiredheight) 
{
    AButton *dobj = (AButton *)(this->GetDataObject());
    CheckIndicator();
    CheckLabelView(FALSE);
    if(label) {
	label->DesiredPrintSize(width, height, pass, desiredwidth, desiredheight);
	if(indicator) {
	    *desiredwidth+=*desiredheight+(long)dobj->spacing;
	}
    } else {
	fontdesc *fd = FindFont();
	double h=fd->GetFontSize()*1.1667;

	short *encoding=NULL;
	char font[64];
	print::LookUpPSFont(font, &encoding, fd, NULL, 0, 0);
	font_afm *afm = print::GetPSFontAFM(font);
	const char *tx=dobj->Text();
	double w=0;
	while(*tx) {
	    w+=print::ComputePSCharWidth(encoding[(unsigned char)*tx], afm, (int)fd->GetFontSize());
	    tx++;
	}
	if(indicator) {
	    w+=h+(long)dobj->spacing;
	}
	*desiredwidth=(long)(w+0.5);
	*desiredheight=(long)(h+0.5);
    }
}

void *AButtonv::GetPSPrintInterface(char *printtype)
{
    if (!strcmp(printtype, "generic")) {
	return (void *)this;
    }

    return NULL;

}

void AButtonv::GetPrintOrigin(long w, long h, long *xoff, long *yoff) {
    CheckLabelView(FALSE);
    CheckIndicator();
    AButton *dobj = (AButton *)(this->GetDataObject());
    if(label) {
	    label->GetPrintOrigin(w, h, xoff, yoff);
    } else {
	fontdesc *fd = FindFont();
	char *ffam = FONT;
	long fsiz = FONTSIZE;
	long fsty = FONTTYPE;
	char fontname[256];
	short *encoding;

	if (fd) {
	    ffam = (fd)->GetFontFamily();
	    fsiz = (fd)->GetFontSize();
	    fsty = (fd)->GetFontStyle();
	}
	print::LookUpPSFont(fontname, &encoding, NULL, ffam, fsiz, fsty);
	font_afm *afm= print::GetPSFontAFM(fontname);
	if(afm==NULL) return;
        *yoff=(long)(afm->fontbbox_t/1000.0*fsiz*1.1667);
	*xoff=0;
    }
}

void AButtonv::PrintPSRect(FILE *outfile, long logwidth, long logheight, struct rectangle *)
{
    CheckLabelView(FALSE);
    CheckIndicator();
    AButton *dobj = (AButton *)(this->GetDataObject());
    fontdesc *fd = FindFont();
    char *ffam = FONT;
    long fsiz = FONTSIZE;
    long fsty = FONTTYPE;
    char fontname[256];
    short *encoding;
    int val;

    if (fd) {
	ffam = (fd)->GetFontFamily();
	fsiz = (fd)->GetFontSize();
	fsty = (fd)->GetFontStyle();
    }
    
    print::LookUpPSFont(fontname, &encoding, NULL, ffam, fsiz, fsty);
    val = print::PSRegisterFont(fontname);

    font_afm *afm= print::GetPSFontAFM(fontname);
    if(afm==NULL) return;
    long left=0;
    if(indicator) {
	double foo;
	long diamond;
	if(label) {
	    long xoff=0, yoff=0;
	    label->GetPrintOrigin(logwidth, logheight, &xoff, &yoff);
	    if(yoff>0) foo=yoff;
	    else foo=logheight;
	} else {
	    foo=(afm->fontbbox_t/1000.0*fsiz);
	}
	diamond=(long)(foo*.8);
	fprintf(outfile, "newpath %ld %ld moveto\n", logheight/2-diamond/2, logheight-diamond/2-(long)((foo-diamond)/2)-((long)(logheight-foo)));
	fprintf(outfile, "%ld %ld rlineto\n", diamond/2, -diamond/2);
	fprintf(outfile, "%ld %ld rlineto\n", diamond/2, diamond/2);
	fprintf(outfile, "%ld %ld rlineto\n", -diamond/2, diamond/2);
	fprintf(outfile, "%ld %ld rlineto\n", -diamond/2, -diamond/2);
	if(dobj->Set()) fprintf(outfile, "fill\n");
	else fprintf(outfile, "stroke\n");
	left=logheight+(long)dobj->spacing;
    }
    if(label) {
	struct rectangle visrect;
	visrect.left=0;
	visrect.width=logwidth;
	visrect.top=logheight;
	visrect.height=logheight;
	if(indicator) {
	    visrect.left+=logheight;
	    visrect.width-=logheight;
	}
	fprintf(outfile, "gsave\n");
	fprintf(outfile, "%ld %d translate\n", logheight, 0);
	fprintf(outfile, "0 0 moveto %ld 0 rlineto 0 %ld rlineto -%ld 0 rlineto closepath clip newpath\n",  logwidth, logheight, logwidth);
	label->PrintPSRect(outfile, visrect.width, visrect.height, &visrect);
	fprintf(outfile, "grestore\n");
    } else {
	fprintf(outfile, "%s%d %ld scalefont setfont\n", print_PSFontNameID, val, fsiz);
	fprintf(outfile, "%ld %ld moveto\n", left, (logheight-(long)(afm->fontbbox_t/1000.0*fsiz*1.1667)));	
	fprintf(outfile, "(");	
	print::OutputPSString(outfile, (char *)(dobj)->Text(), -1);
	fprintf(outfile, ")\n");
	fprintf(outfile, "show\n");
    }
}

/* 
 * $Log: abuttonv.C,v $
 * Revision 1.41  1996/11/07  00:12:17  robr
 * Fixed to avoid calling WantUpdate when GetIM()==NULL.
 * (in particular this was happening while the tree was being unlinked.)
 * BUG
 *
 * Revision 1.40  1996/11/04  23:00:23  robr
 * Added support for the labelActivateCallback.
 * FEATURE
 *
 * Revision 1.39  1996/11/03  20:27:32  robr
 * made FindFont a virtual protected member function so it can be
 * used by derived classes.
 * Removed unnecessary WantUpdate and ObservedChanged methods.
 * FEATURE
 *
 * Revision 1.38  1996/11/02  20:43:46  robr
 * Fixed to avoid disabling WantUpdate passing,and to avoid
 * excessive calls to WantNewSize.
 * BUG
 *
 * Revision 1.37  1996/10/29  21:05:57  robr
 * Fixed uninitialized memory references.
 * BUG
 *
 * Revision 1.36  1996/10/28  17:49:16  robr
 * Added the new indictaorType 'P' for Popup.  It places
 * the indictaor to the right of the label and makes it
 * rectangular.
 * FEATURE
 *
 * Revision 1.35  1996/10/22  16:50:27  robr
 * FIxed to call InsertView on the child view if any when being
 * removed from the screen.
 * BUG
 *
 * Revision 1.34  1996/10/15  00:31:28  robr
 * Removed debugging code, added abutton-radiotoggle to ensure the one-set
 * strategy of radio buttons.
 * BUG
 * (uh, FEATURE)
 *
 * Revision 1.33  1996/10/01  20:25:08  robr
 * Fixed to set the scaling factor for the indicator, also fixed to use the new
 * TotalLowerBorderWidthHeight function in computing the origin.
 * (needed to account for scaling & units)
 * BUG
 *
 * Revision 1.32  1996/09/25  19:34:45  robr
 *  Fixed to use the new AFormulaViewAct formula class.
 * Regularized the method names.
 * BUG
 *
 * Revision 1.31  1996/09/25  15:28:29  robr
 * Suppress scaling by the surrounding text if the designFont is not set.
 * BUG
 *
 * Revision 1.30  1996/09/24  20:17:05  robr
 * Fixed to do font family, and face inheritance as well as size.
 * BUG
 *
 * Revision 1.29  1996/09/24  18:51:42  robr
 * Added support for scaling.
 * FEATURE
 *
 * Revision 1.28  1996/09/10  20:43:14  robr
 * Moved some code into AFormulaAct, the first small
 * step toward integrating all the convenience functions
 * used into the base widget set support.
 * BUG
 *
 * Revision 1.27  1996/09/09  19:35:36  robr
 * Lobotomized FindFont.  It now implements only size inheritance, and
 * that inheritance sets the actual font used to the request font
 * size scaled by the ratio of the inherited font size and the design font
 * size.
 * BUG
 *
 * Revision 1.26  1996/09/05  19:03:02  robr
 * Added support for the surrounding style to augment the
 * font set by the resources.
 * Added resource fontInheritanceFlags to control which
 * aspects of the font selection the style can influence.
 * FEATURE
 *
 * Revision 1.25  1996/09/05  17:27:17  robr
 * Fixed so that changes affecting where the origin should
 * be placed will cause a screen update automatically.
 * BUG
 *
 * Revision 1.24  1996/09/05  15:52:08  robr
 * Updated to support inheriting the font used from an enclosing text object.
 * FEATURE
 *
 * Revision 1.23  1996/06/26  19:50:51  robr
 * Added automatic resizing in response to resource changes.
 * FEATURE
 *
 * Revision 1.22  1996/06/25  18:05:36  robr
 * Fixed to avoid segfault when no border is defined.
 * BUG
 *
 * Revision 1.21  1996/06/25  15:58:08  robr
 * Simplified to avoid another HP C++ 'unimplemented' error.
 * Added missing argument to WantUpdate in AButtonFormula.
 * Added #include of abutton.H
 * BUG
 *
 * Revision 1.20  1996/06/14  19:43:45  robr
 * Another temporary checkin.
 * Now all the formula methods are in AButtonv where they belong.
 * BUG
 *
 * Revision 1.19  1996/06/14  17:23:04  robr
 * More use of formulas.  Re-privatized the data made public in
 * the previous rev.
 * BUG
 *
 * Revision 1.18  1996/06/11  23:14:58  robr
 * temporary checkin of the new version using formulas for screen updates.
 * Still need to work out how to avoid the need to make things public.
 * BUG
 *
 * Revision 1.17  1996/05/22  19:35:06  robr
 * Sigh... sure glad I can't count on border always being non-NULL.
 * BUG
 *
 * Revision 1.16  1996/05/20  18:01:58  robr
 * cleaned up warnings.
 * BUG
 *
 * Revision 1.15  1996/05/20  15:36:21  robr
 * Missing return values.
 * BUG
 *
 * Revision 1.14  1996/05/14  15:42:50  robr
 * Corrected the location of the indicator when the label is an inset which is
 * below the base line.
 * BUG
 *
 * Revision 1.13  1996/05/09  19:10:43  robr
 * Added printing of label insets.
 * FEATURE
 *
 * Revision 1.12  1996/05/09  13:34:39  robr
 * Removed bogus static from simpletext class tag.
 * BUG
 *
 * Revision 1.11  1996/05/08  20:49:53  robr
 * Second stage printing implementation.  The text label now
 * lines up with surrounding text.
 * Still need to implement printing of an inset label.
 * FEATURE
 *
 * Revision 1.10  1996/05/07  19:42:25  robr
 * Fixed to use the awidgetview's ashadow object.
 *
 * Revision 1.9  1996/05/07  16:41:28  robr
 * Initial printing implementation.
 * This will need to be updated when GetPrintOrigin is implemented in view.
 * To be complete this will also need to deal with printing a label view.
 * Currently only text labels are printed.
 * FEATURE
 *
 * Revision 1.8  1996/05/06  15:56:53  robr
 * Finished adding recursive search support for both plain text and inset labels.
 *
 * Revision 1.7  1996/05/03  19:36:50  robr
 * Initial addition of support for arbitrary insets as button labels.
 * FEATURE
 *
 * Revision 1.6  1996/05/02  02:11:02  wjh
 * std hdrs
 *
 */
