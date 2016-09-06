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

TITLE	The Pie Chart View-object

MODULE	chartpie.c

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
  09/05/89	Upgrade to V1.0 (TCP)
  09/07/89	Set CurrentItem (TCP)

END-SPECIFICATION  ************************************************************/

#include <andrewos.h>
ATK_IMPL("chartpie.H")
#include <math.h>
#include "graphic.H"
#include "view.H"
#include "aptv.H"
#include "chartobj.H"
#include "chart.H"
#include "chartpie.H"
#include <ctype.h>
#define  Data			    ((class chart *)(self)->chartobj::data_object)

int chartpie_debug = 0;

#define debug chartpie_debug

struct chartpie_item
  {
  struct chart_item		*item;
  long				 x, y;
  long				 x_label, y_label;
  double			 percent;
  };

struct chartpie_drawing
  {
  struct rectangle		 bounds;
  long				 x_center;
  long				 y_center;
  double			 sum;
  long				 count;
  struct chartpie_item		 item[1];
  };

#define  Drawing		((self)->drawing)
#define  DrawingSum		Drawing->sum
#define  DrawingCount		Drawing->count
#define  DrawingBounds		&Drawing->bounds
#define  DrawingLeft		Drawing->bounds.left
#define  DrawingTop		Drawing->bounds.top
#define  DrawingWidth		Drawing->bounds.width
#define  DrawingHeight		Drawing->bounds.height
#define  DrawingX		Drawing->x_center
#define  DrawingY		Drawing->y_center
#define  DrawingItem(i)		Drawing->item[i].item
#define  DrawingItemX(i)	Drawing->item[i].x
#define  DrawingItemY(i)	Drawing->item[i].y
#define  DrawingItemLabelX(i)	Drawing->item[i].x_label
#define  DrawingItemLabelY(i)	Drawing->item[i].y_label
#define  DrawingItemPercent(i)	Drawing->item[i].percent

#define  Screen			1
#define  Paper			2

#define  Bounds			((self)->ChartBounds())
#define  Left			((self)->ChartLeft())
#define  Top			((self)->ChartTop())
#define  Width			((self)->ChartWidth())
#define  Height			((self)->ChartHeight())
#define  Right			((self)->ChartRight())
#define  Bottom			((self)->ChartBottom())
#define  Middle			((self)->ChartMiddle())
#define  Center			((self)->ChartCenter())



ATKdefineRegistry(chartpie, chartobj, NULL);
static void Show_Pie_Chart( class chartpie	      *self, long			       medium );
static void Compute_Pie_Points( class chartpie	      *self );


chartpie::chartpie( )
      {
  class chartpie *self=this;
  IN(chartpie_InitializeObject);
  (this)->SetShrinkIcon(  'e', "icon12", "PieChart", "andysans10b" );
  (this)->SetHelpFileName(  "/usr/andy/help/ez.help"/*=== ===*/ );
  (this)->SetChartOptions(  chartobj_SuppressScales | chartobj_SuppressLabels );
  Drawing = 0;
  OUT(chartpie_InitializeObject);
  THROWONFAILURE(  TRUE);
  }

chartpie::~chartpie( )
      {
  class chartpie *self=this;
  IN(chartpie_FinalizeObject);
  if ( Drawing )  free( Drawing );
  OUT(chartpie_FinalizeObject);
  }

void
chartpie::SetDebug( boolean			   state )
      {
  IN(chartpie_SetDebug);
  (this)->chartobj::SetDebug(  debug = state );
  OUT(chartpie_SetDebug);
  }

class view *
chartpie::HitChart( enum view_MouseAction       action, long			       x , long			       y , long			       clicks )
        {
  class chartpie *self=this;
  static long			      prior_x, prior_y, candidate;
  double		      radius = DrawingX - DrawingLeft,
				      hyp, x_delta, y_delta, delta = 0,
				      start_angle = 0, end_angle, angle;
  class view		     *hit;

  IN(chartpie_HitChart);
  hit = (class view *) this;
  x_delta = x - DrawingX;  y_delta = y - DrawingY;
  hyp = hypot( x_delta, y_delta );
  angle = -(360.0 * (atan2( y_delta, x_delta ) / 6.28318));
  if ( angle <= 0 )    angle += 360.0; /*===messy===*/
  switch ( action )
    {
    case  view_LeftDown:
      (this)->SetFont(  (this)->IconFont() );
      (this)->SetTransferMode(  graphic_INVERT );
      for ( candidate = 0; candidate < DrawingCount; candidate++ )
	{
	end_angle = start_angle + (3.6 * DrawingItemPercent(candidate));
	if ( angle >= start_angle  &&  angle <= end_angle )
	  {
	  (this )->SetCurrentItem( DrawingItem(candidate) );
	  break;
	  }
	start_angle = end_angle;
	}
      (this)->MoveTo(  prior_x = DrawingItemX(candidate),
			     prior_y = DrawingItemY(candidate) );
      (this)->DrawString(  "E", 0 );
      break;
    case  view_LeftMovement:
      (this)->MoveTo(  prior_x, prior_y );
      (this)->DrawString(  "E", 0 );
      (this)->MoveTo(  prior_x = DrawingX + (radius * (x_delta / hyp)),
			     prior_y = DrawingY + (radius * (y_delta / hyp)) );
      (this)->DrawString(  "E", 0 );
      break;
    case  view_LeftUp:
      (this)->MoveTo(  prior_x, prior_y );
      (this)->DrawString(  "E", 0 );
      if ( delta )/*===*/
        (Data)->NotifyObservers(  chart_ItemValueChanged );
      break;
    default:
      break;
    }
  OUT(chartpie_HitChart);
  return  hit;
  }

void
chartpie::DrawChart( )
    {
  IN(chartpie_DrawChart);
  Show_Pie_Chart( this, Screen );
  OUT(chartpie_DrawChart);
  }

void
chartpie::PrintChart( )
    {
  IN(chartpie_PrintChart);
  Show_Pie_Chart( this, Paper );
  OUT(chartpie_PrintChart);
  }

static
void Show_Pie_Chart( class chartpie	      *self, long			       medium )
      {
  long			      i;
  long				      height;
  char				      percent_string[257];
  struct chart_item	     *item;
  double		      degrees, current_degree = 0.0;
  class fontdesc	     *font;

  IN(Show_Pie_Chart);
  Compute_Pie_Points( self );
  (self)->SetFont(  font = (self)->BuildFont(  "andysans10", &height ) );
  
  if ( medium == Screen ) {
      (self)->SetTransferMode(  graphic_BLACK );
      (self)->DrawOval(  DrawingBounds );
  }
  item = (Data )->ItemAnchor( );
  for ( i = 0; i < (Data)->ItemCount()  &&  item; i++ )
    {
    if ( medium == Screen )
      {
      (self)->MoveTo(  DrawingX, DrawingY );
      (self)->DrawLineTo(  DrawingItemX(i), DrawingItemY(i) );
      }
      else
      {
      degrees = 360.0 *
	((Data)->ItemAttribute(  item, chart_itemvalue ) / DrawingSum);
      (self)->PrintSlice(  DrawingX, DrawingY, DrawingX - DrawingLeft,
			     current_degree, current_degree + degrees,
			     i, (Data)->ItemCount(), 0 );
      current_degree += degrees;
      }
    if ( (Data)->ItemAttribute(  item, chart_itemname ) )
      { DEBUGst(Name,(Data)->ItemAttribute(  item, chart_ItemName(0) ));
      if ( (Data )->ItemFontName( ) )
	{
	font = (self)->BuildFont( 
		    (Data )->ItemFontName( ), &height );
	if ( medium == Screen )
          (self)->SetFont(  font );
	  else
	  (self)->SetPrintFont(  (Data )->ItemFontName( ) );
	}
      sprintf( percent_string, "%.2f%%", DrawingItemPercent(i) );
      if ( medium == Screen )
	{
	(self)->MoveTo(  DrawingItemLabelX(i), DrawingItemLabelY(i) );
	(self)->DrawString(  (char *) ((Data)->ItemAttribute(  item, chart_itemname )),
	    view_BETWEENLEFTANDRIGHT | view_BETWEENTOPANDBOTTOM );
	(self)->MoveTo(  DrawingItemLabelX(i), DrawingItemLabelY(i) + height );
	(self)->DrawString(  percent_string,
	    view_BETWEENLEFTANDRIGHT | view_BETWEENTOPANDBOTTOM );
	}
	else
	{
	(self)->SetPrintGrayLevel(  0.0 );
	(self)->PrintString(  DrawingItemLabelX(i), DrawingItemLabelY(i),
				(char *) ((Data)->ItemAttribute(  item, chart_itemname )), 0 );
	(self)->PrintString(  DrawingItemLabelX(i), DrawingItemLabelY(i) + height,
				percent_string, 0 );
	}
      }
    item = (Data)->NextItem(  item );
    }
  OUT(Show_Pie_Chart);
  }

static
void Compute_Pie_Points( class chartpie	      *self )
    {
  long			      i, count = 0;
  struct chart_item	     *item;
  double		      sum = 0, radian,
				      degrees, current_degree = 90.0, radius;

  IN(Compute_Pie_Points);
  item = (Data )->ItemAnchor( );
  for ( i = 0; i < (Data )->ItemCount( )  &&  item; i++ )
    {
    count++;
    sum += (Data)->ItemAttribute(  item, chart_itemvalue );
    item = (Data)->NextItem(  item );
    }
  if ( Drawing )    free( Drawing );
  Drawing = (struct chartpie_drawing *)
	malloc( sizeof(struct chartpie_drawing) + count * sizeof(struct chartpie_item) );
  DrawingCount = count;
  DEBUGdt(Count,DrawingCount);
  DrawingSum = sum;
  DEBUGgt(Sum,DrawingSum);
  DrawingTop = Top;
  if ( Width > Height )
    DrawingWidth = DrawingHeight = Height - 1;
    else
    {
    DrawingWidth = DrawingHeight = Width - 1;
    DrawingTop = Middle - DrawingHeight / 2;
    }
  DrawingLeft = Center - DrawingWidth / 2;
  DEBUGdt(Left,DrawingLeft);   DEBUGdt(Top,DrawingTop);
  DEBUGdt(Width,DrawingWidth); DEBUGdt(Height,DrawingHeight);
  radius = DrawingWidth / 2;
  DrawingX = Center;
  DrawingY = DrawingTop + radius;
  item = (Data )->ItemAnchor( );
  for ( i = 0; i < (Data)->ItemCount()  &&  item; i++ )
    {
    DrawingItem(i) = item;
    DEBUGdt(Value,(Data)->ItemAttribute(  item, chart_ItemValue(0) ));
    DrawingItemPercent(i) =
	((Data)->ItemAttribute(  item, chart_itemvalue ) / DrawingSum) * 100;
    degrees = 3.6 * DrawingItemPercent(i);
    radian = 6.28318 * ((current_degree + degrees)/360.0);
    DrawingItemX(i) = DrawingX + radius * sin( radian );
    DrawingItemY(i) = DrawingY + radius * cos( radian );
    radian = 6.28318 * ((current_degree + (degrees/2.0))/360.0);
    DrawingItemLabelX(i) = DrawingX + (radius * 0.75) * sin( radian );
    DrawingItemLabelY(i) = DrawingY + (radius * 0.75) * cos( radian );
    current_degree += degrees;
    item = (Data)->NextItem(  item );
    }
  OUT(Compute_Pie_Points);
  }
