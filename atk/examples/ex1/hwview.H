#ifndef _helloworldview_H_
#define _helloworldview_H_
/*
	Copyright IBM Corporation 1988,1991 - All Rights Reserved
	Copyright Carnegie Mellon University 1996
	For full copyright information see: <src or dest>/doc/COPYRITE
*/
/*Prints "hello world" in center of a view"*/

#include <view.H>
class helloworldview: public  view {
public:
	virtual ATKregistryEntry *ATKregistry();


     void FullUpdate(enum view::UpdateType type, long left, long top, long width, long height);
};

#endif /* _helloworldview_H_ */
