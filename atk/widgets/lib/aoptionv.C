/* Copyright 1996 Carnegie Mellon University All rights reserved. */

#include <andrewos.h>

#include <aoption.H>
#include <aoptionv.H>
#include <message.H>
#include <popupwin.H>
#include <text.H>
#include <proctable.H>
#include <aaction.H>
#include <im.H>
#include <textview.H>

ATK_IMPL("aoptionv.H")
ATKdefineRegistry(AOptionMenuv, AButtonv, AOptionMenuv::Init);
ATKdefineRegistryNoInit(AOptionCardv, AButtonv);
ATKdefineRegistryNoInit(AOptionSourceFormula, AFormula);

AOptionMenuv::AOptionMenuv() : source(this) {
    win=NULL;
    b=NULL;
    cv=NULL;
}

AOptionMenuv::~AOptionMenuv() {
    if(win) win->Destroy();
    if(b) b->Destroy();
    if(cv) cv->Destroy();
}

ATK_CLASS(textview);
START_VIEW_MOUSE_METHOD(ButtonHitMethod, AOptionMenu, AOptionMenuv) {
    (void)num_clicks; (void)within; /* unused */
    if(dself->Source()==NULL) {
        text *txt=new text;
        dself->SetSource(txt);
        txt->Destroy(); // the source slot will hang onto the text for us.
    }
    if(action!=view_LeftDown) return;
    if(b==NULL) b=(AButton *)AWidget::NewWidget("OptionCard");
    if(b==NULL) {
        message::DisplayString(this, 100, "Couldn't create option menu.");
        return;
    }
    b->SetLabelData(dself->Source());
    if(cv==NULL) {
        cv=(AOptionCardv *)AWidget::NewWidgetView(b);
        if(cv==NULL) {
            message::DisplayString(this, 100, "Couldn't create option menu.");
            return;
        }
        cv->SetDataObject(b);
        cv->SetOptionMenuv(this);
    }
    dself->NotifyObservers(observable::OBJECTCHANGED);
    if(win==NULL) win=popupwin::Create(GetIM(), cv);
    if(win==NULL) {
        message::DisplayString(this, 100, "Couldn't create option menu.");
        return;
    }
    win->Post(this, x, y, action);
    out.add(cv);
}
END_MOUSE_METHOD();


AOptionCardv::AOptionCardv(AOptionMenuv *v) {
    mv=v;
}

AOptionCardv::~AOptionCardv() {
}


START_VIEW_MOUSE_METHOD(CardHitMethod, AButton, AOptionCardv) {
    (void)dself; (void)num_clicks; (void)within; /* unused */
    if(action==view_LeftUp || action==view_RightUp) GetIM()->VanishWindow();
    view *v=Label();
    if(v==NULL || !v->IsType(class_textview)) return;
    if(GetIM()==NULL) return;
    textview *tv=(textview *)v;
    text *t=(text *)tv->GetDataObject();
    x=tv->EnclosedXToLocalX(ToPixX(x));
    y=tv->EnclosedYToLocalY(ToPixY(y));
    if(x>=tv->GetLogicalWidth() || x<0 || y>=tv->GetLogicalHeight() || y<0) {
        tv->SetDotLength(0);
        tv->SetDotPosition(0);
        return;
    }
    view *vptr=NULL;
    long pos=tv->Locate(x,y,&vptr);
    long lpos=t->GetBeginningOfLine(pos);
    long epos=t->GetEndOfLine(pos);
    if(lpos!=tv->GetDotPosition() || epos-lpos!=tv->GetDotLength()) {
        tv->SetDotPosition(lpos);
        tv->SetDotLength(epos-lpos);
        tv->WantInputFocus(tv);
        im::ForceUpdate();
    }
    if(mv==NULL || action!=view_LeftUp) return;
    long line=t->GetLineForPos(lpos);
    AOptionMenu *m=(AOptionMenu *)mv->GetDataObject();
    if(m) m->SetSelected(line);
}
END_MOUSE_METHOD();

boolean AOptionMenuv::Init() {
    static boolean inited=FALSE;
    if(inited) return TRUE;
    inited=TRUE;
    ENTER_AACTION_METHOD(ButtonHitMethod, AOptionMenuv, "aoption-button-hit", 0L, "Pops up the option list.");
    ENTER_AACTION_METHOD(CardHitMethod, AOptionCardv, "aoption-card-hit", 0L, "Pops up the option list.");
    return TRUE;
}

void AOptionMenuv::LabelDimensions(long &w, long &h, long *below) {
    AOptionMenu *am=(AOptionMenu *)GetDataObject();
    w=42;
    h=20;
    if(below) *below=0;
    text *t=am->Source();
    fontdesc *font=FindFont();
    if(t==NULL || GetIM()==NULL) return;
    graphic *my_graphic=GetDrawable();
    long mw=0, mh=0;
    static flex buf;
    long pos=0, len=t->GetLength();
    while(pos<len) {
        long epos=t->GetEndOfLine(pos);
        long ew, eh;
        buf.Remove((size_t)0, buf.GetN());
        char *txt=buf.Insert((size_t)0, epos-pos+1);
        t->CopySubString(pos, epos-pos, txt, FALSE);
        txt[epos-pos]='\0';
        if(*txt && txt[1]=='\0') {
            struct fontdesc_charInfo ci;
            (font)->CharSummary( my_graphic, *txt, &ci);
            ew=ci.width+(long)(4 * ((double)am->scaleWidth)/uxscale);
            eh=ci.height+(long)(4 * ((double)am->scaleHeight)/uyscale);
        } else {
            struct FontSummary *my_FontSummary;
            (font)->StringSize( my_graphic, (char *)txt, &ew, &eh);
            my_FontSummary =  (font)->FontSummary( my_graphic);
            if(my_FontSummary) {
                eh=my_FontSummary->maxHeight;
                if(below) *below=my_FontSummary->maxBelow;
            }
        }
        if(ew>mw) mw=ew;
        if(eh>mh) mh=eh;
        pos=epos+1;
    }
    w=mw;
    h=mh;
}

void AOptionSourceFormula::Evaluate() {
    AOptionMenu *am=(AOptionMenu *)av->GetDataObject();
    text *src=am->Source();
    if(observed && observed!=src) {
        observed->RemoveObserver(av);
    }
    if(src) src->AddObserver(av);
    observed=src;
}

void AOptionMenuv::ObservedChanged(observable *changed, long change) {
    AButtonv::ObservedChanged(changed, change);
    if(changed==source.Observed()) {
        if(change!=observable::OBJECTDESTROYED) size.Invalidate();
        else source.ClearObserved();
    }
}

void AOptionMenuv::SetDataObject(class dataobject *dobj) {
    AButtonv::SetDataObject(dobj);
    source.Validate();
}
