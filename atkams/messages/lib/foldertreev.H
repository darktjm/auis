#ifndef _foldertreev_H_
#define _foldertreev_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/config/COPYRITE'     *
\* ********************************************************************** */

#include <orgv.H>
class foldertreev: public orgv {
public:
	virtual ATKregistryEntry *ATKregistry();

  
    
	static boolean InitializeClass();
	static boolean atk_inited;

    
	foldertreev();
    
	virtual ~foldertreev();
  
     void FullUpdate( enum view_UpdateType type, long left, long top, long width, long height )  ;
     void PostMenus( class menulist *ml)  ;
     void PostKeyState( class keystate *keystate )  ;
  
    class menulist	*menulistp;
    class keystate	*keystatep;
};

#endif /* _foldertreev_H_ */
