/* 
	Copyright IBM Corporation 1988,1991 - All Rights Reserved
	Copyright Carnegie Mellon University 1996
	For full copyright information see: <src or dest>/doc/COPYRITE
*/

#include <andrewos.h>
ATK_IMPL("hwapp.H")
#include "hwapp.H"

#include "dataobject.H"
#include "view.H"
#include "im.H"
#include "frame.H"
#include "lpair.H"
#include "text.H"
#include "textview.H"
#include "style.H"
#include "fontdesc.H"
#include "helloworld.H"


ATKdefineRegistryNoInit(helloworldapp, application);
static class view *appLayerOrDestroy(class view  *v);
static boolean makeSplitWindow(class dataobject  *dobj1,class dataobject  *dobj2);


static class view *appLayerOrDestroy(class view  *v)
{
    if(v==NULL)
	return NULL;
    else{
	class view *al=(v)->GetApplicationLayer();

	if(al==NULL){
	    (v)->Destroy();
	    return NULL;
	}

	return al;
    }
}

static boolean makeSplitWindow(class dataobject  *dobj1,class dataobject  *dobj2)
{
    class view *v1,*v2;
    class view *al1,*al2,*lpAl;
    class frame *theFrame;
    class im *im;
    class lpair *lp;

    al1=appLayerOrDestroy(v1=(class view *)ATK::NewObject((dobj1)->ViewName()));
    if(al1==NULL)
	return FALSE;

    al2=appLayerOrDestroy(v2=(class view *)ATK::NewObject((dobj2)->ViewName()));
    if(al2==NULL) {
	(v1)->DeleteApplicationLayer(al1);
    	(v1)->Destroy();
	return FALSE;
    }

    lpAl=appLayerOrDestroy((class view *)(lp=new lpair));
    if(lpAl==NULL) {
	(v2)->DeleteApplicationLayer(al2);
    	(v2)->Destroy();
	return FALSE;
    }
	
    /* this call makes a left/right split, with the given
     * percentage allocated to the left view
     */
    (lp)->HSplit(al1,al2,40 /* percent */,TRUE /* moveable boundary */);

    theFrame=new frame;
    if(theFrame==NULL)  {
	(lp)->DeleteApplicationLayer(lpAl);
    	(lp)->Destroy();
	return FALSE;
    }

    im=im::Create(NULL);
    if(im==NULL) {
	(theFrame)->Destroy();
	return FALSE;
    }
	
    (v1)->SetDataObject(dobj1);
    (v2)->SetDataObject(dobj2);
    (theFrame)->SetView(lpAl);
    (im)->SetView(theFrame);

    (v1)->WantInputFocus(v1);

    return TRUE;

}

boolean helloworldapp::Start()
{
    class helloworld *hw;
    class text *t;
    class style *bold,*italic;

    if(!(this)->application::Start())
	return FALSE;

    hw=new helloworld;
    if(hw==NULL)
	return FALSE;

    t=new text;
    if(t==NULL) {
	(hw)->Destroy();
	return FALSE;
    }

    bold=new style;
    if(bold==NULL){
	(t)->Destroy();
	return FALSE;
    }
    (bold)->AddNewFontFace(fontdesc_Bold);

    italic=new style;
    if(italic==NULL) {
	delete bold;
	return FALSE;
   }
    (italic)->AddNewFontFace(fontdesc_Italic);

    (t)->InsertCharacters(0,"Hello world!",sizeof("Hello world!")-1);
    (t)->AddStyle(0,5,bold);
    (t)->AddStyle(6,5,italic);

    if(!makeSplitWindow((class dataobject *)hw,(class dataobject *)t) ||
       !makeSplitWindow((class dataobject *)hw,(class dataobject *)t))  {
	delete italic;
	return FALSE;
    }

    return TRUE;

}
