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


