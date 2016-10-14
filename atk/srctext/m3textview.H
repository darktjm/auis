#ifndef _m3textview_H_
#define _m3textview_H_
/** \addtogroup libsrctext
 * @{ */
/* File m3textview.H created by R L Quinn
   (c) Copyright IBM Corp.  1988-1995.  All rights reserved.

   m3textview: a textview subclass specialized for dealing with Modula-3 code. */

#include "modtextview.H"
class m3textview: public modtextview {
  public:
    TRACED_CLASS(m3textview);

    void PostMenus(class menulist *menulist);
    class keystate *PrependKeyState();

    static boolean InitializeClass();
    m3textview();

    class keystate *m3_state;
    class menulist *m3_menus;    
};
/** @} */
#endif /* _m3textview_H_ */
