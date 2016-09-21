ATK_INTER
#ifndef _metax_H_
#define _metax_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

/** \addtogroup libmetax libmetax.so
 *  A dynamically loadable ATK class file.  Contains metax.
 * @{ */
/* metax package for ATK by Rob Ryan March 19, 1989 */

#include <ATK.H>

class metax : public ATK  {
public:
	virtual ATKregistryEntry *ATKregistry();


    
	static boolean InitializeClass();

};
/** @} */
#endif /* _metax_H_ */
