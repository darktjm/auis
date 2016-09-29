#ifndef _texttag_H_
#define _texttag_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */
/** \addtogroup libtext
 * @{ */
#include <fnote.H>

class texttag : public fnote {
public:
	virtual ATKregistryEntry *ATKregistry();


	 const char * ViewName()  ;

	virtual char * GetTag(long size, char * buf)  ;
};
/** @} */
#endif /* _texttag_H_ */
