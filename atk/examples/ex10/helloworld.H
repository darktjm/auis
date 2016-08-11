ATK_INTER
#ifndef _helloworld_H_
#define _helloworld_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/config/COPYRITE'     *
\* ********************************************************************** */
/* $Header: /afs/cs.cmu.edu/project/atk-src-C++/atk/examples/ex10/RCS/helloworld.H,v 1.2 1994/11/30 20:42:06 rr2b Stab74 $ */
/* Mouse drag, key bindings, menus, scrollbar, data object */

#define POSUNDEF -1

#include <dataobject.H>
class helloworld: public  dataobject {
public:
	virtual ATKregistryEntry *ATKregistry();
	helloworld();


    long x,y;
    boolean blackOnWhite;
};

#endif /* _helloworld_H_ */
