/* File rexxtext.C created by R S Kemmetmueller
   (c) Copyright IBM Corp.  1988-1995.  All rights reserved.

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

   rexxtext, an object for editing REXX code. */

#include <andrewos.h>

static UNUSED const char ibmid[] = "(c) Copyright IBM Corp.  1988-1995.  All rights reserved.";
static UNUSED const char rcsHeader[] = "$Header: /afs/cs.cmu.edu/project/atk-src-C++/atk/srctext/RCS/rexxtext.C,v 2.0 1995/01/27 19:38:38 rr2b Stab74 $";

#include <environment.H>
#include <stylesheet.H>
#include <style.H>
#include <fontdesc.H>
#include <attribs.h>
#include <toolcnt.h>

#include "srctext.H"
#include "rexxtext.H"

static Dict *words[TABLESIZE];

ATKdefineRegistry(rexxtext, srctext, rexxtext::InitializeClass);

boolean rexxtext::IsTokenChar(char ch)
{
    return (isalnum(ch) || ch=='_' || ch=='!' || ch=='?' || ch=='.');
}

/* rexxtext_Keywordify makes buff all uppercase.  checkforceupper and Force Upper status are ignored. */
/* This function "keywordifies" a string. "Keywordify" means "make the word all upper case so it will be found in the hash table". */
char *rexxtext::Keywordify(char *buff, boolean checkforceupper)
{
    if (buff!=NULL && strlen(buff)>0)
	makeupper(buff);
    return buff;
}

void rexxtext::SetAttributes(struct attributes *atts)
{
    (this)->srctext::SetAttributes(atts);
#if 0 /* ctext leftovers */
    while (atts!=NULL) {
	char *key=atts->key;
	if (strcmp(key,"brace-indent")==0)
	    this->braceIndent=atoi(atts->value.string);
	else if (strcmp(key,"switch-label-undent")==0)
	    this->switchLabelUndent=atoi(atts->value.string);
	else if (strcmp(key,"switch-level-indent")==0)
	    this->switchLevelIndent=atoi(atts->value.string);
	atts=atts->next;
    }
#endif /* ctext leftovers */
}

static void SetupStyles(rexxtext *self)
{
    if ((self->srctext::kindStyle[KEYWRD]= (self->text::styleSheet)->Find("keyword")) == NULL) {
	self->srctext::kindStyle[KEYWRD]= new style;
	(self->srctext::kindStyle[KEYWRD])->SetName("keyword");
	(self->srctext::kindStyle[KEYWRD])->AddNewFontFace(fontdesc_Bold);
	(self->text::styleSheet)->Add(self->srctext::kindStyle[KEYWRD]);
    }
}

void rexxtext::SetupStyles()
{
    (this)->srctext::SetupStyles();
    ::SetupStyles(this);
}

boolean rexxtext::InitializeClass()
{
    static Dict rexxkeywords[]={
	{"ADDRESS",0,KEYWRD},
	{"ARG",0,KEYWRD},
	{"BY",0,KEYWRD},
	{"CALL",0,KEYWRD},
	{"DIGITS",0,KEYWRD},
	{"DO",START_BIT+BEGIN_BIT,KEYWRD}, /* has a corresponding END */
	{"DROP",0,KEYWRD},
	{"ELSE",ELSE_BIT+BEGIN_BIT,KEYWRD}, /* must catch ELSEs when parsing back */
	{"END",END_BIT+BEGIN_BIT,KEYWRD}, /* marks the end of something started with START_BIT */
	{"ENGINEERING",0,KEYWRD},
	{"ERROR",0,KEYWRD},
	{"EXIT",0,KEYWRD},
	{"EXPOSE",0,KEYWRD},
	{"FAILURE",0,KEYWRD},
	{"FOR",0,KEYWRD},
	{"FOREVER",0,KEYWRD},
	{"FORM",0,KEYWRD},
	{"FUZZ",0,KEYWRD},
	{"HALT",0,KEYWRD},
	{"IF",IF_BIT+BEGIN_BIT,KEYWRD},
	{"INTERPRET",0,KEYWRD},
	{"ITERATE",0,KEYWRD},
	{"LEAVE",0,KEYWRD},
	{"LINEIN",0,KEYWRD},
	{"NAME",0,KEYWRD},
	{"NOP",0,KEYWRD},
	{"NOTREADY",0,KEYWRD},
	{"NOVALUE",0,KEYWRD},
	{"NUMERIC",0,KEYWRD},
	{"OFF",0,KEYWRD},
	{"ON",0,KEYWRD},
	{"OPTIONS",0,KEYWRD},
	{"OTHERWISE",OTHERWISE_BIT+BEGIN_BIT,KEYWRD},
	{"PARSE",0,KEYWRD},
	{"PROCEDURE",0,KEYWRD},
	{"PULL",0,KEYWRD},
	{"PUSH",0,KEYWRD},
	{"QUEUE",0,KEYWRD},
	{"RETURN",0,KEYWRD},
	{"SAY",0,KEYWRD},
	{"SCIENTIFIC",0,KEYWRD},
	{"SELECT",START_BIT+BEGIN_BIT,KEYWRD}, /* has a corresponding END */
	{"SIGNAL",0,KEYWRD},
	{"SOURCE",0,KEYWRD},
	{"SYNTAX",0,KEYWRD},
	{"THEN",THEN_BIT+BEGIN_BIT,KEYWRD},
	{"TO",0,KEYWRD},
	{"TRACE",0,KEYWRD},
	{"UNTIL",0,KEYWRD},
	{"UPPER",0,KEYWRD},
	{"VALUE",0,KEYWRD},
	{"VAR",0,KEYWRD},
	{"VERSION",0,KEYWRD},
	{"WHEN",WHEN_BIT+BEGIN_BIT,KEYWRD},
	{"WHILE",0,KEYWRD},
	{"WITH",0,KEYWRD},
	{NULL,0,0}
    };
    srctext::BuildTable("rexxtext",::words,rexxkeywords);
    return TRUE;
}

rexxtext::rexxtext()
{
    ATKinit;
    this->srctext::words= (Dict **)::words;
    this->srctext::commentString= "/*  */";
    this->srctext::commentIndent= 3;
    this->srctext::levelIndent= 4;
    this->srctext::indentingEnabled= TRUE;
    ::SetupStyles(this);
    ToolCount("EditViews-rexxtext",NULL);
    THROWONFAILURE(TRUE);
}

/* unstyledCommentStart is a fallback for places where we KNOW we're at the end of a comment, but InCommentStart returns zero becuse there was no style there. */
static long unstyledCommentStart(rexxtext *self, long pos)
{
    int prev, c=0;
    while (pos>0) {
	prev= c;
	c= (self)->GetChar(--pos);
	if (c=='/' && prev=='*')
	    /* found a slash-star. It COULD be *inside* the actual comment, but we'll guess that it's the *opening* slash-star. */
	    return pos;
	else if (c=='*' && prev=='/')
	    /* whoah! found ANOTHER end-of-comment */
	    return 0;
    }
    return 0;
}

static long backwardSkipJunk(rexxtext *self, long pos, boolean skipNewLine)
{
    while (pos>=0) {
	register int c=(self)->GetChar(pos--);

	switch(c) {
	    case '/':
		if ((self)->GetChar(pos)=='*') {
		    long startcomment;
		    if ((startcomment=(self)->InCommentStart(pos)) > 0 || (startcomment=unstyledCommentStart(self,pos)) > 0)
			pos= startcomment-1;
		}
		break;
	    case '\n':
		if (!skipNewLine)
		    return pos + 1;
	    case ' ':		
	    case '\t':
		/* nothing */
		break;
	    default:
		return pos+1;
	}
    }
    return -1;
}

/* assumes ending delimiter AFTER pos */
static long backwardSkipDelimited(rexxtext *self, long pos, char beg, char end)
{
    int level=1;
    while(pos>=0 && level>0){
	int c;
	switch(c=(self)->GetChar(pos--)){
	    case '\'':
	    case '"':
		pos=(self)->BackwardSkipString(pos,c);
		break;
	    case '/':
		if ((self)->GetChar(pos)=='*') {
		    long startcomment;
		    if ((startcomment=(self)->InCommentStart(pos)) > 0 || (startcomment=unstyledCommentStart(self,pos)) > 0)
			pos= startcomment-1;
		}
		break;
	    default:
		if(c==beg)
		    --level;
		else if(c==end)
		    ++level;
	}
    }
    return pos;
}

static void checkfunction(rexxtext *self, long pos)
{
    char buff[256];
    long newpos;
    Dict *word;

    pos = backwardSkipJunk(self, pos, FALSE);
    if ((self)->GetChar(pos--) == ':') {
	newpos = backwardSkipJunk(self,pos, FALSE);
	newpos = (self)->BackwardCopyWord(newpos,0,buff);
	(self)->Keywordify(buff,TRUE);
	if ((word=srctext::Lookup(self->srctext::words,buff))->stng==NULL)
	    (self)->WrapStyle(newpos+1,strlen(buff), self->srctext::function_style, TRUE,TRUE);
    }
}

void rexxtext::BackwardCheckWord(long from, long to)
{
    Dict *word;
    char buff[256];
    long j=from;
    if (IsTokenChar(GetChar(from)) && !GetStyle(from) && !InString(from)) {
	j= BackwardCopyWord(from,to,buff);
	Keywordify(buff,TRUE);
	if ((word =srctext::Lookup(this->srctext::words,buff))->stng !=NULL) {
	    int l= strlen(buff);
	    if (!strcmp(word->stng,"PROCEDURE"))
		checkfunction(this,j);
	    if (GetForceUpper())
		ReplaceCharacters(j+1,l, word->stng,l);
	    WrapStyleNow(j+1,l, this->srctext::kindStyle[word->kind], FALSE,FALSE);
	} else if (GetForceUpper()) {
	    makeupper(buff); /* just in case Keywordify didn't do this (ctext) */
	    if ((word=srctext::Lookup(this->srctext::words,buff))->stng !=NULL) {
		int l= strlen(buff);
		if (word->kind==UPRCSE)
		    ReplaceCharacters(j+1,l, word->stng,l);
	    }
	}
    }
    return;
}

long rexxtext::CheckWord(long i, long end)
{
    long j;
    Dict *word;
    char buff[256];
    i=SkipWhitespace(i,end);
    j=CopyWord(i,end,buff);
    Keywordify(buff,FALSE);
    if ((word=srctext::Lookup(this->srctext::words,buff))->stng!=NULL) {
	if (!strcmp(word->stng,"PROCEDURE"))
	    checkfunction(this,i-1);
	/* style the keyword regardless of wether it is "PROCEDURE" or not */
	WrapStyle(i,strlen(word->stng), this->srctext::kindStyle[word->kind], FALSE,FALSE);
    }
    return j;
}

/* override */
/* CheckComment wraps the comment style, and properly detects NESTED comments. 'start' is the position of slash (not asterisk) that starts the comment. Returns position of ending slash. */
long rexxtext::CheckComment(long start)
{
    int delimcount=1;
    boolean usedslash=FALSE;
    long end=start+1, len=GetLength();

    while (++end<len)
	if (GetChar(end) == '*') {
	    if (GetChar(end-1) == '/' && !usedslash)
		++delimcount;
	    else if (GetChar(end+1) == '/') {
		++end;
		if (--delimcount < 1)
		    break;
		usedslash= TRUE; /* make sure we don't use that slash AGAIN, for a START delimiter */
	    }
	} else
	    usedslash= FALSE;
    WrapStyle(start,end-start+1, this->srctext::comment_style, FALSE,FALSE);
    return end;
}

void rexxtext::RedoStyles()
{
    long last_comment=0, newpos, posn, len=GetLength();
    int prev=0, c='\n', parens=0;
    char buff[256];
    Dict *word;
    /* c is initialized to a newline so the start of the file looks like the start of line. */
    RemoveStyles(); /* Remove the old styles, but leave the root environment in place. */
    for (posn=0; posn<len-1; ++posn) {
	prev = c;
	c = GetChar(posn);
	switch (c) {
	    case ' ': case '\t': case '\n':
		break;
	    case '*':
		if (prev=='/')
		    last_comment= posn= CheckComment(posn-1);
		break;
	    case '(':
		++parens;
		break;
	    case ')':
		--parens;
		break;
	    case ':':
		if (!parens) {
		    newpos = backwardSkipJunk(this,posn-1, FALSE);
		    newpos = BackwardCopyWord(newpos,0,buff);
		    Keywordify(buff,TRUE);
		    if ((word=srctext::Lookup(this->srctext::words,buff))->stng ==NULL)
			WrapStyle(newpos+1,strlen(buff), this->srctext::label_style, TRUE,TRUE);
		    else
			WrapStyle(newpos+1,strlen(buff), this->srctext::kindStyle[word->kind], FALSE,FALSE);
		}
		break;
	    case '\'':
	    case '\"':
		posn= CheckString(posn);
		break;
	    default:
		if (IsTokenChar(c))
		    posn= CheckWord(posn,len);
		break;
	}
    }
}

/* Indentation returns the proper indentation of the line that starts at pos */
int rexxtext::Indentation(long pos) /* WRN, Oct.92 */
{
    long savedPos=(-1), len=GetLength(), newpos, elseIndent=-1;
    char buff[256], ch, initbuff[256];
    int statementCount=0, wordCount=0, endCatch=0, elseCatch=0, openParens=0;
    short keywordVal=0;
    boolean addingElse= FALSE, addingEnd=FALSE, addingThen=FALSE,
    check=FALSE, sawThen=FALSE, sawElse=FALSE, sawNewLine=FALSE;

    /* avoid potential problems with first line of file */
    if (pos<1)
	return 0;
    /* see if indenting any special cases */
    if ((newpos=InCommentStart(pos)) > 0)
	return CurrentIndent(newpos) + this->srctext::commentIndent;
    newpos= SkipWhitespace(pos,len);
    CopyWord(newpos,len,initbuff);
    Keywordify(initbuff,FALSE);
    keywordVal = srctext::Lookup(this->srctext::words,initbuff)->val;
    if (keywordVal & ELSE_BIT) {
	elseCatch=1;
	addingElse=TRUE;
    } else if (keywordVal & END_BIT) {
	endCatch=1;
	addingEnd=TRUE;
    } else if (keywordVal & THEN_BIT)
	addingThen=TRUE;

    pos = backwardSkipJunk(this,pos-1,FALSE);
    if (ch=GetChar(pos) == '\n') {
	pos = backwardSkipJunk(this,pos-1,FALSE);
	newpos=pos;
	if ((ch=GetChar(newpos--)) == ',') {
	    newpos = backwardSkipJunk(this,newpos,FALSE);
	    /* adding the capability of using indenting parenthesis on a continueing line */
	    while((ch=GetChar(newpos)) != '\n') {
		if (newpos < 1)
		    return 0;
		else if (ch == '(')
		    return CurrentColumn(newpos)+1;
		else if (ch == ')')
		    newpos= backwardSkipDelimited(this,newpos-1,'(',')');
		savedPos = newpos;
		newpos = backwardSkipJunk(this,newpos-1,FALSE);	    
	    }
	    newpos=backwardSkipJunk(this, newpos-1,FALSE);
	    if (GetChar(newpos) == ',')
		return CurrentIndent(savedPos);
	    else
		return CurrentIndent(savedPos) + this->srctext::contIndent;
	}
    }
    pos = backwardSkipJunk(this,pos,TRUE);

    while (statementCount < 1 || check || elseCatch > 0 || endCatch > 0 || sawThen)  {
	sawNewLine=FALSE;
	if (pos < 0)
	    return 0;
	else
	    ch = GetChar(pos--);
	switch (ch) {
	    case '/':
		if (GetChar(pos)=='*') {
		    long startcomment;
		    if ((startcomment=InCommentStart(pos)) > 0
			|| (startcomment=unstyledCommentStart(this,pos)) > 0)
			pos= backwardSkipJunk(this,startcomment-1,TRUE);
		}
		break;
	    case '\n':
		newpos = backwardSkipJunk(this,pos,FALSE); 
		if ((ch = GetChar(newpos)) != ',')
		    ++statementCount;		
	    case ';':
	    case ' ':
	    case '\t':
	    case ',':
		break;
	    case '\'':
	    case '"':
		pos=BackwardSkipString(pos,ch); 
		newpos = backwardSkipJunk(this,pos,FALSE);
		if ((ch = GetChar(newpos))== '\n') {
		    sawNewLine = TRUE;
		    newpos = backwardSkipJunk(this,newpos-1,FALSE); 
		    if ((ch = GetChar(newpos)) == ',')  /* line must be continued from above */
			break;
		    if (check && elseCatch <= 0)
			return CurrentIndent(savedPos);
		    else if (!check && elseCatch <= 0 && endCatch <= 0) {
			savedPos=pos;
			check=TRUE;
		    }
		} else {
		    newpos = BackwardCopyWord(newpos,0,buff);
		    Keywordify(buff,FALSE);
		    if (srctext::Lookup(this->srctext::words,buff)->val & (THEN_BIT + ELSE_BIT)) {
			if (elseCatch <= 0 && !sawThen)
			    ++wordCount;
		    }
		}
		pos=backwardSkipJunk(this,pos,TRUE);
		break;
	    case ')':
		pos= backwardSkipDelimited(this,pos,'(',')');
		break;
	    case '(':
		return CurrentColumn(pos+1)+1;
	    default:
		if (!this->IsTokenChar(ch))
		    break;
		pos = BackwardCopyWord(pos+1,0,buff);
		Keywordify(buff,FALSE);
		keywordVal = srctext::Lookup(this->srctext::words,buff)->val;

		/* first check to see if this is just a continuation of the previous line */
		newpos = backwardSkipJunk(this, pos, FALSE);
		if ((ch = GetChar(newpos)) == '\n') {
		    sawNewLine = TRUE;
		    newpos = backwardSkipJunk(this, newpos-1, FALSE);
		    ch = GetChar(newpos);
		    if (ch == ',')
			/* line must be continued from above */
			sawNewLine = FALSE;
		}
		/* now check out the word that is in buff  */
		if ((keywordVal & BEGIN_BIT) || sawNewLine) {
		    if (!check)			 {
			if (keywordVal & IF_BIT) {
			    if (endCatch <= 0 && --elseCatch <= 0)
				if (addingElse || addingThen)
				    return CurrentIndent(pos);
				else if (sawThen && wordCount) {
				    elseCatch = 0;
				    savedPos= pos;
				    check = TRUE;
				} else
				    return CurrentIndent(pos) + this->srctext::levelIndent;
			    sawThen=FALSE;
			} else if (keywordVal & ELSE_BIT) {
			    if (endCatch <= 0)
				++elseCatch;
			} else if (keywordVal & WHEN_BIT) {
			    if (endCatch <= 0)
				if (addingThen || (sawThen && wordCount))
				    return CurrentIndent(pos);
				else
				    return CurrentIndent(pos) + this->srctext::levelIndent;
			    sawThen = FALSE;			      
			} else if (keywordVal & OTHERWISE_BIT) {
			    if (endCatch <=0)
				return CurrentIndent(pos) + this->srctext::levelIndent;
			} else if (keywordVal & THEN_BIT) {
			    sawThen=TRUE;
			} else if (keywordVal & END_BIT) {
			    ++endCatch;
			} else if (keywordVal & START_BIT) {
			    if (--endCatch <= 0)
				if (addingEnd)
				    return CurrentIndent(pos);
				else if (endCatch == 0) {
				    if (elseCatch <= 0) {
					check = TRUE;
					savedPos=pos;
				    }
				} else if (elseCatch <= 0)
				    return CurrentIndent(pos) + this->srctext::levelIndent;
			} else if (elseCatch <= 0  &&  endCatch <= 0) {
			    check = TRUE;
			    savedPos=pos;
			}
		    } else if (check) {
			if (keywordVal & IF_BIT) {
			    if (elseCatch > 0)
				if (CurrentIndent(pos) == elseIndent) {
				    --elseCatch;
				    elseIndent=-1;
				}
			    savedPos=pos;
			} else if (keywordVal & WHEN_BIT)
			    savedPos=pos;
			else if (keywordVal & THEN_BIT)
			    break;
			else if (keywordVal & ELSE_BIT) {
			    if (elseIndent == -1) {
				elseIndent=CurrentIndent(pos);
				++elseCatch;
			    }
			} else if (elseCatch <= 0)
			    return CurrentIndent(savedPos);
		    }
		    break;
		} else if (elseCatch <= 0 && !sawThen) {
		    ++wordCount;
		}
	}
    }
    return 0;
}
