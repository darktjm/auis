ATK_INTER
#ifndef _describer_H_
#define _describer_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/config/COPYRITE'     *
\* ********************************************************************** */
/* $Header: /afs/cs.cmu.edu/project/atk-src-C++/atk/basics/common/RCS/describer.H,v 3.4 1994/11/30 20:42:06 rr2b Stab74 $ */
/* $Source: /afs/cs.cmu.edu/project/atk-src-C++/atk/basics/common/RCS/describer.H,v $ */

#if !defined(lint) && !defined(LOCORE) && defined(RCS_HDRS)
static char *rcsiddescriber_H = "$Header: /afs/cs.cmu.edu/project/atk-src-C++/atk/basics/common/RCS/describer.H,v 3.4 1994/11/30 20:42:06 rr2b Stab74 $";
#endif /* !defined(lint) && !defined(LOCORE) && defined(RCS_HDRS) */

#include <view.H>

#include <ATK.H>

class describer : public ATK  {
public:
	virtual ATKregistryEntry *ATKregistry();


    virtual  view_DescriberErrs Describe(class view * viewToDescribe, char * format, FILE * file, long rock)  ;
};

#endif /* _describer_H_ */
