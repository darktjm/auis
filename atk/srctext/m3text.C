/* File m3text.C created by R L Quinn
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

   m3text, an object for editing Modula-3 code. */

static char ibmid[] = "(c) Copyright IBM Corp.  1988-1995.  All rights reserved.";
static char rcsHeader[] = "$Header: /afs/cs.cmu.edu/project/atk-src-C++/atk/srctext/RCS/m3text.C,v 2.0 1995/01/27 19:38:38 rr2b Stab74 $";

#include <andrewos.h>
#include <stylesheet.H>
#include <style.H>
#include <fontdesc.H>
#include <toolcnt.h>

#include "srctext.H"
#include "m3text.H"

static Dict *words[TABLESIZE];

ATKdefineRegistry(m3text, modtext, m3text::InitializeClass);

/* modpragma should be functionally the same as modtext's CheckComment method */
static long modpragma(m3text *self, long start)
{
    int prev, c=0, pragmas=1;
    long end=start+1, len=(self)->GetLength();

    while (end<len) {
	prev= c;
	c= (self)->GetChar(++end);
	switch(c) {
	    case '*':
		if(prev=='<') ++pragmas;
		break;
	    case '>':
		if(prev=='*') --pragmas;
		break;
	}
	if (pragmas<1) {
	    (self)->WrapStyle(start,end-start+1, self->srctext::kindStyle[PRAGMA], FALSE,FALSE);
	    break;
	}
    }
    return end;
}

/* like modtext_RedoStyles, but with ctext's \' and \" recognition */
void m3text::RedoStyles()
{
    long posn, len = GetLength();
    int prev = 0, c = '\n'; /* c is initialized to a newline so the start of the file looks like the start of line. */
    boolean escape= FALSE;
    RemoveStyles(); /* Remove the old styles, but leave the root environment in place. */
    for (posn=0; posn<len-1; ++posn) {
	prev = c;
	posn=SkipWhitespace(posn,len);
	c = GetChar(posn);
	if(escape)
	    escape=FALSE;
	else
	    switch (c) {
		case '\n':
		    break;
		case '\\':
		    escape=TRUE;
		    break;
		case '*':
		    if (prev == '(')
			posn= CheckComment(posn-1);
		    else if (prev == '<') 
			posn= modpragma(this, posn-1);
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

/* returns true iff the character at pos is quoted (ie. "\"). Takes into account the slash being quoted. (ie "\\"). */
boolean m3text::Quoted(long pos)
{
    boolean retval = FALSE;
    while (pos-->0 && GetChar(pos)=='\\')
	retval= !retval;
    return retval;
}

static void SetupStyles(m3text *self)
{
    if ((self->srctext::kindStyle[IDNTFR]= (self->text::styleSheet)->Find("identifier")) == NULL) {
	self->srctext::kindStyle[IDNTFR]= new style;
	(self->srctext::kindStyle[IDNTFR])->SetName("identifier");
	(self->srctext::kindStyle[IDNTFR])->AddNewFontFace(fontdesc_Bold | fontdesc_Italic);
	(self->text::styleSheet)->Add(self->srctext::kindStyle[IDNTFR]);
    }
    if ((self->srctext::kindStyle[PRAGMA]= (self->text::styleSheet)->Find("pragma")) == NULL) {
	self->srctext::kindStyle[PRAGMA]= new style;
	(self->srctext::kindStyle[PRAGMA])->SetName("pragma");
	(self->srctext::kindStyle[PRAGMA])->AddNewFontFace(fontdesc_Italic);
	(self->srctext::kindStyle[PRAGMA])->SetFontSize(style_PreviousFontSize, 2);
	(self->text::styleSheet)->Add(self->srctext::kindStyle[PRAGMA]);
    }
}

void m3text::SetupStyles()
{
    (this)->modtext::SetupStyles();
    ::SetupStyles(this);
}

boolean m3text::InitializeClass()
{
    static Dict m3keywords[]={
	/* Keywords */
	{"AND",0,KEYWRD},
	{"ANY",0,KEYWRD}, 
	{"ARRAY",0,KEYWRD},
	{"BEGIN",MATCH_END_BIT+LINE_WITH_MODULE_BIT,KEYWRD},
	{"BITS",0,KEYWRD},
	{"BRANDED",0,KEYWRD},
	{"BY",0,KEYWRD},
	{"CASE",MATCH_END_BIT,KEYWRD},
	{"CONST",LINE_WITH_END_BIT+LINE_WITH_MODULE_BIT+INDENT_NEXT_LINE_BIT,KEYWRD},
	{"DIV",0,KEYWRD},
	{"DO",MATCH_END_BIT,KEYWRD},
	{"ELSE",ALT_END_BIT,KEYWRD},
	{"ELSIF",ALT_END_BIT,KEYWRD},
	{"END",END_BIT,KEYWRD},
	{"EVAL",0,KEYWRD},
	{"EXCEPT",ALT_END_BIT,KEYWRD},
	{"EXCEPTION",LINE_WITH_END_BIT+LINE_WITH_MODULE_BIT+INDENT_NEXT_LINE_BIT,KEYWRD},
	{"EXIT",0,KEYWRD},
	{"EXPORTS",0,KEYWRD},
	{"FINALLY",ALT_END_BIT,KEYWRD},
	{"FOR",0,KEYWRD},
	{"FROM",0,KEYWRD},
	{"IF",MATCH_END_BIT,KEYWRD},
	{"IMPORT",0,KEYWRD},
	{"IN",0,KEYWRD},
	{"INTERFACE",MATCH_END_BIT+MODULE_BIT,KEYWRD},
	{"LOCK",0,KEYWRD},
	{"LOOP",MATCH_END_BIT,KEYWRD},
	{"METHODS",ALT_END_BIT,KEYWRD},
	{"MOD",0,KEYWRD},
	{"MODULE",MODULE_BIT+INDENT_NEXT_LINE_BIT,KEYWRD},
	{"NOT",0,KEYWRD},
	{"OBJECT",MATCH_END_BIT,KEYWRD},
	{"OF",0,KEYWRD},
	{"OR",0,KEYWRD},
	{"OVERRIDES",ALT_END_BIT,KEYWRD}, 
	{"PROCEDURE",LINE_WITH_END_BIT+LINE_WITH_MODULE_BIT+INDENT_NEXT_LINE_BIT+PROCEDURE_BIT, KEYWRD},
	{"RAISE",0,KEYWRD},
	{"RAISES",0,KEYWRD},
	{"READONLY",0,KEYWRD},
	{"RECORD",MATCH_END_BIT,KEYWRD},
	{"REF",0,KEYWRD},
	{"REPEAT",MATCH_END_BIT,KEYWRD},
	{"RETURN",0,KEYWRD},
	{"REVEAL",LINE_WITH_END_BIT+LINE_WITH_MODULE_BIT+INDENT_NEXT_LINE_BIT,KEYWRD}, 
	{"ROOT",0,KEYWRD}, 
	{"SET",0,KEYWRD},
	{"THEN",0,KEYWRD},
	{"TO",0,KEYWRD},
	{"TRY",MATCH_END_BIT,KEYWRD},
	{"TYPE",LINE_WITH_END_BIT+LINE_WITH_MODULE_BIT+INDENT_NEXT_LINE_BIT,KEYWRD},
	{"TYPECASE",MATCH_END_BIT,KEYWRD},
	{"UNSAFE",0,KEYWRD},
	{"UNTIL",END_BIT,KEYWRD},
	{"UNTRACED",0,KEYWRD},
	{"VALUE",0,KEYWRD},
	{"VAR",LINE_WITH_END_BIT+LINE_WITH_MODULE_BIT+INDENT_NEXT_LINE_BIT,KEYWRD},
	{"WHILE",0,KEYWRD},
	{"WITH",0,KEYWRD},
	{"|",ALT_END_BIT,KEYWRD},/*this is to align |'s with case statements; note that [Tab] key should be pressed after [|] so that cases line up properly*/
	/* Reserved Identifiers */
	{"ABS",0,IDNTFR},
	{"ADDRESS",0,IDNTFR},
	{"ADR",0,IDNTFR},
	{"ADRSIZE",0,IDNTFR},
	{"BITSIZE",0,IDNTFR},
	{"BOOLEAN",0,IDNTFR},
	{"BYTESIZE",0,IDNTFR},
	{"CARDINAL",0,IDNTFR},
	{"CEILING",0,IDNTFR},
	{"CHAR",0,IDNTFR},
	{"DEC",0,IDNTFR},
	{"DISPOSE",0,IDNTFR},
	{"FALSE",0,IDNTFR},
	{"FIRST",0,IDNTFR},
	{"FLOAT",0,IDNTFR},
	{"FLOOR",0,IDNTFR},
	{"INC",0,IDNTFR},
	{"INTEGER",0,IDNTFR},
	{"ISTYPE",0,IDNTFR},
	{"LAST",0,IDNTFR},
	{"LONGFLOAT",0,IDNTFR},
	{"LONGREAL",0,IDNTFR},
	{"LOOPHOLE",0,IDNTFR},
	{"MAX",0,IDNTFR},
	{"MIN",0,IDNTFR},
	{"MUTEX",0,IDNTFR},
	{"NARROW",0,IDNTFR},
	{"NEW",0,IDNTFR},
	{"NIL",0,IDNTFR},
	{"NULL",0,IDNTFR},
	{"NUMBER",0,IDNTFR},
	{"ORD",0,IDNTFR},
	{"REAL",0,IDNTFR},
	{"REFANY",0,IDNTFR},
	{"ROUND",0,IDNTFR},
	{"SUBARRAY",0,IDNTFR},
	{"TEXT",0,IDNTFR},
	{"TRUE",0,IDNTFR},
	{"TRUNC",0,IDNTFR},
	{"TYPECODE",0,IDNTFR},
	{"VAL",0,IDNTFR},
	{NULL,0,0}
    };
    srctext::BuildTable("m3text",::words,m3keywords);
    return TRUE;
}

m3text::m3text()
{
    ATKinit;
    this->srctext::words= (Dict **)::words;
    ::SetupStyles(this);
    ToolCount("EditViews-m3text",NULL);
    THROWONFAILURE(TRUE);
}
