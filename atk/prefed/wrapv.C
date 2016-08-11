
/* Copyright 1992 by the Andrew Toolkit Consortium and Carnegie Mellon University. All rights Reserved. */

/*
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

#ifndef NORCSID
#define NORCSID
static char rcsid[]="$Header: /afs/cs.cmu.edu/project/atk-src-C++/atk/prefed/RCS/wrapv.C,v 1.5 1994/11/30 20:42:06 rr2b Stab74 $";
#endif



 

#include <andrewos.h>
ATK_IMPL("wrapv.H")


#include "wrapv.H"

#include <dataobject.H>
#include <view.H>
#include <menulist.H>
#include <keystate.H>


ATKdefineRegistry(wrapv, view, NULL);
#ifndef NORCSID
#endif


wrapv::wrapv()
{
    this->intv=this->tv=NULL;
    this->t=NULL;
    THROWONFAILURE( TRUE);
}

wrapv::~wrapv()
{
  if(this->intv!=(class view *)this->tv) {
	(this->tv)->DeleteApplicationLayer( this->intv);
	this->intv=NULL;
    }
    if(this->t) {
	(this->t)->Destroy();
	this->t=NULL;
    }
    if(this->tv) {
	(this->tv)->Destroy();
	this->tv=NULL;
    }
}
 
void wrapv::LinkTree(class view  *parent)
{
    (this)->view::LinkTree( parent);
    if(this->intv) (this->intv)->LinkTree( this);
}

void wrapv::InitChildren()
{
    if(this->intv) (this->intv)->InitChildren();
    else (this)->view::InitChildren();
}

void wrapv::FullUpdate(enum view_UpdateType  type, long  left , long  top , long  width , long  height)
{
    struct rectangle r;
    int i;
    
    if(this->intv) {
	(this)->GetLogicalBounds( &r);

	(this->intv)->InsertView( this, &r);

	(this->intv)->FullUpdate( type, left, top, width, height);
    } else (this)->view::FullUpdate( type, left, top, width, height);
}

void wrapv::Update()
{
    if(this->intv) (this->intv)->Update();
    if(this->tv) (this->tv)->Update();
    else (this)->view::Update();
}


class view *wrapv::Hit(enum view_MouseAction  action, long  x, long  y, long  numberOfClicks)
{
    if(this->intv) return (this->intv)->Hit( action, x, y, numberOfClicks);
    else return (this)->view::Hit( action, x, y, numberOfClicks);
}

view_DSattributes wrapv::DesiredSize(long  width, long  height, enum view_DSpass  pass, long  *dWidth, long  *dHeight)
{
    if(this->intv) return (this->intv)->DesiredSize( width, height, pass, dWidth, dHeight);
    else return (this)->view::DesiredSize( width, height, pass, dWidth, dHeight);
}

void wrapv::GetOrigin(long  width, long  height, long  *originX, long  *originY)
{
    if(this->intv) (this->intv)->GetOrigin( width, height, originX, originY);
    else (this)->view::GetOrigin( width, height, originX, originY);
}

void wrapv::WantInputFocus(class view  *requestor)
{
    if((class view *)this == requestor && this->tv) (this->tv)->WantInputFocus( this->tv);
    else (this)->view::WantInputFocus( requestor);
}

void wrapv::PostMenus(class menulist  *ml)
{
    class menulist *lml=(this)->Menus();

    if(lml) {
	if(ml) (lml)->ChainAfterML( ml, (long)ml);
	(this)->view::PostMenus( lml);
    } else (this)->view::PostMenus( ml);
}

void wrapv::PostKeyState(class keystate  *ks)
{
    class keystate *lks=(this)->Keys();
    if(lks) {
	(lks)->AddBefore( ks);
	(this)->view::PostKeyState( lks);
    } else (this)->view::PostKeyState( ks);
}

class keystate *wrapv::Keys()
{
    return NULL;
}

class menulist *wrapv::Menus()
{
    return NULL;
}

void wrapv::WantUpdate(class view  *req)
{
    (this)->view::WantUpdate( this);
}