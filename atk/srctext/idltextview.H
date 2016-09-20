#ifndef _idltextview_H_
#define _idltextview_H_
/** \addtogroup libsrctext
 * @{ */
/* File idltextview.H created by R S Kemmetmueller
   (c) Copyright IBM Corp.  1988-1995.  All rights reserved.

   idltextview: a textview subclass specialized for dealing with SOM IDL code. */

#include "cpptextview.H"
class idltextview: public cpptextview {
  public:
    virtual ATKregistryEntry *ATKregistry();

    void PostMenus(menulist *menulist);
    class keystate *PrependKeyState();

    static boolean InitializeClass();
    idltextview();
    virtual ~idltextview();

    class keystate *idl_state;
    class menulist *idl_menus;
};
/** @} */
#endif /* _idltextview_H_ */
