/*
 * 20 April 1987 - Miles Bader
 * Last edit by Miles Bader (bader) on Thu,  4 Jun 1987 -  7:24pm
 */

#include <stdio.h>
#include <ctype.h>

#include "com.h"
#include "hash.h"

#define MAXWIDTH    21000

#define BOGUS	    (-1)

static char tobuf[MAXWIDTH], *to;

static FILE *outfp=NULL;

static int  maxLineWidth=MAXWIDTH;

static bool stripThis=TRUE,stripHeader=FALSE,foundHeader=FALSE,defaultStrip=TRUE;

HASHTABLE   *stripTable;

static int dummy;

void headfilter_Begin(fp,width,strip)
FILE	*fp;
bool	strip;
{
    to=tobuf;
    outfp=fp;
    stripThis=strip;
    maxLineWidth=width;
    stripHeader=FALSE;
    foundHeader=FALSE;
}

static void flush()
{
    if(!stripHeader){
	*to='\0';
	fputs(tobuf,outfp);

	putc('\n',outfp);
    }
    to=tobuf;
}

char	*headfilter_Filter(buf,size)
char	*buf;
int	size;
{
    register char   c=BOGUS;

    while(size-->0){
	static char prevc=BOGUS;

	c= *buf++;

	switch (c) {
	    case '\n':
		flush();
		if(prevc=='\n'){
		    putc('\n',outfp);
		    return buf;
		}
		break;
	    case ':':
		if(!foundHeader){
		    *to='\0';
		    foundHeader=TRUE;
		    if(stripThis && (hash_Find(stripTable,tobuf)==NULL) == defaultStrip)
			stripHeader=TRUE;
		}
		/* fall through */
	    default:
		if(prevc=='\n'){
		    foundHeader=FALSE;
		    stripHeader=FALSE;
		}
		/* fall through */
	    case ' ':
	    case '\t':
		if(!stripHeader){
		    if(to-tobuf>=maxLineWidth){
			char	lastc=to[-1];	    /* wrap if whole line
						       doesn't fit */
			to[-1]='\\';
			flush();
			*to++='\t';
			*to++=lastc;
		    }
		    *to++=c;
		}
	}

	prevc=c;
    }

    return buf;
}

void headfilter_End()
{
    flush();
}

void headfilter_Init(bad,hlist)
bool	bad;
char	*hlist;
{
    char *thl;

    thl = (char *)malloc(strlen(hlist)+1);
    strcpy(thl, hlist);

    stripTable=hash_New(NULL);

    defaultStrip=bad;

    while(thl!=NULL && *thl!='\0'){
	char	*split(),*next=split(thl,':');
	hash_Install(stripTable,thl,(void *)&dummy);
	thl=next;
    }

    free(thl);
}
