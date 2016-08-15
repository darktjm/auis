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
static UNUSED const char rcsid[]="$Header: /afs/cs.cmu.edu/project/atk-src-C++/atk/prefed/RCS/pvaltvc.C,v 1.6 1994/11/30 20:42:06 rr2b Stab74 $";
#endif



 

ATK_IMPL("pvaltvc.H")
#include <math.h>


#include "pvaltvc.H"

#include <prefval.H>

#include <proctable.H>
#include <keymap.H>
#include <keystate.H>
#include <menulist.H>
#include <bind.H>
#include <observable.H>
#include <text.H>
#include <textview.H>

#define DATA(self) ((class prefval *)(self)->GetDataObject())
#define TEXT(self) ((self)->GetText())

static class menulist *pvaltvcMenus=NULL;
static class keymap *pvaltvcKeymap=NULL;

#undef MIN
#define MIN(a,b) ((a<b)?a:b)


ATKdefineRegistry(pvaltvc, pvaltv, pvaltvc::InitializeClass);
#ifndef NORCSID
#endif
static void pvaltvcUpdate(class pvaltvc  *self, long  rock);


 void pvaltvc::Select(int  ind)
{
    int i;
    int max=(DATA(this))->GetListMax();
    int size=(DATA(this))->GetListSize();

    if(ind<(DATA(this))->GetChoiceListSize()) {
	for(i=MIN(size,max-1)-1;i>=0;i--) {
	    struct prefval_value *v= (DATA(this))->GetIndexValue( i);
	    (DATA(this))->SetIndexValue( i+1, v);
	    (DATA(this))->FreeValue( (DATA(this))->GetIndexValue( i));
	}
	(DATA(this))->SetIndexValue( 0, &DATA(this)->cvalues[(DATA(this))->GetChoiceListSize() - ind - 1]);
	(DATA(this))->NotifyObservers( prefval_ValuesChanged);
    }
}


static void pvaltvcUpdate(class pvaltvc  *self, long  rock)
{
    (self)->Select( (self)->Locate( ((self)->GetTextView())->GetDotPosition()));
}

static struct bind_Description pvaltvcBindings[]={
    {
	"pvaltvc-update",
	" ",
	' ',
	0,
	0,
	0,
	(proctable_fptr)pvaltvcUpdate,
	"sets the preference value from the text in the entry buffer.",
	"pvaltvc"
    },
    {
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0
    }
};
    
    
boolean pvaltvc::InitializeClass()
{
    pvaltvcMenus=new menulist;
    pvaltvcKeymap=new keymap;

    if(pvaltvcMenus == NULL || pvaltvcKeymap==NULL) {
	if(pvaltvcMenus != NULL) delete pvaltvcMenus;
	if(pvaltvcKeymap != NULL) delete pvaltvcKeymap;
	return FALSE;
    }
    
    bind::BindList(pvaltvcBindings, pvaltvcKeymap, pvaltvcMenus, &pvaltvc_ATKregistry_ );
    return TRUE;
    
    
}

pvaltvc::pvaltvc()
{
	ATKinit;

    this->ks=keystate::Create(this, pvaltvcKeymap);
    if(this->ks==NULL) THROWONFAILURE( FALSE);

    this->menulistp=(pvaltvcMenus)->DuplicateML( this);

    if(this->menulistp==NULL) {
	delete this->ks;
	THROWONFAILURE( FALSE);
    }
    
    (this)->SetInterfaceView( ((this)->GetTextView())->GetApplicationLayer( ));
    
    THROWONFAILURE( TRUE);
}

pvaltvc::~pvaltvc()
{
	ATKinit;

    if(this->ks) delete this->ks;
    if(this->menulistp) delete this->menulistp;
}

void pvaltvc::UpdateText(long  val)
{
    class prefval *pvd=DATA(this);
    class text *pvt=TEXT(this);
    class textview *tv=(this)->GetTextView();
    char *vs;
    int i;
    long pos=(tv)->GetDotPosition();
    long len=(tv)->GetDotLength();
    
    if(val==prefval_ChoicesChanged || val==prefval_Generic) {
	(pvt)->Clear();
	for(i=(pvd)->GetChoiceListSize()-1;i>=0;i--) {
	    if((pvd)->GetIndexChoiceName( i)) (pvt)->InsertCharacters( (pvt)->GetLength(), (pvd)->GetIndexChoiceName( i), strlen((pvd)->GetIndexChoiceName( i)));
	    if(i>0) (pvt)->InsertCharacters( (pvt)->GetLength(), "\n", 1);
	}
	if(pos+len<=(pvt)->GetLength()) {
	    (tv)->SetDotPosition( pos);
	    (tv)->SetDotLength( len);
	    (tv)->FrameDot( pos+len);
	    (tv)->WantUpdate( tv);
	}

    }
}

void pvaltvc::UpdateValue()
{
}

 
void pvaltvc::ObservedChanged(class observable  *changed, long  val)
{
    (this)->pvaltv::ObservedChanged( changed, val);  
}


class keystate *pvaltvc::Keys()
{
    return this->ks;
}
	
void pvaltvc::SetDataObject(class dataobject  *d)
{
    (this)->pvaltv::SetDataObject( d);
    (this)->ObservedChanged( d, prefval_Generic);
    (TEXT(this))->AddObserver( this);
}

class view *pvaltvc::Hit(enum view_MouseAction  action, long  x, long  y, long  numberOfClicks)
{
    class view *result = (this)->pvaltv::Hit( action, x, y, numberOfClicks);
    class textview *tv=(this)->GetTextView();

    if(action==view_LeftUp) {
	(this)->Select( (this)->Locate( (tv)->GetDotPosition()));
    }
    return (result!=(class view *)tv)?result:(class view *)this;

}

