/* File ctext.C created by R L Quinn
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

   ctext, an object for editing C code. */

#include <andrewos.h>

static UNUSED const char ibmid[] = "(c) Copyright IBM Corp.  1988-1995.  All rights reserved.";

#include <stylesheet.H>
#include <style.H>
#include <fontdesc.H>
#include <attribs.h>
#include <toolcnt.h>

#include "srctext.H"
#include "ctext.H"

static Dict *words[TABLESIZE];


ATKdefineRegistry(ctext, srctext, ctext::InitializeClass);


boolean ctext::IsTokenChar(char ch)    
{
    return (isalnum(ch) || (ch) == '_');
}

void ctext::SetAttributes(struct attributes *atts)
{
    (this)->srctext::SetAttributes(atts);
    while (atts!=NULL) {
	const char *key=atts->key;
	if (strcmp(key,"brace-indent")==0)
	    this->braceIndent=atoi(atts->value.string);
	else if (strcmp(key,"switch-label-undent")==0)
	    this->switchLabelUndent=atoi(atts->value.string);
	else if (strcmp(key,"switch-level-indent")==0)
	    this->switchLevelIndent=atoi(atts->value.string);
	else if (strcmp(key,"outdent-preproc")==0)
	    this->outdentPreproc=atoi(atts->value.string);
	atts=atts->next;
    }
}

static void SetupStyles(ctext *self)
{
    if ((self->srctext::kindStyle[KEYWRD]= (self->text::styleSheet)->Find("keyword")) == NULL) {
	self->srctext::kindStyle[KEYWRD]= new style;
	(self->srctext::kindStyle[KEYWRD])->SetName("keyword");
	(self->srctext::kindStyle[KEYWRD])->AddNewFontFace(fontdesc_Bold);
	(self->text::styleSheet)->Add(self->srctext::kindStyle[KEYWRD]);
    }
    if ((self->srctext::kindStyle[PREPRC]= (self->text::styleSheet)->Find("preproc")) == NULL) {
	self->srctext::kindStyle[PREPRC]= new style;
	(self->srctext::kindStyle[PREPRC])->SetName("preproc");
	(self->srctext::kindStyle[PREPRC])->AddNewFontFace(fontdesc_Italic);
	(self->text::styleSheet)->Add(self->srctext::kindStyle[PREPRC]);
    }
}

void ctext::SetupStyles()
{
    (this)->srctext::SetupStyles();
    ::SetupStyles(this);
}

boolean ctext::InitializeClass()
{
    static const Dict ckeywords[]={
	{"auto",0,KEYWRD},
	{"break",0,KEYWRD},
	{"case",0,KEYWRD},
	{"char",0,KEYWRD},
	{"const",0,KEYWRD},
	{"continue",0,KEYWRD},
	{"default",0,KEYWRD},
	{"do",0,KEYWRD},
	{"double",0,KEYWRD},
	{"else",0,KEYWRD},
	{"enum",0,KEYWRD},
	{"extern",0,KEYWRD},
	{"float",0,KEYWRD},
	{"for",0,KEYWRD},
	{"goto",0,KEYWRD},
	{"if",0,KEYWRD},
	{"int",0,KEYWRD},
	{"long",0,KEYWRD},
	{"register",0,KEYWRD},
	{"return",0,KEYWRD},
	{"short",0,KEYWRD},
	{"signed",0,KEYWRD},
	{"sizeof",0,KEYWRD},
	{"static",0,KEYWRD},
	{"struct",0,KEYWRD},
	{"switch",0,KEYWRD},
	{"typedef",0,KEYWRD},
	{"union",0,KEYWRD},
	{"unsigned",0,KEYWRD},
	{"void",0,KEYWRD},
	{"volatile",0,KEYWRD},
	{"while",0,KEYWRD},
	{NULL,0,0}
    };
    srctext::BuildTable("ctext",::words,ckeywords);
    return TRUE;
}

ctext::ctext()
{
    ATKinit;
    this->srctext::words= (Dict **)::words;
    this->braceIndent=0;
    this->switchLabelUndent=4;
    this->switchLevelIndent=8,
    this->outdentPreproc=1;
    this->srctext::commentString= "/*  */";
    this->srctext::indentingEnabled= TRUE;
    ::SetupStyles(this);
    ToolCount("EditViews-ctext",NULL);
    THROWONFAILURE(TRUE);
}

/* returns the length of a *constant* string */
#define cstrlen(str) (sizeof(str)-1)

/* pos should pointer to 2nd from last character-- last character won't be
 * matched; strconst *must* be a constant string, so that sizeof works
 */
#define backwardMatch(self,pos,strConst) \
    (pos>=cstrlen(strConst)-1 && match(self,pos-(cstrlen(strConst)-1)+1,strConst))

#define match(self,pos,str) ((pos==0 || !(self)->IsTokenChar((self)->GetChar(pos-1))) && !(self)->IsTokenChar((self)->GetChar(pos+cstrlen(str))) && (self)->DoMatch(pos,str,cstrlen(str)))

long ctext::MaybeBackwardSkipCppLines(long pos)
{
    long currentEOL;
    long len= GetLength();

    --pos;
    do  {
	currentEOL=pos;
	pos=BackwardSkipString(pos,'\n');
    } while (pos>=0 && GetChar(SkipWhitespace(pos+2,len))=='#');

    return currentEOL+1;
}

long ctext::BackwardSkipJunk(long pos)
{
    while (pos>=0) {
	int c=GetChar(pos--);

	switch(c) {
	    case '/':
		if (GetChar(pos)=='*') {
		    long startcomment;
		    if ((startcomment=InCommentStart(pos)) > 0 || (startcomment=UnstyledCommentStart(pos)) > 0)
			pos= startcomment-1;
		    pos= MaybeBackwardSkipCppLines(pos);
		} else /* that slash ISN'T junk */
		    return pos+1;
		break;
	    case '\n':
		if (GetStyle(pos)==this->srctext::linecomment_style)
		    pos= InCommentStart(pos)-1;
		pos= MaybeBackwardSkipCppLines(pos);
		/* fall through */
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

/* beg and end must be constants (for switch) */
/* assumes ending delimiter AFTER pos */
long ctext::BackwardSkipDelimited(long pos, char beg, char end)
{
    int level=1;
    while(pos>=0 && level>0){
	int c;
	switch(c=GetChar(pos--)){
	    case '\'':
	    case '"':
		pos=BackwardSkipString(pos,c);
		break;
	    case '/':
		if (GetChar(pos)=='*') {
		    long startcomment;
		    if ((startcomment=InCommentStart(pos)) > 0 || (startcomment=UnstyledCommentStart(pos)) > 0)
			pos= startcomment-1;
		}
		break;
	    case '\n': /* for // comments in C++ */
		if (GetStyle(pos)==this->srctext::linecomment_style)
		    pos= InCommentStart(pos)-1;
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

/* returns true iff the character at pos is quoted (ie. "\"). Takes into account the slash being quoted. (ie "\\"). */
boolean ctext::Quoted(long pos)
{
    boolean retval = FALSE;
    while (pos-->0 && GetChar(pos)=='\\')
	retval= !retval;
    return retval;
}

/* ctext_UnstyledCommentStart is a fallback for places where we KNOW we're at the end of a comment, but InCommentStart returns zero becuse there was no style there. */
long ctext::UnstyledCommentStart(long pos)
{
    int prev, c=0;
    while (pos>0) {
	prev= c;
	c= GetChar(--pos);
	if (c=='/' && prev=='*')
	    /* found a slash-star. It COULD be *inside* the actual comment, but we'll guess that it's the *opening* slash-star. */
	    return pos;
	else if (c=='*' && prev=='/')
	    /* whoah! found ANOTHER end-of-comment */
	    return 0;
    }
    return 0;
}

/* CheckPreproc wraps the preproc style around a preprocessor directive */
long ctext::CheckPreproc(long start)
{
    long end, len=GetLength();
    int prev, c;
    CheckSmore: ; /* jump here after we're done handling the comment or whatever */
    for (c=0, end=start; end<len; ++end) {
	prev= c;
	switch (c=GetChar(end)) {
	    case '\n':
		if (!Quoted(end)) {
		    if (end>start)
			WrapStyle(start,end-start, this->srctext::kindStyle[PREPRC], FALSE,TRUE);
		    return end-1;
		}
		break;
	    case '*':
		if (prev=='/') {
		    if (end-1>start)
			WrapStyle(start,end-start-1, this->srctext::kindStyle[PREPRC], FALSE,TRUE);
		    start= CheckComment(end-1) +1;
		    goto CheckSmore; /* continue at end of comment */
		}
		break;
	    case '/':
		if (prev=='/' && this->srctext::linecommentString) { /* cheated to detect C++ object */
		    if (end-1>start)
			WrapStyle(start,end-start-1, this->srctext::kindStyle[PREPRC], FALSE,TRUE);
		    return CheckLinecomment(end-1);
		}
		break;
	}
    }
    return end-1;
}

static void fn_name(ctext *self, long posn, long last_comment)
{
    long end=posn, length=(self)->GetLength();
    int c, parencount=0;
    do switch ((self)->GetChar(end++)) {
	case '(':
	    ++parencount;
	    break;
	case ')':
	    --parencount;
	    break;
    } while (parencount>0 && end<length);
    while ((c=(self)->GetChar(end))<=' ' && end<length)
	++end;
    if (c=='[' || c=='(')
	/* either it's a global array of pointers to functions, or a single pointer to a function. neither case is an actual function declaration */
	return;
    /* Scan backwards for the end of preceding whitespace */
    end= posn= (self)->BackwardSkipJunk(posn-1);
    /* Now find start of token */
    while (posn>=last_comment && (self)->IsTokenChar((self)->GetChar(posn)))
	--posn;
    if (posn<end)
	(self)->WrapStyle(posn+1,end-posn, self->srctext::function_style, TRUE,TRUE);
}

void ctext::RedoStyles()
{
    long last_comment = 0, posn, len = GetLength();
    int prev=0, c='\n', braces=0, escape=FALSE;
    /* c is initialized to a newline so the start of the file looks like the start of line. */
    RemoveStyles(); /* Remove the old styles, but leave the root environment in place. */
    for (posn=0; posn<len-1; ++posn) {
	prev = c;
	c = GetChar(posn);
	if(escape)
	    escape=FALSE;
	else
	    switch (c) {
		case ' ': case '\t':
		    break;
		case '\n':
		    posn= SkipWhitespace(posn+1,len) -1;
		    break;
		case '#':
		    if (prev == '\n')
			posn= CheckPreproc(posn);
		    break;
		case '\\':
		    escape=TRUE;
		    break;
		case '*':
		    if (prev == '/')
			last_comment= posn= CheckComment(posn-1);
		    break;
		case '{':
		    ++braces;
		    break;
		case '}':
		    --braces;
		    break;
		case '(':
		    if (!braces)
			fn_name(this, posn, last_comment);
		    ++braces;
		    break;
		case ')':
		    --braces;
		    break;
		case '\'':
		case '\"':
		    posn= CheckString(posn);
		    break;
		case ':':
		    BackwardCheckLabel(posn);
		    break;		    
		default:
		    if (IsTokenChar(c))
			posn= CheckWord(posn,len);
		    break;
	    }
    }
}

/* Indentation returns the proper indentation of the line that starts at pos */
int ctext::Indentation(long pos)
{
    int c;
    long savedPos=(-1), parensFollowedBy=(-1), colonFollowedBy=(-1);
    int elseCatch=0;
    boolean onestatement=TRUE, skipend=FALSE, braceFlag=FALSE, commaFlag=FALSE;
    int braceExtra=0, levelExtra=this->srctext::levelIndent,
    switchLevelExtra=this->switchLevelIndent, labelExtra=0;

    /* avoid potential problems with first line of file */
    if (pos<1)
	return 0;

    /* see if indenting any special cases */
    {
	long newpos;
	if ((newpos=InCommentStart(pos)) > 0)
	    return CurrentColumn(newpos) + this->srctext::commentIndent;
	for (newpos=pos; (c=GetChar(newpos))==' ' || c=='\t'; ++newpos) ;
	switch (c) {
	    case '}':
		braceFlag=TRUE; /* kluge ? */
		pos= BackwardSkipDelimited(newpos-1,'{','}') +1;
		/* fall through */
	    case '{':
		braceFlag=TRUE; /* kluge ? */
		levelExtra=this->braceIndent;
		switchLevelExtra=this->braceIndent;
		break;
	    case '#':
		if (this->outdentPreproc)
		    return 0;
		break;
	    case 'e':
		if(match(this,newpos,"else"))
		    elseCatch=1;
		break;
	    case 'c':
	    case 'd':
		if(match(this,newpos,"case") || match(this,newpos,"default"))
		    labelExtra= -this->switchLabelUndent;
		break;
	    case '/':
		if (GetChar(newpos+1)=='*' && !this->srctext::reindentComments)
		    return CurrentIndent(newpos);
		break;
	}
    }
    pos= BackwardSkipJunk(pos-1);

    for(;;){
	switch(GetChar(pos)){
	    case '}':
		pos= BackwardSkipDelimited(--pos,'{','}'); /* avoid hitting the { */
		onestatement=FALSE;
		braceFlag=TRUE;
		pos= BackwardSkipJunk(pos);
		continue;
	    case '{':
		braceExtra+=this->srctext::levelIndent;
		braceFlag=TRUE;
		pos= BackwardSkipJunk(--pos);
		continue;
	}
	break;
    }

    if (GetChar(pos)==';') {
	savedPos=pos--;
	onestatement=FALSE;
    }

    for(;;){
	int c;

	if(pos<0)
	    c=EOF;
	else
	    c=GetChar(pos--);
	switch(c){
	    case '/':
		if (GetChar(pos)=='*') {
		    long startcomment;
		    if ((startcomment=InCommentStart(pos)) > 0 || (startcomment=UnstyledCommentStart(pos)) > 0)
			pos= startcomment-1;
		}
		break;
	    case '"':
	    case '\'':
		pos=BackwardSkipString(pos,c);
		savedPos=pos;
		break;
	    case '*':
		/* primitive comment support */
		if(pos>=0 && GetChar(pos)=='/')
		    return CurrentColumn(pos)+this->srctext::commentIndent;
		break;
	    case '\n':
		/* must check to avoid cpp lines */
		pos=MaybeBackwardSkipCppLines(pos);
		/* fall through */
	    case ' ':
	    case '\t':
		/* nothing */
		break;
	    case 'f': /* if */
		if((onestatement || elseCatch>0) && backwardMatch(this,pos,"if"))
		    if(elseCatch==0)
			if(parensFollowedBy==-1)
			    return CurrentIndent(pos) +levelExtra +labelExtra;
			else
			    return CurrentIndent(pos) +levelExtra +this->srctext::contIndent;
		    else if(--elseCatch==0)
			return CurrentIndent(pos); /* line else up with if */
		savedPos=pos;
		break;
	    case 'o': /* do */
		if(onestatement && backwardMatch(this,pos,"do"))
		    if(savedPos==-1)
			return CurrentIndent(pos)+levelExtra+labelExtra;
		    else
			return CurrentIndent(pos) +levelExtra +this->srctext::contIndent;
		savedPos=pos;
		break;
	    case 'e': /* else, case or while */
		if(backwardMatch(this,pos,"case"))
		    if(!onestatement || colonFollowedBy==-1)
			return CurrentIndent(pos) + this->switchLevelIndent - (this->switchLevelIndent - this->switchLabelUndent) +labelExtra;
		    else
			return CurrentIndent(pos) + (this->switchLevelIndent - this->switchLabelUndent) + this->srctext::contIndent;
		else if(onestatement && backwardMatch(this,pos,"while"))
		    if(parensFollowedBy==-1)
			return CurrentIndent(pos) +levelExtra +labelExtra;
		    else
			return CurrentIndent(pos) +levelExtra +this->srctext::contIndent;
		else if(backwardMatch(this,pos,"else"))
		    if(onestatement)
			if(savedPos==-1)
			    return CurrentIndent(pos) +levelExtra +labelExtra;
			else
			    return CurrentIndent(pos) +levelExtra +this->srctext::contIndent;
		    else{
			skipend=TRUE;
			if(elseCatch>0)
			    ++elseCatch;
		    }
		savedPos=pos;
		break;
	    case 'r':
		if(onestatement && backwardMatch(this,pos,"for"))
		    if(parensFollowedBy==-1)
			return CurrentIndent(pos) +levelExtra +labelExtra;
		    else
			return CurrentIndent(pos) +levelExtra +this->srctext::contIndent;
		savedPos=pos;
		break;
	    case 'h':
		if(onestatement && backwardMatch(this,pos,"switch"))
		    if(parensFollowedBy==-1)
			return CurrentIndent(pos) +switchLevelExtra +labelExtra;
		    else
			return CurrentIndent(pos) +switchLevelExtra +this->srctext::contIndent;
		savedPos=pos;
		break;
	    case 't':
		if(backwardMatch(this,pos,"default") && !IsTokenChar(GetChar(pos+2)))
		    if(!onestatement || colonFollowedBy==-1)
			return CurrentIndent(pos) + this->switchLevelIndent - (this->switchLevelIndent - this->switchLabelUndent) + labelExtra;
		    else
			return CurrentIndent(pos) + (this->switchLevelIndent - this->switchLabelUndent) + this->srctext::contIndent;
		savedPos=pos;
		break;
	    case ',':
		/* this is a kluge, which assumes that commas occuring outside of parens
		   * imply a separate statement (i.e., equivalent indentation).  This is mostly true.
		   */
		commaFlag=TRUE;
		savedPos=pos;
		break;
	    case ':':
		colonFollowedBy=savedPos;
		savedPos=pos;
		break;
	    case '{':
		/* Since skipend being set here would indicate a syntax error in the code, we assume it isn't. */
		/* fall through */
	    case '}':
		if(skipend)
		    pos= BackwardSkipDelimited(pos,'{','}');
		/* fall through */
	    case ';':
	    case EOF:
		if(skipend){
		    skipend=FALSE;
		    savedPos=pos;
		}else if(onestatement){
		    int cin=CurrentIndent(savedPos+1);
		    /* savedPos set at beginning */
		    if(braceFlag)
			return cin+braceExtra;
		    else if(cin==0)
			return 0;
		    else if(commaFlag)
			return cin;
		    else
			return cin +this->srctext::contIndent;
		}else { /* CAN'T get here until after something that sets savedPos */
		    /* first make sure the PREVIOUS line didn't ALREADY have 'labelExtra' added to it */
		    long tpos= SkipWhitespace(GetBeginningOfLine(savedPos+1), GetLength());
		    if(match(this,tpos,"case") || match(this,tpos,"default"))
			labelExtra+= this->switchLabelUndent;
		    return CurrentIndent(savedPos+1)+labelExtra+braceExtra;
		}
		break;
	    case ')':
		pos= BackwardSkipDelimited(pos,'(',')');
		/* save to do continuation lines correctly */
		parensFollowedBy=savedPos;
		savedPos=pos;
		break;
	    case ']':
		pos= BackwardSkipDelimited(pos,'[',']');
		savedPos=pos;
		break;
	    case '(':
	    case '[':
		return CurrentColumn(SkipWhitespace(pos+2, savedPos+1));
	    default:
		savedPos=pos;
	}
    }
}
