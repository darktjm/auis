ATK_INTER
#ifndef _consoleClass_H_
#define _consoleClass_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

#define consoleClass_VERSION 1
#define BOXINTERVAL 10

#include <view.H>
class consoleClass : public  view {
public:
	virtual ATKregistryEntry *ATKregistry();


     void FullUpdate(enum view_UpdateType type, long left, long top, long width, long right);
     class view * Hit(enum view_MouseAction action, long x, long y, long numberOfClicks)  ;
     void ReceiveInputFocus();
     void LoseInputFocus();
     void Update();
     void WantUpdate(class view *requestor);
     void PostMenus(class menulist *menulist);

    consoleClass();    
    static boolean InitializeClass();


    class menulist *stdMenulist;
    class menulist *userMenulist;
    boolean haveInputFocus;
    boolean interactive;
    long menuMask;
    boolean is_visible;
};

#endif /* _consoleClass_H_ */
