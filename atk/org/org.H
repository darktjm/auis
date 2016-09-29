#ifndef _org_H_
#define _org_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

/** \addtogroup liborg liborg.so
 *  A dynamically loadable ATK class file.  Contains org, orgv, orgapp.
 * @{ */
/***  SPECIFICATION -- External Facility Suite  *********************************

TITLE	The Org Data-Class

MODULE	org.ch

VERSION	1.0

AUTHOR	TC Peters
	Information Technology Center, Carnegie-Mellon University 

DESCRIPTION
	This is the suite of Methods that support the Org Data-Class.

    NB: The comment-symbol "===" indicates areas which are:
	    1 - Questionable
	    OR
	    2 - Arbitrary
	    OR
	    3 - Temporary Hacks
    Such curiosities need be resolved prior to Project Completion...


HISTORY
  01/19/89	Created (TCP)
  08/23/89	Remove Create method (TCP)

END-SPECIFICATION  ************************************************************/




#define  org_VERSION	      1

#include <apt.H>
class org : public  apt
  {
public:
	virtual ATKregistryEntry *ATKregistry();



   long Read( FILE *file, long id )				 ;
   long Write( FILE *file, long id, int level )		 ;
   const char * ViewName()						 ;



  virtual void SetDebug( boolean state );
  virtual const char * NodeName( struct tree_node *node ) ;



  
	org();
  
	virtual ~org();



  class tree		  	 *tree_data_object;
  };
/** @} */
#endif /* _org_H_ */
