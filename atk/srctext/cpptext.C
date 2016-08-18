/* File cpptext.C created by R L Quinn
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

   cpptext, an object for editing C++ code. */

#include <andrewos.h>

static UNUSED const char ibmid[] = "(c) Copyright IBM Corp.  1988-1995.  All rights reserved.";
static UNUSED const char rcsHeader[] = "$Header: /afs/cs.cmu.edu/project/atk-src-C++/atk/srctext/RCS/cpptext.C,v 2.1 1995/02/07 17:37:27 rr2b Stab74 $";

#include <environment.H>
#include <stylesheet.H>
#include <style.H>
#include <fontdesc.H>
#include <attribs.h>
#include <toolcnt.h>  

#include "srctext.H"
#include "cpptext.H"

static Dict *words[TABLESIZE];


ATKdefineRegistry(cpptext, ctext, cpptext::InitializeClass);


void cpptext::SetAttributes(struct attributes *atts)
{
    (this)->ctext::SetAttributes(atts);
    while (atts!=NULL) {
	if (strcmp(atts->key,"acsctrl-outdent")==0)
	    this->acsctrlOutdent=atoi(atts->value.string);
	atts=atts->next;
    }
}

static void SetupStyles(cpptext *self)
{
    if ((self->srctext::kindStyle[CLASS]= (self->text::styleSheet)->Find("class")) == NULL) {
	self->srctext::kindStyle[CLASS]= new style;
	(self->srctext::kindStyle[CLASS])->SetName("class");
	(self->srctext::kindStyle[CLASS])->AddNewFontFace(fontdesc_Bold);
	(self->srctext::kindStyle[CLASS])->SetFontSize(style_PreviousFontSize, 2);
	(self->text::styleSheet)->Add(self->srctext::kindStyle[CLASS]);
    }
}

void cpptext::SetupStyles()
{
    (this)->ctext::SetupStyles();
    ::SetupStyles(this);
}

boolean cpptext::InitializeClass()
{
    static Dict cppkeywords[]={
	{"asm",0,KEYWRD},
	{"auto",0,KEYWRD},
	{"break",0,KEYWRD},
	{"case",0,KEYWRD},
	{"catch",0,KEYWRD},
	{"char",0,KEYWRD},
	{"class",CLASS_BIT,KEYWRD},
	{"const",0,KEYWRD},
	{"continue",0,KEYWRD},
	{"default",0,KEYWRD},
	{"delete",0,KEYWRD},
	{"do",0,KEYWRD},
	{"double",0,KEYWRD},
	{"else",0,KEYWRD},
	{"enum",0,KEYWRD},
	{"extern",0,KEYWRD},
	{"float",0,KEYWRD},
	{"for",0,KEYWRD},
	{"friend",0,KEYWRD},
	{"goto",0,KEYWRD},
	{"if",0,KEYWRD},
	{"inline",0,KEYWRD},
	{"int",0,KEYWRD},
	{"long",0,KEYWRD},
	{"new",0,KEYWRD},
	{"operator",0,KEYWRD},
	{"overload",0,KEYWRD}, /* in FIRST, but not SECOND edition of Lippman's C++ Primer */
	{"private",ACSCTRL_BIT,KEYWRD},
	{"protected",ACSCTRL_BIT,KEYWRD},
	{"public",ACSCTRL_BIT,KEYWRD},
	{"register",0,KEYWRD},
	{"return",0,KEYWRD},
	{"short",0,KEYWRD},
	{"signed",0,KEYWRD},
	{"sizeof",0,KEYWRD},
	{"static",0,KEYWRD},
	{"struct",CLASS_BIT,KEYWRD}, /* synonym for "class", hence CLASS_BIT is set */
	{"switch",0,KEYWRD},
	{"this",0,KEYWRD},
	{"template",TEMPLATE_BIT,KEYWRD},
	{"throw",0,KEYWRD}, /* in SECOND, but not FIRST edition of Lippman's C++ Primer */
	{"try",0,KEYWRD},
	{"typedef",0,KEYWRD},
	{"union",0,KEYWRD},
	{"unsigned",0,KEYWRD},
	{"virtual",0,KEYWRD},
	{"void",0,KEYWRD},
	{"volatile",0,KEYWRD},
	{"while",0,KEYWRD},
	{NULL,0,0}
    };
    srctext::BuildTable("cpptext",::words,cppkeywords);
    return TRUE;
}

cpptext::cpptext()
{
    ATKinit;

    this->srctext::words= (Dict **)::words;
    this->acsctrlOutdent= 2;
    this->srctext::linecommentString= "// ";
    ::SetupStyles(this);
    ToolCount("EditViews-cpptext",NULL);
    THROWONFAILURE(TRUE);
}

/* override */
/* cpptext_ReflowComment is identical to srctext's, but it will also flow double-slash comments.  pos is the beginning of the line we want to try to flow into the previous one */
boolean cpptext::ReflowComment(long pos)
{
    long startcomment=InCommentStart(pos);
    long skipws= SkipWhitespace(pos, GetLength());
    if (GetChar(skipws) == '\n')
	/* don't flow it if it's a blank line */
	return FALSE;
    if (startcomment && startcomment<pos) {
	/* this line IS a continued comment; flow it with the previous line */
	ReplaceCharacters(pos-1, skipws-pos+1, " ",1);
	return TRUE;
    } else if ((startcomment= InCommentStart(skipws+1)) > 0) {
	/* this line contains a separate comment that might be appropriate to flow with the previous comment */
	style *commenttype=GetStyle(skipws+1);
	if (commenttype==this->srctext::linecomment_style) {
	    int startcol=CurrentColumn(startcomment);
	    if (is_whitespace(GetChar(startcomment+2)) && GetStyle(pos-1)==commenttype && (GetEnvironment(pos-1))->GetLength() > 2) {
		/* this is a flowable double-slash comment and there's a double-slash comment on the previous line too (len>2); flow them together */
		int spaces=1;
		while (is_whitespace(GetChar(startcomment+2+spaces))) ++spaces;
		ReplaceCharacters(pos-1, startcomment+3+spaces-pos, " ",1);
		CheckLinecomment((GetEnvironment(pos-1))->Eval());
		return TRUE;
	    }
	}
    }
    return FALSE;
}

/* override */
/* BreakLine breaks up a line so it fits within the specified max-length. endofline is a mark (so it stays put during reindenting) pointing to the newline at the end of the line to be broken. */
void cpptext::BreakLine(mark *endofline)
{
    long end;
    int c;
    boolean brokebang;
    int ccol= CurrentColumn((endofline)->GetPos()-1);
    int prevlen= ccol+1; /* add 1 to ensure at least 1 iteration of while loop */
    /* ccol & prevlen keep track of how long the remainder of the line IS and WAS */
    /* this prevents infinite loops when a 'word' is too long to break up right */
    while (ccol>=GetMaxLineLength() && ccol<prevlen) {
	/* line too long; fragment it */
	long stopline= (endofline)->GetPos();
	int curcol= ccol+1;
	do  {
	    --stopline;
	    c= GetChar(stopline);
	    if (c=='\t') /* recalculate */
		curcol= CurrentColumn(stopline);
	    else /* decrement (it's a LOT quicker!) */
		--curcol;
	} while (c!='\n' && stopline>0 && (!Breakable(stopline) || curcol>GetMaxLineLength()));
	if (stopline<1 || c=='\n')
	    return; /* never mind */
	brokebang= (GetStyle(stopline)==this->srctext::linecomment_style);
	end= SkipWhitespace(stopline,GetLength());
	while (stopline>0 && is_whitespace(GetChar(stopline-1)))
	    --stopline;
	if (stopline<end)
	    DeleteCharacters(stopline,end-stopline);
	/* pretend there's a blank line so it uses 'desired' _Indentation */
	JustInsertCharacters(stopline,"\n\n",2);
	if (brokebang) {
	    /* remove style from the broken part */
	    environment *bangstyle= GetEnvironment(stopline);
	    long bangstart= (bangstyle)->Eval();
	    int bangcol= CurrentColumn(bangstart);
	    int textoffset= CurrentColumn(SkipWhitespace(bangstart+2, stopline)) - bangcol;
	    (this->text::rootEnvironment)->Remove(stopline, (bangstyle)->GetLength() + bangstart - stopline, environment_Style, FALSE);
	    ReplaceCharacters(stopline+1,1, "// ",3);
	    CheckLinecomment(stopline+1);
	    TabAndOptimizeWS(stopline+1,bangcol);
	    if (textoffset>2)
		/* reindent *within* comments to make a straight left margin for paragraph */
		TabAndOptimizeWS(SkipWhitespace(stopline+1, GetLength()) +2, bangcol+textoffset);
	    /* reindent it to line up with the previous bang */
	} else {
	    /* reindent the "blank line" */
	    ReindentLine(stopline+1);
	    stopline= SkipWhitespace(stopline+1, GetLength());
	    DeleteCharacters(stopline, 1); /* remove extra newline */
	    /* reindent *again* in case we have to right-justify an end-of-comment */
	    ReindentLine(stopline);
	}
	prevlen= ccol;
	ccol= CurrentColumn((endofline)->GetPos()-1);
    }
}

/* skipJunk finds the position of the next "significant" character */
static long skipJunk(cpptext *self, long pos, long len)
{
    int ch=(self)->GetChar(pos++), prev;
    int incomment=FALSE;
    while (pos<len) {
	prev= ch;
	ch= (self)->GetChar(pos++);
	if (incomment) {
	    if (prev=='*' && ch=='/') {
		incomment= FALSE;
		ch= (self)->GetChar(pos++);
	    }
	} else if (prev=='/') {
	    if (ch=='*')
		incomment= TRUE;
	    else if (ch=='/') {
		pos= (self)->GetEndOfLine(pos)+1;
		ch= '\n';
	    }
	} else
	    if (prev>' ')
		return pos-2;
    }
    return len;
}

static boolean isOperatorOverload(cpptext *self, char ch)
{
    return (strchr("!%&,^|~()[]*/+-<=>",ch) != NULL);
}

static void fn_name(cpptext *self, long posn, long backtoofar)
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
    /* Check for any operator overload characters */
    while (posn>backtoofar && isOperatorOverload(self,c=(self)->GetChar(posn))) {
	--posn;
	if (c==')' && (self)->GetChar(posn)!='(')
	    /* the "function name" is in parens, so it must be some global pointer to a function. */
	    return;
    }
    /* Check for fctn name tokens, or possibly an operator like "new" or maybe the keyword "operator" */
    while (posn>backtoofar && (self)->IsTokenChar((self)->GetChar(posn)))
	--posn;
    /* Possible to have destructor op at this point */
    if ((self)->GetChar(posn) == '~')
	--posn;
    { /* Make "operator" keyword part of function name, if present */
	long endop=posn;
	while (is_whitespace((self)->GetChar(endop))) --endop;
	if (endop>8 && (self)->Strncmp(endop-7, "operator",8)==0)
	    posn= endop-8;
    }
    /* Test for scope resolution operator(s).  Include it and the class name if present. Also grab <template args> */
    while (posn>backtoofar && (self)->GetChar(posn)==':' && (self)->GetChar(posn-1)==':') {
	boolean inTplArgList= FALSE;
	posn-= 2;
	while (posn>backtoofar && ((self)->IsTokenChar(c=(self)->GetChar(posn)) || c=='<' || c=='>' || (/* not so picky if inside <template arg list> */ inTplArgList && (is_whitespace(c) || c==',')))) {
	    --posn;
	    if (c=='<') inTplArgList= FALSE;
	    else if (c=='>') inTplArgList= TRUE;
	}
    }
    /* if we see an assignment in front of all this, it means it's a fctn CALL initializing a global var */
    if ((self)->GetChar((self)->BackwardSkipJunk(posn)) == '=')
	return;
    if (posn<end)
	(self)->WrapStyle(posn+1,end-posn, self->srctext::function_style, FALSE,TRUE);
}

/* static global data shared between RedoStyles and CheckWord */
static boolean inClassDef; /* TRUE to flag that "class" keyword was found.  reset to FALSE once inside definition itself */
static int inTplArgList; /* -1 to flag that "template" keyword was found.  + values count <> nesting */ /* this check is probably unnecessary, since the keyword "class" only implies a class decl if it has a : or { after the class name */

void cpptext::RedoStyles()
{
    long posn, len=GetLength(), readyForFName=1;
    int prev=0, c='\n', braces=0; /* c is initialized to a newline so the start of the file looks like the start of line. */
    boolean escape=FALSE, inClassDefBraces[64];
    ::inClassDef= inTplArgList= FALSE;
    RemoveStyles(); /* Remove the old styles, but leave the root environment in place. */
    for (posn= 0; posn<len-1; ++posn) {
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
		    if (prev == '/') {
			posn = CheckComment(posn-1);
			if (readyForFName)
			    readyForFName= posn;
		    }
		    break;
		case '/':
		    if (prev=='/') {
			posn= CheckLinecomment(posn-1);
			if (readyForFName)
			    readyForFName= posn;
		    }
		    break;
		case '{':
		    ++braces;
		    if (::inClassDef) {
			if (braces<64) inClassDefBraces[braces]= TRUE;
			readyForFName= posn;
			::inClassDef= FALSE;
		    } else { /* this must be the open brace for a function definition; quit looking for declarations */
			readyForFName= FALSE;
			if (braces<64) inClassDefBraces[braces]= FALSE;
		    }
		    break;
		case '}':
		    --braces;
		    if (braces<64 && inClassDefBraces[braces]) {
			/* end brace of member function definition */
			readyForFName= posn;
		    }
		    if (braces<=0) {
			braces= 0;
			readyForFName= posn;
		    }
		    break;
		case '<':
		    if (inTplArgList)
			inTplArgList= (inTplArgList<0) ? 1 : (inTplArgList-1);
		    break;
		case '>':
		    if (inTplArgList)
			--inTplArgList;
		    break;
		case '(':
		    if (readyForFName && (!braces || (braces<64 && inClassDefBraces[braces])))
			fn_name(this,posn,readyForFName);
		    ++braces;
		    if (braces<64) inClassDefBraces[braces]= FALSE;
		    break;
		case ')':
		    --braces;
		    if (braces<0) braces= 0;
		    if (GetChar(skipJunk(this, posn+1,len))==':')
			/* some inline variable initializations are coming up, and they only LOOK like function names! */
			readyForFName= FALSE;
		    break;
		case '[':
		    ++braces; /* kludge to prevent styling [sizeof()] as a function name */
		    if (braces<64) inClassDefBraces[braces]= FALSE;
		    break;
		case ']':
		    --braces;
		    if (braces<0) braces= 0;
		    break;
		case ';':
		    if (::inClassDef) /* no braces were found after the "class" keyword */
			::inClassDef= FALSE;
		    if (!braces || (braces<64 && inClassDefBraces[braces]))
			readyForFName= posn;
		    break;
		case '\'':
		case '\"':
		    posn= CheckString(posn);
		    break;
		case ':':
		    if (prev!=':' && GetChar(posn+1)!=':') /* it's not a scope resolution operator */
			BackwardCheckLabel(posn);
		    break;		    
		default:
		    if (IsTokenChar(c))
			posn= CheckWord(posn,len);
		    break;
	    }
    }
}

/* override */
/* cpptext_CheckWord does everything that srctext's would have, but also checks for class names */
long cpptext::CheckWord(long i, long end)
{
    long j, start, endstr;
    long filelen=GetLength();
    Dict *word;
    char c, buff[256];
    i=SkipWhitespace(i,end);
    j=CopyWord(i,end,buff);
    Keywordify(buff,FALSE);
    if ((word=srctext::Lookup(this->srctext::words,buff))->stng!=NULL) {
	WrapStyle(i,strlen(word->stng), this->srctext::kindStyle[word->kind], FALSE,FALSE);
	/* Check for class definition keyword */
	if (((word->val) & CLASS_BIT) && !inTplArgList) {
	    /* it's a "class" or "struct" keyword, and isn't inside a <template-argument-list> */
	    start=endstr=SkipWhitespace(j+1,end);
	    while((endstr<filelen) && (IsTokenChar(GetChar(endstr)))) ++endstr;
	    if (endstr>start && ((c=GetChar(skipJunk(this,endstr,end)))==':' || c=='{' || c==';')) {
		/* it must be a real class decl if it's derived, or has a defn after it, or is just a decl */
		WrapStyle(start,endstr-start, this->srctext::kindStyle[CLASS], TRUE,TRUE);
		j=endstr-1;
		::inClassDef=TRUE;
	    }
	}
	if ((word->val) & TEMPLATE_BIT)
	    inTplArgList= TRUE;
    }
    return j;
}

/* override */
/* cpp's RedoStyles won't call this for scope resolution operators (::), but it DOES still get called when user TYPES a colon */
void cpptext::BackwardCheckLabel(long pos)
{
    if (GetChar(pos-1)!=':')
	(this)->ctext::BackwardCheckLabel(pos);
    else { /* uh-oh. assume we mistakenly styled it as a label when that PRECEDING colon was typed. UN-style it */
	long start=pos-1;
	while (is_whitespace(GetChar(--start))) ;
	if (GetStyle(start)==this->srctext::label_style) {
	    start= GetBeginningOfLine(start);
	    (this->text::rootEnvironment)->Remove(start,pos-start, environment_Style,TRUE);
	    RegionModified(start,pos-start);
	}
    }
}

/* returns the length of a *constant* string */
#define cstrlen(str) (sizeof(str)-1)

/* pos should pointer to 2nd from last character-- last character won't be
 * matched; strconst *must* be a constant string, so that sizeof works
 */
#define backwardMatch(self,pos,strConst) \
    (pos>=cstrlen(strConst)-1 && match(self,pos-(cstrlen(strConst)-1)+1,strConst))

#define match(self,pos,str) ((pos==0 || !(self)->IsTokenChar((self)->GetChar(pos-1))) && !(self)->IsTokenChar((self)->GetChar(pos+cstrlen(str))) && (self)->DoMatch(pos,str,cstrlen(str)))

/* Indentation returns the proper indentation of the line that starts at pos */
int cpptext::Indentation(long pos)
{
    int c;
    long savedPos=(-1), parensFollowedBy=(-1), colonFollowedBy=(-1);
    int elseCatch=0;
    boolean onestatement=TRUE, skipend=FALSE, braceFlag=FALSE, commaFlag=FALSE;
    int braceExtra=0, levelExtra=this->srctext::levelIndent,
    switchLevelExtra=this->ctext::switchLevelIndent, labelExtra=0;

    /* avoid potential problems with first line of file */
    if (pos<1)
	return 0;

    /* see if indenting any special cases */
    {
	long newpos;
	if ((newpos=InCommentStart(pos)) > 0 && GetStyle(pos) != this->srctext::linecomment_style) /* does NOT apply for //-comments! */
	    return CurrentColumn(newpos) + this->srctext::commentIndent;
	for (newpos=pos; (c=GetChar(newpos))==' ' || c=='\t'; ++newpos) ;
	switch(c){
	    case '}':
		braceFlag=TRUE; /* kluge ? */
		pos= BackwardSkipDelimited(newpos-1,'{','}') +1;
		/* fall through */
	    case '{':
		braceFlag=TRUE; /* kluge ? */
		levelExtra=this->ctext::braceIndent;
		switchLevelExtra=this->ctext::braceIndent;
		break;
	    case '#':
		if (this->ctext::outdentPreproc)
		    return 0;
		break;
	    case 'e':
		if(match(this,newpos,"else"))
		    elseCatch=1;
		break;
	    case 'c':
	    case 'd':
		if(match(this,newpos,"case") || match(this,newpos,"default"))
		    labelExtra= -this->ctext::switchLabelUndent;
		break;
	    case 'p':
		if (match(this,newpos,"private") || match(this,newpos,"public") || match(this,newpos,"protected"))
		    labelExtra= -(this->acsctrlOutdent);
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
		if (GetStyle(pos)==this->srctext::linecomment_style)
		    pos= InCommentStart(pos)-1;
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
			return CurrentIndent(pos) + this->ctext::switchLevelIndent - (this->ctext::switchLevelIndent - this->ctext::switchLabelUndent) + labelExtra;
		    else
			return CurrentIndent(pos) + (this->ctext::switchLevelIndent - this->ctext::switchLabelUndent) + this->srctext::contIndent;
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
	    case 's':
		if (onestatement && backwardMatch(this,pos,"class"))
		    return CurrentIndent(pos) +braceExtra +labelExtra;
		savedPos=pos;
		break;
	    case 't':
		if(backwardMatch(this,pos,"default") && !IsTokenChar(GetChar(pos+2)))
		    if(!onestatement || colonFollowedBy==-1)
			return CurrentIndent(pos) + this->ctext::switchLevelIndent - (this->ctext::switchLevelIndent - this->ctext::switchLabelUndent) + labelExtra;
		    else
			return CurrentIndent(pos) + (this->ctext::switchLevelIndent - this->ctext::switchLabelUndent) + this->srctext::contIndent;
		savedPos=pos;
		break;
	    case ',':
		/* this is a kluge, which assumes that commas occuring outside of parens
		   imply a separate statement (i.e., equivalent indentation).  This is mostly true.*/
		commaFlag=TRUE;
		savedPos=pos;
		break;
	    case ':':
		if(backwardMatch(this,pos,"private:") ||  backwardMatch(this,pos,"public:") || backwardMatch(this,pos,"protected:"))
		    return CurrentIndent(pos)+(this->acsctrlOutdent)+labelExtra+braceExtra;
		/* make sure this is a ':' and not a '::' (C++ only) */
		if (GetChar(pos)==':' || GetChar(pos+2)==':')
		    break;
		colonFollowedBy=savedPos;
		savedPos=pos;
		break;
	    case '{':
		/* Since skipend being set here would indicate a syntax error in the code, we assume it isn't.*/
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
			labelExtra+= this->ctext::switchLabelUndent;
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
