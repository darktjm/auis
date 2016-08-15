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
static UNUSED const char rcsid[]="$Header: /afs/cs.cmu.edu/project/atk-src-C++/atk/prefed/RCS/pvalcv.C,v 1.4 1994/11/30 20:42:06 rr2b Stab74 $";
#endif



 

ATK_IMPL("pvalcv.H")


#include "pvalcv.H"
#include "prefval.H"
#include <sbutton.H>
#include "prefsbv.H"
#include <view.H>
#include <atom.H>
#include <observable.H>

#define DATA(self) ((class prefval *)(self)->GetDataObject())


ATKdefineRegistry(pvalcv, pvalsbv, NULL);
#ifndef NORCSID
#endif


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
    char *ps=(DATA(this))->IndexValueString( 0);
    class atom *a;

    if(ps==NULL) return;

    a=atom::Intern(ps);
    if(a==NULL) return;
    
    sbutton::GetTop((sb)->GetDefaultPrefs()) = (a)->Name();

    (sb)->SetChangeFlag(  sbutton_ALLCHANGED|sbutton_SIZECHANGED);
    (sb)->NotifyObservers( observable_OBJECTCHANGED);
}
