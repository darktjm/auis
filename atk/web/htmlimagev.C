/* Copyright 1996 Carnegie Mellon University All rights reserved. */
#include <andrewos.h>
ATK_IMPL("htmlimagev.H")
#include <htmlimagev.H>
#include <htmltextview.H>
#include <matte.H>

ATKdefineRegistryNoInit(htmlimagev, imagev);

htmlimagev::htmlimagev() {
    x=y=(-1);
    bordersize=0;
}

htmlimagev::~htmlimagev() {
}

view *htmlimagev::Hit(enum view::MouseAction action,long xi, long yi, long numberOfClicks) {
    x=xi;
    y=yi;
    return imagev::Hit(action, x, y, numberOfClicks);
}

htmltextview *htmlimagev::HTMLTextView() {
    view *p=GetParent();
    ATK_CLASS(htmltextview);
    while(p) {
	if(p->IsType(class_htmltextview)) return (htmltextview *)p;
	p=p->GetParent();
    }
    return NULL;
}

void htmlimagev::LinkTree(view *p) {
    imagev::LinkTree(p);
    ATK_CLASS(matte);
    if(p==NULL || p->GetIM()==NULL || !(p->IsType(class_matte))) return;
    matte *m=(matte *)p;
    m->SetResizing(0);
}
