/* Copyright 1992 by the Andrew Toolkit Consortium and Carnegie Mellon University. All rights Reserved. */

#include <andrewos.h>
ATK_IMPL("pvaltv.H")
#include <math.h>


#include "pvaltv.H"

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

static class menulist *pvaltvMenus=NULL;
static class keymap *pvaltvKeymap=NULL;

#undef MIN
#define MIN(a,b) (((a)<(b))?a:b)


ATKdefineRegistry(pvaltv, textintv, pvaltv::InitializeClass);
static void pvaltvUpdate(class pvaltv  *self, long  rock);


static void pvaltvUpdate(class pvaltv  *self, long  rock)
{
    (self)->UpdateValue();
}

static struct bind_Description pvaltvBindings[]={
    {
	"pvaltv-update",
	"\r",
	13,
	0,
	0,
	0,
	(proctable_fptr)pvaltvUpdate,
	"sets the preference value from the text in the entry buffer.",
	"pvaltv"
    },
    {
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	NULL
    }
};
    
    
boolean pvaltv::InitializeClass()
{
    pvaltvMenus=new menulist;
    pvaltvKeymap=new keymap;

    if(pvaltvMenus == NULL || pvaltvKeymap==NULL) {
	if(pvaltvMenus != NULL) delete pvaltvMenus;
	if(pvaltvKeymap != NULL) delete pvaltvKeymap;
	return FALSE;
    }
    
    bind::BindList(pvaltvBindings, pvaltvKeymap, pvaltvMenus, &pvaltv_ATKregistry_ );
    return TRUE;
    
    
}
pvaltv::pvaltv()
{
	ATKinit;

    this->ks=keystate::Create(this, pvaltvKeymap);
    if(this->ks==NULL) THROWONFAILURE( FALSE);

    this->menulistp=(pvaltvMenus)->DuplicateML( this);

    if(this->menulistp==NULL) {
	delete this->ks;
	THROWONFAILURE( FALSE);
    }
    THROWONFAILURE( TRUE);
}

pvaltv::~pvaltv()
{
	ATKinit;

    if(this->ks) delete this->ks;
    if(this->menulistp) delete this->menulistp;
}

void pvaltv::UpdateText(long  val)
{
    class prefval *pvd=DATA(this);
    class text *pvt=TEXT(this);
    class textview *tv=(this)->GetTextView();
    const char *vs;
    long pos=(tv)->GetDotPosition();

    if(val==prefval_ValuesChanged || val==prefval_Generic) {
	vs=(pvd)->PreferenceString();
	if(vs==NULL) return;
	vs=strchr(vs, ':');
	if(vs==NULL) return;
	vs++;
	if(*vs==' ') vs++;
	(pvt)->Clear();
	(pvt)->InsertCharacters( 0, vs, strlen(vs));
	(tv)->SetDotPosition( pos);
	(tv)->FrameDot( pos);
    }
}

void pvaltv::UpdateValue()
{
    class text *pvt=TEXT(this);
    unsigned long len=(pvt)->GetLength();
    char buf[1024];
    (pvt)->CopySubString( 0, len>sizeof(buf)-1?sizeof(buf)-1:len, buf, FALSE);
    (DATA(this))->SetFromPreferenceString( buf);
    (DATA(this))->NotifyObservers( prefval_ValuesChanged);
}

void pvaltv::ObservedChanged(class observable  *changed, long  val)
{
    if(changed==(class observable *)DATA(this) && val!=observable::OBJECTDESTROYED) {
	(this)->UpdateText( val);
    }
    (this)->textintv::ObservedChanged( changed, val);
}

void pvaltv::SetDataObject(class dataobject  *d)
{
    (this)->textintv::SetDataObject( d);
    (this)->ObservedChanged( d, prefval_Generic);
}

class keystate *pvaltv::Keys()
{
    return this->ks;
}

class menulist *pvaltv::Menus()
{
    (this->menulistp)->ClearChain();
    return this->menulistp;
}

int pvaltv::Locate(long  pos)
{
    class text *t=(this)->GetText();
    long len=(t)->GetLength();
    long i=0;
    int count=0;
    do {
	long j;
	j=(t)->Index( i, '\n', len-i);
	if(j<0 || (pos>=i && pos<=j)) break;
	count++;
	i=j+1;
    } while(1);
    return count;
}
