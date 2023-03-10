/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

/*
 * Merges a local help alias file (which need not be sorted) into
 * a global help alias file (which must be sorted).
 * Sorting is done only on the first word (words are separated by any
 * whitespace).
 * Lines starting with whitespace or '#' are ignored.
 */

#include <andrewos.h>
#include <stdio.h>
#include <ctype.h>
#include <errno.h>
#include <stdarg.h>

#define MAXLOCKTRIES 	15
#define TEXTALLOCSTEP	5000
#define LINESALLOCSTEP	200

static const char *prog;

static void err(const char *f,...)
{
    va_list ap;
    va_start(ap, f);
    char buf[1000];
    vsprintf(buf,f,ap);
    strcat(buf,"\n");
    write(2,buf,strlen(buf));
    exit(10);
}

static void syserr(const char *f,...)
{
    char buf[1000];
    va_list ap;

    va_start(ap, f);
    vsprintf(buf,f,ap);

    if(errno!=0)
	perror(buf);
    else{
	strcat(buf,"\n");
	write(2,buf,strlen(buf));
    }

    exit(9);
}

static int qsLineCompare(const void *l1,const void *l2)
{
    return strcmp(*(const char **)l1,*(const char **)l2);
}

static char **readFromLines(const char *fromFile,int *numP)
{
    FILE *fp;
    int fromLinesLen=0, fromLinesMax=LINESALLOCSTEP;
    char **fromLines=(char **)malloc(sizeof(char *)*fromLinesMax);
    int fromTextLen=0, fromTextMax=TEXTALLOCSTEP;
    char *fromText=malloc(fromTextMax);

    fp=fopen(fromFile,"r");

    if(fp==NULL)
	syserr("%s: Couldn't open %s",prog,fromFile,0);
    else{
	char buf[1000];

	while(fgets(buf,sizeof(buf),fp)!=NULL){
	    int lineSize=strlen(buf)+1;

	    if(!isgraph(*buf) || *buf=='#')
		continue;	/* not a valid line */

	    if(fromTextLen+lineSize>fromTextMax){
		fromTextMax+=lineSize+TEXTALLOCSTEP;
		fromText=realloc(fromText,fromTextMax);
		if(fromText==NULL)
		    err("%s: Out of memory",prog,0);
	    }

	    if(fromLinesLen+1>fromLinesMax){
		fromLinesMax+=1+LINESALLOCSTEP;
		fromLines=(char **)realloc(fromLines,sizeof(char *)*fromLinesMax);
		if(fromLines==NULL)
		    err("%s: Out of memory",prog,0);
	    }

	    fromLines[fromLinesLen++]=fromText+fromTextLen;
	    
	    strcpy(fromText+fromTextLen,buf);
	    fromTextLen+=lineSize;
	}
    }

    fclose(fp);

    qsort(fromLines,fromLinesLen,sizeof(char *),qsLineCompare);

    *numP=fromLinesLen;

    return fromLines;
}

static int strcmpFirstField(const char *s1,const char *s2)
{
    while(*s1==*s2 && isgraph(*s1))
	s1++, s2++;

    if(!isgraph(*s1))
	return (isgraph(*s2) ? -1 : 0);
    else if(!isgraph(*s2))
	return (isgraph(*s1) ? 1 : 0);
    else
	return *s1-*s2;
}

int main(int argc,const char * const *argv)
{
    int fd;
    FILE *newFp, *oldFp;
    char **fromLines;
    int lockTries;
    int fromLinesLeft;
    char newFile[1000];
    char buf[3000];

    prog=argv[0];
#define fromFile argv[1]
#define intoFile argv[2]

    if(argc!=3)
	err("Usage: %s from into",prog,0);

    fromLines=readFromLines(fromFile,&fromLinesLeft);

    for (lockTries=0; lockTries<MAXLOCKTRIES; lockTries++){
	int rc;

	fd=open(intoFile,osi_O_READLOCK);
	if(fd<0)
	    syserr("%s: Couldn't open %s",prog,intoFile,0);
	rc=osi_ExclusiveLockNoBlock(fd);
	if(rc==0)
	    break;
	else if(rc!=EWOULDBLOCK)
	    syserr("%s: Couldn't lock %s",prog,intoFile,0);
	close(fd);
    }

    if(lockTries>=MAXLOCKTRIES)
	err("%s: Couldn't lock %s",prog,intoFile);

    oldFp=fdopen(fd,"r");

    strcpy(newFile,intoFile);
#ifdef USESHORTFILENAMES
    strcat(newFile,"+");
#else
    strcat(newFile,".NEW");
#endif
    newFp=fopen(newFile,"w");
    if(newFp==NULL)
	syserr("%s: Couldn't open %s for writing",prog,newFile,0);

    while(fgets(buf,sizeof(buf),oldFp)!=NULL){
	if(fromLinesLeft==0 || !isgraph(*buf) || *buf=='#')
	    fputs(buf,newFp);
	else{
	    int cmp=strcmpFirstField(buf,*fromLines);

	    if(cmp==0)		/* replace old entry */
		fputs(*fromLines++,newFp), fromLinesLeft--;
	    else if(cmp<0)	/* insertion not needed yet */
		fputs(buf,newFp);
	    else{		/* insert n new lines before old entry */
		do{
		    fputs(*fromLines++,newFp), fromLinesLeft--;
		}while(fromLinesLeft>0 && strcmpFirstField(buf,*fromLines)>0);
		fputs(buf,newFp);
	    }
	}
    }

    while(fromLinesLeft-->0)
	fputs(*fromLines++,newFp);

    if(fclose(newFp)==EOF)
	syserr("%s: Couldn't close %s",prog,newFile,0);

    if(rename(newFile,intoFile)!=0)
	syserr("%s: Couldn't rename %s as %s",prog,newFile,intoFile);

    fclose(oldFp);

    exit(0);
}
