#ifndef _fourwayV_H_
#define _fourwayV_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1989 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

/** \addtogroup libvalue
 * @{ */
#define fourwayv_NUMLABELS 4
#include <buttonV.H>
class fourwayV : public  buttonV
{
public:
	virtual ATKregistryEntry *ATKregistry();


  
	static boolean InitializeClass();

  
	fourwayV();

   void LookupParameters();

};
/** @} */
#endif /* _fourwayV_H_ */
