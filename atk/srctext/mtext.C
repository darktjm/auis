/* File mtext.C created by R L Quinn
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

   mtext, an object for editing Modula-2 code. */

#include <andrewos.h>

static UNUSED const char ibmid[] = "(c) Copyright IBM Corp.  1988-1995.  All rights reserved.";
static UNUSED const char rcsHeader[] = "$Header: /afs/cs.cmu.edu/project/atk-src-C++/atk/srctext/RCS/mtext.C,v 2.0 1995/01/27 19:38:38 rr2b Stab74 $";

#include <stylesheet.H>
#include <style.H>
#include <fontdesc.H>
#include <toolcnt.h>

#include "srctext.H"
#include "mtext.H"

static Dict *words[TABLESIZE];

ATKdefineRegistry(mtext, modtext, mtext::InitializeClass);

static void SetupStyles(mtext *self)
{
    if ((self->srctext::kindStyle[IDNTFR]= (self->text::styleSheet)->Find("predefined")) == NULL) {
	self->srctext::kindStyle[IDNTFR]= new style;
	(self->srctext::kindStyle[IDNTFR])->SetName("predefined");
	(self->srctext::kindStyle[IDNTFR])->AddNewFontFace(fontdesc_Bold | fontdesc_Italic);
	(self->text::styleSheet)->Add(self->srctext::kindStyle[IDNTFR]);
    }
    self->srctext::kindStyle[PRAGMA]= NULL;
    if ((self->srctext::kindStyle[PREPRC]= (self->text::styleSheet)->Find("preproc")) == NULL) {
	self->srctext::kindStyle[PREPRC]= new style;
	(self->srctext::kindStyle[PREPRC])->SetName("preproc");
	(self->srctext::kindStyle[PREPRC])->AddNewFontFace(fontdesc_Italic);
	(self->text::styleSheet)->Add(self->srctext::kindStyle[PREPRC]);
    }
}

void mtext::SetupStyles()
{
    (this)->modtext::SetupStyles();
    ::SetupStyles(this);
}

boolean mtext::InitializeClass()
{
    static Dict mkeywords[]={
	/* Keywords */
	{"AND",0,KEYWRD},
	{"ARRAY",0,KEYWRD},
	{"BEGIN",MATCH_END_BIT | LINE_WITH_MODULE_BIT,KEYWRD},
	{"BY",0,KEYWRD},
	{"CASE",MATCH_END_BIT,KEYWRD},
	{"CONST",LINE_WITH_END_BIT|LINE_WITH_MODULE_BIT|INDENT_NEXT_LINE_BIT,KEYWRD},
	{"DEFINITION",0,KEYWRD},
	{"DIV",0,KEYWRD},
	{"DO",MATCH_END_BIT, KEYWRD},
	{"ELSE",ALT_END_BIT,KEYWRD},
	{"ELSIF",ALT_END_BIT,KEYWRD},
	{"END",END_BIT,KEYWRD},
	{"EXIT",0,KEYWRD},
	{"EXPORT",0,KEYWRD},
	{"FOR",0,KEYWRD},
	{"FROM",0,KEYWRD},
	{"IF",MATCH_END_BIT,KEYWRD},
	{"IMPLEMENTATION",0,KEYWRD},
	{"IMPORT",0,KEYWRD},
	{"IN",0,KEYWRD},
	{"LOOP",MATCH_END_BIT,KEYWRD},
	{"MOD",0,KEYWRD},
	{"MODULE",MODULE_BIT|INDENT_NEXT_LINE_BIT,KEYWRD},
	{"NOT",0,KEYWRD},
	{"OF",0,KEYWRD},
	{"OR",0,KEYWRD},
	{"POINTER",0,KEYWRD},
	{"PROCEDURE",LINE_WITH_END_BIT|LINE_WITH_MODULE_BIT|INDENT_NEXT_LINE_BIT|PROCEDURE_BIT,KEYWRD},
	{"QUALIFIED",0,KEYWRD},
	{"REF",0,KEYWRD},
	{"RECORD",MATCH_END_BIT,KEYWRD},
	{"REPEAT",MATCH_END_BIT,KEYWRD},
	{"RETURN",0,KEYWRD},
	{"SET",0,KEYWRD},
	{"THEN",0,KEYWRD},
	{"TO",0,KEYWRD},
	{"TYPE",LINE_WITH_END_BIT|LINE_WITH_MODULE_BIT|INDENT_NEXT_LINE_BIT,KEYWRD},
	{"UNTIL",END_BIT,KEYWRD},
	{"VAR",LINE_WITH_END_BIT|LINE_WITH_MODULE_BIT|INDENT_NEXT_LINE_BIT,KEYWRD},
	{"WHILE",0,KEYWRD},
	{"WITH",0,KEYWRD},
	{"|",ALT_END_BIT,KEYWRD},/*this is to align |'s with case statements; note that [Tab] key should be pressed after [|] so that cases line up properly*/
	/* Predefined Identifiers */
	{"ABS",0,IDNTFR},
	{"ADR",0,IDNTFR},
	{"BITSET",0,IDNTFR},
	{"BOOLEAN",0,IDNTFR},
	{"CAP",0,IDNTFR},
	{"CARDINAL",0,IDNTFR},
	{"CHAR",0,IDNTFR},
	{"CHR",0,IDNTFR},
	{"DEC",0,IDNTFR},
	{"DISPOSE",0,IDNTFR},
	{"EXCL",0,IDNTFR},
	{"FALSE",0,IDNTFR},
	{"FLOAT",0,IDNTFR},
	{"HALT",0,IDNTFR},
	{"HIGH",0,IDNTFR},
	{"INC",0,IDNTFR},
	{"INCL",0,IDNTFR},
	{"INTEGER",0,IDNTFR},
	{"LONGINT",0,IDNTFR},
	{"LONGREAL",0,IDNTFR},
	{"MAX",0,IDNTFR},
	{"MIN",0,IDNTFR},
	{"NEW",0,IDNTFR},
	{"NIL",0,IDNTFR},
	{"ODD",0,IDNTFR},
	{"ORD",0,IDNTFR},
	{"PROC",0,IDNTFR},
	{"REAL",0,IDNTFR},
	{"SIZE",0,IDNTFR},
	{"TRUE",0,IDNTFR},
	{"TRUNC",0,IDNTFR},
	{"TSIZE",0,IDNTFR},
	{"VAL",0,IDNTFR},
	{NULL,0,IDNTFR}
    };
    srctext::BuildTable("mtext",::words,mkeywords);
    return TRUE;
}

mtext::mtext()
{
    ATKinit;
    this->srctext::words= (Dict **)::words;
    this->modtext::preprocessor= TRUE;
    ::SetupStyles(this);
    ToolCount("EditViews-mtext",NULL);
    THROWONFAILURE(TRUE);
}
