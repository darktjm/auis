#ifndef _filler_H_
#define _filler_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

/** \addtogroup liblayout
 * @{ */
/* filler.H - filler data object definition and interface */

#include <rect.h>

#define filler_VERSION 1

/* globals for entire package */

#include <cel.H>
class filler: public  cel {
public:
	virtual ATKregistryEntry *ATKregistry();



   const char * ViewName()  ;

};

/* end of filler.H */
/** @} */
#endif /* _filler_H_ */
