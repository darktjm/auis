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

#ifndef NORCSID
#define NORCSID
static char rcsid[]="$Header: /afs/cs.cmu.edu/project/atk-src-C++/atk/prefed/RCS/pvaltvl.C,v 1.7 1994/11/30 20:42:06 rr2b Stab74 $";
#endif



 

#include <andrewos.h>
ATK_IMPL("pvaltvl.H")
#include <math.h>


#include "pvaltvl.H"

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

static class menulist *pvaltvlMenus=NULL;
static class keymap *pvaltvlKeymap=NULL;



ATKdefineRegistry(pvaltvl, pvaltv, pvaltvl::InitializeClass);
#ifndef NORCSID
#endif
static void pvaltvlUpdate(class pvaltvl  *self, long  rock);


static void pvaltvlUpdate(class pvaltvl  *self, long  rock)
{
}

static struct bind_Description pvaltvlBindings[]={
    {
	"pvaltvl-update",
	"\r",
	'\r',
	0,
	0,
	0,
	(proctable_fptr)pvaltvlUpdate,
	"sets the preference value from the text in the entry buffer.",
	"pvaltvl"
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
    
    
boolean pvaltvl::InitializeClass()
{
    pvaltvlMenus=new menulist;
    pvaltvlKeymap=new keymap;

  /*  if(pvaltvlMenus == NULL || pvaltvlKeymap==NULL) {
	if(pvaltvlMenus != NULL) menulist_Destroy(pvaltvlMenus);
	if(pvaltvlKeymap != NULL) keymap_Destroy(pvaltvlKeymap);
	return FALSE;
    }
    
    bind_BindList(pvaltvlBindings, pvaltvlKeymap, pvaltvlMenus, &pvaltvl_classinfo); */
    return TRUE;
    
    
}
pvaltvl::pvaltvl()
{
	ATKinit;

    this->ks=keystate::Create(this, pvaltvlKeymap);
    if(this->ks==NULL) THROWONFAILURE( FALSE);

    this->menulistp=(pvaltvlMenus)->DuplicateML( this);

    if(this->menulistp==NULL) {
	delete this->ks;
	THROWONFAILURE( FALSE);
    }
    
    THROWONFAILURE( TRUE);
}

pvaltvl::~pvaltvl()
{
	ATKinit;

    if(this->ks) delete this->ks;
    if(this->menulistp) delete this->menulistp;
}

void pvaltvl::SetDotPosition(long  pos)
{
    class text *t=(this)->GetText();
    int count=(this)->Locate( pos);
    if(count<(DATA(this))->GetListSize()) {
	(DATA(this))->SetCurrentItem( (DATA(this))->GetListSize() - count - 1);
    }
    (this)->pvaltv::SetDotPosition( pos);
}

void pvaltvl::UpdateText(long  val)
{
    class prefval *pvd=DATA(this);
    class text *pvt=TEXT(this);
    class textview *tv=(this)->GetTextView();
    char *vs;
    int i;
    if(val==prefval_ValuesChanged || val==prefval_Generic) {
	long pos=(tv)->GetDotPosition();
	long len=(tv)->GetDotLength();
	(pvt)->Clear();
	for(i=(pvd)->GetListSize()-1;i>=0;i--) {
	    vs=(pvd)->IndexValueString( i);
	    if(vs) (pvt)->InsertCharacters( (pvt)->GetLength(), vs, strlen(vs));
	    if(i>0) (pvt)->InsertCharacters( (pvt)->GetLength(), "\n", 1);
	}
	(tv)->SetDotPosition( pos);
	(tv)->SetDotLength( len);
	(tv)->FrameDot( pos+len);
    }
}

void pvaltvl::UpdateValue()
{
    class prefval *pvd=DATA(this);
    class text *pvt=TEXT(this);
    struct prefval_value *v;
    class textview *tv=(this)->GetTextView();
    long lines=0, i;
    long tpos=(tv)->GetTopPosition();
    long dpos=(tv)->GetDotPosition();
    long pos=0, npos;
    struct prefval_value *pvl=NULL;
    
    do {
	long len;
	npos=(pvt)->Index( pos, '\n', (pvt)->GetLength()-pos);
	if(npos>=0) len=npos-pos;
	else len=(pvt)->GetLength()-pos;
	lines++;
	pos=npos+1;
    } while(npos>=0);

    if(lines==0) return;

    if(lines>(pvd)->GetListMax()) lines=(pvd)->GetListMax();

    pvl=(struct prefval_value *)malloc(sizeof(*pvl)*lines);
    if(pvl==NULL) return;
    
    for(i=0;i<lines;i++) (pvd)->InitValue( pvl+i);

    i=lines-1;
    do {
	long len;
	char buf[1024];
	npos=(pvt)->Index( pos, '\n', (pvt)->GetLength()-pos);
	if(npos>=0) len=npos-pos;
	else len=(pvt)->GetLength()-pos;
	if(len>0) {
	    (pvt)->CopySubString( pos, len, buf, FALSE);
	    v=(pvd)->StringToValue( buf);
	    (pvd)->CopyValue( pvl+i, v);
	} else pvl[i].set=FALSE;
	i--;
	pos=npos+1;
    } while(npos>=0 && i>=0);
    (pvd)->SetValues( lines, pvl);
    for(i=0;i<lines;i++) {
	(pvd)->FreeValue( pvl+i);
    }
    free(pvl);
    (pvd)->NotifyObservers( prefval_ValuesChanged);
    (tv)->SetTopPosition( tpos);
    (tv)->SetDotPosition( dpos);
    (tv)->FrameDot( dpos);
}

 
void pvaltvl::ObservedChanged(class observable  *changed, long  val)
{

    (this)->pvaltv::ObservedChanged( changed, val);
    
    if(val==observable_OBJECTDESTROYED) return;

    if((class text *)changed==TEXT(this)) (this)->UpdateValue();
}


class keystate *pvaltvl::Keys()
{
    return NULL;
}
	
void pvaltvl::SetDataObject(class dataobject  *d)
{
    (this)->pvaltv::SetDataObject( d);
    (this)->ObservedChanged( d, prefval_Generic);
    (TEXT(this))->AddObserver( this);
    (DATA(this))->SetCurrentItem( (DATA(this))->GetListSize()-1);    
    if((DATA(this))->GetListMax()>1) (this)->SetInterfaceView( ((this)->GetTextView())->GetApplicationLayer( ));
}