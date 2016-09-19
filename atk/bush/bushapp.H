ATK_INTER
#ifndef _bushapp_H_
#define _bushapp_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

/***  SPECIFICATION -- External Facility Suite  *********************************

TITLE	Bush Application

MODULE	bushapp.ch

VERSION	0.0

AUTHOR	TC Peters & GW Keim
	Information Technology Center, Carnegie-Mellon University 

DESCRIPTION
	This is the Application layer for the Bush Inset..

    NB: The comment-symbol "===" indicates areas which are:
	    1 - Questionable
	    OR
	    2 - Arbitrary
	    OR
	    3 - Temporary Hacks
    Such curiosities need be resolved prior to Project Completion...

HISTORY
  08/21/85	Created (TCP)
  01/15/89	Convert to ATK (GW Keim)

END-SPECIFICATION  ******************************************************/

#include <application.H>
class bushapp : public  application  {
public:
	virtual ATKregistryEntry *ATKregistry();

  
    
	bushapp();
    
	virtual ~bushapp();
  
     boolean ParseArgs( int argc, const char **argv )  ;
     boolean Start()  ;
  
    class im	    *imp;
    class frame    *framep;
    class bush	    *bushp;
    class bushv    *bushview;
    int		     argc;
    const char	   **argv;
    int		     debug;
};

#endif /* _bushapp_H_ */
