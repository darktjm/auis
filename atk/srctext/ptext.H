#ifndef _ptext_H_
#define _ptext_H_
/* File ptext.H
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

   ptext: a text subclass specialized for dealing with Pascal code. */

#define KEYWRD 1     /* keywords */

#define idstyle_PLAIN   0
#define idstyle_UPPER   1
#define idstyle_LOWER   2
#define idstyle_CAPIT   3

#include "srctext.H"
class ptext: public srctext {
  public:
    virtual ATKregistryEntry *ATKregistry();

    void SetAttributes(struct attributes *atts);
    void SetupStyles();
    void RedoStyles();
    int Indentation(long pos);
    void BackwardCheckWord(long pos, long first);
    char *Keywordify(char *buff, boolean checkforceupper);
    boolean IsTokenChar(char ch);

    static boolean InitializeClass();
    ptext();

    /* Variables used to control the indenting style. */
    int caseLevelIndent, slashAsteriskComments, idStyle;
};

#endif /* _ptext_H_ */
