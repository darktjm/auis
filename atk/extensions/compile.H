ATK_INTER
#ifndef _compile_H_
#define _compile_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/config/COPYRITE'     *
\* ********************************************************************** */
/* $Header: /afs/cs.cmu.edu/project/atk-src-C++/atk/extensions/RCS/compile.H,v 3.3 1994/11/30 20:42:06 rr2b Stab74 $ */
/* $ACIS:compile.ch 1.2$ */
/* $Source: /afs/cs.cmu.edu/project/atk-src-C++/atk/extensions/RCS/compile.H,v $ */

#if !defined(lint) && !defined(LOCORE) && defined(RCS_HDRS)
static char *rcsidcompile_H = "$Header: /afs/cs.cmu.edu/project/atk-src-C++/atk/extensions/RCS/compile.H,v 3.3 1994/11/30 20:42:06 rr2b Stab74 $";
#endif /* !defined(lint) && !defined(LOCORE) && defined(RCS_HDRS) */

#include <ATK.H>

class compile : public ATK  {
public:
	virtual ATKregistryEntry *ATKregistry();

    
        
	static boolean InitializeClass();

};

#endif /* _compile_H_ */
