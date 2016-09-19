/* 
	Copyright IBM Corporation 1988,1991 - All Rights Reserved
	Copyright Carnegie Mellon University 1996
	For full copyright information see: <src or dest>/doc/COPYRITE
*/

#include <andrewos.h>
ATK_IMPL("hwapp.H")
#include "hwapp.H"

#include "im.H"
#include "frame.H"
#include "text.H"
#include "dataobject.H"
#include "view.H"


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
    class text *t;

    if(!(this)->application::Start())
	return FALSE;

    t=new text;
    if(t==NULL)
	return FALSE;

    (t)->InsertCharacters(0,"Hello world!",sizeof("Hello world!")-1);

    if(!makeWindow((class dataobject *)t) ||
       !makeWindow((class dataobject *)t)){
	(t)->Destroy();
	return FALSE;
    }

    return TRUE;
}
