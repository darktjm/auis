ATK_INTER
#ifndef _dynsearch_H_
#define _dynsearch_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/config/COPYRITE'     *
\* ********************************************************************** */
#include <ATK.H>

class dynsearch : public ATK  {
public:
	virtual ATKregistryEntry *ATKregistry();

    
	
	static boolean InitializeClass();

};

#endif /* _dynsearch_H_ */
