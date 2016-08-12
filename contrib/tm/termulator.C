/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/config/COPYRITE'     *
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

#ifndef NORCSID
#define NORCSID
static char rcsid[]="$Header: /afs/cs.cmu.edu/project/atk-src-C++/contrib/tm/RCS/termulator.C,v 1.7 1996/09/03 19:26:16 robr Exp $";
#endif


#include <andrewos.h> /* sys/types.h sys/file.h */
#include <ctype.h>
#include <sys/ioctl.h>
#include <sys/signal.h>
#include <sys/errno.h>
#ifndef O_NDELAY
#include <fcntl.h>
#endif /* O_NDELAY */



#include <termulator.H>

#include <mark.H>
#include <im.H>
#include <environ.H>
#include <filetype.H>
#include <stylesheet.H>
#include <environment.H>

#include <attribs.h>
#include <util.h>

#ifdef hpux
#include <sgtty.h>
#include <sys/ptyio.h>
#define CBREAK 0
#endif /* hpux */

#ifdef linux
#include <bsd/sgtty.h>
#endif

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
#endif

/* ---------------------------------------------------------------- */

#define scrPos(self,x,y) \
 ((self)->screen+(y)*((self)->width+1)+(x))

static char spaces[]="                                                                                ";
#define SPACES (sizeof(spaces)-1)


ATKdefineRegistry(termulator, text, NULL);
#ifndef NORCSID
#endif
#ifndef hp9000s300
#endif /* hp9000s300 */
#ifndef O_NDELAY
#endif /* O_NDELAY */
#ifdef hpux
#endif /* hpux */
#ifdef M_UNIX
#endif
static int fillChars(class termulator  *self,long  pos,long  num);
static long fillLines(class termulator  *self,long  pos,int  width,int  height);
static long stripEnds(class termulator  *self,long  num);
static boolean termulate(class termulator  *self);
void termulator__InsertChars(class termulator  *self,long  num);
static void readFromProc(FILE  *fp,class termulator  *self);
static char *ignoreChar(class termulator  *self,char  *buf,long  len);
static char *cntlChar(class termulator  *self,char  *buf,long  len);
static char *specialChar(class termulator  *self,char  *buf,register long  len);
static char *readDir(class termulator  *self,char  *buf,register long  len);
static void setupStyles(class termulator  *self);
#ifdef hp9000s300
#else /* hp9000s300 */
#endif /* hp9000s300 */
static void childDied(int  pid,class termulator  *self,int  *status);
#if defined(hpux) || defined(_IBMR2) || defined(NeXT) || defined(sys_pmax_ul4) || defined(sys_dec_alpha)
#ifndef INCORRECTSIGNALS
#endif /* INCORRECTSIGNALS */
#else /* !defined(hpux) && !defined(_IBMR2) && !defined(NeXT) && !defined(sys_pmax_ul4) */
#if !defined(M_UNIX)
#ifndef INCORRECTSIGNALS
#endif /* INCORRECTSIGNALS */
#else /* M_UNIX */
#ifndef INCORRECTSIGNALS
#endif /* INCORRECTSIGNALS */
#endif /*M_UNIX */
#endif /* defined(hpux) || defined(_IBMR2) || defined(NeXT) || defined(sys_pmax_ul4) */
static void preInsert(class termulator  *self,long  *posP ,long  *fenceP);
static void postInsert(class termulator  *self,long  pos,long  fence);


static int fillChars(class termulator  *self,long  pos,long  num)
{
    while(num>0){
	int inserted=(num>SPACES ? SPACES : num);
	(self)->text::AlwaysInsertCharacters(pos,spaces,inserted);
	num-=inserted;
	pos+=inserted;
    }

    return pos+num;
}

static long fillLines(class termulator  *self,long  pos,int  width,int  height)
{
    long fence=(self)->GetFence(),oldpos=pos;

    while(height--){
	pos=fillChars(self,pos,width);
	(self)->text::AlwaysInsertCharacters(pos++,"\n",1);
    }

    if(fence==oldpos)
	(self)->SetFence(pos);

    return pos;
}

static long stripEnds(class termulator  *self,long  num)
{
    long pos=scrPos(self,0,0)+self->width;

    while(num--){
	long end=pos;
	while(pos>0 && (self)->GetChar(pos-1)==' ')
	    pos--;
	if(pos<end)
	    (self)->AlwaysDeleteCharacters(pos,end-pos);
	pos+=1+self->width;
    }

    return pos-self->width;
}

void termulator::SetScreenSize(int  width,int  height)
{
    if(this->screen>=0){
	int dw=width-this->width;
	int dh=height-this->height;

	if(dh>0)
	    /* chop off bottom of too-high screen */
	    (this)->AlwaysDeleteCharacters(
					      scrPos(this,0,this->height),
					      dh*(this->width+1));
	else if(dh<0){
	    /* add lines to a too-short screen */
	    fillLines(this,
		      scrPos(this,0,this->height),
		      width,
		      height-this->height);
	}

	if(dw>0){
	    /* chop off all too-long lines */
	    long pos=scrPos(this,this->width,0);
	    long count=this->height-(dh>0?dh:0);

	    while(count--){
		(this)->AlwaysDeleteCharacters(pos,dw);
		pos+=1+width;
	    }
	}else if(dw<0){
	    /* extend all too-short lines */
	    long pos=scrPos(this,this->width,0);
	    long count=this->height-(dh>0?dh:0);

	    while(count--)
		pos=fillChars(this,pos,-dw)+1+width;
	}
    }

    this->width=width;
    this->height=height;
}

static boolean termulate(class termulator  *self)
{
    if(self->screen<0){
	long bpos=self->lastSubmit,
	    pos=(self)->GetFence();
	int ht=self->height,wid,
	    c;

	while(ht-->0 && pos>bpos){
	    while(pos>bpos &&
		  (self)->GetChar(pos--)!='\n')
		;
	}

	self->screen=pos;

	self->screenEnv=
	  (self->rootEnvironment)->InsertStyle(
			     pos,
			     self->screenStyle,
			     TRUE);
	(self->screenEnv)->SetLength((self)->GetFence()-pos);
	(self->screenEnv)->SetStyle(TRUE,TRUE);

	ht=self->height;

	/* make a screen */
	while(ht-->0){
	    wid=self->width;

	    while((c=(self)->GetChar(pos))!='\n' &&
		  c!=EOF &&
		  wid-->0)
		pos++;

	    if(wid>0)
		pos=fillChars(self,pos,wid);
	    else if(wid<0){
		long end=pos;
		while((c=(self)->GetChar(end))!='\n' && c!=EOF)
		    end++;
		(self)->AlwaysDeleteCharacters(pos,end-pos);
	    }

	    if(c==EOF)
		(self)->text::AlwaysInsertCharacters(pos,"\n",1);

	    pos++;
	}

	(self)->SetFence(scrPos(self,0,self->height));

	self->x=0;
	self->y=0;
    }

    return TRUE;
}  

void termulator::Untermulate()
{
    if(this->screen>=0){
	long pos,fence;

	(this)->SetFence(scrPos(this,0,this->height));

	/* strip off blanks on every line above us.... */
	pos=stripEnds(this,this->y)+this->x;

	/* now get rid of cruft */
	fence=(this)->GetFence();
	if(pos<fence)
	    (this)->AlwaysDeleteCharacters(pos,fence-pos);

	(this->screenEnv)->SetStyle(FALSE,FALSE);

	this->cursor=pos;
	this->screen= -1;
    }
}

void termulator::GotoXY(int  x,int  y)
{
    if(x<0 || y<0 || x>=this->width || y>=this->height)
	return;

    if(this->screen<0 &&
       ((y==0 && x==0 && this->cursor==this->lastSubmit) ||
	!termulate(this)))
	return;

    this->cursor=scrPos(this,x,y);
    this->x=x;
    this->y=y;
}

void termulator::Backspace(int  num)
{
    if(num>this->x)
	num=this->x;

    if(num>0){
	this->x-=num;
	this->cursor-=num;
    }
}

void termulator::CR()
{
    this->cursor-=this->x;
    this->x=0;
}

void termulator::Tab()
{
    int newx=(this->x+8)&~7;
    long pos=this->cursor,
        fence=(this)->GetFence();

    if(this->screen<0){
	int c=EOF;

	while(pos<fence && this->x<newx && (c=(this)->GetChar(pos))!='\n' && c!=EOF){
	    pos++;
	    this->x++;
	}

	if(this->x<newx)
	    (this)->text::AlwaysInsertCharacters(pos,"        ",newx-this->x);
    }else if(newx>=this->width)
	newx=this->width-1;

    pos+=newx-this->x;
    if(pos>fence)
	(this)->SetFence(pos);

    this->cursor=pos;
    this->x=newx;
}

void termulator::Newline(int  num)
{
    if(this->screen>=0){
	if((this->mode&(CRMOD|RAW))==CRMOD) /* \n -> \r\n */
	    this->x=0;
	this->y+=num;
	if(this->y>=this->height)
	    (this)->Scroll(this->height-this->y+1);
	else
	    this->cursor=scrPos(this,this->x,this->y);
    }else{
	long pos=this->cursor,
	    fence=(this)->GetFence();
	int c=EOF;

	do
	    while(pos<fence &&
		  (c=(this)->GetChar(pos))!='\n' && c!=EOF)
		pos++;
	while(c=='\n' && --num>0);

	while(num-->0)
	    (this)->text::AlwaysInsertCharacters(pos,"\n",1);

	this->cursor= ++pos;

	/* I suppose it should really space over to the right
	 * column in raw mode, but oh well...
	 */
	if(pos>fence)
	    (this)->SetFence(pos);

	this->x=0;
    }
}

/* this works either in normal or full-screen mode
  * chars shouldn't include any "special" characters, like newline or tab
 */
void termulator::WriteChars(char  *chars,int  len)
{
    long pos=this->cursor,
        fence=(this)->GetFence(),
        deleted;
    class environment *env = NULL;

    if(this->screen<0){
	if(!this->insert){
	    /* not guaranteed that len characters exist */
	    register long end;
	    register int c;

	    for(end=pos; end-pos<len && end<fence; end++){
		c=(this)->GetChar(end);
		if(c==EOF || c=='\n')
		    break;
	    }

	    deleted=end-pos;
	    if(deleted>0)
		(this)->AlwaysDeleteCharacters(pos,deleted);
	}else
	    deleted=0;
    }else{
	if(len+this->x>=this->width)
	    len=this->width-this->x;

	if(this->insert)
	    (this)->AlwaysDeleteCharacters(
					   pos+
					  (this->width-this->x)-
					      len,
					      len);
	else
	    (this)->AlwaysDeleteCharacters(pos,len);

	deleted=len;
    }
 
    /* Standout mode is NOT HANDLED CORRECTLY here.  It will not un-highlight
     * something written over the middle of a highlighted region.
     * Fortunately this doesn't happen too often.  To do it correctly
     * wouldn't be too hard.
     */
    if(this->standout){
	env=(class environment *)(this->rootEnvironment)->GetInnerMost(pos);
	if(env==this->screenEnv){
	    env=
	      (env)->InsertStyle(
				 pos-this->screen,
				 this->soStyle,
				 TRUE);
	    (env)->SetLength(0);
	}else if(env==this->rootEnvironment){
	    env=(env)->InsertStyle(pos,this->soStyle,TRUE);
	    (env)->SetLength(0);
	}
	(env)->SetStyle(TRUE,TRUE);
    }

    (this)->text::AlwaysInsertCharacters(pos,chars,len);

    if(this->standout)
	(env)->SetStyle(FALSE,FALSE);

    this->x+=len;
    this->cursor=pos+len;

    fence-=deleted;
    if(pos==fence)
	(this)->SetFence(fence+len);
}

void termulator__InsertChars(class termulator  *self,long  num)
{
    long pos=self->cursor;

    if(self->screen>=0){
	if(self->x+num>self->width)
	    num=self->width-self->x;

	(self)->AlwaysDeleteCharacters(pos+num,(self->width-self->x)-num);
    }

    fillChars(self,pos,num);
}

void termulator::DeleteChars(int  num)
{
    long pos=this->cursor,
        fence=(this)->GetFence();

    if(this->screen<0){
	/* not guaranteed that len characters exist */
	long end;
	int c;

	for(end=pos; end-pos<num && end<fence; end++){
	    c=(this)->GetChar(end);
	    if(c==EOF || c=='\n')
		break;
	}

	(this)->AlwaysDeleteCharacters(pos,end-pos);
    }else{
	if(this->x+num>this->width)
	    num=this->width-this->x;

	(this)->AlwaysDeleteCharacters(pos,num);
	fillChars(this,pos+(this->width-this->x)-num,num);
    }
}

void termulator::ClearChars(int  x,int  num)
{
    long pos=this->cursor,
        fence=(this)->GetFence();

    if(this->screen<0){
	/* not guaranteed that len characters exist */
	long end;
	int c;

	for(end=pos; end-pos<num && end<fence; end++){
	    c=(this)->GetChar(end);
	    if(c==EOF || c=='\n')
		break;
	}

	if(end>fence)
	    end=fence;

	(this)->AlwaysDeleteCharacters(pos,end-pos);
	if(end-pos==num)
	    fillChars(this,pos,num);
    }else{
	if(this->x+num>this->width)
	    num=this->width-this->x;

	(this)->AlwaysDeleteCharacters(pos,num);
	fillChars(this,pos,num);
    }
}

void termulator::InsertLines(int  num)
{
    long newpos;

    if(this->screen<0 && !termulate(this))
	return;

    newpos=scrPos(this,0,this->y);
    fillLines(this,newpos,this->width,num);
    (this)->AlwaysDeleteCharacters(
				       scrPos(this,0,this->height),
				       num*(this->width+1));
    this->x=0;
    this->cursor=newpos;
}

void termulator::DeleteLines(int  num)
{
    long newpos;

    if(this->screen<0 && !termulate(this))
	return;

    newpos=scrPos(this,0,this->y);
    fillLines(this,scrPos(this,0,this->height),this->width,num);
    (this)->AlwaysDeleteCharacters(
				      newpos,
				      num*(this->width+1));
    this->x=0;
    this->cursor=newpos;
}

void termulator::ClearLines(int  y,int  num)
{
    long newpos,numchars;

    if(this->screen<0 && !termulate(this))
	return;

    newpos=scrPos(this,0,y);
    numchars=num*(this->width+1);

    /* do the fillLines first, otherwise the screenEnv gets nuked */
    fillLines(this,newpos,this->width,num);
    (this)->AlwaysDeleteCharacters(newpos+numchars,numchars);

    this->cursor=scrPos(this,this->x,this->y);
}

void termulator::Scroll(int  num)
{
    if(num>=this->height){
	/* this in effect clears the screen */
	(this)->ClearLines(0,this->height);
	return;
    }	  

    fillLines(this,scrPos(this,0,this->height),this->width,num);

    /* let scrolled-off-top lines remain... */
    this->screen=stripEnds(this,num);
    
    this->x=0;
    this->y=this->height-1;
    this->cursor=scrPos(this,0,this->height-1);
}

/* ---------------------------------------------------------------- */

#define SYSERROR(str) \
(this->errstr=str,this->errno=::errno,(this)->NotifyObservers(0))
#define ERROR(str) \
   (self->errstr=str,self->errno=0,(self)->NotifyObservers(0))

static void readFromProc(FILE  *fp,class termulator  *self)
{
    int pty=fileno(fp);
    int len;
    struct sgttyb tty;
    char buf[4000];

    len=read(pty,buf,sizeof(buf));

    if(len==0){
	im::RemoveFileHandler(fp);
	fclose(fp);
	self->ptyFile=NULL;
	ERROR("EOF from child.");
	return;
    }else if(len==-1){
	if(::errno!=EINTR){
	    im::RemoveFileHandler(fp);
	    fclose(fp);
	    self->ptyFile=NULL;
	    ERROR("Lost connection with child.");
	}	    
	return;
    }

    ioctl(pty,TIOCGETP,&tty);
    self->mode=tty.sg_flags;

    (self)->ProcessOutput(buf,len);

    (self)->NotifyObservers(0);
}

static char *ignoreChar(class termulator  *self,char  *buf,long  len)
{
    return buf+1;
}

static char *cntlChar(class termulator  *self,char  *buf,long  len)
{
    static char rep[2]="^";

    rep[1]=(*buf+'@');
    (self)->WriteChars(rep,2);
    return buf+1;
}

/* used for some common occurances */
static char *specialChar(class termulator  *self,char  *buf,register long  len)
{
    int c= *buf,num=1;

    while(*++buf==c && --len>0)
	num++;

    switch(c){
	case '\r':
	    (self)->CR();
	    break;
	case '\b':
	    (self)->Backspace(num);
	    break;
	case '\n':
	    (self)->Newline(num);
	    break;
	case '\t':
	    while(num--)
		(self)->Tab();
	    break;
    }

    return buf;
}

/* read in directory string */
static char *readDir(class termulator  *self,char  *buf,register long  len)
{
    char *dir=self->parseBuf+self->parseLen;
    int left=sizeof(self->parseBuf)-self->parseLen;
    char *origbuf = buf;

    if(*buf=='\001')
	buf++;

    while(len-->0 && *buf!='\002' && left>0){
     	if(!isprint((unsigned char)*buf)) {
 	    /*
 	     * Unlikely character, assume he's not really calling
 	     * readDir.
 	     */
 	    (self)->WriteChars( origbuf, 1);
 	    return origbuf+1;
 	}
	*dir++= *buf++;
	self->parseLen++;
	left--;
    }

    if(len>0){
	char *home=environ::Get("HOME");

	if(self->cwd!=NULL)
	    free(self->cwd);

	*dir++='\0';
	filetype::CanonicalizeFilename(self->parseBuf,
				      self->parseBuf,
				      sizeof(self->parseBuf));
	if(home!=NULL){
	    long hlen=strlen(home);
	    if(strncmp(self->parseBuf,home,hlen)==0){
		char *p=self->parseBuf+hlen,*q=self->parseBuf+1;
		*self->parseBuf='~';
		do
		    *q++= *p++;
		while(*(p-1)!='\0');
	    }
	}

	self->cwd=(char *)malloc(strlen(self->parseBuf)+1);
	strcpy(self->cwd,self->parseBuf);

	self->parseLen=0;

	self->state=NULL;

	return buf+1;
    }else{
	self->state=readDir;
	return buf;
    }
}

void termulator::SetDispSize(int  width,int  height)
{
#ifdef TIOCSWINSZ
    int mw,mh;

    if(this->dispHeight==height && this->dispWidth==width)
	return;

    if(this->width<width)
	mw=width;
    else
	mw=this->width;
    if(this->height<height)
	mh=height;
    else
	mh=this->height;

    if(this->width<mw || this->height<mh){
	(this)->SetScreenSize(mw,mh);
	this->width=mw;
	this->height=mh;
    }

#ifdef M_UNIX
    {
	char buf[80];
	sprintf(buf,"LINES=%d",mh);
	putenv(buf);
	sprintf(buf,"COLS=%d",mw);
	putenv(buf);
    }
#else
    if(this->ptyFile!=NULL){
	struct winsize ws;
	ws.ws_row=height;
	ws.ws_col=width;
	if(ioctl(fileno(this->ptyFile),TIOCSWINSZ,&ws)<0)
	    SYSERROR("SetDispSize(ioctl)");
    }
#endif

    this->dispWidth=width;
    this->dispHeight=height;
#endif /* TIOCSWINSZ */
}

/* the state machine for interpretting output */
void termulator::ProcessOutput(char  *buf,register long  len)
{
    register termulator_escfptr *escapes=this->escapes;
    register char *end=buf;

    while(len>0){
	register termulator_statefptr action=this->state;

	if(action==NULL){
	    do{
		action=escapes[(unsigned char)*end];
		if(action!=NULL)
		    break;
		end++;
	    }while(--len>0);
	    
	    if(end>buf){
		(this)->WriteChars(buf,end-buf);
		buf=end;
	    }
	}

	if(len>0){
	    end=(*action)(this,buf,len);
	    len-=(end-buf);
	    buf=end;
	}
    }
}

static void setupStyles(class termulator  *self)
{
    (self)->SetGlobalStyle(
			      (self->styleSheet)->Find( "global"));
    self->cmdStyle=
      (self->styleSheet)->Find("command");
    self->noechoStyle=
      (self->styleSheet)->Find("noechocmd");
    self->soStyle=
      (self->styleSheet)->Find("standout");
    self->screenStyle=
      (self->styleSheet)->Find("screen");
}

void termulator::Clear()
{
    long pos=(this)->GetLength();

    if(this->screen>=0)
	pos=this->screen;
    else{
	while(pos>0 && (this)->GetChar(pos)!='\n')
	    pos--;
	pos++;
    }

    (this)->AlwaysDeleteCharacters(0,pos);

    this->cursor-=pos;
    this->lastSubmit-=pos;
    if(this->screen>0)
	this->screen-=pos;

    (this)->NotifyObservers(0);
}

termulator::termulator()
{
    struct attributes templateAttribute;
    int i;

#ifndef hp9000s300
    (void)signal(SIGTTOU,SIG_IGN);
#endif /* hp9000s300 */

    this->screen= -1;
    this->cursor=0;
    this->lastSubmit=0;
    this->mode=0;
    this->insert=FALSE;
    this->standout=FALSE;
    this->width=80;
    this->height=24;
#ifdef TIOCSWINSZ
    this->dispWidth=this->width;
    this->dispHeight=this->height;
#endif /* TIOCSWINSZ */
    this->inpView=NULL;
    this->errstr=NULL;
    this->x=this->y=0;
    this->parseLen=0;
#ifdef HACKEDNOECHO
    this->inpLen=0;
#endif /* HACKEDNOECHO */
    this->cwd=NULL;
    this->state=NULL;
    this->args=NULL;
    this->screenEnv=NULL;
    this->cmdEnv=NULL;

    this->ptyFile=NULL;
    this->pid=0;

    this->maxsize = environ::GetProfileInt("maxsize", 10000);
    this->extraroom = this->maxsize / 5;

    for(i=0;i<' ';i++)
	this->escapes[i]=cntlChar;
    for(i=' ';i<256;i++)
	this->escapes[i]=NULL;
    this->escapes['\t']=specialChar;
    this->escapes['\n']=specialChar;
    this->escapes['\b']=specialChar;
    this->escapes['\r']=specialChar;
    this->escapes['\001']=readDir;
    this->escapes['\000']=ignoreChar;
    this->escapes['\007']=ignoreChar;
    this->escapes['\177']=ignoreChar;

    this->lastCmd=NULL;
    (this)->EnterCmd(NULL); /* initialize command stack */

    (this)->SetExportEnvironments(FALSE);

    templateAttribute.key = "template";
    templateAttribute.value.string = "tm";
    templateAttribute.next = NULL;

    this->currentlyReadingTemplateKluge=TRUE;
    (this)->SetAttributes(&templateAttribute);
    this->currentlyReadingTemplateKluge=FALSE;

    setupStyles(this);

    THROWONFAILURE( TRUE);
}

termulator::~termulator()
{
    if(this->pid!=0){
	im::RemoveZombieHandler(this->pid);
	kill(this->pid,SIGHUP);
    }
    if(this->ptyFile!=NULL)
	im::RemoveFileHandler(this->ptyFile);
}

static void childDied(int  pid,class termulator  *self,WAIT_STATUS_TYPE  *status)
#ifdef hp9000s300
#else /* hp9000s300 */
#endif /* hp9000s300 */
{
    static char buf[40];

    strcpy(buf,"Child ");
    statustostr(status,buf+6,34);

    if(self->ptyFile!=NULL){
	im::RemoveFileHandler(self->ptyFile);
	fclose(self->ptyFile);
	self->ptyFile=NULL;
    }

    self->pid=0;
    self->mode=0;

    ERROR(buf);
}

static int ON=1;
/* Does ioctl take a pointer arg? */

#if defined(hpux) || defined(_IBMR2) || defined(NeXT) || defined(sys_pmax_ul4) || defined(sys_dec_alpha)

#define ENTER_TIOCREMOTE(self) \
(self->remote || \
  (ioctl(fileno(self->ptyFile),TIOCREMOTE,&ON) != -1 && \
  (/*fprintf(stderr,"Into TIOCREMOTE mode.\n"),*/self->remote++)))

#ifndef INCORRECTSIGNALS
static int OFF=0;
#define LEAVE_TIOCREMOTE(self) \
(!self->remote || \
 (ioctl(fileno(self->ptyFile),TIOCREMOTE, &OFF) != -1 && \
  (/*fprintf(stderr,"Out of TIOCREMOTE mode.\n"),*/self->remote--,TRUE)))
#endif /* INCORRECTSIGNALS */

#else /* !defined(hpux) && !defined(_IBMR2) && !defined(NeXT) && !defined(sys_pmax_ul4) */
/* integer arg3 to ioctl */

#if !defined(M_UNIX) && !defined(linux)
#define ENTER_TIOCREMOTE(self) \
(self->remote || \
 (ioctl(fileno(self->ptyFile),TIOCREMOTE,1) != -1 && \
  (/*fprintf(stderr,"Into TIOCREMOTE mode.\n"),*/self->remote++)))

#ifndef INCORRECTSIGNALS
static int OFF=0;
#define LEAVE_TIOCREMOTE(self) \
(!self->remote || \
 (ioctl(fileno(self->ptyFile),TIOCREMOTE, 0) != -1 && \
  (/*fprintf(stderr,"Out of TIOCREMOTE mode.\n"),*/self->remote--,TRUE)))
#endif /* INCORRECTSIGNALS */

#else /* M_UNIX */
#define ENTER_TIOCREMOTE(self)

#ifndef INCORRECTSIGNALS
static int OFF=0;
#define LEAVE_TIOCREMOTE(self)
#endif /* INCORRECTSIGNALS */
#endif /*M_UNIX */
#endif /* defined(hpux) || defined(_IBMR2) || defined(NeXT) || defined(sys_pmax_ul4) */

int termulator::StartProcess(char  **args)
{
    class termulator *self=this;
    int     pid/*,ppid=getpid()*/;
    int     pty, slave;
#if POSIX_ENV
    int pgrp = getpgrp();
#else
    int pgrp = getpgrp(0);
#endif
    struct sgttyb tty;
#ifdef TIOCSWINSZ
    struct winsize ws;
#endif /* TIOCSWINSZ */
    char ptyname[64];
 
    if(args==NULL)
	return -1;
/*
    {
	fprintf(stderr,"pid: %d, pgrp: %d\n",getpid(),getpgrp(0));
    }
*/
#if !(defined(POSIX_ENV))
    {
	int fd=open("/dev/tty", 2);
	if(fd>=0){
	    ioctl(fd,TIOCNOTTY,0);
	    close(fd);
	}else
	    setpgrp(0,getpid());
    }
#endif /* hpux */

    if(!GetPtyandName(&pty, &slave, ptyname, sizeof(ptyname))) {
	ERROR("Could not open any pty's");
	return -1;
    }

    this->ptyFile=fdopen(pty,"rw");

    if(this->ptyFile==NULL){
	ERROR("fdopen failed!?!");
	return -1;
    }

    this->remote=0;

    ENTER_TIOCREMOTE(this);
#ifndef SOLARIS
    ioctl(pty,FIOCLEX,0);
#endif

#ifdef TIOCSWINSZ
    ws.ws_row=this->dispHeight;
    ws.ws_col=this->dispWidth;
    ioctl(pty,TIOCSWINSZ,&ws);
#endif /* TIOCSWINSZ */

    if((pid = osi_vfork()) < 0){
	SYSERROR("startSubProc(vfork)");
	return ::errno;
    }

    if(pid == 0){
	int pid;
#ifdef POSIX_ENV
	int ptyChannel;

        /* Become a session leader. */
        if(setsid() < 0)
            perror("setsid");
        /* Re-open the pty so that it becomes the controlling terminal.
         * NOTE:  GetPtyandName must open the pty with the O_NOCTTY flag,
         * otherwise the parent (typescript) process will have the controlling
         * terminal and we (the shell) won't be able to get it.
         */
        if((ptyChannel = open(ptyname, O_RDWR)) < 0) {
             fprintf(stderr, "Could not open %s.\n", ptyname);
             exit(1);
        }
        close(ptyChannel);
#else
#ifdef hpux
	setpgrp();
#endif /* hpux */
#endif /* !POSIX_ENV */
	pid=getpid();
	close(pty);
	close(0);
	close(1);
	close(2); 
	dup2(slave, 0);
	close(slave);
#ifdef POSIX_ENV
	/* Set current terminal process group. */
	if(tcsetpgrp(0, pid) < 0)
	    perror("tcsetpgrp failed");
#else
#ifndef hpux
	ioctl(0,TIOCSPGRP,&pid);
	setpgrp(0,pid);
#endif /* hpux */
#endif /* POSIX_ENV */
	dup(0);
	dup(0);
	environ::Put("TERM",(this)->GetTerm());
	environ::Put("TERMCAP",(this)->GetTermcap());

	/* kernel doesn't reset pty's */
	ioctl(0,TIOCGETP,&tty);
	tty.sg_flags|=CRMOD+ECHO;
	tty.sg_flags&=~(CBREAK|RAW);

	ioctl(0,TIOCSETP,&tty);

	execvp(args[0],args);

	/* totally lost it; can't do much else... */
	_exit(-1);
    }

    this->pid=pid;

    im::AddZombieHandler(pid,(im_zombiefptr)childDied,(long)this);
    im::AddFileHandler(this->ptyFile,(im_filefptr)readFromProc,(char *)this,0);

    NEWPGRP();

    this->args=args;

    ioctl(pty,TIOCGETP,&tty);
    this->mode=tty.sg_flags;

    return 0;
}

char *termulator::ViewName()
{
    return "tmview";
}

static char ctrlbuf[2]="^";
#define echoCTRL(self,c) \
  (ctrlbuf[1]=(((c)+'@')&0x7f),(self)->WriteChars(ctrlbuf,2))

void termulator::ProcessInput(char  *buf,long  len)
{
    register char *end=buf;

#ifdef INCORRECTSIGNALS
    if(this->mode&RAW)
#else /* INCORRECTSIGNALS */
    if(this->mode&(RAW|CBREAK))
#endif /* INCORRECTSIGNALS */
	(this)->SendInput(buf,len);
    else{
	while(len-->0)
	    switch(*end++){
		case '\r':
#ifdef INCORRECTSIGNALS
		    if(this->mode&CBREAK)
			end[-1]='\n';
		    else
#endif /* INCORRECTSIGNALS */
			(this)->Submit();
		    break;
#ifdef INCORRECTSIGNALS
		case INTRCHAR:
		case QUITCHAR:
#ifndef hp9000s300
		case STOPCHAR:
#endif /* hp9000s300 */
		    end--;
		    if(end>buf)
			(this)->SendInput(buf,end-buf);

#ifdef HACKEDNOECHO
		    this->inpLen=0; /* flush buffer */
#endif /* HACKEDNOECHO */

		    switch(*end){
			case INTRCHAR:
			    (this)->Signal(SIGINT); break;
			case QUITCHAR:
			    (this)->Signal(SIGQUIT); break;
#ifndef hp9000s300
			case STOPCHAR:
			    (this)->Signal(SIGTSTP); break;
#endif /* hp9000s300 */
		    }

		    if(this->mode&ECHO)
		        echoCTRL(this,*end);

		    buf=end+1;

		    break;
#endif /* INCORRECTSIGNALS */
	    }

	if(end>buf)
	    (this)->SendInput(buf,end-buf);
    }
}

void termulator::SendInput(char  *buf,long  len)
{
    long newlen;

    if(len==0)
	return;

#ifndef INCORRECTSIGNALS
    LEAVE_TIOCREMOTE(this);
#endif /* INCORRECTSIGNALS */
    newlen=write(fileno(this->ptyFile),buf,len);
    if(newlen<len)
	SYSERROR("SendInput(write)");

#ifdef INCORRECTSIGNALS
    if(this->mode&ECHO){
	if(this->mode&RAW)
	    (this)->ProcessOutput(buf,newlen);
	else{
	    char *end=buf;
	    while(newlen-->0){
		if(!isprint((unsigned char)*end)){
		    if(end>buf)
			(this)->WriteChars(
					      buf,
					      end-buf);
		    switch(*end){
			case '\n':
			    (this)->Newline(1);
			    break;
			case '\t':
			    (this)->Tab();
			    break;
			default:
			    echoCTRL(this,*end);
		    }
		    buf=end+1;
		}
		end++;
	    }
	    if(end>buf)
		(this)->WriteChars(buf,end-buf);
	}
	(this)->NotifyObservers(0);
    }
#endif /* INCORRECTSIGNALS */
}

/* this should only get called if self->screen is NULL */
void termulator::Submit()
{
    int wrote;
    class termulator *self=this;
    if(this->ptyFile==NULL){
	ERROR("No child process.");
	return;
    }

#ifndef INCORRECTSIGNALS
    ENTER_TIOCREMOTE(this);
#endif /* INCORRECTSIGNALS */

#ifdef HACKEDNOECHO
    if(this->mode&ECHO)
#endif /* HACKEDNOECHO */
    {
	char buf[5000],*p,*save;
	long pos=(this)->GetFence(), fence=pos;
	int c;

	do{
	    long opos=pos;

	    p=buf;

	    while((c=(this)->GetChar(pos))!=EOF){
		pos++;
		if(c=='\n')
		    break;
		*p++=c;
	    }

	    *p++='\n';

	    wrote=write(fileno(this->ptyFile),buf,p-buf);

	    if(wrote==-1 && ::errno==EWOULDBLOCK){
		ERROR("Process not ready for input.");
		pos=opos;
		break;
	    }

	    if(wrote<p-buf){
		SYSERROR("Submit(write)");
		break;
	    }
	}while(c!=EOF);

	save=(char *)malloc(pos-fence+1);
	p=save;

	if(fence<pos){ /* some highlighted text exists */
#ifndef HACKEDNOECHO
	    if(!(this->mode&ECHO))
		(this)->AlwaysDeleteCharacters(fence,pos-fence);
	    else
#endif /* HACKEDNOECHO */
	    {
		while(fence<pos)
		    *p++=(this)->GetChar(fence++);
		*p='\0';
		if(!(this)->EnterCmd(save))
		    free(save); /* don't save after all */

		(this)->SetFence(pos);

		this->cursor=pos;

		(this->cmdEnv)->SetStyle(FALSE,FALSE); /* leave out \n */
	    }
	}

	(this)->Newline(1);

	this->lastSubmit=(this)->GetFence();
    }
#ifdef HACKEDNOECHO
    else{
	this->inpBuf[this->inpLen++]='\n';
	wrote=write(fileno(this->ptyFile),this->inpBuf,this->inpLen);
	if(wrote<this->inpLen){
	    char *p=this->inpBuf;

	    SYSERROR("Submit(write)");
	    while(p+wrote<this->inpBuf+this->inpLen){
		*p= *(p+wrote);
		p++;
	    }
	}
	this->inpLen-=wrote;
    }
#endif /* HACKEDNOECHO */

    if((this)->GetLength()>this->maxsize){
	long del = (this)->GetLength() - this->maxsize + this->extraroom;

	if(this->screen>=0 && this->screen<del)
	    del=this->screen;

	(this)->AlwaysDeleteCharacters(0,del);

	this->cursor-=del;
	this->lastSubmit-=del;
	if(this->screen>0)
	    this->screen-=del;
    }
}

static void preInsert(class termulator  *self,long  *posP ,long  *fenceP)
{
    *fenceP=(self)->GetFence();

    if(*posP>=*fenceP){
	/* Since cooked mode and termulator don't mix very well,
         * we keep switching back.  This could get sort of silly.
         */
	if(self->screen>=0){
	    long oldfence= *fenceP;
	    (self)->Untermulate();
	    *fenceP=(self)->GetFence();
	    *posP-=oldfence-*fenceP; /* most of screen probably went bye-bye */
	}

	if(*fenceP==(self)->GetLength()){
	    self->cmdEnv=
	      (self->rootEnvironment)->InsertStyle(
				 *fenceP,
				 (self->mode&ECHO
				  ?self->cmdStyle
				  :self->noechoStyle),
				 TRUE);
	    (self->cmdEnv)->SetLength(0);
	    (self->cmdEnv)->SetStyle(FALSE,TRUE);
	}

	if(*fenceP==*posP)
	    (self->cmdEnv)->SetStyle(TRUE,TRUE);
    }
}

static void postInsert(class termulator  *self,long  pos,long  fence)
{
    if((self)->GetLength()==fence)
	self->cmdEnv=NULL;
    else if(pos==fence)
	(self->cmdEnv)->SetStyle(FALSE,TRUE);
}

long termulator::Read(FILE  *fp,long  id)
{
    char buf[1000];
    static char *argbuf[500];
    char **args,dummyS[100];
    int dummyI;

    if(this->currentlyReadingTemplateKluge)
	return (this)->text::Read(fp,id);

    if(fgets(buf,sizeof(buf),fp)==NULL)
	return dataobject_PREMATUREEOF;

    if(*buf=='\0')
	args=NULL;
    else
	args=strtoargv(buf,argbuf,sizeof(argbuf));

    if(fscanf(fp,"\\enddata{%[^,],%d}\n",dummyS,&dummyI)!=2)
	return dataobject_MISSINGENDDATAMARKER;

/* A statement here to start up termulator with the read args 
has been removed , as it represented a major security hole.  tpn 2/28/90 */

    return dataobject_NOREADERROR;
}

long termulator::Write(FILE  *fp,long  writeID,int  level)
{
    if(writeID!=(this)->GetWriteID()){ /* only write a given version once */
	char buf[1000];

	(this)->SetWriteID(writeID);

	fprintf(fp,"\\begindata{%s,%d}\n",
		(this)->GetTypeName(), (this)->UniqueID());

	if(this->args==NULL)
	    *buf='\0';
	else
	    argvtostr(this->args,buf,sizeof(buf));

	fprintf(fp,"%s\n\\enddata{%s,%d}\n",
		buf,(this)->GetTypeName(), (this)->UniqueID());
    }

    return (this)->UniqueID();
}

/* this routine in effect puts the bold cooked-mode characters at the end of the document */
void termulator::AlwaysInsertCharacters(long  pos,char  *buf,long  len)
{
    long fence;

    preInsert(this,&pos,&fence);
    (this)->text::AlwaysInsertCharacters(pos,buf,len);
    postInsert(this,pos,fence);
}

/* need this for yanks */
long termulator::ReadSubString(long  pos,FILE  *file,boolean  quoteCharacters)
{
    long fence, rval;

    preInsert(this,&pos,&fence);
    rval=(this)->text::ReadSubString(pos,file,quoteCharacters);
    postInsert(this,pos,fence);

    return rval;
}

void termulator::EOT()
{
#ifndef INCORRECTSIGNALS
    ENTER_TIOCREMOTE(this);
#endif /* INCORRECTSIGNALS */
    if(write(fileno(this->ptyFile),this,0)<0)
	SYSERROR("EOT(write)");
}

void termulator::Signal(int  signo)
{
    int pgrp;

#ifdef POSIX_ENV
    /*
     * NOTE: we assume here that if you have a POSIX system, you also
     * have an "ioctl" that lets you send a signal to the process
     * group of the slave side of the pty.  That is true for SunOS 4.1[.x]
     * (TIOCSIGNAL), System V Release 4 (TIOCSIGNAL, although it takes
     * the value of the signal, rather than a pointer to that value,
     * as an argument), 4.3-reno and presumably 4.4BSD (TIOCSIG), and,
     * I think, later versions of HP-UX.
     */
#ifdef TIOCSIGNAL
#if defined(sys_sun3_41) || defined(sys_sun4_41)
    /* SunOS 4.1[.x] */
    if(ioctl(fileno(this->ptyFile),TIOCSIGNAL,&signo)<0)
	SYSERROR("Signal(ioctl)");
#else /* defined(sys_sun3_41) || defined(sys_sun4_41) */
    /* SVR4 */
    if(ioctl(fileno(this->ptyFile),TIOCSIGNAL,signo)<0)
	SYSERROR("Signal(ioctl)");
#endif /* defined(sys_sun3_41) || defined(sys_sun4_41) */
#else /* TIOCSIGNAL */
#ifdef TIOCSIG
    if(ioctl(fileno(this->ptyFile),TIOCSIG,&signo)<0)
	SYSERROR("Signal(ioctl)");
#endif /* TIOCSIG */
#endif /* TIOCSIGNAL */
#else /* POSIX_ENV */
#ifdef hpux
    if (0) {}
#else /* hpux */
    if(ioctl(fileno(this->ptyFile),TIOCGPGRP,&pgrp)<0)
	SYSERROR("Signal(ioctl)");
#endif /* hpux */
    else if(killpg(pgrp,signo)==-1)
	SYSERROR("Signal(kill)");
#endif /* POSIX_ENV */
}

char *termulator::GetTerm()
{
    return "wm";
}

char *termulator::GetTermcap()
{
    return "wm|termulator dumb terminal:bs";
}

char *termulator::GrabPrevCmd(char  *str)
{
    long slen=(str==NULL ? 0 : strlen(str));
    struct cmd *start=this->curCmd;

    while(this->curCmd!=NULL && this->curCmd->prev!=NULL){
	this->curCmd=this->curCmd->prev;
	if(str==NULL || strncmp(this->curCmd->string,str,slen)==0)
	    return this->curCmd->string;
    }

    this->curCmd=start;

    return NULL;
}

char *termulator::GrabNextCmd(char  *str)
{
    long slen=(str==NULL ? 0 : strlen(str));
    struct cmd *start=this->curCmd;

    while(this->curCmd!=NULL && this->curCmd->next!=NULL){
	this->curCmd=this->curCmd->next;
	if(str==NULL || strncmp(this->curCmd->string,str,slen)==0)
	    return this->curCmd->string;
    }

    this->curCmd=start;

    return NULL;
}

void termulator::RewindCmds()
{
    this->curCmd=this->lastCmd;
}

boolean termulator::EnterCmd(char  *str)
{
    struct cmd *lc=this->lastCmd;    

    if(str!=NULL &&
       (strlen(str)==1 ||
	(lc!=NULL && lc->prev!=NULL && strcmp(str,lc->prev->string)==0))){
	this->curCmd=lc;
	return FALSE;
    }

    this->lastCmd=(struct cmd *)malloc(sizeof(struct cmd));
    this->lastCmd->string="";
    this->lastCmd->next=NULL;
    this->lastCmd->prev=lc;

    if(lc!=NULL){
	lc->next=this->lastCmd;
	lc->string=str;
    }

    this->curCmd=this->lastCmd;

    return TRUE;
}
