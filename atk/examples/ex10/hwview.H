#ifndef _helloworldview_H_
#define _helloworldview_H_
/* 
	Copyright IBM Corporation 1988,1991 - All Rights Reserved
	Copyright Carnegie Mellon University 1996
	For full copyright information see: <src or dest>/doc/COPYRITE
*/

/* Mouse drag, key bindings, menus, scroll bar, messages, prompting for input */

#include <view.H>
class helloworldview: public  view {
public:
	virtual ATKregistryEntry *ATKregistry();


     void FullUpdate(enum view_UpdateType type, long left, long top, long width, long height);
     void Update();
     class view * Hit (enum view_MouseAction action, long x, long y, long numberOfClicks)  ;
     void ReceiveInputFocus();
     void LoseInputFocus();
     class view * GetApplicationLayer()  ;
     void DeleteApplicationLayer(class view *v);
     const void * GetInterface(const char *type)  ;

    helloworldview();
	
	static boolean InitializeClass();


    class keystate *keystate;
    class menulist *menulistDup;
    boolean haveInputFocus;
    boolean HaveDownTransition;
    long hitX,hitY;
    long x,y;
    boolean blackOnWhite;
    long frameX, frameY;
    long newFrameX, newFrameY;
    int vrWidth,vrHeight;
};

#endif /* _helloworldview_H_ */
