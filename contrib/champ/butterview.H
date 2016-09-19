#ifndef _butterview_H_
#define _butterview_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */
#include <view.H>
class butterview: public  view {
public:
	virtual ATKregistryEntry *ATKregistry();

    
       void FullUpdate(enum view_UpdateType type, long left, long top, long width, long height);
       void Update();
       class view * Hit (enum view_MouseAction action, long x, long y, long numberOfClicks)  ;
};


#endif /* _butterview_H_ */
