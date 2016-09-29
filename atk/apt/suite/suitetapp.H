#ifndef _suitetapp_H_
#define _suitetapp_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */
/** \addtogroup libsuite
 * @{ */
#include <application.H>
class suitetapp : public  application 
  {
public:
	virtual ATKregistryEntry *ATKregistry();

  
    boolean Start()				     ;
    boolean ParseArgs( int argc, const char **argv )	     ;
  };
/** @} */
#endif /* _suitetapp_H_ */
