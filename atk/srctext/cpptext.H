#ifndef _cpptext_H_
#define _cpptext_H_
/* File cpptext.H created by R L Quinn
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

   cpptext: a text subclass specialized for dealing with C++ code. */

/* kinds of styles that are used in hash table */
/*ctext KEYWRD 1     / * keywords */
/*ctext PREPRC 2     / * preprocessor directives */
#define CLASS  3     /* class definition */

/* bit values used for indentation identification */
#define	CLASS_BIT	1   /* bit 0 */
#define	ACSCTRL_BIT	2   /* bit 1 */
#define TEMPLATE_BIT	4   /* bit 2 */
/*note: CLASS_BIT keywords do NOT get wrapped with kindStyle[CLASS], and ACSCTRL_BIT isn't used anyway, because ctext and cpptext still use the old match() method*/

#include "ctext.H"
class cpptext: public ctext {
  public:
    virtual ATKregistryEntry *ATKregistry();

    void BackwardCheckLabel(long pos);
    void BreakLine(class mark *endofline);
    long CheckWord(long i, long end);
    int Indentation(long pos);
    void RedoStyles();
    boolean ReflowComment(long pos);
    void SetAttributes(struct attributes *atts);
    void SetupStyles();

    static boolean InitializeClass();
    cpptext();

    boolean inClassDef;
    int acsctrlOutdent;
};

#endif /* _cpptext_H_ */
