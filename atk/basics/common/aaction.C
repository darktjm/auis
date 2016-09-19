/* Copyright 1995 Carnegie Mellon University All rights reserved. */

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

