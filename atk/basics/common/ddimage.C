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

#ifndef NORCSID
#define NORCSID
static char rcsid[]="$Header: /afs/cs.cmu.edu/project/atk-src-C++/atk/basics/common/RCS/ddimage.C,v 1.3 1996/01/31 19:36:36 robr Stab74 $";
#endif

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
	if(change==observable_OBJECTDESTROYED) {
	    source=NULL;
	    outdated=FALSE;
	    return;
	}
	if(change==observable_OBJECTCHANGED) {
	    outdated=TRUE;
	}
    }
}

void iddimage::Draw(graphic * /* dest */, long /* left */, long /* top */ , long /* width */ , long /* height */ , long /* destLeft */ , long /* destTop */) {	
}

void iddimage::Draw(graphic * /*dest */ , const struct rectangle * /* src */ , long /* destLeft*/ , long /* destTop */) {
}
