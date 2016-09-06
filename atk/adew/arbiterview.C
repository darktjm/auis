/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
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
ATK_IMPL("arbiterview.H")

#include <arbiterview.H>
#include <dictionary.H>
#include <atomlist.H>
#include <celview.H>
#include <value.H>
#include <cel.H>
#include <rm.H>
#include <arbiter.H>
#include <view.H>
#include <environ.H>
#include <lsetview.H>
#include <message.H>
#include <filetype.H>
#include <completion.H>
#include <im.H>
#include <text.H>
#include <cursor.H>
#include <arbcon.H>
#include <buffer.H>
#include <frame.H>
#include <dataobject.H>

static class cursor *WaitCursor;

#define INCREMENTSIZE 64
#define INITIALSIZE 128
#define SEPCHAR '_'
#define ExistingObjectDefaultView 2
#define ExistingObjectNamedView 3
#define NewObjectDefaultView 0
#define NewObjectNamedView 1
#define NamedView 0
#define ExistingObject 0
#define SaveAll 0
static class atom *a_vp;
static class arbiterview *firstlink , *lastlink;
#define NameSet(V) (((class view *)V)->name_explicitly_set)
#define DataObject(A) (A->dataobject)
#define Cel(A) ((class cel *) DataObject(A))
#define Parent(V) (((class view *)V)->parent)
#define Arbiter(A) ((class arbiter *) DataObject(A))

ATKdefineRegistry(arbiterview, celview, arbiterview::InitializeClass);
static int addlist(class arbiterview  *self,class celview  *cv);
static int deletelist(class arbiterview  *self,class celview  *cv);


class celview *arbiterview::lookupname(const char  *ViewName)
{
    class cel *cl;
    const char *st;
    class celview **v;
    int i ;
    for(v = (this->celviewlist), i = 0; i < this->celcount; v++,i++){	
	cl = Cel((*v));
	st = (cl)->GetRefName();
	if(*st == *ViewName && strcmp(st,ViewName) == 0){
	    return *v;
	}
    }
    return NULL;
}

void arbiterview::FullUpdate(enum view_UpdateType  type,long  left,long  top,long  width,long  height)
{
/*
    if(self->NeedsInit ){
	struct value *v;
	if((v = (struct value *) arbiterview_GetNamedObject(self,"ArbGoButton")) != NULL)
	    value_SetValue(v,1);
	self->NeedsInit = FALSE;
    }
*/
    (this)->celview::FullUpdate(type,left,top,width,height);
}

void arbiterview::InitCell(class celview  *cv)
{
    class cel *cl;
    cl = Cel(cv);
#ifdef DEBUG
printf("Initing %s (%s)- %d\n", (cl)->GetRefName(),((cl)->GetObject())->GetTypeName(),(cl)->GetObject());
#endif /* DEBUG */
    if(cl != NULL && cl->viewType == NULL){
#ifdef DEBUG
	printf("In INITCELL\n");
#endif /* DEBUG */
/*	arbcon_InitCel(cv,self); */
	addlist(this,cv); /* sometimes redundent */
	return;
    }
#if 0
	if((name = arbcon_GetDesiredObjectName(&type)) != NULL){
	    if(type == arbcon_OBJECTNAME){
		if((celv = (this)->lookupname(name)) != NULL){
		    dob = (Cel(celv))->GetObject();
		    (cl)->SetObject(dob);
		    name = (dob)->GetTypeName();
		}
	    }
	    else if(type == arbcon_OBJECTTYPE){
		(cl)->SetObjectByName(name);
	    }
	}
	vwname = arbcon_GetDesiredViewName(&writein);
	(cl)->SetViewName(vwname,writein);

	if (arbcon_GetApplicationChoice() && (cl)->GetApplication() != cel_VALUE){
	    (cl)->SetApplication(cel_APPLICATION);
	}

	if (arbcon_GetNameChoice()){
	    do{
		sprintf(buf,"%s-%d",name,count++);
	    } while ((this)->registername(cv,buf)== FALSE);
	    return;
	}
	
#endif /* 0 */
    if(cl && cl->refname != NULL){
	if((this)->registername(cv,cl->refname) == FALSE){
	    const char *dp;
	    char *cp,buf[512];
	    const char *name = cl->refname;
	    int count = 0;
	    if((dp = strrchr(name,SEPCHAR) ) != NULL) {
		dp++;
		if(*dp >= '0' && *dp <= '9') count = atoi(dp) + 1;
		dp--;
	    } else count = 1;
	    for(cp = buf; *name != '\0' && name != dp; name++, cp++)
		*cp = *name;
	    do{
		sprintf(cp,"%c%d",SEPCHAR,count++);
	    } while ((this)->registername(cv,buf)== FALSE);
	}
    }
    else addlist(this,cv);
#ifdef DEBUG
printf("Leaving initcell %s (%s)- %d\n", (cl)->GetRefName(),((cl)->GetObject())->GetTypeName(),(cl)->GetObject());
#endif /* DEBUG */
}


void arbiterview::ArbRead(char  *frs)
{
/* user code begins here for ArbReadButtonCallBack */
    FILE *f;
    class celview *cv = (class celview *) this;
    class view *child = cv->child;
    class arbiter *ab;
    if(*frs == '\0' || (f = fopen(frs,"r")) == NULL) {
	sprintf(frs,"ERROR:Can't Read %s",frs);
	message::DisplayString(NULL,0,frs);
	return;
    }
    if(child){
	(child)->UnlinkTree();
	(child)->Destroy();
    }
    ab = Arbiter(this);
 /*   celview_InitializeObject(cv);*/
    cv->child = NULL;
    (ab)->Read(f,0L);
/*    arbiterview_SetDataObject(self,ab); */
    (this)->WantInputFocus(this);
    fclose(f);
 /*   strcpy(self->filename,frs); */
}
boolean arbiterview::CreateCon(class text  *EditText)
{
    char fnm[64];
    char buf[1060];
    FILE *f;
    class buffer *buff;
    class dataobject *dat;
    class view *truechild;
    boolean erase = TRUE;

    if(environ::GetProfileSwitch("SecurityConscious", FALSE)) {
	message::DisplayString(this, 0, "Creating a controller is not allowed under SecurityConsciousness.");
	fprintf(stderr, "Creating a controller is not allowed under SecurityConsciousness.\n");
	return TRUE;
    }
    
    truechild = (this)->GetTrueChild();
    strcpy(fnm, "/tmp/arbtmpXXXXXX");
    im::SetProcessCursor(WaitCursor);

    if(ATK::IsTypeByName((truechild)->GetTypeName(),"frame") &&
	(buff= (((class frame *) truechild))->GetBuffer())!= NULL &&
	(dat = (buff)->GetData()) != NULL){
	int ver;
	ver =  (dat)->GetModified();
	if((buff)->GetWriteVersion() == ver){
	    sprintf(buf,"createcon %s",(buff)->GetFilename());
	    erase = FALSE;
	}
	else if((buff)->GetCkpVersion() == ver){
	    sprintf(buf,"createcon %s",(buff)->GetCkpFilename());
	    erase = FALSE;
	}
	else {
	    int fd = mkstemp(fnm);
	    if(fd < 0 || !(f = fdopen(fd, "w"))) return FALSE;
	    (dat)->Write(f,im::GetWriteID(),0);
	}
    }
    else {
	if((f = fopen(fnm,"w")) == NULL) return FALSE;
	(Arbiter(this))->Write(f,im::GetWriteID(),0);
    }
    if(erase){
	fclose(f);
	sprintf(buf,"createcon %s",fnm);
    }
    (EditText)->Clear();
    if((f = popen(buf,"r")) == NULL) return FALSE;
    (EditText)->Read(f,0);
    (EditText)->NotifyObservers(0);
    pclose(f);
    if(erase) unlink(fnm);
    im::SetProcessCursor(NULL);
    return TRUE;
}

void arbiterview::DeleteCell(class celview  *cv)
{
    if(deletelist(this,cv))
    	(cv)->RemoveObserver(this);

}
int arbiterview::registername(class celview  *cv,const char  *refname)
{
    class cel *clp;
    class cel *cl = Cel(cv);
    class celview **v;
    int i;
#ifdef DEBUG
printf("In Register name \n");
#endif /* DEBUG */
for(v = (this->celviewlist), i = 0; i < this->celcount; v++,i++){
#if 0
    if(cv == *v) /* already registered */ return TRUE;
#endif /* 0 */
    if(refname == NULL || *refname == '\0') return FALSE;
    clp = Cel((*v));
    if(*refname == *((clp)->GetRefName()) && strcmp(refname,(clp)->GetRefName()) == 0 && cv != *v )
	return FALSE;
}
#ifdef DEBUG
printf("calling addlist\n");
#endif /* DEBUG */
    i = addlist(this,cv);
#ifdef DEBUG
printf("out of addlist, cl = %d i = %d %s %s \n",(long)cl,i,refname,cl->refname);
#endif /* DEBUG */
    if(cl->refname != refname)
	(cl)->SetRefName(refname);
    arbcon::AddCel(this,cl,TRUE);
    if(this->handler){
	(*(this->handler))(cv,this->hrock);
    }
#ifdef DEBUG
printf("continueing\n");
#endif /* DEBUG */

#ifdef DEBUG
printf(" exiting Register name \n");
#endif /* DEBUG */
    return TRUE;
}
arbiterview::arbiterview()
{
	ATKinit;

    if(lastlink != NULL) lastlink->next = this;
    this->next = NULL;
    lastlink = this;
    if(firstlink == NULL) firstlink = this;
   this->celcount = 0;
     this->celsize = INITIALSIZE;
    this->celviewlist = (class celview  **) malloc(INITIALSIZE * sizeof(class celview *));
    if(this->celviewlist == NULL) THROWONFAILURE( FALSE);
    this->viewchoice = this->applicationchoice = NULL;
   this->CelNameVal = this->NameChoice = NULL;
   this->EditText = NULL;
   this->NeedsInit = TRUE;
   this->CopyMode = this->CelMode = FALSE;
   this->handler = NULL;
 /*  arbcon_AddArbiter(self); */
    THROWONFAILURE( TRUE);
}
boolean arbiterview::InitializeClass()
{
    firstlink = lastlink = NULL;
    a_vp = atom::Intern("struct dataobject *");
    WaitCursor = cursor::Create(NULL);
    if(WaitCursor) (WaitCursor)->SetStandard(Cursor_Wait);
    return TRUE;
}
arbiterview::~arbiterview()
{
	ATKinit;

    class celview **v;
    int i = this->celcount;
    for(v = this->celviewlist; i; v++,i--)
	(*v)->RemoveObserver(this);
    free( this->celviewlist);
    if(this == firstlink) {
	firstlink = this->next;
	if(this == lastlink) lastlink = NULL;
    }
    else {
	class arbiterview *ep;
	for(ep = firstlink; ep->next != NULL; ep = ep->next)
	    if(ep->next == this) break;
	ep->next = this->next;
	if(this == lastlink) lastlink = ep;
    }
    arbcon::DeleteArbiter(this);
}

static int addlist(class arbiterview  *self,class celview  *cv)
{
    class celview **v;
    int i = 0;
    for(v = (self->celviewlist); i < self->celcount; v++,i++){	
	if(*v == cv) return i;
    }
    *v++ = cv;
    if(++(self->celcount) >= self->celsize){
	self->celsize += INCREMENTSIZE;
	self->celviewlist = (class celview **)realloc(self->celviewlist,self->celsize* sizeof(class celview *));
    }
    (cv)->AddObserver(self);
    (self)->NotifyObservers(0);
    return self->celcount - 1;
}
static int deletelist(class arbiterview  *self,class celview  *cv)
{
    class celview **v;
    int i = self->celcount;
    int shift ;
    class cel *cl = Cel(cv);
    shift = 0;
    if(arbcon::currentcelview() == cv) arbcon::SetCurrentCelview(NULL);
#ifdef DEBUG
printf("in deletelist cv = %d\n",(long)cv);
#endif /* DEBUG */
    for(v = self->celviewlist ; i; v++,i--){
	if(shift) *(v - 1) = *v;
	if(*v == cv){
#ifdef DEBUG
printf("Found ref %d \n" ,cl->refname);
#endif /* DEBUG */
	    shift++;
	    if(cl->refname) {
#ifdef DEBUG
printf("found string\n");
#endif /* DEBUG */
		arbcon::DeleteCelview(self,cv);
	    }
	}
    }
    if(shift){
	(self->celcount)--;
    }
/*    if(value_GetValue(self->objectchoice) == ExistingObject)
	objectchoicechanged(self,self->objectchoice,0,ExistingObject); */
    (self)->NotifyObservers(0);
    return shift;
}

void arbiterview::AddHandler(arbiterview_hfptr handler,long  rock)
{
    class celview **v;
    int i = this->celcount;

    this->hrock = rock;
    this->handler = handler;
    if(this->handler){
	for(v = this->celviewlist ; i; v++,i--){
	    (*(this->handler))(*v,this->hrock);
	}
    }
}
ATK  * arbiterview::WantHandler(const char  *handlerName)
{
    if(strcmp(handlerName,"arbiterview") == 0) return (ATK  *)this;
    return (this)->celview::WantHandler( handlerName);
}
class dataobject * arbiterview::GetNamedObject(class view  *vw,const char  *name)
{
	ATKinit;

    long val;
    char ss[256];
    class cel *cl;
    char *st;
    class arbiterview *self;
    class atomlist *al;
    class celview **v;
    int i ;
    class atom *at;
    char *ObjectName,buf[1024];
    strcpy(buf,name);
    ObjectName = buf;
#ifdef DEBUG
printf("in arb for %s\n",ObjectName);
#endif /* DEBUG */
    if(vw) self = arbiterview::FindArb(vw);
    else {
	if((st = strrchr(buf,arbiterview_SEPCHAR)) != NULL){
	    *st = '\0';
	    ObjectName = ++st;
	    self = arbiterview::FindArbByName(buf);
	}
	else self = NULL;
    }

#ifdef DEBUG
printf("Self = %ld\n",self);
#endif /* DEBUG */
    at = atom::Intern(name);
    if(self){
	for(v = (self->celviewlist), i = 0; i < self->celcount; v++,i++){	
	    cl = Cel((*v));
	    if(at == (cl)->GetRefAtom()){
#ifdef DEBUG
printf("Found via arbiter %s = %s (%s)- Returning %d\n",(cl)->GetRefName(),ObjectName,((cl)->GetObject())->GetTypeName(),(cl)->GetObject());
#endif /* DEBUG */
		return (cl)->GetObject();
	    }
	}
	{ /* support the new child finding stuff in arbiter */
	    class cel *cel;
	    class arbiter *fcel;
	    if((fcel = (class arbiter *)  self->dataobject) != NULL){

		for(cel = (fcel)->GetFirst(); cel != (class cel *)fcel; cel = (cel)->GetNextChain()){
		    if(at == (cel)->GetRefAtom()){
			return (cel)->GetObject();
		    }
		}
	    }
	}
    }
    sprintf(ss,"DOBJ-%s",ObjectName);
    al = atomlist::StringToAtomlist(ss);
    if(rm::GetResource(al,al,a_vp,&val)) {
#ifdef DEBUG
printf("Returning %d\n",val);
#endif /* DEBUG */
	return (class dataobject *)  val;
    }
#ifdef DEBUG
printf("Returning NULL\n");
#endif /* DEBUG */
    return NULL;
}
class view * arbiterview::GetNamedView(class view  *vw,const char  *name)
{
	ATKinit;

    class celview *v;
    if((v = arbiterview::GetNamedCelview(vw,name)) == NULL) return NULL;
    return (v)->GetTrueChild();

}

class celview * arbiterview::GetNamedCelview(class view  *vw,const char  *name)
{
	ATKinit;


    class cel *cl;
    const char *st;
    char *sl;
    class arbiterview *self;
    class celview **v;
    int i ;
    char *ViewName,buf[1024];
    strcpy(buf,name);
    ViewName = buf;
#ifdef DEBUG
printf("in arb for %s\n",ViewName);
#endif /* DEBUG */
    if(vw) self = (class arbiterview *) arbiterview::FindArb(vw);
    else {
	if((sl = strrchr(buf,arbiterview_SEPCHAR)) != NULL){
	    *sl = '\0';
	    ViewName = ++sl;
	    self = arbiterview::FindArbByName(buf);
	}
	else self = NULL;
    }
#ifdef DEBUG
printf("Self = %ld\n",self);
#endif /* DEBUG */
    if(self){
	for(v = (self->celviewlist), i = 0; i < self->celcount; v++,i++){	
	    cl = Cel((*v));
	    st = (cl)->GetRefName();
	    if(*st == *ViewName && strcmp(st,ViewName) == 0){
#ifdef DEBUG
printf("Found via arbiter - Returning %d\n",val);
#endif /* DEBUG */
		return *v;
	    }
	}
    }
    return NULL;
}
void arbiterview::ObservedChanged(class observable  *changed, long  value)
{
    if (value == observable_OBJECTDESTROYED){
	if(deletelist(this,(class celview *)changed)) return;
    }
    (this)->celview::ObservedChanged( changed, value);
}
void arbiterview::SetDataObject(class dataobject  *dd)
{
    ((class arbiter *)dd)->SetApplication(cel_APPLICATION);
    (this)->celview::SetDataObject(dd);
}
void arbiterview::InitArbcon()
{
    class celview **v;
    class cel *cl;
    int i;
    for(v = (this->celviewlist), i = 0; i < this->celcount; v++,i++){	
	cl = Cel((*v));
	arbcon::AddCel(this,cl,FALSE);
    }
    (this)->SetCelMode(TRUE);
}
class arbiterview *arbiterview::GetFirstLink()
{
	ATKinit;

    return firstlink;
}
long arbiterview::GetArbName(char  *buf,long  buflen)
{
    int csize;
    class buffer *b;
    const char *myname;
    class celview *cv = (class celview *) this;
    class view *parent = this->parent;
    if(Cel(this) == NULL) return buflen;
    myname = (Cel(this))->GetRefName();
    if(parent != NULL && cv->TopLevel == FALSE &&  cv->arb != NULL) {
	csize = (cv->arb)->GetArbName(buf,buflen);
    }
    else{
	csize = 0;
	if(myname == NULL || *myname == '\0'){
	    if((b = buffer::FindBufferByData(DataObject(this))) != NULL || 
	       (b = buffer::FindBufferByData((this)->GetTrueChild()->GetDataObject())) != NULL )
		myname = (b)->GetName();
	}
    }
    if(myname == NULL || *myname == '\0'){
	myname = "UNNAMED";
    }
    if(strlen(myname) + csize + 2 < (unsigned)buflen){
	if(csize) buf[csize++] = arbiterview_SEPCHAR;
	sprintf(buf + csize,"%s",myname);
    }
   return (csize + strlen(myname));
}
class arbiterview *arbiterview::FindArbByName(char  *str)
{
	ATKinit;

    char buf[2048];
    long len,slen;
    class arbiterview *ep;
    slen = strlen(str);
    for(ep = firstlink; ep != NULL ; ep = ep->next){
	len = (ep)->GetArbName(buf,2048);
	if(len == slen && strcmp(buf,str) == 0)
	    return ep;
    }
    return NULL;
}

const char *arbiterview::GetDataName()
{
    return "arbiter";
}
void arbiterview::LinkTree(class view  *parent)
{
    (this)->celview::LinkTree(parent);
    if(parent == NULL){
	arbcon::DeleteArbiter(this);
    }
    else
	arbcon::AddArbiter(this);

}
boolean arbiterview::InTree()
{
/*  printf("In Intree %d, %s, parent = %d\n",self ,arbiter_GetRefName(Arbiter(self)), ((struct view *)self)->parent);
 printf("cp = %d\n",arbiterview_GetTrueChild(self)->parent);
 if(arbiterview_GetTrueChild(self)->parent) printf("cpp = %d\n",arbiterview_GetTrueChild(self)->parent->parent);
	    fflush(stdout);
*/
    if(((class view *)this)->parent == NULL){
	if(((this)->GetTrueChild() != NULL) &&
	   ATK::IsTypeByName(((this)->GetTrueChild())->GetTypeName(),"frame") &&
/*	   (strcmp(class_GetTypeName(arbiterview_GetTrueChild(self)),"frame") == 0) && */
	   (this)->GetTrueChild()->parent != NULL && 
	   ((this)->GetApplication() != NULL) &&
	   (((this)->GetApplication())->parent != NULL)){
	    fflush(stdout);
	    return TRUE;
	}
	return FALSE;
    }
    if(((class celview *)this)->arb == NULL) return TRUE;
    return((((class celview *)this)->arb)->InTree());
}

void arbiterview::SetIgnoreUpdates(class view  *vw,boolean  val)
{
	ATKinit;

    class arbiterview *self;
    class view *fr;
    class buffer *buf;
    if(vw){
	self = arbiterview::FindArb(vw);
	if(self && Arbiter(self)){
	    (Arbiter(self))->SetNoSave(val);
	}
	fr = (self)->GetTrueChild();
	if(ATK::IsTypeByName((fr)->GetTypeName(),"frame")){
	    if((buf = (((class frame *) fr))->GetBuffer()) != NULL)
		(buf)->SetScratch(val);
	}
    }
}
class arbiterview *arbiterview::FindArb(class view  *vw)
{
	ATKinit;

    class arbiterview *self;
    class celview *cself;
    class arbiter *arb;
    class im *im;
    class frame *frame;
    class buffer *buf;
    self = NULL;
    if(vw){
	self = (class arbiterview *) (vw)->WantHandler("arbiterview");
	if(self == NULL && (im = (vw)->GetIM()) != NULL && im->topLevel &&
	   ATK::IsTypeByName((im->topLevel)->GetTypeName(),"frame")&& 
	   (frame = (class frame *)(im->topLevel)) != NULL  &&
/*	   strcmp(class_GetTypeName(frame),"frame") == 0  && */
	   (frame)->GetChildView() != NULL){
	    for(self = firstlink; self != NULL ; self = self->next){
		if(frame == (class frame *) (self)->GetTrueChild()){
		    return(self);
		}
	    }
	    arb = new arbiter;
	    self = new arbiterview;
	    (self)->SetDataObject(arb);
	    if((buf = (frame)->GetBuffer()) == NULL)
		 (arb)->SetRefName("NoName");
	    else (arb)->SetRefName((buf)->GetName());
	    cself = (class celview *) self;
	    cself->child = cself->truechild = (class view *) frame;
	    (frame)->AddObserver(self); 
	    cself->TopLevel = TRUE;
	}
    }
    return self;
}

