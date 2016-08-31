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
 * robin.c -- main service routines for batmail
 * 1 march 1986, Miles Bader
 * Copyright 1986,1987,1988,1989,1990 by Miles Bader
 */
#include <andrewos.h>
#include <stdio.h>
#include <ctype.h>
#include <sys/file.h>

#include <cui.h>
#include <hdrparse.h>
#include <util.h>

#include "com.h"
#include "robin.h"
#include "death.h"
#include "hshbuf.h"
#include "falias.h"
#include "fakedom.h"
#include "date.h"
#include "hash.h"


/* this gets around an fplumber bogosity */
#undef fopen

#define fcreat(file,prot) fdopen(open(file,O_CREAT|O_TRUNC|O_WRONLY,prot),"w")
#define PRIVATE 0600

/*
 * batching factors for message server rpc
 */
#define SNAPSHOTBATCH	200	    /* in snapshots */
#define BODYBATCH	30000	    /* in bytes */

/*
 * these define widths of fields in the one-line quick captions
 */
#define ATTRIBWIDTH 4	    /* unseen, deleted, 2 spaces padding */
#define DATEWIDTH   11	    /* 9 characters for date, 2 spaces padding */
#define SUBJWIDTH   (MAXSUBJ+1)	/* 1 space padding */
#define FROMWIDTH   (MAXFROM)

char faliasFile[MAXFILENAME]=FOLDERALIASFILE;
char fakedomFile[MAXFILENAME]=FAKEDOMFILE;
char killFile[MAXFILENAME]=KILLFILE;

/*
 * the cui library expects these, but we can't provide them (and since
 * they're never called, it doesn't matter
 */
int Interactive;	/* need to fool out CUI_CheckNewMessages */
SetTerminalParams(){}	/* fool machines.c */
GetTerminalParams(){}	/*  .... */


/* function headers */
static VOID cacheFolderUpdate(/* char *folder, char *date */);

/* ----------------------------------------------------------------
 * dealing with the quick captions
 */

/*
 * print str until character c is reached in a field wid wide; return a
 * pointer into str after c (or the null byte).  If more is TRUE, the
 * field is ended with a space, otherwise, a newline.
 */
static buildField(bufP,strP,c,wid,eolp)
char		**bufP,
		**strP;
char	c;
int	wid;
bool		eolp;
{
    char   *output= *bufP,
		    *str= *strP;

    if(!eolp)
	wid--;

    while(*str && *str!=c && wid>0){
	if(isprint(*str))
	    *output++= *str;
	else if(*str=='\b' && output>*bufP)
	    output--, wid++;
	else if(wid>1){
	    *output++='^';
	    wid--;
	    *output++=(*str^'@')&0x7f;
	}
	str++;
	wid--;
    }

    if(!eolp)
	while(wid-->=0)
	    *output++=' ';

    *bufP=output;

    while(*str && *str!=c)
	str++;

    if(*str)
	*strP= ++str;
    else
	*strP=str;
}

struct attFlag {
    char    symbol;
    int	    att;
    bool    wonly;
};

static buildAttribs(bufP,snapshot,flags,wid,eolp)
char	**bufP;
char	*snapshot;
int	flags;
int	wid;
bool	eolp;
{
    static struct attFlag flagList[]={
	{'U',	AMS_ATT_UNSEEN,	    TRUE},
	{'D',	AMS_ATT_DELETED,    TRUE},
	{'!',	AMS_ATT_URGENT,	    FALSE},
	{'A',	AMS_ATT_REPLIEDTO,  TRUE},
	{'C',	AMS_ATT_CLOSED,	    FALSE},
	{'E',	AMS_ATT_ENCLOSURE,  FALSE},
	{'R',	AMS_ATT_RRR,	    TRUE},
	{'u',	AMS_ATT_UNAUTH,	    FALSE},
	{'v',	AMS_ATT_VOTE,	    FALSE},
	{'\0',	0,		    FALSE}
    };
    static struct attFlag robinFlagList[]={
        {'2', FLAG_DUP, FALSE},
        {'K', FLAG_KILLED, FALSE},
        {'\0', 0, FALSE}
    };
    struct attFlag *flag;
    char    *output= *bufP;
    bool    writeable=AMS_GET_ATTRIBUTE(snapshot,AMS_ATT_MAYMODIFY);

    if(!eolp)
	wid--;

    for(flag=flagList; flag->symbol!='\0' && wid>0; flag++)
	if(AMS_GET_ATTRIBUTE(snapshot,flag->att) &&
	   (writeable || !flag->wonly)){
	    *output++=flag->symbol;
	    wid--;
	}

    for(flag=robinFlagList; flag->symbol!='\0' && wid>0; flag++)
	if(flags&flag->att && (writeable || !flag->wonly)){
	    *output++=flag->symbol;
	    wid--;
	}

    if(!eolp)
	while(wid-->=0)
	    *output++=' ';

    *bufP=output;
}

static	int	subjWidth=SUBJWIDTH,
		fromWidth=FROMWIDTH;

/*
 * set the field widths depending on the desired width of the output
 * line
 */
VOID robin_SetCaptionWidth(wid)
int	wid;
{
    subjWidth=SUBJWIDTH;
    fromWidth=FROMWIDTH;

    wid-=(ATTRIBWIDTH+DATEWIDTH+SUBJWIDTH+FROMWIDTH);
    if(wid<0){
	/*
	 * if have to shrink, shrink both subject and from fields
	 */
	subjWidth+=wid/2+wid%2;
	fromWidth+=wid/2;

	if(fromWidth<=10){
	    error("Caption length of %d is too little!",
		  wid+(ATTRIBWIDTH+DATEWIDTH+SUBJWIDTH+FROMWIDTH));
	    subjWidth=fromWidth=10;
	}
    }else
	/*
	 * but only grow the from field (since CUI limits the length
	 * of the subject field anyway)
	 */
	fromWidth+=wid;
}

/*
 * print caption to fit into a character line
 * ATTRIBS   DATE   SUBJECT   SENDER
 * <--4--> <--11--> <--30--> <--29-->
 */
static char *buildCaption(snapshot,flags)
char	*snapshot;
int	flags;
{
    char    *caption=AMS_CAPTION(snapshot);
    int leftWidth;
    static char	outputBuf[150];
    char    *output=outputBuf,*end;

    buildAttribs(&output,snapshot,flags,ATTRIBWIDTH,FALSE); /* attributes */

    buildField(&output,&caption,'\t',DATEWIDTH,FALSE);	/* date */

    if (!(char *)strchr(caption, '\t')) {
	/* No from field -- DowJones-style long subjects */
	leftWidth = subjWidth + fromWidth;
    }
    else {
	buildField(&output,&caption,'\t',subjWidth,FALSE);	/* subject */
	leftWidth = fromWidth;
    }

    /* Specially process last field, so hopefully, we can show the
     * whole msg size
     */
    for(end=caption; *end!='\0' && *end!='\t'; end++)
      ;
    if(end>caption && end[-1]==')' && end-caption>leftWidth){
	int datewidth=0;
	while(end>caption && *end!='(')
	  end--,datewidth++;
	buildField(&output,&caption,'\t',leftWidth-datewidth,FALSE);/* sender */
	buildField(&output,&end,'\t',datewidth,TRUE);	/* msg size */
    }
    else {
	buildField(&output,&caption,'\t',leftWidth,TRUE);/* sender */
    }
    *output='\0';

    return outputBuf;
}

/* ----------------------------------------------------------------
 */

char	currentFolder[MAXFOLDERNAME]="",
	*currentFullName=NULL;

VOID robin_SetFolder(folder)
char	*folder;
{
    folder=falias_Unalias(folder);
    if(strcmp(folder,currentFolder)!=0){
	strcpy(currentFolder,folder);
	currentFullName=NULL;
    }
}

static bool noFolder()
{
    if(*currentFolder=='\0'){
	error("No current folder");
	return TRUE;
    }else
	return FALSE;
}

static bool badFolder()
{
    if(noFolder())
	return TRUE;
    else if(currentFullName==NULL &&
	     CUI_DisambiguateDir(currentFolder,&currentFullName)!=0){
	error("Bad folder: %s",falias_Alias(currentFolder));
	return TRUE;
    }else
	return FALSE;
}

static bool badCuid(cuid)
int	cuid;
{
    extern int	CUI_CuidsInUse;

    if(cuid<=0 || cuid>CUI_CuidsInUse){
	error("Bad cuid: %d",cuid);
	return TRUE;
    }else
	return FALSE;
}

/* ----------------------------------------------------------------
 */

static char *ensureLocalFile(buf)
char	*buf;
{
    extern int	CUI_OnSameHost;

    if(!CUI_OnSameHost){
	char	newBuf[MAXFILENAME];

	CUI_GenTmpFileName(newBuf);
	if(CUI_GetFileFromVice(newBuf,buf)==0)
	    strcpy(buf,newBuf);
	else{
	    flagError();
	    return NULL;
	}
    }

    return buf;
}

static char *ensureRemoteFile(buf)
char	*buf;
{
    extern int	CUI_OnSameHost;

    if(!CUI_OnSameHost){
	char	newBuf[MAXFILENAME];

	CUI_GenTmpFileName(newBuf);
	if(CUI_StoreFileToVice(buf,newBuf)==0)
	    strcpy(buf,newBuf);
	else{
	    flagError();
	    return NULL;
	}
    }

    return buf;
}

/* ----------------------------------------------------------------
 */

static void reportKill(ks,startCuid)
struct killSpec *ks;
int startCuid;
{
    if(ks!=NULL){
	int num;
	
	beginState("Killing");
	num=robin_FindKilledCaptions(startCuid,ks);
	endState();
	
	if(num==0)
	    info("No captions killed!?!?");
	else
	    info("%d caption%s killed",num,(num==1?"":"s"));
    }
}

VOID robin_KillMsg(cuid,field,to,startCuid)
int cuid;
int field;
bool to;
int startCuid;
{
    char buf[AMS_SNAPSHOTSIZE];
    if(CUI_GetSnapshotFromCUID(cuid,buf)==0){
	death_SetFolder(currentFolder);
	reportKill(death_KillMsg(cuid,buf,field,to),startCuid);
    }
}

VOID robin_KillStr(str,field,meth,to,startCuid)
char *str;
int field;
bool to;
int startCuid;
{
    death_SetFolder(currentFolder);
    reportKill(death_KillBuf(str,strlen(str),field,meth,to),startCuid);
}

VOID robin_CaptionsSince(since)
char	*since;
{
    static char	snapshots[SNAPSHOTBATCH*AMS_SNAPSHOTSIZE],*ss;
    int	    start=0,left,size,count=0,new=0,skipped=0,killed=0;
    bool    isUpdate;
    char    lastRead[AMS_DATESIZE],	/* last updated to this point */
	    since64[AMS_DATESIZE],	/* read since this date */
	    skippedTo[AMS_DATESIZE];	/* date where last message skipped
					   before any are read is */


    if(badFolder())
	return;

    bzero(skippedTo,sizeof(skippedTo));
    mserrcode=MS_GetAssociatedTime(currentFullName,lastRead,AMS_DATESIZE);
    if(mserrcode!=0){
	mserror("Couldn't get last-read date for %s",currentFolder);
	return;
    }

    isUpdate=(since==NULL || *since=='\0');
    if(isUpdate)
	if(date64_IsCreation(lastRead))
	    /* first time reading this, don't give everything */
	    since="4 days ago";
	else
	    memmove(since64,lastRead,AMS_DATESIZE);

    if(since!=NULL && *since!='\0'){
	char	*date64=date64_Parse(since);

	if(date64==NULL){
	    error("Bad date format: %s",since);
	    return;
	}else
	    memmove(since64,date64,AMS_DATESIZE);
    }

    sendStartFolderUpdate(falias_Alias(currentFolder));

    death_SetFolder(currentFolder);

    do{
	if(CUI_GetHeaders(currentFullName,since64,snapshots,
			  SNAPSHOTBATCH*AMS_SNAPSHOTSIZE,
			  start,&size,&left,TRUE)!=0){
	    char    *bogusFullName;

	    if(CUI_DisambiguateDir(currentFullName,&bogusFullName,0)!=0 && AMS_ERRNO==ENOENT)
		CUI_HandleMissingFolder(currentFullName);

	    sendFailFolderUpdate();

	    flagError();
	    return;
	}

	start+=size;

	for(ss=snapshots; size>0; size-=AMS_SNAPSHOTSIZE,ss+=AMS_SNAPSHOTSIZE){
	    int	    cuid;
	    int     isDup;
	    int	    flags=0;

	    cuid=CUI_GetCuid(AMS_ID(ss),currentFullName,&isDup);
	    if(isDup)
		flags|=FLAG_DUP;

/*XXX WAS	    if(death_MsgKilled(cuid,ss,flags)){*/
	    if(death_MsgKilled(cuid,ss)){
		killed++;
		flags|=FLAG_KILLED;
	    }

	    if(flags&FLAG_KILLED && isUpdate){
		if(new==0)
		    memmove(skippedTo,AMS_DATE(ss),AMS_DATESIZE);
		skipped++;
	    }else{
/*XXX		count++==0; */
	        count++;
		if(new>0 || date64_Cmp(AMS_DATE(ss),lastRead)>0)
		    new++;		
		sendCaption(cuid,buildCaption(ss,flags),(new>0));
	    }
	}
    }while(left>0);

    /*
     * if we skipped any message initially, we'll never see them again
     * (this is so that if message killing criteria changes, you
     * won't suddenly see old messages from bboard *never* displayed).
     */
    if(skipped>0 && date64_Cmp(lastRead,skippedTo)<0){
	mserrcode=MS_SetAssociatedTime(currentFullName,skippedTo);
#if 0
	urgent("Set %s last-update to %s in captions-since",
	       currentFolder,date64_Unparse(skippedTo));
#endif
	if(mserrcode!=0)
	    mserror("Error updating state of folder %s",
		    falias_Alias(currentFolder));
    }

    /*
     * If the first time reading & no message, update the last read date
     * now, so we won't have to check again next time.
     */
    if(count==0 && date64_IsCreation(lastRead)){
	mserrcode=MS_SetAssociatedTime(currentFullName,since64);
#if 0
	urgent("Set %s last-update to %s in captions-since-2",
	       currentFolder,date64_Unparse(skippedTo));
#endif
	if(mserrcode!=0)
	    mserror("Error updating state of folder %s",
		    falias_Alias(currentFolder));
    }

    if(count>0){
	VOID cacheFolderUpdate();
	cacheFolderUpdate(currentFolder,since64);
    }

    sendEndFolderUpdate(since,count,new,killed);
}

int robin_FindKilledCaptions(startCuid,ks)
int startCuid;
struct killSpec *ks;
{
    static char	snapshots[SNAPSHOTBATCH*AMS_SNAPSHOTSIZE];
    char snapshot[AMS_SNAPSHOTSIZE],*ss;
    char *msgDate;
    int foundStart=FALSE, alter=FALSE;
    int start=0,left,killed=0;
    long conv64tolong(),convlongto64();
    char    lastRead[AMS_DATESIZE];	/* last updated to this point */

    if(badFolder())
	return 0;

    if(badCuid(startCuid) || CUI_GetSnapshotFromCUID(startCuid,snapshot)!=0)
	return 0;

    msgDate=AMS_DATE(snapshot);

    mserrcode=MS_GetAssociatedTime(currentFullName,lastRead,AMS_DATESIZE);
    if(mserrcode!=0){
	mserror("Couldn't get last-read date for %s",currentFolder);
	return 0;
    }

    if(date64_Cmp(lastRead,msgDate)>=0)
	alter=TRUE;		/* heuristic to decide whether to delete  */

    strcpy(msgDate, (char *)convlongto64(conv64tolong(msgDate)-1,0));

    death_SetFolder(currentFolder);

    do{
	int size;

	if(CUI_GetHeaders(currentFullName,msgDate,snapshots,
			  SNAPSHOTBATCH*AMS_SNAPSHOTSIZE,
			  start,&size,&left,TRUE)!=0){
	    char    *bogusFullName;

	    if(CUI_DisambiguateDir(currentFullName,&bogusFullName,0)!=0 &&
	       AMS_ERRNO==ENOENT)
		CUI_HandleMissingFolder(currentFullName);

	    flagError();
	    return killed;
	}

	start+=size;

	for(ss=snapshots; size>0; size-=AMS_SNAPSHOTSIZE,ss+=AMS_SNAPSHOTSIZE){
	    int	    cuid;
	    int     isDup;
	    int	    flags=0;

	    cuid=CUI_GetCuid(AMS_ID(ss),currentFullName,&isDup);
	    if(isDup)
		flags|=FLAG_DUP;

	    if(death_MsgKilledBy(cuid,ss,ks)){
		flags|=FLAG_KILLED;
		killed++;
	    }

	    if((foundStart || (cuid==startCuid && (foundStart=TRUE))) &&
	       flags&FLAG_KILLED)
		if(alter)
		    sendAlteredCaption(cuid,buildCaption(ss,flags));
		else
		    sendKill(cuid);
	}
    }while(left>0);

    return killed;
}

/* ----------------------------------------------------------------
 */

HASHTABLE   *folderUpdateTable;

static VOID cacheFolderUpdate(folder,date)
char	*folder,*date;
{
    char    *copy=(char *)malloc(AMS_DATESIZE);

    memmove(copy,date,AMS_DATESIZE);
    hash_Install(folderUpdateTable,folder,copy);
}

VOID robin_UpdateLastRead(cuid)
int	cuid;
{
    char    *date;
    int discard;

    if(badFolder())
        return;

    discard=death_Update(killFile);
    if(discard>0)
	info("%d kill specs discarded",discard);

    if(cuid==0){	/* we think we have a cached date */
	date=(char *)hash_Find(folderUpdateTable,currentFolder);
	if(date==NULL){	/* probably never saw any messages in this folder */
	    error("No cache date for %s!",falias_Alias(currentFolder));
	    return;
	}
#if 0
	urgent("Found date %s stored in folder-update-table",
	       date64_Unparse(date));
#endif
    }else{
        char	snapshot[AMS_SNAPSHOTSIZE];

        if(badCuid(cuid) || CUI_GetSnapshotFromCUID(cuid,snapshot)!=0){
            flagError();
	    return;
        }

	date=AMS_DATE(snapshot);
#if 0
	urgent("Found date %s on snapshot %d",date64_Unparse(date),cuid);
#endif
    }

    mserrcode=MS_SetAssociatedTime(currentFullName,date);
#if 0
    urgent("Set %s last-update to %s in update-last-read",
	   currentFolder,date64_Unparse(date));
#endif
    if(mserrcode!=0)
	mserror("Error updating state of folder %s",
		falias_Alias(currentFolder));
}

/* ----------------------------------------------------------------
 */

static char	text[BODYBATCH];

VOID robin_BodyFile(cuid,width,stripheaders,metamail)
int	cuid;
int	width;
bool	stripheaders;
bool	metamail;
{
    char bodyFile[MAXFILENAME];
    FILE    *fp;


    CUI_GenTmpFileName(bodyFile);
    fp=fcreat(bodyFile,PRIVATE);
    if(fp==NULL)
	error("Can't open body file: %s",bodyFile);
    else{
	int	offset=0,left,size;
	char	ss[AMS_SNAPSHOTSIZE];
	char	format[256];
	bool	inheaders=TRUE;
	bool	unscribe=FALSE;
	char	remfile[MAXFILENAME];

	if(CUI_GetSnapshotFromCUID(cuid,ss)!=0){
	    flagError();
	    fclose(fp);
	    unlink(bodyFile);
	    return;
	}
	
	beginState("Fetching %d",cuid);
	if(AMS_GET_ATTRIBUTE(ss,AMS_ATT_FORMATTED) &&
	   CUI_GetHeaderContents(cuid,NULL,HP_SCRIBEFORMAT,format,20)==0 &&
	   strcmp(format,"2")==0)
	    unscribe=TRUE;
	else{
	    char *MSid, *MSdir;
	    if(CUI_GetAMSID(cuid,&MSid,&MSdir)!=0){
		flagError();
		fclose(fp);
		unlink(bodyFile);
		failState();
		return;
	    }
	    mserrcode=MS_WriteUnscribedBodyFile(MSdir,MSid,remfile);
	    if(mserrcode!=0){
		mserror("Error unscribing message %d",cuid);
		fclose(fp);
		unlink(bodyFile);
		failState();
		return;
	    }
	}

	headfilter_Begin(fp,width,stripheaders);

	do{
	    int ret=
	      (unscribe
	       ? CUI_GetPartialBody(text,sizeof(text),cuid,offset,&left,&size)
	       : MS_GetPartialFile(remfile,text,sizeof(text),offset,&left,&size));

	    if(ret!=0){
		fclose(fp);
		unlink(bodyFile);
		failState();
		flagError();
		return;
	    }

	    offset+=size;
	    if(size>0){
		if(inheaders){
		    char    *headfilter_Filter(),
			    *end=headfilter_Filter(text,size);
		    if(end!=NULL){
			inheaders=FALSE;
			if(unscribe){
			    unscribe_begin(fp,width);
			    unscribe_filter(end,size-(end-text));
			}else{
			    wrap_Begin(fp,width);
			    wrap_Filter(end,size-(end-text));
			}
		    }
		}else if(unscribe)
		    unscribe_filter(text,size);
		else
		    wrap_Filter(text,size);
	    }
	}while(left>0);

	if(inheaders)
	    headfilter_End();
	else if(unscribe)
	    unscribe_end();
	else
	    wrap_End();

	if(fclose(fp)==EOF){
	    syserr("Error closing",bodyFile);
	    failState();
	    return;
	}

	if (metamail) {
	    sendMetamailFile(bodyFile,cuid);
	}
	else {
	    sendBodyFile(bodyFile,cuid);
	}
	endState();

	if(AMS_GET_ATTRIBUTE(ss,AMS_ATT_NEWDIR)){
	    char    fullname[MAXFILENAME],nickname[MAXFOLDERNAME];
	    int	    status;

	    if(CUI_GetHeaderContents(cuid,NULL,HP_DIRECTORYCREATION,
				     fullname,MAXFILENAME)==0)
	    {
		mserrcode=MS_GetSubscriptionEntry(fullname,nickname,&status);
		if(mserrcode!=0)
		    mserror("Couldn't get subscription entry for %s",fullname);
		else if(status==AMS_UNSUBSCRIBED){
		    if(*nickname=='\0')
			CUI_BuildNickName(fullname,nickname);
		    sendWantSubscription(falias_Alias(nickname));
		}
	    }
	}

	if(AMS_GET_ATTRIBUTE(ss,AMS_ATT_VOTE))
	    sendWantVote();
	if(AMS_GET_ATTRIBUTE(ss,AMS_ATT_RRR) &&
	   AMS_GET_ATTRIBUTE(ss,AMS_ATT_MAYMODIFY))
	    CUI_HandleAckRequest(cuid,ss);

/*	if(AMS_GET_ATTRIBUTE(ss,AMS_ATT_ENCLOSURE))
	    CUI_HandleEnclosure(cuid,ss);
*/
	if(AMS_GET_ATTRIBUTE(ss,AMS_ATT_REDISTRIBUTION))
	    CUI_HandleRedistributionMessage(cuid,ss);

	CUI_HandleCustomizationMessage(cuid,ss);

	CUI_PrefetchMessage(cuid,TRUE);

	if(!unscribe)
	    MS_UnlinkFile(remfile);
    }
}

VOID robin_AppendToFile(cuid,filename)
int	cuid;
char	*filename;
{
    FILE    *fp;
    int pipe=FALSE;
    FILE *popen();
    int pclose();

    if(*filename=='|'){
	pipe=TRUE;
	fp=popen(++filename,"w");
    }else
	fp=fopen(filename,"a");

    if(fp==NULL)
	error("Can't %s: %s",(pipe?"start process":"open file"),filename);
    else{
	int	offset=0,left,size;
	char	ss[AMS_SNAPSHOTSIZE];
	
	if(CUI_GetSnapshotFromCUID(cuid,ss)!=0){
	    flagError();
	    return;
	}

	do{
	    if(CUI_GetPartialBody(text,sizeof(text),cuid,offset,&left,&size)!=0){
		fclose(fp);
		flagError();
		return;
	    }

	    offset+=size;

	    if(fwrite(text,1,size,fp)<size){
		if(pipe)
		    syserr("Error writing to pipe",filename);
		else
		    syserr("Error writing to",filename);
		break;
	    }
	}while(left>0);

	if(pipe){
	    int exstat=pclose(fp);
	    if(exstat!=0)
		error("`%s' exited with status %d",exstat);
	}else
	    if(fclose(fp)==EOF)
		syserr("Error closing",filename);
    }
}

/* ----------------------------------------------------------------
 */

VOID robin_CheckNewMail(all)
bool	all;
{
    extern int homeUsesAMSDelivery;
    char    *box;
    int	    newmsgs;

    if(all)
	box=NULL;
    else if(!badFolder())
	box=currentFullName;
    else
	return;

    if (homeUsesAMSDelivery) {
	mserrcode=MS_DoIHaveMail(&newmsgs);
	if (mserrcode==0 && newmsgs == 0) {
	    return;
	}
    }
    beginState("Checking mailbox");
    if(CUI_CheckMailboxes(box)==0)
      endState();
    else
      failState();
}

/* ----------------------------------------------------------------
 */

VOID robin_SubsFile()
{
    char mapfile[MAXFILENAME];

    if(badFolder())
	return;

    mserrcode=MS_NameSubscriptionMapFile(currentFullName,mapfile);
    if(mserrcode!=0)
	mserror("Couldn't generate subscription file");
    else if(ensureLocalFile(mapfile)==NULL)
	return;
}

/* ----------------------------------------------------------------
 */

struct checkRock {
    int foldersOnly;
    char *since;
};

static VOID checkSub(nickname,fullname,subsMeth,rock)
char *nickname,*fullname;
int subsMeth;
struct checkRock *rock;
{
    static int sawprint = 0;

    if(subsMeth!=AMS_UNSUBSCRIBED){
	if(rock->foldersOnly || subsMeth==AMS_ASKSUBSCRIBED)
	    sendFoldersOnlyFolderUpdate(falias_Alias(nickname),rock->since);
	else{
	    /* bypass SetFolder for speed */
	    strcpy(currentFolder,nickname);
	    
	    currentFullName=fullname;
	    
	    switch(subsMeth){
	      case AMS_ALWAYSSUBSCRIBED:
		robin_CaptionsSince(rock->since);
		break;
	      case AMS_SHOWALLSUBSCRIBED:
		robin_CaptionsSince("creation");
		break;
	      case AMS_PRINTSUBSCRIBED:
		if (!sawprint) {
		    sawprint++;
		    info("robin: Holy paper hog, BatMail!  I don't support print subscriptions.");
		}
		break;
	    }
	}
    }
}

static VOID listStatus(nickname,fullname,subsMeth,rock)
char *nickname,*fullname;
int subsMeth;
VOID *rock;
{
    char *status;

    switch(subsMeth){
	case AMS_UNSUBSCRIBED:
	    status="Not subscribed";
	    break;
	case AMS_ALWAYSSUBSCRIBED:
	    status="Normal subscription";
	    break;
	case AMS_ASKSUBSCRIBED:
	    status="Ask subscribed";
	    break;
	case AMS_SHOWALLSUBSCRIBED:
	    status="Showall subscribed";
	    break;
	case AMS_PRINTSUBSCRIBED:
	    status="Print subscribed";
	    break;
	default:
	    error("Bad subscription type: %c",subsMeth);
	    return;
    }

    sendListFolderStatus(falias_Alias(nickname),status);
}

static bool	haltScan=FALSE;

VOID robin_StopFolderScan()
{
    haltScan=TRUE;
}

/* assume (1) can overwrite filename, (2) will delete file */
scanMapfile(file,proc,rock)
char *file;
VOID (*proc)();
VOID *rock;
{
    FILE    *fp;

    haltScan=FALSE;

    if(ensureLocalFile(file)==NULL)
	return;

    fp=fopen(file,"r");
    if(fp==NULL){
	error("Couldn't open file (%s) in scanMapfile",file);
	return;
    }

    for(;;){
	char	fullname[MAXFILENAME], nickname[MAXFOLDERNAME];
	int	subsMeth, isNew;
	int	red=fscanf(fp,"%[^:]:%[^ ] %d %d\n",nickname,fullname,&subsMeth,&isNew);

	if(red<2)
	    break;

	(*proc)(nickname,fullname,subsMeth,rock);

	robin_PollDriver(FALSE);
	if(haltScan){
	    flagError();
	    break;
	}
    }

    fclose(fp);
    unlink(file);
}

VOID scanAllSubscr(proc,rock)
VOID (*proc)();
VOID *rock;
{
    char    mapfile[MAXFILENAME];
    int numChanged,D;

    mserrcode=MS_NameChangedMapFile(mapfile,FALSE,FALSE,&numChanged,&D,&D,&D,&D);
    if(mserrcode!=0)
	mserror("Couldn't get changed map file");
    else{
	info("%d folders with new messages...",numChanged);
	scanMapfile(mapfile,proc,rock);
    }
}

VOID scanRootSubscr(root,proc,rock)
char *root;
VOID (*proc)();
VOID *rock;
{
    char fullname[MAXFILENAME], nickname[MAXFOLDERNAME];
    int	subsMeth, rootlen;

    haltScan=FALSE;

    robin_SetFolder(root);
    if(badFolder())
	return;

    rootlen=strlen(root);
    strcpy(fullname,currentFullName);

    mserrcode=MS_GetSubscriptionEntry(fullname,nickname,&subsMeth);

    while(mserrcode==0 && *fullname!='\0' && strncmp(nickname,root,rootlen)==0){
	(*proc)(nickname,fullname,subsMeth,rock);

	robin_PollDriver(FALSE);
	if(haltScan){
	    flagError();
	    break;
	}

	mserrcode=MS_GetNextSubsEntry(fullname,nickname,&subsMeth);
    }

    if(mserrcode!=0)
	mserror("Error in scanRootSubscr(%s)",fullname);
}

/* assume (1) can overwrite filename, (2) will delete file */
scanMatchFile(file,match,proc,rock)
char *file;
char *match;			/* if non-NULL, only uses immediate children */
VOID (*proc)();
VOID *rock;
{
    char nickname[MAXFOLDERNAME];
    FILE *fp;
    int mlen;

    haltScan=FALSE;

    if(ensureLocalFile(file)==NULL)
	return;

    fp=fopen(file,"r");
    if(fp==NULL){
	error("Couldn't open file (%s) in scanMatchFile",file);
	return;
    }

    if(match!=NULL)
	mlen=strlen(match);

    while(fgets(nickname,MAXFOLDERNAME,fp)!=NULL){
	int subsMeth;

	nickname[strlen(nickname)-1] = 0; /* Nuke trailing newline */

	if(match==NULL || (char *)strchr(nickname+mlen+1,'.')==NULL){
	    robin_SetFolder(nickname);
	    
	    if(!badFolder()){
		mserrcode=MS_GetSubscriptionEntry(currentFullName,nickname,&subsMeth);
		if(mserrcode!=0)
		    mserror("Couldn't get subscription entry for %s",currentFullName);
		else
		    (*proc)(nickname,currentFullName,subsMeth,rock);
	    }
	}

	robin_PollDriver(FALSE);
	if(haltScan){
	    flagError();
	    break;
	}
    }

    fclose(fp);
    unlink(file);
}

VOID scanRootAll(root,immOnly,proc,rock)
char *root;
bool immOnly;			/* only use immediate children */
VOID (*proc)();
VOID *rock;
{
    char outputFile[MAXFILENAME];
    char pfx[MAXFOLDERNAME];

    robin_SetFolder(root);
    if(badFolder())
	return;

    strcpy(pfx,root);
    strcat(pfx,".");

    mserrcode=MS_MatchFolderName(pfx,outputFile);
    if(mserrcode!=0)
	mserror("Couldn't match folder name %s",root);
    else
	scanMatchFile(outputFile,(immOnly ? root : NULL),proc,rock);
}

VOID robin_ScanFolders(since,foldersOnly)
char	*since;
bool	foldersOnly;
{
    struct checkRock rock;

    rock.since=since;
    rock.foldersOnly=foldersOnly;

    if(*currentFolder!='\0' || (since!=NULL && *since!='\0'))
	scanRootSubscr(currentFolder,checkSub,&rock);
    else
	scanAllSubscr(checkSub,&rock);

    robin_SetFolder("");	/* compensate for trickery in checkSub */
}

VOID robin_ListFolders(immOnly)
bool immOnly;
{
    scanRootAll(currentFolder,immOnly,listStatus,NULL);
}

/* ----------------------------------------------------------------
 */

VOID robin_SetAttribute(cuid,att,on,ignorable)
int	cuid;
int	att;
bool	on, ignorable;
{
    char    buf[AMS_SNAPSHOTSIZE];
    int	    err;
    char    *dir;

    if((err=CUI_GetSnapshotFromCUID(cuid,buf))==0 &&
       (!AMS_GET_ATTRIBUTE(buf,att))!=!on &&
       (!ignorable || AMS_GET_ATTRIBUTE(buf,AMS_ATT_MAYMODIFY))){
	if(on)
	    AMS_SET_ATTRIBUTE(buf,att);
	else
	    AMS_UNSET_ATTRIBUTE(buf,att);
	switch(att){
	    case AMS_ATT_DELETED:
		err=(on ? CUI_DeleteMessage(cuid) : CUI_UndeleteMessage(cuid));
		break;
	    case AMS_ATT_UNSEEN:
		err=(on ? CUI_MarkAsUnseen(cuid) : CUI_MarkAsRead(cuid));
		break;
	    default:
		err=CUI_AlterSnapshot(cuid,buf,ASS_REPLACE_ATTRIBUTES,&dir);
		if(err!=0)
		    error("Error modifying caption of message %d",cuid);
	}
	if(err==0)
	    sendAlteredCaption(cuid,buildCaption(buf,0));
    }

    if(err!=0)
	flagError();
}

/* ----------------------------------------------------------------
 */

VOID robin_MailFile(code,cuid)
int	code;
int	cuid;
{
    char    file[MAXFILENAME];

    beginState("Making mail template");

    if(code==AMS_REPLY_GRIPE || code==AMS_REPLY_POST){ /* special cases */
	FILE	*fp;

	CUI_GenTmpFileName(file);

	fp=fcreat(file,PRIVATE);
	if(fp==NULL){
	    failState();
	    error("Can't open reply template: %s",file);
	    return;
	}

	switch(code){
  	    case AMS_REPLY_GRIPE: {
		static char *MessagesBugAddress = NULL;
		if (MessagesBugAddress == NULL) {
			MessagesBugAddress =
				GetConfiguration("MessagesBugAddress");
		}
		if (MessagesBugAddress == NULL) {
			MessagesBugAddress =
#ifdef CMU_ENV
				"info-andrew-bugs+batmail@andrew.cmu.edu";
#else /* CMU_ENV */
				"postmaster";
#endif /* CMU_ENV */
		}

		fprintf(fp,"To: %s\nSubject: bat-gripe\n\n",MessagesBugAddress);
		}
		break;
	    case AMS_REPLY_POST:
		if(!badFolder())
		    fprintf(fp,"To: %s\nSubject: \nCC: \n\n",currentFolder);
		break;
	}

	fclose(fp);
    }else if(CUI_NameReplyFile(cuid,code,file)!=0 ||
	     ensureLocalFile(file)==NULL){
	flagError();
	failState();
	return;
    }

    sendMailFile(file);

    endState();
}

VOID robin_RewriteHeader(name,contents)
char	*name,*contents;
{
    char    *rewrittenContents, *faliasContents, *fakedomContents;

    if(*contents=='\0')
	return;

    faliasContents=falias_RewriteAddrs(contents);
    fakedomContents=fakedom_RewriteAddrs(faliasContents);

    if(CUI_RewriteHeaderLine(fakedomContents,&rewrittenContents)==0){
	if(*rewrittenContents!='\0')
	    sendRewrittenHeader(name,rewrittenContents);
	free(rewrittenContents);
    }else
	flagError();

    if(fakedomContents!=faliasContents)
	free(fakedomContents);
    if(faliasContents!=contents)
	free(faliasContents);
}

VOID robin_SubmitMail(file,blind,inReplyTo)
char	*file;
bool	blind;
int	inReplyTo;
{
    char    fname[MAXFILENAME];

    strcpy(fname,file);
    if(ensureRemoteFile(fname)==NULL)
	return;

    beginState("Delivering");

    if(CUI_SubmitMessage(fname,(blind ? AMS_SEND_BLINDYES : 0))!=0){
	flagError();
	failState();
    }else{
	endState();	/* end state first for quicker feedback */
	if(inReplyTo>0)
	    robin_SetAttribute(inReplyTo,AMS_ATT_REPLIEDTO,TRUE,TRUE);
    }
}

/* ----------------------------------------------------------------
 */

VOID robin_AlterSubscription(subsMeth)
int	subsMeth;
{
    char    *name,*fullname=currentFullName;

    if(noFolder())
	return;

    if(fullname==NULL &&
       CUI_DisambiguateDir(currentFolder,&fullname)!=0){
	static char fname[MAXFILENAME];
	int	oldSubsMeth;

	mserrcode=MS_GetSubscriptionEntry(fname,currentFolder,&oldSubsMeth);
	if(mserrcode!=0){
	    mserror("Not subscribed to %s",falias_Alias(currentFolder));
	    return;
	}else
	    fullname=fname;
    }

    switch(subsMeth){
      case AMS_UNSUBSCRIBED:
	name="Unsubscribed"; break;
      case AMS_ALWAYSSUBSCRIBED:
	name="Subscribed"; break;
      case AMS_ASKSUBSCRIBED:
	name="Ask subscribed"; break;
      case AMS_SHOWALLSUBSCRIBED:
	name="Showall subscribed"; break;
      case AMS_PRINTSUBSCRIBED:
	name="Print subscribed"; break;
      default:
	error("Bad subscription type: %d",subsMeth);
	return;
    }

    mserrcode=MS_SetSubscriptionEntry(fullname,currentFolder,subsMeth);
    if(mserrcode!=0)
	mserror("Couldn't subscribe to %s!",falias_Alias(currentFolder));
    else
	info("%s to %s",name,falias_Alias(currentFolder));
}

VOID robin_Resend(cuid,to)
int	cuid;
char	*to;
{
    char    *faliasContents, *fakedomContents;

    if(badCuid(cuid))
	return;

    faliasContents=falias_RewriteAddrs(to);
    fakedomContents=fakedom_RewriteAddrs(faliasContents);

    if(CUI_ResendMessage(cuid,fakedomContents)!=0)
	flagError();

    if(fakedomContents!=faliasContents)
	free(fakedomContents);
    if(faliasContents!=to)
	free(faliasContents);
}

/* ----------------------------------------------------------------
 */

/*
 * this is also needed for cui's benefit
 */
DirectoryChangeHook(add,delete,rock)
char	*add,*delete,*rock;
{
    info("DirectoryChangeHook (add==%s, delete==%s, rock==%x)",add,delete,rock);
}

SubscriptionChangeHook()
{
}

/*
 * return CUI_RPC_RETRY or CUI_RPC_RESTART or CUI_RPC_BUGOUT
 */
HandleTimeout(name,retries,restarts)
char	*name;
int	retries,restarts;
{
    if(retries<5)
	return CUI_RPC_RETRY;
    else if(restarts<3){
	info("Restarting message server...");
	return CUI_RPC_RESTART;
    }else{
	error("Can't restart message server!");
	return CUI_RPC_BUGOUT;
    }
}

/*
 * this is also needed for cui's benefit
 */
DidRestart()
{
    info("Restarted message server...");
}

/* ----------------------------------------------------------------
 */

VOID robin_PurgeFolders(all)
bool	all;
{
    char    *name;

    if(all)
	name=NULL;
    else if(!badFolder())
	name=currentFullName;
    else
	return;

    if(CUI_PurgeDeletions(name)!=0)
	flagError();
}

/* ----------------------------------------------------------------
 */

VOID robin_CloneMessage(code,cuid)
int code;
int cuid;
{
    char    buf[AMS_SNAPSHOTSIZE];
    bool    deletable;

    if(noFolder())
	return;

    if(CUI_GetSnapshotFromCUID(cuid,buf)!=0)
	return;

    deletable=AMS_GET_ATTRIBUTE(buf,AMS_ATT_MAYMODIFY);
    if(!deletable)
	switch(code){
	  case MS_CLONE_COPYDEL:
	  case MS_CLONE_MOVE:
	    code=MS_CLONE_COPY; break;
	  case MS_CLONE_APPENDDEL:
	    code=MS_CLONE_APPEND; break;
	}

    if(CUI_CloneMessage(cuid,currentFolder,code)==0){
	char newbuf[AMS_SNAPSHOTSIZE];
        if(CUI_GetSnapshotFromCUID(cuid,newbuf)==0 &&
	   bcmp(buf,newbuf,AMS_SNAPSHOTSIZE)!=0)
	    sendAlteredCaption(cuid,buildCaption(newbuf,FALSE));
    }else
	flagError();
}

VOID robin_Init()
{
    extern char *expandSquiggle();
    char GlobalFAliasFile[MAXFILENAME];
    char GlobalFakeDomFile[MAXFILENAME];
    char *tname;

    unscribe_init();
    headfilter_Init(TRUE,"from:date:subject:to");
    folderUpdateTable=hash_New(free);

    tname = (char *)getenv("BATCAVE");
    if (tname == NULL || *tname == '\0')
	tname = (char *)AndrewDir("");
    strcpy (GlobalFAliasFile, tname);
    strcat (GlobalFAliasFile, "/etc/folderaliases");
    strcpy (GlobalFakeDomFile, tname);
    strcat (GlobalFakeDomFile, "/etc/fakedomains");

    falias_Read(GlobalFAliasFile);
    falias_Read(expandSquiggle(faliasFile,MAXFILENAME));

    fakedom_Read(GlobalFakeDomFile);
    fakedom_Read(expandSquiggle(fakedomFile,MAXFILENAME));

    death_Read(expandSquiggle(killFile,MAXFILENAME));
}

VOID robin_Update()
{
    int discard=death_Update(killFile);
    if(discard>0)
	info("%d kill specs discarded",discard);
    mserrcode=MS_UpdateState();
    if(mserrcode!=0)
	mserror("Error updating state");
}

/* ----------------------------------------------------------------
 * end.
 */
