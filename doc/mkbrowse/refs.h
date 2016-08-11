enum refs_toks {
refs_EOF,
refs_Name,
refs_Other,
refs_Semi,
refs_Class,
refs_Inheritance,
refs_Include,
refs_Define,
refs_Comma,
refs_Equal,
refs_Colon,
refs_LeftBrace,
refs_RightBrace,
refs_LeftParen,
refs_RightParen,
refs_WhiteSpace,
refs_Character,
refs_String,
refs_LessThan,
refs_GreaterThan,
refs_ATKdefineRegistry,
refs_ATKregistryEntry,
refs_ATKregistry_,
refs_Extern,
refs_LASTTYPE
};

#ifndef FILESTACKSIZE
#define FILESTACKSIZE 50
#endif

#include <atkproto.h>
BEGINCPLUSPLUSPROTOS
#ifdef ANSI_COMPILER
int refs_TokUnput(int tok);
int refs_TokComment(void);
int refs_TokString(void);
int refs_TokCharacter(void);
int refs_TokWhiteSpace(void);
int refs_TokLineComment(void);
extern int yylook(void);
extern int yywrap(void);
extern int yylex(void);
#else
int refs_TokUnput();
int refs_TokComment();
int refs_TokString();
int refs_TokCharacter();
int refs_TokWhiteSpace();
int refs_TokLineComment();
extern int yylook();
extern int yywrap();
extern int yylex();
#endif
ENDCPLUSPLUSPROTOS

extern FILE *yyin, *yyout;

