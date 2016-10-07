/* Copyright 1995 Carnegie Mellon University All rights reserved. */

#include <andrewos.h>
ATK_IMPL("ddimage.H")
#include <ddimage.H>
#include <im.H>
#include <windowsystem.H>

ATKdefineRegistryNoInit(iddimage, observable);


ddimage::ddimage(image *i, colormap **map) {
    windowsystem *ws=im::GetWindowSystem();
    if(ws) {
	di=ws->CreateDDImage(i, map);
    } else di=NULL;
}

ddimage::ddimage(iddimage *i) {
    di=i;
    if(i) i->Reference();
}

ddimage::ddimage(const ddimage &ip) {
    di=ip.di;
    di->Reference();
}

ddimage::~ddimage() {
    if(di) di->Destroy();
    di=NULL;
}

ddimage &ddimage::operator=(iddimage *i) {
    iddimage *old=di;
    di=i;
    di->Reference();
    if(old) old->Destroy();
    return *this;
}

ddimage &ddimage::operator=(image *i) {
    iddimage *old=di;
    windowsystem *ws=im::GetWindowSystem();
    if(ws) {
	di=ws->CreateDDImage(i, Colormap());
    } else di=NULL;
    if(old) old->Destroy();
    return *this;
}

ddimage &ddimage::operator=(const ddimage &id) {
    iddimage *old=di;
    di=id.di;
    di->Reference();
    if(old) old->Destroy();
    return *this;
}

void iddimage::Release(boolean /* delayok */) {
}

void iddimage::ReleaseData(boolean /* delayok */) {
}

boolean iddimage::Process(graphic * /* dest */) {
    return FALSE;
}

iddimage::iddimage() {
    cmap=NULL;
    outdated=TRUE;
    source=NULL;
    quota=ddimage_MAXQUOTA;
}

iddimage::iddimage(image *src, colormap **map) {
    cmap=map;
    if(cmap && *cmap) (*cmap)->AddObserver(this);
    outdated=TRUE;
    source=NULL;
    SetSource(src);
    quota=ddimage_MAXQUOTA;
}

iddimage::~iddimage() {
    SetSource(NULL);
    if(cmap && *cmap) (*cmap)->RemoveObserver(this);
}

void iddimage::SetSource(image *src) {
    if(source!=src) {
	if(source) {
	    source->RemoveObserver(this);
	    source->Destroy();
	}
	source=src;
	if(source) {
	    source->Reference();
	    source->AddObserver(this);
	}
	outdated=TRUE;
    }
}

void iddimage::ObservedChanged(observable *changed, long change) {
    if(changed==source) {
	if(change==observable::OBJECTDESTROYED) {
	    source=NULL;
	    outdated=FALSE;
	    return;
	}
	if(change==observable::OBJECTCHANGED) {
	    outdated=TRUE;
	}
    }
}

void iddimage::Draw(graphic * /* dest */, long /* left */, long /* top */ , long /* width */ , long /* height */ , long /* destLeft */ , long /* destTop */) {	
}

void iddimage::Draw(graphic * /*dest */ , const struct rectangle * /* src */ , long /* destLeft*/ , long /* destTop */) {
}
