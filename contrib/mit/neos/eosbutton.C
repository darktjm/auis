/* $Header: /afs/cs.cmu.edu/project/atk-src-C++/contrib/mit/neos/RCS/eosbutton.C,v 1.6 1996/06/11 01:26:13 wjh Exp $ */
/* $Source: /afs/cs.cmu.edu/project/atk-src-C++/contrib/mit/neos/RCS/eosbutton.C,v $ */
/* $Author: wjh $ */

#ifndef lint
static char *eosbut_rcsid = "$Header: /afs/cs.cmu.edu/project/atk-src-C++/contrib/mit/neos/RCS/eosbutton.C,v 1.6 1996/06/11 01:26:13 wjh Exp $";
#endif /* lint */
/*
 * eosbutt.c
 *
 * Package for creating lists of buttons.
*/

/* ************************************************************
 *  Copyright (C) 1989, 1990, 1991
 *  by the Massachusetts Institute of Technology
 *   For full copyright information see:'mit-copyright.h'     *
 ************************************************************ */

#include <andrewos.h>
#include <mit-copyright.h>

#include "eosbutton.H"


ATKdefineRegistry(eosbutton, ATK, NULL);
#ifndef lint
#endif /* lint */


struct buttonList *eosbutton::MakeButton(struct buttonList  *blist, char  *text, observable_fptr function, class view  *object) 
/* Creates a new button and with given attributes
   returns [newbutton::blist]
 */
{
    struct buttonList *button = (struct buttonList *)     malloc(sizeof(struct buttonList));
    int style;

    style = environ::GetProfileInt("buttonstyle", 2);

    button->butt  = new class pushbutton;
    button->buttv = new class newbuttonview;
    (button->butt)->SetStyle( style);
    (button->butt)->SetText( text);
    (button->buttv)->SetDataObject( button->butt);
    (button->buttv)->AddRecipient(   atom::Intern("buttonpushed"), object, function, 0L);
      
    button->next = blist;
    return button;
}

struct lplist *eosbutton::MakeLpair(struct lplist  *lpl)
/* Creates a new lpair and returns [newlpair::lplist]
 */
{
    struct lplist *lpair = (struct lplist *) malloc(sizeof(struct lplist));
    
    lpair->lp = new class lpair;
    lpair->next = lpl;
    return lpair;
}

