/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'     *
\* ********************************************************************** */

/*
	$Disclaimer: 
 * Permission to use, copy, modify, and distribute this software and its 
 * documentation for any purpose and without fee is hereby granted, provided 
 * that the above copyright notice appear in all copies and that both that 
 * copyright notice and this permission notice appear in supporting 
 * documentation, and that the name of IBM not be used in advertising or 
 * publicity pertaining to distribution of the software without specific, 
 * written prior permission. 
 *                         
 * THE COPYRIGHT HOLDERS DISCLAIM ALL WARRANTIES WITH REGARD 
 * TO THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF 
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL ANY COPYRIGHT 
 * HOLDER BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL 
 * DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, 
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE 
 * OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION 
 * WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 * 
 *  $
*/

#include <andrewos.h>
#include <sys/signal.h>
#include <sys/errno.h>
#ifdef linux
#include <bsd/sgtty.h>
#else

#ifdef SOLARIS
#undef M_UNIX
#define M_UNIX 1
#endif

#ifdef M_UNIX
#include <sys/termio.h>
#include <sys/stream.h>
#include <sys/ptem.h>
#include <sys/ttold.h>
#define CRMOD O_CRMOD
#define RAW O_RAW
#define CBREAK O_CBREAK
#else
#include <sgtty.h>
#endif
#endif

#include <ctype.h>

#include <tmview.H>

#include <termulator.H>
#include <keymap.H>
#include <keystate.H>
#include <menulist.H>
#include <bind.H>
#include <proctable.H>
#include <message.H>
#include <environ.H>
#include <frame.H>
#include <im.H>
#include <mark.H>
#include <environment.H>
#include <fontdesc.H>
#include <text.H>
#include <txtstvec.h>

#include <util.h>

#if (defined(hpux) || defined(M_UNIX)) && !defined(CBREAK)
#define CBREAK 0
#endif /* hpux */

class keymap *tmviewKeymap=NULL;
class menulist *tmviewMenus=NULL;
struct proctable_Entry *rawKeyPE=NULL;

#define BORDERWIDTH 5

static proctable_fptr textview_BeginningOfLine=NULL;


ATKdefineRegistry(tmview, textview, tmview::InitializeClass);
void rawKey(class tmview  *self,long  key);
static enum keymap_Types rawKeyLookup(class tmview  *self,long  key,struct proctable_Entry  **peP,long  *rockP);
static void trackTermulator(class tmview  *self);
static void submit(class tmview  *self);
static void submitStay(class tmview  *self);
static void selfInsert(class tmview  *self,long  key);
static void sendSignal(class tmview  *self,long  key);
static void eraseLine(class tmview  *self);
static void replaceCmd(class tmview  *self,long  how);
static void getLine(class tmview  *self,int  pos,int  *beginP,int  *endP);
static void move(class tmview  *self);
static void exec(class tmview  *self);
static void deleteOrEOT(class tmview  *self);
static void startProc(class tmview  *self);
static void bol(class tmview  *self);
static void clear(class tmview  *self);
static void execStr(class tmview  *self,char  *str);


void rawKey(class tmview  *self,long  key)
{
    char buf=key;

    ((class termulator *)self->dataobject)->ProcessInput(
			      &buf,1);
}

void tmview::SetDataObject(class dataobject  *t)
{
    class termulator *tm=(class termulator *)t;
    class termulator *oldtm=(class termulator *)this->dataobject;

    (this)->textview::SetDataObject(tm);

    if(oldtm!=NULL){
	(oldtm)->RemoveMark(this->curpos);
	delete this->curpos;
    }

    if(tm!=NULL){
	(tm)->SetScreenSize(80,24);
	this->curpos=(tm)->CreateMark(0,0);
	(this->curpos)->SetStyle(FALSE,FALSE); /* so it acts like the dot */
	if(tm->inpView==NULL){
	    tm->inpView=this;
	    this->height=this->width= -1; /* recalc on redraw */
	}
    }
}

static enum keymap_Types rawKeyLookup(class tmview  *self,long  key,struct proctable_Entry  **peP,long  *rockP)
{
    *peP=rawKeyPE;
    *rockP=key;

    return keymap_Proc;
}

static void trackTermulator(class tmview  *self)
{
    class termulator *tm=(class termulator *)self->dataobject;
    int pos=tm->cursor;

    if(tm->mode&(CBREAK|RAW))
	(self->keystate)->SetOverride((keystate_fptr)rawKeyLookup,(long)self);
    else
	(self->keystate)->SetOverride(NULL,0);

    if(tm->errstr!=NULL && self->hasInputFocus){
	char buf[100];
	if(tm->errno!=0){
	    sprintf(buf,"%s: %s",tm->errstr,strerror(tm->errno));
	    message::DisplayString(self,0,buf);
	}else
	    message::DisplayString(self,0,tm->errstr);
	tm->errstr=NULL;
    }

    if(self->screen!=tm->screen){
	if(tm->screen>=0 /* &&
	   !tmview_Visible(self,tm->screen) ||
	   !tmview_Visible(self,tm->screen+(tm->width+1)*tm->height) */)
	    (self)->SetTopPosition(tm->screen);
	self->screen=tm->screen;
    }

    if(self->cwd!=tm->cwd && self->title==NULL){
	((self)->GetIM())->SetTitle(tm->cwd);
	self->cwd=tm->cwd;
    }

    if((self)->GetDotLength()==0){
	int dot=(self)->GetDotPosition();
	if(dot!=pos &&
	   (tm->mode&(CBREAK|RAW) ||
	    dot==(self->curpos)->GetPos())){
	   (self)->SetDotPosition(pos);
	   if((self)->Visible(dot))
	       (self)->FrameDot(pos);
	}
	(self->curpos)->SetPos(pos);
    }
}

void tmview::Update()
{
    trackTermulator(this);
    (this)->textview::Update();
}

void tmview::FullUpdate(enum view_UpdateType  type,long  left,long  top,long  width,long  height)
{
    class termulator *tm=(class termulator *)this->dataobject;

    if(tm->inpView==this && (width!=this->width || height!=this->height)){
	struct text_statevector sv;
	struct FontSummary *fs;

	text::InitStateVector(&sv);
	if(tm->screen < 0)
	    text::ApplyEnvironment(&sv,
				  tm->screenStyle,
				  tm->rootEnvironment);
	else
	    text::ApplyEnvironment(&sv,tm->screenStyle,tm->screenEnv);
	/* ApplyEnvironment doesn't do this */
	sv.CurCachedFont=fontdesc::Create(sv.CurFontFamily,
					 sv.CurFontAttributes,
					 sv.CurFontSize);
	fs=(sv.CurCachedFont)->FontSummary((this)->GetDrawable());
	(tm)->SetDispSize(
			       (width-2*BORDERWIDTH)/fs->maxSpacing,
			       (height-2*BORDERWIDTH)/fs->maxHeight);
	this->width=width;
	this->height=height;
    }

    if(this->title!=NULL)
	((this)->GetIM())->SetTitle(this->title);

    trackTermulator(this);
    (this)->textview::FullUpdate(type,left,top,width,height);
}

void tmview::ReceiveInputFocus()
{
    (this)->textview::ReceiveInputFocus();
}    

tmview::tmview()
{
	ATKinit;

    this->keystate=keystate::Create(this,tmviewKeymap);
    this->curpos=NULL;
    this->screen= -1;
    this->cwd=NULL;
    this->menus=(tmviewMenus)->DuplicateML(this);
    this->shmenus=NULL;
    (this)->SetBorder(BORDERWIDTH,BORDERWIDTH);
    this->height=this->width=0;
    this->title=NULL;
    THROWONFAILURE( TRUE);
}

tmview::~tmview()
{
	ATKinit;

    class termulator *tm=(class termulator *)this->dataobject;

    if(this->curpos!=NULL && tm!=NULL){
	(tm)->RemoveMark(this->curpos);
	delete this->curpos;
    }
}	

void tmview::GetClickPosition(long  pos,long  noc,enum view_MouseAction  action,long  startLeft,long  startRight,long  *leftPosP,long  *rightPosP)
{
    if(noc%3!=0){
	(this)->textview::GetClickPosition(pos,noc,action,startLeft, startRight,leftPosP,rightPosP);
    }else{
	class termulator *tm=
	  (class termulator *)this->dataobject;
	class environment *rootEnv=tm->rootEnvironment,*env;

	(tm)->Untermulate();

	env=(class environment *)(rootEnv)->GetEnclosing(pos);
	if(env==rootEnv)
	    (this)->textview::GetClickPosition(pos,noc,action,startLeft, startRight,leftPosP,rightPosP);
	else{
	    *leftPosP=(env)->Eval();
	    *rightPosP=(env)->GetLength()+*leftPosP;
	}
    }
}

void tmview::PostKeyState(class keystate  *ks)
{
    class termulator *tm=(class termulator *)this->dataobject;

    if(tm->mode&(CBREAK|RAW))
	(this->keystate)->SetOverride((keystate_fptr)rawKeyLookup,(long)this);
    else
	(this->keystate)->SetOverride(NULL,0);

    this->keystate->next=NULL;
    (this)->textview::PostKeyState((this->keystate)->AddBefore(ks));
}

void tmview::PostMenus(class menulist  *ml)
{
    if(this->shmenus!=NULL)
	(this->menus)->ChainAfterML(this->shmenus,1);
    if(ml!=this->menus)
	(this->menus)->ChainAfterML(ml,0);
    (this)->textview::PostMenus(this->menus);
}

static void submit(class tmview  *self)
{
    class termulator *tm=(class termulator *)self->dataobject;
    int pos;

    tm->inpView=self;

    (tm)->Untermulate();

    pos=(tm)->GetLength();

    (self->curpos)->SetPos(pos);
    (self)->SetDotPosition(pos);
    (self)->SetDotLength(0);

    (tm)->Submit();

    (self)->FrameDot(tm->cursor);
}

static void submitStay(class tmview  *self)
{
    class termulator *tm=(class termulator *)self->dataobject;
    int pos=(tm)->GetLength();

    tm->inpView=self;

    (self)->SetDotPosition(pos);
    (self)->SetDotLength(0);
    while(pos>0 && (tm)->GetChar(pos-1)!='\n')
	pos--;
    (self)->SetTopPosition(pos);

    (tm)->Submit();

    /* assumedly, the length has changed, so curpos!=dot after this */
    (self->curpos)->SetPos((tm)->GetLength());
}

static void selfInsert(class tmview  *self,long  key)
{
    char c=key;
    class termulator *tm=(class termulator *)self->dataobject;
    int pos;

    tm->inpView=self;

    (tm)->Untermulate();

    (self)->CollapseDot();

    pos=(self)->GetDotPosition();
    if(pos<(tm)->GetFence())
	pos=(tm)->GetLength();

#ifdef HACKEDNOECHO
    if(tm->mode&ECHO)
#endif /* HACKEDNOECHO */
	(tm)->InsertCharacters(pos++,&c,1);
#ifdef HACKEDNOECHO
    else
	(tm)->ProcessInput(&c,1);
#endif /* HACKEDNOECHO */

    (self)->FrameDot(pos);
    (self)->SetDotPosition(pos);

    (tm)->NotifyObservers(0);
}

static void sendSignal(class tmview  *self,long  key)
{
    char c=key;
    class termulator *tm=
      (class termulator *)self->dataobject;

    tm->inpView=self;

    (tm)->ProcessInput(&c,1);
    (tm)->NotifyObservers(0);
}

static void eraseLine(class tmview  *self)
{
    class termulator *tm=
      (class termulator *)self->dataobject;
    int fence;

    tm->inpView=self;

    (tm)->Untermulate();

    fence=(tm)->GetFence();

#ifdef HACKEDNOECHO
    if(tm->mode&ECHO)
#endif /* HACKEDNOECHO */
	(tm)->DeleteCharacters(fence,(tm)->GetLength()-fence);

    (self)->FrameDot((self)->GetDotPosition());

    (tm)->NotifyObservers(0);
}

#define REP_PREV 1
#define REP_NEXT 2
#define REP_MATCH 4

static void replaceCmd(class tmview  *self,long  how)
{
    class termulator *tm=
      (class termulator *)self->dataobject;
    int pos,len,fence,c;
    char *cmd,buf[200],*p=buf;

    tm->inpView=self;

    (tm)->Untermulate();

    pos=(self)->GetDotPosition();
    len=(self)->GetDotLength();
    fence=(tm)->GetFence();

    if((tm)->GetLength()==fence)
	(tm)->RewindCmds();

    if(how&REP_MATCH)
	while(fence<pos && (c=(tm)->GetChar(fence++))!=EOF)
	    *p++=c;
    *p='\0';

    cmd=(how&REP_PREV ? (tm)->GrabPrevCmd(buf) : (tm)->GrabNextCmd(buf));

    if(cmd==NULL){
	message::DisplayString(self,1,"No more commands!");
	return;
    }

    cmd+=p-buf;

    if(fence>pos){
	len-=(fence-pos);
	pos=fence;
	if(len<0)
	    len=0;
    }

    (tm)->DeleteCharacters(pos,len);

    len=strlen(cmd);
    (tm)->InsertCharacters(pos,cmd,len);

    (self)->FrameDot(pos);
    (self)->SetDotPosition(pos);
    (self)->SetDotLength(len);

    (tm)->NotifyObservers(0);
}

static void getLine(class tmview  *self,int  pos,int  *beginP,int  *endP)
{
    class termulator *tm=
      (class termulator *)self->dataobject;
    class environment *env=tm->rootEnvironment;
    int c,nc;
    int end=pos;

    while(pos>0 && (tm)->GetChar(pos-1)!='\n')
	pos--;
    *beginP=pos;
    while((c=(tm)->GetChar(end))!=EOF && c!='\n')
	end++;
    *endP=end;

    nc=(env)->GetNextChange(pos)+pos;
    if(nc<end){
	env=(class environment *)(env)->GetInnerMost(nc);
	*beginP=nc;
	*endP=nc+(env)->GetLength();
    }
}

static void move(class tmview  *self)
{
    int pos,len,begin,end;
    class termulator *tm=
      (class termulator *)self->dataobject;

    tm->inpView=self;

    (tm)->Untermulate();

    pos=(self)->GetDotPosition();
    len=(self)->GetDotLength();
    end=(tm)->GetLength();
    begin=end;

    if(len==0){
	int endpos;
	if(pos>=end){
	    replaceCmd(self,REP_PREV);
	    return;
	}
	getLine(self,pos,&pos,&endpos);
	len=endpos-pos;
    }

    while(len-->0){
	char c=(tm)->GetChar(pos++);
	(tm)->InsertCharacters(end++,&c,1);	
    }

    (self)->SetDotLength(end-begin);
    (self)->SetDotPosition(begin);
    (self)->FrameDot(begin);
}

static void exec(class tmview  *self)
{
    move(self);
    submit(self);
}

static void deleteOrEOT(class tmview  *self)
{
    class termulator *tm=(class termulator *)self->dataobject;
    int pos;
    int c;

    tm->inpView=self;

    (tm)->Untermulate();

    pos=(self)->GetDotPosition();

    if((c=(tm)->GetChar(pos))=='\n' || c==EOF)
	(tm)->EOT();
    else
	(tm)->DeleteCharacters(pos,1);

    (self)->FrameDot(pos);

    (tm)->NotifyObservers(0);
}

static void startProc(class tmview  *self)
{
    class termulator *tm=(class termulator *)self->dataobject;
    char buf[5000],*com;
    char *shell=environ::Get("SHELL");
    char *argbuf[500],**argv;

    tm->inpView=self;

    if(shell==NULL)
	shell="/bin/sh";

#define processExists(pid) (kill(pid,0)!=-1 || errno!=ESRCH)

    if(tm->pid!=0 && !processExists(tm->pid))
	tm->pid=0;
    
    if(tm->pid!=0 &&
       tm->ptyFile!=NULL &&
       (message::AskForString(self,
			     0,
			     "Process already exists, start a new one? [n] ",
			     NULL,
			     buf,sizeof(buf))==-1 ||
	*buf!='y'))
	return;

    if(tm->pid!=0 &&
       message::AskForString(self,
			     0,
			     "Kill old process? [y] ",
			     NULL,
			     buf,sizeof(buf))==0 &&
       *buf=='y'){
	(tm)->Signal(SIGTERM);
	if(processExists(tm->pid))
	    (tm)->Signal(SIGKILL);
    }

    com=argvtostr(tm->args,buf,sizeof(buf));
    if(com==NULL)
	strcpy(buf,shell);

    if(message::AskForString(self,0,"Command: ",buf,buf,sizeof(buf))==-1)
	return;

    argv=strtoargv(buf,argbuf,sizeof(argbuf));
    if(argv==NULL){
	message::DisplayString(self,1,"Too many arguments.");
	return;
    }

    (tm)->StartProcess(argv);
}

static void bol(class tmview  *self)
{
    class termulator *tm=(class termulator *)self->dataobject;
    int pos,fence;

    tm->inpView=self;

    (tm)->Untermulate();

    pos=(self)->GetDotPosition();
    fence=(tm)->GetFence();

    if(textview_BeginningOfLine!=NULL)
	(*textview_BeginningOfLine)(self, 0);
    if(pos>fence && (self)->GetDotPosition()<fence)
	(self)->SetDotPosition(fence);
}

static void clear(class tmview  *self)
{
    ((class termulator *)self->dataobject)->Clear();
}

static void execStr(class tmview  *self,char  *str)
{
    class termulator *tm=(class termulator *)self->dataobject;
    eraseLine(self);
    (tm)->InsertCharacters((tm)->GetLength(),str,strlen(str));
    submit(self);
}

boolean tmview::ReadShMenus(const char  *filename)
{
    FILE *fp;
    char buf[1000];
    struct proctable_Entry *pe=
      proctable::DefineProc("tmview-exec-str",
			    (proctable_fptr)execStr,
			    &tmview_ATKregistry_ ,
			    NULL,
			    "Execute a string.");

    if(this->shmenus==NULL){
	this->shmenus=menulist::Create(this);
	if(this->shmenus==NULL)
	    return FALSE;
    }

    fp=fopen(filename,"r");
    if(fp==NULL)
	return FALSE;

    while(fgets(buf,500,fp)!=NULL){
	char *str=strchr(buf,':');
	if(str!=NULL){
	    char *save;
	    int len;
	    *str++='\0';
	    len=strlen(str);
	    str[len-1]='\0';
	    save=(char *)malloc(len);
	    strcpy(save,str);
	    (this->shmenus)->AddToML(buf,pe,(long)save,0);
	}else
	    (this->shmenus)->AddToML(buf,NULL,0,0);
    }

    fclose(fp);

    return TRUE;
}

struct bind_Description tmviewBindings[]={
    {"tmview-submit", "\r",0, NULL,0,0, (proctable_fptr)submit, "Submits a line of input to the subprocess."},
    {"tmview-submit-and-stay", "\n",0, NULL,0,0, (proctable_fptr)submitStay, "Submits a line of input, but also keeps cursor at this line, and puts it at the top of the screen."},
    {"tmview-signal", "\003",'\003', NULL,0,0, (proctable_fptr)sendSignal, "Send a signal to the subproces."},
    {"tmview-signal", "\032",'\032'},
    {"tmview-signal", "\034",'\034'},
    {"tmview-erase-line", "\025",0, NULL,0,0, (proctable_fptr)eraseLine, "Erases current line."},
    {"tmview-prev-cmd", "\033=",REP_PREV, NULL,0,0, (proctable_fptr)replaceCmd, "Retrieves previous command."},
    {"tmview-next-cmd", "\033-",REP_NEXT, NULL,0,0, (proctable_fptr)replaceCmd, "Retrieves next command."},
    {"tmview-next-cmd", "\033`",0},
    {"tmview-prev-match-cmd", "\033[",REP_PREV|REP_MATCH, NULL,0,0, (proctable_fptr)replaceCmd, "Retrieves previous matching command."},
    {"tmview-next-match-cmd", "\033]",REP_NEXT|REP_MATCH, NULL,0,0, (proctable_fptr)replaceCmd, "Retrieves next matching command."},
    {"tmview-move", "\033+",0, "Move~30",0,0, (proctable_fptr)move, "Move selected text to end."},
    {"tmview-exec", "\033\r",0, "Execute~30",0,0, (proctable_fptr)exec, "Move, then execute command."},
    {"tmview-delete-or-eot", "\004",0, NULL,0,0, (proctable_fptr)deleteOrEOT, "Sends EOT at end of line, otherwise deletes next character."},
    {"tmview-start-new-process", "\030!",0, "Termulator,Start new process",0,0, (proctable_fptr)startProc, "Starts a new child process."},
    {"tmview-beginning-of-line", "\001",0, NULL,0,0, (proctable_fptr)bol, "Beginning of line or input, whichever is closest."},
    {"tmview-clear", NULL,0, "Termulator,Clear",0,0, (proctable_fptr)clear, "Clears the text of the tm."},

    {NULL, NULL,0, "Save"},
    {NULL, NULL,0, "Switch File"},
    {NULL, NULL,0, "Plainer"},
    {NULL, NULL,0, "Plainest"},
    {NULL, NULL,0, "File,Save As",0,1},
    {NULL, NULL,0, "File,Save All"},
    {NULL, NULL,0, "File,Insert File"},
    {NULL, NULL,0, "File,Set Printer",0,1},
    {NULL, NULL,0, "File,Preview",0,1},
    {NULL, NULL,0, "File,Print",0,1},
    {NULL, NULL,0, "File,Add Template"},
    {NULL, NULL,0, "Search/Spell,Query Replace"},
    {NULL, NULL,0, "Search/Spell,Check Spelling"},
    NULL
};

void tmview::SetFileMenus(boolean  on)
{
    if((this->menus)->SetMask((on?0:1)))
	(this)->PostMenus(this->menus);
}

boolean tmview::InitializeClass()
{
    struct proctable_Entry *pe=
      proctable::DefineProc("tmview-self-insert",
			    (proctable_fptr)selfInsert,
			    &tmview_ATKregistry_ ,
			    NULL,
			    "If echoing, insert a key, otherwise store it.");
    char buf[2],c;

    if(textview_BeginningOfLine==NULL){
	struct proctable_Entry *pe;

	ATK::LoadClass("textview");
	pe=proctable::Lookup("textview-beginning-of-line");
	if(pe!=NULL)
	    textview_BeginningOfLine=proctable::GetFunction(pe);
    }

    tmviewKeymap=new keymap;
    tmviewMenus=new menulist;
    
    if(tmviewKeymap==NULL || tmviewMenus==NULL)
	return FALSE;
 bind::BindList(tmviewBindings,tmviewKeymap,tmviewMenus,&tmview_ATKregistry_ );
 
    rawKeyPE=
      proctable::DefineProc("tmview-raw-key",
			    (proctable_fptr)rawKey,&tmview_ATKregistry_ ,NULL,
			    "Send a key directly to the subprocess in this view.");

    buf[1]='\0';
    for(c=' '; c<127; c++){
	buf[0]=c;
	(tmviewKeymap)->BindToKey(buf,pe,c);
    }

    return TRUE;
}
