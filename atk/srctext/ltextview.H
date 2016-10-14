#ifndef _ltextview_H_
#define _ltextview_H_
/** \addtogroup libsrctext
 * @{ */
/* File ltextview.H
   (c) Copyright IBM Corp.  1988-1995.  All rights reserved.

   ltextview: a textview subclass specialized for dealing with Lisp code. */

#include "srctextview.H"
class ltextview: public srctextview {
  public:
    TRACED_CLASS(ltextview);

    class keystate *PrependKeyState();
    void PostMenus(class menulist *menulist);

    static boolean InitializeClass();
    ltextview();

    class keystate *l_state;
    class menulist *l_menus;    
};
/** @} */
#endif /* _ltextview_H_ */
