#ifndef _mtextview_H_
#define _mtextview_H_
/** \addtogroup libsrctext
 * @{ */
/* File mtextview.H created by R L Quinn
   (c) Copyright IBM Corp.  1988-1995.  All rights reserved.

   mtextview: a textview subclass specialized for dealing with Modula-2 code. */

#include "modtextview.H"
class mtextview: public modtextview {
  public:
    TRACED_CLASS(mtextview);

    void PostMenus(class menulist *menulist);
    class keystate *PrependKeyState();

    static boolean InitializeClass();
    mtextview();

    class keystate *m_state;
    class menulist *m_menus;    
};
/** @} */
#endif /* _mtextview_H_ */
