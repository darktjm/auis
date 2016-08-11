%{
#include "refs.h"
#undef YY_INPUT
#define YY_INPUT(buf,result,max_size) \
{ \
     int c = getc(yyin); \
     result = (c == EOF) ? YY_NULL : (buf[0] = c, 1); \
}
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
<CHAR>"\\"		yymore();
<CHAR>.*\'                    return(BEGIN(INITIAL),refs_Character);
\"                              {
                                  BEGIN(STRING);
                                  yymore();
                                }
<STRING>"\\"		yymore();
<STRING>[^"]*\n		{}
<STRING>.*\"                    return(BEGIN(INITIAL),refs_String);
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
static char rcsid[] = "$Header: /afs/cs.cmu.edu/project/atk-src-C++/doc/mkbrowse/RCS/refs.flex,v 1.1 1994/06/07 17:41:19 rr2b Stab74 $";
#endif


int refs_TokWhitespace()
{
    /* convert an escaped newline into a space */	
    if(yytext[0]=='\\'){
	yytext[0]=' ';
	yytext[1]='\0';
    }

    return (refs_WhiteSpace);
}

void resetlexer() {
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