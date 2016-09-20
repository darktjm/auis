#ifndef _asmtextview_H_
#define _asmtextview_H_
/** \addtogroup libsrctext
 * @{ */
/* File asmtextview.H created by R S Kemmetmueller
   (c) Copyright IBM Corp.  1988-1995.  All rights reserved.

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
/** @} */
#endif /* _asmtextview_H_ */
