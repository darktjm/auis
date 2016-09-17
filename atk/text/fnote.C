  /* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'     *
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
ATK_IMPL("fnote.H")
#include <environment.H>
#include "text.H"
#include "style.H"
#include <viewref.H>
#include <stylesheet.H>
#include <fnote.H>

class fnote **stack , **endstack;
class text *tmptext;
long notecount;
boolean donumber;
class style *Style = NULL;
class style *HStyle = NULL;
#define BACKWARD 0
#define FORWARD 1
#define NOPROCESS 2
#define MAXNOTES 500
#define OPEN TRUE
#define CLOSE FALSE
#define NEW -333
#define FLIP 42


ATKdefineRegistry(fnote, text, fnote::InitializeClass);
class style *GetStyle(class fnote  *self,class text  *txt,int  openflag);
static boolean doupdate(class fnote  *self,class text  *text,long  pos,class environment  *env);
static boolean copy(class fnote  *self,class text  *text);
static boolean open(class fnote  *self,class text  *text);
static boolean fnote_close(class fnote  *self,class text  *text);
typedef boolean (*doallcb)(class fnote *self, class text *t);

static void DoAll(class text  *text, doallcb callBack,int  order);


class style *GetStyle(class fnote  *self,class text  *txt,int  openflag)
{
    class style *Style = NULL;
    if(txt != self->parenttext){
	self->hstyle = self->vstyle = NULL;
	self->parenttext = txt;
    }
    if(openflag == OPEN){
	 if(self->vstyle) return self->vstyle;
	 if(txt && (Style = (txt->styleSheet)->Find("footnote" )) == NULL){
	    Style = new style;
	    (Style)->SetName( "footnote");
	    (txt->styleSheet)->Add( Style);
	    (Style)->SetFontSize(style_PreviousFontSize,-2);
	    (Style)->AddOverBar();
	}
	self->vstyle = Style;
    }
    else {
	if(self->hstyle) return self->hstyle;
	if(txt && (Style = (txt->styleSheet)->Find( "hfootnote")) == NULL){
	    Style = new style;
	    (Style)->SetName( "hfootnote");
	    (txt->styleSheet)->Add( Style);
	    (Style)->AddHidden();
	}
	self->hstyle = Style;
    }
    return Style;
}

void fnote::addenv(class text  *txt,long  pos)
{
    class style *Style = NULL;
    class environment *te;
    Style = GetStyle(this,txt,OPEN);
#ifdef DEBUG
printf("In addenv, Style = %d %s,txt = %d, pos = %d\n",Style,(Style)->GetName(),txt,pos);
fflush(stdout);
#endif /* DEBUG */
/*    te = environment_InsertStyle(txt->rootEnvironment,pos, Style, TRUE);*/
    te = (txt->rootEnvironment)->WrapStyle(pos,1,Style);
    (te)->SetStyle( FALSE, TRUE);
    this->env = te;
    this->loc = pos;
    this->open = OPEN;
}

static boolean doupdate(class fnote  *self,class text  *text,long  pos,class environment  *env)
{
    static class environment *lastenv = NULL;
    static long lastpos = -1;
    class style *st;
    class viewref *vr;
    const char *name, *sn;
    boolean retval = FALSE;
    if(env->type == environment_Style) {
	st = env->data.style;
	if(st != NULL && ((sn = (st)->GetName()) != NULL) &&
	   *sn == 'f' && strcmp(sn,"footnote") == 0){
	    lastenv = env;
	    lastpos = pos;
	}
	else lastenv = NULL;
#ifdef DEBUG
printf("In doupdate, Style = %s,lastenv= %d, pos = %d\n",sn,lastenv);
fflush(stdout);
#endif /* DEBUG */
	
    }
    else if(env->type == environment_View){
	vr = env->data.viewref;
	name = (vr->dataObject)->GetTypeName();
	if(self == NULL) {
	    if(!ATK::IsTypeByName(name,"fnote")){
		return FALSE;
	    }
	    self = (class fnote *) vr->dataObject;
	    if(stack != NULL && stack < endstack) *stack++ = self;
	}
	else{
	    if(self !=(class fnote *) vr->dataObject) return FALSE;
	    retval = TRUE;
	}
	/* we have a fnote */
	if(pos != lastpos)
	    (self)->SetEnv(NULL); 
	else {
	    (self)->SetEnv(lastenv); 
	    (self)->SetLoc(lastpos);
	}
	(self)->SetOwnLoc(pos);
	if(strcmp(name,"fnote") == 0) self->notecount = ::notecount++;
	lastenv = NULL;
    }
    return retval;
}
static boolean copy(class fnote  *self,class text  *text)
{   /* This routine should only be called after doupdate has properly updated the env pointers*/
    long len;
    char buf[64];
    if(self->env == NULL) (self)->addenv(text,self->ownloc);
    if(tmptext){
	len = (self->loc + (self->env)->GetLength() - 1 ) - self->ownloc;
	if(donumber){
	    sprintf(buf,"%ld\t",self->notecount);
	    /* should probably superscript this number */
	}
	(tmptext)->InsertCharacters((tmptext)->GetLength(),buf,strlen(buf));

	if(self->ownloc > self->loc){
	    (tmptext)->AlwaysCopyText((tmptext)->GetLength(),text,self->loc,self->ownloc - self->loc);
	}
	(tmptext)->AlwaysCopyText((tmptext)->GetLength(),self,0,(self)->GetLength());
	(tmptext)->AlwaysCopyText((tmptext)->GetLength(),text,self->ownloc + 1,len);
	if((tmptext)->GetChar((tmptext)->GetLength())!= '\n')
	    (tmptext)->InsertCharacters((tmptext)->GetLength(),"\n",1);
    }
    return TRUE;
}
static boolean open(class fnote  *self,class text  *text)
{   /* This routine should only be called after doupdate has properly updated the env pointers*/
    long oldmod,selfmod;
    if(self->open == OPEN) return TRUE;
    oldmod = (text)->GetModified();
    selfmod = (self)->GetModified();
    if(self->env == NULL) (self)->addenv(text,self->ownloc);
    (self->env)->SetStyle( FALSE, TRUE);
    (text)->AlwaysCopyText(self->loc + 1,self,0,(self)->GetLength());
    (self)->Clear();
    self->open = OPEN;
    (text)->RestoreModified(oldmod);
    (self)->RestoreModified(selfmod);
    return TRUE;
}
static boolean fnote_close(class fnote  *self,class text  *text)
{   /* This routine should only be called after doupdate has properly updated the env pointers*/
    long len;
    long oldmod,selfmod;
    if(self->open == CLOSE) return TRUE;
    oldmod = (text)->GetModified();
    selfmod = (self)->GetModified();
    if(self->env == NULL) (self)->addenv(text,self->ownloc);
    (self->env)->SetStyle( FALSE, FALSE);
    len = (self->loc + (self->env)->GetLength() - 1 ) - self->ownloc;
    (self)->AlwaysCopyText((self)->GetLength(),text,self->ownloc + 1,len);
    (text)->AlwaysDeleteCharacters(self->ownloc + 1,len);
    if(self->ownloc > self->loc){
	(self)->AlwaysCopyText(0,text,self->loc,self->ownloc - self->loc);
	(text)->AlwaysDeleteCharacters(self->loc,self->ownloc - self->loc);
    }
    self->open = CLOSE;
    (text)->RestoreModified(oldmod);
    (self)->RestoreModified(selfmod);
    return TRUE;
}
long fnote::GetLocLength()
{
    if(this->env)
	return (this->env)->GetLength() - 1;
    return 0L;
}

static void DoAll(class text  *text,doallcb callBack,int  order)
{
    class fnote *st[MAXNOTES];
    stack = st;
    endstack = st + MAXNOTES;
    if(text){
	(text)->EnumerateEnvironments(0,(text)->GetLength(),(text_eefptr)doupdate,0);
	switch(order){
	    case BACKWARD:
		while(--stack >= st)
		    (*(callBack))(*stack,text);
		break;
	    case FORWARD:
		for(endstack = st; endstack < stack; endstack++)
		    (*(callBack))(*endstack,text);
		break;
	    default:
		break;
	}
	(text)->NotifyObservers(0);
    }
    stack = NULL;
}

void fnote::CloseAll(class text  *text)
{
	ATKinit;

    DoAll(text,fnote_close,BACKWARD);
}

void fnote::Close(class text  *text)
{
/* printf("in close text = %d\n",text); */
    if(text){
	(text)->EnumerateEnvironments(0,(text)->GetLength(),(text_eefptr)doupdate,(long)this);
	if (fnote_close(this,text))
	    (text)->NotifyObservers(0);
    }
}
long fnote::CopyNote(class text  *text,class text  *desttext,long  count,boolean  number)
{
    tmptext = desttext;
    donumber = number;
    if(count < 0) {
	/* flag indicates the env is already set up */
	copy(this,text);
	::notecount = this->notecount;
    }
    else {    
	::notecount = count;
	if(text){
	    (text)->EnumerateEnvironments(0,(text)->GetLength(),(text_eefptr)doupdate,(long)this);
	    copy(this,text);
	}
    }
   return ::notecount;
}

int fnote::CopyAll(class text  *text,class text  *desttext,long  count,boolean  number)
{
	ATKinit;

    tmptext = desttext;
    ::notecount = count;
    donumber = number;
    DoAll(text,copy,FORWARD);
    tmptext = NULL;
    return ::notecount;
}
int fnote::UpdateAll(class text  *text,long  count)
{
	ATKinit;

    tmptext = NULL;
    ::notecount = count;
    DoAll(text,0,NOPROCESS); /* just assign numbers and update the env */
    return ::notecount;
}
void fnote::OpenAll(class text  *text)
{
	ATKinit;

    DoAll(text,::open,BACKWARD);
}

void fnote::Open(class text  *text)
{
    if(text){
	(text)->EnumerateEnvironments(0,(text)->GetLength(),(text_eefptr)doupdate,(long)this);
	if (::open(this,text))
	    (text)->NotifyObservers(0);
    }
}
boolean fnote::IsOpen()
{
    return (this->open == OPEN); 
 /*   return (fnote_GetLength(self) == 0); */
}
fnote::fnote()
{
	ATKinit;

    this->loc = this->ownloc = this->notecount = -1;
    this->parenttext = NULL;
    this->env =	NULL;
    this->vstyle = this->hstyle = NULL;
    this->open = NEW;
    THROWONFAILURE( TRUE);
}
long fnote::Read(FILE  *file,long  id)
{
    long foo;
    foo = (this)->text::Read(file,id);
    if((this)->GetLength() == 0) this->open = OPEN;
    else this->open = CLOSE;
    return foo;
}
boolean fnote::InitializeClass()
{
    stack = NULL;
    endstack = NULL;
    tmptext = NULL;
    ::notecount = 0;
    donumber = 0;
    Style = NULL;
    HStyle = NULL;
    return TRUE;
}
const char * fnote::ViewName()
{
    return "fnotev";
}
