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

#include <alistv.H>
#include <text.H>
#include <textview.H>
#include <style.H>
#include <fontdesc.H>
#include <im.H>

ATKdefineRegistry(AListv, ATextv, AListv::Init);


void AListvTextviewStateFormula::Evaluate() {
    atv->dataViewFormula.Validate();
    if(atv->tv) atv->tv->hitcallback=&atv->hitAct;
}


void AListv::SingleMode(view_MouseAction act, long ind) {
    AList *dself=(AList *)GetDataObject();
    switch(act) {
        case view_LeftDown: {
            long i, c;
            dself->Toggle(ind);
            c=dself->selectionFormula.Count();
            AListSelectedItem *items=dself->selectionFormula.Items();
            for(i=0;i<c;i++) {
                if(items[i].ind!=ind) dself->DeSelect(items[i].ind);
            }
            }
            break;
        default: ;
    }
}

void AListv::BrowseMode(view_MouseAction act, long ind) {
    AList *dself=(AList *)GetDataObject();
    switch(act) {
        case view_LeftMovement:
        case view_LeftDown:
        case view_LeftUp: {
            long i, c;
            dself->Select(ind);
            c=dself->selectionFormula.Count();
            AListSelectedItem *items=dself->selectionFormula.Items();
            for(i=0;i<c;i++) {
                if(items[i].ind!=ind) dself->DeSelect(items[i].ind);
            }
            }
            break;
        default: ;
    }
}

void AListv::MultipleMode(view_MouseAction act, long ind) {
    AList *dself=(AList *)GetDataObject();
    switch(act) {
        case view_LeftDown: {
            dself->Toggle(ind);
            }
            break;
        default: ;
    }
}


void AListv::ExtendedMode(view_MouseAction act, long ind) {
    AList *dself=(AList *)GetDataObject();
    long i,c;
    long min, max;
    char *mods;
    AListSelectedItem *items;
    if(firstInd==0 && act!=view_LeftDown) return;
    
    switch(act) {
        case view_LeftDown:
            mods=GetIM()-> WantInformation("hit_modifiers");
            if(mods==NULL || *mods!='s') {
                firstInd=ind;
                dself->Select(ind);
                c=dself->selectionFormula.Count();
                AListSelectedItem *items=dself->selectionFormula.Items();
                for(i=0;i<c;i++) {
                    if(items[i].ind!=ind) dself->DeSelect(items[i].ind); 
                }
            } else {
                firstInd=ind;
                toggleMode=TRUE;
                dself->Toggle(ind);
            }
            break;
        case view_LeftMovement:
        case view_RightMovement:
        case view_RightDown:
            min=MIN(ind,firstInd);
            max=MAX(ind,firstInd);
            c=dself->selectionFormula.Count();
            items=dself->selectionFormula.Items();
            if(!toggleMode) {
                for(i=0;i<c;i++) {
                    if(items[i].ind<min || items[i].ind>max) dself->DeSelect(items[i].ind); 
                }
                for(i=min;i<=max;i++) {
                    dself->Select(i);
                }
            } else {
                boolean selected=dself->Selected(firstInd);
                
                for(i=min;i<=max;i++) {
                    if(selected) dself->Select(i);
                    else dself->DeSelect(i);
                }
            }
            break;
        default: ;
    }
    if(act==view_LeftUp || act==view_RightUp) toggleMode=FALSE;
}

START_VIEW_CALLBACK_METHOD(TextHitMethod,AList,AListv) {
    text *t=(text *)tv->GetDataObject();
    view_MouseAction act=(view_MouseAction)in[1].Integer();
    long pos=in[2].Integer();
    long num_clicks=in[3].Integer();
    long line=t->GetLineForPos(pos);
    if(num_clicks==2) {
        avalueflex dummy;
        aaction *act=dself->defaultActionCallback;
        if(act) (*act)(dself, avalueflex()|this|line, dummy);
        return;
    }
    switch(*(const char*)dself->selectionPolicy) {
        default:
        case 'S':
            SingleMode(act,line);
            break;
        case 'B':
            BrowseMode(act,line);
            break;
        case 'M':
            MultipleMode(act,line);
            break;
        case 'E':
            ExtendedMode(act,line);
            break;
    }
    t->NotifyObservers(observable_OBJECTCHANGED);
    im::ForceUpdate();
}
END_CALLBACK_METHOD();

boolean AListv::Init() {
    ENTER_AACTION_METHOD(TextHitMethod, AListv, "alist-text-hit", 0L, "Called for every mouse hit.");
    return TRUE;
}

void AListvTextAct::operator()(ATK *obj, const avalueflex &in, avalueflex &out) {
    AList *al=(AList *)av->GetDataObject();
    aaction *act=al->listHitCallback;
    if(act) (*act)(al, avalueflex()| av | in[1].Integer() | in[2].Integer() | in[3].Integer(), out);
}

AListv::AListv() : hitAct(this), listTextviewStateFormula(this) {
    firstInd=0;
    toggleMode=FALSE;
}


void AListv::InitChildren() {
    ATextv::InitChildren();
    listTextviewStateFormula.Validate();
}

void AListv::LinkTree(class view *parent) {
    ATextv::LinkTree(parent);
    if(parent && parent->GetIM()) {
        listTextviewStateFormula.Validate();
    }
}
