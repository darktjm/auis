#ifndef _fcomp_H_
#define _fcomp_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */
/** \addtogroup libtypescript
 * @{ */
/* fcomp.ch -- typescript package to perform filename completion
  * David Anderson
  * August, 1988
  */

#include <ATK.H>
class fcomp : public ATK  {
public:
	virtual ATKregistryEntry *ATKregistry();

    
      
	static boolean InitializeClass();

};
/** @} */
#endif /* _fcomp_H_ */
