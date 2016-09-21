ATK_INTER
#ifndef _onoffV_H_
#define _onoffV_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1989 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

/** \addtogroup libvalue
 * @{ */
#include <fontdesc.H>
#include <rect.h>

#include <buttonV.H>
class onoffV : public  buttonV
{
public:
	virtual ATKregistryEntry *ATKregistry();

 
   void LookupParameters();

  
	static boolean InitializeClass();

  
	onoffV();

};

/* parameters to spst
   label		string		Defaults to NULL
   bodyfont		string		Defaults to "andytype"
   bodyfont-size	long		Defaults to 10
   max-value		long		defaults to 100
  min-value		long		dafaults to 0
  increment		float		defaults to 1.0
*/
/** @} */
#endif /* _onoffV_H_ */
