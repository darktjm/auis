#ifndef _rexxtextview_H_
#define _rexxtextview_H_
/* File rexxtextview.H created by R S Kemmetmueller
   (c) Copyright IBM Corp.  1988-1995.  All rights reserved.

   rexxtextview: a textview subclass specialized for dealing with REXX code. */

#include "srctextview.H"
class rexxtextview: public srctextview {
  public:
    virtual ATKregistryEntry *ATKregistry();

    void EndComment(char key);
    void PostMenus(class menulist *menulist);
    class keystate *PrependKeyState();

    static boolean InitializeClass();
    rexxtextview();
    virtual ~rexxtextview();

    class keystate *rexx_state;
    class menulist *rexx_menus;
};

#endif /* _rexxtextview_H_ */
