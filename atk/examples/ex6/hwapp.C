/* 
	Copyright IBM Corporation 1988,1991 - All Rights Reserved
	Copyright Carnegie Mellon University 1996
	For full copyright information see: <src or dest>/doc/COPYRITE
*/

#include <andrewos.h>
ATK_IMPL("hwapp.H")
#include "hwapp.H"

#include "im.H"
#include "scroll.H"
#include "hwview.H"


ATKdefineRegistryNoInit(helloworldapp, application);

boolean helloworldapp::Start()
{
    class helloworldview *hwv;
    class im *im;
    class scroll *scroll;

    if(!(this)->application::Start())
	return FALSE;

    hwv=new helloworldview;
    if(hwv==NULL)
	return FALSE;

    scroll=scroll::Create(hwv,scroll_LEFT|scroll_BOTTOM);
    if(scroll==NULL){
	(hwv)->Destroy();
	return FALSE;
    }

    im=im::Create(NULL);
    if(im==NULL){
	(hwv)->Destroy();
	(scroll)->Destroy();
	return FALSE;
    }

    (im)->SetView(scroll);

    (hwv)->WantInputFocus(hwv);

    return TRUE;
}
