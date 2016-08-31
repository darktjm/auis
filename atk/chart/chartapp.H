ATK_INTER
#ifndef _chartapp_H_
#define _chartapp_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/config/COPYRITE'     *
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

#include <application.H>

/*
    $Log: chartapp.H,v $
// Revision 1.5  1994/11/30  20:42:06  rr2b
// Start of Imakefile cleanup and pragma implementation/interface hack for g++
//
// Revision 1.4  1994/08/12  17:37:58  rr2b
// new fiasco
//
// Revision 1.3  1993/06/10  20:23:07  rr2b
// Great atk_inited removal.
//
 * Revision 1.2  1993/05/18  18:57:14  gk5g
 * C++ conversion
 *
*Revision 1.3  1992/12/14  20:37:37  rr2b
*disclaimerization
*
Revision 1.2  1991/09/12  19:26:42  bobg
Update copyright notice

Revision 1.1  1989/04/28  22:44:33  tom
Initial revision

*/

/**  SPECIFICATION -- External Facility Suite  *********************************

TITLE	The Chart Application-object

MODULE	chartapp.ch

VERSION	0.0

AUTHOR	TC Peters
	Information Technology Center, Carnegie-Mellon University 

DESCRIPTION
	This is the suite of Methods that suport the Chart Application-object.

    NB: The comment-symbol "===" indicates areas which are:
	    1 - Questionable
	    OR
	    2 - Arbitrary
	    OR
	    3 - Temporary Hacks
    Such curiosities need be resolved prior to Project Completion...


HISTORY
  02/23/88	Created (TCP)

END-SPECIFICATION  ************************************************************/

class chartapp: public  application 
  {
public:
	virtual ATKregistryEntry *ATKregistry();

  
    chartapp();
    
	virtual ~chartapp();

  
    boolean Start()				     ;
    boolean ParseArgs( int argc, const char **argv )	     ;

  
    char				    source[512];
    class chart			   *chart_data_object;
    class chartv			   *chart_view_object;
    class im				   *imp;
    class frame			   *framep;
  };

#endif /* _chartapp_H_ */
