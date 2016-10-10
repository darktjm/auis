/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

/**  SPECIFICATION -- External Facility Suite  *********************************

TITLE	The Cartesian Chart View-object

MODULE	chartcsn.c

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
  05/31/89	Add classID parameter in FinalizeObject (TCP)

END-SPECIFICATION  ************************************************************/

#include <andrewos.h>
ATK_IMPL("chartcsn.H")
#include <stdio.h>
#include <math.h>
#include "graphic.H"
#include "observable.H"
#include "view.H"
#include "im.H"
#include "rect.h"
#include "aptv.H"
#include "chartobj.H"
#include "chart.H"
#include "chartcsn.H"
#include <ctype.h>

static boolean chartscn_debug = 0;
#define debug chartscn_debug

#define  Screen			1
#define  Paper			2

#define  Bounds			((self)->BodyBounds())
#define  Left			((self)->BodyLeft())
#define  Top			((self)->BodyTop())
#define  Width			((self)->BodyWidth())
#define  Height			((self)->BodyHeight())
#define  Right			((self)->BodyRight())
#define  Bottom			((self)->BodyBottom())

#define  PixelsPerUnit		((self )->PixelsPerUnit( ))
#define  PixelsPerInterval	((self )->PixelsPerInterval( ))

#define  Items			((self)->Items())
#define  ItemBounds(shadow)	((self)->ItemBounds((shadow)))
#define  ItemLeft(shadow)	((self)->ItemLeft((shadow)))
#define  ItemTop(shadow)	((self)->ItemTop((shadow)))
#define  ItemWidth(shadow)	((self)->ItemWidth((shadow)))
#define  ItemHeight(shadow)	((self)->ItemHeight((shadow)))
#define  ItemCenter(shadow)	((self)->ItemCenter((shadow)))
#define  ItemMiddle(shadow)	((self)->ItemMiddle((shadow)))
#define  NextItem(shadow)	((self)->NextItem((shadow)))


ATKdefineRegistry(chartcsn, chartobj, NULL);

chartcsn::chartcsn( )
      {
  IN(chartcsn_InitializeObject);
  (this)->SetShrinkIcon(  'e', "icon12", "CartesianChart", "andysans10b" );
  (this)->SetHelpFileName(  "/usr/andy/help/ez.help"/*=== ===*/ );
  OUT(chartcsn_InitializeObject);
  THROWONFAILURE(  TRUE);
  }

chartcsn::~chartcsn( )
      {}

void
chartcsn::SetDebug( boolean			   state )
      {
  IN(chartcsn_SetDebug);
  (this)->chartobj::SetDebug(  debug = state );
  OUT(chartcsn_SetDebug);
  }

class view *
chartcsn::HitChart( enum view::MouseAction       action, long			       x , long			       y , long			       clicks )
        {

  IN(chartcsn_HitChart);
/*===*/
  OUT(chartcsn_HitChart);
  return  (class view *) this;
  }

void
chartcsn::DrawChart( )
    {
  IN(chartcsn_DrawChart);
/*===*/
  OUT(chartcsn_DrawChart);
  }

void
chartcsn::PrintChart( )
    {
  IN(chartcsn_PrintChart);
/*===*/
  OUT(chartcsn_PrintChart);
  }


/*
    $Log: chartcsn.C,v $
 * Revision 1.4  1995/11/07  20:17:10  robr
 * OS/2 port
 *
 * Revision 1.3  1994/11/30  20:42:06  rr2b
 * Start of Imakefile cleanup and pragma implementation/interface hack for g++
 *
 * Revision 1.2  1993/05/18  18:57:14  gk5g
 * C++ conversion
 *
 * Revision 1.7  1992/12/15  21:29:20  rr2b
 * more disclaimerization fixing
 *
 * Revision 1.6  1992/12/14  23:20:33  rr2b
 * add $Logs back after disclaimerization took them out
 *
 * Revision 1.4  1992/08/25  19:40:53  rr2b
 * hacked to avoid global name conflicts
 * .
 *
 * Revision 1.3  1991/09/12  16:04:28  bobg
 * Update copyright notice and rcsid
 *
 * Revision 1.2  1989/05/31  17:24:27  tom
 * y
 * Added classID as parameter to FinalizeObject.
 *
 * Revision 1.1  89/04/28  22:44:56  tom
 * Initial revision
 * 
*/
