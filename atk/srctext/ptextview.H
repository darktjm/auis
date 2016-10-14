#ifndef _ptextview_H_
#define _ptextview_H_
/** \addtogroup libsrctext
 * @{ */
/* File ptextview.H
   (c) Copyright IBM Corp.  1988-1995.  All rights reserved.

   ptextview: a textview subclass specialized for dealing with Pascal code. */

#include "srctextview.H"
class ptextview: public srctextview {
  public:
    TRACED_CLASS(ptextview);

    class keystate *PrependKeyState();
    void PostMenus(class menulist *menulist);
    void Paren(char key);
    void RenameIdent();
    void HandleNewlineAndRetrn(long key, boolean reindentThisLine, boolean preindentNewLine);

    static boolean InitializeClass();
    ptextview();

    class keystate *c_state;
    class menulist *c_menus;    
};
/** @} */
#endif /* _ptextview_H_ */
