#ifndef _ctextview_H_
#define _ctextview_H_
/** \addtogroup libsrctext
 * @{ */
/* File ctextview.H created by R L Quinn
   (c) Copyright IBM Corp.  1988-1995.  All rights reserved.

   ctextview: a textview subclass specialized for dealing with C code. */

#include "srctextview.H"
class ctextview: public srctextview {
  public:
    TRACED_CLASS(ctextview);

    void HandleEndOfLineStyle(long pos);
    void Paren(char key);
    void PostMenus(menulist *menulist);
    class keystate *PrependKeyState();

    static boolean InitializeClass();
    ctextview();

    class keystate *c_state;
    class menulist *c_menus;
};
/** @} */
#endif /* _ctextview_H_ */
