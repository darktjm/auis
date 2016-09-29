#ifndef _lprruler_H_
#define _lprruler_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

/** \addtogroup libsupviews
 * @{ */
/*
lprruler.H

	lprruler data object

	This module stores nothing and provides no methods.

	The file format for a lprruler has nothing between the lines for
	\begindata and \enddata.

*/

#define  lprruler_PROGRAMMERVERSION    1

/* value for NotifyObservers */
#define lprruler_DATACHANGED 2


#include <dataobject.H>

class lprruler : public  dataobject{
public:
	virtual ATKregistryEntry *ATKregistry();




	 long Read( /* struct lprruler *self, */ FILE *file, long id )  ;
	 long Write( /* struct lprruler *self, */ FILE *file, long id, int level )
			 ;







	
	lprruler();
	
	virtual ~lprruler();



};
/** @} */
#endif /* _lprruler_H_ */
