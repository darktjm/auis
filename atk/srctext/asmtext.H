#ifndef _asmtext_H_
#define _asmtext_H_
/* File asmtext.H created by R S Kemmetmueller
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


   asmtext: a text subclass specialized for dealing with Assembly code. */

#define MAX_BANGCHARS 32 /* arbitrary limit for how many chars can be bang-comment-starters */
#define MAX_TABSTOPS  80 /* arbitrary number for how many Tab Stops can be set in preferences */

#include "srctext.H"
class asmtext: public srctext {
  public:
    virtual ATKregistryEntry *ATKregistry();

    long Indent(mark *range);
    char * Keywordify(char *buff, boolean checkforceupper);
    void RedoStyles();
    void SetAttributes(struct attributes *atts);

    inline void SetCComments(boolean newValue) { (((this)->CComments) = (newValue)); }
    inline boolean UseCComments() { return ((this)->CComments); }
    inline boolean HasReindentFilter() { return ((this)->reindentFilterName!=NULL && *((this)->reindentFilterName)!='\0'); }
    inline char *ReindentFilterName() { return ((this)->reindentFilterName); }

    static boolean InitializeClass();
    asmtext();

    boolean CComments;
    char bangComments[MAX_BANGCHARS+1];
    char *reindentFilterName;
};

#endif /* _asmtext_H_ */
