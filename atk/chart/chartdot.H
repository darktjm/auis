ATK_INTER
#ifndef _chartdot_H_
#define _chartdot_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'     *
\* ********************************************************************** */

/*
	$Disclaimer: 
// Permission to use, copy, modify, and distribute this software and its 
// documentation for any purpose and without fee is hereby granted, provided 
// that the above copyright notice appear in all copies and that both that 
// copyright notice and this permission notice appear in supporting 
// documentation, and that the name of IBM not be used in advertising or 
// publicity pertaining to distribution of the software without specific, 
// written prior permission. 
//                         
// THE COPYRIGHT HOLDERS DISCLAIM ALL WARRANTIES WITH REGARD 
// TO THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF 
// MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL ANY COPYRIGHT 
// HOLDER BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL 
// DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, 
// DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE 
// OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION 
// WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
// 
//  $
*/

/***  SPECIFICATION -- External Facility Suite  *********************************

TITLE	The Dot Chart View-object

MODULE	chartdot.ch

VERSION	0.0

NOTICE	IBM Internal Use Only

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
  06/02/89	Use super-class Hit method (TCP)

END-SPECIFICATION  ************************************************************/

#define  chartdot_VERSION		    1

#include <chartobj.H>
class chartdot : public  chartobj
  {
public:
	virtual ATKregistryEntry *ATKregistry();



   void DrawChart();
   void PrintChart();
   const char * Moniker()				     ;
   void SetDebug( boolean state );





  
	chartdot();
  
	virtual ~chartdot();



  };


/*
    $Log: chartdot.H,v $
// Revision 1.4  1994/11/30  20:42:06  rr2b
// Start of Imakefile cleanup and pragma implementation/interface hack for g++
//
// Revision 1.3  1993/06/10  20:23:17  rr2b
// Great atk_inited removal.
//
 * Revision 1.2  1993/05/18  18:57:14  gk5g
 * C++ conversion
 *
*Revision 1.5  1992/12/14  23:20:33  rr2b
*add $Logs back after disclaimerization took them out
*
Revision 1.3  1991/09/12  19:26:51  bobg
Update copyright notice

Revision 1.2  1989/06/02  16:53:12  tom
Provide default HitHandler.

Revision 1.1  89/04/28  22:44:37  tom
Initial revision

*/

#endif /* _chartdot_H_ */
