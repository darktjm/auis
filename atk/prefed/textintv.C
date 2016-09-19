/* Copyright 1992 by the Andrew Toolkit Consortium and Carnegie Mellon University. All rights Reserved. */

#include <andrewos.h>
ATK_IMPL("textintv.H")


#include "textintv.H"

#include <text.H>
#include <textview.H>
#include "titextv.H"


ATKdefineRegistry(textintv, wrapv, NULL);

textintv::textintv()
{
    class text *t;
    class textview *tv;
    
    t=new text;
    if(t==NULL) THROWONFAILURE( FALSE);
    
    tv=(class textview *)new titextv;
    if(tv==NULL) {
	(t)->Destroy();
	THROWONFAILURE( FALSE);
    }

    ((class titextv *)tv)->SetTextIntv( this);
    (this)->SetData( t);
    (this)->SetView( tv);
    (this)->SetInterfaceView( tv);
    (tv)->SetDataObject( t);
    THROWONFAILURE( TRUE);
    
}

textintv::~textintv()
{
    /* the wrapv class takes care of destroying everything */
}


void textintv::SetDotPosition(long  pos)
{
}


