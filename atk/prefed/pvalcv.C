/* Copyright 1992 by the Andrew Toolkit Consortium and Carnegie Mellon University. All rights Reserved. */

#include <andrewos.h>
ATK_IMPL("pvalcv.H")


#include "pvalcv.H"
#include "prefval.H"
#include <sbutton.H>
#include "prefsbv.H"
#include <view.H>
#include <atom.H>
#include <observable.H>

#define DATA(self) ((class prefval *)(self)->GetDataObject())


ATKdefineRegistryNoInit(pvalcv, pvalsbv);

pvalcv::pvalcv()
{
    class sbutton *sb=(this)->GetSButton();
    (sb)->SetLabel( 0, "      ");
    sbutton::GetStyle((sb)->GetDefaultPrefs())= sbutton_PLAINBOX;
    THROWONFAILURE( TRUE);
}

pvalcv::~pvalcv()
{
    /* the wrapv class takes care of destroying everything */
}

void pvalcv::UpdateSButton()
{
    class sbutton *sb=(this)->GetSButton();
    const char *ps=(DATA(this))->IndexValueString( 0);
    const class atom *a;

    if(ps==NULL) return;

    a=atom::Intern(ps);
    if(a==NULL) return;
    
    sbutton::GetTop((sb)->GetDefaultPrefs()) = (a)->Name();

    (sb)->SetChangeFlag(  sbutton_ALLCHANGED|sbutton_SIZECHANGED);
    (sb)->NotifyObservers( observable::OBJECTCHANGED);
}
