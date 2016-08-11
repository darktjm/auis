ATK_INTER
#ifndef _ezprintapp_H_
#define _ezprintapp_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/config/COPYRITE'     *
\* ********************************************************************** */
/* $Header: /afs/cs.cmu.edu/project/atk-src-C++/atk/ezprint/RCS/ezprintapp.H,v 1.5 1994/11/30 20:42:06 rr2b Stab74 $ */
/* $ACIS:ezprinta.ch 1.2$ */
/* $Source: /afs/cs.cmu.edu/project/atk-src-C++/atk/ezprint/RCS/ezprintapp.H,v $ */

#if !defined(lint) && !defined(LOCORE) && defined(RCS_HDRS)
static char *rcsidezprintapp_H = "$Header: /afs/cs.cmu.edu/project/atk-src-C++/atk/ezprint/RCS/ezprintapp.H,v 1.5 1994/11/30 20:42:06 rr2b Stab74 $";
#endif /* !defined(lint) && !defined(LOCORE) && defined(RCS_HDRS) */

#include <application.H>
class ezprintapp : public  application {
public:
	virtual ATKregistryEntry *ATKregistry();

  static boolean InitializeClass();
      
	ezprintapp();
    
	 boolean ParseArgs(int argc,char **argv)  ;
	 int Run()  ;
         void ReadInitFile();
};

#endif /* _ezprintapp_H_ */
