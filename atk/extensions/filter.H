ATK_INTER
#ifndef _filter_H_
#define _filter_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */
/** \addtogroup libfilter libfilter.so
 *  A dynamically loadable ATK class file.  Contains filter.
 * @{ */
/* filter the textview selection region through a command
 */

#include <ATK.H>

class filter : public ATK  {
public:
	virtual ATKregistryEntry *ATKregistry();

    
        
	static boolean InitializeClass();

};
/** @} */
#endif /* _filter_H_ */
