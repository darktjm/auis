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

view *htmlimagev::Hit(enum view_MouseAction action,long xi, long yi, long numberOfClicks) {
    x=xi;
    y=yi;
    return imagev::Hit(action, x, y, numberOfClicks);
}

htmltextview *htmlimagev::HTMLTextView() {
    view *p=parent;
    ATK_CLASS(htmltextview);
    while(p) {
	if(p->IsType(class_htmltextview)) return (htmltextview *)p;
	p=p->parent;
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
