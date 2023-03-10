#ifndef _modtext_H_
#define _modtext_H_
/** \addtogroup libsrctext
 * @{ */
/* File modtext.H created by R L Quinn
   (c) Copyright IBM Corp.  1988-1995.  All rights reserved.

   modtext: a text subclass specialized for dealing with Modula-X code. */

/* #define PROCEDURE_VAL 201 */

/* kinds of styles that are used in hash table */
#define KEYWRD 1    /* keywords */
#define IDNTFR 2    /* identifiers (reserved(M3) or predefined(M2)) */
#define PRAGMA 3    /* M3 only, put here for convenience of ReverseBalance, Indentation, etc */
#define PREPRC 4    /* M2 only, put here for convenience and reuse */


/* meanings of bit values:
  bit#0   (1) match with an END (e.g.BEGIN,IF,etc)
  bit#1   (2) match END with BEGIN,IF,etc
  bit#2   (4) used to match ELSE and ELSIF with IF
  bit#3   (8) used to align CONST,TYPE,VAR,PROCEDURE,and BEGIN
  bit#4  (16) line with an END
  bit#5  (32) completely outdents MODULE and INTERFACE
  bit#6  (64) indent lines, but don't match with an END (e.g.PROCEDURE)
  bit#7 (128) used for lining up PROCEDURE arguments or something
*/

#define MATCH_END_BIT		1
#define LINE_WITH_END_BIT	16
#define END_BIT			2
#define ALT_END_BIT		4
#define LINE_WITH_MODULE_BIT	8
#define MODULE_BIT		32
#define INDENT_NEXT_LINE_BIT	64
#define PROCEDURE_BIT		128


#include "srctext.H"
class modtext: public srctext {
  public:
    TRACED_CLASS_NODEST(modtext);

    void BackwardCheckWord(long pos, long first);
    long CheckComment(long start);
    long CheckWord(long i, long end);
    int Indentation(long pos);
    boolean IsTokenChar(char ch);
    char *Keywordify(char *buff, boolean checkforceupper);
    void RedoStyles();
    long ReverseBalance(long pos);
    void SetAttributes(struct attributes *atts);
    void SetupStyles();

    virtual boolean IsTokenOrPeriod(char ch);

    modtext();

    boolean preprocessor; /* TRUE if the language has a C-style preprocessor */
    boolean outdentPreproc;
    int nestedprocindent;
};
/** @} */
#endif /* _modtext_H_ */
