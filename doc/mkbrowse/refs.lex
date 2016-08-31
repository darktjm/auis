%{
#include "refs.h"
%}
%%
struct[^A-Z0-9a-z_]		return(refs_TokUnput(refs_Class));
class[^A-Z0-9a-z_]		return(refs_TokUnput(refs_Class));
public[^A-Z0-9a-z_]		return(refs_TokUnput(refs_Inheritance));
private[^A-Z0-9a-z_]		return(refs_TokUnput(refs_Inheritance));
protected[^A-Z0-9a-z_]		return(refs_TokUnput(refs_Inheritance));
virtual[^A-Z0-9a-z_]		return(refs_TokUnput(refs_Inheritance));
#define[^A_Z0-9a-z_]		return(refs_TokUnput(refs_Define));
#include[^A-Z0-9a-z_]		return(refs_TokUnput(refs_Include));
ATKdefineRegistry[^A-Z0-9a-z_]			return(refs_TokUnput(refs_ATKdefineRegistry));
extern[^A-Z0-9a-z_]		return(refs_TokUnput(refs_Extern));
ATKregistryEntry[^A-Z0-9a-z_]			return(refs_TokUnput(refs_ATKregistryEntry));
[A-Za-z][0-9A-Za-z]*_ATKregistry_[^A-Z0-9a-z_]	return(refs_TokUnput(refs_ATKregistry_));
\/\*				return(refs_TokComment());
\"				return(refs_TokString());
\'				return(refs_TokCharacter());
\/\/				return(refs_TokLineComment());
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
int refs_TokUnput(tok)
int tok;
{
    unput(yytext[--yyleng]);
    yytext[yyleng]='\0';
    return tok;
}

int refs_TokWhitespace()
{
    /* convert an escaped newline into a space */	
    if(yytext[0]=='\\'){
	yytext[0]=' ';
	yytext[1]='\0';
    }

    return (refs_WhiteSpace);
}

int refs_TokComment() {
    /* must search to the end of the comment */
    /* attempt to return it in str - up to the point that fits */

    int i = 2, c, SawStar = 0;
    yytext[YYLMAX-3] = '*';
    yytext[YYLMAX-2] = '/';
    yytext[YYLMAX-1] = '\0';
    while ((c = input()) > 0)  {
	if (i < YYLMAX - 3) yytext[i] = c;
	i += 1;
	if (SawStar && c == '/') break;
	SawStar = (c == '*');
    }
    if (i < (YYLMAX - 3)) yytext[i] = '\0';
    return (refs_WhiteSpace);
}

int refs_TokLineComment() {
    /* must search to the end of the comment */
    /* attempt to return it in str - up to the point that fits */

    int i = 2, c;
    
    yytext[YYLMAX-1] = '\0';
    
    while ((c = input()) > 0)  {
	if (i < YYLMAX - 1) yytext[i] = c;
	i += 1;
	if ( c == '\n') break;
    }
    if (i < (YYLMAX - 1)) yytext[i] = '\0';
    return (refs_WhiteSpace);
}

int refs_TokString() {
    int i = 1, c, SawBackSlash = 0;
    
    yytext[YYLMAX-2] = '"';
    yytext[YYLMAX-1] = '0';
    
    while ((c = input()) > 0)  {
	if (i < YYLMAX - 2) yytext[i] = c;
	i += 1;
	if (! SawBackSlash && c == '"') break;
	if(c == '\\') SawBackSlash=!SawBackSlash;
	else SawBackSlash=0;
    }
    if (i < (YYLMAX - 2)) yytext[i] = '\0';
  /*  if (i > (YYLMAX - 2))  {
	fprintf(stderr, "String Constant too long - truncated\n");
    }*/
    return (refs_String);
}

int refs_TokCharacter() {
    int i = 1, c, SawBackSlash=0;
    yytext[YYLMAX-2] = '\'';
    yytext[YYLMAX-1] = '0';
	while ((c = input()) > 0)  {
	if (i < YYLMAX - 2) yytext[i] = c;
	i += 1;
	if (! SawBackSlash && c == '\'') break;
	if(c == '\\') SawBackSlash=!SawBackSlash;
	else SawBackSlash=0;
    }
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
%%
