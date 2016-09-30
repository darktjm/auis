#ifndef _figio_H_
#define _figio_H_
/** \addtogroup libfigure
 * @{ */
/* figio.H - various format conversion packages */
/*
  Copyright Carnegie Mellon University 1992 - All rights reserved
  */

#include <ATK.H>
#include <stdio.h>

class figio : public ATK  {
public:
	virtual ATKregistryEntry *ATKregistry();


    

      
	static boolean InitializeClass();

      static long ReadZipFile(FILE *fl, class figure *fig, long parent, double ratio)  ;
};
/** @} */
#endif /* _figio_H_ */
