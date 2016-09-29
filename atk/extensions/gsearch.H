#ifndef _gsearch_H_
#define _gsearch_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

/** \addtogroup libgsearch libgsearch.so
 *  A dynamically loadable ATK class file.  Contains gsearch.
 * @{ */
#include <ATK.H>

class gsearch : public ATK  {
public:
	virtual ATKregistryEntry *ATKregistry();

    
      
	static boolean InitializeClass();

};
/** @} */
#endif /* _gsearch_H_ */
