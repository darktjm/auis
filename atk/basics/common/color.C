/* Copyright 1995 Carnegie Mellon University All rights reserved.
  $Disclaimer: 
// Permission to use, copy, modify, and distribute this software and its 
// documentation for any purpose and without fee is hereby granted, provided 
// that the above copyright notice appear in all copies and that both that 
// copyright notice and this permission notice appear in supporting 
// documentation, and that the name of IBM not be used in advertising or 
// publicity pertaining to distribution of the software without specific, 
// written prior permission. 
//                         
// THE COPYRIGHT HOLDERS DISCLAIM ALL WARRANTIES WITH REGARD 
// TO THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF 
// MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL ANY COPYRIGHT 
// HOLDER BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL 
// DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, 
// DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE 
// OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION 
// WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
// 
//  $
 */

#include <andrewos.h>
ATK_IMPL("color.H");
#include <color.H>
#include <util.h>

static color *dilist=NULL;

ddcolor::ddcolor() {
    ir=ig=ib=0;
    refs=1;
    cmap=NULL;
    iname=NULL;
}

ddcolor::ddcolor(colormap *m, const atom *n, unsigned short r, unsigned short g, unsigned short b) {
    ir=r;
    ig=g;
    ib=b;
    refs=1;
    cmap=m;
    iname=n;
}

void ddcolor::Destroy() {
    if(refs<=1) delete this;
    else refs--;
}

ddcolor::~ddcolor() {
    if(cmap && cmap->colors.GetN()) {
	size_t max=cmap->colors.GetN()-1;
	for(size_t i=0;i<=max;i++) {
	    if(cmap->colors[max-i]==this) {
		cmap->colors.Remove(max-i, 1);
		break;
	    }
	}
    }
}

void ddcolor::GetRGB(unsigned short &ra, unsigned short &ga, unsigned short &ba) {
    ra=ir;
    ga=ig;
    ba=ib;
}

color::color() {
    name=NULL;
    r=g=b=0;
    devcolors=NULL;
    next=dilist;
    dilist=this;
}

color::color(const char *n) {
    name=atom::Intern(n);
    r=g=b=0;
    devcolors=NULL;
    next=dilist;
    dilist=this;
}

color::color(const atom *n) {
    name=n;
    r=g=b=0;
    devcolors=NULL;
    next=dilist;
    dilist=this;
}

color::color(unsigned short ar, unsigned short ag, unsigned short ab) {
    r=ar;
    g=ag;
    b=ab;
    devcolors=NULL;
    name=NULL;
    next=dilist;
    dilist=this;
}

color::color(const color &src) {
    next=dilist;
    dilist=this;
    devcolors=NULL;
    r=src.r;
    g=src.g;
    b=src.b;
    name=src.name;
}
    
color::~color() {
    while(devcolors) {
	ddcolor_list *next=devcolors->next;
	devcolors->dcolor->Destroy();
	delete devcolors;
	devcolors=next;
    }
    color *s=dilist;
    color **prev=&dilist;
    while(s) {
	if(s==this) {
	    *prev=s->next;
	    break;
	} else prev=&s->next;
	s=s->next;
    }
}

void color::AddMapping(ddcolor *dc) {

    if(dc->cmap && dc!=FindMapping(dc->cmap)) {
	DeleteMapping(dc->cmap);
	ddcolor_list *d=new ddcolor_list;
	d->dcolor=dc;
	d->next=devcolors;
	devcolors=d;
    } else dc->Destroy(); // destroy the ref the caller got for us.
}

void color::DeleteMapping(colormap *dm) {
    ddcolor_list *d=devcolors, **prev=&devcolors;
    while(d) {
	if(d->dcolor->cmap==dm) {
	    *prev=d->next;
	    d->dcolor->Destroy();
	    delete d;
	    return;
	}
	prev=&d->next;
	d=d->next;
    }
}

ddcolor *color::FindMapping(colormap *dm) {
    ddcolor_list *d=devcolors;
    while(d) {
	if(d->dcolor->cmap==dm) return d->dcolor;
	d=d->next;
    }
    return NULL;
}

void color::HardwareRGB(colormap *dm, unsigned short &ra, unsigned short &ga, unsigned short &ba) {
    if(dm==NULL) return;
    ddcolor *d=FindMapping(dm);
    if(d==NULL) {
	d=dm->Allocate(this);
	if(d) AddMapping(d);
	else return;
    }
    d->ddcolor::GetRGB(ra, ga, ba);
}

color &color::operator=(const color &src) {
    name=src.name;
    r=src.r;
    g=src.g;
    b=src.b;
    ddcolor_list *d=devcolors;
    while(d) {
	ddcolor_list *n=d->next;
	d->dcolor->Destroy();
	delete d;
	d=n;
    }
    devcolors=NULL;
    return *this;
}

color &color::operator=(const char *n) {
    name=atom::Intern(n);
    r=g=b=0;
    ddcolor_list *d=devcolors;
    while(d) {
	ddcolor_list *n=d->next;
	d->dcolor->Destroy();
	delete d;
	d=n;
    }
    devcolors=NULL;
    return *this;
}

color &color::operator=(const atom *n) {
    name=n;
    r=g=b=0;
    ddcolor_list *d=devcolors;
    while(d) {
	ddcolor_list *n=d->next;
	d->dcolor->Destroy();
	delete d;
	d=n;
    }
    devcolors=NULL;
    return *this;
}

// ColorName stores a name for the color into result;  returns result
// 	copies name if not null;  otherwise uses #xxxxxx from rgb
char *color::ColorName(char *result, char *name, unsigned char rgb[3]) {
	if (name && *name) {
		strncpy(result, name, 100);
		result[99] = '\0';
	}
	else 
		sprintf(result, "#%02x%02x%02x",  
                        rgb[0], rgb[1], rgb[2]);
        return result;
}


static icolor *idilist=NULL;

icolor::icolor() {
    refs=1;
    next=idilist;
    idilist=this;
}

icolor::~icolor() {
    icolor **prev=&idilist;
    icolor *s=idilist;
    while(s) {
	if(s==this) {
	    *prev=s->next;
	    break;
	} else prev=&s->next;
	s=s->next;
    }
}

icolor::icolor(const char *n) : color(n) {
    refs=1;
    next=idilist;
    idilist=this;
}

icolor::icolor(const atom *n) : color(n) {
    refs=1;
    next=idilist;
    idilist=this;
}


icolor::icolor(unsigned short ar, unsigned short ag, unsigned short ab) : color(ar, ag, ab) {
    refs=1;
    next=idilist;
    idilist=this;
}


icolor *icolor::Find(const char *name) {
    return Find(atom::Intern(name));
}

icolor *icolor::Find(const atom *name) {
    icolor *s=idilist;
    while(s) {
	if(s->name==name) return s;
	s=s->next;
    }
    return NULL;
}

icolor *icolor::Find(const color *c) {
    icolor *s=idilist;
    while(s) {
	if(s==c) return s;
	s=s->next;
    }
    return NULL;
}

icolor *icolor::Find(unsigned short r, unsigned short g, unsigned short b) {
    icolor *s=idilist;
    while(s) {
	if(s->name==NULL && s->r==r && s->g==g && s->b==b) return s;
	s=s->next;
    }
    return NULL;
}

icolor *icolor::Create(const char *name) {
    return Create(atom::Intern(name));
}

icolor *icolor::Create(const atom *name) {
    icolor *result=Find(name);
    if(result) {
	result->refs++;
	return result;
    }
    return new icolor(name);
    
}
icolor *icolor::Create(unsigned short r, unsigned short g, unsigned short b) {
    icolor *result=Find(r, g, b);
    if(result) {
	result->refs++;
	return result;
    }
    return new icolor(r, g, b);
}

void icolor::Destroy() {
    if(--refs>0) return;
    delete this;
}


icolormap::icolormap() {
}

icolormap::~icolormap() {
    Clear();
}

icolor *icolormap::Alloc(const char *name, unsigned short r, unsigned short g, unsigned short b) {
    return name?Alloc(name):Alloc(r, g, b);
}

icolor *icolormap::Alloc(const atom *name, unsigned short r, unsigned short g, unsigned short b) {
    return name?Alloc(name):Alloc(r, g, b);
}

icolor *icolormap::Alloc(const char *name) {
    return Alloc(atom::Intern(name));
}

icolor *icolormap::Alloc(const atom *name) {
    size_t i;
    icolor *ic;
    for(i=0;i<GetN();i++) {
	ic=(*this)[i];
	if(ic && ic->name && ic->name==name) return ic;
    }
    ic=icolor::Create(name);
    if(ic==NULL) return NULL;
    *Append()=ic;
    return ic;
}

icolor *icolormap::Alloc(unsigned short r, unsigned short g, unsigned short b) {
    size_t i;
    icolor *ic;
    for(i=0;i<GetN();i++) {
	ic=(*this)[i];
	if(ic->name==NULL && ic->r==r && ic->g==g && ic->b==b) return ic;
    }
    ic=icolor::Create(r, g, b);
    if(ic==NULL) return NULL;
    *Append()=ic;
    return ic;
}

void icolormap::Clear() {
    for(unsigned int i=0;i<GetN();i++) {
	(*this)[i]->Destroy();
    }
    Remove((size_t)0, GetN()-1);
}

ATKdefineRegistryNoInit(colormap, observable);
colormap::colormap() {
}

colormap::~colormap() {
    Flush();    
}

void colormap::Flush() {
    NotifyObservers(colormap_FLUSH);
    color *s=dilist;
    while(s) {
	s->DeleteMapping(this);
	s=s->Next();
    }
}


ddcolor *colormap::Allocate(const color *c) {
    if(c->name) return Alloc(c->name);
    return Alloc(c->r, c->g, c->b);
}
		
ddcolor *colormap::Allocate(const char *name, unsigned short r, unsigned short g, unsigned short b) {
    return name?Allocate(name):Alloc(r, g, b);
}

ddcolor *colormap::Allocate(const atom *name, unsigned short r, unsigned short g, unsigned short b) {
    return name?Alloc(name):Alloc(r, g, b);
}

ddcolor *colormap::Allocate(const char *name) {
    return Alloc(atom::Intern(name));
}

ddcolor *colormap::Alloc(const atom */* name */) {
    return NULL;
}

ddcolor *colormap::Alloc(unsigned short /* r */, unsigned short /* g */, unsigned short /* b */) {
    return NULL;
}

void colormap::Free(ddcolor *dc) {
    for(size_t i=0;i<colors.GetN();i++) {
	if(colors[i]==dc) {
	    if(dc->refs==1) colors.Remove(i);
	    dc->Destroy();
	    return;
	}
    }
}

