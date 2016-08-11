ATK_INTER
#ifndef _init_H_
#define _init_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/config/COPYRITE'     *
\* ********************************************************************** */

/*
	$Disclaimer: 
// Permission to use, copy, modify, and distribute this software and its 
// documentation for any purpose and without fee is hereby granted, provided 
// that the above copyright notice appear in all copies and that both that 
// copyright notice and this permission notice appear in supporting 
// documentation, and that the name of IBM not be used in advertising or 
// publicity pertaining to distribution of the software without specific, 
// written prior permission. 
//                         
// THE COPYRIGHT HOLDERS DISCLAIM ALL WARRANTIES WITH REGARD 
// TO THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF 
// MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL ANY COPYRIGHT 
// HOLDER BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL 
// DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, 
// DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE 
// OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION 
// WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
// 
//  $
*/


 

/* init.H
 * Class header file for initialization file reader.
 *
 */

#include <ATK.H>

typedef void (*init_fptr)(long rock, char *buf);

struct menus {
    struct menus *next;
    char *class_c;
    boolean inherit;
    class menulist *menulist;
    class menulist *orig;
};


class init : public ATK   {
public:
	virtual ATKregistryEntry *ATKregistry(); /* error function */
        
	init();
	
	virtual ~init();
    
        virtual class menulist * ModifyMenulist(class menulist *ml)  ;
        virtual class keystate * ModifyKeystate(class keystate *ks)  ;
        virtual class init * Duplicate()  ;
        virtual int Load(char *filename, init_fptr errorProc, long errorRock, boolean executeImmediately)  ;
        virtual void AddKeyBinding(char *class_c, boolean inherit, class keymap *keymap);
        virtual void DeleteKeyBinding(char *class_c, boolean inherit, class keymap *keymap);
        virtual void AddMenuBinding(char *class_c, boolean inherit, class menulist *menulist);
        virtual void DeleteMenuBinding(char *class_c, boolean inherit, class menulist *menulist);

    
        struct keys *keys;
        struct menus *menus;
        struct keystateList *usedKeystates;
	struct mlList *usedMenus;
	struct children *kids;
	class init *parent;
	int version;
	struct menus *buttons;	// using menulist for now.
};

#endif /* _init_H_ */