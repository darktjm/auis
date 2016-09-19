/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

/**  SPECIFICATION -- External Facility Suite  *********************************

TITLE	The Chart View-object Abstract-class

MODULE	chartobj.c

VERSION	0.0

AUTHOR	TC Peters
	Information Technology Center, Carnegie-Mellon University 

DESCRIPTION
	This is the suite of Methods that support Chart parochial View-objects.

    NB: The comment-symbol "===" indicates areas which are:
	    1 - Questionable
	    OR
	    2 - Arbitrary
	    OR
	    3 - Temporary Hacks
    Such curiosities need be resolved prior to Project Completion...


HISTORY
  03/23/89	Created (TCP)
  05/08/89	Add UnlinklNotification (RetractViewCursors) (TCP)
  05/10/89	Precompute data-points into Shadows (TCP)
  06/01/89	Improve Scale-tick numbers (TCP)
		Handle negative values
		Provide generic Hit method
  06/02/89	Improve Scale-tick printing (TCP)
  06/08/89	Honor suppression of Labels & Scales in printing (TCP)
  06/26/89	Correct BaseLine setting (TCP)
  09/05/89	Upgrade to V1.0 (TCP)

END-SPECIFICATION  ************************************************************/

#include <andrewos.h>
ATK_IMPL("chartobj.H")
#include <math.h>
#include "chartobj.H"
#include "graphic.H"
#include "observable.H"
#include "view.H"
#include "fontdesc.H"
#include "im.H"
#include "rect.h"
#include "aptv.H"
#include "chart.H"
#include "chartv.H"
#include <ctype.h>

static boolean chartobj_debug = 0;

#define debug chartobj_debug

#define  Chart			(self->data_object)

#define  Shadows		(self->item_shadows)
#define  ShadowX(s)		(s->x)
#define  ShadowY(s)		(s->y)
#define  ShadowItem(s)		(s->item)
#define  ShadowDatum(s)		(s->datum)
#define  ShadowBounds(s)	(&s->bounds)
#define  ShadowLeft(s)		(s->bounds.left)
#define  ShadowTop(s)		(s->bounds.top)
#define  ShadowWidth(s)		(s->bounds.width)
#define  ShadowHeight(s)	(s->bounds.height)
#define  ShadowCenter(s)	(ShadowLeft(s)+ShadowWidth(s)/2)
#define  ShadowMiddle(s)	(ShadowTop(s)+ShadowHeight(s)/2)
#define  NEXTITEM(s)		(s->next)
#define  CURRENTITEM		(self->current_item)

#define  BOUNDS			((self)->BodyBounds())
#define  Left			((self)->BodyLeft())
#define  Top			((self)->BodyTop())
#define  Width			((self)->BodyWidth())
#define  Height			((self)->BodyHeight())
#define  Right			((self)->BodyRight())
#define  Center			((self)->BodyCenter())
#define  Middle			((self)->BodyMiddle())
#define  Bottom			((self)->BodyBottom())

#define  ChartBounds		(&self->chart_bounds)
#define  ChartLeft		(self->chart_bounds.left)
#define  ChartTop		(self->chart_bounds.top)
#define  ChartWidth		(self->chart_bounds.width)
#define  ChartHeight		(self->chart_bounds.height)
#define  ChartRight		(self->chart_bounds.left+self->chart_bounds.width-1)
#define  ChartBottom		(self->chart_bounds.top+self->chart_bounds.height-1)
#define  ChartBaseLine		(self->baseline)

#define  PIXELSPERUNIT		((self)->pixels_per_unit_value)
#define  PIXELSPERINTERVAL	((self)->pixels_per_value_interval)

#define  LeftScaleLeft		(self->left_scale.left)
#define  LeftScaleTop		(self->left_scale.top)
#define  LeftScaleWidth		(self->left_scale.width)
#define  LeftScaleHeight	(self->left_scale.height)
#define  LeftScaleRight		(LeftScaleLeft+LeftScaleWidth)
#define  LeftScaleBottom	(LeftScaleTop+LeftScaleHeight-1)
#define  LeftScaleBarX		(LeftScaleRight-10)
#define  LeftScaleBarY		(LeftScaleTop-10)

#define  RightScaleLeft		(self->right_scale.left)
#define  RightScaleTop		(self->right_scale.top)
#define  RightScaleWidth	(self->right_scale.width)
#define  RightScaleHeight	(self->right_scale.height)
#define  RightScaleRight	(RightScaleLeft+RightScaleWidth)
#define  RightScaleBottom	(RightScaleTop+RightScaleHeight-1)
#define  RightScaleBarX		(RightScaleRight-10)
#define  RightScaleBarY		(RightScaleTop-10)

#define  TopScaleLeft		(self->top_scale.left)
#define  TopScaleTop		(self->top_scale.top)
#define  TopScaleWidth		(self->top_scale.width)
#define  TopScaleHeight		(self->top_scale.height)
#define  TopScaleRight		(TopScaleLeft+TopScaleWidth)
#define  TopScaleBottom		(TopScaleTop+TopScaleHeight-1)
#define  TopScaleBarX		(TopScaleRight-10)
#define  TopScaleBarY		(TopScaleTop-10)

#define  BottomScaleLeft	(self->bottom_scale.left)
#define  BottomScaleTop		(self->bottom_scale.top)
#define  BottomScaleWidth	(self->bottom_scale.width)
#define  BottomScaleHeight	(self->bottom_scale.height)
#define  BottomScaleRight	(BottomScaleLeft+BottomScaleWidth)
#define  BottomScaleBottom	(BottomScaleTop+BottomScaleHeight-1)
#define  BottomScaleBarX	(BottomScaleRight-10)
#define  BottomScaleBarY	(BottomScaleTop-10)

#define  ScalePositions		(self->scale_positions)
#define  ScaleFont		(self->scale_font)
#define  ScaleFontName		(self->scale_font_name)
#define  ScaleTick		(self->scale_tick)

#define  LeftLabelsLeft		(self->left_labels.left)
#define  LeftLabelsTop		(self->left_labels.top)
#define  LeftLabelsWidth	(self->left_labels.width)
#define  LeftLabelsHeight	(self->left_labels.height)
#define  LeftLabelsRight	(LeftLabelsLeft+LeftLabelsWidth)
#define  LeftLabelsBottom	(LeftLabelsTop+LeftLabelsHeight-1)
#define  LeftLabelsCenter	(LeftLabelsLeft+LeftLabelsWidth/2)
#define  LeftLabelsMiddle	(LeftLabelsTop+LeftLabelsHeight/2)

#define  RightLabelsLeft	(self->right_labels.left)
#define  RightLabelsTop		(self->right_labels.top)
#define  RightLabelsWidth	(self->right_labels.width)
#define  RightLabelsHeight	(self->right_labels.height)
#define  RightLabelsRight	(RightLabelsLeft+RightLabelsWidth)
#define  RightLabelsBottom	(RightLabelsTop+RightLabelsHeight-1)
#define  RightLabelsCenter	(RightLabelsLeft+RightLabelsWidth/2)
#define  RightLabelsMiddle	(RightLabelsTop+RightLabelsHeight/2)

#define  TopLabelsLeft		(self->top_labels.left)
#define  TopLabelsTop		(self->top_labels.top)
#define  TopLabelsWidth		(self->top_labels.width)
#define  TopLabelsHeight	(self->top_labels.height)
#define  TopLabelsRight		(TopLabelsLeft+TopLabelsWidth)
#define  TopLabelsBottom	(TopLabelsTop+TopLabelsHeight-1)
#define  TopLabelsCenter	(TopLabelsLeft+TopLabelsWidth/2)
#define  TopLabelsMiddle	(TopLabelsTop+TopLabelsHeight/2)

#define  BottomLabelsLeft	(self->bottom_labels.left)
#define  BottomLabelsTop	(self->bottom_labels.top)
#define  BottomLabelsWidth	(self->bottom_labels.width)
#define  BottomLabelsHeight	(self->bottom_labels.height)
#define  BottomLabelsRight	(BottomLabelsLeft+BottomLabelsWidth)
#define  BottomLabelsBottom	(BottomLabelsTop+BottomLabelsHeight-1)
#define  BottomLabelsCenter	(BottomLabelsLeft+BottomLabelsWidth/2)
#define  BottomLabelsMiddle	(BottomLabelsTop+BottomLabelsHeight/2)

#define  LabelFont		(self->label_font)
#define  LabelFontName		(self->label_font_name)
#define  LabelPositions		(self->label_positions)
#define  GraphicFont		(self->graphic_font)
#define  DottedGraphic		(self->dotted_graphic)
#define  DottedIcon		('5')
#define  DashedGraphic		(self->dashed_graphic)
#define  DashedIcon		('5')

#define  RightTop		(view_ATRIGHT | view_ATTOP)
#define  RightBottom		(view_ATRIGHT | view_ATBOTTOM)
#define  RightMiddle		(view_ATRIGHT | view_BETWEENTOPANDBOTTOM)
#define  Balanced	        (view_BETWEENLEFTANDRIGHT | view_BETWEENTOPANDBOTTOM)

#define  ScalesSuppressed	(self->suppress_scales)
#define  LabelsSuppressed	(self->suppress_labels)
#define  GridsSuppressed	(self->suppress_grids)

#define  abs(x)			(((x)<0) ? -(x) : (x))


ATKdefineRegistry(chartobj, aptv, NULL);
static void Generate_Shadows( class chartobj	    *self );
static void Set_Shadows( class chartobj	      *self );
static void Free_Shadows( class chartobj	      *self );
void Printer( class chartobj	      *self, FILE			      *file, char			      *processor, char			      *format, boolean		       level, aptv_printfptr printer );
static void Draw_Labels( class chartobj	  *self );
static void Draw_Horizontal_Labels( class chartobj	  *self, long left, long top, long width, long height );
static void Draw_Vertical_Labels( class chartobj	  *self, long left, long top, long width, long height );
static void Draw_Scales( class chartobj	  *self );
static void Draw_Left_Scale( class chartobj	  *self );
static void Draw_Right_Scale( class chartobj	  *self );
static void Prepare_Vertical_Scale( class chartobj	  *self );
static void Draw_Top_Scale( class chartobj	  *self );
static void Draw_Bottom_Scale( class chartobj	  *self );
static void Prepare_Horizontal_Scale( class chartobj	  *self );
static void Print_Scales( class chartobj	  *self );
static void Print_Left_Scale( class chartobj	  *self );
static void Print_Labels( class chartobj	  *self );
static void Print_Horizontal_Labels( class chartobj	  *self, long left, long width, long middle );


chartobj::chartobj( )
      {
  class chartobj *self=this;
  IN(chartobj_InitializeObject);
  DEBUGst(RCSID,rcsidchartobj);
  (this)->SetShrinkIcon(  'e', "icon12", "Chart", "andysans10b" );
  (this)->SetHelpFileName(  "/usr/andy/help/chart.help"/*=== ===*/ );
  (this)->SetOptions(  aptv_SuppressControl );
  Shadows = NULL;
  Chart = NULL;
  ScalePositions = LabelPositions = 0;
  ScalesSuppressed = LabelsSuppressed = GridsSuppressed = false;
  PIXELSPERUNIT = PIXELSPERINTERVAL = 0;
  ScaleTick = 1;
/*===*/ScalePositions = chartv_Left;
/*===*/LabelPositions = chartv_Bottom;
  ScaleFontName = "andysans8";
  ScaleFont = (this)->BuildFont(  ScaleFontName, NULL );
  LabelFontName = "andysans8";
  LabelFont = (this)->BuildFont(  LabelFontName, NULL );
  GraphicFont = (this)->BuildFont(  "shape10", NULL );
  DottedGraphic = DashedGraphic = NULL;
  CURRENTITEM = NULL;
  OUT(chartobj_InitializeObject);
  THROWONFAILURE(  TRUE);
  }

chartobj::~chartobj( )
      {
  IN(chartobj_FinalizeObject);
  Free_Shadows( this );
/*===*/
  OUT(chartobj_FinalizeObject);
  }

void 
chartobj::UnlinkNotification( class view		  *linkee )
{
  IN(chartobj_UnlinkNotification);
  if ( linkee == (class view *)this )
      (this)->RetractViewCursors(  this );
  aptv::UnlinkNotification(linkee);
/*===*/
  OUT(chartobj_UnlinkNotification);
  }

void
chartobj::ObservedChanged( class observable	      *changed, long			       value )
        {
  class chartobj *self=this;
  IN(chartobj_ObservedChanged);
  switch ( value )
    {
    case chart_ItemValueChanged:
/*===*/
      break;
    case chart_ItemsSorted:
/*===*/
      break;
    }
  (this )->ClearBody( );
  (this)->FullUpdate(  view_FullRedraw, Left, Top, Width, Height );
  OUT(chartobj_ObservedChanged);
  }

void
chartobj::ObserveChart( long			     change )
      {
  class chartobj *self=this;
  IN(chartobj_ObserveChart);
  (this)->SetFont(  (this)->BuildFont(  "andysans10b", NULL ) );
  (this)->MoveTo(  Center, Middle );
  (this)->DrawString(  "MISSING ObserveChart METHOD", Balanced );
  OUT(chartobj_ObserveChart);
  }

void
chartobj::SetDebug( boolean		      state )
      {
  IN(chartobj_SetDebug);
  debug = state;
  OUT(chartobj_SetDebug);
  }

void
chartobj::SetDataObject( class dataobject	      *data )
      {
  class chartobj *self=this;
  IN(chartobj_SetDataObject);
  (this)->aptv::SetDataObject(  Chart = (class chart *)data );
  DEBUGst(ChartType,chart_Type( (class chart *)data ));
  OUT(chartobj_SetDataObject);
  }

void
chartobj::SetChartOptions( long		       options )
      {
  class chartobj *self=this;
  IN(chartobj_SetChartOptions);

  if ( options & chartobj_SuppressScales )
    ScalesSuppressed = true;
  if ( options & chartobj_SuppressLabels )
    LabelsSuppressed = true;
  OUT(chartobj_SetChartOptions);
  }

const char *
chartobj::Moniker( )
    {
  IN(chartobj_Moniker);
  OUT(chartobj_Moniker);
  return  "UNKNOWN";
  }

void 
chartobj::FullUpdate( enum view_UpdateType	   type, long			   left , long			   top , long			   width , long			   height )
        {
  class chartobj *self=this;
  char				  value_string[25];
  long				  W, H;

  IN(chartobj_FullUpdate);
  (this)->aptv::FullUpdate(  type, left, top, width, height );
  if ( ! (this)->BypassUpdate()  &&  Height > 0  &&
	(type == view_FullRedraw || type == view_LastPartialRedraw) )
    { DEBUG(Not Bypassed);
    ChartLeft = Left;  ChartTop = Top;  ChartWidth = Width;  ChartHeight = Height;
    Generate_Shadows( this );
    LeftScaleLeft    = LeftScaleTop    = LeftScaleWidth    = LeftScaleHeight    = 0;
    RightScaleLeft   = RightScaleTop   = RightScaleWidth   = RightScaleHeight   = 0;
    TopScaleLeft     = TopScaleTop     = TopScaleWidth     = TopScaleHeight     = 0;
    BottomScaleLeft  = BottomScaleTop  = BottomScaleWidth  = BottomScaleHeight  = 0;
    LeftLabelsLeft   = LeftLabelsTop   = LeftLabelsWidth   = LeftLabelsHeight   = 0;
    RightLabelsLeft  = RightLabelsTop  = RightLabelsWidth  = RightLabelsHeight  = 0;
    TopLabelsLeft    = TopLabelsTop    = TopLabelsWidth    = TopLabelsHeight    = 0;
    BottomLabelsLeft = BottomLabelsTop = BottomLabelsWidth = BottomLabelsHeight = 0;
    if ( !ScalesSuppressed )
      { DEBUG(Prepare for Scales);
      DottedGraphic = (GraphicFont)->CvtCharToGraphic( 
			    (this )->GetDrawable( ), DottedIcon );
      sprintf( value_string, "%ld", (Chart)->ItemValueGreatest( ) );
      (ScaleFont)->StringSize(  (this )->GetDrawable( ),
			   value_string, &W, &H );
      if ( ScalePositions & chartv_Left )
	{
	LeftScaleLeft    = ChartLeft;
	LeftScaleTop     = ChartTop;
	LeftScaleWidth   = W + 20;
	LeftScaleHeight  = ChartHeight;
	ChartLeft	+= LeftScaleWidth;
	ChartWidth	-= LeftScaleWidth;
	}
      if ( ScalePositions & chartv_Right )
	{
	RightScaleTop     = ChartTop;
	RightScaleWidth   = W + 20;
	RightScaleLeft    = ChartRight;
	RightScaleHeight  = ChartHeight;
	ChartWidth	 -= RightScaleWidth;
	}
      if ( ScalePositions & chartv_Top )
	{
	TopScaleLeft      = ChartLeft;
	TopScaleTop       = ChartTop;
	TopScaleWidth     = ChartWidth;
	TopScaleHeight    = H + 20;
	ChartHeight	 -= TopScaleHeight;
	LeftScaleTop     += TopScaleHeight;
	RightScaleTop    += TopScaleHeight;
	LeftScaleHeight  -= TopScaleHeight;
	RightScaleHeight -= TopScaleHeight;
	}
      if ( ScalePositions & chartv_Bottom )
	{
	BottomScaleLeft   = ChartLeft;
	BottomScaleWidth  = ChartWidth;
	BottomScaleHeight = H + 20;
	BottomScaleTop    = ChartBottom - BottomScaleHeight;
	ChartHeight	 -= BottomScaleHeight;
	LeftScaleHeight  -= TopScaleHeight;
	RightScaleHeight -= TopScaleHeight;
	}
      }
    if ( !LabelsSuppressed )
      { DEBUG(Prepare for Labels);
      (LabelFont)->StringSize(  (this )->GetDrawable( ),
			   "M", &W, &H );
      if ( LabelPositions & chartv_Left )
	{
	LeftLabelsLeft    = ChartLeft;
	LeftLabelsTop     = ChartTop;
	LeftLabelsWidth   = W + 20;
	LeftLabelsHeight  = ChartHeight;
	ChartLeft	 += LeftLabelsWidth;
	ChartWidth	 -= LeftLabelsWidth;
	}
      if ( LabelPositions & chartv_Right )
	{
	RightLabelsTop     = ChartTop;
	RightLabelsWidth   = W + 20;
	RightLabelsLeft    = ChartRight;
	RightLabelsHeight  = ChartHeight;
	ChartWidth	  -= RightLabelsWidth;
	}
      if ( LabelPositions & chartv_Top )
	{
	TopLabelsLeft      = ChartLeft;
	TopLabelsTop       = ChartTop;
	TopLabelsWidth     = ChartWidth;
	TopLabelsHeight    = H + 20;
	ChartHeight	  -= TopLabelsHeight;
	LeftLabelsTop     += TopLabelsHeight;
	RightLabelsTop    += TopLabelsHeight;
	LeftLabelsHeight  -= TopLabelsHeight;
	RightLabelsHeight -= TopLabelsHeight;
	}
      if ( LabelPositions & chartv_Bottom )
	{
	BottomLabelsLeft   = ChartLeft;
	BottomLabelsWidth  = ChartWidth;
	BottomLabelsHeight = H + 20;
	BottomLabelsTop    = ChartBottom - BottomLabelsHeight;
	ChartHeight	  -= BottomLabelsHeight;
	LeftLabelsHeight  -= TopLabelsHeight;
	RightLabelsHeight -= TopLabelsHeight;
	}
      }
    DEBUGdt(ItemValueSpan,(Chart)->ItemValueSpan());
    PIXELSPERUNIT = (1.0*ChartHeight) /
	(1.0*(((Chart )->ItemValueSpan( ) > 0) ?
	     (Chart )->ItemValueSpan( ) : 1));
    DEBUGgt(PIXELSPERUNIT,PIXELSPERUNIT);
    PIXELSPERINTERVAL = PIXELSPERUNIT * (Chart )->ItemValueRangeInterval( );
    DEBUGgt(PIXELSPERINTERVAL,PIXELSPERINTERVAL);
    ChartBaseLine = ChartTop + (Chart )->ItemValueGreatest( )*PIXELSPERINTERVAL;
    if ( !ScalesSuppressed )
      {
      LeftScaleHeight  -= BottomLabelsHeight + 1;
      RightScaleHeight -= BottomLabelsHeight + 1;
      Draw_Scales( this );
      }
    if ( !LabelsSuppressed )
      {
      Draw_Labels( this );
      }
    Set_Shadows( this );
    (this )->DrawChart( );
    }
  OUT(chartobj_FullUpdate);
  }

void
chartobj::DrawChart( )
    {
  class chartobj *self=this;
  IN(chartobj_DrawChart);
  (this)->SetFont(  (this)->BuildFont(  "andysans10b", NULL ) );
  (this)->MoveTo(  Center, Middle );
  (this)->DrawString(  "MISSING DrawChart METHOD", Balanced );
  OUT(chartobj_DrawChart);
  }

class view *
chartobj::Hit( enum view_MouseAction       action, long			       x , long			       y , long			       clicks )
        {
  class view		     *hit;

  IN(chartobj_Hit );
  if ( (hit = (this)->aptv::Hit(  action, x, y, clicks )) == NULL )
    hit = (class view *) (this)->HitChart(  action, x, y, clicks );
  OUT(chartobj_Hit );
  return  hit;
  }

class view *
chartobj::HitChart( enum view_MouseAction     action, long			     x , long			     y , long			     clicks )
        {
  class chartobj *self=this;
  static struct chart_item_shadow  *shadow;
  long			    delta;
  static long			    value, value_original, initial_y, prior_y;
  static char			    value_string[25], *name;

  IN(chartobj_HitChart);
  if ( shadow  ||
	 (action == view_LeftDown  &&  (shadow = (this)->WhichItem(  x, y ))) )
    {
    CURRENTITEM = shadow->item;
    if ( y > Bottom )	y = Bottom;
    if ( y < Top )	y = Top;
    switch ( action )
      {
      case  view_LeftDown:
	(this )->UseInvisibleCursor( );
        (this)->SetTransferMode(  graphic_INVERT );
	initial_y = y;
	y = ShadowMiddle(shadow);
	(this)->MoveTo(  Left, prior_y = y );
	(this)->DrawLineTo(  Right, y );
	name = (char *) (Chart)->ItemAttribute(  shadow->item, chart_itemname );
	value = value_original = (Chart)->ItemAttribute(  shadow->item, chart_itemvalue );
	DEBUGdt(Initial-value,value);
        break;
      case  view_LeftMovement:
	(this)->MoveTo(  Left, prior_y );
	(this)->DrawLineTo(  Right, prior_y );
	if ( abs(delta = prior_y - y) > PIXELSPERINTERVAL )
	  value += (delta / PIXELSPERINTERVAL) * (Chart )->ItemValueRangeInterval( );
	(this)->MoveTo(  Left, prior_y = y );
	(this)->DrawLineTo(  Right, y );
        break;
      case  view_LeftUp:
	(this)->MoveTo(  Left, prior_y );
	(this)->DrawLineTo(  Right, prior_y );
	if ( abs(delta = initial_y - y) > PIXELSPERINTERVAL )
	  {
	  value_original += (delta / PIXELSPERINTERVAL) * (Chart )->ItemValueRangeInterval( );
	  DEBUGdt(Final-value,value);
	  (Chart)->SetItemAttribute(  shadow->item,
	    chart_ItemValue( (value = value_original) ) );
	  (Chart )->SetModified( );
	  (Chart)->NotifyObservers(  chart_ItemValueChanged );
	  }
	shadow = NULL;
	(this )->UseNormalCursor( );
        break;
      default:
	break;
      }
    sprintf( value_string, "%s:  Value = %ld", name, value );
    (this)->Announce(  value_string );
    }
  OUT(chartobj_HitChart);
  return  (class view *) this;
  }

static
void Generate_Shadows( class chartobj	    *self )
    {
  struct chart_item_shadow   *shadow = NULL, *prior = NULL;
  struct chart_item	   *chart_item = (Chart )->ItemAnchor( );

  IN(Generate_Shadows);
  if ( Shadows )
    Free_Shadows( self );
  while ( chart_item )
    {
    shadow = (struct chart_item_shadow *) calloc( 1, sizeof(struct chart_item_shadow) );
    if ( prior )
      prior->next = shadow;
      else
      Shadows = shadow;
    prior = shadow;
    ShadowItem(shadow) = chart_item;
    chart_item = (Chart)->NextItem(  chart_item );
    }
  DEBUGdt(ItemValueRangeLow,(Chart)->ItemValueRangeLow());
  DEBUGdt(ItemValueRangeInterval,(Chart)->ItemValueRangeInterval());
  DEBUGdt(ItemValueRangeHigh,(Chart)->ItemValueRangeHigh());
  DEBUGdt(ItemValueSpan,(Chart)->ItemValueSpan());
  OUT(Generate_Shadows);
  }

static
void Set_Shadows( class chartobj	      *self )
    {
  long			      i, x, y, width, high_adjust = 0, low_adjust = 0,
				      count = (Chart )->ItemCount( ),
				      excess, fudge;
  struct chart_item_shadow  *shadow = Shadows;

  IN(Set_Shadows);
  width = (ChartWidth / ((count) ? count : 1));
  x = ChartLeft + width/2;
  if ( ( excess = (ChartWidth - (width * count)) / 2 ) )
    excess = count / excess;
  DEBUGdt(width,width);
  if (  (ScaleTick > 0) && ((Chart )->ItemValueGreatest( ) % ScaleTick ) )
    high_adjust = (ScaleTick - ((Chart )->ItemValueGreatest( ) % ScaleTick)) * PIXELSPERUNIT;
  DEBUGdt(HighAdjust,high_adjust);
  if ( (Chart)->ItemValueLeast( ) < 0 )
    {
    low_adjust = abs((Chart )->ItemValueLeast( ) * PIXELSPERUNIT);
    ChartBaseLine = ChartBottom - low_adjust;
    if (  (ScaleTick > 0) && ((Chart )->ItemValueLeast( ) % ScaleTick ) )
      low_adjust += (ScaleTick + ((Chart )->ItemValueLeast( ) % ScaleTick)) * PIXELSPERUNIT;
    }
  DEBUGdt(LowAdjust,low_adjust);
  DEBUGdt(BaseLine,ChartBaseLine);  DEBUGdt(Bottom,ChartBottom);
  for ( i = 0; i < count  &&  shadow; i++ )
    {
    DEBUGdt(Value,(Chart)->ItemAttribute(  ShadowItem(shadow), chart_ItemValue(0) ));
    if ( (y = (ChartBottom -
	((Chart)->ItemAttribute(  ShadowItem(shadow), chart_itemvalue ) *
	PIXELSPERUNIT)) + high_adjust - low_adjust) > ChartBottom - 2 )
      y = ChartBottom - 2;
    ShadowY(shadow) = y;
    ShadowTop(shadow) = y - 5;
    fudge = (excess) ? ((i % excess) ? 0 : 1) : 0;
    ShadowX(shadow) = x + fudge;
    ShadowLeft(shadow) = x + fudge - 5;
    x += width + fudge;
    ShadowWidth(shadow) = ShadowHeight(shadow) = 10;
    shadow = NEXTITEM(shadow);
    }
  OUT(Set_Shadows);
  }

static
void Free_Shadows( class chartobj	      *self )
    {
  struct chart_item_shadow  *shadow = Shadows, *next;

  while ( shadow )
    {
    next = NEXTITEM(shadow);
    free( shadow );
    shadow = next;
    }
  Shadows = NULL;
  }

struct chart_item_shadow *
chartobj::WhichItem( long			       x , long			       y )
      {
  class chartobj *self=this;
  struct chart_item_shadow  *shadow = Shadows;

  while ( shadow )
    {
    if ( (this)->Within(  x, y, ShadowBounds(shadow) ) )
      break;
    shadow = NEXTITEM(shadow);
    }
  return  shadow;
  }

void
Printer( class chartobj	      *self, FILE			      *file, char			      *processor, char			      *format, boolean		       level, aptv_printfptr printer )
            {
  IN(Printer);
  if ( !ScalesSuppressed )
    Print_Scales( self );
  if ( !LabelsSuppressed )
    Print_Labels( self );
  (self )->PrintChart( );
  OUT(Printer);
  }

void
chartobj::PrintChart( )
    {
  class chartobj *self=this;
  IN(chartobj_PrintChart);
  (this)->SetFont(  (this)->BuildFont(  "andysans10b", NULL ) );
  (this)->MoveTo(  Center, Middle );
  (this)->DrawString(  "MISSING PrintChart METHOD", Balanced );
  OUT(chartobj_PrintChart);
  }

void
chartobj::Print( FILE			      *file, const char			      *processor, const char			      *format, boolean		       top_level )
            {
  IN(chartobj_Print);
  if ( top_level )
    {
    (this)->SetPrintOptions(  aptv_PrintFillPage |
				    aptv_PrintLandScape );
    (this)->SetPrintUnitDimensions(  8.5, 11.0 );
    (this)->SetPrintPageDimensions(  8.5, 11.0 );
    }
  (this)->aptv::PrintObject(  file, processor, format, top_level, (aptv_printfptr) Printer );
  OUT(chartobj_Print);
  }

static
void Draw_Labels( class chartobj	  *self )
    {
  IN(Draw_Labels);
  (self)->SetFont(  LabelFont );
  if ( LabelPositions & chartv_Left )
    Draw_Vertical_Labels( self, LeftLabelsLeft,  LeftLabelsTop,
				LeftLabelsWidth, LeftLabelsHeight );
  if ( LabelPositions & chartv_Right )
    Draw_Vertical_Labels( self, RightLabelsLeft,  RightLabelsTop,
				RightLabelsWidth, RightLabelsHeight );
  if ( LabelPositions & chartv_Top )
    Draw_Horizontal_Labels( self, TopLabelsLeft,  TopLabelsTop,
				  TopLabelsWidth, TopLabelsHeight );
  if ( LabelPositions & chartv_Bottom )
    Draw_Horizontal_Labels( self, BottomLabelsLeft,  BottomLabelsTop,
				  BottomLabelsWidth, BottomLabelsHeight );
  OUT(Draw_Labels);
  }

static
void Draw_Horizontal_Labels( class chartobj	  *self, long left, long top, long width, long height )
    {
  struct chart_item	 *chart_item = (Chart )->ItemAnchor( );
  short		  x, x_increment, y, excess, fudge, i = 0;

  IN(Draw_Horizontal_Labels);
  (self)->SetTransferMode(  graphic_WHITE );
  (self)->FillRectSize(  left, top, width, height, graphic_WHITE );
  (self)->SetTransferMode(  graphic_BLACK );
  if ( (Chart )->ItemCount( ) )
    {
    x_increment = width / (Chart )->ItemCount( );
    x = left + x_increment/2;
    y = top + height/2;
    if ( ( excess = (width - (x_increment * (Chart )->ItemCount( ))) / 2 ) )
      excess = (Chart )->ItemCount( ) / excess;
    while ( chart_item )
      {
      (self)->MoveTo(  x, y );
      fudge = (excess) ? ((i % excess) ? 0 : 1) : 0;
      x += x_increment + fudge;
      (self)->DrawString( 
	(char *) ((Chart)->ItemAttribute(  chart_item, chart_itemname )), Balanced );
      chart_item = (Chart)->NextItem(  chart_item );
      i++;
      }
    }
  OUT(Draw_Horizontal_Labels);
  }

static
void Draw_Vertical_Labels( class chartobj	  *self, long left, long top, long width, long height )
    {
  IN(Draw_Left_Labels);
/*===*/
  OUT(Draw_Left_Labels);
  }

static
void Draw_Scales( class chartobj	  *self )
    {
  IN(Draw_Scales);
  (self)->SetFont(  ScaleFont );
  if ( ScalePositions & chartv_Left )	Draw_Left_Scale( self );
  if ( ScalePositions & chartv_Right )	Draw_Right_Scale( self );
  if ( ScalePositions & chartv_Top )	Draw_Top_Scale( self );
  if ( ScalePositions & chartv_Bottom )	Draw_Bottom_Scale( self );
  OUT(Draw_Scales);
  }

static
void Draw_Left_Scale( class chartobj	  *self )
    {
  long			  value = 0, Y, adjust;
  float		  y, y_increment;
  long				  half_y_increment;
  char				  value_string[25];

  IN(Draw_Left_Scale); /*=== NEEDS WORK ===*/
  Prepare_Vertical_Scale( self );
  (self)->SetTransferMode(  graphic_WHITE );
  (self)->FillRectSize(  LeftScaleLeft,  LeftScaleTop,
			       LeftScaleWidth, LeftScaleHeight, graphic_WHITE );
  (self)->SetTransferMode(  graphic_COPY );
  (self)->MoveTo(  LeftScaleBarX, LeftScaleTop );
  (self)->DrawLineTo(  LeftScaleBarX, LeftScaleBottom );
  (self)->MoveTo(  LeftScaleBarX, LeftScaleTop );
  (self)->DrawLineTo(  LeftScaleRight, LeftScaleTop );
  (self)->MoveTo(  LeftScaleBarX, LeftScaleBottom );
  (self)->DrawLineTo(  LeftScaleRight, LeftScaleBottom );
  (self)->SetLineDash(  "\001\004", 0, graphic_LineOnOffDash);
  (self)->MoveTo(  LeftScaleRight, LeftScaleTop );
  (self)->DrawLineTo(  LeftScaleRight+ChartWidth, LeftScaleTop );
  (self)->MoveTo(  LeftScaleRight, LeftScaleBottom );
  (self)->DrawLineTo(  LeftScaleRight+ChartWidth, LeftScaleBottom );
  (self)->SetLineDash(  NULL, 0, graphic_LineSolid);
  if ( (ScaleTick > 0) && (adjust = (value = (Chart)->ItemValueLeast()) % ScaleTick ) )
    value -= ScaleTick + adjust;
  sprintf( value_string, "%ld", value );
  (self)->MoveTo(  LeftScaleBarX-5, LeftScaleBottom );
  (self)->DrawString(  value_string, RightBottom );
  if ( (ScaleTick > 0) && (adjust = (value = (Chart)->ItemValueGreatest()) % ScaleTick ) )
    value += ScaleTick - adjust;
  sprintf( value_string, "%ld", value );
  (self)->MoveTo(  LeftScaleBarX-5, LeftScaleTop );
  (self)->DrawString(  value_string, RightTop );
  if ( ( y_increment = PIXELSPERUNIT * ScaleTick ) )
    {
    half_y_increment = y_increment / 2;
    Y = y = LeftScaleTop + y_increment;
    (self)->MoveTo(  LeftScaleBarX, Y - half_y_increment );
    (self)->DrawLineTo(  LeftScaleRight - 5, Y - half_y_increment );
    while ( Y < (LeftScaleBottom - half_y_increment) )
      {
      (self)->MoveTo(  LeftScaleBarX, Y );
      (self)->DrawLineTo(  LeftScaleRight, Y );
      (self)->SetLineDash(  "\001\004", 0, graphic_LineOnOffDash);
      (self)->MoveTo(  LeftScaleRight, Y );
      (self)->DrawLineTo(  LeftScaleRight+ChartWidth, Y );
      (self)->SetLineDash(  NULL, 0, graphic_LineSolid);
      (self)->MoveTo(  LeftScaleBarX, Y + half_y_increment );
      (self)->DrawLineTo(  LeftScaleRight - 5, Y + half_y_increment );
      value -= ScaleTick;
      sprintf( value_string, "%ld", value );
      (self)->MoveTo(  LeftScaleBarX-5, Y );
      (self)->DrawString(  value_string, RightMiddle );
      Y = y = y + y_increment;
      }
    if ( y < (LeftScaleBottom - half_y_increment ) )
      {
      (self)->MoveTo(  LeftScaleBarX, Y = (y + half_y_increment) );
      (self)->DrawLineTo(  LeftScaleRight - 5, Y = (y + half_y_increment) );
      }
    }
  OUT(Draw_Left_Scale); /*=== NEEDS WORK ===*/
  }

static
void Draw_Right_Scale( class chartobj	  *self )
    {
  IN(Draw_Right_Scale);
  Prepare_Vertical_Scale( self );
/*===*/
  OUT(Draw_Right_Scale);
  }

static
void Prepare_Vertical_Scale( class chartobj	  *self )
    {
  long  ValueSpanScale = 1;
  long  ScaledValueSpan;
  static int nice_tick_values[] = {1, 5, 10, 25, 50, 100, 0};
  int tick, tick_index;

  IN(Prepare_Vertical_Scale);
  /* First scale the value span to < 1000. */
  DEBUGdt(Value Span,(Chart )->ItemValueSpan( ));
  ScaledValueSpan = (Chart )->ItemValueSpan( );
  while ( ScaledValueSpan >= 100 ) {
    ValueSpanScale *= 10;
    ScaledValueSpan /= 10;
  }
  DEBUGdt(ValueSpanScale, ValueSpanScale);
  /* Now find a tick size that is >= 20 pixels high */
  tick = nice_tick_values[0];
  tick_index = 0;
  while ( tick != 0 && PIXELSPERUNIT*tick*ValueSpanScale < 20 )
    tick = nice_tick_values[++tick_index];
  if (tick == 0)
    tick = nice_tick_values[tick_index-1];  /* use last nice value (can't happen) */
  DEBUGdt(tick, tick);
  ScaleTick = tick*ValueSpanScale;
  if (ScaleTick > (Chart)->ItemValueSpan())
    ScaleTick = (Chart)->ItemValueSpan();
  DEBUGdt(ScaleTick,ScaleTick);
  OUT(Prepare_Vertical_Scale);
  }

static
void Draw_Top_Scale( class chartobj	  *self )
    {
  IN(Draw_Top_Scale);
  Prepare_Horizontal_Scale( self );
/*===*/
  OUT(Draw_Top_Scale);
  }

static
void Draw_Bottom_Scale( class chartobj	  *self )
    {
  IN(Draw_Bottom_Scale);
  Prepare_Horizontal_Scale( self );
/*===*/
  OUT(Draw_Bottom_Scale);
  }

static
void Prepare_Horizontal_Scale( class chartobj	  *self )
    {
  IN(Prepare_Horizontal_Scale);
/*===*/
  OUT(Prepare_Horizontal_Scale);
  }

static
void Print_Scales( class chartobj	  *self )
    {
  IN(Print_Scales);
  (self)->SetPrintLineWidth(  1 );
  (self)->SetPrintFont(  ScaleFontName );
  if ( ScalePositions & chartv_Left )	Print_Left_Scale( self );
/*===
  if ( ScalePositions & chartv_Right )	Print_Right_Scale( self );
  if ( ScalePositions & chartv_Top )	Print_Top_Scale( self );
  if ( ScalePositions & chartv_Bottom )	Print_Bottom_Scale( self );
===*/
  OUT(Print_Scales);
  }

static
void Print_Left_Scale( class chartobj	  *self )
    {
  long			  value, y, adjust,
				  y_increment, half_y_increment;
  char				  value_string[25];

  IN(Print_Left_Scale);
  Prepare_Vertical_Scale( self );
  (self)->PrintLine(  LeftScaleBarX, LeftScaleTop, LeftScaleBarX, LeftScaleBottom );
  (self)->PrintLine(  LeftScaleBarX, LeftScaleTop, LeftScaleRight, LeftScaleTop );
/*  chartobj_FillTrapezoid( self, LeftScaleRight, LeftScaleTop, ChartWidth,
				LeftScaleRight, LeftScaleTop, ChartWidth, DottedGraphic );*/
  (self)->PrintLine(  LeftScaleBarX, LeftScaleBottom, LeftScaleRight, LeftScaleBottom );
/*  chartobj_FillTrapezoid( self, LeftScaleRight, LeftScaleBottom, ChartWidth,
				LeftScaleRight, LeftScaleBottom, ChartWidth, DottedGraphic );*/
  if ( ( adjust = (value = (Chart)->ItemValueLeast( )) % ScaleTick ) )
    value -= ScaleTick + adjust;
  sprintf( value_string, "%ld", value );
  (self)->PrintString(  LeftScaleBarX-5, LeftScaleBottom, value_string, RightBottom );
  if ( ( adjust = (value = (Chart)->ItemValueGreatest( )) % ScaleTick ) )
    value += ScaleTick - adjust;
  sprintf( value_string, "%ld", value );
  (self)->PrintString(  LeftScaleBarX-5, LeftScaleTop, value_string, RightTop );
  if ( ( y_increment = PIXELSPERUNIT * ScaleTick ) )
    {
    half_y_increment = y_increment / 2;
    y = LeftScaleTop + y_increment;
    (self)->PrintLine(  LeftScaleBarX, y - half_y_increment,
			      LeftScaleRight - 5, y - half_y_increment );
    while ( y < (LeftScaleBottom - y_increment) )
      {
      (self)->PrintLine(  LeftScaleBarX, y, LeftScaleRight, y );
/*      chartobj_FillTrapezoid( self, LeftScaleRight, y, ChartWidth,
				    LeftScaleRight, y, ChartWidth, DottedGraphic );*/
      (self)->PrintLine(  LeftScaleBarX, y + half_y_increment,
				LeftScaleRight - 5, y + half_y_increment );
      value -= ScaleTick;
      sprintf( value_string, "%ld", value );
      (self)->PrintString(  LeftScaleBarX-5, y, value_string, RightMiddle );
      y += y_increment;
      }
    if ( y < (LeftScaleBottom - half_y_increment ) )
      {
      (self)->PrintLine(  LeftScaleBarX, y + half_y_increment,
				LeftScaleRight - 5, y + half_y_increment );
      }
    }
  OUT(Print_Left_Scale);
  }

static
void Print_Labels( class chartobj	  *self )
    {
  IN(Print_Labels);
  (self)->SetPrintFont(  LabelFontName );
/*===
  if ( LabelPositions & chartv_Left )	Print_Left_Labels( self );
  if ( LabelPositions & chartv_Right )	Print_Right_Labels( self );
===*/
  if ( LabelPositions & chartv_Top )
    Print_Horizontal_Labels( self, TopLabelsLeft, TopLabelsWidth, TopLabelsMiddle );
  if ( LabelPositions & chartv_Bottom )
    Print_Horizontal_Labels( self, BottomLabelsLeft, BottomLabelsWidth, BottomLabelsMiddle );
  OUT(Print_Labels);
  }

static
void Print_Horizontal_Labels( class chartobj	  *self, long left, long width, long middle )
    {
  struct chart_item	 *chart_item = (Chart )->ItemAnchor( );
  short		  x, x_increment, y;

  IN(Print_Horizontal_Labels);
  if ( (Chart )->ItemCount( ) )
    {
    x_increment = width / (Chart )->ItemCount( );
    x = left + x_increment/2;
    y = middle;
    while ( chart_item )
      {
      (self)->PrintString(  x, y,
	(char *) ((Chart)->ItemAttribute(  chart_item, chart_itemname )), Balanced );
      x += x_increment;
      chart_item = (Chart)->NextItem(  chart_item );
      }
    }
  OUT(Print_Horizontal_Labels);
  }

