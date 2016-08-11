ATK_INTER
#ifndef _incsearch_H_
#define _incsearch_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/config/COPYRITE'     *
\* ********************************************************************** */
/* $Header: /afs/cs.cmu.edu/project/atk-src-C++/atk/extensions/RCS/incsearch.H,v 3.3 1994/11/30 20:42:06 rr2b Stab74 $ */
/* $ACIS:incrsearch.ch 1.2$ */
/* $Source: /afs/cs.cmu.edu/project/atk-src-C++/atk/extensions/RCS/incsearch.H,v $ */

#ifndef lint
static char *rcsidincsearch_H = "$Header: /afs/cs.cmu.edu/project/atk-src-C++/atk/extensions/RCS/incsearch.H,v 3.3 1994/11/30 20:42:06 rr2b Stab74 $";
#endif /* lint */

#include <ATK.H>

class incsearch : public ATK  {
public:
	virtual ATKregistryEntry *ATKregistry();

    
      
	static boolean InitializeClass();

};

#endif /* _incsearch_H_ */
