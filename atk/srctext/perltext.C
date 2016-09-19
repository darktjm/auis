/* File perltext.C created by R S Kemmetmueller
   (c) Copyright IBM Corp.  1988-1995.  All rights reserved.

   perltext, an object for editing perl code. */

#include <andrewos.h>

static UNUSED const char ibmid[] = "(c) Copyright IBM Corp.  1988-1995.  All rights reserved.";

#include <stylesheet.H>
#include <style.H>
#include <fontdesc.H>
#include <attribs.h>
#include <toolcnt.h>
#include <environment.H>

#include "srctext.H"
#include "perltext.H"

static Dict *words[TABLESIZE];

ATKdefineRegistry(perltext, srctext, perltext::InitializeClass);

boolean perltext::IsTokenChar(char ch)
{
    return (isalnum(ch) || ch=='_');
}

static void SetupStyles(perltext *self)
{
    if ((self->srctext::kindStyle[KEYWRD]= (self->text::styleSheet)->Find("keyword")) == NULL) {
	self->srctext::kindStyle[KEYWRD]= new style;
	(self->srctext::kindStyle[KEYWRD])->SetName("keyword");
	(self->srctext::kindStyle[KEYWRD])->AddNewFontFace(fontdesc_Bold);
	(self->text::styleSheet)->Add(self->srctext::kindStyle[KEYWRD]);
    }
    self->srctext::kindStyle[STRING]=NULL;
    self->srctext::kindStyle[NONE]=NULL;
}	

void perltext::SetupStyles()
{
    (this)->srctext::SetupStyles();
    ::SetupStyles(this);
}

boolean perltext::InitializeClass()
{
    static const Dict perlkeywords[]={
	{"continue",0,KEYWRD},
	{"else",0,KEYWRD},
	{"elsif",0,KEYWRD},
	{"eq",COMPARE_STR_BIT,NONE},
	{"for",0,KEYWRD},
	{"foreach",0,KEYWRD},
	{"ge",COMPARE_STR_BIT,NONE},
	{"gt",COMPARE_STR_BIT,NONE},
	{"if",STR_FOLLOW_KEYWD,KEYWRD},
	{"last",0,KEYWRD},
	{"le",COMPARE_STR_BIT,NONE},
	{"lt",COMPARE_STR_BIT,NONE},
	{"m",TWO_DELIM_BIT,STRING},
	{"ne",COMPARE_STR_BIT,NONE},
	{"next",0,KEYWRD},
	{"q",TWO_DELIM_BIT,STRING},
	{"qq",TWO_DELIM_BIT,STRING},
	{"redo",0,KEYWRD},
	{"return",0,KEYWRD},
	{"s",THREE_DELIM_BIT,STRING},
	{"sub",SUB_BIT,KEYWRD},
	{"tr",THREE_DELIM_BIT,STRING},
	{"unless",STR_FOLLOW_KEYWD,KEYWRD},
	{"until",STR_FOLLOW_KEYWD,KEYWRD},
	{"while",STR_FOLLOW_KEYWD,KEYWRD},
	{"y",THREE_DELIM_BIT,STRING},
	{NULL,0,KEYWRD}
    };
    srctext::BuildTable("perltext",::words,perlkeywords);
    return TRUE;
}

perltext::perltext()
{
    ATKinit;
    this->srctext::words= (Dict **)::words;
    this->srctext::linecommentString= "# "; 
    this->srctext::levelIndent= 4;
    this->srctext::contIndent=2;
    this->srctext::indentingEnabled= TRUE;
    ::SetupStyles(this);
    ToolCount("EditViews-perltext",NULL);
    THROWONFAILURE(TRUE);
}

static long backwardSkipJunk(perltext *self, long pos)
{
    long startcomment;
    while (pos>0) {
	int c=(self)->GetChar(pos--);
	switch(c) {
	    case '\n':
		if ((startcomment=(self)->InCommentStart(pos+1)) > 0)
		    pos= startcomment-1;
	    case ' ':		
	    case '\t':
		/* nothing */
		break;
	    default:
		if ((startcomment=(self)->InCommentStart(pos+1)) > 0) {
		    pos= startcomment-1;
		    break;
		} else if ((self)->InString(pos+1)) {
		    pos= (self)->BackwardSkipString(pos+1, c);
		    break;
		}
		return pos+1;
	}
    }
    return -1;
}

/* assumes ending delimiter AFTER pos */
static long backwardSkipDelimited(perltext *self, long pos, char beg, char end)
{
    int level=1;
    long startcomment;
    while (pos>=0 && level>0){
	char c;
	if ((self)->InString(pos)) 
	    pos = (self)->BackwardSkipString(pos, 0);
	else if ((startcomment=(self)->InCommentStart(pos)) > 0)
	    pos= startcomment-1;
	else if ((c= (self)->GetChar(pos--))==beg) --level;
	else if (c==end) ++level;
    }
    return pos;
}

static long backwardSkipWhitespace(perltext *self, long pos)
{
    int c;

    while ((pos > 0) && (is_whitespace(c=(self)->GetChar(pos)) || c == '\n')) --pos;
    return pos;
}

/* returns true iff the character at pos is quoted (ie. "\"). Takes into account the slash being quoted. (ie "\\"). */
boolean perltext::Quoted(long pos)
{
    boolean retval = FALSE;
    while (pos-->0 && GetChar(pos)=='\\') 
	retval= !retval;
    return retval;
}


/* function is an override from srctext.c.  It is necessary since perl can have many
   different forms of a string.  Here are some examples of strings that can be used in perl:
   "this is a normal string", 'c', `str`, <here is a string>, qq#and another#,
   <<"hi" there are weird \n hi \n.  Those where just a few (perl strings can be nasty).
   It cannot be easily determined wether you are in a string or not.  Don't
   assume you are in a string unless the previous char is in a string.*/
boolean perltext::InString(long pos) 
{
    return (GetStyle(pos) == this->srctext::string_style);
}

long perltext::BackwardSkipString(long pos, char ch)
{
    if (!this->InString(pos))
	return pos;
    pos = (GetEnvironment(pos))->Eval() -1;
    return pos;
}

void perltext::BackwardCheckWord(long from, long to)
{
    Dict *word;
    char buff[256], c;
    long j=from;
    if (IsTokenChar(GetChar(from)) && !GetStyle(from) && !InString(from)) {
	j= BackwardCopyWord(from,to,buff);
	c = GetChar(j);
	if (c == '#') c='$';			/* watch for strings of the form  $#blah */
	if (strchr(VARIABLE_START, c) && c != 0) return;
	if ((word=srctext::Lookup(this->srctext::words,buff))->stng!=NULL) {
	    int l= strlen(buff);
	    if (GetForceUpper())
		ReplaceCharacters(j+1,l, word->stng,l);
	    WrapStyleNow(j+1,l, this->srctext::kindStyle[word->kind], FALSE,FALSE);
	}
    }
    return /*j*/;
}

static long my_CheckWord(perltext *self, long i, long end, int state /* boolean:  0 if not in a block, 1 if in a block */)
{
    long j;
    Dict *word;
    char buff[256], delim, ch;
    i=(self)->SkipWhitespace(i,end);
    j=(self)->CopyWord(i,end,buff);
    if ((word=srctext::Lookup(self->srctext::words,buff))->stng != NULL) {
	(self)->WrapStyle(i,strlen(word->stng), self->srctext::kindStyle[word->kind], FALSE,FALSE);

	if ((word->val & SUB_BIT) && !state) {
	    i=(self)->SkipWhitespace(j+1,end);
	    j=(self)->CopyWord(i,end,buff);
	    (self)->WrapStyle(i,strlen(buff), self->srctext::function_style, TRUE, TRUE);
	} else if (word->val & (TWO_DELIM_BIT | THREE_DELIM_BIT)) {
	    delim=(self)->GetChar(++j);
	    while (j < end && ((ch = (self)->GetChar(++j)) != delim  || (self)->Quoted(j)));
	    if (j<end && word->val & THREE_DELIM_BIT)
		while (j<end && ((ch = (self)->GetChar(++j)) != delim || (self)->Quoted(j)));
	    (self)->WrapStyle(i, j-i+1, self->srctext::string_style, FALSE, FALSE);
	}
    }
    return j;
}

static long copyString(perltext *self, long pos, char buff[])
{
    int delim, i=0;
    long len = (self)->GetLength();

    delim = (self)->GetChar(pos);
    while (pos < len)
	if ((buff[i++] = (self)->GetChar(++pos)) == delim)
	    break;
    buff[i-1] = 0;
    return pos;
}

/* CheckString will wrap the string style around strings and character constants.*/
long perltext::CheckString(long start)
{
    long end=start, len=GetLength(), pos;
    int delim=GetChar(start), ch;
    char delim_string[256];


    if (delim == '<')
    {
	if (GetChar(start+1) == '<' &&
	    ((ch = GetChar(start+2)) == '\'' ||
	     ch == '\"' || ch == '`'))
	{/* get delimiter string */
	    pos = copyString(this, start+2, delim_string);
	    /* get eol and style a comment if it is there */
	    while ((ch = GetChar(++pos)) != '\n')
		if (ch == '#') {
		    WrapStyle(start, pos-start, this->srctext::string_style,FALSE,FALSE);
		    start = CheckLinecomment(pos);
		}
	    end = start+1;
	    while (end < len && Strncmp(end+1, delim_string, strlen(delim_string)))  {
		end = GetEndOfLine(end+1);
	    }
	    if (end < len)
		WrapStyle(start,end+strlen(delim_string)+1 -start, this->srctext::string_style, FALSE,FALSE);
	    return end+strlen(delim_string);
	}
	delim = '>';
    }

    while (end<len)
    {
	if ((ch = GetChar(++end))==delim && !Quoted(end))
	    break;
    }
    WrapStyle(start,end-start+1, this->srctext::string_style, FALSE,FALSE);
    return end;
}


void perltext::RedoStyles()
{
    long posn, len=GetLength();
    int prev=0, c=0, d, parens=0, newpos=0, oldpos;
    char buff[256];
    Dict *word;

    RemoveStyles(); /* Remove the old styles, but leave the root environment in place. */

    for (posn=0; posn<len-1; ++posn) {
	oldpos = posn;			/* oldpos used to keep prev consistent */
	prev = c;
	c = GetChar(posn);
	switch (c) {
	    case '\n':
	    case ' ':
	    case '\t':
		break;
	    case '/':
	    /*case '<':*/ /* < and > delimit file handles, not strings, even though the spec lists this under "string" */
		newpos= backwardSkipWhitespace(this, posn-1);
		d = GetChar(newpos);
		switch (d) {
		    case '=' : 
		    case '(' :
		    case ',' :
		    case ';' :
		    case '{' :
		    case '}' :
		    case '~' :
		    case '&' :
		    case '|' :
			/* look for strings of the form: < blah > or
			   <<"blah";\n this is the string\nblah\n */
			posn = CheckString(posn);
			break;
		    default:
			BackwardCopyWord(newpos, 0, buff);
			word = srctext::Lookup(this->srctext::words, buff);
			if (word->stng && (word->val & (STR_FOLLOW_KEYWD | COMPARE_STR_BIT))) {
			    posn = CheckString(posn);
			}
		}
		break;
	    case '(':
	    case '{':
	    case '[':
		++parens;
		break;
	    case ')':
	    case '}':
	    case ']':
		--parens;
		break;
	    case ':':
		if (!parens) {
		    newpos = backwardSkipJunk(this,posn-1);
		    newpos = BackwardCopyWord(newpos,0,buff);
		    if ((word=srctext::Lookup(this->srctext::words,buff))->stng ==NULL)
			WrapStyle(newpos+1,strlen(buff), this->srctext::label_style, TRUE,TRUE);
		    else WrapStyle(newpos+1,strlen(buff), this->srctext::kindStyle[word->kind], FALSE,FALSE);
		}
		break;
	    case '\'':
	    case '\"':
	    case '`':
		if (prev != '$') {		/* if not $' or $" or $`   */
		    posn = CheckString(posn);
		}
		break;
	    case '#':
		if (prev != '$') {
		    posn= CheckLinecomment(posn);
		    break;
		}
		/* fall through  */

	    case '$':
	    case '%':
	    case '@':
	    case '*':
	    case '&':
		posn=CopyWord(posn+1,len,buff);
		if (posn < oldpos) posn = oldpos;
		break;

	    case '_':
		if (prev=='_' && Strncmp(posn+1, "END__",5)==0)
		    /* __END__ means end of program, so forget the rest exists */
		    return;
		break;
	    default:
		if (IsTokenChar(c) && prev!='-' /* the keyword "s" in "-s $foo" is NOT a string */)
		    posn = my_CheckWord(this,posn,len, parens);
	}
	/* if CheckWord read over more than one char then set c=0
	   so that prev will be set to 0 next time around.  There
	   would be a problem otherwise with "{$mock}" as it would
	   think that you intended something like "m}blah}" WRN */

	if (posn != oldpos) c = 0;
    }
}

static long currentStatementIndent(perltext *self, long pos, boolean continued)
{
    int ch;
    long savedPos=pos;

    while (pos >=0)
    {
	ch= (self)->GetChar(pos--);
	if ((self)->InString(pos+1))
	    pos= (self)->BackwardSkipString(pos,ch);
	else
	    switch (ch) {
		case ';':
		case '}':
		    return (self)->CurrentIndent(savedPos);
		case '{':
		    return (self)->CurrentIndent(pos) + self->srctext::levelIndent;
		case ')':
		    pos= backwardSkipDelimited(self,pos,'(',')');
		    break;
		case '(':
		    if (continued)
			return (self)->CurrentColumn(pos+1) - self->srctext::contIndent + 1;
		    else
			return (self)->CurrentColumn(pos+1) + 1;
		case '\n':
		    pos= backwardSkipJunk(self,pos);
		    break;
		default:
		    savedPos = pos+1;
		    pos=backwardSkipJunk(self,pos);
	    }
    }
    return 0;
}

/* Indentation returns the proper indentation of the line that starts at pos */
int perltext::Indentation(long pos) /* WRN, Oct.92 */
{
    long newpos;
    int addingChar=0, ch;

    /* avoid potential problems with first line of file */ 
    if (pos<1)
	return 0;
    if (InString(pos-1))
	return 0;
    /* see if indenting any special cases */
    newpos = pos;
    while ((addingChar=GetChar(newpos++)) == ' ' || addingChar =='\t');
    if (addingChar=='}')
	return CurrentIndent(backwardSkipDelimited(this,pos-1,'{','}')+1);
    /* if it is a continued line . . . */
    if (((pos = backwardSkipJunk(this,pos-1)) != -1)
	&& (ch= GetChar(pos)) != ';'
	&& ch !='}'
	&& ch !='{'
	&& addingChar != '{')
	return currentStatementIndent(this,pos,TRUE) + this->srctext::contIndent;
    if (InString(newpos-1))
	return 0;
    for (;;)
    {
	if (pos < 0)
	    return 0;
	else if (InString(pos)) {	    
	    pos = BackwardSkipString(pos, 0);
	    continue;
	} else 
	    ch = GetChar(pos--);
	switch (ch) {
	    case '{':
		return (CurrentIndent(pos) + this->srctext::levelIndent);
	    case '}':
		pos= backwardSkipDelimited(this,pos,'{','}');
		/* fall through */
	    case ';':
		return currentStatementIndent(this,pos,FALSE);
	    case ' ':
	    case '\t':
	    case '\n':
		break;
	    case ')':
		pos= backwardSkipDelimited(this,pos,'(',')');
		break;
	    case '(':
		return CurrentColumn(pos+1) + 1;
	    default:
		break;
	}
    }
}


/* Paren Balancer data structures */
struct paren_node {
    long type;
    struct paren_node *next;
};
#define new_c(type) (type *) malloc(sizeof(type))

long perltext::ReverseBalance(long pos)
{
    boolean atleastone=FALSE;
    int thischar=0;
    const char *parentype;
    struct paren_node *parenstack=NULL, *temp_stack_ptr;
    static const char opens[]="({[", closes[]=")}]";

    while ((parenstack !=NULL || !atleastone) && (pos>0)) {
	thischar= GetChar(--pos);

	if ((parentype=strchr(opens,thischar)) != NULL) {
	    if (InCommentStart(pos) || InString(pos))
		/* we're in a comment or string. Ignore character! */
		continue;
	    if (thischar != '{' && GetChar(pos-1)=='$')
		continue;	/* don't match $[ with ]   or $( with ) */

	    if (parenstack == NULL) return EOF;			/* atleatone must be FALSE */

	    if (parenstack->type != (parentype-opens))	break;	/* doesn't match top of stack */

	    /* found a match so pop the stack */
	    temp_stack_ptr=parenstack;
	    parenstack= parenstack->next;
	    free(temp_stack_ptr);
	} else if ((parentype=strchr(closes, thischar)) != NULL) {
	    if (InCommentStart(pos) || InString(pos))
		/* we're in a comment or string. Ignore character! */
		continue;
	    if (thischar != '}' && GetChar(pos-1)=='$')
		continue;	/* don't push  $] on stack   or  $)  */

	    /* we've got  a close character.  push it on to the stack */
	    temp_stack_ptr = new_c(struct paren_node);
	    temp_stack_ptr->type= parentype - closes;
	    temp_stack_ptr->next= parenstack;
	    parenstack= temp_stack_ptr;
	    atleastone=TRUE;
	}
    }
    if (parenstack==NULL)
	return pos;
    else {
	/* empty the stack */
	while (parenstack != NULL) {
	    temp_stack_ptr = parenstack;
	    parenstack = parenstack->next;
	    free(temp_stack_ptr);
	}
	return EOF;
    }
}
