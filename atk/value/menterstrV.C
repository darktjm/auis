/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1989 - All Rights Reserved      *
 *        For full copyright information see:'andrew/config/COPYRITE'     *
\* ********************************************************************** */

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
static UNUSED const char rcsid[]="$Header: /afs/cs.cmu.edu/project/atk-src-C++/atk/value/RCS/menterstrV.C,v 1.4 1994/11/30 20:42:06 rr2b Stab74 $";
#endif


 

ATK_IMPL("menterstrV.H")
#include <lpair.H>
#include <mentertext.H>
#include <metextview.H>
#include <observable.H>
#include <value.H>
#include <valueview.H>

#include <menterstrV.H>




ATKdefineRegistry(menterstrV, buttonV, NULL);
#ifndef NORCSID
#endif


class valueview *menterstrV::DoHit( enum view_MouseAction  type,long  x,long  y,long  hits )
               {
  if (this->etext &&(type == view_RightUp || type == view_LeftUp))
      (this->etext)->updatebuf();
  return (class buttonV *)this;
}
void menterstrV::ObservedChanged(class observable  *changed,long  value)
{
    class value *val ;
    char *str,*os;
    val = (this)->Value();
    if( changed == (class observable *) this->etext){
	if(value == observable_OBJECTDESTROYED){
	    this->etext = NULL;
	    (val)->SetString(NULL);
	}
	else if(value == mentertext_BUFCHANGEDFLAG){
	    long size;
	    char **buf = (this->etext)->GetStringArray();
	    class value *val = (this)->Value();

	    size = (this->etext)->GetArraySize();
	    if(size == 0) return;
	    (val)->SetNotify(FALSE);
	    (val)->SetArraySize(size);
	    (val)->SetNotify(TRUE);
	    (val)->SetStringArray(buf);
	}
    }
    else {
	if(val != (class value *)this->dataobject){
	    /* ERROR */
	    fflush(stdout);
	    val = (class value *)this->dataobject;
	}
	str = (val)->GetString();
	os = (this->etext)->GetSrcString();
	if(str == NULL) {
	    if(os != NULL) (this->etext)->SetChars(NULL,0);
	}
	else if(os == NULL || strcmp(os,str) != 0){
	    (this->etext)->SetChars(str,strlen(str));
	}
    }
}
class view *menterstrV::GetApplicationLayer()
{
    class lpair *lp;	
    class metextview *ev;
    long w,h;
    if((this->etext = new mentertext) == NULL) return (class view *)this;
    if((ev = new metextview) == NULL) return (class view *)this;
    (ev)->SetDataObject(this->etext);
    (this->etext)->AddObserver(this);
    this->etextview = ev;
    h = 40;
    if(((class view *)this)->parent != NULL){
	/* can't call desired size on unlinked text */
	(ev)->LinkTree(this);
	(ev)->DesiredSize(500,500,view_NoSet,&w,&h);
	(ev)->UnlinkTree();
    }
    lp = lpair::Create((ev)->GetApplicationLayer(),this,h);
    (ev)->SetValueview((class valueview *)this);
    return (class view *)lp;
}
menterstrV::menterstrV()
{
    this->etext = NULL;
    this->etextview = NULL;
    THROWONFAILURE( TRUE);
}
menterstrV::~menterstrV()
{
    if(this->etext)
	(this->etext)->RemoveObserver(this);
}
void menterstrV::WantInputFocus(class view *req)
{
    if(this->etextview) 
	(this->etextview)->WantInputFocus(this->etextview);
}
