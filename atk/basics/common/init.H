#ifndef _init_H_
#define _init_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

/** \addtogroup libbasics
 * @{ */
/* init.H
 * Class header file for initialization file reader.
 *
 */

#include <ATK.H>

/** A pointer to a function called to report initialization errors.
 *  \param rock A generic parameter passed down to the reporting function.
 *  \param buf A 0-terminated string describing the error.
 */
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
/** @} */
#endif /* _init_H_ */
