#ifndef _helloworldview_H_
#define _helloworldview_H_
/* 
	Copyright IBM Corporation 1988,1991 - All Rights Reserved
	Copyright Carnegie Mellon University 1996
	For full copyright information see: <src or dest>/doc/COPYRITE
*/
/* hello world respond to mouse hits*/

#include <view.H>
class helloworldview: public  view {
public:
	virtual ATKregistryEntry *ATKregistry();
	helloworldview();


     void FullUpdate(enum view::UpdateType type, long left, long top, long width, long height);
     void Update();
     class view * Hit (enum view::MouseAction action, long x, long y, long numberOfClicks)  ;
  
    long x;
    long y;
    long newX;
    long newY;
    boolean HaveDownTransition;
};

#endif /* _helloworldview_H_ */
