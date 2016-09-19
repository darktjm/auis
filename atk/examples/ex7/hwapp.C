/* 
	Copyright IBM Corporation 1988,1991 - All Rights Reserved
	Copyright Carnegie Mellon University 1996
	For full copyright information see: <src or dest>/doc/COPYRITE
*/

#include <andrewos.h>
ATK_IMPL("hwapp.H")
#include "hwapp.H"

#include "im.H"
#include "hwview.H"


ATKdefineRegistry(helloworldapp, application, NULL);

boolean helloworldapp::Start()
{
    class helloworldview *hwv;
    class view *applayer;
    class im *im;

    if(!(this)->application::Start())
	return FALSE;

    hwv=new helloworldview;
    if(hwv==NULL)
	return FALSE;

    applayer=(hwv)->GetApplicationLayer();
    if(applayer==NULL){
	(hwv)->Destroy();
	return FALSE;
    }

    im=im::Create(NULL);
    if(im==NULL){
	(hwv)->DeleteApplicationLayer(applayer);
	(hwv)->Destroy();
	return FALSE;
    }

    (im)->SetView(applayer);

    (hwv)->WantInputFocus(hwv);

    return TRUE;
}
