ATK_INTER
#ifndef _compchar_H_
#define _compchar_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'     *
\* ********************************************************************** */

#include <ATK.H>

class compchar : public ATK  {
public:
	virtual ATKregistryEntry *ATKregistry();


    
	static boolean InitializeClass();

};

#endif /* _compchar_H_ */
