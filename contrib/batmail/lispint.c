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
#ifndef NORCSID
#define NORCSID
static char rcsid[]="$Header: /afs/cs.cmu.edu/project/atk-src-C++/contrib/batmail/RCS/lispint.c,v 1.4 1995/06/28 17:10:13 wjh Stab74 $";
#endif
/*
 * MLisp interface for mspi
 * 25 feb 1986 - Miles Bader
 * Copyright 1986,1987,1988,1989,1990 by Miles Bader
 * Last edit by Miles Bader (bader) on Wed, 16 Mar 1988 -  9:21pm
 */

#include <andrewos.h>
#include <errprntf.h>
#include <mserrno.h>

#include "com.h"
#include "robin.h"

#define MAXLISPSTRING	150

/*
 * This knows about the ideo-syncrosies of MLisp string constants,
 * and should be used whenever full generality is needed.
 */
putLispString(str)
register char	*str;
{
    bool    tooLong=(strlen(str)>MAXLISPSTRING);

    if(tooLong)
	fputs("(concat ",stdout);

    do{
	int	count=MAXLISPSTRING;

	putchar('"');
	while(*str && count-->0)
	    switch(*str){
		case '"':
		case '\\':
		    putchar('\\');
		    putchar(*str++);
		    break;
		case '\n':
		    putchar('\\');
		    putchar('n');
		    str++;
		    break;
		default:
		    putchar(*str++);
	    }
	putchar('"');

	if(*str)
	    putchar(' ');
    }while(*str);

    if(tooLong)
	putchar(')');
}

#if 0
sendInit()
{
    puts("Remember, crime does not pay!");
    fflush(stdout);
}
#endif

sendCleanup() {}

sendNewMailCount(count)
int count;
{
    printf("(~bat-new-mail-count %d)\n",count);
    fflush(stdout);
}

sendStartFolderUpdate(folder)
char	*folder;
{
    printf("(~bat-start-folder-update \"%s\")\n",folder);
    fflush(stdout);
}

sendEndFolderUpdate(since,count,new,killed)
char	*since;
int	count,new,killed;
{
    printf("(~bat-end-folder-update \"%s\" %d %d %d)\n",since,count,new,killed);
    fflush(stdout);
}

sendFailFolderUpdate()
{
    printf("(~bat-fail-folder-update)\n");
    fflush(stdout);
}

sendFoldersOnlyFolderUpdate(name,since)
char	*name,*since;
{
    printf("(~bat-folders-only-folder-update \"%s\" \"%s\")\n",name,since);
    fflush(stdout);
}

sendListFolderStatus(name,status)
char	*name,*status;
{
    printf("(~bat-list-folder-status \"%s\" \"%s\")\n",name,status);
    fflush(stdout);
}

sendCallback(command)
char	*command;
{
    puts(command);
    fflush(stdout);
}

	void
beginState(char	*fmt, ...) {
    va_list ap;
    char    mbuf[80];
    va_start(ap, fmt);
    vsprintf(mbuf, fmt, ap);
    va_end(ap);

    printf("(~bat-begin-state \"%s\")\n",mbuf);
    fflush(stdout);
}

endState()
{
    puts("(~bat-end-state)");
    fflush(stdout);
}

failState()
{
    puts("(~bat-fail-state)");
    fflush(stdout);
}

sendCaption(cuid,caption,isnew)
int	cuid;
char	*caption;
bool	isnew;
{
    printf("(~bat-add-caption %d ",cuid);
    putLispString(caption);
    putchar(' ');
    if(isnew) {
	putchar('1');
    }
    else {
	putchar('0');
    }
    putchar(')');
    putchar('\n');
    fflush(stdout);
}

sendAlteredCaption(cuid,caption)
int	cuid;
char	*caption;
{
    printf("(~bat-alter-caption %d ",cuid);
    putLispString(caption);
    putchar(')');
    putchar('\n');
    fflush(stdout);
}

sendBodyFile(file,cuid)
char	*file;
int	cuid;
{
    printf("(~bat-body \"%s\" %d)\n",file,cuid);
    fflush(stdout);
}

sendMetamailFile(file,cuid)
char	*file;
int	cuid;
{
    printf("(~bat-metamail \"%s\" %d)\n",file,cuid);
    fflush(stdout);
}

sendMailFile(file)
char	*file;
{
    printf("(~bat-mail \"%s\")\n",file);
    fflush(stdout);
}

sendWantSubscription(to)
char	*to;
{
    fputs("(~bat-want-subscription ",stdout);
    putLispString(to);
    puts(")");
    fflush(stdout);
}

sendWantVote()
{
    puts("(~bat-want-vote)");
    fflush(stdout);
}

sendRewrittenHeader(name,contents)
char	*name,*contents;
{
    fputs("(~bat-rewrite-header ",stdout);
    putLispString(name);
    putchar(' ');
    putLispString(contents);
    putchar(')');
    putchar('\n');
    fflush(stdout);
}

sendSubmissionFinished()
{
    printf("(~bat-submission-finished)\n");
    fflush(stdout);
}

sendKill(cuid)
int cuid;
{
    printf("(~bat-kill-caption %d)\n",cuid);
    fflush(stdout);
}

/* ----------------------------------------------------------------
 */

bool	gotAnswer=FALSE;
char	answer[500];

inputAnswer(str)
char	*str;
{
    strcpy(answer,str);
    gotAnswer=TRUE;
}

flushAnswer()
{
    gotAnswer=FALSE;
}

char	*awaitAnswer(def)
char	*def;
{
    while(!gotAnswer)
	if(!robin_PollDriver(TRUE))
	    return def;
    gotAnswer=FALSE;
    return answer;
}

/*
 * this is also needed for cui's benefit
 */
ChooseFromList(questions,def)
char	**questions;
int	def;
{
    char    *choice;

    flushAnswer();

    printf("(~bat-choose-from-list %d",def);
    while(*questions!=NULL){
	putchar(' ');
	putLispString(*questions++);
    }
    fputs(")\n",stdout);
    fflush(stdout);

    choice=awaitAnswer(NULL);
    if(choice==NULL || atoi(choice)==0)
	return def;
    else
	return atoi(choice);
}

/*
 * this is also needed for cui's benefit
 */
bool	GetBooleanFromUser(prompt,def)
char	*prompt;
bool	def;
{
    flushAnswer();

    fputs("(~bat-get-boolean-from-user ",stdout);
    putLispString(prompt);
    printf(" %d)\n",def);
    fflush(stdout);

    return atoi(awaitAnswer(def?"1":"0"));
}

/*
 * this is also needed for cui's benefit
 */
GetStringFromUser(prompt,buf,len,noEcho)
char	*prompt,*buf;
int	len;
bool	noEcho;
{
    char mbuf[80];

    flushAnswer();

    /* Nuke any trailing colon--~bat-get-string-from-user takes care of that */
    if (strlen(prompt) < 80 && prompt[strlen(prompt)-1] == ':') {
	strcpy(mbuf, prompt);
	prompt = mbuf;
	prompt[strlen(prompt)-1] = '\0';
    }

    fputs("(~bat-get-string-from-user ",stdout);
    putLispString(prompt);
    printf(" %d)\n",!noEcho);
    fflush(stdout);

    strcpy(buf,awaitAnswer(""));
}

#define MAXMSG 500

static char mbuf[MAXMSG],*end=mbuf;

	void
info(char *fmt, ...) {
    va_list val;
    va_start(val, fmt);
    vsprintf(mbuf, fmt, val);
    va_end(val);

    ReportSuccess(mbuf);
    end=mbuf+strlen(mbuf);
}

	void
more(char *fmt, ...) {
    va_list val;
    va_start(val, fmt);
    vsprintf(end, fmt, val);
    va_end(val);

    ReportSuccess(mbuf);
    end+=strlen(end);
}

/*
 * this is also needed for cui's benefit
 */
ReportSuccess(text)
char	*text;
{
    fputs("(~bat-info ",stdout);
    putLispString(text);
    putchar(')');
    putchar('\n');
    fflush(stdout);
}

	void
error(char *fmt, ...) {
    va_list val;
    va_start(val, fmt);

    flagError();
    strcpy(mbuf,"robin: ");
    vsprintf(mbuf+sizeof("robin: ")-1, fmt, val);
    va_end(val);
    ReportError(mbuf,ERR_WARNING,FALSE);
    end=mbuf;
}

/* Same as errors, but for errors from the message server;
 * assumes mserrcode is set.
 */
	void
mserror(char *fmt, ...) {
    va_list val;
    va_start(val, fmt);

    flagError();
    strcpy(mbuf,"robin: ");
    vsprintf(mbuf+sizeof("robin: ")-1, fmt, val);
    va_end(val);

    ReportError(mbuf,ERR_WARNING,TRUE);
    end=mbuf;
}

/* urgent is like error, but more for important info than errors */
	void
urgent(char *fmt, ...) {
    va_list val;
    va_start(val, fmt);
    vsprintf(mbuf, fmt, val);
    va_end(val);
    ReportError(mbuf,ERR_MONITOR,FALSE);
    end=mbuf;
}

static void getAmsErrInfo(buf)
char *buf;
{
    extern char **unix_sys_errlist,
        *ms_errlist[],
        *ms_errcauselist[],
        *ms_errvialist[],
	*rpc_errlist[];
    extern int  unix_sys_nerr,
	ms_nerr,
	ms_nerrcause,
	ms_nerrvia,
	rpc_nerr;

    if(mserrcode!=0){
	bool isViceErr=FALSE;
	int errnum=AMS_ERRNO;
	int errcause=AMS_ERRCAUSE;
	int errvia=AMS_ERRVIA;
	int errrpc=AMS_RPCERRNO;

	/* stolen from cui */
	if(errrpc){
	    sprintf(buf,"(AMS RPC error: %s)",rpc_errlist[errrpc]);
	    return;
	}

	if(errnum<0 || errnum >=(EMSBASE+ms_nerr)
	   ||(errnum<EMSBASE && errnum>unix_sys_nerr && !vdown(errnum)))
	    errnum=EMSUNKNOWN;
	if(errcause<0 || errcause>=ms_nerrcause)
	    errcause=EIN_UNKNOWN;
	if(errvia<0 || errvia>=ms_nerrvia)
	    errvia=EVIA_UNKNOWN;

	if(errnum<EMSBASE){
	    if(vdown(errnum))
		strcpy(buf,"Connection timed out");
	    else{
		if(unix_sys_errlist[errnum])
		    strcpy(buf,unix_sys_errlist[errnum]);
		else
		    sprintf(buf,"Unknown error %d",errnum);
	    }
	}else{
	    if(ms_errlist[errnum-EMSBASE])
		strcpy(buf,ms_errlist[errnum-EMSBASE]);
	    else
		sprintf(buf,"Unknown error %d",errnum);
	}
	buf+=strlen(buf);

	if(ms_errcauselist[errcause])
	    sprintf(buf," (in %s",ms_errcauselist[errcause]);
	else
	    sprintf(buf," (in unknown call %d",errcause);
	buf+=strlen(buf);

	if(ms_errvialist[errvia])
	    sprintf(buf," in %s)",ms_errvialist[errvia]);
	else
	    sprintf(buf," in unknown caller %d)",errvia);

	if(errnum==EACCES){
	    int auth;
	    
	    mserrcode=MS_CheckAuthentication(&auth);
	    if(mserrcode){
		if(vdown(AMS_ERRNO))
		    isViceErr=TRUE;
	    }else if(!auth)
		strcat(buf," (Your Vice authentication has apparently expired)");
	}
	if(isViceErr || vdown(errnum))
	    strcat(buf," (A file server or the network is down)");
    }
}

/*
 * this is also needed for cui's benefit
 */
ReportError(text,level,decode)
char	*text;
int	level;
bool	decode;
{
    if(level==0 || level>=ERR_WARNING)
	fputs("(~bat-error ",stdout);
    else
	fputs("(~bat-urgent ",stdout);

    if(decode){
	char errbuf[2000];
	strcpy(errbuf,text);
	strcat(errbuf,": ");
	getAmsErrInfo(errbuf+strlen(errbuf));
	putLispString(errbuf);
    }else
	putLispString(text);

    putchar(')');
    putchar('\n');

    fflush(stdout);
}
