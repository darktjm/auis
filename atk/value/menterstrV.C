/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1989 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

#include <andrewos.h>
ATK_IMPL("menterstrV.H")
#include <lpair.H>
#include <mentertext.H>
#include <metextview.H>
#include <observable.H>
#include <value.H>
#include <valueview.H>

#include <menterstrV.H>




ATKdefineRegistry(menterstrV, buttonV, NULL);

class valueview *menterstrV::DoHit( enum view::MouseAction  type,long  x,long  y,long  hits )
               {
  if (this->etext &&(type == view::RightUp || type == view::LeftUp))
      (this->etext)->updatebuf();
  return (class buttonV *)this;
}
void menterstrV::ObservedChanged(class observable  *changed,long  value)
{
    class value *val ;
    const char *str,*os;
    val = (this)->Value();
    if( changed == (class observable *) this->etext){
	if(value == observable::OBJECTDESTROYED){
	    this->etext = NULL;
	    (val)->SetString(NULL);
	}
	else if(value == mentertext_BUFCHANGEDFLAG){
	    long size;
	    const char * const *buf = (this->etext)->GetStringArray();
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
	os = (this->etext)->GetString();
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
	(ev)->DesiredSize(500,500,view::NoSet,&w,&h);
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
