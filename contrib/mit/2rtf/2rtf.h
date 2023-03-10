/*
 2rtf.h of 

 2rtf: a facility to convert files in the ATK file format to files 
 compatible with the RTF format.

 Scribetext is copyright (c) 1989, 1990 by the Massachusetts Institute of 
 Technology.

 RTF is a product of the Microsoft Corporation.

 Permission to use, copy, modify, and distribute this software and
 its documentation for any purpose and without fee is hereby granted,
 provided that the above copyright notice and the name of the author(s)
 appear in all copies; that both that copyright notice, the name of
 the author(s) and this permission notice appear in supporting
 documentation; and that the name of the Massachusetts Institute of
 Technology not be used in advertising or publicity pertaining to
 distribution of the software without specific, written prior
 permission.  The Massachusetts Institute of Technology makes no
 representations about the suitability of this software for any purpose.
 It is provided "as is" without express or implied warranty.
 
 2rtf was written by Scott Rixner, rixner@ATHENA.MIT.EDU and Jeremy Paul Kirby, jpkirby@ATHENA.MIT.EDU
*/

#include "stvec.h"

/* PROGRAM DEFINITIONS */
#define VERSION "1.1"


#define TRUE 1
#define FALSE 0
#define TMP_SIZE 255

/* Value guaranteed not to be equal to tofind for any invocation of
 * ParseText(). */
#define CONTINUE 0

/* VALUES OF tofind for ParseText() */
/*   Must be greater than 255, save EOF   */
#define POP_JOB 4297

/* FOR USE AS BIT SETTINGS FOR mode: */
#define WORD 0
#define COMMAND 1
#define EZCOMMAND 0
#define RTFCOMMAND 2
#define PARAMETERIZED 0
#define NAKED 4
#define ANYTHINGELSE 0
#define QUOTEDCHAR 8

/* SEARCH PATH FOR FindNode()   */
#define EZCOLUMN 1
#define RTFCOLUMN 2


/* VALUES FOR transform IN ParseText()  */
#define INDEX 5
#define HEADER 4
#define GREEK 3  /* RESERVED FOR FUTURE USE */
#define CAPS 2
#define LOWERCASE 1
#define NORMAL 0

/* VALUES FOR FONTS */
#define CHICAGO 0
#define NEW_YORK 2
#define MONACO 4
#define SYMBOL 23

/* ACTION CODES FOR ParseText()  */
#define PRINTTOFILE 0
#define PRINTTOSTRING 1  /* RESERVED FOR FUTURE USE */
#define NOP 2

#define FUNCTION_SIZE 29
#define TYPE_SIZE 6
#define FONT_SIZE 3
#define UNITS 4
#define JSIZE 5
#define FSIZE 3
#define FTSIZE 5
#define TSIZE 5

#define UNBOUND -2534

#define STATES 10

struct TableStruct
{
  char *ezword;
  int mode;
  union
    {
      char *word;
      int (*fun)();
      char quote;
    } rtf;
  struct TableStruct *next;
};


struct ValuesStruct
{
  const char *name;
  const char *value;
  struct ValuesStruct *next;
};

/*
struct FileStackStruct
{
  FILE *ptr;
  long int line;
  struct FileStackStruct *next;
};
*/

struct FontStruct
{
  const char *name;
  int num;
};

struct IdStackStruct
{
   long int idnum;
   struct IdStackStruct *next;
};

struct StyleStackStruct
{
   int idnum;
   char name[TMP_SIZE];
   char string[TMP_SIZE];
   int state_count;
   enum style_Attribute state_attribute[STATES];
   long modify[STATES];
   struct StyleStackStruct *next;
};

struct style_words
{
   const char *ezword;
   const char *rtfword;
};

typedef int (*FP)(const char *command, int transform, int tofind);

extern char *me;

extern int paragraph, TextDSVersion, RTFVersion;
extern long int CurrLine;

extern FILE *fin, *fout, *ftrans, *ferr;

/* 2RFunctions.c */
extern FP AssignFunc(const char *rtfword);
extern void RStyleApply(struct StyleStackStruct *tmp);
/* main.c */
extern struct TableStruct *Table;
extern struct IdStackStruct *IdStack;
extern struct StyleStackStruct *Style;
/* misc.c */
extern char *makelower(char *instruction);
extern int offset(char *string, char character); /* unused */
extern int roffset(char *string, char character);
extern void usage(void);
extern void CloseFiles(void);
extern void AbsorbSpace(void); /* unused */
extern void AbsorbNewlines(void);
extern void Newlines(void);
/* parse.c */
extern void ParseMain(char *Filein, char *Fileout);
extern long int ParseText(int tofind, int transform, int action);
