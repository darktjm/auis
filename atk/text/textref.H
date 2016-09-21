ATK_INTER
#ifndef _textref_H_
#define _textref_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */
/** \addtogroup libtext
 * @{ */
#include <fnote.H>

class textref : public fnote {
public:
	virtual ATKregistryEntry *ATKregistry();


	 const char * ViewName()  ;

	virtual char * GetRef(long size, char * buf)  ;
};
/** @} */
#endif /* _textref_H_ */
