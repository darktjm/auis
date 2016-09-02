/* user code begins here for HeaderInfo */

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

/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/config/COPYRITE'     *
\* ********************************************************************** */
/* user code ends here for HeaderInfo */
#include <andrewos.h>
ATK_IMPL("arbcon.H")

#include <proctable.H>
#include <view.H>
#include <arbiterview.H>
#include <arbcon.H>
#include <celview.H>
#include <controlV.H>
#include <cel.H>
/* #include <buttonv.ih>  removed to get past "too many defines" error with a braindead cpp
#include <onoffv.ih>
  */
#include <menterstrV.H>
#include <value.H>
#include <clicklistV.H>
/* user code begins here for includes */
#define celview_COVERCHILD 1 /* should be in celv.ch */
#define SEPCHAR '_'
#define ARBCONMESSAGE "Arbcon\nAdew Version 5.0\n"
#include <environ.H>
#include <completion.H>
#include <message.H>
#include <im.H>
#include <arbiter.H>
#include <frame.H>
#include <buffer.H>
#include <dataobject.H>
#include <menulist.H>
#include <bind.H>
#include <style.H>
#include <stylesheet.H>
#include <environment.H>
#include <cltextview.H>
#include <text.H>



static class arbcon *Gself;
static class arbiterview *OwnArb;
#define INITIALSIZE 512
#define DataObject(A) (A->dataobject)
#define Cel(A) ((class cel *) DataObject(A))
#define Arbiter(A) ((class arbiter *) DataObject(A))
#define Parent(V) (((class view *)V)->parent)
#define NamedView 0
#define EXISTINGOBJECT 1
#define DEFAULTVIEW 1
#define WRITEINVIEW 0
#define INVISPOS 1
#define VISIBLEPOS 0
#define ARBCONNAME environ::AndrewDir("/lib/arbiters/Arb")
#define LISTNAME environ::AndrewDir("/lib/arbiters/vallist")
#define VWLISTFILE  environ::AndrewDir("/lib/arbiters/viewlist")
static const char defaultvwlist[] = "text,fad,table,eq,raster,lookz,lset,page,ness,zip,link,chomp,calc,bush,chart,value bargraphV,value fourwayV,value sliderV,value thumbV,value buttonV,"
"value onoffV,value sliderstrV,value thumbstrV,value controlV,value pianoV,value stringV,value enterstrV,value menterstrV,value clicklistV,arbiter" ;
class menulist *arbconMenus;
static class atom *atta[7];

ATKdefineRegistry(arbcon, observable, arbcon::InitializeClass);
static void DoCopy(class arbcon  *self,boolean  clear);
static int mystrcmp(const char  *s1,const char  *s2);
static int findinlist(char  **lst ,int  cnt,char  *str);
static char *parseobv(const char  *str,char  *buf);
static void SetNotice(const char  *str);
static int appendlist(char  **lst,int  cnt,char  *str,int  TEST);
static void SetName(class celview  *cv,class arbiterview  *abv,const char  *name);
static long findstring(class text  *txt,const char  *str);
static void handleclicks(class arbcon  *self,class cltextview  *ct,long  *position, long  *numberOfClicks, enum view_MouseAction  *action, long  *startLeft, long  *startRight, long  *leftPos, long  *rightPos,long  which,long  type);
static void NewWindow(const char  *filename,int  bflags,boolean  AddArb);
void arbcon_Create();
static void addtypes(class cel  *cl);
static boolean setupcel(class cel  *cl);
#ifdef NOTUSED
static void CopyText(class text  *dst,class text  *src);
#endif /* NOTUSED */
static class celview *currentcelview(struct classheader  *ClassID);
static boolean isarbcon(class celview  *cv);
static void setobview(class arbcon  *self,const char  *str,boolean  docopy);
static void addobview(class view  *v);
static void copy(class view  *v);
static void copylink(class view  *v);
static void cut(class view  *v);
static void newwin(class view  *v);
static void newlist(class view  *v);
#ifdef NOTUSED
static void arbnewwin(class view  *v);
#endif /* NOTUSED */
static void init(class arbcon  *self);
static void showcels(class view  *v);
static void arbchdir(class view  *v);
static void createcon(class view  *v);
static boolean dolistfile(class arbcon  *self,const char  *s);
static boolean createGself(class arbcon  *self);
static class arbcon *FindSelf(class view  *v);
static void ArbLinkCelCallBack(class arbcon  *self,class value  *val,long  r1,long  r2);
static void ArbCutCelCallBack(class arbcon  *self,class value  *val,long  r1,long  r2);
static void ArbApplicationChoiceCallBack(class arbcon  *self,class value  *val,long  r1,long  r2);
static void ArbobviewlistCallBack(class arbcon  *self,class value  *val,long  r1,long  r2);
static void ArbTextEditCallBack(class arbcon  *self,class value  *val,long  r1,long  r2);
static void initself(class arbcon  *self,class view  *v);
void arbcon_copycon(class view  *v,long  dat);


static void DoCopy(class arbcon  *self,boolean  clear)
{
   char buf[256];
   const char *oldref;
   const char *s;
   *buf = '\0';
    if(Gself != NULL){
	if (Gself->currentcelviewval && (Gself->ArbLinkCel->GetValue() == 0)){
	    (Gself->currentcelviewval)->Copy();
	    sprintf(buf,"Copying %s",(Cel(Gself->currentcelviewval))->GetRefName());
	    message::DisplayString(NULL,0,buf);
	    clear = FALSE;
	}
	else {
	    FILE *cutFile;
	    class cel *cl;
	    cl = new cel;
	    if(setupcel(cl)){
		class im *mim = im::GetLastUsed();
		cutFile = (mim)->ToCutBuffer();
		if (Gself->currentcelviewval){
		    oldref = (Cel(Gself->currentcelviewval))->GetRefName();
		    if(oldref == NULL) oldref = "";
		    (cl)->WriteLink(cutFile,im::GetWriteID(),0);
		    if((s = Gself->ViewName) != NULL && *s)
			sprintf(buf,"Copying New %s link to %s",s,oldref);
		    else 
			sprintf(buf,"Copying New link to %s",oldref);
		    clear = FALSE;
		    message::DisplayString(NULL,0,buf);
		}
		else {
		    (cl)->Write(cutFile,im::GetWriteID(),0);
		    if((s = Gself->ViewName) != NULL && *s)
			sprintf(buf,"Copying new %s-%s inset",Gself->ObjectName,s);
		    else
			sprintf(buf,"Copying new %s object" ,Gself->ObjectName);
		    if((Gself->ArbApplicationChoice)->GetValue())
			strcat(buf," w/ scroll bar(s)");
		}
		(mim)->CloseToCutBuffer( cutFile);
	    }
	    (cl)->Destroy();
	}
	if(clear){
	    arbcon::SetCurrentCelview(NULL);
	    SetNotice(buf);
	}
    }
}
static int mystrcmp(const char  *s1,const char  *s2)
{
    if(s1 == NULL && s2 == NULL) return 0;
    if(s1 == NULL || s2 == NULL) return 1;
    return strcmp(s1,s2);
}
static int findinlist(char  **lst ,int  cnt,char  *str)
{
    int i;
    for(i = 0; i < cnt; i++,lst++){
	if(*lst == NULL || str == NULL) return -1;
	if(**lst == *str && (*lst == str || strcmp(*lst,str) == 0)){
	    return i;
	}
    }
    return -1;
}
static char *parseobv(const char  *str,char  *buf)
{
    char *stop = NULL;
    while(*str){
	*buf = *str++;
	if(*buf == ' ') {
	    *buf = '\0';
	    if(stop == NULL || *stop == ' ')
		stop = buf + 1;
	}
	buf++;
    }
    *buf = '\0';
    if(stop && *stop) return stop;
    return NULL;
}
    
static void SetNotice(const char  *str)
{
    if(Gself == NULL) return;
    if(str) {
	int len;
	len = strlen(str);
	strncpy(Gself->arr,str,Gself->arrlen - 1);
	if(len  >= Gself->arrlen - 1){
	    Gself->arr[Gself->arrlen - 2] = '\0';
	    len = Gself->arrlen - 2;
	}
	if(len==0 || Gself->arr[len - 1] != '\n'){
	    Gself->arr[len++] = '\n';
	    Gself->arr[len] = '\0';
	}
    }
    else {
	const char *srs, *vv;
	if((srs = Gself->ObjectName) == NULL || *srs == '\0')
	    sprintf(Gself->arr,"< New Object >");
	else{
	    if((vv = Gself->ViewName) == NULL) vv = "";
	    sprintf(Gself->arr,"< New %s-%s inset>\n",srs,vv);
	}
    }
    (Gself->ArbTextEdit)->SetArraySize(0);
    (Gself->ArbTextEdit)->SetString(Gself->arr);
    message::DisplayString(NULL,0,"");
}
static int appendlist(char  **lst,int  cnt,char  *str,int  TEST)
{   /* BUG -- OVERFLOWS NOT DETECTED */
    int next = 1;
    if(TEST){
	if(findinlist(lst,cnt,str) != -1) return cnt;
    }
    while(*str){
	if(*str == ',' || *str == '\n') {
	    *str = '\0';
	    next++;
	}
	else if(*str == '\0') break;
/*	else if(*str == ' ') ; */
	else if(next){
	    lst[cnt++] = str;
	    next = 0;
	}
	str++;
    }
    return cnt;
}
static void SetName(class celview  *cv,class arbiterview  *abv,const char  *name)
{

    int count = 0;
    char buf[256],nbuf[256];
    if(abv && cv){
	if(name == NULL || Gself == NULL) return;
	strcpy(nbuf,name);
	name = nbuf;
	if(cv == Gself->currentcelviewval) 
	    arbcon::DeleteCelview(abv,cv);
	if((abv)->registername(cv,name)== FALSE){
	    const char *dp;
	    char *cp;
	    if((dp = strrchr(name,SEPCHAR) ) != NULL) {
		dp++;
		if(*dp >= '0' && *dp <= '9') count = atoi(dp) + 1;
		dp--;
	    } else count = 1;
	    for(cp = buf; *name != '\0' && name != dp; name++, cp++)
		*cp = *name;
	    do{
		sprintf(cp,"%c%d",SEPCHAR,count++);
	    } while ((abv)->registername(cv,buf)== FALSE);
	}
    }
}
static long findstring(class text  *txt,const char  *str)
{   /* searches text for str by itself on a line */
    long i,ch,pos,len;
    const char *p;
    if(txt == NULL) return -2 ;
    len = (txt)->GetLength();
    p = str;pos = 0;
    for(i = 0;i < len; i++){
	if((ch = (txt)->GetChar(i)) == '\n'){
	    if (p != NULL && (*p == '\0' || *p == '\n')) return pos;
	    p = str;
	    pos = i + 1;
	}
	else if (p == NULL) continue;
	else if (*p == ch) p++;
	else p = NULL;
    }
    return -1;
}
static void handleclicks(class arbcon  *self,class cltextview  *ct,long  *position, long  *numberOfClicks, enum view_MouseAction  *action, long  *startLeft, long  *startRight, long  *leftPos, long  *rightPos,long  which,long  type)
{   /* deal with clicks */
    char buf[512];
    int len;
    class celview *cv;
    class arbiterview *ab;
    if(type == cltextview_PREPROCESS || Gself == NULL){
	*numberOfClicks = 3;
	return;
    }
    *numberOfClicks = 1;
    len = *rightPos - *leftPos;
    if(*action == view_LeftUp){	
	if(which == 0){
	    if(len > 0){
		(self->ArbCelList)->CopySubString(*leftPos, len,buf,FALSE);
		if(self->CurrentArbiterview != NULL &&
		   (cv = (self->CurrentArbiterview)->lookupname( buf)) != NULL &&
		   cv != Gself->currentcelviewval){
		    arbcon::SetCurrentCelview(cv);
		}
	    }
	    if(self->overlay != NULL){
		(self->ArbCelListView)->DeleteApplicationLayer(self->overlay);
		(self->ArbCelListView)->Destroy();
		self->ArbCelListView = NULL;
		self->overlay = NULL;
	    }
	}
	else if(len > 0){
	    (self->ArbArbList)->CopySubString(*leftPos, len,buf,FALSE);
	    if((ab = arbiterview::FindArbByName(buf)) != NULL && ab != Gself->CurrentArbiterview){
		arbcon::SetCurrentArbiterview(ab);
	    }
	}
    }
}
static void NewWindow(const char  *filename,int  bflags,boolean  AddArb)
{

    class frame *newFrame;
    class im *window;
    class buffer *buffer;
    const char *type;
    const char *otype;
    if((buffer = buffer::GetBufferOnFile(filename, bflags)) == NULL){
	char buf[1300];
	sprintf(buf,"ERROR: can't create %s",filename);
	message::DisplayString(NULL,0,buf);
	return;
    }
    otype = ((buffer)->GetData())->GetTypeName();
    if(strcmp(otype,"arbiter") != 0){
	class arbiter *ab = NULL;
	class dataobject *ob,*nb;
	boolean newobject = FALSE;

	nb = ob = (buffer)->GetData();
	if(Gself && access(filename,F_OK) == -1 && (type = Gself->ObjectName)!= NULL && *type && strcmp(type,otype) != 0){
	    if(AddArb){
		ab = new arbiter;
		(ab)->SetObjectByName(type);
		nb = (class dataobject *) ab;
		newobject = TRUE;
	    }
	    else if(ATK::IsTypeByName(type,"dataobject")){
		nb = (class dataobject *)ATK::NewObject(type);
		newobject = TRUE;
	    }
	}
	else {
	    if(AddArb){
		ab = new arbiter;
		(ab)->SetObject(ob);
		nb = (class dataobject *) ab;
	    }
	}
	if(ab) (ab)->SetRefName("");
	if(nb != ob){
	    (buffer)->SetData(nb);
	    if(newobject) (ob)->Destroy();
	}
    }
    if ((newFrame = new frame) != NULL) {
	if ((window = im::Create(NULL)) != NULL) {
	    (newFrame)->SetCommandEnable( TRUE);
	    (window)->SetView( newFrame);
	    (newFrame)->PostDefaultHandler( "message", (newFrame)->WantHandler( "message"));
/*
	    if ((tempMessageLine = (struct msghandler *) frame_WantHandler(newFrame, "message")) != NULL) {
		msghandler_DisplayString(messageLine, 0, "");
		messageLine = tempMessageLine;
	    }
*/
	    (newFrame)->SetBuffer( buffer, TRUE);
	}
	else {
	    fprintf(stderr,"Could not create new window.\n");
	    (newFrame)->Destroy();
	}
    }
    else {
	fprintf(stderr,"Could not allocate enough memory.\n");
    }
}
void arbcon_Create(){
    char foo[1024];
    if(Gself) return;
    strcpy(foo,ARBCONNAME);
    NewWindow(foo,0,FALSE);
}
class celview *arbcon::currentcelview()
{
	ATKinit;

    if(Gself == NULL) return NULL;
    return Gself->currentcelviewval;
}
void arbcon::SetCurrentArbiterview(class arbiterview  *ab)
{
	ATKinit;
  
    static char buf[256];
    if(Gself == NULL) return ;
    if(ab == Gself->CurrentArbiterview || ab == OwnArb) return;
    arbcon::SetCurrentCelview(NULL);
    (Gself->ArbCelList)->Clear();
    if(ab == NULL || Arbiter(ab) == NULL){
	Gself->CurrentArbiterview = NULL;
	*buf= '-';
	buf[1] = '\0';
    }
    else {
	(ab)->GetArbName(buf,256);
	Gself->CurrentArbiterview = ab;
	if(Parent(ab)!= NULL)
	    arbcon::AddArbiter(ab);
	(ab)->InitArbcon();
#if 0
	if( (Gself->ArbCopyMode)->GetValue() != (ab)->GetCopyMode())
	    (Gself->ArbCopyMode)->SetValue((ab)->GetCopyMode());
#endif /* 0 */
    }
    Gself->ArbiterName = buf;
    (Gself->ArbCelList)->NotifyObservers(0);
}
void arbcon::DeleteArbiter(class arbiterview  *arb)
{
	ATKinit;

    if(Gself == NULL) return ;
    if(Gself->CurrentArbiterview == arb) arbcon::SetCurrentArbiterview(NULL);
    arbcon::InitArbiters();
}

void arbcon::AddArbiter(class arbiterview  *arb)
{
	ATKinit;

    char buf[512];
    if(Gself == NULL || arb == OwnArb) return;
    (arb)->GetArbName(buf,512);
    strcat(buf,"\n");
    if(findstring(Gself->ArbArbList,buf) == -1){
	((Gself->ArbArbList))->AlwaysInsertCharacters(0,buf,strlen(buf));
	(Gself->ArbArbList)->NotifyObservers(0);	
    }
#ifdef DEBUG
printf("Adding %s (%d) to text %d (%s) \n", buf,strlen(buf),Gself->ArbArbList,(Gself->ArbArbList)->GetTypeName()); fflush(stdout);
#endif /* DEBUG */

}
void arbcon::InitArbiters()
{
	ATKinit;

    class arbiterview *ab,*cab;
    cab = NULL;
    if(Gself == NULL ) return;
    if(Gself->ArbArbList) (Gself->ArbArbList)->Clear();
    for(ab = arbiterview::GetFirstLink(); ab != NULL; ab = ab->next){
	if((ab)->InTree()){
	    arbcon::AddArbiter(ab);
	    if(ab != OwnArb && (cab == NULL || ab->celcount > 0)) cab = ab;
	}
	else if(ab == Gself->CurrentArbiterview)
	    arbcon::SetCurrentArbiterview(NULL);
    }
/*    printf("cab = %d, Gself->CurrentArbiterview = %d, ownarb = %d\n\n ",
	    cab,Gself->CurrentArbiterview,OwnArb); fflush(stdout); */
    if(Gself->CurrentArbiterview == NULL && cab != NULL)
	arbcon::SetCurrentArbiterview(cab);
}
static void addtypes(class cel  *cl)
{
/*  Not Currently Supported
    char *obstr,*vwstr;
    int i;
    if((obstr = cel_GetObjectName(cl)) != NULL && *obstr && (i =appendlist(Gself->obnamelist,Gself->obcount,obstr,TRUE)) != Gself->obcount){
	Gself->obcount = i;
	value_SetArraySize(Gself->ArbObjectWheel,Gself->obcount);
    } 
    if((vwstr = cel_GetViewName(cl)) != NULL && *vwstr && ( i =appendlist(Gself->vwlist,Gself->vwcount,vwstr,TRUE)) != Gself->vwcount){
	Gself->vwcount = i;
	value_SetArraySize(Gself->ArbViewWheel,Gself->vwcount);
    }
*/
}
static boolean setupcel(class cel  *cl)
{
    const char *obstr,*vwstr;
    const char *name,*str;
    if(Gself == NULL) return FALSE;
    obstr = Gself->ObjectName;
    vwstr = Gself->ViewName;
    str = NULL;
    if((Gself->ArbLinkCel)->GetValue() && Gself->currentcelviewval){
	(cl)->SetObject(Cel(Gself->currentcelviewval)->dataObject);
	(cl)->SetLinkName((Cel(Gself->currentcelviewval))->GetRefName());
	name = (Cel(Gself->currentcelviewval))->GetObjectName();
	str =(Cel(Gself->currentcelviewval))->GetRefName();
    }
    else if(obstr && *obstr){
/*
	cel_SetObjectByName(cl,obstr);
*/
	(cl)->SetObjectName(obstr);
	name = obstr;
    }
    else return FALSE;
    if(vwstr && *vwstr){
/*
	if( !class_IsTypeByName(name,"value")){
	    char *cp;
	    for(cp = vwstr; *cp != '\0'; cp++) ;
	    cp--;
	    if(*cp == 'V') return FALSE;
	}
	else cel_SetApplication(cl,cel_VALUE);
*/
	(cl)->SetViewName(vwstr,FALSE);
    }
    else (cl)->SetViewName(NULL,TRUE);

    if ((Gself->ArbApplicationChoice)->GetValue() && (cl)->GetApplication() != cel_VALUE){
#ifdef DEBUG
	printf("setting application\n");
#endif /* DEBUG */
	(cl)->SetApplication(cel_APPLICATION);
    }
    /*	else  cel_SetApplication(cl,cel_NORMAL); */
    if (str && *str) name = str;
    (cl)->SetRefName(name);
    addtypes(cl);
    (cl)->NotifyObservers(0);
    return TRUE;
}

boolean arbcon::InitCel(class celview  *cv,class arbiterview  *abv)
{
	ATKinit;

#if 0
    class cel *cl;
    const char  *obstr,*vwstr;
    class dataobject *dob;
    const char *name;
    if((cl = Cel(cv)) == NULL || Gself == NULL) return FALSE;
    if( cl->viewType != NULL && !(cv->NeedsRemade)) return FALSE;
    obstr = Gself->ObjectName;
    vwstr = Gself->ViewName;
    if((Gself->ArbLinkCel)->GetValue()){
	if((Gself->CurrentArbiterview != abv)  || Gself->currentcelviewval == NULL || abv == NULL ){
	    /* can't link to objects in other arbiters */
	    return FALSE;
	}
	dob = Cel(Gself->currentcelviewval)->dataObject;
	(cl)->SetObject(dob);
	name = (dob)->GetTypeName();
    }
    else if(obstr && *obstr){
	(cl)->SetObjectByName(obstr);
	name = obstr;
    }
    else return FALSE;
    if(vwstr && *vwstr){
	if(strcmp(name,"value") != 0){
	    char *cp;
	    for(cp = vwstr; *cp != '\0'; cp++) ;
	    cp--;
	    if(*cp == 'V') return FALSE;
	}
	(cl)->SetViewName(vwstr,FALSE);
    }
    else (cl)->SetViewName(NULL,TRUE);

    if ((Gself->ArbApplicationChoice)->GetValue() && (cl)->GetApplication() != cel_VALUE){
#ifdef DEBUG
	printf("setting application\n");
#endif /* DEBUG */
	(cl)->SetApplication(cel_APPLICATION);
    }
    /*	else  cel_SetApplication(cl,cel_NORMAL); */
    SetName(cv,abv,name);
    addtypes(cl);
    (cl)->NotifyObservers(0);
    return TRUE;
#else /* 0 */
    return FALSE;
#endif /* 0 */
}

/* REPLACE WITH TEXT_COPYTEXT 
arbcon__EditCurrentCelview SHOULD CAUSE THE CEL TO REINIT ITSELF IF LENGTH OF TEXT = 0
*/
#ifdef NOTUSED
static void CopyText(class text  *dst,class text  *src)
{
    char foo[4000];
    int len = (src)->GetLength();
    if(len > 4000) len = 4000;
    (dst)->Clear();
    (src)->CopySubString(0,len,foo,TRUE);
    (dst)->AlwaysInsertCharacters(0,foo,len);
    (dst)->NotifyObservers(NULL);
}
#endif /* NOTUSED */
void arbcon::EditCurrentCelview()
{
	ATKinit;

    class text *src;
    long i,len,bufsize;
    char foo,*c;
    const char *refname;
    char buf[256];
    const char *vtype;
    class cel *cl;
    if(Gself == NULL || Gself->ArbTextEdit == NULL )
	return;
    SetNotice("");
    if(Gself->currentcelviewval == NULL || (cl = Cel(Gself->currentcelviewval)) == NULL ){
	return;
    }
    len = 0;
    src = (cl)->GetScript();
    refname = (cl)->GetRefName();
    if(refname == NULL) refname = "";
    vtype =(cl)->GetViewName();
    if(vtype == NULL || *vtype == '\0')
	sprintf(buf,"%s Object. Name@%s\n",(cl)->GetObjectName(),refname);
    else 
	sprintf(buf,"%s-%s Inset. Name@%s\n",(cl)->GetObjectName(),vtype,refname);

    bufsize = strlen(buf);
    if(src) len = (src)->GetLength();
    i = len + bufsize + 2;
    if(Gself->arrlen <= i){
	i++;
	(Gself->ArbTextEdit)->SetString(NULL);
	Gself->arr = (char *)realloc(Gself->arr,i);
	Gself->arrlen = i;
    }
    strcpy(Gself->arr,buf);
    if(src){
	c = Gself->arr + bufsize;
	for(i = 0; i < len; i++){
	    foo = (src)->GetChar(i);
	    if(foo == '[' || foo == ']' || foo == '<' ||
	       foo == '>' || foo == ')') continue;
	    if(foo == '(') *(c - 1) = '@';
	    else *c++ = foo;
	}
	*c = '\0';
    }
    (Gself->ArbTextEdit)->SetString(Gself->arr);
    (Gself->ArbTextEdit)->SetArraySize(0);
    ((class view *)Gself->ArbTextEditView)->WantInputFocus( Gself->ArbTextEditView);
}
static class celview *currentcelview(struct classheader  *ClassID)
{
    if(Gself == NULL) return NULL;
    return Gself->currentcelviewval;
}
void arbcon::AddCel(class arbiterview  *arb,class cel  *cl,boolean  notify)
{
	ATKinit;

    char buf[512];
    class celview *cv;
    const char *str = (cl)->GetRefName();
    if(str == NULL || *str == '\0' || *str == '-') return;
    strcpy(buf,str);
    strcat(buf,"\n");
    if(Gself == NULL || Gself->ArbCelList == NULL) return;
    if(notify){
	if(arb != Gself->CurrentArbiterview){
	    arbcon::SetCurrentArbiterview(arb);
/*	    return; */
	}
	if(findstring(Gself->ArbCelList,buf) == -1){
	    (Gself->ArbCelList)->AlwaysInsertCharacters(0,buf,strlen(buf));
	    (Gself->ArbCelList)->NotifyObservers(0);
	}
	(Gself->ArbCelList)->NotifyObservers(0);
	if(Gself->CurrentArbiterview && ((cv = (Gself->CurrentArbiterview)->lookupname( str)) != NULL) && cv != Gself->currentcelviewval){
	    arbcon::SetCurrentCelview(cv);
	}
    }
    else {
	if(findstring(Gself->ArbCelList,buf) == -1){
	    (Gself->ArbCelList)->AlwaysInsertCharacters(0,buf,strlen(buf));
	    (Gself->ArbCelList)->NotifyObservers(0);
	}
    }
    addtypes(cl);
}
void arbcon::DeleteCelview(class arbiterview  *arb,class celview  *cv)
{
	ATKinit;

    int loc;
    const char *name;
    if(Gself == NULL || cv == NULL) return;

    if(cv == Gself->currentcelviewval) 
	arbcon::SetCurrentCelview(NULL);
    if(Gself->ArbCelList && 
	Cel(cv) != NULL &&
	cv->arb == Gself->CurrentArbiterview &&
	(name = (Cel(cv))->GetRefName()) != NULL &&
	*name &&
	(loc = findstring(Gself->ArbCelList,name)) >= 0){
	(Gself->ArbCelList)->AlwaysDeleteCharacters(loc,strlen(name) + 1);
	(Gself->ArbCelList)->NotifyObservers(0);
    }
}
void arbcon::SaveCurrentCelview()
{
	ATKinit;

#if 0
    class text *src;
    if(Gself == NULL || Gself->ArbText == NULL || Gself->currentcelviewval == NULL ||
	Cel(Gself->currentcelviewval) == NULL ||
	(src = (Cel(Gself->currentcelviewval))->GetScript()) == NULL) return;
    CopyText( src,Gself->ArbText);
    (Gself->currentcelviewval)->PostParameters();
#else /* 0 */
    class text *src;
    char buf[256];
    const char *str;
    const char * const *arr;
    const char *oldstr;
    long len,size;
    if(Gself == NULL || Gself->ArbTextEdit == NULL || Gself->currentcelviewval == NULL ||
	Cel(Gself->currentcelviewval) == NULL ||
	 (Gself->ArbTextEdit)->GetString() == NULL || (Gself->ArbTextEdit)->GetArraySize() == 0 ||
	(Gself->ArbTextEdit)->GetStringArray() == NULL || *((Gself->ArbTextEdit)->GetString()) == '\0') return;
/*    CopyText( src,Gself->ArbText); */
    src = (Cel(Gself->currentcelviewval))->GetScript();
    size = (Gself->ArbTextEdit)->GetArraySize();
    arr  = (Gself->ArbTextEdit)->GetStringArray();
    size--;
    str = *arr++;
    if(src && arr != NULL && size > 0){
	/* merge arr and value_GetString and write into src */
	long start,end;
	start = end = 0;
	len = (src)->GetLength();
	while((start = (src)->Index(end,'(',len - end))!= EOF){
	    start++;
	    if((end = (src)->Index(start,')',len - start))== EOF) break;
	    if(end== start){
		if(*arr == '\0') continue;
		(src)->InsertCharacters(start,*arr,strlen(*arr));
	    }
	    else if(*arr == '\0') (src)->DeleteCharacters(start,end - start);
	    else (src)->ReplaceCharacters(start,end - start,*arr,strlen(*arr));
	    if(++arr == NULL || --size == 0) break;
	    len = (src)->GetLength();
	}
/*	celview_PostParameters(Gself->currentcelviewval); */
	(Cel(Gself->currentcelviewval))->NotifyObservers(cel_NeedsRepost);
    }
    oldstr = (Cel(Gself->currentcelviewval))->GetRefName();

    if(str && *str && oldstr && *oldstr && strcmp(str,oldstr) != 0){
	
	sprintf(buf,"saving attributes and renaming %s to %s",oldstr,str);
	message::DisplayString(NULL,0,buf);
	SetName(Gself->currentcelviewval,Gself->CurrentArbiterview,str); 
    }
    else {
	sprintf(buf,"saving attributes for %s",oldstr);
	message::DisplayString(NULL,0,buf);
    }

#endif /* 0 */
}

static boolean isarbcon(class celview  *cv)
{
    class atom *att,**atp;
    att =  (Cel(cv))->GetRefAtom();
    for(atp = atta; *atp != NULL; atp++)
	if(*atp == att){
	    return TRUE;
	}
    return FALSE;
    
}

void arbcon::SetCurrentCelview(class celview  *cv)
{
	ATKinit;

    const char *srs;
    char buf[256];
    /* printf("In SetCurrentCelview cv = %d\n",(long)cv); */
   /*  fprintf(stderr,"In scc\n");
    fflush(stderr);
*/
    if(Gself == NULL || Gself->currentcelviewval == cv ) return;
    if(cv == NULL || Cel(cv) == NULL ||  (Cel(cv))->GetRefName() == NULL){
	Gself->currentcelviewval = NULL;
	if((Gself->ArbLinkCel)->GetValue())
	    (Gself->ArbLinkCel)->SetValue(FALSE);
	SetNotice("");
    }
    else {
	if(cv->arb == OwnArb) return;
	if(isarbcon(cv)) return;
	Gself->currentcelviewval = NULL;
	if((Gself->ArbLinkCel)->GetValue())
	    (Gself->ArbLinkCel)->SetValue(FALSE);
	if(cv->arb != Gself->CurrentArbiterview) 
	    arbcon::SetCurrentArbiterview(cv->arb);
	Gself->currentcelviewval = cv;
    }
    if(cv != NULL && Cel(cv) != NULL){
	Gself->currentcelviewval = NULL; /* so enter object won't try to unset */
	if(((srs = (Cel(cv))->GetObjectName()) != NULL)){
	    Gself->ObjectName = srs;
	}
	else Gself->ObjectName = "";
	if((srs = (Cel(cv))->GetViewName()) != NULL)
	    Gself->ViewName = srs;
	else Gself->ViewName = "";
	Gself->currentcelviewval = cv;
	(Gself->ArbApplicationChoice)->SetValue(((Cel(cv))->GetApplication()== cel_APPLICATION) ? 1:0);
	/*
	 value_SetValue(Gself->ArbSetVisible,cel_Visible(Cel(cv)) ? VISIBLEPOS: INVISPOS);
	 */
	sprintf(buf,"Setting current celview to %s",(Cel(cv))->GetRefName());
	message::DisplayString(NULL,0,buf);
    }
    if(cv) arbcon::EditCurrentCelview();
}
void arbcon::DestroyCurrentCelview()
{
	ATKinit;

    if(Gself != NULL && Gself->currentcelviewval!= NULL) {
	class celview *cv;
	cv = Gself->currentcelviewval;
	/* cel_Destroy(Cel(cv)); */
	(cv)->UnlinkTree();
	(cv)->Destroy();
    }
}
static void setobview(class arbcon  *self,const char  *str,boolean  docopy)
{
    const char *vw,*obs,*vws;
    static char buf[128];
    boolean NeedUpdate = FALSE;
    obs = self->ObjectName;
    vws = self->ViewName;
    if(obs == buf) NeedUpdate = TRUE;
    if(str == NULL || *str == '\0' || Gself == NULL) return;
    vw = parseobv(str,buf);
    if(NeedUpdate || obs == NULL || strcmp(obs,buf) != 0){
	if(self->currentcelviewval != NULL) arbcon::SetCurrentCelview(NULL);
	if((self->ArbLinkCel)->GetValue() == 1)
	    (self->ArbLinkCel)->SetValue(0);
	self->ObjectName = buf;
    }
    
    if(NeedUpdate || (vws == NULL && vw != NULL) || mystrcmp(vws,vw) != 0){
	if(self->currentcelviewval != NULL && (self->ArbLinkCel)->GetValue() == 0)
	    arbcon::SetCurrentCelview(NULL);
	Gself->ViewName = vw;
    }
    if(self->currentcelviewval != NULL && (self->ArbLinkCel)->GetValue() == 0)
	arbcon::SetCurrentCelview(NULL);
     if(docopy)  DoCopy(self,TRUE);
}
static void addobview(class view  *v)
{
     char buf[256],*m,objbuf[256],*vw;
     int which;
     if(Gself == NULL) return;
     if(message::AskForString(NULL,0,"new object to add:",NULL,buf,256) == -1 ) return;
     if(*buf == '\0') return;
     /* should check for valid input here */
     if((which = findinlist(Gself->vwlist,Gself->vwcount,buf)) != -1){
	 /* already on list */
	 setobview(Gself,Gself->vwlist[which],TRUE);
	 return;
     }
     vw = parseobv(buf,objbuf);
     if(!ATK::IsTypeByName(objbuf,"dataobject")){
	 sprintf(buf,"No known dataobject named %s",objbuf);
	 message::DisplayString(NULL,0,buf);
	 return;
     }
     if(vw && *vw && !ATK::IsTypeByName(vw,"view")){
	 sprintf(buf,"No known view named %s",vw);
	 message::DisplayString(NULL,0,buf);
	 return;
     }
     m  = (char *)malloc(strlen(buf) + 1);
     if (m == NULL) return;
     strcpy(m,buf);
     Gself->vwcount = appendlist(Gself->vwlist,Gself->vwcount,m,FALSE);
     (Gself->Arbobviewlist)->SetNotify(FALSE);
     (Gself->Arbobviewlist)->SetArraySize(Gself->vwcount);
     (Gself->Arbobviewlist)->SetNotify(TRUE);
     (Gself->Arbobviewlist)->SetStringArray(Gself->vwlist);
     setobview(Gself,m,TRUE);
}

static void copy(class view  *v)
{
    if(Gself == NULL) return;
    if((Gself->ArbLinkCel)->GetValue() == 1)
	(Gself->ArbLinkCel)->SetValue(0);
    else DoCopy(Gself,TRUE);
}
static void copylink(class view  *v)
{
    if(Gself == NULL) return;
    if(Gself->currentcelviewval == NULL) {
	message::DisplayString(NULL,0,"No current cel to link to");
	return;
    }
    if((Gself->ArbLinkCel)->GetValue() == 0)
	(Gself->ArbLinkCel)->SetValue(1);
    else  DoCopy(Gself,TRUE);
}
static void cut(class view  *v)
{
    if(Gself == NULL) return;
    ArbCutCelCallBack(Gself,Gself->ArbCutCel,0,0);
}
static void newwin(class view  *v)
{
    char frs[1024],prompt[256];
    const char *type;
    if(Gself == NULL) return;
    type = NULL;
    if(Gself && (type = Gself->ObjectName) != NULL && *type){
	sprintf(prompt,"File to read (default object is %s): ",type);
    }
    else sprintf(prompt,"File to read: ");
    if(completion::GetFilename(Gself->v,prompt,"",frs,1024,FALSE,FALSE) == -1)
	return;
    if(type != NULL && *type)
	buffer::SetDefaultObject(type);
    NewWindow(frs,0,FALSE);
    if(type != NULL && *type)
	buffer::SetDefaultObject("text");
}
static void newlist(class view  *v)
{
    const char *p;
    arbcon::SetCurrentArbiterview(NULL);
    if((p = environ::GetProfile("ValueFile"))!= NULL && (access(p,4) == 0))
	NewWindow(p,0,FALSE);
    else
	NewWindow(LISTNAME,0,FALSE);
    if(Gself && Gself->CurrentArbiterview)
	(Gself->CurrentArbiterview)->SetCopyMode(TRUE);
}
#ifdef NOTUSED
static void arbnewwin(class view  *v)
{
    char frs[1024],prompt[256],*type;
    if(Gself == NULL) return;
    if(Gself && (type = Gself->ObjectName) != NULL && *type)
	sprintf(prompt,"File to read into arbiter(default object is %s): ",type);
    else sprintf(prompt,"File to read: ");
    if(completion::GetFilename(Gself->v,prompt,"",frs,1024,FALSE,FALSE) == -1)
	return;
    NewWindow(frs,0,TRUE);
}
#endif /* NOTUSED */
static void init(class arbcon  *self)
{
    if(Gself == NULL) return;
    arbcon::InitArbiters();
    if(self->CurrentArbiterview != NULL && self->ArbCelList) {
	(Gself->ArbCelList)->Clear();
	(self->CurrentArbiterview)->InitArbcon();
    }
}
static void showcels(class view  *v)
{
    struct rectangle rec;
    class arbiterview *abv;
    char buf[512];
    if(Gself == NULL) return;
    abv = Gself->arbv;
    init(Gself);
    if(Gself->ArbCelListView != NULL || Gself->CurrentArbiterview == NULL || (Gself->ArbCelList)->GetLength() == 0) {
	message::DisplayString(abv,0,"No Current Cels");
	return;
    }
    Gself->ArbCelListView = new cltextview;
    (Gself->ArbCelListView)->SetDataObject(Gself->ArbCelList);
    (Gself->ArbCelListView)->AddClickObserver(Gself,(cltextview_hitfptr)handleclicks,0);
    rec.width = (abv)->GetLogicalWidth();
    rec.height = (abv)->GetLogicalHeight();
    rec.top = 0;rec.left = 0;
    Gself->overlay = (Gself->ArbCelListView)->GetApplicationLayer();
/*    arbiterview_PushOverlay(abv,Gself->overlay,NULL,celview_COVERCHILD); */
    (abv)->PushOverlay(Gself->overlay,&rec,celview_COVERCHILD);	
    sprintf(buf,"Click on desired %s cel",Gself->ArbiterName);
    message::DisplayString(abv,0,buf);
}
static void arbchdir(class view  *v)
{
    class arbiterview *abv;
    char buf[1024],bb[1200];
    int pf;
    if(Gself == NULL) return;
    abv = Gself->arbv;
    *bb = '\0';
    im::GetDirectory(bb);
    pf = completion::GetFilename(abv,"New Directory: ",bb,buf, sizeof(buf),FALSE,FALSE) ;
    if(pf>= 0){
	if(im::ChangeDirectory(buf)== -1){
	    sprintf(bb,"ERROR: Can't cd to %s",buf);
	}
	else sprintf(bb,"New directory is %s",buf);
	message::DisplayString(NULL,0,bb);
    }
}
static void createcon(class view  *v)
{
    class arbiterview *abv;
    if(Gself == NULL) return;
    if((abv = Gself->CurrentArbiterview) == NULL) {
	message::DisplayString(NULL,0,"No Current Arbiterview"); 
	return;
    }
    arbcon::SetCurrentCelview(NULL);
    (abv)->CreateCon( (class text *)Gself->ArbTextEditView->etext);
}

static struct bind_Description arbconBindings[]={
    {"arbcon-copy-cel",NULL,0,"Arbcon~0,Copy Cel~3",0,0,(proctable_fptr)copy,"Copy the current cel"},
    {"arbcon-copy-link",NULL,0,"Arbcon~0,Copy Link~2",0,0,(proctable_fptr)copylink,"Copy link to current cel"},
    {"arbcon-cut-cel",NULL,0,"Arbcon~0,Cut Cel~10",0,0,(proctable_fptr)cut,"Cut the current cel"},
    {"arbcon-new-window",NULL,0,"Arbcon~0,New Window~20",0,0,(proctable_fptr)newwin,"prompt for a file for a new window"},
    {"arbcon-add-object",NULL,0,"Arbcon~0,Add Object~22",0,0,(proctable_fptr)addobview,"prompt for a new object view pair to add to the list"},
    {"arbcon-add-valueview-list",NULL,0,"Arbcon~0,Init Value Window~23",0,0,(proctable_fptr)newlist,"Bring up window of valueviews"},
    {"arbcon-chdir",NULL,0,"ADEW~1,Change Directory~1",0,0,(proctable_fptr)arbchdir,"Change Directory"},
    {"arbcon-creatcon",NULL,0,"ADEW~1,Create Controller~1",0,0,(proctable_fptr)createcon,"Create Controller"},
/* 
    {"arbcon-make-app",NULL,0,"ADEW~2,Make Application~2",0,0,makeapp,"Make Application"},
    {"arbcon-run-app",NULL,0,"ADEW~2,Run Application~2",0,0,runapp,"Run Application"},
*/
/*    {"arbcon-new-arb-window",NULL,0,"Arbcon~0,New Arbiter Window~20",0,0,arbnewwin,"prompt for a file for a new window and surround with an arbiter"},*/
    {"arbcon-show-cels",NULL,0,"Arbcon~0,Show Cels~21",0,0,(proctable_fptr)showcels,"Show cel names"},
    NULL
};
static boolean dolistfile(class arbcon  *self,const char  *s)
{
    /* open file and append to list of objects */
    FILE *f;
    char *buf;
    long size;
    if((f = fopen(s,"r")) == NULL) return FALSE;
    fseek(f,0,2);	
    size = ftell(f);
    fseek(f,0,0);
    if((buf = (char *) malloc(size + 1)) == NULL){
	fclose(f);
	return FALSE;
    }
    fread(buf,1,size,f);
    buf[size] = '\0';
    fclose(f);
    self->vwcount =appendlist(self->vwlist,self->vwcount,buf,FALSE);
    return TRUE;
}
static boolean createGself(class arbcon  *self)
{   /* initialization code */
    FILE *f;
    const char *p;
    char *m;
    struct ATKregistryEntry  *viewtype = ATK::LoadClass("view");
    Gself = self;
    controlV::SetAutoInit(FALSE);
    OwnArb = (class arbiterview *) (Gself->v)->WantHandler("arbiterview");
    (Arbiter(OwnArb))->SetNoSave(TRUE);
    (OwnArb)->AddObserver(self);
    self->vwlist = (char **) malloc(INITIALSIZE * sizeof(char *));
    self->vwcount = 0;

    if((p = environ::GetProfile("ObViewList"))!= NULL){
	if((m = (char *)malloc(strlen(p)+ 1)) == NULL) return FALSE;
	strcpy(m,p);
	self->vwcount =appendlist(self->vwlist,self->vwcount,m,FALSE);
    }
    if((p = environ::GetProfile("ObViewFile"))!= NULL){
	dolistfile(self,p);
    }
    if(environ::GetProfileSwitch("IgnoreDefaultViewList",FALSE) != TRUE){ 
	const char *dflist;
	if((dflist = environ::GetConfiguration("AdewObViewList")) == NULL){
	    if(dolistfile(self,VWLISTFILE))
		dflist = NULL;
	    else dflist = defaultvwlist;
	}
	if(dflist != NULL){
	    if((m = (char *)malloc(strlen(dflist)+ 1)) == NULL) return FALSE;
	    strcpy(m,dflist);
	    self->vwcount =appendlist(self->vwlist,self->vwcount,m,FALSE);
	}
    }
    (self->Arbobviewlist)->SetNotify(FALSE);
    (self->Arbobviewlist)->SetArraySize(self->vwcount);
    (self->Arbobviewlist)->SetNotify(TRUE);
    (self->Arbobviewlist)->SetStringArray(self->vwlist);

    atta[0] = atom::Intern("ArbLinkCel");
    atta[1] = atom::Intern("ArbCutCel");
    atta[2] = atom::Intern("ArbApplicationChoice");
    atta[3] = atom::Intern("Arbobviewlist");
    atta[4] = atom::Intern("ArbTextEdit");
    atta[5] = atom::Intern("copycon");
    atta[6] = NULL;
    self->ArbCelListView = NULL; 
    self->ArbCelList = new text;
    self->ArbArbList = new text;
    self->ObjectName = "";
    self->ArbiterName = "";
    self->ViewName = "";



    arbconMenus = new menulist;
    bind::BindList(arbconBindings, NULL , arbconMenus, viewtype);
    
    (self->arbv)->SetMenulist(arbconMenus);
#ifdef INFOFILE
    if((f = fopen(INFOFILE,"r")) != NULL){
	class text *txt;
	txt = (class text *) Gself->ArbTextEditView->etext;
	(txt)->Read(f,1);
	fclose(f);
    }
    else 
#endif /* INFOFILE */
    {
	/* all this to center the title, sigh */
	class style *Style = NULL;
	class environment *te;
	const char *buf ;
	class text *txt;

	buf = ARBCONMESSAGE;
	txt = (class text *) Gself->ArbTextEditView->etext;
	(txt)->AlwaysInsertCharacters(0,buf,strlen(buf));
	if(txt && (Style = (txt->styleSheet)->Find( "center")) == NULL){
	    Style = new style;
	    (Style)->SetName( "center");
	    (txt->styleSheet)->Add( Style);
	    (Style)->SetJustification(style_Centered);
	}
	te = (txt->rootEnvironment)->InsertStyle( 0, Style, TRUE);
	(te)->SetStyle( FALSE, FALSE);
	(te)->SetLength( (txt)->GetLength());
    }
    ((class view *)Gself->ArbTextEditView)->WantInputFocus( Gself->ArbTextEditView);
    ((class view *)Gself->ArbTextEditView)->AddObserver(self);
    init(self);
    return TRUE;
}
/* user code ends here for includes */

static class arbcon *firstarbcon;
static class arbcon *FindSelf(class view  *v)
{
	class arbcon *self,*last = NULL;
	class arbiterview *arbv =arbiterview::FindArb(v);
	for(self= firstarbcon; self != NULL; self = self->next){
		if(self->arbv == arbv) return self;
		last = self;
		}
	if(Gself != NULL){
	    return Gself;
	}
	self = new arbcon;
	self->arbv = arbv;
	initself(self,v);
	if(last == NULL) firstarbcon = self;
	else last->next = self;
	return self;
}
static void ArbLinkCelCallBack(class arbcon  *self,class value  *val,long  r1,long  r2)
{
if(r2 == value_OBJECTDESTROYED) {
	self->ArbLinkCel = NULL;
	self->ArbLinkCelView = NULL;
	return;
}
/* user code begins here for ArbLinkCelCallBack */
#if 0
    if((val)->GetValue() == 1){
	if(self->currentcelviewval == NULL){
	    (val)->SetValue(0);
	    message::DisplayString(NULL,0,"No current cel to link to");
	}
	else {
	     DoCopy(self,TRUE);
	}
    }
#endif /* 0 */
    if(self->currentcelviewval == NULL){
	if((val)->GetValue() == 1){
	    (val)->SetValue(0);
	    if(Gself)
	    message::DisplayString(NULL,0,"No current cel to link to !");
	}
    }	
    else {
	    DoCopy(self,TRUE);
	}


/* user code ends here for ArbLinkCelCallBack */
}
static void ArbCutCelCallBack(class arbcon  *self,class value  *val,long  r1,long  r2)
{
if(r2 == value_OBJECTDESTROYED) {
	self->ArbCutCel = NULL;
	self->ArbCutCelView = NULL;
	return;
}
/* user code begins here for ArbCutCelCallBack */
{
    char buf[256];
    if(Gself != NULL && Gself->currentcelviewval){
	sprintf(buf,"Cutting %s",(Cel(Gself->currentcelviewval))->GetRefName());
	message::DisplayString(NULL,0,buf);
	(Gself->currentcelviewval)->Copy();
	arbcon::DestroyCurrentCelview();
	SetNotice(buf);
    }
    else message::DisplayString((Gself == NULL)? NULL : Gself->arbv,0,"No current cel to cut");
}
/* user code ends here for ArbCutCelCallBack */
}
static void ArbApplicationChoiceCallBack(class arbcon  *self,class value  *val,long  r1,long  r2)
{
if(r2 == value_OBJECTDESTROYED) {
	self->ArbApplicationChoice = NULL;
	self->ArbApplicationChoiceView = NULL;
	return;
}
/* user code begins here for ArbApplicationChoiceCallBack */
    if(Gself == NULL) return;
if (Gself->currentcelviewval){
    int setto,curval;
    class cel *cl = Cel(Gself->currentcelviewval);
    setto = (Gself->ArbApplicationChoice)->GetValue();
    curval = ((cl)->GetApplication() == cel_APPLICATION);
    if((setto == 0)  !=  (curval == 0)){
	(val)->SetValue((setto == 0) ? 1:0);
	message::DisplayString(NULL,0,"Can't change application status of existing cel");
    }
}
else DoCopy(self,TRUE);
/* user code ends here for ArbApplicationChoiceCallBack */
}
static void ArbobviewlistCallBack(class arbcon  *self,class value  *val,long  r1,long  r2)
{
if(r2 == value_OBJECTDESTROYED) {
	self->Arbobviewlist = NULL;
	self->ArbobviewlistView = NULL;
	return;
}
/* user code begins here for ArbobviewlistCallBack */
    setobview(self,(val)->GetString(),((val)->GetValue() == 1));
/* user code ends here for ArbobviewlistCallBack */
}
static void ArbTextEditCallBack(class arbcon  *self,class value  *val,long  r1,long  r2)
{
if(r2 == value_OBJECTDESTROYED) {
	self->ArbTextEdit = NULL;
	self->ArbTextEditView = NULL;
	return;
}
/* user code begins here for ArbTextEditCallBack */
    arbcon::SaveCurrentCelview();
/* user code ends here for ArbTextEditCallBack */
}
static void initself(class arbcon  *self,class view  *v)
{
	self->v = v;
	self->ArbLinkCelView = (class onoffV *)arbiterview::GetNamedView(v,"ArbLinkCel");
	self->ArbLinkCel = (class value *)arbiterview::GetNamedObject(v,"ArbLinkCel");
	if(self->ArbLinkCel) (self->ArbLinkCel)->AddCallBackObserver( self,(value_fptr)ArbLinkCelCallBack,0);
	if(self->ArbLinkCelView) ((class view *)self->ArbLinkCelView)->AddObserver(self);
	self->ArbCutCelView = (class buttonV *)arbiterview::GetNamedView(v,"ArbCutCel");
	self->ArbCutCel = (class value *)arbiterview::GetNamedObject(v,"ArbCutCel");
	if(self->ArbCutCel) (self->ArbCutCel)->AddCallBackObserver( self,(value_fptr)ArbCutCelCallBack,0);
	if(self->ArbCutCelView) ((class view *)self->ArbCutCelView)->AddObserver(self);
	self->ArbApplicationChoiceView = (class onoffV *)arbiterview::GetNamedView(v,"ArbApplicationChoice");
	self->ArbApplicationChoice = (class value *)arbiterview::GetNamedObject(v,"ArbApplicationChoice");
	if(self->ArbApplicationChoice) (self->ArbApplicationChoice)->AddCallBackObserver( self,(value_fptr)ArbApplicationChoiceCallBack,0);
	if(self->ArbApplicationChoiceView) ((class view *)self->ArbApplicationChoiceView)->AddObserver(self);
	self->ArbobviewlistView = (class clicklistV *)arbiterview::GetNamedView(v,"Arbobviewlist");
	self->Arbobviewlist = (class value *)arbiterview::GetNamedObject(v,"Arbobviewlist");
	if(self->Arbobviewlist) (self->Arbobviewlist)->AddCallBackObserver( self,(value_fptr)ArbobviewlistCallBack,0);
	if(self->ArbobviewlistView) (self->ArbobviewlistView)->AddObserver(self);
	self->ArbTextEditView = (class menterstrV *)arbiterview::GetNamedView(v,"ArbTextEdit");
	self->ArbTextEdit = (class value *)arbiterview::GetNamedObject(v,"ArbTextEdit");
	if(self->ArbTextEdit) (self->ArbTextEdit)->AddCallBackObserver( self,(value_fptr)ArbTextEditCallBack,0);
	if(self->ArbTextEditView) ((class view *)self->ArbTextEditView)->AddObserver(self);
}
void arbcon_copycon(class view  *v,long  dat)
 {
class arbcon *self;
if((self = FindSelf(v)) == NULL) return;
/* user code begins here for arbcon_copycon */
if(Gself == NULL){
    if(createGself(self) == FALSE) Gself = NULL;
}
else{
    class arbiterview *arbv =arbiterview::FindArb(v);
    if(((class celview *)arbv)->menulistp == NULL){
	struct ATKregistryEntry  *viewtype = ATK::LoadClass("view");
	arbconMenus = new menulist;
	bind::BindList(arbconBindings, NULL , arbconMenus, viewtype);
	(arbv)->SetMenulist(arbconMenus);
    }
    /*
      fprintf(stderr,"In CopyCon\n");
      fflush(stderr);
      */
    DoCopy(Gself,TRUE);
}
/* user code ends here for arbcon_copycon */
}
void arbcon::ObservedChanged(class observable  * observed,long  status)
{
/* user code begins here for ObservedChanged */
    if(status == observable_OBJECTDESTROYED &&
	observed == (class observable * )OwnArb &&
	this == Gself){
	this->ArbTextEditView = NULL;
#ifdef DESTROYFAILS
	OwnArb = NULL;
	Gself = NULL;
#else /*  DESTROYFAILS */
  	(this)->Destroy();  
#endif /*  DESTROYFAILS */
    }
/* user code ends here for ObservedChanged */
    if (status == observable_OBJECTDESTROYED) {
	if (observed == (class observable *) this->ArbLinkCelView) this->ArbLinkCelView=NULL;
	if (observed == (class observable *) this->ArbCutCelView) this->ArbCutCelView=NULL;
	if (observed == (class observable *) this->ArbApplicationChoiceView) this->ArbApplicationChoiceView=NULL;
	if (observed == (class observable *) this->ArbobviewlistView) this->ArbobviewlistView=NULL;
	if (observed == (class observable *) this->ArbTextEditView) this->ArbTextEditView=NULL;
    }
}
boolean arbcon::InitializeClass()
{
struct ATKregistryEntry  *viewtype = ATK::LoadClass("view");
firstarbcon = NULL;
proctable::DefineProc("arbcon-copycon",(proctable_fptr)arbcon_copycon, viewtype,NULL,"arbcon copycon");
/* user code begins here for InitializeClass */
proctable::DefineProc("arbcon-create",(proctable_fptr)arbcon_Create, viewtype,NULL,"Create an Arbcon");
Gself= NULL;
/* user code ends here for InitializeClass */
return TRUE;
}
arbcon::~arbcon()
{
	ATKinit;

	if(this->ArbLinkCel) (this->ArbLinkCel)->RemoveCallBackObserver( this);
	if(this->ArbCutCel) (this->ArbCutCel)->RemoveCallBackObserver( this);
	if(this->ArbApplicationChoice) (this->ArbApplicationChoice)->RemoveCallBackObserver( this);
	if(this->Arbobviewlist) (this->Arbobviewlist)->RemoveCallBackObserver( this);
	if(this->ArbTextEdit) (this->ArbTextEdit)->RemoveCallBackObserver( this);
/* user code begins here for FinalizeObject */
	if(this == firstarbcon){
	    firstarbcon = this->next; /* should always be NULL */
	}
	if(this->ArbTextEditView){
	    ((class view *)this->ArbTextEditView)->RemoveObserver(this);
	}
	if(this->ArbCelList)
	    (this->ArbCelList)->Destroy();
	if(this->ArbArbList)
	    (this->ArbArbList)->Destroy();
	    
	if(this == Gself) {
	    Gself = NULL;
	    if(OwnArb){
		(OwnArb)->RemoveObserver(this);
		OwnArb = NULL;
		controlV::SetAutoInit(TRUE);
	    }
	}
	
/* user code ends here for FinalizeObject */
}
arbcon::arbcon()
{
	ATKinit;
this->ArbLinkCel = NULL;
this->ArbLinkCelView = NULL;
this->ArbCutCel = NULL;
this->ArbCutCelView = NULL;
this->ArbApplicationChoice = NULL;
this->ArbApplicationChoiceView = NULL;
this->Arbobviewlist = NULL;
this->ArbobviewlistView = NULL;
this->ArbTextEdit = NULL;
this->ArbTextEditView = NULL;
this->v = NULL;
this->next = NULL;
/* user code begins here for InitializeObject */
{

	ObjectName=NULL;
	ViewName=NULL;
    this->obsize = this->vwsize =  INITIALSIZE;
    this->currentcelviewval = NULL;
    this->obcount = this->vwcount = 0;
    this->CurrentArbiterview = NULL;
    this->arrlen = 128;
    this->arr = (char *)malloc(this->arrlen);
    this->overlay = NULL;
    obnamelist=NULL;
    vwlist=NULL;
    cl=NULL;
    ArbCelList=NULL;
    ArbCelListView=NULL;
    ArbArbList=NULL;
    ArbiterName=NULL;
}
/* user code ends here for InitializeObject */
THROWONFAILURE( TRUE);}
/* user code begins here for Other Functions */
/* user code ends here for Other Functions */
