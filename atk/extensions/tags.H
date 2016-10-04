#ifndef _tags_H_
#define _tags_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */
/** \addtogroup libtags Object tags
 *  A dynamically loadable ATK class file.  Contains tags.
 * @{ */
/* tags.H -- class header file for tags package */

#include <ATK.H>

class tags : public ATK  {
public:
	virtual ATKregistryEntry *ATKregistry();

    
        
	static boolean InitializeClass();

};
/** @} */
#endif /* _tags_H_ */
