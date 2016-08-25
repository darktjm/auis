%{
#include <andrewos.h>
#include "refs.h"
static int refs_TokWhitespace(void);
#define ECHO (void)fwrite(yytext, yyleng, 1, stderr);
%}
%x COMMENT
%x STRING
%x CHAR
%x LINECOMMENT
%%
struct[^A-Z0-9a-z_]		yyless(yyleng-1);return(refs_Class);
class[^A-Z0-9a-z_]			yyless(yyleng-1);return(refs_Class);
public[^A-Z0-9a-z_]		yyless(yyleng-1);return(refs_Inheritance);
private[^A-Z0-9a-z_]		yyless(yyleng-1);return(refs_Inheritance);
protected[^A-Z0-9a-z_]		yyless(yyleng-1);return(refs_Inheritance);
virtual[^A-Z0-9a-z_]		yyless(yyleng-1);return(refs_Inheritance);
#define[^A_Z0-9a-z_]		yyless(yyleng-1);return(refs_Define);
#include[^A-Z0-9a-z_]		yyless(yyleng-1);return(refs_Include);
ATKdefineRegistry[^A-Z0-9a-z_]			yyless(yyleng-1);return(refs_ATKdefineRegistry);
ATKdefineRegistryNoInit[^A-Z0-9a-z_]			yyless(yyleng-1);return(refs_ATKdefineRegistry);
extern[^A-Z0-9a-z_]		yyless(yyleng-1);return(refs_Extern);
ATKregistryEntry[^A-Z0-9a-z_]			yyless(yyleng-1);return(refs_ATKregistryEntry);
[A-Za-z][0-9A-Za-z]*_ATKregistry_[^A-Z0-9a-z_]	yyless(yyleng-1);return(refs_ATKregistry_);
"/*"                            {
                                  BEGIN(COMMENT);
                                  yymore();
                                }
<COMMENT>[^*]*                  yymore();
<COMMENT>"*"+[^*/]*             yymore();
<COMMENT>"*"+"/"                return(BEGIN(INITIAL),refs_WhiteSpace);

"\/\/"				{
BEGIN(LINECOMMENT);
	yymore();
}
<LINECOMMENT>.*"\n" {
	return (BEGIN(INITIAL), refs_WhiteSpace);
}
\'                              {
                                  BEGIN(CHAR);
                                  yymore();
                                }
<CHAR>\\.  	{
yymore();
}
<CHAR>\\\n {
yymore();
}
<CHAR>[^\'\\]* {
yymore();
}
<CHAR>\'                   {
 	BEGIN(INITIAL);
	return refs_Character;
}
\"                              {
                                  BEGIN(STRING);
                                  yymore();
                                }
<STRING>\\. {
yymore();
}
<STRING>\\\n {
yymore();
}
<STRING>[^\"\\]* {
yymore();
}
<STRING>\"                   {
BEGIN(INITIAL);
return refs_String;
}
\000				return(refs_EOF);
";"				return(refs_Semi);
","				return(refs_Comma);
"="				return(refs_Equal);
":"				return(refs_Colon);
"("				return(refs_LeftParen);
")"				return(refs_RightParen);
"{"				return(refs_LeftBrace);
"}"				return(refs_RightBrace);
"<"				return(refs_LessThan);
">"				return(refs_GreaterThan);
\\\n				return(refs_TokWhitespace());
[ \n\t\r]*				return(refs_TokWhitespace());
[A-Za-z_][0-9A-Za-z_]*		return(refs_Name);
.				return(refs_Other);
%%
#ifndef NORCSID
static UNUSED const char rcsid[] = "$Header: /afs/cs.cmu.edu/project/atk-src-C++/overhead/grefs/RCS/refs.flex,v 1.7 1996/07/21 18:31:36 robr Exp $";
#endif


static int refs_TokWhitespace(void)
{
    /* convert an escaped newline into a space */	
    if(yytext[0]=='\\'){
	yytext[0]=' ';
	yytext[1]='\0';
    }

    return (refs_WhiteSpace);
}

void resetlexer(void) {
yy_init=1;
yyrestart(yyin);
}

/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/config/COPYRITE'     *
\* ********************************************************************** */
/* Copyright 1993 Carnegie Mellon University All rights reserved.
  $Disclaimer: 
# Permission to use, copy, modify, and distribute this software and its 
# documentation for any purpose and without fee is hereby granted, provided 
# that the above copyright notice appear in all copies and that both that 
# copyright notice and this permission notice appear in supporting 
# documentation, and that the name of IBM not be used in advertising or 
# publicity pertaining to distribution of the software without specific, 
# written prior permission. 
#                         
# THE COPYRIGHT HOLDERS DISCLAIM ALL WARRANTIES WITH REGARD 
# TO THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF 
# MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL ANY COPYRIGHT 
# HOLDER BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL 
# DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, 
# DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE 
# OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION 
# WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
# 
#  $ */
