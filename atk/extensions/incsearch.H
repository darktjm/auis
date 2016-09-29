#ifndef _incsearch_H_
#define _incsearch_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

/** \addtogroup libincsearch libincsearch.so
 *  A dynamically loadable ATK class file.  Contains incsearch.
 * @{ */
#include <ATK.H>

class incsearch : public ATK  {
public:
	virtual ATKregistryEntry *ATKregistry();

    
      
	static boolean InitializeClass();

};
/** @} */
#endif /* _incsearch_H_ */
