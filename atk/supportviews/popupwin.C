/* Copyright 1995 Carnegie Mellon University All rights reserved. */

#include <andrewos.h>
ATK_IMPL("popupwin.H");
#include "popupwin.H"
#include <proctable.H>
#include <keymap.H>
#include <im.H>
#include <aaction.H>

static boolean InitializeClass();
ATKdefineRegistry(popupwin, view, InitializeClass);

DEFINE_AACTION_FUNC_CLASS(popupwinfunc,popupwin);

static void RetractProc(popupwin *p,  const avalueflex &aux, const avalueflex &in, avalueflex &out) {
    p->Retract();
}

static popupwinfunc func(RetractProc,1L);
static keymap *km=NULL;

static boolean InitializeClass() {
    proctable_Entry *proc=proctable::DefineAction("popupwin-cancel", &func, &popupwin_ATKregistry_, NULL, "Cancels a popup.");
    km=new keymap;
    km->BindToKey("\007", proc, 0);
    return TRUE;
}

popupwin::popupwin() {
    ATKinit;
    lks=keystate::Create(this, km);
    v=NULL;
}

popupwin::~popupwin() {
    GetIM()->Destroy();
    delete lks;
}

view *popupwin::Hit(view_MouseAction act, long x, long y, long numclicks) {
    view *ret=this;
    if(v) ret=v->Hit(act, x, y, numclicks);
    if(act==view_LeftUp) Retract();
    return ret;
};

popupwin *popupwin::Create(im *oi, view *vw) {
    popupwin *res=new popupwin;
    im *i=im::CreateTransient(oi, im_OVERRIDEREDIRECT, NULL, 0);
    i->SetView(res);
    i->SetBorderWidth(0);
    res->v=vw;
    res->v->LinkTree(res);
    return res;
}

void popupwin::SetView(view *vw) {
    v=vw;
    v->LinkTree(this);
}

void popupwin::Post(view *origin, long mousex, long mousey, view_MouseAction act, long viewx, long viewy, long w, long h) {
    if(w==0 && h==0 && v) {
	v->DesiredSize(500, 500, view_NoSet, &w, &h);
    }
    GetIM()->ResizeWindow(w,h);
    
    rectangle rect;
    origin->GetIM()->GetLoc(origin, &rect);
    GetIM()->MoveWindow(rect.left + mousex - viewx, rect.top + mousey - viewy);
    GetIM()->ExposeWindow();
    if(v) v->WantInputFocus(v);
    im::ForceUpdate();
  
}

 
void popupwin::LinkTree(class view  *parent)
{
    view::LinkTree( parent);
    if(v) (v)->LinkTree( this);
}

void popupwin::InitChildren()
{
    if(v) (v)->InitChildren();
    else view::InitChildren();
}

void popupwin::FullUpdate(enum view_UpdateType  type, long  left , long  top , long  width , long  height)
{
    struct rectangle r;
    
    if(v) {
	GetLogicalBounds( &r);

	(v)->InsertView( this, &r);

	(v)->FullUpdate( type, left, top, width, height);
    } else view::FullUpdate( type, left, top, width, height);
}

void popupwin::Update()
{
    view::Update();
}

view_DSattributes popupwin::DesiredSize(long  width, long  height, enum view_DSpass  pass, long  *dWidth, long  *dHeight)
{
    if(v) return (v)->DesiredSize( width, height, pass, dWidth, dHeight);
    else return view::DesiredSize( width, height, pass, dWidth, dHeight);
}

void popupwin::GetOrigin(long  width, long  height, long  *originX, long  *originY)
{
    if(v) (v)->GetOrigin( width, height, originX, originY);
    else view::GetOrigin( width, height, originX, originY);
}

void popupwin::WantInputFocus(class view  *requestor)
{
    if((class view *)this == requestor && v) (v)->WantInputFocus( v);
    else view::WantInputFocus( requestor);
}

void popupwin::PostMenus(class menulist  *ml)
{
    view::PostMenus( ml);
}

void popupwin::PostKeyState(class keystate  *ks)
{
    if(lks) {
	(lks)->AddBefore( ks);
	view::PostKeyState( lks);
    } else view::PostKeyState( ks);
}

void popupwin::WantUpdate(class view  *req)
{
    view::WantUpdate( req);
}

void popupwin::Retract() {
    GetIM()->VanishWindow();
}
