#ifndef _rexxtext_H_
#define _rexxtext_H_
/** \addtogroup libsrctext
 * @{ */
/* File rexxtext.H created by R S Kemmetmueller
   (c) Copyright IBM Corp.  1988-1995.  All rights reserved.

   rexxtext: a text subclass specialized for dealing with REXX code. */

/* kinds of styles that are used in hash table */
#define KEYWRD 1     /* keywords */
#define BILTIN 2     /* built-in function */

/* bit values used for indentation identification */ /*RSK91add*/
#define START_BIT 1    /* for DO and SELECT, which have corresponding ENDs */
#define END_BIT 2      /* for END, to denote the end of a DO or SELECT */
#define ELSE_BIT 4     /* to flag an else catch */
#define IF_BIT 8
#define THEN_BIT 16
#define WHEN_BIT 32
#define OTHERWISE_BIT 64
#define BEGIN_BIT 128

#include "srctext.H"
class rexxtext: public srctext {
  public:
    TRACED_CLASS_NODEST(rexxtext);

    void BackwardCheckWord(long from, long to);
    long CheckComment(long start);
    long CheckWord(long i, long end);
    int Indentation(long pos);
    boolean IsTokenChar(char ch);
    char *Keywordify(char *buff, boolean checkforceupper);
    void RedoStyles();
    void SetAttributes(struct attributes *atts);
    void SetupStyles();

    static boolean InitializeClass();
    rexxtext();
};
/** @} */
#endif /* _rexxtext_H_ */
