ATK_INTER
#ifndef _filter_H_
#define _filter_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/config/COPYRITE'     *
\* ********************************************************************** */
/* $Header: /afs/cs.cmu.edu/project/atk-src-C++/atk/extensions/RCS/filter.H,v 3.3 1994/11/30 20:42:06 rr2b Stab74 $ */
/* $ACIS:filter.ch 1.2$ */
/* $Source: /afs/cs.cmu.edu/project/atk-src-C++/atk/extensions/RCS/filter.H,v $ */

#if !defined(lint) && !defined(LOCORE) && defined(RCS_HDRS)
static char *rcsidfilter_H = "$Header: /afs/cs.cmu.edu/project/atk-src-C++/atk/extensions/RCS/filter.H,v 3.3 1994/11/30 20:42:06 rr2b Stab74 $";
#endif /* !defined(lint) && !defined(LOCORE) && defined(RCS_HDRS) */

/* filter the textview selection region through a command
 */

#include <ATK.H>

class filter : public ATK  {
public:
	virtual ATKregistryEntry *ATKregistry();

    
        
	static boolean InitializeClass();

};

#endif /* _filter_H_ */
