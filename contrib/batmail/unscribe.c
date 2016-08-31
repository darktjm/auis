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
#include <stdio.h>
#include <ctype.h>
#include "com.h"
#include "hash.h"

#define MAXNESTING  100
#define MAXWIDTH    200

#define INDENTSIZE  4
#define TABSIZE	    8

#define BOGUS (-1)

static char tobuf[MAXWIDTH], *to=tobuf, *wrapAt=NULL;
static char leaderBuf[MAXWIDTH], *leader=leaderBuf;
static char cmdBuf[100], *cmd=cmdBuf;
static char delimStack[MAXNESTING], *delim=delimStack, envDelim=BOGUS;

static int  maxLineWidth=MAXWIDTH;
static int  centered=0;

static FILE *outfp=NULL;

static bool inCmd=FALSE;
static char emphasis=' ';

static docmd(/* char *str */);
static undocmd();
static emphasize(/* bool entering, char c */);


#define linewidth (to-tobuf+leader-leaderBuf)

#define NOSEMI(cmd) do cmd while(0)

#define WRAPHERE() (wrapAt=to)
#define BOLP() (to==tobuf)
#define EMIT(c) \
 NOSEMI({*to++=c;if(linewidth>=maxLineWidth)wrapNewline();})

#if !defined(mips)
#define EMITNODUP(c) \
 (to>tobuf ? (int)((*to=c)==(*(to-1)) ? to-- : to++) : (int)(*to++=c))
#else	/* the mips c-compiler sucks pretty badly */
#define EMITNODUP(c) EMIT(c)
#endif

#define PARSECMD() (inCmd=TRUE, cmd=cmdBuf)
#define BEGINCMD(endDelim) \
 (inCmd=FALSE, *cmd='\0', *delim++=envDelim, envDelim=endDelim, docmd(cmdBuf))
#define ENDCMD() (inCmd=FALSE, envDelim= *--delim, undocmd())

static char	*center(buf,end,width)
char	*buf,*end;
int	width;
{
    char    *beg=buf,*last;
    int	    move;

    while(beg<end && isspace(*beg))
	beg++;

    move=(width-(end-beg))/2;
    last=end+move;
    while(beg<=end){
	*(end+move)= *end;
	end--;
    }

    while(move-->0)
	*buf++=' ';

    return last;
}

static bool eatNewline=FALSE;

#define freshline() if(!BOLP())newline(),eatNewline=TRUE

static newline()
{
    if(eatNewline){
	eatNewline=FALSE;
	if(BOLP())
	    return;
    }

    *leader='\0';

    while(to>tobuf && isspace(to[-1]))
	to--;

    if(centered>0)
	to=center(tobuf,to,maxLineWidth-(leader-leaderBuf));

    *to='\0';
    fputs(leaderBuf,outfp);
    fputs(tobuf,outfp);

    to=tobuf;

    if(emphasis!=' '){
	putc(emphasis,outfp);
	*to++=emphasis;
    }

    putc('\n',outfp);

    wrapAt=NULL;
}

static wrapNewline()
{
    char    *whiteEnd=wrapAt;
    if(wrapAt!=NULL){
	while(wrapAt>tobuf && isspace(wrapAt[-1]))
	    wrapAt--;
	*wrapAt='\0';
	newline();
	while(*++whiteEnd)
	    *to++= *whiteEnd;
    }else{
	char	lastc=to[-1];

	to[-1]='\\';
	newline();
	*to++=lastc;
    }
}

unscribe_begin(tofp,mlw)
FILE	*tofp;
int	mlw;
{
    outfp=tofp;
    to=tobuf;
    leader=leaderBuf;
    emphasis=' ';
    maxLineWidth=mlw;
}

unscribe_end()
{
    freshline();
}

unscribe_filter(frombuf,fromSize)
char	*frombuf;
long	fromSize;
{
    int	    i;
    int   c=BOGUS;

    while(fromSize-->0){
	static char prevc=BOGUS;

	c= *frombuf++;
	switch(c){
	    case '@':
		if(prevc=='@'){
		    EMIT(c);
		    c=BOGUS;		/* prevent interpretation of @ */
		}
		break;
	    case '\n':
		if(prevc=='\n')
		    newline();
		else{
		    WRAPHERE();
		    if(!BOLP() && isprint(to[-1]))
		     /* EMITNODUP(emphasis); Use this for underlines in spaces */
			EMIT(' ');
		}
		break;
	    case '\t':
		WRAPHERE();
		i=TABSIZE-linewidth%TABSIZE;
		while(i-->0)
		    EMIT(' ');
		 /* EMITNODUP(emphasis); Use this for underlines in spaces */
		break;
	    case ' ':
		WRAPHERE();
		EMIT(' ');
	     /* EMIT(emphasis); Use this for underlines in spaces */
		break;
	    default:
		if(c==envDelim)
		    ENDCMD();
		else if(inCmd)
		    if(isalnum(c))
			*cmd++=c;
		    else{
			char	endDelim;
			switch(c){
			    case '(':
				endDelim=')'; break;
			    case '[':
				endDelim=']'; break;
			    case '<':
				endDelim='>'; break;
			    case '{':
				endDelim='}'; break;
			    default:
				endDelim=c;
			}

			BEGINCMD(endDelim);
		    }
		else if(prevc=='@'){
		    PARSECMD();
		    *cmd++=c;
		}else
		    EMIT(c);
	}

	prevc=c;
    }
}

static HASHTABLE    *cmdTable;

static VOID	(*cmdStack[MAXNESTING])(), (**curCmd)()=cmdStack;

static docmd(str)
char	*str;
{
    VOID (*f)()=(VOID (*)())hash_Find(cmdTable,str);

    if(f!=NULL)
	(*f)(TRUE);

    *curCmd++=f;
}

static undocmd()
{
    if(curCmd>cmdStack){
	VOID    (*f)()= *--curCmd;
	if(f!=NULL)
	    (*f)(FALSE);
    }
}

static quotationCmd(entering)
bool	entering;
{
    freshline();
    if(entering){
	if(leader>=leaderBuf+2 && leader[-1]==' ' && leader[-2]=='>')
	    leader--;
	*leader++='>';
	*leader++=' ';
    }else{
	leader-=2;
	if(leader>leaderBuf && leader[-1]=='>')
	    *leader++=' ';
    }
}

static indentCmd(entering)
bool	entering;
{
    int i;

    freshline();
    if(entering)
	for(i=0;i<INDENTSIZE;i++)
	    *leader++=' ';
    else
	leader-=INDENTSIZE;
}

static boldCmd(entering)
bool	entering;
{
    EMITNODUP('*');
    emphasize(entering,'*');
}

static italicCmd(entering)
bool	entering;
{
    EMITNODUP('_');
    emphasize(entering,'_');
}

static beginCmd(entering)
bool	entering;
{
    if(entering)
	PARSECMD();
    else
	BEGINCMD(BOGUS);
}

static endCmd(entering)
bool	entering;
{
    if(entering)
	PARSECMD();
    else
	ENDCMD();
}

static centerCmd(entering)
bool	entering;
{
    freshline();
    if(entering)
	centered++;
    else
	centered--;
}

static mjrHdngCmd(entering)
bool	entering;
{
    boldCmd(entering);
    centerCmd(entering);
}

static emphasize(entering,c)
bool	entering;
char	c;
{
    static char	prevSpace[MAXNESTING], *ps=prevSpace;

    if(entering){
	*ps++=emphasis;
	emphasis=c;
    }else
	emphasis= *--ps;
}

unscribe_init()
{
    cmdTable=hash_New(NULL);
    hash_Install(cmdTable,"begin",beginCmd);
    hash_Install(cmdTable,"end",endCmd);
    hash_Install(cmdTable,"quotation",quotationCmd);
    hash_Install(cmdTable,"indent",indentCmd);
    hash_Install(cmdTable,"description",indentCmd);
    hash_Install(cmdTable,"itemize",indentCmd);
    hash_Install(cmdTable,"enumerate",indentCmd);
    hash_Install(cmdTable,"example",indentCmd);
    hash_Install(cmdTable,"bold",boldCmd);
    hash_Install(cmdTable,"b",boldCmd);
    hash_Install(cmdTable,"p",boldCmd);		    /* what fonts we have! */
    hash_Install(cmdTable,"italic",italicCmd);
    hash_Install(cmdTable,"i",italicCmd);
    hash_Install(cmdTable,"majorheading",mjrHdngCmd);
    hash_Install(cmdTable,"center",centerCmd);
}
