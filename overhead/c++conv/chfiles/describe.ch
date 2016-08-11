/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/config/COPYRITE'     *
\* ********************************************************************** */
/* $Header: /afs/cs.cmu.edu/project/atk-src-C++/overhead/c++conv/chfiles/RCS/describe.ch,v 1.1 1993/10/13 15:52:18 rr2b Stab74 $ */
/* $Source: /afs/cs.cmu.edu/project/atk-src-C++/overhead/c++conv/chfiles/RCS/describe.ch,v $ */

#if !defined(lint) && !defined(LOCORE) && defined(RCS_HDRS)
static char *rcsiddescriber_H = "$Header: /afs/cs.cmu.edu/project/atk-src-C++/overhead/c++conv/chfiles/RCS/describe.ch,v 1.1 1993/10/13 15:52:18 rr2b Stab74 $";
#endif /* !defined(lint) && !defined(LOCORE) && defined(RCS_HDRS) */

#include <view.ih>

class describer[describe] {
methods:
    Describe(struct view * viewToDescribe, char * format, FILE * file, long rock) returns enum view_DescriberErrs;
};
