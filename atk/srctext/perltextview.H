#ifndef _perltextview_H_
#define _perltextview_H_
/* File perltextview.H created by W R Nelson
   (c) Copyright IBM Corp.  1988-1995.  All rights reserved.

   perltextview: a textview subclass specialized for dealing with perl code. */

#include "srctextview.H"
class perltextview: public srctextview {
  public:
    virtual ATKregistryEntry *ATKregistry();

    void HandleEndOfLineStyle(long pos);
    void Paren(char key);
    void PostMenus(class menulist *menulist);
    class keystate *PrependKeyState();

    static boolean InitializeClass();
    perltextview();
    virtual ~perltextview();

    class keystate *perl_state;
    class menulist *perl_menus;
};

#endif /* _perltextview_H_ */
