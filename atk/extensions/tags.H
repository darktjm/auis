ATK_INTER
#ifndef _tags_H_
#define _tags_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/config/COPYRITE'     *
\* ********************************************************************** */
/* $Header: /afs/cs.cmu.edu/project/atk-src-C++/atk/extensions/RCS/tags.H,v 3.3 1994/11/30 20:42:06 rr2b Stab74 $ */
/* $ACIS:tags.ch 1.2$ */
/* $Source: /afs/cs.cmu.edu/project/atk-src-C++/atk/extensions/RCS/tags.H,v $ */

#if !defined(lint) && !defined(LOCORE) && defined(RCS_HDRS)
static char *rcsidtags_H = "$Header: /afs/cs.cmu.edu/project/atk-src-C++/atk/extensions/RCS/tags.H,v 3.3 1994/11/30 20:42:06 rr2b Stab74 $";
#endif /* !defined(lint) && !defined(LOCORE) && defined(RCS_HDRS) */

/* tags.ch -- class header file for tags package */

#include <ATK.H>

class tags : public ATK  {
public:
	virtual ATKregistryEntry *ATKregistry();

    
        
	static boolean InitializeClass();

};

#endif /* _tags_H_ */