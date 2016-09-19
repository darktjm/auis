/* Copyright 1992 by the Andrew Toolkit Consortium and Carnegie Mellon University. All rights Reserved. */

#include <andrewos.h>
ATK_IMPL("pvaltvfc.H")
#include <math.h>


#include "pvaltvfc.H"

#include <prefval.H>

#include <proctable.H>
#include <keymap.H>
#include <keystate.H>
#include <menulist.H>
#include <bind.H>
#include <observable.H>
#include <text.H>
#include <textview.H>

#define DATA(self) ((class prefval *)(self)->GetDataObject())
#define TEXT(self) ((self)->GetText())
#undef MIN
#define MIN(a,b) ((a<b)?a:b)


ATKdefineRegistry(pvaltvfc, pvaltvc, pvaltvfc::InitializeClass);

 void pvaltvfc::Select(int  ind)
{
    if(ind<(DATA(this))->GetChoiceListSize()) {
	(DATA(this))->SetIndexValue( (DATA(this))->GetCurrentItem(),  &DATA(this)->cvalues[(DATA(this))->GetChoiceListSize() - ind - 1]);
	(DATA(this))->NotifyObservers( prefval_ValuesChanged);
    }
}
    
boolean pvaltvfc::InitializeClass()
{
    return TRUE;
}

pvaltvfc::pvaltvfc()
{
	ATKinit;

    THROWONFAILURE( TRUE);
}

pvaltvfc::~pvaltvfc()
{
	ATKinit;

}

