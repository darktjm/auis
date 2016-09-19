#ifndef _modtextview_H_
#define _modtextview_H_
/* File modtextview.H created by R L Quinn
   (c) Copyright IBM Corp.  1988-1995.  All rights reserved.

   modtextview: a textview subclass specialized for dealing with Modula-X code. */

#include "srctextview.H"
class modtextview: public srctextview {
  public:
    virtual ATKregistryEntry *ATKregistry();

    void HandleEndOfLineStyle(long pos);
    void Paren(char key);
    void PostMenus(class menulist *menulist);
    class keystate *PrependKeyState();

    static boolean InitializeClass();
    modtextview();
    virtual ~modtextview();

    class keystate *mod_state;
    class menulist *mod_menus;    
};

#endif /* _modtextview_H_ */
