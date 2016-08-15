/* File modtext.C created by R L Quinn
   (c) Copyright IBM Corp.  1988-1995.  All rights reserved.

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

   modtext, an object for editing Modula-x code. */

#include <andrewos.h>

static UNUSED const char ibmid[] = "(c) Copyright IBM Corp.  1988-1995.  All rights reserved.";
static UNUSED const char rcsHeader[] = "$Header: /afs/cs.cmu.edu/project/atk-src-C++/atk/srctext/RCS/modtext.C,v 2.2 1995/02/09 21:56:46 susan Stab74 $";

#include <environment.H>
#include <stylesheet.H>
#include <style.H>
#include <fontdesc.H>
#include <attribs.h>

#include "srctext.H"
#include "modtext.H"

ATKdefineRegistry(modtext, srctext, NULL);

boolean modtext::IsTokenChar(char ch)    
{
    return isalnum(ch)||ch=='_'||ch=='|';
}

boolean modtext::IsTokenOrPeriod(char ch)
{
    return IsTokenChar(ch)||(ch=='.');
}

void modtext::SetAttributes(struct attributes *atts)
{
    (this)->srctext::SetAttributes(atts);
    while (atts!=NULL) {
	char *key=atts->key;
	if (strcmp(key,"preprocessor")==0)
	    this->preprocessor=atoi(atts->value.string);
	else if (strcmp(key,"outdent-preproc")==0)
	    this->outdentPreproc=atoi(atts->value.string);
	else if (strcmp(key,"nested-proc-indent")==0)
	    this->nestedprocindent=atoi(atts->value.string);
	atts=atts->next;
    }
}

/* modtext_Keywordify makes buff all uppercase IF it wasn't mixed case to begin with AND ForceUpper is ON.  The latter condition only applies if checkforceupper is TRUE. */
/* This function "keywordifies" a string. RedoStyles says checkforceupper is FALSE, so "begin" won't ever be found in the hash table. BackwardCheckWord says checkforceupper is TRUE, so, if ForceUpper is ON, "begin" changes to "BEGIN", hence it gets "keywordified". */
char *modtext::Keywordify(char *buff, boolean checkforceupper)
{
    if (buff!=NULL && strlen(buff)>0 && checkforceupper && GetForceUpper()) {
	char *b= buff;
	while (*b!='\0' && !isupper(*b))
	    ++b;
	if (*b=='\0')
	    makeupper(buff);
    }
    return buff;
}

/* override */
/* modtext_BackwardCheckWord calls CheckWord to wrap procedure name whenever PROCEDURE_VAL is found */
void modtext::BackwardCheckWord(long from, long to)
{
    Dict *word;
    char buff[256];
    long j=from;
    if (IsTokenChar(GetChar(from)) && !GetStyle(from) && !InString(from)) {
	j=BackwardCopyWord(from,to,buff);
	Keywordify(buff,TRUE);
	if((word=srctext::Lookup(this->srctext::words,buff))->stng!=NULL) {
	    int l=strlen(buff);
	    if (GetForceUpper())
		ReplaceCharacters(j+1,l, word->stng,l);
	    WrapStyleNow(j+1,l, this->srctext::kindStyle[word->kind], FALSE,FALSE);
	}
    }
    return /*j*/;
}

/* override */
/* modtext_CheckWord does the procedure-name recognizing */
long modtext::CheckWord(long i, long end)
{
    long j;
    Dict *word;
    char buff[256];
    i=SkipWhitespace(i,end);
    j=CopyWord(i,end,buff);
    Keywordify(buff,FALSE);
    if((word=srctext::Lookup(this->srctext::words,buff))->stng!=NULL) {
	WrapStyle(i,strlen(word->stng), this->srctext::kindStyle[word->kind], FALSE,FALSE);
	if (word->val & PROCEDURE_BIT) {
	    i=SkipWhitespace(j+1,end);
	    j=CopyWord(i,end,buff);/*get procedure name*/
	    Keywordify(buff,FALSE);
	    if ((word=srctext::Lookup(this->srctext::words, buff))->stng==NULL) /*legit proc name*/
		WrapStyle(i,strlen(buff), this->srctext::function_style, TRUE,TRUE);
	    else /*not a proc name!! make appropriate style*/
		WrapStyle(i,strlen(word->stng), this->srctext::kindStyle[word->kind], FALSE,FALSE);
	}
    }
    return j;
}

static void SetupStyles(modtext *self)
{
    if ((self->srctext::kindStyle[KEYWRD]= (self->text::styleSheet)->Find("keyword")) == NULL) {
	self->srctext::kindStyle[KEYWRD]= new style;
	(self->srctext::kindStyle[KEYWRD])->SetName("keyword");
	(self->srctext::kindStyle[KEYWRD])->AddNewFontFace(fontdesc_Bold);
	(self->text::styleSheet)->Add(self->srctext::kindStyle[KEYWRD]);
    }
}

/* only setup styles that are common to both mtext and m3text! */
void modtext::SetupStyles()
{
    (this)->srctext::SetupStyles();
    ::SetupStyles(this);
}

modtext::modtext()
{
    this->srctext::useTabs=FALSE;
    this->srctext::commentIndent= 3;
    this->srctext::commentString= "(*  *)";
    this->srctext::indentingEnabled= TRUE;
    this->preprocessor= FALSE;
    this->outdentPreproc= TRUE;
    this->nestedprocindent= 0;
    ::SetupStyles(this);
    THROWONFAILURE(TRUE);
}

/* override */
/* CheckComment will wrap the comment style around paren-star comments, taking nesting into account. */
long modtext::CheckComment(long start)
{
    int prev, c=0, comments=1;
    long end=start+1, len=GetLength();

    while (end<len) {
	prev= c;
	c= GetChar(++end);
	switch(c) {
	    case '*':
		if(prev=='(') ++comments;
		break;
	    case ')':
		if(prev=='*') --comments;
		break;
	}
	if (comments<1) {
	    WrapStyle(start,end-start+1, this->srctext::comment_style, FALSE,FALSE);
	    break;
	}
    }
    return end;
}

/* checkPreproc wraps the preproc style around a preprocessor directive */
static long checkPreproc(modtext *self, long start)
{
    long end, len=(self)->GetLength();
    int prev, c;
    CheckSmore: ; /* jump here after we're done handling the comment or whatever */
    for (c=0, end=start; end<len; ++end) {
	prev= c;
	switch (c=(self)->GetChar(end)) {
	    case '\n':
		if (!(self)->Quoted(end)) {
		    if (end>start)
			(self)->WrapStyle(start,end-start, self->srctext::kindStyle[PREPRC], FALSE,TRUE);
		    return end-1;
		}
		break;
	    case '*':
		if (prev=='(') {
		    if (end-1>start)
			(self)->WrapStyle(start,end-start-1, self->srctext::kindStyle[PREPRC], FALSE,TRUE);
		    start= (self)->CheckComment(end-1) +1;
		    goto CheckSmore; /* continue at end of comment */
		}
		break;
	}
    }
    return end-1;
}

void modtext::RedoStyles()
{
    long posn, len=GetLength();
    int prev=0, c='\n'; /* c is initialized to a newline so the start of the file looks like the start of line. */
    RemoveStyles(); /* Remove the old styles, but leave the root environment in place. */
    for (posn=0; posn<len-1; ++posn) {
	prev = c;
	c = GetChar(posn);
	switch (c) {
	    case ' ': case '\t':
		break;
	    case '\n':
		posn= SkipWhitespace(posn+1,len) -1;
		break;
	    case '#':
		if (prev=='\n' && this->preprocessor)
		    posn= checkPreproc(this,posn);
		break;
	    case '*':
		if (prev == '(')
		    posn= CheckComment(posn-1);
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

long modtext::ReverseBalance(long pos)
{
    boolean instring=FALSE, doublestring=FALSE;
    int incomment=0, inpragma=0;
    int thischar, prechar;
    char *parentype;
    struct paren_node {
	long type;
	struct paren_node *next;
    } *parenstack=NULL, *temp;
    static char *opens = "({[", *closes = ")}]";

    while (pos>0) {
	thischar= GetChar(--pos);
	prechar= GetChar(pos-1);
	if (incomment) {
	    if (thischar=='*' && prechar=='(') {
		--incomment;
		--pos;
		if (incomment<1 && parenstack==NULL)
		    return pos;
	    } else if (thischar==')' && prechar=='*')
		++incomment;
	} else if (inpragma) {
	    if(thischar == '*' && prechar == '<') {
		--inpragma;
		--pos;
	    } else if (thischar=='>' && prechar=='*') 
		++inpragma;
	} else if (!Quoted(pos)) {
	    if (instring) {
		if ((thischar == '"' && doublestring) || (thischar == '\'' && !doublestring))
		    instring = FALSE;
	    } else if (thischar == '"') {
		instring = TRUE;
		doublestring = TRUE;
	    } else if (thischar == '\'') {
		instring = TRUE;
		doublestring = FALSE;
	    } else if (thischar == ')' && prechar == '*')
		++incomment;
	    else if (thischar == '>' && prechar == '*')
		++inpragma;
	    else if ((parentype= strchr(opens, thischar)) != NULL) {
		if (parenstack==NULL || parenstack->type != (parentype-opens))
		    /* empty stack or wrong paren type. Abort! */
		    break;
		/* pop close-paren off stack */
		temp= parenstack;
		parenstack= parenstack->next;
		free(temp);
		if (parenstack==NULL)
		    return pos;
	    } else if ((parentype= strchr(closes, thischar)) != NULL) {
		/* push close-paren onto stack */
		temp= (struct paren_node *)malloc(sizeof(struct paren_node));
		temp->type= parentype - closes;
		temp->next= parenstack;
		parenstack= temp;
	    }
	}
    }
    /* free stack if we found a bogus paren or made it to start of file */
    while (parenstack) {
	temp= parenstack;
	parenstack= parenstack->next;
	free(temp);
    }
    return EOF;
}

static long backwardSkipComment(modtext *self, long pos)
{
    int count=1;
    int ch=0,prev;
    if(pos>=0)
	while((count>0)&&(--pos>0)) {
	    prev=ch;
	    if (((ch=(self)->GetChar(pos))=='(')&& (prev=='*')) --count;
	    else if ((ch=='*')&&(prev==')')) ++count;
	}
    return pos-1;
}

static long backwardSkipPragma(modtext *self, long pos)
{
    int count=1;
    int ch=0,prev;
    if(pos>=0)
	while((count>0)&&(--pos>0)) {
	    prev=ch;
	    if(((ch=(self)->GetChar(pos))=='<')&& (prev=='*')) --count;
	    else if ((ch=='*')&&(prev=='>')) ++count;
	}
    return pos-1;
}

static long maybeBackwardSkipCppLines(modtext  *self,long  pos)
{
    long currentEOL, len=(self)->GetLength();
    --pos;
    do  {
	currentEOL= pos;
	pos= (self)->BackwardSkipString(pos,'\n');
    } while (pos>=0 && (self)->GetChar((self)->SkipWhitespace(pos+2,len))=='#');
    return currentEOL+1;
}

static long backwardSkipJunk(modtext  *self,long  i)
{
    int ch=0;
    boolean done=FALSE;
    while(!done && (i>=0)) {
	ch=(self)->GetChar(i);
	switch(ch) {
	    case '\'':
	    case '\"':
		i=(self)->BackwardSkipString(i-1,ch);
		break;
	    case ')':
		if(i<1 || (self)->GetChar(i-1)!='*') done=TRUE;
		else i=backwardSkipComment(self,i-1)+1;
		break;
	    case '>':
		if(i<1 || (self)->GetChar(i-1)!='*') done=TRUE;
		else i=backwardSkipPragma(self,i-1)+1;
		break;
	    case '\n':
		if (self->preprocessor)
		    i= maybeBackwardSkipCppLines(self,i);
		/* fall through */
	    case '\t':
	    case ' ':
		/* nothing */
		break;
	    default:
		done=TRUE;
	}
	--i;
    }
    return i+1;
}

static long backwardSkipDelimited(modtext *self, long pos, char beg, char end)
{
    int level=1;
    while(pos>=0 && level>0){
	int c;
	c=(self)->GetChar(pos--);
	switch(c){
	    case '\'':
	    case '"':
		pos=(self)->BackwardSkipString(pos,c);
		break;
	    case ')':
		if ((self)->GetChar(pos)=='*') {
		    pos = backwardSkipComment(self,pos-1);
		} else if(c==end) {
		    ++level;
		}
		break;
	    default:
		if(c==beg) {
		    --level;
		} else if(c==end) {
		    ++level;
		}
	}
    }
    return pos;
}

static long matchend(modtext *self, long pos, int fornext)
{
    int count=1, curr_indent;
    Dict *word2;
    char buff[256];

    while((count>0)&&(--pos>=0)) {
	pos=backwardSkipJunk(self,pos);
	if(pos>=0) {
	    if((self)->IsTokenChar((self)->GetChar(pos))) {
		pos=(self)->BackwardCopyWord(pos,0,buff)+1;
		(self)->Keywordify(buff,FALSE);
		word2=srctext::Lookup(self->srctext::words,buff);
		if (word2->val & END_BIT) ++count;
		else if(word2->val & MATCH_END_BIT) --count;
	    }
	}
    }
    curr_indent = (self)->CurrentIndent(pos);
    if (fornext) {
	if (strcmp(word2->stng, "BEGIN") == 0)
	    curr_indent -= self->nestedprocindent;
    }
    return ((curr_indent<0)? 0 : curr_indent);
}

static long SkipPragma(modtext *self, long pos, long end)
{
    int count=1;
    int ch=0,prev;
    while((count>0)&&(++pos<end)) {
	prev=ch;
	if(((ch=(self)->GetChar(pos))=='>')&& (prev=='*')) --count;
    }
    return pos+1;
}

static int backwardFirstnonwhitespace(modtext *self, long pos)   
{
    int ch;
    while (pos>0) {
	if (((self)->GetChar(pos)=='>')&&((self)->GetChar(pos-1)=='*')) pos=backwardSkipPragma(self,pos);
	if (!is_whitespace(ch=(self)->GetChar(pos--)))
	    return ch;
    }
    return '\n'; /* pretend there's a newline before the start of the file */
}

/* should only get called with one of the following in word:  MODULE, PROCEDURE, BEGIN, VAR, TYPE, CONST */
int backwardFindProcIndent(modtext *self, long pos, Dict *word)
{
    char buff[256], ch;
    Dict *word2;
    while(pos>=0) {
	pos = backwardSkipJunk(self,pos);
	if (pos>=0) {
	    ch = (self)->GetChar(pos);
	    if((self)->IsTokenChar(ch)) {
		pos=(self)->BackwardCopyWord(pos,0,buff)+1;
		(self)->Keywordify(buff,FALSE);
		word2=srctext::Lookup(self->srctext::words,buff);
		if (word->val & PROCEDURE_BIT) {
		    if ((word2->val & PROCEDURE_BIT) && backwardFirstnonwhitespace(self, pos-1)=='\n')
			return (self)->CurrentIndent(pos) + self->nestedprocindent;
		    else if (word2->val & MODULE_BIT)
			return (self)->CurrentIndent(pos);
		    else if (word2->stng && strcmp(word2->stng, "BEGIN") == 0)
			return (self)->CurrentIndent(pos);
		} else /* word is one of: BEGIN, VAR, TYPE, CONST   etc ...*/ {
		    if ((word2->val & PROCEDURE_BIT) && (backwardFirstnonwhitespace(self, pos-1)=='\n'))
			return (self)->CurrentIndent(pos);
		    else if (word2->val & MODULE_BIT)
			return (self)->CurrentIndent(pos);
		    else if (word2->stng && strcmp(word2->stng, "BEGIN") == 0)
			return (self)->CurrentIndent(pos) - self->nestedprocindent;
		}
	    } else if (ch == '(') {
		pos=(self)->SkipWhitespace(pos+1,(self)->GetLength());
		return (self)->CurrentColumn(pos);
	    } else if (ch == ')') {
		pos = backwardSkipDelimited(self, pos-1, '(', ')')+1;
	    }
	}
	--pos;
    }
    return 0;
}

/* STYLESTARTCOL is a macro to find the current column of the beginning of the style wrapped around position pos.*/
#define STYLESTARTCOL(self,pos) (((self))->CurrentColumn((((self))->GetEnvironment((pos)))->Eval()))

/* Indentation returns the proper indentation of the line that starts at pos */
int modtext::Indentation(long pos)
{       
    char buff[256], ch;
    Dict *word;
    long i,j,k,count,savcount;
    int indent=0;
    long oldj,temppos, len =GetLength();

    /* avoid potential problems with first line of file */
    if (pos<1)
	return 0;
    if(GetStyle(pos)==this->srctext::comment_style)
	return STYLESTARTCOL(this,pos)+this->srctext::commentIndent;
    temppos=SkipWhitespace(pos,len);
    if (GetChar(temppos)=='(' && GetChar(temppos+1)=='*' && !this->srctext::reindentComments)
	return CurrentIndent(pos);
    if (GetChar(temppos)=='<' && GetChar(temppos+1)=='*') {
	temppos= pos= SkipPragma(this,temppos,len);
	temppos= SkipWhitespace(pos, len);
    }
    if (this->preprocessor && this->outdentPreproc && GetChar(temppos)=='#')
	return 0;
    (void) CopyWord(/*pos*/temppos,len,buff);
    Keywordify(buff,FALSE);
    word=srctext::Lookup(this->srctext::words,buff);

    if (word->val & MODULE_BIT) return 0;
    if (word->val & LINE_WITH_MODULE_BIT) return backwardFindProcIndent(this, pos, word);
    if (word->val & END_BIT) return matchend(this,pos-1,0);
    if (word->val & ALT_END_BIT) return matchend(this,pos-1,0);
    j=backwardSkipJunk(this,pos-1);
    while ((GetChar(j)=='(')&&(GetChar(j+1)=='*')&&(j>0))
	j=backwardSkipJunk(this,--j);
    i=GetBeginningOfLine(j);
    k=SkipWhitespace(i,len);
    if (GetChar(k)=='|'){
	return CurrentIndent(k)+this->srctext::levelIndent; 
    }
    oldj=j;
    while (j>i && j >=0) {
	ch= GetChar(j);
	if (ch == '(') {
	    j=SkipWhitespace(j+1,len);
	    return CurrentColumn(j);
	} else if (ch == ')') {
	    j = backwardSkipDelimited(this, j-1, '(', ')')+1;
	    i=GetBeginningOfLine(j);
	    if (GetChar(k)=='|'){
		return CurrentIndent(k)+this->srctext::levelIndent; 
	    }
	    oldj=j;
	} else if (IsTokenChar(ch)) {
	    j=BackwardCopyWord(j,i,buff)+1;
	    Keywordify(buff,FALSE);
	    word=srctext::Lookup(this->srctext::words,buff);
	    if((word->val & MATCH_END_BIT || word->val & ALT_END_BIT
		||(word->val & INDENT_NEXT_LINE_BIT && backwardFirstnonwhitespace(this,j-1)=='\n'))
	       && !(word->val & PROCEDURE_BIT))
		indent=this->srctext::levelIndent;
	    if(word->val&END_BIT) {
		indent += matchend(this,j,1);
		return indent;
	    }
	}
	j=backwardSkipJunk(this,j-1);
    }
    if ((i=CurrentIndent(oldj))+indent<0) indent=(-i);

    return i+indent;
}
