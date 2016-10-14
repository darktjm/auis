/* File ltext.C
   (c) Copyright IBM Corp.  1988-1995.  All rights reserved.

   ltext, an object for editing Lisp code. */

#include <andrewos.h>

static UNUSED const char ibmid[] = "(c) Copyright IBM Corp.  1988-1995.  All rights reserved.";

#include <setjmp.h>
#include <ctype.h>

#include <environ.H>
#include <environment.H>

#include <stylesheet.H>
#include <style.H>

#include <attribs.h>
#include <toolcnt.h>

#include "srctext.H"
#include "ltext.H"

#define LOW(ch) (isupper(ch)?tolower(ch):(ch))
#define new_c(type) (type *)malloc(sizeof(type))

ATKdefineRegistryNoInit(ltext, srctext);

static void addindent(ltext *self, const char *name, int args)
{
    struct indenter *newindent = new_c(struct indenter);
    char *ptr;

    ptr = newindent->name = (char *)malloc(strlen(name) + 1);
    while ((*ptr++ = LOW(*name)) != '\0')
	++name;
    newindent->args = args;
    newindent->next = self->Indents;
    self->Indents = newindent;
}

void ltext::SetAttributes(struct attributes *atts)
{
    (this)->srctext::SetAttributes(atts);
    while (atts != NULL) {
	const char *key = atts->key;
	if (strcmp(key, "defun-indent")==0)
	    addindent(this, "defun", atoi(atts->value.string));
	else if (strcmp(key, "defmacro-indent")==0)
	    addindent(this, "defmacro", atoi(atts->value.string));
	else if (strcmp(key, "defconstant-indent")==0)
	    addindent(this, "defconstant", atoi(atts->value.string));
	else if (strcmp(key, "defstruct-indent")==0)
	    addindent(this, "defstruct", atoi(atts->value.string));
	else if (strcmp(key, "let-indent")==0)
	    addindent(this, "let", atoi(atts->value.string));
	else if (strcmp(key, "let*-indent")==0)
	    addindent(this, "let*", atoi(atts->value.string));
	else if (strcmp(key, "do-indent")==0)
	    addindent(this, "do", atoi(atts->value.string));
	else if (strcmp(key, "if-indent")==0)
	    addindent(this, "if", atoi(atts->value.string));
	atts=atts->next;
    }
}

ltext::ltext()
{
    char *t, *t2, *s, *copy;
    const char *spref = environ::GetProfile("ltextindents");
    this->Indents = NULL;

    if (!spref) {
	s = strdup("defun:2,defmacro:2,defconstant:1,defstruct:1,let:1,let*:1,do:2,if:1");
    } else {
	s = strdup(spref);
    }
    while (isspace(*s)) ++s;
    copy = s;
    while ((t = copy)) {
	if ((copy = strchr(t, ','))) {
	    *copy++ = '\0';
	}
	if ((t2=strchr(t, ':'))) *t2++ = '\0';
	addindent(this, t, t2 ? atoi(t2) : 1);
    }

    this->srctext::indentingEnabled=TRUE;
    if (s) free(s);
    ToolCount("EditViews-ltext",NULL);
    THROWONFAILURE(TRUE);
}

void ltext::RedoStyles()
{
    long posn, len = GetLength();
    enum {normal, comment, string, vertbars} state = normal;
    enum {nothing, justin, foundD, foundE} substate = nothing;
    enum {nodef, indef, inwhitespace, inname} defwhatever = nodef;
    int parens = 0, c;
    boolean quoted = FALSE;
    long commentpos = 0, stringpos = 0, fnpos = 0;

    RemoveStyles();

    /* Two things of interest:
       1 - semicolons. Can't be quoted or inside of vert bars.
       2 - "def". must be first thing after top level paren.
       */

    for (posn = 0; posn < len; ++posn) {
	c = GetChar(posn);
	switch (state) {
	    case normal:
		switch (substate) {
		    case nothing:
			break;
		    case justin:
			switch (c) {
			    case ' ':
			    case '\t':
				break;
			    case 'd':
			    case 'D':
				substate = foundD;
				break;
			    default:
				substate = nothing;
				break;
			}
			break;
		    case foundD:
			if (c == 'e' || c == 'E')
			    substate = foundE;
			else
			    substate = nothing;
			break;
		    case foundE:
			if (c == 'f' || c == 'F') {
			    defwhatever = indef;
			    fnpos = -1;
			}
			substate = nothing;
			break;
		}
		switch (defwhatever) {
		    case nodef:
			break;
		    case indef:
			switch (c) {
			    case ' ':
			    case '\t':
			    case '\n':
				defwhatever = inwhitespace;
				break;
			    default:
				break;
			}
			break;
		    case inwhitespace:
			switch (c) {
			    case ' ':
			    case '\t':
			    case '\n':
				break;
			    case ';':
				if (!quoted)
				    break;
				/* else fall through */
			    default:
				defwhatever = inname;
				fnpos = posn;
				break;
			}
			break;

		    case inname:
			if (c <= ' ' || c == '(' || c == ')' || c == ';') {
			    WrapStyle(fnpos, posn-fnpos, this->srctext::function_style, TRUE, TRUE);
			    defwhatever = nodef;
			}
			break;
		}

		if (!quoted)
		    switch (c) {
			case '(':
			    if (parens++ == 0)
				substate = justin;
			    defwhatever = nodef;
			    break;
			case ')':
			    --parens;
			    defwhatever = nodef;
			    break;
			case '|':
			    state = vertbars;
			    break;
			case '\"':
			    state = string;
			    defwhatever = nodef;
			    stringpos = posn;
			    break;
			case ';':
			    commentpos = posn;
			    state = comment;
			    break;
			default:
			    break;
		    }
		break;

	    case comment:
		substate = nothing;
		if (c == '\n') {
		    WrapStyle(commentpos, posn-commentpos, this->srctext::linecomment_style, FALSE, TRUE);
		    state = normal;
		}
		break;

	    case string:
		substate = nothing;
		if (c == '\"' && !quoted) {
		    WrapStyle(stringpos, posn-stringpos+1, this->srctext::string_style, FALSE, FALSE);
		    state = normal;
		}
		break;

	    case vertbars:
		substate = nothing;
		if (c == '|' && !quoted)
		    state = normal;
		break;
	}
	if (c == '\\')
	    quoted = !quoted;
	else
	    quoted = FALSE;
    }
}

/****************************************************************
 *
 * Paren balancer.
 *
 ****************************************************************/

/* Things that need to be dealt with:
    Comments: start anywhere on a line and go to the end of the line.
    Quotes: between sets of ``|''
    Strings: Double Quotes. Can span lines. Backslash quotes.
    Paren-Types: Normal, Curly, Square. No Angle braces.

  Dealing with Quotes is hard, so it is punted until I think of something that will work. 
  */
struct paren_node {
    long type;
    struct paren_node *next;
};

/* returns true iff the character at pos is quoted (ie. "\"). Takes into account the slash being quoted. (ie "\\"). */
boolean ltext::Quoted(long pos)
{
    boolean retval = FALSE;
    while (pos-->0 && GetChar(pos) == '\\')
	retval = !retval;
    return retval;
} 

long ltext::ReverseBlance(long pos, int type)
{
    /* Balance parens backwards. If type is EOF, scan back to the start of the previous paren pair, otherwise scan back to the matching open paren. Never look at characters positioned at "pos" or later. */

    boolean found = FALSE, instring = FALSE, atleastone = FALSE;
    int thischar, prechar;
    const char *parentype;
    struct paren_node *parenstack = NULL;
    static const char opens[] = "({[", closes[] = ")}]";

    if (type == EOF)
	if (pos > 0)
	    prechar = GetChar(--pos);
	else
	    return EOF;
    else {
	if (strchr(closes, type) == NULL)
	    /* Don't know anything about this type of paren. */
	    return EOF;
	prechar = type;
    }

    while ((parenstack != NULL || !atleastone) && (pos >= 0)) {
	thischar = prechar;
	if (pos-- > 0)
	    prechar = GetChar(pos);

	if (thischar == '\n') {
	    /* Check for a comment. */
	    long newline, semi = -1;
	    int c;

	    /* Scan backwards until we find a newline, checking for a ';' */
	    for (newline = pos; newline >= 0 && (c = GetChar(newline)) != '\n'; --newline)
		if (c == ';' && !Quoted(newline))/* added ltext_ tjm*/
		    semi = newline;

	    /* Found a semi. */
	    if (semi >= 0) {
		prechar = (pos = semi) > 0 ? GetChar(pos - 1) : 0;
		continue;
	    }
	}

	if (prechar!='\\' || !Quoted(pos+1)) {
	    if (instring) {
		if (thischar == '\"')
		    instring = FALSE;
	    } else if (thischar == '\"') {
		instring = TRUE;
	    } else if ((parentype = strchr(opens, thischar)) != NULL) {
		if (parenstack == NULL || parenstack->type != (parentype - opens)) {
		    break;
		} else {
		    struct paren_node *temp = parenstack;

		    parenstack = parenstack->next;
		    free(temp);
		    if ((prechar == '\n' || pos <= 0) && parenstack != NULL) {
			break;
		    } else if (parenstack == NULL) {
			found = TRUE;
			break;
		    }
		}
	    } else if ((parentype = strchr(closes, thischar)) != NULL) {
		struct paren_node *temp = new_c(struct paren_node);

		temp->type = parentype - closes;
		temp->next = parenstack;
		parenstack = temp;
	    }
	}
    }
    if (found)
	return pos + 1;
    else
	return EOF;
}

static long findsexpr(ltext *self, long pos, long limit)
{
    /* Returns the start of the next sexpr before limit. Takes into account comments and ws. */
    int c;

    while (pos < limit && (c = (self)->GetChar(pos)) != EOF)
	if (c == ';' && !(self)->Quoted(pos))
	    while ((c = (self)->GetChar(++pos)) != EOF && c != '\n') ;
	else
	    if (isspace(c))
		++pos;
	    else
		return pos;

    return EOF;
}

static long skipsexpr(ltext *self, long pos, long limit)
{
    /* Returns the first position after the end of the current sexpr. */
    int c;
    long nextexpr;

    switch ((self)->GetChar(pos)) {
	case '(':
	    /* It's a list. */
	    ++pos;
	    while (pos < limit) {
		nextexpr = findsexpr(self, pos, limit);
		if (nextexpr < 0 || nextexpr >= limit)
		    return EOF;
		if ((self)->GetChar(nextexpr) == ')')
		    return nextexpr + 1;
		pos = skipsexpr(self, nextexpr, limit);
	    }
	    break;

	case '\"':
	    /* It's a string. */
	    while (pos++ < limit && (c = (self)->GetChar(pos)) != EOF)
		if (c == '\"' && !(self)->Quoted(pos))
		    return pos;
	    break;

	case '.':
	    /* Might be a dotted pair. */
	    c = (self)->GetChar(pos+1);
	    if (c > ' ' && c != '(' && c != ')' && !(self)->Quoted(pos))
		return pos+1;
	    /* Fall through. */
	default:
	    /* Skip until ws, delimeter, or comment. */
	    while (pos < limit && (c = (self)->GetChar(pos)) != EOF)
		if (isspace(c) || ((c == ';' || c == '(' || c == ')') && !(self)->Quoted(pos)))
		    return pos;
		else
		    ++pos;
	    break;
    }
    return EOF;
}

int ltext::Indentation(long pos)
{
    /* Return the correct indentation for the line containing pos assuming nothing exists beyond pos. Note: Assumed that the text object is not modified while we are doing our stuff so that longs can be used instead of marks. */
    struct indenter *ind;
    long linestart, sexprstart, functionstart, functionend, firstarg, argstart;
    boolean comment = FALSE;
    int c, args;
    char buf[256], *ptr;

    /* Find the start of the line, checking to see if we are in a left-flush comment. */
    while (pos > 0 && (c = GetChar(pos-1)) != '\n') {
	--pos; 
	if (!isspace(c))
	    comment = (c == ';');
    } 
    linestart = pos;

    /* If we are in a comment, make it left-flush */
    if (comment)
	return 0;

    /* Find the start of the sexpr we are in. */
    sexprstart = ReverseBlance(linestart, ')');

    /* If we aren't in an sexpr, see if a previous sexpr can be scanned. If so, assume the same indentation, otherwise assume a top level form. */
    if (sexprstart < 0) {
	sexprstart = ReverseBlance(linestart, EOF);
	if (sexprstart < 0)
	    return 0;
	else
	    return CurrentColumn(sexprstart);
    }

    /* Find the start of the first sexpr (the function) inside our parent sexpr, but don't look at anything on this line. */
    functionstart = findsexpr(this, sexprstart+1, linestart);

    /* If we couldn't find one, assume that we will be the function and return one more than the paren. */
    if (functionstart < 0)
	return CurrentColumn(sexprstart) + 1;

    /* Determine the name of the function. Note: it is impossible for skipexpr to return a position on this line or further. This is because ReverseBalance would not have passed over this sexpr it was not totally before the start of this line. */
    functionend = skipsexpr(this, functionstart, linestart);
    args = -1;
    if (functionend - functionstart < (int)sizeof(buf)) {
	/* Don't bother if the ``function'' is real long, cause then it's probably not a function */
	ptr = buf;
	for (pos = functionstart; pos < functionend; ++pos)
	    *ptr++ = LOW(GetChar(pos));
	*ptr = '\0';
	for (ind = this->Indents; ind != NULL; ind = ind->next)
	    if (strcmp(buf, ind->name) == 0) {
		args = ind->args;
		break;
	    }
    }

    /* If the function wants zero args indented, it's easy. */
    if (args == 0)
	return CurrentColumn(functionstart) + 1;

    /* Find the start of the second expr (the first arg). */
    argstart = firstarg = findsexpr(this, functionend, linestart);

    /* Try to find more than "args" args. If so, return two more than function indent. */
    if (args > 0)
	while (argstart > 0) {
	    if (--args == 0)
		return CurrentColumn(functionstart) + 1;
	    argstart = findsexpr(this, skipsexpr(this, argstart, linestart), linestart);
	}

    /* If the arg doesn't exist, assume that we are going to be it, and return the column of the function name. */
    if (firstarg < 0)
	return CurrentColumn(functionstart);

    /* Assume that we are another argument, and return the column of the first arg. */
    return CurrentColumn(firstarg);
}
