#ifndef _cpptextview_H_
#define _cpptextview_H_
/* File cpptextview.H created by R L Quinn
   (c) Copyright IBM Corp.  1988-1995.  All rights reserved.

   cpptextview: a textview subclass specialized for dealing with C++ code. */

#include "ctextview.H"
class cpptextview: public ctextview {
  public:
    virtual ATKregistryEntry *ATKregistry();

    void PostMenus(class menulist *menulist);
    class keystate* PrependKeyState();

    static boolean InitializeClass();
    cpptextview();
    virtual ~cpptextview();

    class keystate *cpp_state;
    class menulist *cpp_menus;
};

#endif /* _cpptextview_H_ */
