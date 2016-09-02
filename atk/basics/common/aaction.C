/* Copyright 1995 Carnegie Mellon University All rights reserved.
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

#include <aaction.H>

ATKdefineRegistryNoInit(aaction,ATK);

void aaction::operator()(ATK *, const avalueflex &, avalueflex &) {
}

static avalueflex empty;

avalueflex &aaction::ArgTypes() {
    return empty;
}

avalueflex &aaction::RetTypes() {
    return empty;
}

aaction::~aaction() {
}

void aaction::Destroy() {
    if(refs>0) {
	refs--;
	if(refs==0) {
	    delete this;
	    return;
	}
    }
}

ATKdefineRegistryNoInit(multiact,aaction);

multiact &multiact::Prepend(aaction *act) {
    *list.Insert((size_t)0)=act;
    act->Reference();
    return *this;
}

multiact &multiact::Append(aaction *act) {
    *list.Append()=act;
    act->Reference();
    return *this;
}

multiact &multiact::Remove(aaction *act) {
    size_t i;
    for(i=0;i<list.GetN();i++) {
	if(list[i]==act) {
	    list.Remove(i);
	    act->Destroy();
	    return *this;
	}
    }
    return *this;
}

multiact *multiact::ChainBefore(aaction *current, aaction *newact) {
    multiact *m=NULL;
    if(current && current->IsType(&multiact_ATKregistry_)) {
	m=(multiact *)current;
	m->Prepend(newact);
    } else {
	m=new multiact;
	m->Append(newact);
	m->Append(current);
    }
    return m;
}


multiact *multiact::ChainAfter(aaction *current, aaction *newact) {
    multiact *m=NULL;
    if(current && current->IsType(&multiact_ATKregistry_)) {
	m=(multiact *)current;
	m->Append(newact);
    } else {
	m=new multiact;
	m->DeReference();
	if(current) m->Append(current);
	m->Append(newact);
    }
    return m;
}

void multiact::operator()(ATK *obj, const avalueflex &in, avalueflex &out) {
    size_t i;
    for(i=0;i<list.GetN();i++) {
	if(list[i]) (*list[i])(obj, in, out);
    }
}

