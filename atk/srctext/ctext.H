#ifndef _ctext_H_
#define _ctext_H_
/* File ctext.H created by R L Quinn
   (c) Copyright IBM Corp.  1988-1995.  All rights reserved.

$Disclaimer: 
// Permission to use, copy, modify, and distribute this software and its 
// documentation for any purpose and without fee is hereby granted, provided 
// that the above copyright notice appear in all copies and that both that 
// copyright notice and this permission notice appear in supporting 
// documentation, and that the name of IBM not be used in advertising or 
// publicity pertaining to distribution of the software without specific, 
// written prior permission. 
//                         
// THE COPYRIGHT HOLDERS DISCLAIM ALL WARRANTIES WITH REGARD 
// TO THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF 
// MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL ANY COPYRIGHT 
// HOLDER BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL 
// DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, 
// DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE 
// OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION 
// WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
// 
//  $

   ctext: a text subclass specialized for dealing with C code. */

/* kinds of styles that are used in hash table */
#define KEYWRD 1     /* keywords */
#define PREPRC 2     /* preprocessor directives */

#include "srctext.H"
class ctext: public srctext {
  public:
    virtual ATKregistryEntry *ATKregistry();

    int Indentation(long pos);
    boolean IsTokenChar(char ch);
    boolean Quoted(long pos);
    void RedoStyles();
    void SetAttributes(struct attributes *atts);
    void SetupStyles();

    virtual long BackwardSkipDelimited(long pos, char begin, char end);
    virtual long BackwardSkipJunk(long pos);
    virtual long CheckPreproc(long start);
    virtual long MaybeBackwardSkipCppLines(long pos);
    virtual long UnstyledCommentStart(long pos);

    static boolean InitializeClass();
    ctext();

    /* Variables used to control the indenting style. */
    int braceIndent, switchLabelUndent, switchLevelIndent;
    boolean outdentPreproc;
};

#endif /* _ctext_H_ */
