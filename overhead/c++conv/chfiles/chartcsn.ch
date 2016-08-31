/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/config/COPYRITE'     *
\* ********************************************************************** */

/*
	$Disclaimer: 
 * Permission to use, copy, modify, and distribute this software and its 
 * documentation for any purpose and without fee is hereby granted, provided 
 * that the above copyright notice appear in all copies and that both that 
 * copyright notice and this permission notice appear in supporting 
 * documentation, and that the name of IBM not be used in advertising or 
 * publicity pertaining to distribution of the software without specific, 
 * written prior permission. 
 *                         
 * THE COPYRIGHT HOLDERS DISCLAIM ALL WARRANTIES WITH REGARD 
 * TO THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF 
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL ANY COPYRIGHT 
 * HOLDER BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL 
 * DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, 
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE 
 * OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION 
 * WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 * 
 *  $
*/

/**  SPECIFICATION -- External Facility Suite  *********************************

TITLE	The Cartesian Chart View-object

MODULE	chartcsn.ch

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

END-SPECIFICATION  ************************************************************/

#define  chartcsn_VERSION		    1

class chartcsn : chartobj
  {
overrides:

  DrawChart();
  PrintChart();
  HitChart( action, x, y, clicks )	    returns struct view *;
  SetDebug( boolean state );

methods:

classprocedures:

  InitializeObject( struct chartcsn *self ) returns boolean;
  FinalizeObject();

data:

  };


/*
    $Log: chartcsn.ch,v $
 * Revision 1.1  1993/10/13  15:52:18  rr2b
 * installing the old ch files for converting code
 *
*Revision 1.5  1993/05/04  01:11:11  susan
*RCS Tree Split
*
*Revision 1.4.1.1  1993/02/02  01:16:38  rr2b
*new R6tape branch
*
*Revision 1.4  1992/12/14  23:20:33  rr2b
*add $Logs back after disclaimerization took them out
*
Revision 1.2  1991/09/12  19:26:47  bobg
Update copyright notice

Revision 1.1  1989/04/28  22:44:35  tom
Initial revision

*/
