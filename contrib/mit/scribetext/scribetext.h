/*
 scribetext.h of 

 Scribetext: a facility to convert Scribe manuscript files to files 
 compatible with the ATK format.

 Scribetext is copyright (c) 1989, 1990 by the Massachusetts Institute of 
 Technology.

 Scribe is a registered trademark of Scribe Systems, Inc.

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
 
 Scribetext was written entirely by Jeremy Paul Kirby, jpkirby@ATHENA.MIT.EDU
*/

/* PROGRAM DEFINITIONS */
#define VERSION "1.2Alpha"
#define SCRIBESITELOCATION "/usr/lib/scribe.sit"

/* CHARACTER FLAGS FOR SCRIBETEXT COMMANDS IN TRANSFILE  */
#define COMMAND_LIST "@!#$^"

#define TRUE 1
#define FALSE 0
#define TMP_SIZE 255


/* VALUES OF tofind for ParseText() */
/*   Must be greater than 255, save EOF   */
#define POP_JOB 4297

/* FOR USE AS BIT SETTINGS FOR mode: */
#define WORD 0
#define COMMAND 1
#define SCRIBECOMMAND 0
#define EZCOMMAND 2
#define PARAMETERIZED 0
#define NAKED 4
#define USEDELS 0
#define SUPPRESSDELS 8
#define ANYTHINGELSE 0
#define QUOTEDCHAR 16

/* SEARCH PATH FOR FindNode()   */
#define SCRIBECOLUMN 1
#define EZCOLUMN 2


/* VALUES FOR transform IN ParseText()  */
#define GREEK 3  /* RESERVED FOR FUTURE USE */
#define CAPS 2
#define LOWERCASE 1
#define NORMAL 0


/* ACTION CODES FOR ParseText()  */
#define PRINTTOFILE 0
#define PRINTTOSTRING 1  /* RESERVED FOR FUTURE USE */
#define NOP 2


typedef int (*FP)(const char *command, int tofind);

typedef struct TableStruct
{
  char *scribeword;
  int mode;
  union
    {
      char *word;
      FP fun;
      char quote;
    } ez;
  struct TableStruct *next;
} *TABLE;


typedef struct ValuesStruct
{
  const char *name;
  const char *value;
  struct ValuesStruct *next;
} *VALUES;

typedef struct FileStackStruct
{
  FILE *ptr;
  long int line;
  struct FileStackStruct *next;
} *FILESTACK;


extern char *me, *Scribechars, *Scribeopendelimiters, *Scribeclosedelimiters;

extern int Token, MasterToken, verbatim, TextDSVersion;
extern long int CurrLine;

extern FILE *fin, *fout, *ftrans, *ferr;

/* main.c */
extern TABLE Table;
extern FILESTACK FileStack;
/* misc.c */
extern char *makelower(char *instruction);
extern int offset(char *string, char character);
extern int roffset(char *string, char character);
extern void usage(void);
extern void CloseFiles(void);
extern void AbsorbSpace(void);
extern void AbsorbNewlines(void);
extern void PushFile(char *filename);
extern int PopFile(void);
/* parse.c */
extern void ParseMain(char *Filein, char *Fileout);
extern long int ParseText(int tofind, const char *prepend, const char *append, int transform, int action);
extern TABLE FindNode(int field, char *string);
extern void ReplaceText(const char *instruction, int mode, char tofind);
/* STFunctions.c */
extern FP AssignFunc(const char *ezword);
extern int STUniqueID(void);
