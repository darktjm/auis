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

#include <andrewos.h>
ATK_IMPL("pvalbv.H")


#include "pvalbv.H"
#include "prefval.H"
#include <sbutton.H>
#include "prefsbv.H"
#include <view.H>

#define DATA(self) ((class prefval *)(self)->GetDataObject())

#define ON 1
#define OFF 0


ATKdefineRegistry(pvalbv, pvalsbv, NULL);

pvalbv::pvalbv()
{
    class sbutton *t=(this)->GetSButton();
    (t)->SetLabel( OFF, "No");
    (t)->SetLabel( ON, "Yes");
    (t)->SetLayout( 1, 2, sbutton_GrowColumns);
    this->activated=(-1);
    THROWONFAILURE( TRUE);
    
}

pvalbv::~pvalbv()
{
    /* the wrapv class takes care of destroying everything */
}


class view *pvalbv::Hit(enum view_MouseAction  action, long  x, long  y, long  numberOfClicks)
{
    if(action==view_LeftDown) this->activated=(this)->GetSButton()->activated;

    (this)->pvalsbv::Hit( action, x, y, numberOfClicks);
    
    if((DATA(this))->GetValue()==NULL) return (class view *)this;

    if((action == view_LeftUp) && (this->activated!=(this)->GetSButton()->activated)) {

	(DATA(this))->GetValue()->v.bval=((this)->GetSButton()->activated==ON);
	(DATA(this))->SetModified();

	(DATA(this))->NotifyObservers( prefval_ValuesChanged);
    }
    return (class view *)this;    
}


void pvalbv::UpdateSButton()
{
    if((DATA(this))->GetType()==prefval_Boolean) {
	if((DATA(this))->GetValue()) {
	    if((DATA(this))->GetValue()->v.bval) ((this)->GetSButton())->ActivateButton( ON);
	    else ((this)->GetSButton())->ActivateButton( OFF);
	}
    }
}
