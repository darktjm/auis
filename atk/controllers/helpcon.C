/* user code begins here for HeaderInfo */

/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */
 
/* user code ends here for HeaderInfo */
#include <andrewos.h>
ATK_IMPL("helpcon.H")
#include "proctable.H"
#include "view.H"
#include "arbiterview.H"
#if POSIX_ENV || defined(M_UNIX)
#include <dirent.h>
#else
#define dirent direct
#endif
#include "helpcon.H"
#include "celview.H"
#include "cltextview.H"
#include "text.H"
#include "controlV.H"
#include "cel.H"
#include "lsetview.H"
#include "lset.H"
#include "value.H"
#include "textview.H"
#include "stringV.H"
/* user code begins here for includes */
#include "message.H"
#include "filetype.H"
#include "environ.H"
#include <ctype.h>

/* #define HELPDIR "/usr/andy/help" */
#define HELPDIR environ::AndrewDir("/help") 


ATKdefineRegistry(helpcon, ATK, helpcon::InitializeClass);
static void NoteHistory(class helpcon  *self);
static void settopics(class helpcon  *self);
static boolean GetHelpOn(class helpcon  *self,char  *buf,char  *type,boolean SaveHistory);
static void handleclicks(class helpcon  *self,class cltextview  *cv,long  *position, long  *numberOfClicks, enum view::MouseAction  *action, long  *startLeft, long  *startRight, long  *leftPos, long  *rightPos,long  which,long  type);
static class helpcon *FindSelf(class view  *v);
static void topicschoiceCallBack(class helpcon  *self,class value  *val,long  r1,long  r2);
static void choicelabelCallBack(class helpcon  *self,class value  *val,long  r1,long  r2);
static void initself(class helpcon  *self,class view  *v);
static void helpcon_help(class view  *v,long  dat);


static void NoteHistory(class helpcon  *self)
{   /* insert current file and position into the history text buffer */
    char buf[256];
    class text *txt =  self->historytext;
    if(*self->CurrentName == '\0') return;
    sprintf(buf,"%s (%ld,%ld)\n",self->CurrentName,(self->bodyView)->GetDotPosition(), (self->bodyView)->GetDotLength());
    (txt)->AlwaysInsertCharacters( (txt)->GetLength(),buf,strlen(buf));
    (txt)->NotifyObservers(1);
}
static void settopics(class helpcon  *self)
{   /* insert current file and position into the history text buffer */
    struct dirent **dl;
    char *cp,*dp;
    long count;
    (self->topicschoice)->SetString(self->CurrentType);
    (self->topics)->Clear();
    if(*(self->CurrentType) == '\0')
	(self->topics)->AlwaysCopyText(0,self->historytext,0,(self->historytext)->GetLength());
    else for(dl = self->dl ,count = self->count; count--; dl++){
	cp = (*dl)->d_name;
	dp = strchr(cp,'.');
	if(dp && strcmp((dp + 1),self->CurrentType) == 0){
	    (self->topics)->AlwaysInsertCharacters( (self->topics)->GetLength(),cp,dp - cp);
	    (self->topics)->AlwaysInsertCharacters( (self->topics)->GetLength(),"\n",1);
	}
    }
    (self->topics)->NotifyObservers(1);
}

static boolean GetHelpOn(class helpcon  *self,char  *buf,char  *type,boolean SaveHistory)
{   /* look up the file in the help directory and insert in the body text */
    char bb[512],*objectName,*cp;
    FILE *f;
    long objectID;
    if((cp = strchr(buf,'(')) != NULL)
	*(cp++ - 1) = '\0';
    if(type && *type)
	sprintf(bb,"%s/%s.%s",HELPDIR,buf,type);
    else 
	sprintf(bb,"%s/%s",HELPDIR,buf);
    if((f = fopen(bb,"r")) == NULL){
	sprintf(bb,"Can't find help on %s",buf);
	message::DisplayString(NULL,0,bb);
	return FALSE;
    }
    if(SaveHistory) NoteHistory(self);
    if(type && *type)
	sprintf(self->CurrentName,"%s.%s",buf,type);
    else 
	sprintf(self->CurrentName,"%s",buf);
    (self->body)->Clear();
    objectName = filetype::Lookup(f, bb, &objectID, NULL);
    rewind(f);
    if(objectID == 0 && (strcmp(objectName,"text") == 0))
	/* reading old be1 datastream file */
	(self->body)->Read(f,0);
    else (self->body)->AlwaysInsertFile(f,bb,0);
    (self->body)->NotifyObservers(1); 
    fclose(f);
    if(cp){
	(self->bodyView)->SetDotPosition(atoi(cp));
	if((cp = strchr(cp,',')) != NULL){
	    cp++;
	    (self->bodyView)->SetDotLength(atoi(cp));
	}
	(self->bodyView)->FrameDot((self->bodyView)->GetDotPosition());
    }
    return TRUE;
}
static void handleclicks(class helpcon  *self,class cltextview  *cv,long  *position, long  *numberOfClicks, enum view::MouseAction  *action, long  *startLeft, long  *startRight, long  *leftPos, long  *rightPos,long  which,long  type)
{   /* deal with clicks */
    char buf[256],*cp;
    int len;
    int SaveHistory = TRUE;
    if(type == cltextview_PREPROCESS){
	*numberOfClicks = 3;
	return;
    }
    *numberOfClicks = 1;
    len = *rightPos - *leftPos;
    if(*action == view::LeftUp &&  len > 0 && len < 256){	
	if(which != 0){
	    (self->choice)->CopySubString(*leftPos, len,self->CurrentType,FALSE);
	    if(strcmp(self->CurrentType,"History") == 0){
	       *self->CurrentType = '\0';
	       self->ShowHistory = TRUE;
	       }
	    else self->ShowHistory = FALSE;
	    settopics(self);
	}
	else {
	    (self->topics)->CopySubString(*leftPos, len,buf,FALSE);
	    if((! self->ShowHistory) || which != 0){
		if((cp = strchr(buf,'\n')) != NULL) *cp = '\0';
	    }
	    else SaveHistory = FALSE;
	    GetHelpOn(self,buf,(self->ShowHistory ? NULL:self->CurrentType),SaveHistory);
	    (self->body)->NotifyObservers(1);
	    if(!SaveHistory)(self->bodyView)->WantInputFocus(self->bodyView);
	}
    }
}

/* user code ends here for includes */

static class helpcon *firsthelpcon;
static class helpcon *FindSelf(class view  *v)
{
	class helpcon *self,*last = NULL;
	class arbiterview *arbv = (class arbiterview *) (v)->WantHandler("arbiterview");
	for(self= firsthelpcon; self != NULL; self = self->next){
		if(self->arbv == arbv) return self;
		last = self;
		}
	self = new helpcon;
	self->arbv = arbv;
	initself(self,v);
	if(last == NULL) firsthelpcon = self;
	else last->next = self;
	return self;
}
static void topicschoiceCallBack(class helpcon  *self,class value  *val,long  r1,long  r2)
{
/* user code begins here for topicschoiceCallBack */
/* user code ends here for topicschoiceCallBack */
}
static void choicelabelCallBack(class helpcon  *self,class value  *val,long  r1,long  r2)
{
/* user code begins here for choicelabelCallBack */
/* user code ends here for choicelabelCallBack */
}
static void initself(class helpcon  *self,class view  *v)
{
	self->v = v;
	self->topicsView = (class cltextview *)arbiterview::GetNamedView(v,"topics");
	self->topics = (class text *)arbiterview::GetNamedObject(v,"topics");
	self->choiceView = (class cltextview *)arbiterview::GetNamedView(v,"choice");
	self->choice = (class text *)arbiterview::GetNamedObject(v,"choice");
	self->topicschoiceView = (class stringV *)arbiterview::GetNamedView(v,"topicschoice");
	self->topicschoice = (class value *)arbiterview::GetNamedObject(v,"topicschoice");
	if(self->topicschoice) (self->topicschoice)->AddCallBackObserver( self,(value_fptr)topicschoiceCallBack,0);
	self->bodyView = (class textview *)arbiterview::GetNamedView(v,"body");
	self->body = (class text *)arbiterview::GetNamedObject(v,"body");
	self->choicelabelView = (class stringV *)arbiterview::GetNamedView(v,"choicelabel");
	self->choicelabel = (class value *)arbiterview::GetNamedObject(v,"choicelabel");
	if(self->choicelabel) (self->choicelabel)->AddCallBackObserver( self,(value_fptr)choicelabelCallBack,0);
}
static void helpcon_help(class view  *v,long  dat)
 {
class helpcon *self;
if((self = FindSelf(v)) == NULL) return;
/* user code begins here for helpcon_help */
{
    char *cp,*dp,*c,buf[1024];
    struct dirent **dl;
    long count,len;
     if(self->historytext == NULL){
	 /* initialize the history text and set up the click observers */
	 arbiterview::SetIgnoreUpdates(v, TRUE);
	 self->historytext = new text;
	 self->count = scandir(HELPDIR,&dl,NULL,SCANDIRCOMPFUNC(alphasort));
	 self->dl = dl;
	 if(self->topics) (self->topicsView)->AddClickObserver(self,(cltextview_hitfptr)handleclicks,0);
	 if(self->choice) (self->choiceView)->AddClickObserver(self,(cltextview_hitfptr)handleclicks,1);
	 (self->historytext)->InsertCharacters(0,"History\n\n",9);
	 message::DisplayString(NULL,0,"Help is now initialized");
	 return;
     }
     if(self->bodyView != NULL){
	 char helpchoice[264];
	 int dotlen = (self->bodyView)->GetDotLength();
	 if( dotlen > 0 && dotlen < 256 ){
	     /* use the selected string as the help topic */
	     (self->body)->CopySubString( (self->bodyView)->GetDotPosition() ,dotlen,helpchoice,FALSE);
	 }
	 else /* prompt for help topic  */
	     if(message::AskForString(NULL,0,"Enter a keyword: ",NULL,helpchoice,256) < 0) return;
	 for(c = helpchoice; *c != '\0'; c++){
	     if(isupper(*c)) *c = tolower(*c);
	     if(isspace(*c)) {
		 *c = '\0';
		 break;
	     }
	 }
	 len =  strlen(helpchoice);
	 for(dl = self->dl ,count = self->count; count--; dl++){
	     cp = (*dl)->d_name;
	     dp = strchr(cp,'.');
	     if(dp - cp == len && 
		strncmp(cp,helpchoice,len) == 0){
		 GetHelpOn(self,cp,NULL,TRUE);
		 return;
	     }
	 }
	 sprintf(buf,"Can't find help on %s",helpchoice);
	 message::DisplayString(NULL,0,buf);
     }
}
/* user code ends here for helpcon_help */
 }
boolean helpcon::InitializeClass()
{
struct ATKregistryEntry  *viewtype = ATK::LoadClass("view");
firsthelpcon = NULL;
proctable::DefineProc("helpcon-help",(proctable_fptr)helpcon_help, viewtype,NULL,"helpcon help");
/* user code begins here for InitializeClass */
/* user code ends here for InitializeClass */
return TRUE;
}
helpcon::helpcon()
{
	ATKinit;

this->topics = NULL;
this->topicsView = NULL;
this->choice = NULL;
this->choiceView = NULL;
this->topicschoice = NULL;
this->topicschoiceView = NULL;
this->body = NULL;
this->bodyView = NULL;
this->choicelabel = NULL;
this->choicelabelView = NULL;
this->v = NULL;
this->next = NULL;
/* user code begins here for InitializeObject */
this->historytext = NULL;
this->ShowHistory = FALSE;
*this->CurrentName = '\0';
/* user code ends here for InitializeObject */
THROWONFAILURE( TRUE);}
/* user code begins here for Other Functions */
/* user code ends here for Other Functions */

