#ifndef _chartobj_H_
#define _chartobj_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

/** \addtogroup libchart
 * @{ */
/***  SPECIFICATION -- External Facility Suite  *********************************

TITLE	The Chart View-object Abstract-class

MODULE	chartobj.ch

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
  05/08/89	Use UnlinkNotification (TCP)
  05/10/89	Precompute data-points into Shadows (TCP)
  06/01/89	Add ChartBaseLine macro-method (TCP)
  09/06/89	Add CurrentItem method (TCP)

END-SPECIFICATION  ************************************************************/


#define  chartobj_VERSION		    1
#include <rect.h>

/***  Options  ***/

#define  chartobj_SuppressScales    (1<<0)
#define  chartobj_SuppressLabels    (1<<1)
#define  chartobj_SuppressGrids	    (1<<2)


struct chart_item_shadow
  {
  struct chart_item_shadow	    *next;
  struct chart_item		    *item;
  long				     datum;
  struct rectangle		     bounds;
  short				     x, y;
  };


#include <aptv.H>
class chartobj : public  aptv
  {
public:
	TRACED_CLASS(chartobj);



   void FullUpdate( enum view::UpdateType type, long left, long top, long width, long height );
   void SetDataObject( class dataobject *data );
   class view * Hit( enum view::MouseAction action, long x, long y, long n )	 ;
   void ObservedChanged( class observable *changed, long value );
   void UnlinkNotification( class view *linkee );
   void Print( FILE *file, const char *processor, const char *finalFormat, boolean topLevel );



  virtual struct chart_item_shadow * WhichItem( long x, long y ) ;
  virtual void SetChartOptions( long options );
  virtual void SetDebug( boolean state );

  /***  The following Methods are to be Overriden by sub-classes  ***/
  virtual void DrawChart();
  virtual void PrintChart();
  virtual class view * HitChart( enum view::MouseAction action, long x, long y, long clicks )		 ;
  virtual void ObserveChart( long change );
  virtual const char * Moniker()					 ;



  inline float PixelsPerUnit()		{ return (this->pixels_per_unit_value); }
  inline float PixelsPerInterval()		{ return (this->pixels_per_value_interval); }
  inline struct chart_item_shadow	 * Items()			{ return (this->item_shadows); }
  inline struct rectangle * ItemBounds( struct chart_item_shadow *shadow)		{ return (&(shadow)->bounds); }
  inline long ItemLeft( struct chart_item_shadow * shadow)		{ return ((shadow)->bounds.left); }
  inline long ItemTop( struct chart_item_shadow * shadow)		{ return ((shadow)->bounds.top); }
  inline long ItemWidth( struct chart_item_shadow * shadow)		{ return ((shadow)->bounds.width); }
  inline long ItemHeight( struct chart_item_shadow * shadow)		{ return ((shadow)->bounds.height); }
  inline long ItemCenter( struct chart_item_shadow * shadow)		{ return (ItemLeft(shadow)+ItemWidth(shadow)/2); }
  inline long ItemMiddle( struct chart_item_shadow * shadow)		{ return (ItemTop(shadow)+ItemHeight(shadow)/2); }
  inline long ItemRight( struct chart_item_shadow * shadow)		{ return (ItemLeft(shadow)+ItemWidth(shadow)); }
  inline long ItemBottom( struct chart_item_shadow * shadow)		{ return (ItemTop(shadow)+ItemHeight(shadow)); }
  inline short ItemX( struct chart_item_shadow * shadow)		{ return ((shadow)->x); }
  inline short ItemY( struct chart_item_shadow * shadow)		{ return ((shadow)->y); }
  inline long ItemDatum( struct chart_item_shadow * shadow)		{ return ((shadow)->datum); }
  inline struct chart_item_shadow * NextItem( struct chart_item_shadow * shadow)		{ return ((shadow)->next); }
  inline struct rectangle * ChartBounds()			{ return (&this->chart_bounds); }
  inline long ChartLeft()			{ return (this->chart_bounds.left); }
  inline long ChartTop()			{ return (this->chart_bounds.top); }
  inline long ChartWidth()			{ return (this->chart_bounds.width); }
  inline long ChartHeight()			{ return (this->chart_bounds.height); }
  inline long ChartCenter()			{ return ((this)->ChartLeft() + (this)->ChartWidth()/2); }
  inline long ChartRight()			{ return ((this)->ChartLeft() + (this)->ChartWidth() - 1); }
  inline long ChartMiddle()			{ return ((this)->ChartTop()  + (this)->ChartHeight()/2); }
  inline long ChartBottom()			{ return ((this)->ChartTop()  + (this)->ChartHeight() - 1); }
  inline long ChartBaseLine()		{ return (this->baseline); }
  inline struct chart_item * CurrentItem()			{ return (this->current_item); }
  inline void SetCurrentItem( struct chart_item *item )  { ((this)->current_item = item); }


  
	chartobj();
  



  class chart			 *data_object;
  float				  pixels_per_unit_value,
				  pixels_per_value_interval;
  char				  scale_positions, label_positions;
  long				  scale_tick;
  struct rectangle		  chart_bounds,
				  left_scale, right_scale, top_scale, bottom_scale,
				  left_labels, right_labels, top_labels, bottom_labels;
  class fontdesc		 *scale_font, *label_font, *graphic_font;
  const char			 *scale_font_name, *label_font_name;
  class graphic		 *dotted_graphic, *dashed_graphic;
  struct chart_item_shadow	 *item_shadows;
  struct chart_item		 *current_item;
  short				  baseline;
  boolean			  suppress_scales, suppress_labels, suppress_grids;
  };
/** @} */
#endif /* _chartobj_H_ */
