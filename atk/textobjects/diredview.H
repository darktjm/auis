ATK_INTER
#ifndef _diredview_H_
#define _diredview_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */
/** \addtogroup libtextobjs
 * @{ */
/*
 * Return status values for StartNewDir
 */

#define diredview_Done         0
#define diredview_NoSuchDir   -1
#define diredview_NotADir     -2

#include <textview.H>
class diredview: public  textview {
public:
	virtual ATKregistryEntry *ATKregistry();


    
	static boolean InitializeClass();

    
	diredview();
    
	virtual ~diredview();

     void SetDataObject(class dataobject *object);
     class view * Hit(enum view_MouseAction action, long x, long y,
      long numberOfClicks)  ;
     void PostKeyState(class keystate *keystate);
     void PostMenus(class menulist *menus);
     void ObservedChanged(class observable *changed, long value);


    class keystate *keystate;
    class menulist *menulist;
};
/** #} */
#endif /* _diredview_H_ */
