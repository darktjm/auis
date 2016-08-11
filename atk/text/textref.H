ATK_INTER
#ifndef _textref_H_
#define _textref_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/config/COPYRITE'     *
\* ********************************************************************** */
/* $Header: /afs/cs.cmu.edu/project/atk-src-C++/atk/text/RCS/textref.H,v 1.4 1994/11/30 20:42:06 rr2b Stab74 $ */
/* $ACIS: $ */
/* $Source: /afs/cs.cmu.edu/project/atk-src-C++/atk/text/RCS/textref.H,v $ */

#if !defined(lint) && !defined(LOCORE) && defined(RCS_HDRS)
static char *rcsid_celview_H = "$Header: /afs/cs.cmu.edu/project/atk-src-C++/atk/text/RCS/textref.H,v 1.4 1994/11/30 20:42:06 rr2b Stab74 $ ";
#endif

#include <fnote.H>

class textref : public fnote {
public:
	virtual ATKregistryEntry *ATKregistry();


	 char * ViewName()  ;

	virtual char * GetRef(long size, char * buf)  ;
};


#endif /* _textref_H_ */
