ATK_INTER
#ifndef _framecmds_H_
#define _framecmds_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

/* framecmd.H
 * A vestigial class to make static loading of framecmds optional.
 *
 */

#include <ATK.H>

class framecmds : public ATK  {
public:
	virtual ATKregistryEntry *ATKregistry();

    
        static class keymap * InitKeymap(class menulist **menuOut, class menulist **defaultmenuOut, class keymap **defkeymap)  ;
        
	static boolean InitializeClass();

        static void DeleteWindow(class frame *self);
};

#endif /* _framecmds_H_ */
