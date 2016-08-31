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
ATK_IMPL("ezdiff.H")


 /* sys/file.h */

#if POSIX_ENV || defined(hpux)
#include <sys/signal.h>
#endif /* POSIX_ENV || hpux*/
#include <buffer.H>
#include <mark.H>
#include <textview.H>
#include <text.H>
#include <message.H>
#include <proctable.H>
#include <environ.H>
#include <im.H>
#include <cursor.H>
#include <ezdiff.H>

#define ObjectOf(V) (((class view *)(V))->dataobject)
#define USECURRENTMARK -32000l
static class ezdiff *firstlink , *lastlink;
static char buffername[1024];
static class cursor *WaitCursor;

struct diffinfo {
    class mark *m;
    char buf[128];
};

#define buffertext(BUF) (class text *)(BUF)->GetData()

ATKdefineRegistry(ezdiff, observable, ezdiff::InitializeClass);
static class ezdiff *FindBufferDiff(class buffer  *b,int  *which);
static class ezdiff *FindViewDiff(class textview  *v,int  *which);
static char *getpair(char  *cp,int  *ip);
static class mark *setmark (int  *ip,class text  *d,int  *pi  ,int  *ppos);
static int ezdiff_setupmarkers(class ezdiff  *self,char  *s);
static int LocateInView(class view  *v1,class view  *v2,class view  *v3,long  dat);
static void ezdiff_PointOut(class ezdiff  *self,class textview  *v,long  delta);
static void ezdiff_Current(class textview  *v,long  delta);
static void ezdiff_Next(class textview  *v,long  delta);
static void ezdiff_Change(class textview  *v,long  delta);
static void ezdiff_Last(class textview  *v,long  delta);
static void ezdiff_Start(class view  *v,long  dat);


static class ezdiff *FindBufferDiff(class buffer  *b,int  *which)
{
    register class ezdiff *ep;
    if(b == NULL) return NULL;
    for(ep = firstlink; ep != NULL ; ep = ep->next){
	if(ep->buf[0] == b){
	    if(which) *which = 0;
	    return ep;
	}
	else if( ep->buf[1] == b) {
	    if(which) *which = 1;
	    return ep;
	}
    }
    return NULL;
}
static class ezdiff *FindViewDiff(class textview  *v,int  *which)
{
    return FindBufferDiff(buffer::FindBufferByData(ObjectOf(v)),which);
}
static char *getpair(char  *cp,int  *ip)
{
    if(*cp == '\0') return(NULL);
    while  (*cp == ' ' || *cp ==  ',' || *cp == 'a' || *cp == 'd' || *cp == 'c' ){
	cp++;
	if(*cp == '\0') return(NULL);
    }
    *ip = atoi(cp);
    if(*ip < 1 && *cp != '0') return(NULL);
    while(*cp >= '0' &&  *cp <= '9') cp++;
    while  (*cp == ' ' || *cp ==  ',' ){
	cp++;
    }
    ip++;
    *ip = atoi(cp);
    if(*ip == 0) *ip = *(ip - 1);
    else 	while(*cp >= '0' &&  *cp <= '9') cp++;
    return(cp);
}
#define BUFSIZE 4096
static class mark *setmark (int  *ip,class text  *d,int  *pi  ,int  *ppos)
{
    int spos, len;
    register int i, pos;
    len = (d)->GetLength();
    pos = *ppos;
    i= *pi;
    while (i<*ip)
    {
	if (pos >= len) break;
	if ((d)->GetChar(pos) == '\012') i=i+1;
	pos = pos+1;
    }
    spos = pos;ip++;
    (*ip)++;
    while (i<*ip )
    {
	if (pos >= len) break;
	if ((d)->GetChar(pos) == '\012') i=i+1;
	pos = pos+1;
    }
    *pi = i;
    *ppos = pos;
    return((d)->CreateMark(spos,pos - spos));
}

/* Return -1 if something here fails.  Return 0 for success. */
static int ezdiff_setupmarkers(class ezdiff  *self,char  *s)
{
    class text *d1,*d2;
    FILE *file;
    char buf[BUFSIZE],*cp,dc;
    int ip[2],lastmark;
    int i1,i2,pos1,pos2,count = 0;
    d1 = buffertext(self->buf[0]);
    d2 = buffertext(self->buf[1]);
    i1 = i2 = 1;
    pos1 = pos2 = 0;
    lastmark = 0;
    self->cmark = 0;

    if((file = fopen(s,"r")) == NULL) return -1;

    while(fgets(buf,BUFSIZE,file) != NULL){
	/* count the number of needed marks */
	switch(*buf){
	    case '<':
	    case '>':
	    case '-':
		break;
	    default :
		count++;
	}
    }
    count++;count++;
    self->m1 = (class mark **) malloc(sizeof(class mark *) * count);
    self->m2 = (class mark **) malloc(sizeof(class mark *) * count);
    rewind(file);
    while(fgets(buf,BUFSIZE,file) != NULL){
	switch(*buf){
	    case '<':
	    case '>':
	    case '-':
		break;
	    default :
		if((cp = getpair(buf,ip)) == NULL) break;
		if((dc = *cp) == 'a'){ *(ip + 1) = 0;(*ip)++; }
		self->m1[lastmark] = setmark(ip,d1,&i1,&pos1);
		getpair(cp,ip);
		if(dc == 'd')  { *(ip + 1) = 0;(*ip)++; }
		self->m2[lastmark] = setmark(ip,d2,&i2,&pos2);
		if(++lastmark >= count) return -1;
	}
    }
    self->nummarks = lastmark;
    fclose(file);
    return 0;
}

class ezdiff *ezdiff::Create(class buffer  *buf1,class buffer  *buf2,char  *diffopts)
{
	ATKinit;

    class ezdiff *self;
    char name1[256],name2[256],fnm[512];
    const char *argv[6],**av;
    int pid, exitstatus;
#if POSIX_ENV || defined(hpux)
    int status;
#else /* hpux */
    union wait status;
#endif /* hpux */
    SIGNAL_RETURN_TYPE (*oldhandler)(int);
    class text *dobj1=(class text *)(buf1)->GetData(), *dobj2=(class text *)(buf2)->GetData();
    enum textwritestyle tws1=(dobj1)->GetWriteStyle(), tws2=(dobj2)->GetWriteStyle(); /*RSK*/

    sprintf(name1,"/tmp/%s-1",(buf1)->GetName());
    sprintf(name2,"/tmp/%s-2",(buf2)->GetName());
    sprintf(fnm,"/tmp/wmdiff.%s.%s", (buf1)->GetName(),(buf2)->GetName());
    (dobj1)->SetWriteStyle(text_NoDataStream);
    (dobj2)->SetWriteStyle(text_NoDataStream);
    (buf1)->WriteToFile(name1,0);
    (buf2)->WriteToFile(name2,0);
    (dobj1)->SetWriteStyle(tws1);
    (dobj2)->SetWriteStyle(tws2);
    /* should check that filetype == NULL */
    av = argv;
    *av++ = "diff";
    if(diffopts && *diffopts!='\0') {
	char *space= index(diffopts,' ');
	while (space) {
	    *space= '\0';
	    *av++= diffopts;
	    diffopts= space+1;
	    space= index(diffopts,' ');
	}
	*av++ = diffopts;
    }
    *av++ = name1;
    *av++ = name2;
    *av = NULL;
    self = NULL;
    if(WaitCursor) im::SetProcessCursor(WaitCursor);
#if POSIX_ENV || defined(hpux)
    oldhandler = signal(SIGCHLD, SIG_DFL);
#endif /* POSIX || hpux */
    if((pid = osi_vfork()) == 0) {
	close(1);
	if((open(fnm,O_RDWR | O_CREAT | O_TRUNC,0600)) == -1){
	    _exit(3);
	}
	execvp("diff",(char **)argv);
	_exit(4);
    }
#if POSIX_ENV || defined(hpux)
    waitpid(pid, &status, 0);
#else /* POSIX_ENV || defined(hpux) */
    wait(&status);
#endif /* POSIX_ENV || defined(hpux) */

#ifdef hpux
    exitstatus = (((int)status >>8)&0377);
#else /* hpux */
#if POSIX_ENV
    /* XXX This may also work for hpux. */
    exitstatus = (WEXITSTATUS(status));
#else
    exitstatus = (status.w_T.w_Retcode);
#endif /* POSIX_ENV */
#endif /* hpux */

#if POSIX_ENV || defined(hpux)
    signal(SIGCHLD, oldhandler);
#endif /* POSIX || hpux */

    switch (exitstatus) {
	case 0: /* No Differences */
	    message::DisplayString(NULL,0,"Files are identical");
	    break;
	case 2:
	    message::DisplayString(NULL,0,"Diff Failed");
	    break;
	case 3:
	    message::DisplayString(NULL,0,"Can't open output file in /tmp");
	    break;
	case 4:
	    message::DisplayString(NULL,0,"Fork of diff failed");
	    break;
	case 1:	/* Success */
	    if((self = FindBufferDiff(buf1,NULL))!= NULL)
		(self)->Destroy();
	    if((self = FindBufferDiff(buf2,NULL))!= NULL)
		(self)->Destroy();
	    self = new ezdiff;
	    self->buf[0] = buf1;
	    self->buf[1] = buf2;
	    self->bname[0] = (char *)malloc(strlen((buf1)->GetName()) + 1);
	    strcpy(self->bname[0],(buf1)->GetName());
	    self->bname[1] = (char *)malloc(strlen((buf2)->GetName()) + 1);
	    strcpy(self->bname[1],(buf2)->GetName());
	    self->numbufs = 2;
	    if (ezdiff_setupmarkers(self,fnm) == -1)
	    {
		message::DisplayString(NULL,0,"Can't make sense of diff.");
		self->buf[0] = NULL;
		self->buf[1] = NULL;
		(self)->Destroy();
		self = NULL;
		break;
	    }
	    (self->buf[0])->AddObserver(self);
	    (self->buf[1])->AddObserver(self);
	    break;
	default:
	    message::DisplayString(NULL,0,"Unknown return status from diff");
	    break;
    }
    im::SetProcessCursor(NULL);
    unlink(name1); unlink(name2); unlink(fnm);
    return self;
}
ezdiff::ezdiff()
{
	ATKinit;

    if(lastlink != NULL) lastlink->next = this;
    this->next = NULL;
    lastlink = this;
    if(firstlink == NULL) firstlink = this;
    this->m1 = this->m2 = NULL;
    this->numbufs = 0;
    this->cmark = 0;
    this->buf[0] = this->buf[1] = NULL;
    this->bname[0] = this->bname[1] = NULL;
    this->asking = FALSE;
    THROWONFAILURE( TRUE);
}
void ezdiff::ObservedChanged(class observable  *changed, long  value)
{
    /* If the buffer changes, Diff is probably wrong. So.. */
    if(value == observable_OBJECTDESTROYED){
	if(changed == (class observable *) this->buf[0] )
	    this->buf[0] = NULL;
	else if(changed == (class observable *) this->buf[1] )
	    this->buf[1] = NULL;
    }
    else {
	if(this->bname[0] && this->bname[1] && 
	   this->buf[0] && this->buf[1] &&
	   strcmp((this->buf[0])->GetName(), this->bname[0]) == 0 &&
	   strcmp((this->buf[1])->GetName(), this->bname[1]) == 0)
	    /* probably an unimportant change to the buffer, ignore */
	    return;
    }
    message::DisplayString(NULL,0,"Diff buffer being modified. Destroying diff");
    (this)->Destroy();
}
ezdiff::~ezdiff()
{
	ATKinit;

    if(this->buf[0] != NULL) (this->buf[0])->RemoveObserver(this);
    if(this->buf[1] != NULL) (this->buf[1])->RemoveObserver(this);

    if(this == firstlink) {
	firstlink = this->next;
	if(this == lastlink) lastlink = NULL;
    }
    else {
	class ezdiff *ep;
	for(ep = firstlink; ep->next != NULL; ep = ep->next)
	    if(ep->next == this) break;
	ep->next = this->next;
	if(this == lastlink) lastlink = ep;
    }
    if(this->m1 != NULL) free((char *)this->m1);
    if(this->m2 != NULL) free((char *)this->m2);
    if(this->bname[0]) free(this->bname[0]);
    if(this->bname[1]) free(this->bname[1]);

}
static int LocateInView(class view  *v1,class view  *v2,class view  *v3,long  dat)
{
    struct diffinfo *d = (struct diffinfo *) dat;
    class mark *m = d->m;
    if(ATK::IsTypeByName((v2)->GetTypeName(),"textview")){
	class textview *tv = (class textview *)v2;
	(tv)->SetDotPosition((m)->GetPos());
	(tv)->SetDotLength((m)->GetLength());
	(tv)->FrameDot((m)->GetPos());
	if (!tv->Visible(m->GetPos() + m->GetLength())) {
	    /* End of the block is not visible.  Position the block near the top of the view. */
	    long toppos;
	    text *txt= (text *)tv->GetDataObject();

	    toppos= txt->GetBeginningOfLine(m->GetPos());
	    if (toppos > 1)
		toppos= txt->GetBeginningOfLine(toppos-1); /* back up a line */
	    tv->SetTopPosition(toppos);
	}
	message::DisplayString(tv,0,d->buf);
    }
    return 0; /* go through all views */
}
static void ezdiff_PointOut(class ezdiff  *self,class textview  *v,long  delta)
{
    static struct diffinfo d1;
    if(self == NULL){
	if((self = FindViewDiff(v,NULL)) == NULL){
	    message::DisplayString(v,0,"No diff currently associated with this buffer");
	    return ;
	}
    }
    if(delta < 0 && delta != USECURRENTMARK){
	message::DisplayString(v,0,"No previous file differences");
	return ;
    }
    if(delta >= self->nummarks){
	message::DisplayString(v,0,"No subsequent file differences");
	return ;
    }
    if(delta != USECURRENTMARK) self->cmark = delta;
    d1.m = self->m1[self->cmark];
    sprintf(d1.buf,"diff %d of %d",self->cmark + 1,self->nummarks);
    (self->buf[0])->EnumerateViews((buffer_evfptr) LocateInView, (long) &d1);
    d1.m = self->m2[self->cmark];
    (self->buf[1])->EnumerateViews((buffer_evfptr) LocateInView, (long) &d1);
}
static void ezdiff_Current(class textview  *v,long  delta)
{
    ezdiff_PointOut(NULL,v,USECURRENTMARK);
}
static void ezdiff_Next(class textview  *v,long  delta)
{
    int which,cmark,pos;
    class ezdiff *self;
    class mark **m;
    if((self = FindViewDiff(v,&which)) == NULL){
	message::DisplayString(v,0,"No diff associated with this file");
	return ;
    }
    pos = (v)->GetDotPosition();
    if(which == 0) m = self->m1;
    else m = self->m2;
    for(cmark = 0; cmark < self->nummarks; cmark++){
	if((m[cmark])->GetPos() > pos) break;
    }
    ezdiff_PointOut(self,v,cmark);
}
static void ezdiff_Change(class textview  *v,long  delta)
{
    int which,cmark;
    long szz;
    class ezdiff *self;
    class mark *srcm,*dstm;
    class text *dsttext,*srctext;
    char buf[32000];
    if((self = FindViewDiff(v,&which)) == NULL){
	message::DisplayString(v,0,"No diff associated with this file");
	return ;
    }
    cmark = self->cmark;
    if(which == 1) {
	srcm = self->m1[cmark];
	dstm = self->m2[cmark];
    }
    else{
	srcm = self->m2[cmark];
	dstm = self->m1[cmark];
    }
    srctext = (class text *)  (srcm)->GetObject();
    dsttext = (class text *)  (dstm)->GetObject();
    if (dsttext->GetReadOnly()) {
	message::DisplayString(v,80, "Document is read only.");
	return;
    }
    if((szz = (srcm)->GetLength()) >= 32000) {
	message::DisplayString(v,100,"Mark too big to copy, Use Cut and Paste");
	return;
    }
    if(szz > 0){
	(srctext)->CopySubString((srcm)->GetPos(),szz,buf,FALSE);
    }
    (dsttext)->AlwaysReplaceCharacters((dstm)->GetPos() ,(dstm)->GetLength(),buf,szz);
    (dsttext)->NotifyObservers(1);
    ezdiff_PointOut(NULL,v,USECURRENTMARK);
}
static void ezdiff_Last(class textview  *v,long  delta)
{
    int which,cmark,pos;
    class ezdiff *self;
    class mark **m;
    if((self = FindViewDiff(v,&which)) == NULL){
	message::DisplayString(v,0,"No diff associated with this file");
	return ;
    }
    pos = ((class textview *)v)->GetDotPosition();
    if(which == 0) m = self->m1;
    else m = self->m2;
    for(cmark = 0; cmark < self->nummarks; cmark++){
	if((m[cmark])->GetPos() >= pos ) break;
    }
    cmark--;
    ezdiff_PointOut(self,v,cmark);
}

static void ezdiff_Start(class view  *v,long  dat)
{
    class ezdiff *self;
    char ans[5], opts[256];
    class buffer *NotedBuf;
    class buffer *bb;
    boolean promptforopts=environ::GetProfileSwitch("EzdiffOPtionPrompt", TRUE);
    *opts= '\0';
    /* check for argument passed to proc, to use instead of prompting */
    if (dat && (unsigned long)dat>=256) { /* hoky "is string" test */
	strcpy(opts, (char *)dat);
	if (*opts) /* not an empty string */
	    promptforopts= FALSE;
    }
    /* check for Ctrl-U prefix, to force prompt */
    if((v)->GetIM() && (v)->GetIM()->ArgProvided()) promptforopts= ((v)->GetIM())->ArgProvided();
    ((v)->GetIM())->ClearArg();
    if((bb =  buffer::FindBufferByData(ObjectOf(v))) == NULL) return;
    if(!ATK::IsTypeByName((v)->GetTypeName(),"textview")){
	message::DisplayString(v, 0, "Diff works on text files only");
	return;
    }
    if((self = FindBufferDiff(bb,NULL)) != NULL){
	if (self->asking) {
	    message::DisplayString(v, 0, "You must answer the \"Reinitialize?\" question in the other window.");
	    return;
	}
	self->asking = TRUE;
	if(message::AskForString(v,0, "Diff Exists. Reinitialize?[n]",0,ans,sizeof(ans)) < 0 || (*ans != 'y' && *ans != 'Y') ) {
	    message::DisplayString(v, 0, "Punt!");
	    self->asking= FALSE;
	    return;
	}
	(self)->Destroy();
    }
    if(*buffername) NotedBuf = buffer::FindBufferByName(buffername);
    else NotedBuf = NULL;
    if(NotedBuf == NULL){
	/* we save the buffer name instead of a pointer to the buffer itself.
	 This protects us in the case where a selected buffer is destroyed */
	strncpy(buffername,(bb)->GetName(),255);
	message::DisplayString(v, 0, "Buffer Noted. Start in second buffer to Initialize");
    }
    else{
	if(bb == NotedBuf){
	    message::DisplayString(v, 0, "Can't Diff a buffer with itself.");
	    *buffername = '\0';
	    return;
	}
	if (*opts=='\0' && environ::ProfileEntryExists("EzdiffOptions", TRUE))
	    strncpy(opts, environ::GetProfile("EzdiffOptions"), sizeof(opts));
	if (promptforopts) {
	    message::DisplayString(v,0, "-b (ignore blanks)  -w (ignore all spaces&tabs)  -i (ignore case)  See 'diff' help for others.");
	    if (message::AskForString(v,60, "Diff command options: ",opts, opts,sizeof(opts)) < 0) {
		message::DisplayString(v, 0, "Cancelled.");
		return;
	    }
	}
	message::DisplayString(v, 0, "Running Diff. Please Wait...");
	while(im::Interact(0));
	if(ezdiff::Create(NotedBuf,bb,opts)!= NULL){
	    message::DisplayString(v, 0, "Diff Completed");
	    ezdiff_PointOut(NULL,(class textview *)v,USECURRENTMARK);
	}
	*buffername = '\0';
    }
}
boolean ezdiff::InitializeClass()
{
    struct ATKregistryEntry  *textviewtype = ATK::LoadClass("textview");

    firstlink = lastlink = NULL;
    *buffername = '\0';
    proctable::DefineProc("ezdiff-start",(proctable_fptr)ezdiff_Start,textviewtype,NULL,"Start diff process");
    proctable::DefineProc("ezdiff-current",(proctable_fptr)ezdiff_Current,textviewtype,NULL,"Get Current Diff");
    proctable::DefineProc("ezdiff-next",(proctable_fptr)ezdiff_Next,textviewtype,NULL,"Get Next Diff");
    proctable::DefineProc("ezdiff-last",(proctable_fptr)ezdiff_Last,textviewtype,NULL,"Get Last Diff");
    proctable::DefineProc("ezdiff-change",(proctable_fptr)ezdiff_Change,textviewtype,NULL,"Change to other mark");
    WaitCursor = cursor::Create(NULL);
    if(WaitCursor) (WaitCursor)->SetStandard(Cursor_Wait);
    return TRUE;
}


