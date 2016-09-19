ATK_INTER
#ifndef _bpair_H_
#define _bpair_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

/*
bpair.H

	Class definitions for bpair

*/

#define  bpair_PROGRAMMERVERSION    1

#include <lpair.H>

class bpair: public  lpair
{
public:
	virtual ATKregistryEntry *ATKregistry();



	 void FullUpdate(/* struct bpair *self, */ enum view_UpdateType type, 
			long left, long top, long width, long height);



	
	bpair();
	
	virtual ~bpair();

};

#endif /* _bpair_H_ */
