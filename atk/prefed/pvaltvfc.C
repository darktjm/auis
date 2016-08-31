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

