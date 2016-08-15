ATK_INTER
#ifndef _previewapp_H_
#define _previewapp_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/config/COPYRITE'     *
\* ********************************************************************** */
/* $Header: /afs/cs.cmu.edu/project/atk-src-C++/contrib/preview/RCS/previewapp.H,v 1.4 1994/11/30 20:42:06 rr2b Stab74 $ */
/* $ACIS:previewa.ch 1.2$ */
/* $Source: /afs/cs.cmu.edu/project/atk-src-C++/contrib/preview/RCS/previewapp.H,v $ */

#if !defined(lint) && !defined(LOCORE) && defined(RCS_HDRS)
static char *rcsidpreviewapp_H = "$Header: /afs/cs.cmu.edu/project/atk-src-C++/contrib/preview/RCS/previewapp.H,v 1.4 1994/11/30 20:42:06 rr2b Stab74 $";
#endif /* !defined(lint) && !defined(LOCORE) && defined(RCS_HDRS) */

#include <application.H>
class previewapp : public  application {
public:
	virtual ATKregistryEntry *ATKregistry();

    
      
	previewapp();
    
	 boolean ParseArgs(int argc,const char **argv)  ;
         boolean Start()  ;
	 int Run()  ;
};

#endif /* _previewapp_H_ */
