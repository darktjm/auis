#ifndef _helloworld_H_
#define _helloworld_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

/* Mouse drag, key bindings, menus, scrollbar, data object, read/write */

#define POSUNDEF (-1)

#include <dataobject.H>
class helloworld: public  dataobject {
public:
	virtual ATKregistryEntry *ATKregistry();


     long Read(FILE *file,long id)  ;
     long Write(FILE *file,long writeId,int level)  ;

    
	helloworld();
    
	virtual ~helloworld();

    long x,y;
    boolean blackOnWhite;
    class dataobject *dobj;
};

#define helloworld_SubObjectChanged 1

#endif /* _helloworld_H_ */
