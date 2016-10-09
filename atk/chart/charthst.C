/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

/**  SPECIFICATION -- External Facility Suite  *********************************

TITLE	The Histogram Chart View-object

MODULE	charthst.c

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
  05/10/89	Utilize pre-computed data-points per chartobj (TCP)
  05/31/89	Add classID parameter in FinalizeObject (TCP)
  06/01/89	Accomodate negative values (TCP)
  06/02/89	Fix Printing of negative values (TCP)
  09/05/89	Upgrade to V1.0 (TCP)
  09/07/89	Set CurrentItem (TCP)

END-SPECIFICATION  ************************************************************/

#include <andrewos.h>
ATK_IMPL("charthst.H")
#include <math.h>
#include "graphic.H"
#include "view.H"
#include "aptv.H"
#include "chartobj.H"
#include "chart.H"
#include "charthst.H"
#include <ctype.h>
#define  Data			    ((class chart *)(self)->chartobj::data_object)

static boolean charthst_debug = 0;

#define debug charthst_debug

#define  Screen			1
#define  Paper			2

#define  Bounds			((self)->ChartBounds())
#define  Left			((self)->ChartLeft())
#define  Top			((self)->ChartTop())
#define  Width			((self)->ChartWidth())
#define  Height			((self)->ChartHeight())
#define  Right			((self)->ChartRight())
#define  Bottom			((self)->ChartBottom())
#define  BaseLine		((self)->ChartBaseLine())

#define  PIXELSPERUNIT		((self )->PixelsPerUnit( ))
#define  PIXELSPERINTERVAL	((self )->PixelsPerInterval( ))

#define  ITEMS			((self)->Items())
#define  ITEMBOUNDS(shadow)	((self)->ItemBounds((shadow)))
#define  ITEMX(shadow)		((shadow)->x)
#define  ITEMY(shadow)		((shadow)->y)
#define  ITEMLEFT(shadow)	((shadow)->bounds.left)
#define  ITEMTOP(shadow)	((shadow)->bounds.top)
#define  ITEMWIDTH(shadow)	((shadow)->bounds.width)
#define  ITEMHEIGHT(shadow)	((shadow)->bounds.height)
#define  ITEMCENTER(shadow)	(ITEMLEFT(shadow)+ITEMWIDTH(shadow)/2)
#define  ITEMMIDDLE(shadow)	(ITEMTOP(shadow)+ITEMHEIGHT(shadow)/2)
#define  NEXTITEM(shadow)	((self)->NextItem((shadow)))

#define  abs(x)			(((x) < 0) ? -(x): (x))


ATKdefineRegistry(charthst, chartobj, NULL);

charthst::charthst( )
      {
  IN(charthst_InitializeObject);
  (this)->SetShrinkIcon(  'e', "icon12", "HistogramChart", "andysans10b" );
  (this)->SetHelpFileName(  "/usr/andy/help/ez.help"/*=== ===*/ );
  OUT(charthst_InitializeObject);
  THROWONFAILURE(  TRUE);
  }

charthst::~charthst( )
      {}

const char *
charthst::Moniker( )
    {
  IN(charthst_Moniker);
  OUT(charthst_Moniker);
  return  "Histogram";
  }

void
charthst::SetDebug( boolean		        state )
      {
  IN(charthst_SetDebug);
  (this)->chartobj::SetDebug(  debug = state );
  OUT(charthst_SetDebug);
  }

class view *
charthst::HitChart( enum view_MouseAction       action, long			       x , long			       y , long			       clicks )
        {
  class charthst *self=this;
  class view		     *hit;
  static struct chart_item_shadow    *shadow;
  long			      delta;
  static long			      initial_y, prior_y, y_offset,
				      value, value_original;
  static char			      value_string[257], *name;

  IN(charthst_HitChart);
  hit = (class view *) this;
  if ( shadow  ||
	 (action == view_LeftDown  &&  (shadow = (this)->WhichItem(  x, y ))) )
    {
    (this)->SetCurrentItem( shadow->item );
    if ( y > Bottom ) y = Bottom;
    if ( y < Top )    y = Top;
    y += y_offset;
    switch ( action )
      {
      case  view_LeftDown:
	(this )->UseInvisibleCursor( );
        (this)->SetTransferMode(  graphic::INVERT );
	y_offset = ITEMY(shadow) - y;
	initial_y = prior_y = y = ITEMY(shadow);
	(this)->MoveTo(  Left, y );
	(this)->DrawLineTo(  Right, y );
	name = (char *) (Data)->ItemAttribute(  shadow->item, chart_itemname );
	value = value_original = (Data)->ItemAttribute(  shadow->item, chart_itemvalue );
	DEBUGdt(Initial-value,value);
        break;
      case  view_LeftMovement:
	(this)->MoveTo(  Left, prior_y );
	(this)->DrawLineTo(  Right, prior_y );
	if ( (abs(delta = prior_y - y)) > PIXELSPERINTERVAL )
	  value += (delta / PIXELSPERINTERVAL) * (Data )->ItemValueRangeInterval( );
	(this)->MoveTo(  Left, prior_y = y );
	(this)->DrawLineTo(  Right, y );
        break;
      case  view_LeftUp:
	(this)->MoveTo(  Left, prior_y );
	(this)->DrawLineTo(  Right, prior_y );
	if ( abs(delta = initial_y - y) > PIXELSPERINTERVAL )
	  {
	  value_original += (delta / PIXELSPERINTERVAL) *
				(Data )->ItemValueRangeInterval( );
	  DEBUGdt(Final-value,value);
	  (Data)->SetItemAttribute(  shadow->item, chart_ItemValue((value = value_original)) );
	  (Data )->SetModified( );
	  (Data)->NotifyObservers(  chart_ItemValueChanged );
	  }
	shadow = NULL;
	(this )->UseNormalCursor( );
	y_offset = 0;
        break;
      default:
        break;
      }
    sprintf( value_string, "%s:  Value = %ld", name, value );
    (this)->Announce(  value_string );
    }
  OUT(charthst_HitChart);
  return  hit;
  }

void
charthst::DrawChart( )
    {
  class charthst *self=this;
  long			      left, width, top, height, i = 0,
				      count = (Data )->ItemCount( ),
				      excess, fudge;
  struct chart_item_shadow  *shadow = ITEMS;

  IN(charthst_DrawChart);
  width = (Width / ((count) ? count : 1)) - 1;
  DEBUGdt(width,width);
  left = Left;
  if (( excess = (Width - ((width + 1) * count)) / 2 ))
    excess = count / excess;
  DEBUGdt(excess,excess);
  while ( shadow )
    {
    DEBUGdt(Value,(Data)->ItemAttribute(  shadow->item, chart_ItemValue(0) ));
    fudge = (excess) ? ((i % excess) ? 0 : 1) : 0;
    top = (ITEMY(shadow) < BaseLine) ? ITEMY(shadow) : BaseLine;
    height = abs(BaseLine - ITEMY(shadow));
    (this)->FillRectSize(  ITEMLEFT(shadow) = left, ITEMTOP(shadow) = top,
	ITEMWIDTH(shadow) = width+fudge, ITEMHEIGHT(shadow) = height, (class graphic *) graphic::BLACK );
    ITEMTOP(shadow) -= 5;
    ITEMHEIGHT(shadow) += 10;
    left += width + 1 + fudge;
    shadow = NEXTITEM(shadow);
    i++;
    }
  OUT(charthst_DrawChart);
  }

void
charthst::PrintChart( )
    {
  class charthst *self=this;
  long			      i, left, top, width, height,
				      count = (Data )->ItemCount( ),
				      excess, fudge;
  struct chart_item_shadow  *shadow = ITEMS;
  struct aptv_path	     *path;
  struct aptv_point	     *path_point;

  IN(charthst_PrintChart);
  path = (struct aptv_path *)
     malloc( sizeof(struct aptv_path) + 5 * sizeof(struct aptv_point) );
  width = (Width / ((count) ? count : 1)) - 1;
  left = Left;
  if ( ( excess = (Width - ((width + 1) * count)) / 2 ) )
    excess = count / excess;
  (this)->SetPrintLineWidth(  1 );
  path->count = 5;
  path_point = &path->points[0];
  for ( i = 0; i < count; i++ )
    {
    top = (ITEMY(shadow) < BaseLine) ? ITEMY(shadow) : BaseLine;
    height = abs(BaseLine - ITEMY(shadow));
    fudge = (excess) ? ((i % excess) ? 0 : 1) : 0;
    path_point = &path->points[0];
    path_point->x = left;		    path_point->y = top;	    path_point++;
    path_point->x = left + width + fudge;   path_point->y = top;	    path_point++;
    path_point->x = left + width + fudge;   path_point->y = top + height;   path_point++;
    path_point->x = left;		    path_point->y = top + height;   path_point++;
    path_point->x = left;		    path_point->y = top;
    (this)->SetPrintGrayLevel(  0.7 );
    (this)->PrintPathFilled(  path );
    (this)->SetPrintGrayLevel(  0.0 );
    (this)->PrintPath(  path );
    left += width + 1 + fudge;
    shadow = NEXTITEM(shadow);
    }
  (this)->SetPrintLineWidth(  1 );
  if ( path )  free( path );
  OUT(charthst_PrintChart);
  }
