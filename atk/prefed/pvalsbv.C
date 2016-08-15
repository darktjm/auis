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

#ifndef NORCSID
#define NORCSID
static UNUSED const char rcsid[]="$Header: /afs/cs.cmu.edu/project/atk-src-C++/atk/prefed/RCS/pvalsbv.C,v 1.5 1994/11/30 20:42:06 rr2b Stab74 $";
#endif



 

ATK_IMPL("pvalsbv.H")


#include "pvalsbv.H"
#include "prefval.H"
#include <sbutton.H>
#include "prefsbv.H"
#include <view.H>

#define DATA(self) ((class prefval *)(self)->GetDataObject())


ATKdefineRegistry(pvalsbv, wrapv, NULL);
#ifndef NORCSID
#endif


pvalsbv::pvalsbv()
{
    class sbutton *t;
    class prefsbv *tv;
    
    t=new sbutton;
    if(t==NULL) THROWONFAILURE( FALSE);
    
    tv=new prefsbv;
    if(tv==NULL) {
	(t)->Destroy();
	THROWONFAILURE( FALSE);
    }

    (this)->SetData( t);
    (this)->SetView( tv);
    (this)->SetInterfaceView( tv);
    (tv)->SetDataObject( t);
    THROWONFAILURE( TRUE);
    
}

pvalsbv::~pvalsbv()
{
    /* the wrapv class takes care of destroying everything */
}

void pvalsbv::UpdateSButton()
{
}

void pvalsbv::SetDataObject(class dataobject  *d)
{
    (this)->wrapv::SetDataObject( d);
    (this)->UpdateSButton();
}

void pvalsbv::ObservedChanged(class observable  *changed, long  val)
{
    (this)->wrapv::ObservedChanged( changed, val);
    if(changed==(class observable *)DATA(this)) {
	(this)->UpdateSButton();
    }
}
