/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */
%{
#include <andrewos.h>
#include "refs.h"
static int refs_TokWhitespace(void);
#define ECHO (void)fwrite(yytext, yyleng, 1, stderr);
#define YY_NO_UNPUT 1
#define YY_NO_INPUT 1
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
