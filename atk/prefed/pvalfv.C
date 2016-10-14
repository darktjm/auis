/* Copyright 1992 by the Andrew Toolkit Consortium and Carnegie Mellon University. All rights Reserved. */

#include <andrewos.h>
ATK_IMPL("pvalfv.H")


#include "pvalfv.H"
#include "prefval.H"
#include <sbutton.H>
#include "prefsbv.H"
#include <view.H>
#include <fontdesc.H>

#define DATA(self) ((class prefval *)(self)->GetDataObject())


ATKdefineRegistryNoInit(pvalfv, pvalsbv);

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
    const char *name=(DATA(this))->IndexValueString( 0);
    char buf2[1024];
    long style, size;
    if(name==NULL) return;
    if(fontdesc::ExplodeFontName(name, buf2, sizeof(buf2), &style, &size))	{
	font=fontdesc::Create(buf2,style,size);
	sbutton::GetFont(prefs) = font;
    }
    (sb)->SetChangeFlag( sbutton_ALLCHANGED|sbutton_FONTCHANGED);
    (sb)->NotifyObservers( observable::OBJECTCHANGED);
}

