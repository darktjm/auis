/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1989 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

#include <andrewos.h>
ATK_IMPL("clicklistV.H")

#include <lpair.H>
#include <text.H>
#include <cltextview.H>
#include <observable.H>
#include <value.H>
#include <valueview.H>

#include <clicklistV.H>


ATKdefineRegistry(clicklistV, stringV, NULL);
static void handleclicks(class clicklistV  *self,class cltextview  *cv,long  *position, long  *numberOfClicks, enum view::MouseAction  *action, long  *startLeft, long  *startRight, long  *leftPos, long  *rightPos,long  which,long  type);
#ifdef USEGETCOUNT
static void getcount(char  **str,long  size);
#endif /* USEGETCOUNT */
static void SetArray(class clicklistV  *self,const char  * const *str,long  size);


static void handleclicks(class clicklistV  *self,class cltextview  *cv,long  *position, long  *numberOfClicks, enum view::MouseAction  *action, long  *startLeft, long  *startRight, long  *leftPos, long  *rightPos,long  which,long  type)
{   /* deal with clicks */
    class value *val;
    if(type == cltextview_PREPROCESS){
	if(*numberOfClicks > 1 && 
	   ((val = (self)->Value()))->GetValue() != *numberOfClicks)
	    (val)->SetValue(*numberOfClicks);
	return;
    }
    if(*action == view::LeftUp){
	char *cp;
	int start,end,len,tlen;
	val = (self)->Value();
	if(*numberOfClicks == 1){
	    for(start = *leftPos - 1; start > 0 ; start--){
		if((self->textp)->GetChar(start) == '\n'){
		    start++;
		    break;
		}
	    }
            if(start < 0) start = 0;
	    tlen = (self->textp)->GetLength();
	    if((end = (self->textp)->Index(start,'\n',tlen - start)) == EOF)
		end = tlen;
	    len = end - start;
	    if(  len > 0){	
		if(len >= self->csize){
		    self->choice = (char *)realloc(self->choice,len + 1);
		    self->csize = len + 1;
		}
		(self->textp)->CopySubString(start, len,self->choice,FALSE);
		if((cp = strchr(self->choice,'\n')) != NULL) *cp = '\0';
		self->choicechanged = TRUE;
		if((val)->GetValue() != 1){
		    (val)->SetNotify(FALSE);
		    (val)->SetValue(1);
		    (val)->SetNotify(TRUE);
		}
		(val)->SetString(self->choice);
	    }
	}
    }
}
#ifdef USEGETCOUNT
static void getcount(char  **str,long  size)
{
    char *c;
    long cnt = 0;
    for(c = *str; size > 0; size--){
	while(*c) cnt += (long) *c++;
    }
    return cnt;
}
#endif /* USEGETCOUNT */
static void SetArray(class clicklistV  *self,const char * const *str,long  size)
{
    class text *txt;
    long i,end,sl,textchanged;
    txt = self->textp;
    textchanged = 0;
    end = (txt)->Index(0,'\n',(txt)->GetLength());
    for(i = 0; size > 0; str++,size--){
	sl = strlen(*str);
/*	printf("str = %s, i = %d (%c), end = %d\n",*str,i,text_GetChar(txt,i),end);*/
	if(end == EOF){
	    (txt)->AlwaysInsertCharacters((txt)->GetLength(),*str,sl);
	    (txt)->AlwaysInsertCharacters((txt)->GetLength(),"\n",1);
	    textchanged++;
	}
	else{
	    if(sl == (end - i) && (txt)->Strncmp(i,*str,sl) == 0){
		/* string is correct */
		i = end;
	    }
	    else {
		(txt)->AlwaysReplaceCharacters(i,end - i,*str,sl);
		i += sl;
		textchanged++;
	    }
	    i++;
	    end = (txt)->Index(i,'\n',(txt)->GetLength() - i);
	}
    }
    if(end != EOF){
	sl = (txt)->GetLength();
	if(sl > i){
	    (txt)->AlwaysDeleteCharacters(i, sl - i);
	    textchanged++;
	}
    }
    if(textchanged) {
	(txt)->SetFence((txt)->GetLength());
	(txt)->NotifyObservers(0);
	(self->cltextviewp)->CollapseDot();
    }
}
void clicklistV::ObservedChanged(class observable  *changed,long  value)
{
    class value *val ;
    const char * const *arr;
    long size;
    val = (this)->Value();
    if(value == observable::OBJECTDESTROYED){
	if(changed == (class observable *)this->cltextviewp)
	    this->cltextviewp = NULL;
	if( changed == (class observable *) this->textp)
	    this->textp = NULL;
	return;
    }
    if( changed == (class observable *) this->textp){
    }
    else {
	if(val != (class value *)this->GetDataObject()){
	    /* ERROR */
	    fflush(stdout);
	    val = (class value *)this->GetDataObject();
	}
	if(this->choicechanged){
	    this->choicechanged = FALSE;
	    (this->cltextviewp)->SetDotLength(0);
	}
	else{
	    arr = (val)->GetStringArray();
	    size = (val)->GetArraySize();
#ifdef USEGETCOUNT
	    {
		long ::count;
		::count = getcount(arr,size);
		if(this->count != ::count){
		    this->count = ::count;
		    SetArray(this,arr,size);
		}
	    }
#else /* USEGETCOUNT */
	    SetArray(this,arr,size);
#endif /* USEGETCOUNT */
	}
	(this)->stringV::ObservedChanged(changed,value);
    }
}

class view *clicklistV::GetApplicationLayer()
{
    class lpair *lp;
    class cltextview *ev;
    long w,h;
    if((this->textp = new text) == NULL) return (class view *)this;
    if((ev = new cltextview) == NULL) return (class view *)this;
    (ev)->SetDataObject(this->textp);
    h = 40;
    if(((class view *)this)->GetParent() != NULL){
	/* can't call desired size on unlinked text */
	(ev)->LinkTree(this);
	(ev)->DesiredSize(500,500,view::NoSet,&w,&h);
	(ev)->UnlinkTree();
    }
    lp = new lpair;
    (lp)->VTFixed(this,(ev)->GetApplicationLayer(),h,TRUE);
    (lp)->SetLPState(lpair_TOPFIXED,lpair_HORIZONTAL,lpair_NOCHANGE);
    this->cltextviewp = ev;
    (ev)->AddClickObserver(this,(cltextview_hitfptr)handleclicks,0);
    (ev)->AddObserver(this);
    return (class view *)lp;
}
clicklistV::clicklistV()
{
    this->textp = NULL;
    this->cltextviewp = NULL;
    this->csize = 128;
    this->choice = (char *)malloc(128);
    this->choicechanged = FALSE;
    (this)->SetUseAlt(FALSE);
    THROWONFAILURE( TRUE);
}

clicklistV::~clicklistV()
{
    if(this->textp)
	(this->textp)->RemoveObserver(this);
    free(this->choice);
}
void clicklistV::WantInputFocus(class view *req)
{
    if(this->cltextviewp) 
	(this->cltextviewp)->WantInputFocus(this->cltextviewp);
}
