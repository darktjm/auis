/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1989 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

#include <andrewos.h>
ATK_IMPL("enterstrV.H")
#include <lpair.H>
#include <entertext.H>
#include <etextview.H>
#include <observable.H>
#include <value.H>
#include <valueview.H>
#include <sbuttonv.H>
#include <buttonV.H>
#include <enterstrV.H>




ATKdefineRegistry(enterstrV, buttonV, NULL);

class valueview *enterstrV::DoHit( enum view::MouseAction  type,long  x,long  y,long  hits )
               {
  if (this->etext &&(type == view::RightUp || type == view::LeftUp))
      (this->etext)->updatebuf();
  return (class buttonV *)this;
}
void enterstrV::ObservedChanged(class observable  *changed,long  value)
{
    class value *val ;
    const char *str,*os;

    val = (this)->Value();
    if( changed == (class observable *) this->etext){
	if(value == observable::OBJECTDESTROYED){
	    this->etext = NULL;
	    (val)->SetString(NULL);
	}
	else if(value == entertext_BUFCHANGEDFLAG && this->etext){
	    char *buf = (this->etext)->GetString();
	    class value *val = (this)->Value();
	    (val)->SetNotify(FALSE);
	    (val)->SetValue(atoi(buf));
	    (val)->SetNotify(TRUE);
	    (val)->SetString(buf);
	}
    }
    else if(this->etext){
	if(val != (class value *)this->dataobject){
	    /* ERROR */
	    fflush(stdout);
	    val = (class value *)this->dataobject;
	}
	str = (val)->GetString();
	os = (this->etext)->GetString();
	if(str == NULL) str = "";
	if(os == NULL) os = "";
	if(str != os ){
	    if(strcmp(str,os) != 0){
		(this->etext)->SetChars(str,strlen(str));
		(this->etext)->updatebuf();
		return;
	    }
	}
	(this)->buttonV::ObservedChanged(changed,value);
    }
}
void enterstrV::DrawButtonText(char  *text,long  len,struct rectangle  *rect,struct rectangle  *rect2,boolean  pushd)
{
    struct rectangle r,r2;
    class buttonV *ss;
    ss = (class buttonV *) this;
    r = *rect;
    r2 = *rect2;
    if(text && len > 0){
	r.height  = r.height / 2;
	r2.height = r2.height / 2;
	(this)->buttonV::DrawButtonText(text,len,&r,&r2,pushd);
	r2.top = r2.top + r2.height;
    }
    
    sbuttonv::DrawRectBorder(this,&r2,ss->prefs,TRUE,TRUE,&r);
    (this->etextviewp)->InsertView( this, &r);
    (this)->RetractViewCursors( this->etextviewp);
    (this->etextviewp)->FullUpdate(view::FullRedraw, 0, 0, 0, 0);
}

enterstrV::enterstrV()
{
    this->etext = NULL;
    this->etextviewp = NULL;
    if((this->etext = new entertext) == NULL) THROWONFAILURE( FALSE);
    if((this->etextviewp  = new etextview) == NULL) THROWONFAILURE( FALSE);
    (this->etextviewp)->SetDataObject(this->etext);
    (this->etext)->AddObserver(this);
    (this->etextviewp)->SetValueview((class valueview *)this);
    (this->etextviewp)->LinkTree(this);
    ((class buttonV *)this)->buttontype = TRUE;
    THROWONFAILURE( TRUE);
}
enterstrV::~enterstrV()
{
    if(this->etext)
	(this->etext)->RemoveObserver(this);
}
void enterstrV::LinkTree(class view *parent)
{
    (this)->buttonV::LinkTree(parent);
    (this->etextviewp)->LinkTree(this);
}
class view * enterstrV::Hit(enum view::MouseAction  type, long  x , long  y , long  numberOfClicks)
{
    if(this->etextviewp) 
	(this->etextviewp)->WantInputFocus(this->etextviewp);

    return (this->etextviewp)->Hit( type, (this->etextviewp)->EnclosedXToLocalX( x), (this->etextviewp)->EnclosedYToLocalY( y), numberOfClicks);
}
