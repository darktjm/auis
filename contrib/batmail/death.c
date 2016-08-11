/* ***************************************************************** *\
 *	Copyright 1988-1990 by Miles Bader
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
static char rcsid[]="$Header: /afs/cs.cmu.edu/project/atk-src-C++/contrib/batmail/RCS/death.c,v 1.6 1996/03/06 17:30:49 susan Stab74 $";
#endif

/*
 * death.c -- message killing code for batmail
 * Copyright 1988,1989,1990 by Miles Bader
 */

#include <andrewos.h>	/* is included anyway by cui */
#include <ctype.h>
/* #include <stdio.h> 
 *  #include <sys/types.h>
 *  #include <sys/stat.h>
 *  #include <sys/time.h>
 */

#include <ams/ams.h>
/* kill the ams group */
#undef fopen
#undef fclose
#undef fprintf

#include <regexp.h>
extern void (*reg_errhandler())();

#include "com.h"
#include "robin.h"
#include "falias.h"

#include "death.h"
#include "hshbuf.h"

static struct deathFolder {
    struct hshbuf *name;
    struct killSpec *fields;
    struct killSpec **endP;	/* pointer to end of list */
    struct deathFolder *next;
    struct deathFolder *nextSearch; /* to be searched after this one */
} *folders=NULL, *curFolder=NULL;

static struct hshbuf *curFolderName=NULL;

int somethingChanged=FALSE;

void death_SetFolder(folder)
char *folder;
{
    struct deathFolder *f;

    if(folder==NULL)
	curFolder=folders;
    else{
	hshbuf_FILL(curFolderName,folder,strlen(folder));
    
	for(f=folders; f!=NULL; f=f->next){
	    struct hshbuf *hb=f->name;
	    if(hb!=NULL && hshbuf_EQ(hb,curFolderName))
		break;
	}

	curFolder=f;
    }
}

#define DOPEYDOTS 2
#define MAXUSEABLESUBJ (MAXSUBJ-DOPEYDOTS-4)

static void canonSubjBuf(bufP,lenP)
char **bufP;
int *lenP;
{
    char *buf=(*bufP);
    int len=(*lenP);

    while(isspace(*buf))
	buf++, len--;

    while((buf[0]=='R' || buf[0]=='r') &&
	  (buf[1]=='e' || buf[1]=='E') &&
	  (buf[2]==':' || (buf[2]=='^' && buf[4]==':')))
    {
	while(*buf!=':')
	    buf++, len--;

	buf++, len--;

	while(isspace(*buf))
	    buf++, len--;
    }	    

    /* some dopey mailers (athena) append spaces to subj lines
     * when replying
     */
    while(len>0 && isspace(buf[len-1]))
	len--;
    
    if(len>MAXUSEABLESUBJ)
	len=MAXUSEABLESUBJ;

    *bufP=buf;
    *lenP=len;
}

bool findSubjField(caption,bufP,lenP,canon)
char *caption;
char **bufP;
int *lenP;
bool canon;
{
    char *subj=strchr(caption,'\t');
    int subjLen;
    
    if(subj==NULL)
	return FALSE;
    else{
	char *nxf=strchr(++subj,'\t');
	if(nxf==NULL)
	    subjLen=strlen(subj);
	else
	    subjLen=nxf-subj;
    }
    
    *bufP=subj;
    *lenP=subjLen;

    if (canon && (subjLen > 0))
	canonSubjBuf(bufP,lenP);

    return TRUE;
}


/* returns a static buffer */
static struct hshbuf *getSubjHB(caption)
char *caption;
{
    char *buf;
    int len;
    static struct hshbuf *subjHB=NULL;
    
    if(findSubjField(caption,&buf,&len,TRUE)){
	hshbuf_FILL(subjHB,buf,len);
	return subjHB;
    }else
	return NULL;
}

bool findFromField(caption,bufP,lenP)
char *caption;
char **bufP;
int *lenP;
{
    char *from=strchr(caption,'\t');
    int fromLen;

    if(from!=NULL)
	from=strchr(from+1,'\t');

    if(from==NULL)
	return FALSE;
    else{
	char *nxf=strchr(++from,'\t');
	if(nxf==NULL)
	    fromLen=strlen(from);
	else
	    fromLen=nxf-from;
	
	if(from[fromLen-1]==')'){
	    nxf=strrchr(from,'(');
	    if(nxf!=NULL)
		fromLen=nxf-from-1;
	}
    }

    *bufP=from;
    *lenP=fromLen;

    return TRUE;
}

/* returns a static buffer */
static struct hshbuf *getFromHB(caption)
char *caption;
{
    char *buf;
    int len;
    static struct hshbuf *fromHB=NULL;
    
    if(findFromField(caption,&buf,&len)){
	hshbuf_FILL(fromHB,buf,len);
	return fromHB;
    }else
	return NULL;
}

static char *getSubjString(caption)
char *caption;
{
    char *subj;
    static char buf[MAXCAPTION];
    int len;

    if(findSubjField(caption,&subj,&len,FALSE)){
	memmove(buf,subj,len);
	buf[len]='\0';
	return buf;
    }else
	return "";
}

static char *getFromString(caption)
char *caption;
{
    char *from;
    static char buf[MAXCAPTION];
    int len;

    if(findFromField(caption,&from,&len)){
	memmove(buf,from,len);
	buf[len]='\0';
	return buf;
    }else
	return "";
}

static void timeStamp(kf)
struct killSpec *kf;
{
    if(kf->timestamp>0){
	kf->timestamp=0;
	somethingChanged=TRUE;
    }
}

bool death_MsgKilledBy(cuid,ss,kf)
int cuid;
char *ss;
struct killSpec *kf;
{
    char *caption=AMS_CAPTION(ss);

    if(!kf->delete)
	switch(kf->method){
	    case death_HASHED:
		switch(kf->field){
		    case death_SUBJ:
			if(hshbuf_EQ(getSubjHB(caption),kf->contents)){
			    timeStamp(kf);
			    return TRUE;
			}
			break;
		    case death_FROM:
			if(hshbuf_EQ(getFromHB(caption),kf->contents)){
			    timeStamp(kf);
			    return TRUE;
			}
			break;
		}
		break;
		
	    case death_REGEXP:
		switch(kf->field){
		    case death_SUBJ:
			if(reg_exec(kf->regexp,getSubjString(caption))){
			    timeStamp(kf);
			    return TRUE;
			}
			break;
		    case death_FROM:
			if(reg_exec(kf->regexp,getFromString(caption))){
			    timeStamp(kf);
			    return TRUE;
			}
			break;
		}		
		break;
	}

    return FALSE;
}

bool death_MsgKilled(cuid,ss)
int cuid;
char *ss;
{
    struct deathFolder *f;
    struct killSpec *kf;
    char *caption=AMS_CAPTION(ss);
    struct hshbuf *subjHB=NULL,*fromHB=NULL;
    char *subj=NULL, *from=NULL;

    /* largely the same as MsgKilledBy, but we can cache more things */

    for(f=(curFolder==NULL ? folders : curFolder); f!=NULL; f=f->nextSearch)
	for(kf=f->fields; kf!=NULL; kf=kf->next)
	    if(!kf->delete)
		switch(kf->method){
		    case death_HASHED:
			switch(kf->field){
			    case death_SUBJ:
				if(subjHB==NULL)
				    subjHB=getSubjHB(caption);
				if(hshbuf_EQ(subjHB,kf->contents)){
				    timeStamp(kf);
				    return TRUE;
				}
				break;
			    case death_FROM:
				if(fromHB==NULL)
				    fromHB=getFromHB(caption);
				if(hshbuf_EQ(fromHB,kf->contents)){
				    timeStamp(kf);
				    return TRUE;
				}
				break;
			}
			break;
			
		    case death_REGEXP:
			switch(kf->field){
			    case death_SUBJ:
				if(subj==NULL)
				    subj=getSubjString(caption);
				if(reg_exec(kf->regexp,subj)){
				    timeStamp(kf);
				    return TRUE;
				}
				break;
			    case death_FROM:
				if(from==NULL)
				    from=getFromString(caption);
				if(reg_exec(kf->regexp,from)){
				    timeStamp(kf);
				    return TRUE;
				}
				break;
			}		
		}

    return FALSE;
}

static void freeKill(kf)
struct killSpec *kf;
{
    if(kf->regexpSrc!=NULL)
	free(kf->regexpSrc);
    if(kf->regexp!=NULL)
	FREE(kf->regexp);
    if(kf->contents!=NULL)
	FREE(kf->contents);
    FREE(kf);
}    

static struct deathFolder *createFolder(name)
struct hshbuf *name;
{
    struct deathFolder *f;
    
    if(folders==NULL){
	/* first folder is "global" folder */
	
	folders=NEW(struct deathFolder);
	folders->name=NULL;
	folders->fields=NULL;
	folders->endP=(&folders->fields);
	folders->next=NULL;
	folders->nextSearch=NULL;
    }
    
    if(name==NULL || hshbuf_Len(name)==0)
	f=folders;	/* global folder */
    else{
	struct deathFolder **fP=(&folders);
	
	/* put at end of folders */
	while(*fP!=NULL)
	    fP=(&(*fP)->next);
	
	f=NEW(struct deathFolder);
	f->name=hshbuf_Copy(name);
	f->fields=NULL;
	f->endP=(&f->fields);
	f->next=NULL; /* its at the end, remember? */
	f->nextSearch=folders; /* global folder */
	
	*fP=f;
    }

    return f;
}

static void death_regerr(msg)
char *msg;
{
    error("Bad regular expression: %s", msg);
}

static struct killSpec *addKill(buf,len,field,method,ts,saved)
char *buf;
int len;
int field,method;
long ts;
bool saved;
{
    struct killSpec *kf;
    static struct hshbuf *hb=NULL;
    void (*oldregerr)();

    switch(method){
      case death_HASHED:
	hshbuf_FILL(hb,buf,len);
	break;
    }

    if(curFolder==NULL)
	curFolder=createFolder(curFolderName);

    /* check for duplicates */
    for(kf=curFolder->fields; kf!=NULL; kf=kf->next)
	if(method==kf->method)
	    if((method==death_HASHED && hshbuf_EQ(hb,kf->contents)) ||
	       (method==death_REGEXP && bcmp(buf,kf->regexpSrc,len)==0))
	    {
		kf->delete=FALSE; /* new lease on life */
		if(saved) {
		    kf->saved=TRUE;
		}
		else {
		    info("Already killed");
		}
		return NULL;
	    }

    if(!saved)
	somethingChanged=TRUE;

    kf=NEW(struct killSpec);
    kf->field=field;
    kf->method=method;
    kf->next=(*curFolder->endP);
    kf->timestamp=ts;
    kf->delete=FALSE;
    kf->saved=saved;		/* unless otherwise stated */

    switch(method){
      case death_HASHED:
	kf->contents=hshbuf_Copy(hb);
	kf->regexp=NULL;
	kf->regexpSrc=NULL;
	break;
      case death_REGEXP:
	kf->regexpSrc=(char *)malloc(len+1);
	memmove(kf->regexpSrc,buf,len);
	kf->regexpSrc[len]='\0';

	oldregerr = reg_errhandler(death_regerr);
	kf->regexp=(struct regexp *)reg_comp(kf->regexpSrc);
	(void) reg_errhandler(oldregerr);
	if (!kf->regexp) {
	    FREE(kf);
	    return 0;
	}

	kf->contents=NULL;
	break;
    }

    *curFolder->endP=kf;
    curFolder->endP=(&kf->next);

    return kf;
}

static struct killSpec *removeKill(buf,len,field,method)
char *buf;
int len;
int field,method;
{
    struct killSpec *kf;
    static struct hshbuf *hb=NULL;

    switch(method){
      case death_HASHED:
	hshbuf_FILL(hb,buf,len);
	break;
    }

    if(curFolder!=NULL)
	for(kf=curFolder->fields; kf!=NULL; kf=kf->next)
	    if(method==kf->method)
		if((method==death_HASHED && hshbuf_EQ(hb,kf->contents)) ||
		   (method==death_REGEXP && bcmp(buf,kf->regexpSrc,len)==0))
		{
		    kf->delete=FALSE; /* new lease on life */
		    return kf;
		}

    return NULL;
}

struct killSpec *death_KillMsg(cuid,ss,field)
int cuid;
char *ss;
int field;
{
    char *buf;
    int len;

    switch(field){
      case death_SUBJ:
	findSubjField(AMS_CAPTION(ss),&buf,&len,TRUE); break;
      case death_FROM:
	findFromField(AMS_CAPTION(ss),&buf,&len); break;
    }

    return addKill(buf,len,field,death_HASHED,0,FALSE);
}

struct killSpec *death_UnkillMsg(cuid,ss,field)
int cuid;
char *ss;
int field;
{
    char *buf;
    int len;

    switch(field){
      case death_SUBJ:
	findSubjField(AMS_CAPTION(ss),&buf,&len,TRUE); break;
      case death_FROM:
	findFromField(AMS_CAPTION(ss),&buf,&len); break;
    }

    return removeKill(buf,len,field,death_HASHED);
}

struct killSpec *death_KillBuf(buf,len,field,meth,to)
char *buf;
int len, field, meth;
{
    return addKill(buf,len,field,meth,(to?0:-1),FALSE);
}

struct killSpec *death_UnkillBuf(buf,len,field,meth)
char *buf;
int len, field, meth;
{
    return removeKill(buf,len,field,meth);
}

void scanWhite(pP)
char **pP;
{
    while(isspace(**pP))
	(*pP)++;
}

int scanChar(pP)
char **pP;
{
    int ch=(**pP);
    if(ch!='\0'){
	(*pP)++;
	scanWhite(pP);
	return ch;
    }else
	return -1;
}    

int scanInt(pP)
char **pP;
{
    int val=0,sign=1;
    if(**pP=='-')
	sign=(-1);
    while(isdigit(**pP))
	val=(val*10)+(*(*pP)++-'0');
    scanWhite(pP);
    return sign*val;
}

int scanDelim(pP,delim)
char **pP;
int delim;
{
    int len=0;

    while(**pP!='\0'){
	if(**pP=='\\' && *(*pP+1)==delim){
	    /* ugh! shrink the string */
	    char *q=(*pP);
	    while(*++q!='\0')
		*(q-1)=(*q);
	}else if(**pP==delim){
	    (*pP)++;		/* skip the final delimiter */
	    scanWhite(pP);	/* and trailing ws */
	    break;
	}

	(*pP)++;
	len++;
    }

    return len;
}

static void scanKill(p)
char *p;
{
    long ts;
    char fieldCode,methCode;
    int field,meth;
    char *string;
    int length;
    
    scanWhite(&p);
    
    if(isdigit(*p)){
	/* obsolete format for backwards comptability */
	
	somethingChanged=TRUE; /* make sure we write out new format */

	ts=scanInt(&p);
	fieldCode=scanChar(&p);
	methCode='"';
	string=p;
	length=strlen(string);
    }else{
	fieldCode=scanChar(&p);
	methCode=(*p++);
	string=p;
	length=scanDelim(&p,methCode);
	if(*p=='('){
	    p++;
	    ts=scanInt(&p);
	}else
	    ts=(-1);
    }
    
    switch(fieldCode){
      case 's':
	field=death_SUBJ; break;
      case 'f':
	field=death_FROM; break;
    }
    
    switch(methCode){
      case '"':
	meth=death_HASHED; break;
      case '/':
	meth=death_REGEXP; break;
    }
    
    addKill(string,length,field,meth,ts,TRUE);
}

static void deathRead(fn)
char *fn;
{
    FILE *fp=fopen(fn,"r");
    char buf[200];

    if(fp==NULL){
	syserr("Couldn't open",fn);
	return;
    }

    curFolder=NULL;		/* so we don't put things in the wrong place */
    curFolderName=NULL;		/* ditto */

    while(fgets(buf,sizeof(buf),fp)!=NULL){
	char *nl=strchr(buf,'\n');

	if(nl!=NULL)
	    *nl='\0';

	switch(*buf){
	  case ':':
	    /* the following falias_Unalias should eventually be removed */
	    death_SetFolder(falias_Unalias(buf+1));
	    break;
	  case '\0':
	  case '#':
	    break;		/* comments */
	  default:
	    scanKill(buf);
	}
    }

    fclose(fp);
}

static void readUpdates(fn)
char *fn;
{
    struct deathFolder *f;
    struct killSpec *kf;
    static long lastTs=0;
    struct stat buf;

    if(stat(fn,&buf)<0){
	if(errno!=ENOENT)
	    syserr("Couldn't stat",fn);
	return;
    }

    if(lastTs==0)
	lastTs=buf.st_mtime;
    else if(lastTs>buf.st_mtime)
	return;

    /*
     * mark everything that's been written to disk already as deleted
     * so that anything not explicitly un-deleted from the file will
     * be deleted.
     */
    for(f=folders; f!=NULL; f=f->next)
	for(kf=f->fields; kf!=NULL; kf=kf->next)
	    if(kf->saved)
		kf->delete=TRUE;

    deathRead(fn);
}

static void putDelim(fp,str,len,delim)
FILE *fp;
char *str;
int len;
int delim;
{
    putc(delim,fp);
    while(len-->0){
	if(*str==delim)
	    putc('\\',fp);
	putc(*str++,fp);
    }
    putc(delim,fp);
}

static int writeKill(fp,kf,ts)
FILE *fp;
struct killSpec *kf;
long ts;
{
    long kts=kf->timestamp;

    if(kts==0)
	kts=ts;

    if(!kf->delete && kts<0 || ts-kts<MAXKILLAGE){
	char fieldCode, methCode;
	char *string;
	int length;

	switch(kf->field){
	  case death_SUBJ:
	    fieldCode='s'; break;
	  case death_FROM:
	    fieldCode='f'; break;
	}

	switch(kf->method){
	  case death_HASHED:
	    methCode='"';
	    string=hshbuf_Buf(kf->contents);
	    length=hshbuf_Len(kf->contents);
	    break;
	  case death_REGEXP:
	    methCode='/';
	    string=kf->regexpSrc;
	    length=strlen(string);
	    break;
	}
	
	putc(fieldCode,fp);
	putc(' ',fp);

	putDelim(fp,string,length,methCode);

	if(kts>=0)
	    fprintf(fp," (%d)",kts);

	putc('\n',fp);

	kf->saved=TRUE;

	return 0;
    }else
	return 1;
}

int death_Write(fn)
char *fn;
{
    FILE *fp;
    struct timeval tv;
    char buf[MAXFILENAME];
    struct deathFolder *f;
    int discard=0;

    gettimeofday(&tv,NULL);

    if(!lockFile(fn)){
	syserr("Couldn't lock",fn);
	return 0;
    }

    readUpdates(fn);		/* in case it's been written by anyone */

    strcpy(buf,fn);
    strcat(buf,".NEW");
    fp=fopen(fn,"w");
    if(fp==NULL){
	unlockFile(fn);
	return 0;
    }

    fputs("# Batmail kill file\n",fp);

    for(f=folders; f!=NULL; f=f->next){
	if(f->fields!=NULL){
	    struct killSpec **selfP=(&f->fields);
	    struct killSpec *kf=f->fields;

	    putc('\n',fp);
	    
	    if(f->name==NULL)
		fputs("# globally killed items\n",fp);
	    else{
		putc(':',fp);
		fwrite(hshbuf_Buf(f->name),hshbuf_Len(f->name),1,fp);
		putc('\n',fp);
	    }
	    
	    while(kf!=NULL){
		int punt=writeKill(fp,kf,tv.tv_sec);

		if(punt){
		    *selfP=kf->next;
		    freeKill(kf);
		}else
		    selfP=(&kf->next);

		kf=(*selfP);
		discard+=punt;
	    }
	}
    }

    fclose(fp);

    rename(buf,fn);

    unlockFile(fn);

    somethingChanged=FALSE;

    return discard;
}

int death_Update(fn)
char *fn;
{
    if(somethingChanged)
	return death_Write(fn);
    else{
	readUpdates(fn);
	return 0;
    }
}
