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
#include "hwview.H"


ATKdefineRegistry(helloworldapp, application, NULL);

boolean helloworldapp::Start()
{
    class helloworldview *hwv;
    class view *applayer;
    class frame *theFrame;
    class im *im;

    if(!(this)->application::Start())
	return FALSE;

    hwv=new helloworldview;
    if(hwv==NULL)
	return FALSE;

    applayer=(hwv)->GetApplicationLayer();
    if(applayer==NULL) {
	(hwv)->Destroy();
	return FALSE;
    }

    theFrame=new frame;
    if(theFrame==NULL) {
	(hwv)->DeleteApplicationLayer(applayer);
	return FALSE;
    }

    im=im::Create(NULL);
    if(im==NULL) {
	(theFrame)->Destroy();
	return FALSE;
    }

    (theFrame)->SetView(applayer);
    (im)->SetView(theFrame);

    (hwv)->WantInputFocus(hwv);

    return TRUE;

}
