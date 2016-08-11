ATK_INTER
#ifndef _texttag_H_
#define _texttag_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/config/COPYRITE'     *
\* ********************************************************************** */
/* $Header: /afs/cs.cmu.edu/project/atk-src-C++/atk/text/RCS/texttag.H,v 3.3 1994/11/30 20:42:06 rr2b Stab74 $ */
/* $ACIS: $ */
/* $Source: /afs/cs.cmu.edu/project/atk-src-C++/atk/text/RCS/texttag.H,v $ */

#if !defined(lint) && !defined(LOCORE) && defined(RCS_HDRS)
static char *rcsid_celview_H = "$Header: /afs/cs.cmu.edu/project/atk-src-C++/atk/text/RCS/texttag.H,v 3.3 1994/11/30 20:42:06 rr2b Stab74 $ ";
#endif

#include <fnote.H>

class texttag : public fnote {
public:
	virtual ATKregistryEntry *ATKregistry();


	 char * ViewName()  ;

	virtual char * GetTag(long size, char * buf)  ;
};


#endif /* _texttag_H_ */
