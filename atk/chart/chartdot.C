/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'     *
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

TITLE	The Dot Chart View-object

MODULE	chartdot.c

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
  05/04/89	Eliminate Dot connecting-lines (now in chartlin formatter) (TCP)
  05/08/89	Use ZipDot20 for Dot-faces (TCP)
  05/10/89	Depend upon pre-computed data-points in DrawChart (TCP)
  05/31/89	Add classID parameter in FinalizeObject (TCP)
  06/02/89	Use super-class Hit method (TCP)
  09/05/89	Upgrade to V1.0 (TCP)

END-SPECIFICATION  ************************************************************/

#include <andrewos.h>
ATK_IMPL("chartdot.H")
#include <math.h>
#include "graphic.H"
#include "observable.H"
#include "view.H"
#include "im.H"
#include "rect.h"
#include "aptv.H"
#include "chartobj.H"
#include "chart.H"
#include "chartdot.H"
#include <ctype.h>
#define  Data			    ((class chart *)(self)->chartobj::data_object)

static boolean chartdot_debug = 0;

#define debug chartdot_debug

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

static class fontdesc		*dot_font;
#define  DotFont		(dot_font)


ATKdefineRegistry(chartdot, chartobj, NULL);

chartdot::chartdot( )
      {
  IN(chartdot_InitializeObject);
  (this)->SetShrinkIcon(  'e', "icon12", "DotChart", "andysans10b" );
  (this)->SetHelpFileName(  "/usr/andy/help/ez.help"/*=== ===*/ );
  OUT(chartdot_InitializeObject);
  THROWONFAILURE(  TRUE);
  }

chartdot::~chartdot( )
      {}

void
chartdot::SetDebug( boolean			   state )
      {
  IN(chartdot_SetDebug);
  (this)->chartobj::SetDebug(  debug = state );
  OUT(chartdot_SetDebug);
  }

const char *
chartdot::Moniker( )
    {
  IN(chartdot_Moniker);
  OUT(chartdot_Moniker);
  return  "Dot";
  }

void
chartdot::DrawChart( )
    {
  class chartdot *self=this;
  struct chart_item_shadow  *shadow = Items;

  IN(chartdot_DrawChart);
  if ( DotFont == NULL )
    DotFont = (this)->BuildFont(  "zipdot20", NULL );
  (this)->SetFont(  DotFont );
  while ( shadow )
    {
    DEBUGdt(Value,(Data)->ItemAttribute(  shadow->item, chart_ItemValue(0) ));
    (this)->MoveTo(  ItemX(shadow), ItemY(shadow) );
    (this)->DrawString(  "C", 0 );
    shadow = NEXTITEM(shadow);
    }
  OUT(chartdot_DrawChart);
  }

void
chartdot::PrintChart( )
    {
  class chartdot *self=this;
  long			      i, left, top, width,
				      count = (Data )->ItemCount( );
  struct chart_item	     *chart_item = (Data )->ItemAnchor( );
  struct aptv_path	     *path;
  struct aptv_point	     *path_point;

  IN(chartdot_PrintChart);
  path = (struct aptv_path *)
     malloc( sizeof(struct aptv_path) + 5 * sizeof(struct aptv_point) );
  DEBUGdt(ItemCount,count);
  width = (Width / ((count) ? count : 1)) - 1;
  left = Left +( width / 2) + (Width - ((width + 1) * count)) / 2;
  (this)->SetPrintLineWidth(  1 );
  path->count = 5;
  path_point = &path->points[0];
  for ( i = 0; i < (Data)->ItemCount( )  &&  chart_item; i++ )
    {
    DEBUGdt(Value,(Data)->ItemAttribute(  chart_item, chart_ItemValue(0) ));
    top = Bottom - ((Data)->ItemAttribute(  chart_item, chart_itemvalue ) * PixelsPerUnit);
    path_point = &path->points[0];
    path_point->x = left;	    path_point->y = top;	    path_point++;
    path_point->x = left + 5;	    path_point->y = top;	    path_point++;
    path_point->x = left + 5;	    path_point->y = top + 5;	    path_point++;
    path_point->x = left;	    path_point->y = top + 5;	    path_point++;
    path_point->x = left;	    path_point->y = top;
    (this)->SetPrintGrayLevel(  0.7 );
    (this)->PrintPathFilled(  path );
    (this)->SetPrintGrayLevel(  0.0 );
    (this)->PrintPath(  path );
    left += width + 1;
    chart_item = (Data)->NextItem(  chart_item );
    }
  if ( path )  free( path );
  OUT(chartdot_PrintChart);
  }
