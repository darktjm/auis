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
#include <atext.H>
#include <atextv.H>
#include <text.H>
#include <style.H>
#include <fontdesc.H>
ATK_IMPL("atext.H")
static boolean Init() {
    ATextv::Init();
    return TRUE;
}

void ATextStateFormula::Evaluate() {
    text *src=txt->Source();
    AFORMULA_BEGIN;
    {
        boolean ro=!txt->Editable();
        if(src) src->SetReadOnly(ro);
    }
    AFORMULA_END;
}

ATKdefineRegistry(AText,AWidget,Init);

void AText::CommonInit() {
    value.SetFormula(&valueAct);
    value.SetAssign(&valueAssignAct);
    source.SetAssign(&sourceAct);
    SLOT(font);
    SLOTINIT(cursorPosition,0);
    SLOTINIT(marginHeight,480);
    SLOTINIT(marginWidth,480);
    SLOTINIT(activateCallback,NULLACT);
    SLOTINIT(focusCallback,NULLACT);
    SLOTINIT(losingFocusCallback,NULLACT);
    SLOTINIT(valueChangedCallback,NULLACT);
#if 0
    SLOTINIT(autoShowCursorPosition,TRUE);
    SLOTINIT(resizeHeight,FALSE);
    SLOTINIT(resizeWidth,FALSE);
#endif
    SLOTINIT(wordWrap,FALSE);
    SLOTINIT(scrollHorizontal,TRUE);
    SLOTINIT(scrollLeftSide,TRUE);
    SLOTINIT(scrollTopSide,FALSE);
    SLOTINIT(scrollVertical,TRUE);
    SLOTINIT(scrolled,FALSE);
    SLOTINIT(editable,TRUE);
    SLOTINIT(editMode,AText_SingleLine);
    SLOTINIT(topCharacter,0);
    SLOTINIT(rows,1);
    SLOTINIT(columns,20);
    TSLOTINIT(source,NULLATK);
    SLOT(value);
    SLOT(textViewClass);
    observeTextChanges=TRUE;
    doingTextValue=FALSE;
    stateFormula.Validate();
}

static void ATextValue(ASlot *slot, const avalueflex &aux, const avalueflex &in, avalueflex &out) {
    AText *at=(AText *)aux[0].ATKObject();
    if(at->Source()) {
        text *t=at->Source();
        static flex local;
        if(t->GetLength()+1>(int)local.GetN()) {
            local.Insert(local.GetN(), t->GetLength()+1-local.GetN());
        }
        size_t dummy;
        char *p=local.GetBuf(0, local.GetN(), &dummy);
        t->CopySubString(0, t->GetLength(), p, FALSE);
        p[t->GetLength()]='\0';
        at->SetDoingTextValue(TRUE);
        at->SetValue(p);
        at->SetDoingTextValue(FALSE);
    }
}

static atom_def type("avalue_u");
static void ATextSourceAssign(ASlot *slot, const avalueflex &aux, const avalueflex &in, avalueflex &out) {
    AText *at=(AText *)aux[0].ATKObject();
    text *ot=at->Source();
    if(ot) ot->RemoveObserver(at);
    slot->UseOnlyFromAnAssignFunction((avalue_u *)in[0].VoidPtr(type));
    text *nt=at->Source();
    if(nt) nt->AddObserver(at);    
}

static void ATextValueAssign(ASlot *slot, const avalueflex &aux, const avalueflex &in, avalueflex &out) {
    AText *at=(AText *)aux[0].ATKObject();
    avalue_u *val=(avalue_u *)in[0].VoidPtr(type);
    slot->UseOnlyFromAnAssignFunction(val);
    if(at->DoingTextValue()) return;
    text *t=at->Source();
    if(t==NULL) {
        at->SetSource(t=new text);
    }
    t->AlwaysDeleteCharacters(0, t->GetLength());
    if(val->cstr) t->AlwaysInsertCharacters(0, (char *)val->cstr, strlen(val->cstr));
    at->SetObserveTextChanges(FALSE);
    t->NotifyObservers(observable_OBJECTCHANGED);
    at->SetObserveTextChanges(TRUE);
}

AText::AText() : stateFormula(this), sourceAct(ATextSourceAssign, this), valueAct(ATextValue, this), valueAssignAct(ATextValueAssign, this) {
    ATKinit;
    CommonInit();
}

AText::AText(const char *txt) : stateFormula(this), sourceAct(ATextSourceAssign, this), valueAct(ATextValue, this), valueAssignAct(ATextValueAssign, this)  {
    ATKinit;
    value=(char *)txt;
    CommonInit();
}

AText::~AText() {
    if((dataobject *)source) ((dataobject *)source)->Destroy();
}

void AText::ObservedChanged(observable *changed, long change) {
    if(observeTextChanges) {
        value.Invalidate();
        value.Remember();
    }
}
