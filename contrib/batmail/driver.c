/* ***************************************************************** *\
 *	Copyright 1986-1990 by Miles Bader
 *	Copyright Carnegie Mellon Univ. 1994 - All Rights Reserved
\* ***************************************************************** */
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

/*
 * robin -- Message System Primitive Interface
 * 25 feb 1986 - Miles Bader
 * Last edit by Miles Bader (bader) on Mon, 12 Sep 1988 - 11:44pm
 */

#include <andrewos.h>
#include <ctype.h>
/*  #include <stdio.h>
 *  #include <sys/time.h>
 *  #include <sys/signal.h>
 */
#include <cui.h>

#include "com.h"
#include "robin.h"
#include "death.h"
#include "version.h"

#undef fopen

char ProgramName[100]="robin";
char ProgramVersion[60];

#define FUNQSIZE 50

#define CHECKPOINTINTERVAL 200
#define INTERRUPTINTERVAL 30

#define INITBUFSZ 20
#define INCBUFSZ 20

#define MAXBLOCKNESTING 20


static void freeLines(/* struct lineNode *ln */);
static bool execBlock(/* struct cmdBlock *block */);
static bool doLine(/* struct lineNode *line */);



static struct qel{
    void (*fun)();
    long rock;
}   funq[FUNQSIZE],*qhead=funq,*qtail=funq;

void robin_QFunc(fun,rock)
void (*fun)();
long rock;
{
    struct qel *next=qhead+1;
    if(next>=&funq[FUNQSIZE])
	next=funq;
    if(next!=qtail){
	qhead->fun=fun;
	qhead->rock=rock;
	qhead=next;
    }
}
	
static void dqFuncs()
{
    while(qtail!=qhead){
	(*qtail->fun)(qtail->rock);
	qtail++;
	if(qtail>=&funq[FUNQSIZE])
	    qtail=funq;
    }
}

static long lastModTime=0,lastWriteTime=0,lastMsgCount=0;

void robin_Checkpoint()
{
    int oldSave=lastWriteTime;

    beginState("Checkpointing");

    lastWriteTime=lastModTime;
    if(MS_FastUpdateState()==0){
	endState();
    }else{
	failState();
	lastWriteTime=oldSave;
    }
}

static void newMail()
{
    int newmsgs;
    MS_DoIHaveMail(&newmsgs);
    if(newmsgs!=lastMsgCount)
	sendNewMailCount(newmsgs);
    lastMsgCount=newmsgs;
}

static void interruptHandler()
{
    robin_QFunc(newMail,0);

    if(lastModTime>lastWriteTime && time(0)-lastWriteTime>CHECKPOINTINTERVAL)
	robin_QFunc(robin_Checkpoint,0);

    signal(SIGALRM,interruptHandler);
    alarm(INTERRUPTINTERVAL);
}

static void startInterrupts()
{
    lastWriteTime=lastModTime=time(0);
    signal(SIGALRM,interruptHandler);
    interruptHandler();
}

static struct cmdBlock {
    struct lineNode *body, *fail, *unwind;
} *cbFreeList=NULL;

static struct cmdBlock *newBlock()
{
    struct cmdBlock *new=cbFreeList;

    if(new==NULL)
	new=NEW(struct cmdBlock);
    else
	cbFreeList=(struct cmdBlock *)new->body; /* body used for free list */

    new->body=NULL;
    new->fail=NULL;
    new->unwind=NULL;

    return new;
}

static void freeBlock(cb)
struct cmdBlock *cb;
{
    if(cb->body!=NULL)
	freeLines(cb->body);
    if(cb->fail!=NULL)
	freeLines(cb->fail);
    if(cb->unwind!=NULL)
	freeLines(cb->unwind);

    cb->body=(struct lineNode *)cbFreeList; /* body used for free list */
    cbFreeList=cb;
}

static struct lineNode {
    char *buf,*end;
    struct cmdBlock *block;
    bool ignoreErrs;
    int size;
    struct lineNode *next;
} *lnFreeList=NULL;

static struct lineNode *newLine()
{
    struct lineNode *new=lnFreeList;

    if(new==NULL){
	new=NEW(struct lineNode);

	new->size=INITBUFSZ;
	new->buf=(char *)malloc(new->size);
    }else
	lnFreeList=new->next;

    new->ignoreErrs=TRUE;
    new->next=NULL;
    new->block=NULL;
    new->end=new->buf;

    return new;
}

static void freeLine(ln)
struct lineNode *ln;
{
    if(ln->block!=NULL)
	freeBlock(ln->block);

    ln->next=lnFreeList;
    lnFreeList=ln;
}

static void freeLines(ln)
struct lineNode *ln;
{
    while(ln!=NULL){
	struct lineNode *next=ln->next;
	ln->next=lnFreeList;
	lnFreeList=ln;
	ln=next;
    }
}

static struct lineNode *readLine(fp)
FILE *fp;
{
    struct lineNode *ln;
    int c=getc(fp);

    if(c==EOF)
	return NULL;

    ln=newLine();

    while(c!=EOF && c!='\n'){
	if(c=='\\'){
	    c=getc(fp);

	    if(c=='\n'){		/* continuation line */
		c=getc(fp);
		continue;
	    }else{
		ungetc(c,fp);
		c='\\';
	    }
	}
	    
	if(ln->end-ln->buf>=ln->size-1){
	    char *new=(char *)realloc(ln->buf,ln->size+=INCBUFSZ);
	    ln->end=new+(ln->end-ln->buf);
	    ln->buf=new;
	}

	*ln->end++=c;

	c=getc(fp);
    }

    *ln->end='\0';

    return ln;
}

static bool errOccurred=FALSE;

void flagError()
{
    errOccurred=TRUE;
}

static bool execLine(line)
struct lineNode *line;
{
    int mod=FALSE;
    char *split(),*arg;

    if(line->block!=NULL)
	return execBlock(line->block);

    arg=line->buf;

    errOccurred=FALSE;

    switch(*arg++){
	    char *arg2,*arg3;		/* use as temp */

	case 'n':			/* check new mail of folder or all */
	    /*
	     * format: "c" x, where x=="a" for all or "f" for folder
	     */
	    robin_CheckNewMail(*arg=='a');
	    newMail();
	    break;
	case 'a':			/* append to file */
	    /*
	     * format: "a" cuid ":" filename
	     */
	    arg2=split(arg,':');
	    if(arg2==NULL)
		error("No ':' in a(ppend) command");
	    else{
		char file[MAXFILENAME];
		strcpy(file,arg2);
		if((char *)expandSquiggle(file,MAXFILENAME)==NULL)
		    error("Bad path to a(ppend): %s",file);
		else
		    robin_AppendToFile(atoi(arg),file);
	    }
	    break;
	case 'd':			/* set new folder or keep old */
	    /*
	     * format: "d" messageFolder
	     */
	    robin_SetFolder(arg);
	    break;
	case 'h':			/* captions since date or updates */
	    /*
	     * format: "h" date
	     */
	    robin_CaptionsSince(arg);
	    break;
	case 'H':			/* all captions since date or updates */
	    /*
	     * format: "H" date, where date=="" means updates
	     */
	    robin_ScanFolders(arg,FALSE);
	    break;
        case 'F':			/* Folders since a certain date */
	    /*
	     * format: "F" date, where date=="" means updates
	     */
	    robin_ScanFolders(arg,TRUE);
	    break;
        case 'L':			/* list folders under current folder */
	    /*
	     * format: "L"
	     */
	    robin_ListFolders(FALSE);
	    break;
        case 'l':			/* list children of current folder */
	    /*
	     * format: "l"
	     */
	    robin_ListFolders(TRUE);
	    break;
	case 'u':			/* set last-read to date of message */
	    /*
	     * format: "u" cuid
	     */
	    if(*arg=='\0')
		robin_UpdateLastRead(0);
	    else
		robin_UpdateLastRead(atoi(arg));
	    mod=TRUE;
	    break;
	case 'b':			/* send name of file with message */
	    /*
	     * format: "b" cuid ":" width ":" strip, where strip==s for strip, n for no
	     */
	    arg2=split(arg,':');
	    if(arg2==NULL)
		arg2="79";
	    arg3=split(arg2,':');
	    if(arg3==NULL)
		arg3="s";
	    robin_BodyFile(atoi(arg),atoi(arg2),*arg3=='s',*arg3=='m');
	    break;
	case 'g':			/* "good" headers to show */
	    /*
	     * format: [ "!"] header1 ":" header2 ...
	     */
	    if(*arg=='!')
		headfilter_Init(FALSE,arg+1);
	    else
		headfilter_Init(TRUE,arg);
	    break;
	case 'S':			/* change subscription status */
	    /*
	     * format: "s" method, where method is one of n,u,a,s,p
	     */
            {
		int subsMeth;

		switch(*arg){
		    case 'u':
			subsMeth=AMS_UNSUBSCRIBED; break;
		    case 'n':
			subsMeth=AMS_ALWAYSSUBSCRIBED; break;
		    case 'a':
			subsMeth=AMS_ASKSUBSCRIBED; break;
		    case 's':
			subsMeth=AMS_SHOWALLSUBSCRIBED; break;
		    case 'p':
			subsMeth=AMS_PRINTSUBSCRIBED; break;
		}

		robin_AlterSubscription(subsMeth);
		mod=TRUE;
	    }
	    break;
	case 'R':			/* reset state */
	    CUI_FreeCaches();
	    robin_StopFolderScan();
	    robin_SetFolder("");
	    break;
	case 'm':			/* generate outgoing mail template */
	    /*
	     * format: "m" meth cuid, meth one of m,r,w,a,f
	     */
	    {
		int code;
		switch(*arg){
		    case 'm':
		        code=AMS_REPLY_FRESH; break;
		    case 'r':
			code=AMS_REPLY_SENDER; break;
		    case 'w':
			code=AMS_REPLY_WIDE; break;
		    case 'a':
			code=AMS_REPLY_WIDER; break;
		    case 'f':
			code=AMS_REPLY_FORWARD; break;
		    case 'p':
			code=AMS_REPLY_POST; break;
		    case 'g':
			code=AMS_REPLY_GRIPE; break;
		}
		robin_MailFile(code,atoi(arg+1));
	    }
	    break;
        case 'k':
	    /*
	     * format: "k" field to startCuid ":" cuid, field one of [sf],
	     *   to one of [tn]
	     */
	    arg2=split(arg+2,':');
	    if(arg2==NULL)
		error("No ':' in k(ill) command");
	    else{
		int field;

		switch(*arg){
		    case 's':
			field=death_SUBJ; break;
		    case 'f':
			field=death_FROM; break;
		}

		robin_KillMsg(atoi(arg2),field,arg[1]=='t',atoi(arg+2));
	    }
	    break;
        case 'K':
	    /*
	     * format: "K" field to startCuid ":" meth string,
	     *   field one of [sf], to one of [tn], meth one of ["/]
	     */
	    arg2=split(arg+2,':');
	    if(arg2==NULL)
		error("No ':' in K(ill) command");
	    else{
		int meth,field;

		switch(arg2[0]){
		    case '"':
			meth=death_HASHED; break;
		    case '/':
			meth=death_REGEXP; break;
		}
		switch(*arg){
		    case 's':
			field=death_SUBJ; break;
		    case 'f':
			field=death_FROM; break;
		}
		
		robin_KillStr(arg2+1,field,meth,arg[1]=='t',atoi(arg+2));
	    }
	    break;
	case 'c':			/* clone message to current folder */
	    /*
	     * format: "c" cuid
	     */
	    {
		int code;

		switch(*arg){
		    case 'c':
		        code=MS_CLONE_COPYDEL; break;
		    case 'C':
			code=MS_CLONE_COPY; break;
		    case 'a':
			code=MS_CLONE_APPENDDEL; break;
		    case 'A':
			code=MS_CLONE_APPEND; break;
		    case 'S':
			code=MS_CLONE_SYMLINK; break;
		    case 'm':
			code=MS_CLONE_MOVE; break;
		}

		robin_CloneMessage(code,atoi(arg+1));
		mod=TRUE;
	    }
	    break;
	case 'r':			/* rewrite caption contents */
	    /*
	     * format: "r" headerName ":" contents
	     */
	    arg2=split(arg,':');
	    if(arg2==NULL)
		error("No ':' in r(ewrite) command");
	    else
		robin_RewriteHeader(arg,arg2);
	    break;
	case 's':			/* submit mail from file */
	    /*
	     * format: "s" x cuid ":" filename, where x==b for blind, n for noblind
	     */
	    arg2=split(arg+1,':');
	    if(arg2==NULL)
		error("No ':' in s(ubmit) command");
	    else
		robin_SubmitMail(arg2,*arg=='b',atoi(arg+1));
	    break;
	case 'z':			/* resend a message [!] */
	    /*
	     * format: "z" cuid ":" address
	     */
	    arg2=split(arg,':');
	    if(arg2==NULL)
		error("No ':' in z (resend) command");
	    else
		robin_Resend(atoi(arg),arg2);
	    break;
	case 'w':			/* set maximum caption width */
	    /*
	     * format: "w" width
	     */
	    robin_SetCaptionWidth(atoi(arg));
	    break;
	case 'M':			/* modify message attributes */
	    /*
	     * format: "M" how ignorable cuid
	     * how one of d u c a, uppercase->unset
	     * ignorable==i if can ignore read-only msg, n otherwise
	     */
	    {
		bool	on=islower(*arg);
		char	which=(on ? *arg : tolower(*arg));
		int	att;

		switch(which){
		    case 'd':
			att=AMS_ATT_DELETED; break;
		    case 'u':
			att=AMS_ATT_UNSEEN; break;
		    case 'c':
			att=AMS_ATT_CLOSED; break;
		    case 'a':
			att=AMS_ATT_REPLIEDTO; break;
		    default:
			error("Unknown msg attribute code: %c",which);
			att=0;
		}

		if(att!=0){
		    robin_SetAttribute(atoi(arg+2),att,on,arg[1]=='i');
		    mod=TRUE;
		}
	    }
	    break;
	case 'I':			/* include commands from a file */
	    /*
	     * format: "I" filename
	     */
	    {
		FILE	*fp=fopen(arg,"r");

		if(fp!=NULL){
		    struct lineNode *line;
		    while((line=readLine(fp))!=NULL && !errOccurred)
			errOccurred=doLine(line);
		    fclose(fp);
		}else
		    error("I can't open %s",arg);
	    }
	    break;
	case 'D':			/* unlink a file */
	    /*
	     * format: "D" filename
	     */
	    if(unlink(arg)!=0)
		error("I can't unlink %s",arg);
	    break;
	case 'P':			/* print body with given cuid */
	    CUI_PrintBodyFromCUID(atoi(arg));
	    break;
	case 'p':			/* purge all folders or current folder */
	    /*
	     * format: "p" all, where all=="a" for all, "d" for current folder
	     */
	    robin_PurgeFolders(*arg=='a');
	    mod=TRUE;
	    break;
	case 'i':			/* user input, to satisfy question */
	    /*
	     * format: "i" answer
	     */
	    inputAnswer(arg);
	    break;
	case 'C':			/* explicit call back... */
	    /*
	     * format: "C" callback-command
	     */
	    sendCallback(arg);
	    break;
	case 'U':			/* update message server state */
	    /*
	     * format: "U"
	     */
	    lastWriteTime=time(0);
	    robin_Update();
	    break;
	case 'v':			/* vote */
	    /*
	     * format: "v" cuid
	     */
	    CUI_HandleVote(atoi(arg),NULL);
	    break;
	case '\0':		/* null command */
	    break;
	default:
	    error("I don't understand %s",arg);
    }

    if(mod)
	lastModTime=time(0);

    return line->ignoreErrs ? FALSE : errOccurred;
}

/* returns TRUE if an error occurred */
static bool execBlock(block)
struct cmdBlock *block;
{
    struct lineNode *ln;
    bool err=FALSE, uerr=FALSE;

    for(ln=block->body; ln!=NULL && !err; ln=ln->next)
	err=execLine(ln);
    
    if(err)
	for(ln=block->fail; ln!=NULL; ln=ln->next)
	    if(execLine(ln))
		break;

    for(ln=block->unwind; ln!=NULL && !uerr; ln=ln->next)
	uerr=execLine(ln);

    return err || uerr;
}

static struct lineNode **endP(startP)
struct lineNode **startP;
{
    while(*startP!=NULL)
	startP=(&(*startP)->next);
    return startP;
}

static bool doLine(line)
struct lineNode *line;
{
    struct lineNode *cl;
    static struct {
	struct cmdBlock *block;
	struct lineNode **lineP;
	bool ignoreErrs;
    } stack[MAXBLOCKNESTING], *cur=stack-1;
    static bool ignoreErrs=TRUE;
    bool err=FALSE, freeline=TRUE;

#define ADDLINE(l) \
  (cl=(l), *cur->lineP=cl, cur->lineP=(&(*cur->lineP)->next), cl)

    /*
     * process meta commands
     */
    switch(*line->buf){
	    struct cmdBlock *cb;

	case '[':		/* start block */
	    cb=newBlock();

	    if(cur>=stack)
		ADDLINE(newLine())->block=cb;

	    cur->ignoreErrs=ignoreErrs;	/* save value */
	    ignoreErrs=FALSE;

	    cur++;
	    cur->block=cb;
	    cur->lineP=endP(&cb->body);

	    break;
	case ']':		/* end block */
	    if(cur>=stack){
		cb=cur->block;

		if(--cur<stack){
		    ignoreErrs=TRUE;
		    err=execBlock(cb);
		    freeBlock(cb);
		}else
		    ignoreErrs=cur->ignoreErrs;
	    }
	    break;
	case '^':		/* unwind-protect */
	    if(cur>=stack)
		cur->lineP=endP(&cur->block->unwind);
	    break;
	case '-':		/* turn off error handling */
	    ignoreErrs=TRUE;
	    break;
	case '+':		/* turn on error handling */
	    ignoreErrs=FALSE;
	    break;
	case '|':		/* block error handler */
	    if(cur>=stack)
		cur->lineP=endP(&cur->block->fail);
	    break;
	default:
	    if(cur>=stack){
		line->ignoreErrs=ignoreErrs;
		ADDLINE(line);
		freeline=FALSE;
	    }else
		err=execLine(line);
    }

    if(freeline)
	freeLine(line);

    return err;
}

robin_PollDriver(block)
bool	block;
{
    FILE    *fpvec[1];
    static struct timeval   zero={0,0};

    fpvec[0]=stdin;
    while(fselect(1,fpvec,NULL,NULL,(block?(struct timeval *)NULL:&zero))>0){
	struct lineNode *ln=readLine(stdin);

	if(ln==NULL)
	    return FALSE;

	doLine(ln);		/* doLines frees it */

        fpvec[0]=stdin;
        block=FALSE;		/* only block the first time */
    }

    dqFuncs();

    return TRUE;
}

SIGNAL_RETURN_TYPE cleanup()
{
    CUI_EndConversation();

    sendCleanup();

    exit(0);
}

main(argc,argv)
int	argc;
char	**argv;
{
    char idString[60];

    sprintf(idString,"BatMail/robin v%d.%d",
	    BATMAIL_VERSION_MAJOR,BATMAIL_VERSION_MINOR);
    sprintf(ProgramVersion,"((prog batmail %d %d))",
	    BATMAIL_VERSION_MAJOR,BATMAIL_VERSION_MINOR);

    robin_Init();
    CUI_Initialize(startInterrupts,NULL);
    CUI_SetClientVersion(idString);

    signal(SIGINT,  cleanup);
    signal(SIGQUIT, cleanup);
    signal(SIGHUP,  cleanup);
    signal(SIGTERM, cleanup);

    if (getuid()==2971) {	/* Maintainer (jm36@andrew.cmu.edu) gets real core dumps */
	signal(SIGBUS, SIG_DFL);
	signal(SIGSEGV, SIG_DFL);
    }

    dqFuncs();
    while(robin_PollDriver(TRUE))
	;

    cleanup();
}
