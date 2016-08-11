/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/config/COPYRITE'     *
\* ********************************************************************** */
 
/* $Header: /afs/cs.cmu.edu/project/atk-src-C++/overhead/c++conv/chfiles/RCS/bp.ch,v 1.1 1993/10/13 15:52:18 rr2b Stab74 $ */

#if !defined(lint) && !defined(LOCORE) && defined(RCS_HDRS)
static char *rcsid_bp_ch = "$Header: /afs/cs.cmu.edu/project/atk-src-C++/overhead/c++conv/chfiles/RCS/bp.ch,v 1.1 1993/10/13 15:52:18 rr2b Stab74 $ ";
#endif

class bp : dataobject [dataobj] {
overrides:
      ViewName() returns char *;
};

