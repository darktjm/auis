#ifndef _perltext_H_
#define _perltext_H_
/* File perltext.H created by W R Nelson
   (c) Copyright IBM Corp.  1988-1995.  All rights reserved.

   perltext: a text subclass specialized for dealing with perl code. */

/* kinds of styles that are used in hash table */
#define KEYWRD 1     /* keywords */
#define STRING 2     /* built-in function */
#define NONE 3       /* no style on the "keyword" */

/* bit values used to distingues the types of keywords for recognizing strings and such */
#define SUB_BIT 1 /* for sub */
#define TWO_DELIM_BIT 2 /* for 'm', 'q', 'qq' ( the ones that have 2 delims ) */
#define THREE_DELIM_BIT 4 /* for 's', 'tr' ( those having 3 delims ) */
#define COMPARE_STR_BIT 8 /* for "eq", "ne", "le", . . . */
#define STR_FOLLOW_KEYWD 16 /* wrn94 bit to tell whether a string can follow this keyword  */

#define VARIABLE_START "$@%*&"

#include "srctext.H"
class perltext: public srctext {
  public:
    virtual ATKregistryEntry *ATKregistry();

    void BackwardCheckWord(long from, long to);
    long BackwardSkipString(long pos, char ch);
    long CheckString(long start);
/*    CheckWord(long i,long end) returns long; */
    int Indentation(long pos);
    boolean InString(long pos);
    boolean IsTokenChar(char ch);
    boolean Quoted(long pos);
    long ReverseBalance(long pos);
    void RedoStyles();
    void SetupStyles();

    static boolean InitializeClass();
    perltext();
};

#endif /* _perltext_H_ */
