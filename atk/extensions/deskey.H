#ifndef _deskey_H_
#define _deskey_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */
/** \addtogroup libdeskey libdeskey.so
 *  A dynamically loadable ATK class file.  Contains deskey.
 * @{ */
/* deskey.H -- EZ package to describe key sequences.
 * David Nichols
 * September, 1987 
 */

#include <ATK.H>

class deskey : public ATK  {
public:
	virtual ATKregistryEntry *ATKregistry();

    
	
	static boolean InitializeClass();

};
/** @} */
#endif /* _deskey_H_ */
