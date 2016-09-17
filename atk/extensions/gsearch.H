ATK_INTER
#ifndef _gsearch_H_
#define _gsearch_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'     *
\* ********************************************************************** */

#include <ATK.H>

class gsearch : public ATK  {
public:
	virtual ATKregistryEntry *ATKregistry();

    
      
	static boolean InitializeClass();

};

#endif /* _gsearch_H_ */
