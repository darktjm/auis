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

#include <andrewos.h>

#ifndef NORCSID
#define NORCSID
static UNUSED const char rcsid[]="$Header: /afs/cs.cmu.edu/project/atk-src-C++/atk/chart/RCS/chartlin.C,v 1.6 1995/11/07 20:17:10 robr Stab74 $";
#endif

/**  SPECIFICATION -- External Facility Suite  *********************************

TITLE	The Line Chart View-object

MODULE	chartlin.c

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
  05/05/89	Improve positioning of line segments (TCP)
  05/08/89	Further improve positioning (TCP)
  05/10/89	Depend upon pre-computed data-points in DrawChart (TCP)
  05/31/89	Add classID parameter in FinalizeObject (TCP)
  06/02/89	Use super-class Hit method (TCP)
  09/05/89	Upgrade to V1.0 (TCP)

END-SPECIFICATION  ************************************************************/

ATK_IMPL("chartlin.H")
#include <math.h>
#include "graphic.H"
#include "observable.H"
#include "view.H"
#include "im.H"
#include "rect.h"
#include "aptv.H"
#include "chartobj.H"
#include "chart.H"
#include "chartlin.H"
#include <ctype.h>
#define  Data			    ((class chart *)(self)->chartobj::data_object)

boolean chartlin_debug = 0;
#define debug chartlin_debug


#define  Screen			1
#define  Paper			2

#define  Bounds			((self)->ChartBounds())
#define  Left			((self)->ChartLeft())
#define  Top			((self)->ChartTop())
#define  Width			((self)->ChartWidth())
#define  Height			((self)->ChartHeight())
#define  Right			((self)->ChartRight())
#define  Bottom			((self)->ChartBottom())

#define  Items			((self)->Items())
#define  ItemBounds(s)		((self)->ItemBounds((s)))
#define  ItemLeft(s)		((self)->ItemLeft((s)))
#define  ItemTop(s)		((self)->ItemTop((s)))
#define  ItemWidth(s)		((self)->ItemWidth((s)))
#define  ItemHeight(s)		((self)->ItemHeight((s)))
#define  ItemCenter(s)		((self)->ItemCenter((s)))
#define  ItemMiddle(s)		((self)->ItemMiddle((s)))
#define  ItemX(s)		((self)->ItemX((s)))
#define  ItemY(s)		((self)->ItemY((s)))
#define  NEXTITEM(s)		((self)->NextItem((s)))

#define  PixelsPerUnit		((self )->PixelsPerUnit( ))
#define  PixelsPerInterval	((self )->PixelsPerInterval( ))


ATKdefineRegistry(chartlin, chartobj, NULL);
#ifndef NORCSID
#endif


chartlin::chartlin( )
      {
  IN(chartlin_InitializeObject);
  (this)->SetShrinkIcon(  'e', "icon12", "LineChart", "andysans10b" );
  (this)->SetHelpFileName(  "/usr/andy/help/ez.help"/*=== ===*/ );
  OUT(chartlin_InitializeObject);
  THROWONFAILURE(  TRUE);
  }

chartlin::~chartlin( )
      {}

void
chartlin::SetDebug( boolean			   state )
      {
  IN(chartlin_SetDebug);
  (this)->chartobj::SetDebug(  debug = state );
  OUT(chartlin_SetDebug);
  }

char *
chartlin::Moniker( )
    {
  IN(chartlin_Moniker);
  OUT(chartlin_Moniker);
  return  "Line";
  }

void
chartlin::DrawChart( )
    {
  class chartlin *self=this;
  register struct chart_item_shadow  *shadow = Items;
  register short		      prior_x, prior_y;

  IN(chartlin_DrawChart);
  prior_x = 0;
  prior_y = 0;
  while ( shadow )
    {
    DEBUGdt(Value,(Data)->ItemAttribute(  shadow->item, chart_ItemValue(0) ));
    if ( prior_x )
      {
      (this)->MoveTo(  prior_x, prior_y );
      (this)->DrawLineTo(  ItemX(shadow), ItemY(shadow) );
      }
    prior_x = ItemX(shadow);
    prior_y = ItemY(shadow);
    shadow = NEXTITEM(shadow);
    }
  OUT(chartlin_DrawChart);
  }

void
chartlin::PrintChart( )
    {
  class chartlin *self=this;
  register long			      i, left, top, width,
				      count = (Data)->ItemCount( ),
				      excess, fudge;
  register struct chart_item	     *chart_item = (Data )->ItemAnchor( );
  struct point			      prior_point;

  IN(chartlin_PrintChart);
  DEBUGdt(ItemCount,count);
  width = (Width / ((count) ? count : 1));
  left = Left + width/2;
  if ( excess = (Width - (width * count)) / 2 )
    excess = count / excess;
  (this)->SetPrintLineWidth(  1 );
  (this)->SetPrintGrayLevel(  0.0 );
  prior_point.x = 0;
  for ( i = 0; i < count  &&  chart_item; i++ )
    {
    DEBUGdt(Value,(Data)->ItemAttribute(  chart_item, chart_itemvalue ));
    top = Bottom - ((Data)->ItemAttribute(  chart_item, chart_itemvalue ) * PixelsPerUnit);
    if ( prior_point.x )
      (this)->PrintLine(  prior_point.x, prior_point.y, left, top );
    fudge = (excess) ? ((i % excess) ? 0 : 1) : 0;
    prior_point.x = left + fudge;
    prior_point.y = top;
    left += width + fudge;
    chart_item = (Data)->NextItem(  chart_item );
    }
  OUT(chartlin_PrintChart);
  }
