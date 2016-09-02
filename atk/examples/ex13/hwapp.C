/* 
	Copyright IBM Corporation 1988,1991 - All Rights Reserved
	Copyright Carnegie Mellon University 1996
	For full copyright information see: <src or dest>/config/COPYRITE

	$Disclaimer: 
 * Permission to use, copy, modify, and distribute this software and its 
 * documentation for any purpose and without fee is hereby granted, provided 
 * that the above copyright notice appear in all copies and that both that 
 * copyright notice and this permission notice appear in supporting 
 * documentation, and that the name of IBM not be used in advertising or 
 * publicity pertaining to distribution of the software without specific, 
 * written prior permission. 
 *                         
 * THE COPYRIGHT HOLDERS DISCLAIM ALL WARRANTIES WITH REGARD 
 * TO THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF 
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL ANY COPYRIGHT 
 * HOLDER BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL 
 * DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, 
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE 
 * OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION 
 * WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 * 
 *  $
*/

#include <andrewos.h>
ATK_IMPL("hwapp.H")
#include "hwapp.H"

#include "dataobject.H"
#include "view.H"
#include "frame.H"
#include "im.H"
#include "helloworld.H"


ATKdefineRegistry(helloworldapp, application, NULL);
static boolean makeWindow(class dataobject  *dobj);


static boolean makeWindow(class dataobject  *dobj)
{
    class view *v;
    class view *applayer;
    class frame *theFrame;
    class im *im;

    v=(class view *)ATK::NewObject((dobj)->ViewName());
    if(v==NULL)
	return FALSE;

    applayer=(v)->GetApplicationLayer();
    if(applayer==NULL) {
	(v)->Destroy();
	return FALSE;
    }

    theFrame=new frame;
    if(theFrame==NULL) {
	(v)->DeleteApplicationLayer(applayer);
	return FALSE;
    }

    im=im::Create(NULL);
    if(im==NULL) {
	(theFrame)->Destroy();
	return FALSE;
    }

    (v)->SetDataObject(dobj);
    (theFrame)->SetView(applayer);
    (im)->SetView(theFrame);

    (v)->WantInputFocus(v);

    return TRUE;

}

boolean helloworldapp::Start()
{
    class helloworld *hw;

    if(!(this)->application::Start())
	return FALSE;

    hw=new helloworld;
    if(hw==NULL)
	return FALSE;

    if(!makeWindow((class dataobject *)hw) ||
       !makeWindow((class dataobject *)hw)){
	(hw)->Destroy();
	return FALSE;
    }

    return TRUE;
}
