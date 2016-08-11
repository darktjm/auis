#ifndef _asmtextview_H_
#define _asmtextview_H_
/* File asmtextview.H created by R S Kemmetmueller
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
 
   asmtextview: a textview subclass specialized for dealing with Assembly code. */

#include "srctextview.H"
class asmtextview: public srctextview {
  public:
    virtual ATKregistryEntry *ATKregistry();

    void EndComment(char key);
    void PostMenus(class menulist *menulist);
    class keystate *PrependKeyState();
    void Reindent();
    void SetDataObject(class dataobject *dataobj);
    void StartComment(char key);
    void StartLineComment(char key);

    static boolean InitializeClass();
    asmtextview();
    virtual ~asmtextview();

    class keystate *asm_state;
    class menulist *asm_menus;    
};

#endif /* _asmtextview_H_ */