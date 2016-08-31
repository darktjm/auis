#ifndef _eosbutton_H_
#define _eosbutton_H_
/*
 * eosbutt.ch
 *
 * Package for creating lists of buttons.
 */

/* ************************************************************
 *  Copyright (C) 1989, 1990, 1991
 *  by the Massachusetts Institute of Technology
 *   For full copyright information see:'mit-copyright.h'     *
 ************************************************************ */

#include <mit-copyright.h>
#include "environ.H"
#include "lpair.H"
#include "pushbutton.H"
#include "newbuttonview.H"

/* Structure is linked list of buttons */     
struct buttonList {
  struct buttonList *next;
  class pushbutton *butt;
  class newbuttonview *buttv;
};

struct lplist {
    struct lplist *next;
    class lpair *lp;
};

#include <ATK.H>
class eosbutton : public ATK  {
public:
	virtual ATKregistryEntry *ATKregistry();

    
      static struct buttonList * MakeButton (struct buttonList *blist,char *text, observable_fptr, class view *object)  ;
      static struct lplist * MakeLpair (struct lplist *lpl)  ;
};

#endif /* _eosbutton_H_ */
