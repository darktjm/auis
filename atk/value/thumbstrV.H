#ifndef _thumbstrV_H_
#define _thumbstrV_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1989 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

/** \addtogroup libvalue
 * @{ */
#include <fontdesc.H>
#include <rect.h>

#include <thumbV.H>
class thumbstrV : public  thumbV {
public:
	TRACED_CLASS_NODEST(thumbstrV);

 
   const char * GetValueString()  ;
   class valueview  * DoHit(enum view::MouseAction type, long x,long y,long hits)
     ;
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
#endif /* _thumbstrV_H_ */
