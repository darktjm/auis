/* File ptext.C
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

   ptext, an object for editing Pascal code. */

static char ibmid[] = "(c) Copyright IBM Corp.  1988-1995.  All rights reserved.";
static char rcsHeader[] = "$Header: /afs/cs.cmu.edu/project/atk-src-C++/atk/srctext/RCS/ptext.C,v 2.0 1995/01/27 19:38:38 rr2b Stab74 $";

/*
 * ptext, a Pascal mode for ATK
 *
 * Originally taken from C-Text by Miles Bader
 *  Made into a MODULA-2 editor by Rob Ryan, Chris Newman
 * Then completely re-done to work for Pascal by Curt McDowell
 * Made a subclass of srctext by Travis Michels
 */

#include <andrewos.h>
#include <setjmp.h>
#include <ctype.h>

#include <environment.H>
#include <stylesheet.H>
#include <style.H>
#include <fontdesc.H>

#include <attribs.h>
#include <toolcnt.h>

#include "srctext.H"
#include "ptext.H"

#define	TOUPPER(c) (islower(c) ? toupper(c) : (c))
#define	TOLOWER(c) (isupper(c) ? tolower(c) : (c))

#define PARENCOMMENT 0
#define BRACECOMMENT 1
#define SLASHCOMMENT 2

#define ID_INDENT       (1 << 0)
#define ID_BEGIN        (1 << 1)
#define ID_END          (1 << 2)
#define ID_DO           (1 << 3)
#define ID_LEAVE        (1 << 4)

static Dict *words[TABLESIZE];

ATKdefineRegistry(ptext, srctext, ptext::InitializeClass);

boolean ptext::IsTokenChar(char ch)
{
    return (isalnum(ch) || ch == '_');
}

static void casify(char *s, int style)
{
    if (style == idstyle_PLAIN || *s == '\0')
	return;
    if (style == idstyle_LOWER)
	*s = TOLOWER(*s);
    else
	*s = TOUPPER(*s);
    for (++s; *s != '\0'; ++s)
	if (style == idstyle_UPPER)
	    *s = TOUPPER(*s);
	else
	    *s = TOLOWER(*s);
}

void ptext::BackwardCheckWord(long from, long to)
{
    environment *me;
    Dict *word;
    char buf1[256], buf2[256];
    long j = from;
    if ((!InString(from)) &&
	IsTokenChar(GetChar(from)) &&
	(((me = GetEnvironment(from)) ==
	  this->text::rootEnvironment) ||
	 (me->data.style==this->srctext::kindStyle[KEYWRD]))) {
	j = BackwardCopyWord(from, to, buf1);
	Keywordify(buf1, TRUE);
	if ((word=srctext::Lookup(this->srctext::words, buf1))->stng != NULL) {
	    int l = strlen(buf1);
	    strcpy(buf2, buf1);
	    casify(buf2, GetForceUpper()?idstyle_UPPER:this->idStyle);
	    ReplaceCharacters(j + 1, l, buf2, l);
	    WrapStyleNow(j+1, l, this->srctext::kindStyle[word->kind], FALSE, FALSE);
	}
    }
    return;
}

void ptext::SetAttributes(struct attributes *atts)
{
    (this)->srctext::SetAttributes(atts);
    while (atts!=NULL) {
	char *key=atts->key;
	if(strcmp(key,"case-level-indent")==0)
	    this->caseLevelIndent=atoi(atts->value.string);
	else if(strcmp(key, "slash-asterisk-comments")==0)
	    this->slashAsteriskComments=atoi(atts->value.string);
	else if(strcmp(key,"id-style")==0) {
	    if (strcmp(atts->value.string, "upper")==0)
		this->idStyle = idstyle_UPPER;
	    else if (strcmp(atts->value.string, "lower")==0)
		this->idStyle = idstyle_LOWER;
	    else if (strcmp(atts->value.string, "capit")==0)
		this->idStyle = idstyle_CAPIT;
	    else
		this->idStyle = idstyle_PLAIN;
	}
	atts=atts->next;
    }    
}

static void SetupStyles(ptext *self)
{
    if ((self->srctext::kindStyle[KEYWRD]= (self->text::styleSheet)->Find("keyword")) == NULL) {
	self->srctext::kindStyle[KEYWRD]= new style;
	(self->srctext::kindStyle[KEYWRD])->SetName("keyword");
	(self->srctext::kindStyle[KEYWRD])->AddNewFontFace(fontdesc_Bold);
	(self->text::styleSheet)->Add(self->srctext::kindStyle[KEYWRD]);
    }
}

void ptext::SetupStyles()
{
    (this)->srctext::SetupStyles();
    ::SetupStyles(this);
}

boolean ptext::InitializeClass()
{
    static Dict pkeywords[] = {
	{"ABORT", 0, KEYWRD},
	{"ABS", 0, KEYWRD},
	{"ADR", 0, KEYWRD},
	{"ADS", 0, KEYWRD},
	{"AND", 0, KEYWRD},
	{"ARCTAN", 0, KEYWRD},
	{"ARRAY", 0, KEYWRD},
	{"ASSIGN", 0, KEYWRD},
	{"BEGIN", ID_LEAVE | ID_BEGIN | ID_INDENT, KEYWRD},
	{"BOOLEAN", 0, KEYWRD},
	{"BREAK", 0, KEYWRD},
	{"BYWORD", 0, KEYWRD},
	{"CASE", 0, KEYWRD},
	{"CHAR", 0, KEYWRD},
	{"CHR", 0, KEYWRD},
	{"CLOSE", 0, KEYWRD},
	{"CONCAT", 0, KEYWRD},
	{"CONST", ID_LEAVE | ID_INDENT, KEYWRD},
	{"COPYLST", 0, KEYWRD},
	{"COPYSTR", 0, KEYWRD},
	{"COS", 0, KEYWRD},
	{"CYCLE", 0, KEYWRD},
	{"DECODE", 0, KEYWRD},
	{"DELETE", 0, KEYWRD},
	{"DISCARD", 0, KEYWRD},
	{"DISPOSE", 0, KEYWRD},
	{"DIV", 0, KEYWRD},
	{"DO", ID_DO, KEYWRD},
	{"DOWNTO", 0, KEYWRD},
	{"ELSE", ID_DO, KEYWRD},
	{"ENCODE", 0, KEYWRD},
	{"END", ID_END, KEYWRD},
	{"EOL", 0, KEYWRD},
	{"EOLN", 0, KEYWRD},
	{"EVAL", 0, KEYWRD},
	{"EXIT", 0, KEYWRD},
	{"EXP", 0, KEYWRD},
	{"EXTERN", 0, KEYWRD},
	{"EXTERNAL", 0, KEYWRD},
	{"FALSE", 0, KEYWRD},
	{"FILE", 0, KEYWRD},
	{"FILEMODES", 0, KEYWRD},
	{"FLOAT", 0, KEYWRD},
	{"FOR", 0, KEYWRD},
	{"FORWARD", 0, KEYWRD},
	{"FUNCTION", ID_LEAVE, KEYWRD},
	{"GET", 0, KEYWRD},
	{"GOTO", 0, KEYWRD},
	{"HALT", 0, KEYWRD},
	{"HIBYTE", 0, KEYWRD},
	{"IF", 0, KEYWRD},
	{"IMPLEMENTATION", 0, KEYWRD},
	{"IN", 0, KEYWRD},
	{"INPUT", 0, KEYWRD},
	{"INSERT", 0, KEYWRD},
	{"INTEGER", 0, KEYWRD},
	{"INTERFACE", 0, KEYWRD},
	{"LABEL", 0, KEYWRD},
	{"LN", 0, KEYWRD},
	{"LOBYTE", 0, KEYWRD},
	{"LOWER", 0, KEYWRD},
	{"LSTRING", 0, KEYWRD},
	{"MAXINT", 0, KEYWRD},
	{"MAXWORD", 0, KEYWRD},
	{"MOD", 0, KEYWRD},
	{"MODULE", 0, KEYWRD},
	{"NEW", 0, KEYWRD},
	{"NIL", 0, KEYWRD},
	{"NOT", 0, KEYWRD},
	{"NULL", 0, KEYWRD},
	{"ODD", 0, KEYWRD},
	{"OF", ID_BEGIN | ID_INDENT, KEYWRD},
	{"OR", 0, KEYWRD},
	{"ORD", 0, KEYWRD},
	{"OTHERWISE", 0, KEYWRD},
	{"OUTPUT", 0, KEYWRD},
	{"PACK", 0, KEYWRD},
	{"PACKED", 0, KEYWRD},
	{"POSITN", 0, KEYWRD},
	{"PRED", 0, KEYWRD},
	{"PROCEDURE", ID_LEAVE, KEYWRD},
	{"PROGRAM", ID_LEAVE, KEYWRD},
	{"PUBLIC", 0, KEYWRD},
	{"PURE", 0, KEYWRD},
	{"PUT", 0, KEYWRD},
	{"READ", 0, KEYWRD},
	{"READFN", 0, KEYWRD},
	{"READLN", 0, KEYWRD},
	{"READONLY", 0, KEYWRD},
	{"READSET", 0, KEYWRD},
	{"REAL", 0, KEYWRD},
	{"RECORD", ID_BEGIN | ID_INDENT, KEYWRD},
	{"REPEAT", ID_BEGIN | ID_INDENT, KEYWRD},
	{"RESET", 0, KEYWRD},
	{"RESULT", 0, KEYWRD},
	{"RETURN", 0, KEYWRD},
	{"REWRITE", 0, KEYWRD},
	{"ROUND", 0, KEYWRD},
	{"SCANEQ", 0, KEYWRD},
	{"SCANNE", 0, KEYWRD},
	{"SEQUENTIAL", 0, KEYWRD},
	{"SET", 0, KEYWRD},
	{"SIN", 0, KEYWRD},
	{"SIZEOF", 0, KEYWRD},
	{"SQR", 0, KEYWRD},
	{"SQRT", 0, KEYWRD},
	{"STATIC", 0, KEYWRD},
	{"STRING", 0, KEYWRD},
	{"SUCC", 0, KEYWRD},
	{"SUPER", 0, KEYWRD},
	{"TERMINAL", 0, KEYWRD},
	{"TEXT", 0, KEYWRD},
	{"THEN", ID_DO, KEYWRD},
	{"TO", 0, KEYWRD},
	{"TRUE", 0, KEYWRD},
	{"TRUNC", 0, KEYWRD},
	{"TYPE", ID_LEAVE | ID_INDENT, KEYWRD},
	{"UNIT", 0, KEYWRD},
	{"UNPACK", 0, KEYWRD},
	{"UNTIL", ID_END, KEYWRD},
	{"UPPER", 0, KEYWRD},
	{"USES", 0, KEYWRD},
	{"VAL", 0, KEYWRD},
	{"VALUE", 0, KEYWRD},
	{"VAR", ID_LEAVE | ID_INDENT, KEYWRD},
	{"VARS", 0, KEYWRD},
	{"WHILE", 0, KEYWRD},
	{"WITH", 0, KEYWRD},
	{"WORD", 0, KEYWRD},
	{"WRD", 0, KEYWRD},
	{"WRITE", 0, KEYWRD},
	{"WRITELN", 0, KEYWRD},
	{"XOR", 0, KEYWRD},
	{NULL, 0, 0}
    };
    srctext::BuildTable("ptext", ::words, pkeywords);
    return TRUE;
}

ptext::ptext()
{
    ATKinit;
    this->srctext::words= (Dict **)::words;
    this->slashAsteriskComments=0;
    this->caseLevelIndent=4;
    this->idStyle=idstyle_CAPIT;
    this->srctext::indentingEnabled=TRUE;

    ::SetupStyles(this);
    ToolCount("EditViews-ptext", NULL);
    THROWONFAILURE(TRUE);
}


/****************************************************************
 *  Style parsing code.
 ****************************************************************/

/* comment returns the last character of the (possibly nested) comment.
 * The start character should be the '*' in a PARENCOMMENT or the
 * first character in the comment for a BRACECOMMENT. */

static long comment(ptext *self, long start, int comtype)
{
    int end, len = (self)->GetLength();
    char prev, c = 0;

    for (end = start; end < len; ++end) {
	prev = c;
	c = (self)->GetChar(end);
	if (c == '*' && prev == '(' && comtype == PARENCOMMENT)
	    end = comment(self, end, comtype);
	else if (c == ')' && prev == '*' && comtype == PARENCOMMENT)
	    break;
	else if (c == '*' && prev == '/' && comtype == SLASHCOMMENT)
	    end = comment(self, end, comtype);
	else if (c == '/' && prev == '*' && comtype == SLASHCOMMENT)
	    break;
	else if (c == '{' && comtype == BRACECOMMENT)
	    end = comment(self, end + 1, comtype);
	else if (c == '}' && comtype == BRACECOMMENT)
	    break;
    }
    return end;
}

/* backComment returns the first character (paren or brace) of the (possibly nested)
 * comment.  The start character should be the '*' in a PARENCOMMENT
 * or the last character in the comment for a BRACECOMMENT. */

static long backComment(ptext *self, long end, int comtype)
{
    int start;
    char past, c = 0;

    for (start = end; start > 0; --start) {
	past = c;
	c = (self)->GetChar(start);
	if (c == '*' && past == ')' && comtype == PARENCOMMENT)
	    start = backComment(self, start, comtype);
	else if (c == '(' && past == '*' && comtype == PARENCOMMENT)
	    break;
	else if (c == '*' && past == '/' && comtype == SLASHCOMMENT)
	    start = backComment(self, start, comtype);
	else if (c == '/' && past == '*' && comtype == SLASHCOMMENT)
	    break;
	else if (c == '}' && comtype == BRACECOMMENT)
	    start = backComment(self, start - 1, comtype);
	else if (c == '{' && comtype == BRACECOMMENT)
	    break;
    }
    return start;
}

char *ptext::Keywordify(char *buff, boolean checkforceupper)
{
    return (makeupper(buff));
}

void ptext::RedoStyles()
{
    long posn, eposn, len = GetLength();
    int prev = 0, c = '\n'/*, string = NONE*/;
    /* c is initialize to a newline so the start of the file looks like the start of line. */

    RemoveStyles();
    for (posn=0; posn<len; ++posn) {
	prev= c;
	c= GetChar(posn);
	switch (c) {
	    case '\n':
		posn= SkipWhitespace(posn,len);
		/* fall through */
	    case '\t': case ' ':
		break;
	    case '*':
		if (prev == '(') {
		    eposn = comment(this, posn, PARENCOMMENT);
		    WrapStyle(posn-1, eposn-posn + 2, this->srctext::comment_style, FALSE, FALSE);
		    posn = eposn;
		} else if (prev == '/' && this->slashAsteriskComments) {
		    eposn = comment(this, posn, SLASHCOMMENT);
		    WrapStyle(posn-1, eposn-posn + 2, this->srctext::comment_style, FALSE, FALSE);
		    posn = eposn;
		}
		break;
	    case '{':
		eposn = comment(this, posn + 1, BRACECOMMENT);
		WrapStyle(posn-1, eposn-posn + 2, this->srctext::comment_style, FALSE, FALSE);
		posn = eposn;
		break;
	    case '\'':
	    case '\"':
		posn= CheckString(posn);
		break;
	    default:
		if (IsTokenChar(c))
		    posn= CheckWord(posn, len);
		break;
	}
    }
}

/****************************************************************
  * Paren balancer.
  ****************************************************************/

struct paren_node {
    long type;
    struct paren_node *next;
};

/* skipJunk moves forward and returns the position of the first character
 * which is at or beyond the position, but is not is not white space, part of
 * a (possibly nested) comment, part of a single- or double- quoted string,
 * or greater than the length of the file. */
static long skipJunk(ptext *self, long pos)
{
    int stringtype = 0;
    long len = (self)->GetLength();

    while (pos < len) {
	int c = (self)->GetChar(pos++);
	if (stringtype && c == stringtype) {
	    stringtype = 0;
	    continue;
	}
	switch (c) {
	    case '(':
		if ((self)->GetChar(pos) == '*')
		    pos = comment(self, pos, PARENCOMMENT) + 1;
		else
		    return pos - 1;
		break;
	    case '/':
		if ((self)->GetChar(pos) == '*' && self->slashAsteriskComments)
		    pos = comment(self, pos, SLASHCOMMENT) + 1;
		else
		    return pos - 1;
		break;
	    case '{':
		pos = comment(self, pos, BRACECOMMENT) + 1;
		break;
	    case ' ':
	    case '\t':
	    case '\n':
		break;
	    case '\'':
	    case '\"':
		stringtype = c;                
		break;
	    default:
		return pos - 1;
	}
    }
    return len - 1;
}

/* backSkipJunk returns returns the position of the first character
 * which is at or earlier than the position, but is not white space, part of
 * a (possibly nested) comment, part of a single- or double-quoted string,
 * or less than zero. */
static long backSkipJunk(ptext *self, long pos)
{
    int stringtype=0;

    while (pos > 0) {
	int c = (self)->GetChar(pos--);
	if (stringtype && c == stringtype) {
	    stringtype = 0;
	    continue;
	}
	switch (c) {
	    case '\'':
	    case '\"':
		stringtype = c;
		break;
	    case ')':
		if ((self)->GetChar(pos) == '*')
		    pos = backComment(self, pos, PARENCOMMENT) - 1;
		else
		    return pos + 1;
		break;
	    case '/':
		if ((self)->GetChar(pos) == '*' && self->slashAsteriskComments)
		    pos = backComment(self, pos, SLASHCOMMENT) - 1;
		else
		    return pos + 1;
		break;
	    case '}':
		pos = backComment(self, pos, BRACECOMMENT) - 1;
		break;
	    case '\n':
	    case '\t':
	    case ' ':
		break;
	    default:
		return pos + 1;
	}
    }
    return 0;
}

/* matchBegin: takes a position of the first character in a 'end'
 * type of identifier (end, until, ...) and searches backwards.  It
 * returns the indentation of the line containing a matching 'begin'
 * type of identifier (begin, repeat, record, of, ...) */
static int matchBegin(ptext *self, long pos)
{
    int count = 1;
    Dict *word;
    char buff[256];

    while (count > 0 && --pos >= 0) {
	pos = backSkipJunk(self, pos);
	if (pos == 0)
	    return 0;
	if (! (self)->IsTokenChar((self)->GetChar(pos)))
	    continue;
	pos = (self)->BackwardCopyWord(pos, 0, buff);
	(self)->Keywordify(buff, TRUE);
	word = srctext::Lookup(self->srctext::words, buff);
	if (word->stng != NULL) {
	    if (word->val & ID_END)
		++count;
	    else if (word->val & ID_BEGIN)
		--count;
	}
    }
    return (self)->CurrentIndent(pos);
}

/* indentation: pos must be the first character on a line. */
int ptext::Indentation(long pos)
{
    char buff[256];
    Dict *word;
    long i, j;
    long endline, len, newpos;

    if ((newpos=InCommentStart(pos)) > 0)
	return CurrentIndent(newpos) + this->srctext::commentIndent;

    len = GetLength();
    endline = pos;
    while (GetChar(endline) != '\n' && endline < len)
	++endline;

    /* Find the keyword on the line, if any */
    /* It must be before any other non-white-space or comments */
    j = skipJunk(this, pos);
    if (j >= endline || ! IsTokenChar(GetChar(j)))
	word = NULL;
    else {
	j = SkipWhitespace(j, len);
	(void)CopyWord(j, len, buff);
	Keywordify(buff, TRUE);
	word = srctext::Lookup(this->srctext::words, buff);
	if (word->stng != NULL && word->val == 0)
	    word = NULL;
    }

    if (word != NULL) {
	/* If the first word on the line is a 'leave' type of keyword,
	   * then don't bother the indentation at all. */

	if (word->val & ID_LEAVE)
	    return CurrentIndent(pos);

	/* If the first word on the line is an 'end' type of keyword,
	   * the indentation is determined by a matching 'begin' type. */

	if (word->val & ID_END)
	    return matchBegin(this, pos);
    }

    /* In other cases, the indentation of the line is determined entirely
       * by the first preceding relevant indentation-related keyword. */

    j = pos;

    do  {
	--j;
	while (j > 0 && ! IsTokenChar(GetChar(j)))
	    j = backSkipJunk(this, j - 1);
	if (j <= 0) {
	    word = NULL;
	    break;
	}
	j = BackwardCopyWord(j, 0, buff);
	Keywordify(buff, TRUE);
	word = srctext::Lookup(this->srctext::words, buff);
    } while (word->stng == NULL || word->val == 0);

    if (word == NULL) 
	return CurrentIndent(pos);    /* Can't do much */

    if (word->val & ID_END) 
	return CurrentIndent(j);

    if (word->val & ID_INDENT) 
	return CurrentIndent(j) + this->srctext::levelIndent;

    if (word->val & ID_DO) {
	/* See if there's no semicolon between the DO and pos */
	for (i = j; i < pos; ++i)
	    if (GetChar(i) == ';')
		break;
	if (i == pos) 
	    return CurrentIndent(j) + this->srctext::levelIndent; /* No semi */
	return CurrentIndent(j);
    }

    /* Default: indentation of previous line */
    return (pos < 2) ? 0 : CurrentIndent(pos - 2);
}
