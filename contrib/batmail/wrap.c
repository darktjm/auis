/*
 * 20 April 1987 - Miles Bader
 * Last edit by Miles Bader (bader) on Thu, 30 Apr 1987 - 12:07am
 */

#include <stdio.h>
#include <com.h>

#define MAXWIDTH    200

#define TABSIZE	    8

static char tobuf[MAXWIDTH], *to;

static FILE *outfp=NULL;

static int  maxLineWidth=MAXWIDTH;


flush()
{
    *to='\0';
    fputs(tobuf,outfp);
    putc('\n',outfp);
    to=tobuf;
}

wrap_Begin(fp,width)
FILE	*fp;
int	width;
{
    outfp=fp;
    maxLineWidth=width;
    to=tobuf;
}

#define EMIT(c) \
 do{\
    if(to-tobuf>=maxLineWidth)\
	{\
	    char lastc=to[-1];\
	    to[-1]='\\';\
	    flush();\
	    *to++=lastc;\
	}\
	*to++=c;\
 }while(0)


wrap_Filter(buf,size)
char	*buf;
int	size;
{
    while(size-->0){
	char	c= *buf++;

	switch(c){
	    case '\n':
		flush();
		break;
	    case '\t':
		{
		    int	i=TABSIZE-(to-tobuf)%TABSIZE;
		    while(i-->0)
			EMIT(' ');
		}
		break;
	    default:
		EMIT(c);
	}
    }
}

wrap_End()
{
    flush();
}
