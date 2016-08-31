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
#ifdef FLEX_ENV
extern void resetlexer();
#endif
extern int yylex(void);
ENDCPLUSPLUSPROTOS

extern FILE *yyin, *yyout;

