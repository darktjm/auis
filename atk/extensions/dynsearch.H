ATK_INTER
#ifndef _dynsearch_H_
#define _dynsearch_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/config/COPYRITE'     *
\* ********************************************************************** */
/* $Header: /afs/cs.cmu.edu/project/atk-src-C++/atk/extensions/RCS/dynsearch.H,v 3.3 1994/11/30 20:42:06 rr2b Stab74 $ */
/* $ACIS:dsearch.ch 1.2$ */
/* $Source: /afs/cs.cmu.edu/project/atk-src-C++/atk/extensions/RCS/dynsearch.H,v $ */

#if !defined(lint) && !defined(LOCORE) && defined(RCS_HDRS)
static char *rcsiddynsearch_H = "$Header: /afs/cs.cmu.edu/project/atk-src-C++/atk/extensions/RCS/dynsearch.H,v 3.3 1994/11/30 20:42:06 rr2b Stab74 $";
#endif /* !defined(lint) && !defined(LOCORE) && defined(RCS_HDRS) */

#include <ATK.H>

class dynsearch : public ATK  {
public:
	virtual ATKregistryEntry *ATKregistry();

    
	
	static boolean InitializeClass();

};

#endif /* _dynsearch_H_ */
