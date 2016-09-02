#ifndef _perltext_H_
#define _perltext_H_
/* File perltext.H created by W R Nelson
   (c) Copyright IBM Corp.  1988-1995.  All rights reserved.

$Disclaimer: 
 * Permission to use, copy, modify, and distribute this software and its 
 * documentation for any purpose and without fee is hereby granted, provided 
 * that the above copyright notice appear in all copies and that both that 
 * copyright notice and this permission notice appear in supporting 
 * documentation, and that the name of IBM not be used in advertising or 
 * publicity pertaining to distribution of the software without specific, 
 * written prior permission. 
 *                         
 * THE COPYRIGHT HOLDERS DISCLAIM ALL WARRANTIES WITH REGARD 
 * TO THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF 
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL ANY COPYRIGHT 
 * HOLDER BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL 
 * DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, 
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE 
 * OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION 
 * WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 * 
 *  $

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
