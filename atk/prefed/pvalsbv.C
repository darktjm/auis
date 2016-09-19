/* Copyright 1992 by the Andrew Toolkit Consortium and Carnegie Mellon University. All rights Reserved. */

#include <andrewos.h>
ATK_IMPL("pvalsbv.H")


#include "pvalsbv.H"
#include "prefval.H"
#include <sbutton.H>
#include "prefsbv.H"
#include <view.H>

#define DATA(self) ((class prefval *)(self)->GetDataObject())


ATKdefineRegistry(pvalsbv, wrapv, NULL);

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
