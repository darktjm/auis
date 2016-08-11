/*
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

#include <alist.H>
#include <alistv.H>
#include <text.H>
#include <style.H>
#include <environment.H>

static boolean Init() {
    AListv::Init();
    return TRUE;
}

ATKdefineRegistry(AList,AText,Init);

void AList::Count(ASlot *slot, const avalueflex &aux, const avalueflex &in, avalueflex &out) {
    AList *al=(AList *)aux[0].ATKObject();
    text *t=al->Source();
    if(t==NULL || t->GetLength()==0) al->itemCount=0;
    else {
        long result=t->GetLineForPos(t->GetLength()-1);
        if(result<0) result=0;
        al->itemCount=result;
    }
}

void AListStateFormula::Evaluate() {
    text *t=al->Source();
    long topline=al->topItemPosition;
    if(t) {
        if(topline==0) {
            al->SetTopCharacter(t->GetLength());
        } else {
            al->SetTopCharacter( t->GetPosForLine(topline));
        }
    }
}


void AList::Rows(ASlot *slot, const avalueflex &aux, const avalueflex &in, avalueflex &out) {
    AList *at=(AList *)aux[0].ATKObject();
    at->SetRows(at->visibleItemCount);
    slot->EnableFlags(ASlot::valid);
}

void AList::SelCount(ASlot *slot, const avalueflex &aux, const avalueflex &in, avalueflex &out) {
    AList *at=(AList *)aux[0].ATKObject();
    at->selectionFormula.Validate();
    out.add(at->selectionFormula.Count());
}

void AList::Items(ASlot *slot, const avalueflex &aux, const avalueflex &in, avalueflex &out) {
    AList *at=(AList *)aux[0].ATKObject();
    at->selectionFormula.Validate();
    out.add((void *)at->selectionFormula.Items());
}

boolean AListSelectionFormula::ScanSelection(long rock, text *txt, long pos, environment *curenv) {
    AListSelectionFormula *self=(AListSelectionFormula *)rock;
    if(curenv->type==environment_Style && curenv->data.style==self->al->selected) {
        AListSelectedItem *item=self->items.Append();
        item->pos=curenv->Eval();
        item->len=curenv->GetLength();
        item->ind=txt->GetLineForPos(pos);
    }
    return FALSE;
}

void AListSelectionFormula::Evaluate() {
    text *txt=al->Source();
    items.Remove((size_t)0,items.GetN());
    if(txt==NULL) return;
    ScanSelection((long)this, txt, 0, txt->rootEnvironment);
    txt->EnumerateEnvironments(0, txt->rootEnvironment->GetLength(), ScanSelection, (long)this);
}

AList::~AList() {
    delete selected;
    delete notselected;
}

AList::AList() : selectionFormula(this), selCountAct(SelCount, this), itemsAct(Items, this), stateFormula(this), rowsAct(Rows, this), countAct(Count, this) {
    ATKinit;

    SLOT(itemCount);
    
    SLOTINIT(listHitCallback,NULLACT);
    SLOTINIT(defaultActionCallback,NULLACT);
    SLOT(selectionPolicy);
    SLOTINIT(topItemPosition,1L);
    SLOTINIT(visibleItemCount,1L);
    
    TSLOTINIT(selectedItemCount,0L);
    TSLOTINIT(selectedItems,(void *)NULL);
    
    selectedItemCount.SetFormula(&selCountAct);
    selectedItems.SetFormula(&itemsAct);
    itemCount.SetFormula(&countAct);
    
    selectionFormula.SetMode(AFORMULAACCESS);
    
    selected=new style;
    selected->AddNewFontFace(fontdesc_Bold);
    selected->SetName("selected");
    notselected=new style;
    notselected->SetName("notselected");
    
    stateFormula.Validate();

    rows.SetFormula(&rowsAct);

    itemCount=0;
}


void AList::Insert(long ind, const char *str) {
    text *txt=Source();
    if(txt==NULL) return;
    long pos=txt->GetPosForLine(ind);
    long len=strlen(str);
    txt->AlwaysInsertCharacters(pos, (char *)str, len);
    txt->AlwaysInsertCharacters(pos+len, "\n", 1);
    txt->AlwaysAddStyle(pos, len, notselected);
    topCharacter.Invalidate();
    topItemPosition.Invalidate();
}

void AList::Remove(long ind) {
    text *txt=Source();
    if(txt==NULL) return;
    long pos=txt->GetPosForLine(ind);
    long epos=txt->Index(pos, '\n', txt->GetLength()-pos);
    if(epos<0) epos=txt->GetLength();
    txt->AlwaysDeleteCharacters(pos, epos-pos+1);
    selectionFormula.Invalidate();
    topCharacter.Invalidate();
    topItemPosition.Invalidate();
}

const char *AList::Item(long ind) {
    text *txt=Source();
    long pos=txt->GetPosForLine(ind);
    long epos=txt->Index(pos, '\n', txt->GetLength()-pos);
    if(epos<0) epos=txt->GetLength();
    static flex buf;
    buf.Remove((size_t)0,buf.GetN());
    buf.Insert((size_t)0, epos-pos);
    size_t dummy;
    char *p=buf.GetBuf((size_t)0, epos-pos+1, &dummy);
    txt->CopySubString(pos, epos-pos, p, FALSE);
    p[epos+pos]='\0';
    return p;
}

static environment *FindEnv(text *txt, long pos) {
    environment *env=(environment *)txt->rootEnvironment-> GetInnerMost(pos);
    while(env && env->type!=environment_Style) env=(environment *)env->parent;
    return env;
}

void AList::Select(long ind) {
    text *txt=Source();
    if(txt==NULL) return;
    long pos=txt->GetPosForLine(ind);
    long epos=txt->Index(pos, '\n', txt->GetLength()-pos);
    if(epos<0) epos=txt->GetLength();
    environment *env=FindEnv(txt, pos);
    if(env && (env->data.style==notselected ||  env->data.style==selected)) {
        if(env->data.style!=selected) {
            env->data.style=selected;
            txt->RegionModified(pos, epos-pos);
        }
    } else txt->AlwaysAddStyle(pos, epos-pos, selected);
    selectionFormula.Invalidate();
}


void AList::DeSelect(long ind) {
    text *txt=Source();
    if(txt==NULL) return;
    long pos=txt->GetPosForLine(ind);
    long epos=txt->Index(pos, '\n', txt->GetLength()-pos);
    if(epos<0) epos=txt->GetLength();
    environment *env=FindEnv(txt, pos);
    if(env && (env->data.style==notselected ||  env->data.style==selected)) {
        if(env->data.style==selected) {
            env->data.style=notselected;
            txt->RegionModified(pos, epos-pos);
        }
    } else txt->AlwaysAddStyle(pos, epos-pos, notselected);
    selectionFormula.Invalidate();
}

void AList::Toggle(long ind) {
    text *txt=Source();
    if(txt==NULL) return;
    long pos=txt->GetPosForLine(ind);
    long epos=txt->Index(pos, '\n', txt->GetLength()-pos);
    if(epos<0) epos=txt->GetLength();
    environment *env=FindEnv(txt, pos);
    if(env && (env->data.style==notselected ||  env->data.style==selected)) {
        env->data.style= (env->data.style==notselected) ? selected : notselected;
        txt->RegionModified(pos, epos-pos);
    } else txt->AlwaysAddStyle(pos, epos-pos, selected);
    selectionFormula.Invalidate();
}

boolean AList::Selected(long ind) {
    text *txt=Source();
    if(txt==NULL) return FALSE;
    long pos=txt->GetPosForLine(ind);
    environment *env=FindEnv(txt, pos);
    if(env && env->data.style==selected) {
        return TRUE;
    }
    return FALSE;
}

