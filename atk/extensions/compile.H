#ifndef _compile_H_
#define _compile_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */
/** \addtogroup libcompile Object compile
 *  A dynamically loadable ATK class file.  Contains compile.
 * @{ */
#include <ATK.H>

class compile : public ATK  {
public:
	virtual ATKregistryEntry *ATKregistry();

    
        
	static boolean InitializeClass();

};
/** @} */
#endif /* _compile_H_ */
