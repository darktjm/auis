ATK_INTER
#ifndef _helloworldview_H_
#define _helloworldview_H_
/* 
	Copyright IBM Corporation 1988,1991 - All Rights Reserved
	Copyright Carnegie Mellon University 1996
	For full copyright information see: <src or dest>/doc/COPYRITE
*/


#include <view.H>
class helloworldview : public  view {
public:
	virtual ATKregistryEntry *ATKregistry();

 
     void FullUpdate(enum view_UpdateType type, long left, long top, long
width, long height);
     void Update();
     class view * Hit (enum view_MouseAction action, long x, long y, long numberOfClicks)
 ;
     void ReceiveInputFocus();
     void LoseInputFocus();
 
    helloworldview();
	
	static boolean InitializeClass();

 
    class keystate *keystate;
    long x,y;
    long distX,distY;
    long newX,newY;
    boolean HaveDownTransition;
    boolean haveInputFocus;
    boolean blackOnWhite;
    boolean newBlackOnWhite;
};


#endif /* _helloworldview_H_ */
