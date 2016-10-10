#ifndef _charthst_H_
#define _charthst_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

/** \addtogroup libchart
 * @{ */
/***  SPECIFICATION -- External Facility Suite  *********************************

TITLE	The Histogram Chart View-object

MODULE	charthst.ch

VERSION	0.0

AUTHOR	TC Peters
	Information Technology Center, Carnegie-Mellon University 

DESCRIPTION
	This is the suite of Methods that support the Chart View-object.

    NB: The comment-symbol "===" indicates areas which are:
	    1 - Questionable
	    OR
	    2 - Arbitrary
	    OR
	    3 - Temporary Hacks
    Such curiosities need be resolved prior to Project Completion...


HISTORY
  03/28/89	Created (TCP)

END-SPECIFICATION  ************************************************************/

#define  charthst_VERSION		    1

#include <chartobj.H>
class charthst : public  chartobj
  {
public:
	virtual ATKregistryEntry *ATKregistry();



   void DrawChart();
   void PrintChart();
   class view * HitChart( enum view::MouseAction action, long x, long y, long clicks )	     ;
   const char * Moniker()				     ;
   void SetDebug( boolean state );





  
	charthst();
  
	virtual ~charthst();



  };


/*
    $Log: charthst.H,v $
// Revision 1.4  1994/11/30  20:42:06  rr2b
// Start of Imakefile cleanup and pragma implementation/interface hack for g++
//
// Revision 1.3  1993/06/10  20:23:23  rr2b
// Great atk_inited removal.
//
 * Revision 1.2  1993/05/18  18:57:14  gk5g
 * C++ conversion
 *
*Revision 1.4  1992/12/14  23:20:33  rr2b
*add $Logs back after disclaimerization took them out
*
Revision 1.2  1991/09/12  19:26:54  bobg
Update copyright notice

Revision 1.1  1989/04/28  22:44:39  tom
Initial revision

*/
/** @} */
#endif /* _charthst_H_ */
