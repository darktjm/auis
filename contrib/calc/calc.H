#ifndef _calc_H_
#define _calc_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */
/** \addtogroup libcalc Object calc
 *  A dynamically loadable ATK class file.  Contains calc, calcv.
 * @{ */
/*
    $Log: calc.H,v $
// Revision 1.2  1993/05/29  16:30:44  rr2b
// Fixed level argument to Write method to be an int.
// Made the return value from the inline Value method be a reference
// to make it more macro-like.
// Removed and fixed bogus rcs spooge.
// Added typedef for the hit function pointer.
// Fixed the argument types for SetDataObject and ObservedChanged to match the superclass.
//
 * Revision 1.1  1993/05/29  15:33:56  rr2b
 * Initial revision
 *
*Revision 1.2  1992/12/14  21:43:11  rr2b
*fixed disclaimerization
*.
*
# Revision 1.1  1992/10/05  20:07:19  susan
# Initial revision
#
Revision 1.3  1991/09/12  19:25:30  bobg
Update copyright notice

Revision 1.2  1989/08/04  17:09:31  tom
Accept keyboard inputs;
Suppress Shrink & Help Icons.

Revision 1.1  89/05/10  20:57:54  tom
Initial revision

*/

/***  SPECIFICATION -- External Facility Suite  *********************************

TITLE	The Calc Data-object

MODULE	calc.ch

VERSION	1.0

AUTHOR	TC Peters
	Information Technology Center, Carnegie-Mellon University 

DESCRIPTION
	This is the suite of Methods that suport the Calc Data-object.

HISTORY
  02/23/88	Created (TCP)

END-SPECIFICATION  ************************************************************/


#define  calc_VERSION    1

#define  calc_value_changed				1


#include <apt.H>
class calc : public  apt
  {
public:
	virtual ATKregistryEntry *ATKregistry();




   long Read( FILE *file, long id )				 ;
   long Write( FILE *file, long id, int level )		 ;
   const char * ViewName()						 ;



  virtual void SetValue( double value );



  inline double &Value()			      { return ((this)->value); }



  
	calc();
  
	virtual ~calc();



  long				      memory;
  double			      value;
  long				      id;
  };
/** @} */
#endif /* _calc_H_ */
