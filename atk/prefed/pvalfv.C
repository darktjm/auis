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
static UNUSED const char rcsid[]="$Header: /afs/cs.cmu.edu/project/atk-src-C++/atk/prefed/RCS/pvalfv.C,v 1.4 1994/11/30 20:42:06 rr2b Stab74 $";
#endif



 

ATK_IMPL("pvalfv.H")


#include "pvalfv.H"
#include "prefval.H"
#include <sbutton.H>
#include "prefsbv.H"
#include <view.H>
#include <fontdesc.H>

#define DATA(self) ((class prefval *)(self)->GetDataObject())


ATKdefineRegistry(pvalfv, pvalsbv, NULL);
#ifndef NORCSID
#endif


pvalfv::pvalfv()
{
    class sbutton *sb=(this)->GetSButton();
    struct sbutton_prefs *prefs=(sb)->GetDefaultPrefs();
    (sb)->SetLabel( 0, "SampleText");
    sbutton::GetStyle(prefs)= sbutton_PLAIN;
    THROWONFAILURE( TRUE);
    
}

pvalfv::~pvalfv()
{
    /* the wrapv class takes care of destroying everything */
}

void pvalfv::UpdateSButton()
{
    class sbutton *sb=(this)->GetSButton();
    struct sbutton_prefs *prefs=(sb)->GetDefaultPrefs();
    class fontdesc *font=NULL;
    char *name=(DATA(this))->IndexValueString( 0);
    char buf2[1024];
    long style, size;
    if(name==NULL) return;
    if(fontdesc::ExplodeFontName(name, buf2, sizeof(buf2), &style, &size))	{
	font=fontdesc::Create(buf2,style,size);
	sbutton::GetFont(prefs) = font;
    }
    (sb)->SetChangeFlag( sbutton_ALLCHANGED|sbutton_FONTCHANGED);
    (sb)->NotifyObservers( observable_OBJECTCHANGED);
}

