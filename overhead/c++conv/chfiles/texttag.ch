/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/config/COPYRITE'     *
\* ********************************************************************** */
/* $Header: /afs/cs.cmu.edu/project/atk-src-C++/overhead/c++conv/chfiles/RCS/texttag.ch,v 1.1 1993/10/13 15:52:18 rr2b Stab74 $ */
/* $ACIS: $ */
/* $Source: /afs/cs.cmu.edu/project/atk-src-C++/overhead/c++conv/chfiles/RCS/texttag.ch,v $ */

#if !defined(lint) && !defined(LOCORE) && defined(RCS_HDRS)
static char *rcsid_celview_H = "$Header: /afs/cs.cmu.edu/project/atk-src-C++/overhead/c++conv/chfiles/RCS/texttag.ch,v 1.1 1993/10/13 15:52:18 rr2b Stab74 $ ";
#endif

class texttag :fnote {
overrides:
	ViewName() returns char *;
methods:
	GetTag(long size, char * buf) returns char *;
};

