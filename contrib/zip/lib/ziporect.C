/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

/* ziporect.c	Zip Object -- Rectangle					      */
/* Author	TC Peters						      */
/* Information Technology Center		   Carnegie-Mellon University */

/**  SPECIFICATION -- Internal Facility Suite  *********************************

TITLE	Zip Object -- Rectangle

MODULE	ziporect.c

NOTICE	IBM Internal Use Only

DESCRIPTION

.......................

HISTORY
  04/13/88	Created (TC Peters)
  05/01/89	Use symbolic font-names (TCP)
   08/24/89	Remove excess SetTransferMode() activity in Draw() (SCG)
   08/24/89	Modify Build to handle non-refresh of pane on build completion (SCG)
   08/14/90	Add Ensure_Attribute on Draw and Print (SCG)

END-SPECIFICATION  ************************************************************/

#include <andrewos.h>
#include <view.H>
#include <fontdesc.H>
#include "zip.H"
#include "zipview.H"
#include "zipedit.H"
#include "zipprint.H"
#include "zipobject.H"
#include "ziporect.H"

static const char		  * const rect_attributes[] =
					{
					"icon", "B",
					"iconfontname", IconFontName,
					NULL
					};

ATKdefineRegistry(ziporect, zipobject, NULL);

static long Draw( class ziporect *self, zip_type_figure  figure, zip_type_pane   pane, short  action );
static void Compute_Handle_Positions( class ziporect  *self, zip_type_figure  figure, zip_type_pane  pane, zip_type_pixel  *X1 , zip_type_pixel *X2 , zip_type_pixel *X3, zip_type_pixel *Y1 , zip_type_pixel *Y2 , zip_type_pixel *Y3 );


char
ziporect::Object_Attributes( const char * const *attributes[] )
  {
  IN(ziporect::Object_Attributes);
  *attributes = rect_attributes;
  OUT(ziporect::Object_Attributes);
  return  zip_ok;
  }

char
ziporect::Object_Icon( )
    {
  IN(ziporect::Object_Icon);
  OUT(ziporect::Object_Icon);
  return  'G';
  }

char
ziporect::Object_Icon_Cursor( )
    {
  IN(ziporect::Object_Icon_Cursor);
  OUT(ziporect::Object_Icon_Cursor);
  return  'B';
  }

char
ziporect::Object_Datastream_Code( )
    {
  IN(ziporect::Object_Datastream_Code);
  OUT(ziporect::Object_Datastream_Code);
  return  'G';
  }

long
ziporect::Show_Object_Properties( zip_type_pane		   pane, zip_type_figure		   figure )
        {
  (this->view_object)->Announce(  "Draw Rectangle from Upper-left to Lower-right." );
  return  zip_ok;
  }

long
ziporect::Build_Object( zip_type_pane   pane, enum view_MouseAction  action , long  x , long  y , long  clicks, zip_type_point X , zip_type_point  Y )
{
  zip_type_figure		  figure;
  long				  status = zip_ok;

  IN(ziporect::Build_Object);
  switch ( action )
    {
    case view_LeftDown:
      if ( (status =
        (this->data_object)->Create_Figure(  &CurrentFigure, NULL, zip_rectangle_figure,
			   CurrentImage, NULL )) == zip_ok )
	{
        (this)->Set_Object_Point(  CurrentFigure,	zip_figure_origin_point, X, Y );
        (this)->Set_Object_Point(  CurrentFigure,	zip_figure_auxiliary_point, X, Y );
	CurrentFigure->zip_figure_zoom_level = pane->zip_pane_zoom_level;
        pane->zip_pane_edit->zip_pane_edit_last_point_id = zip_figure_auxiliary_point;
	(this->data_object)->Set_Figure_Shade(  CurrentFigure,
			      pane->zip_pane_edit->zip_pane_edit_current_shade );
	(this->view_object)->Set_Pane_Painting_Mode(  pane, zipview_paint_inverted );
	}
      break;
    case view_LeftUp:
      if ( ( figure = CurrentFigure ) )
	{
	(this->view_object)->Set_Pane_Painting_Mode(  pane, zip_default );
	if ( figure_x_point == figure_x_points(0)  &&
	     figure_y_point == figure_y_points(0) )
	  {
	  (this->edit_object)->Delete_Figure(  figure, pane );
	  }
        else
          {
	  (this->view_object)->Draw_Figure(  CurrentFigure, pane );
  	  }
	}
      break;
    case view_LeftMovement:
      if ( CurrentFigure )
	{
	(this->view_object)->Draw_Figure(  CurrentFigure, pane );
        (this)->Set_Object_Point(  CurrentFigure,	zip_figure_auxiliary_point, X, Y );
	(this->view_object)->Draw_Figure(  CurrentFigure, pane );
	}
      break;
    default:
      break;
    }
  OUT(ziporect::Build_Object);
  return  status;
  }

long
ziporect::Draw_Object( zip_type_figure		   figure, zip_type_pane		   pane )
        {
  long				  status = zip_ok;

  IN(ziporect::Draw_Object);
  if ( (this->view_object)->Condition(  pane, figure, zip_draw ) )
    status = Draw( this, figure, pane, zip_draw );
  OUT(ziporect::Draw_Object);
  return  status;
  }

long
ziporect::Clear_Object( zip_type_figure		   figure, zip_type_pane		   pane )
        {
  long				  status = zip_ok;

  IN(ziporect::Clear_Object);
  if ( (this->view_object)->Condition(  pane, figure, zip_clear ) )
    status = Draw( this, figure, pane, zip_clear );
  OUT(ziporect::Clear_Object);
  return  status;
  }

static
long Draw( class ziporect *self, zip_type_figure  figure, zip_type_pane  pane, short   action )
{
    long				  status = zip_ok,
    left = window_x_point, top = window_y_point,
    width = window_x_points(0) - left,
    height = window_y_points(0) - top;
    unsigned char		  pattern = 0, shade;

    IN(Draw);
    if ( width < 0 )
    { left = window_x_points(0);  width = -width; }
    if ( height < 0 )
    { top = window_y_points(0);   height = -height;  }
    if ( (figure->zip_figure_mode.zip_figure_mode_shaded  ||
	  figure->zip_figure_mode.zip_figure_mode_patterned) ) {
	if ( self->view_object->mouse_action != view_LeftMovement  &&  action == zip_draw ) {
	    if ( figure->zip_figure_mode.zip_figure_mode_patterned  &&
		(pattern = (self->data_object)->Contextual_Figure_Pattern(  figure )) )
	    { DEBUGct(Pattern,pattern);
	    (self->view_object)->FillTrapezoid( left, top, width, left, top+height+1, width+1,
		(self->view_object)->Define_Graphic( (self->view_object)->Select_Contextual_Figure_Font( figure ), pattern ) );
	    }
	    else
	/* Shade of '0' means Transparent --- Shade of '1' means White */
		if ( (shade = (self->data_object)->Contextual_Figure_Shade(  figure )) >= 1  &&
		    shade <= 100 )
		{ DEBUGdt(Shade,shade);
		if ( (shade = ('0' + ((shade + 10) / 10)) - 1) > '9' )  shade = '9';
		DEBUGdt(Shade-index,shade);
		(self->view_object)->Ensure_Fill_Attributes(  figure );
		(self->view_object)->FillRectSize( left, top, width+1, height+1,
			(self->view_object)->Define_Graphic( (self->data_object)->Define_Font(  ShadeFontName, NULL ), shade ));
		}
	}
	else
	    if ( action == zip_clear )
	    { DEBUG(Clear Action);
	    (self->view_object)->EraseRectSize(  left+1, top+1, width-1, height-1 );
	    }
    }
    if ( (self->view_object)->Ensure_Line_Attributes(  figure ) == zip_ok )
	(self->view_object)->DrawRectSize(  left, top, width, height );
    if ( ExposePoints )
	(self)->Expose_Object_Points(  figure, pane );
    if ( HighlightPoints )
	(self)->Highlight_Object_Points(  figure, pane );
    OUT(Draw);
    return  status;
}

long
ziporect::Print_Object( zip_type_figure figure, zip_type_pane pane )
{
  class ziporect *self=this;
  long				  status = zip_ok;
  long				  left, right, top, bottom;

  IN(ziporect::Print_Object);
  left = print_x_point;       top = print_y_point;
  right = print_x_points(0);  bottom = print_y_points(0);
  if ( right < left )
    { DEBUG(X-Flipped);
    left = right;
    right = print_x_point;
    }
  if ( bottom < top )
    { DEBUG(Y-Flipped);
    top = bottom;
    bottom = print_y_point;
    }
  (this->print_object)->Set_Shade(  figure->zip_figure_fill.zip_figure_shade ); 
  (this->print_object)->Ensure_Line_Attributes(  figure );
  (this->print_object)->Draw_Rectangle(  left, top, right, bottom );
  OUT(ziporect::Print_Object);
  return  status;
  }

long
ziporect::Proximate_Object_Points( zip_type_figure		   figure, zip_type_pane		   pane, zip_type_pixel		   x , zip_type_pixel		   y )
          {
  int				  point = 0;
  zip_type_pixel			  X1, X2, X3, Y1, Y2, Y3;

  IN(ziporect::Proximate_Object_Points);
  Compute_Handle_Positions( this, figure, pane, &X1, &X2, &X3, &Y1, &Y2, &Y3 );
  if ( (this->view_object)->Proximate_Figure_Point(  pane, figure, X1, Y1, x, y ) )
    point = zip_figure_origin_point;	    /* Upper Left Corner */
  else
  if ( (this->view_object)->Proximate_Figure_Point(  pane, figure, X3, Y3, x, y ) )
    point = zip_figure_auxiliary_point;	    /* Lower Right Corner */
  else
  if ( (this->view_object)->Proximate_Figure_Point(  pane, figure, X3, Y1, x, y ) )
    point = zip_figure_auxiliary_point + 1; /* Upper Right Corner */
  else
  if ( (this->view_object)->Proximate_Figure_Point(  pane, figure, X1, Y3, x, y ) )
    point = zip_figure_auxiliary_point + 2; /* Lower Left Corner */
  else
  if ( (this->view_object)->Proximate_Figure_Point(  pane, figure, X2, Y1, x, y ) )
    point = zip_figure_auxiliary_point + 3; /* 12 O'Clock */
  else
  if ( (this->view_object)->Proximate_Figure_Point(  pane, figure, X3, Y2, x, y ) )
    point = zip_figure_auxiliary_point + 4; /* 3 O'Clock */
  else
  if ( (this->view_object)->Proximate_Figure_Point(  pane, figure, X2, Y3, x, y ) )
    point = zip_figure_auxiliary_point + 5; /* 6 O'Clock */
  else
  if ( (this->view_object)->Proximate_Figure_Point(  pane, figure, X1, Y2, x, y ) )
    point = zip_figure_auxiliary_point + 6; /* 9 O'Clock */
  else
  if ( (this->view_object)->Proximate_Figure_Point(  pane, figure, X2, Y2, x, y ) )
    point = zip_figure_auxiliary_point + 7; /* Center */
  OUT(ziporect::Proximate_Object_Points);
  return  point;
  }

long
ziporect::Within_Object( zip_type_figure		   figure, zip_type_pane		   pane, zip_type_pixel		   x , zip_type_pixel		   y )
          {
  long				  distance = -1,
					  X1, Y1, X2, Y2,
					  X, Y, XA, XB, YA, YB;

  IN(ziporect::Within_Object);
  X1 = window_x_point;	    Y1 = window_y_point;
  X2 = window_x_points(0);  Y2 = window_y_points(0);
  if ( x >= X1  &&  x <= X2  &&  y >= Y1  &&  y <= Y2 )
    {
    if ( (XA = abs(x - X1)) < (XB = abs(x - X2)) )
      X = XA;
      else
      X = XB;
    if ( (YA = abs(y - Y1)) < (YB = abs(y - Y2)) )
      Y = YA;
      else
      Y = YB;
    distance = (X * X) + (Y * Y);
    }
  OUT(ziporect::Within_Object);
  return  distance;
  }

boolean
ziporect::Enclosed_Object( zip_type_figure		   figure, zip_type_pane		   pane, zip_type_pixel		   x , zip_type_pixel		   y , zip_type_pixel		   w , zip_type_pixel		   h )
          {
  boolean			  enclosed = false;
  zip_type_pixel		  X1, Y1, X2, Y2;

  IN(ziporect::Enclosed_Object);
  X1 = window_x_point;	    Y1 = window_y_point;
  X2 = window_x_points(0);  Y2 = window_y_points(0);
  if ( X1 > x  &&  Y1 > y  &&  X2 < (x + w)  &&  Y2 < (y + h) )
    enclosed = true;
  OUT(ziporect::Enclosed_Object);
  return  enclosed;
  }

long
ziporect::Object_Enclosure( zip_type_figure		   figure, zip_type_pane		   pane, zip_type_pixel		  *x , zip_type_pixel		  *y , zip_type_pixel		  *w , zip_type_pixel		  *h )
          {
  zip_type_pixel			  X1, X2, X3, Y1, Y2, Y3;

  IN(ziporect::Object_Enclosure);
  Compute_Handle_Positions( this, figure, pane, &X1, &X2, &X3, &Y1, &Y2, &Y3 );
  *x = X1;  *y = Y1;  *w = abs(X3 - X1);  *h = abs(Y3 - Y1);
  OUT(ziporect::Object_Enclosure);
  return  zip_ok;
  }

long
ziporect::Highlight_Object_Points( zip_type_figure		   figure, zip_type_pane		   pane )
        {
  zip_type_pixel			  X1, X2, X3, Y1, Y2, Y3;
  long				  status = zip_ok;

  IN(ziporect::Highlight_Object_Points);
  Compute_Handle_Positions( this, figure, pane, &X1, &X2, &X3, &Y1, &Y2, &Y3 );
  (this->edit_object)->Highlight_Handles(  pane, X1, X2, X3, Y1, Y2, Y3 );
  OUT(ziporect::Highlight_Object_Points);
  return  status;
  }

long
ziporect::Normalize_Object_Points( zip_type_figure		   figure, zip_type_pane		   pane )
        {
  zip_type_pixel			  X1, X2, X3, Y1, Y2, Y3;
  long				  status = zip_ok;

  IN(ziporect::Normalize_Object_Points);
  Compute_Handle_Positions( this, figure, pane, &X1, &X2, &X3, &Y1, &Y2, &Y3 );
  (this->edit_object)->Normalize_Handles(  pane, X1, X2, X3, Y1, Y2, Y3 );
  OUT(ziporect::Normalize_Object_Points);
  return  status;
  }

long
ziporect::Expose_Object_Points( zip_type_figure		   figure, zip_type_pane		   pane )
        {
  long				  status = zip_ok;

  IN(ziporect::Expose_Object_Points);
  (this->edit_object)->Expose_Point(  pane, figure, figure_x_point,     figure_y_point );
  (this->edit_object)->Expose_Point(  pane, figure, figure_x_points(0), figure_y_point );
  (this->edit_object)->Expose_Point(  pane, figure, figure_x_points(0), figure_y_points(0) );
  (this->edit_object)->Expose_Point(  pane, figure, figure_x_point,     figure_y_points(0) );
  OUT(ziporect::Expose_Object_Points);
  return  status;
  }

long
ziporect::Hide_Object_Points( zip_type_figure		   figure, zip_type_pane		   pane )
        {
  long				  status = zip_ok;

  IN(ziporect::Hide_Object_Points);
  (this->edit_object)->Hide_Point(  pane, figure, figure_x_point,     figure_y_point );
  (this->edit_object)->Hide_Point(  pane, figure, figure_x_points(0), figure_y_point );
  (this->edit_object)->Hide_Point(  pane, figure, figure_x_points(0), figure_y_points(0) );
  (this->edit_object)->Hide_Point(  pane, figure, figure_x_point,     figure_y_points(0) );
  OUT(ziporect::Hide_Object_Points);
  return  status;
  }

long
ziporect::Set_Object_Point( zip_type_figure		   figure, int				   point, zip_type_point		   x , zip_type_point		   y )
          {
  long				  status = zip_ok;

  IN(ziporect::Set_Object_Point);
  if ( figure->zip_figure_points == NULL  &&
       (status = (this->data_object)->Allocate_Figure_Points_Vector(
		     &figure->zip_figure_points )) == zip_ok )
    {
    figure->zip_figure_points->zip_points_count = 1;
    }
  if ( status == zip_ok )
    {
    switch ( point )
      {
      case zip_figure_origin_point:
        figure_x_point = x;
        figure_y_point = y;
        break;
      case  zip_figure_auxiliary_point:
        figure_x_points(0) = x;
        figure_y_points(0) = y;
	break;
      case zip_figure_auxiliary_point + 1: /* Upper Right Corner */
        figure_x_points(0) = x;
	figure_y_point = y;
        break;
      case zip_figure_auxiliary_point + 2: /* Lower Left Corner */
        figure_x_point = x;
	figure_y_points(0) = y;
	break;
      case zip_figure_auxiliary_point + 3: /* 12 O'Clock */
	figure_y_points(0) -= y - figure_y_point;
	figure_y_point = y;
	break;
      case zip_figure_auxiliary_point + 4: /* 3 O'Clock */
	figure_x_point -= x - figure_x_points(0);
	figure_x_points(0) = x;
	break;
      case zip_figure_auxiliary_point + 5: /* 6 O'Clock */
	figure_y_point += figure_y_points(0) - y;
	figure_y_points(0) = y;
	break;
      case zip_figure_auxiliary_point + 6: /* 9 O'Clock */
	figure_x_points(0) += figure_x_point - x;
	figure_x_point = x;
	break;
      case zip_figure_auxiliary_point + 7: /* Center */
	(this)->Adjust_Object_Point_Suite(  figure,
	    x - (figure_x_point + (figure_x_points(0) - figure_x_point)/2),
	    y - (figure_y_point - (figure_y_point - figure_y_points(0))/2) );
	break;
      default: status = zip_failure; /*=== zip_invalid_point_type ===*/
      }
    if ( status == zip_ok )
      {
      (this->data_object)->Set_Image_Extrema(  figure->zip_figure_image, x, y );
/*===handle both extrema 7/20/86===*/
/*===have extrema check for REDUCTIONS as well as EXPANSIONS 7/20/86===*/
      (this->data_object)->Set_Stream_Extrema(  figure->zip_figure_image->zip_image_stream,
			      figure->zip_figure_image );
      }
    }
  OUT(ziporect::Set_Object_Point);
  return  status;
  }

long/*=== MUST RATIONALIZE POINTS VS HANDLES ===*/
ziporect::Object_Point( zip_type_figure figure, long point , long *x , long	*y )
{
  long				  status = zip_ok;

  *x = 0;
  *y = 0;
  switch( point )
    {
    case 1:  *x = figure_x_point; *y = figure_y_point;      break;
    default: status = zip_failure;
    }
  return  status;
  }

long
ziporect::Adjust_Object_Point_Suite( zip_type_figure		   figure, zip_type_point		   x_delta , zip_type_point		   y_delta )
        {
  long				  status = zip_ok;

  IN(ziporect::Adjust_Object_Point_Suite);
  figure_x_point += x_delta;
  figure_y_point += y_delta;
  figure_x_points(0) += x_delta;
  figure_y_points(0) += y_delta;
  (this->data_object)->Set_Image_Extrema(  figure->zip_figure_image, figure_x_point, figure_y_point );
  (this->data_object)->Set_Image_Extrema(  figure->zip_figure_image, figure_x_points(0), figure_y_points(0) );
/*===have extrema check for REDUCTIONS as well as EXPANSIONS 7/20/86===*/
  (this->data_object)->Set_Stream_Extrema(  figure->zip_figure_image->zip_image_stream,
			    figure->zip_figure_image );
  OUT(ziporect::Adjust_Object_Point_Suite);
  return  status;
  }

static
void Compute_Handle_Positions( class ziporect *self, zip_type_figure  figure, zip_type_pane  pane, zip_type_pixel *X1 , zip_type_pixel *X2 , zip_type_pixel *X3, zip_type_pixel *Y1 , zip_type_pixel *Y2 , zip_type_pixel *Y3 )
{
    *X1 = (self->view_object)->X_Point_To_Pixel(  pane, figure, figure_x_point );
    *X2 = *X1 + (window_x_points(0) - window_x_point)/2;
    *X3 = (self->view_object)->X_Point_To_Pixel(  pane, figure, figure_x_points(0) );
    *Y1 = (self->view_object)->Y_Point_To_Pixel(  pane, figure, figure_y_point );
    *Y2 = *Y1 + (window_y_points(0) - window_y_point)/2;
    *Y3 = (self->view_object)->Y_Point_To_Pixel(  pane, figure, figure_y_points(0) );
}


long
ziporect::Set_Object_Shade( zip_type_figure figure, unsigned char shade )
{
    IN(ziporect::Set_Object_Shade);
    figure->zip_figure_fill.zip_figure_shade = shade;
    if ( shade >= 1  &&  shade <= 100 )
	figure->zip_figure_mode.zip_figure_mode_shaded = on;
    else
	figure->zip_figure_mode.zip_figure_mode_shaded = off;
    OUT(ziporect::Set_Object_Shade);
    return  zip_ok;
}

boolean
ziporect::Contains( zip_type_figure figure, zip_type_pane pane, zip_type_pixel x , zip_type_pixel y )
{
    boolean			status = FALSE;
    int				x1, y1, x2, y2;

    IN( ziporect::Contains )
    x1 = window_x_point;
    y1 = window_y_point;
    x2 = window_x_points(0);
    y2 = window_y_points(0);
    if ( figure->zip_figure_mode.zip_figure_mode_shaded )
    {
	if ( y1 <= y && y <= y2 && x1 <= x && x <= x2 )
	    status = TRUE;
    }
    else if ((( x == x1 || x == x2 ) && y1 <= y && y <= y2 ) ||
	     (( y == y1 || y == y2 ) && x1 <= x && x <= x2 ))
	status = TRUE;
    OUT( ziporect::Contains )
      return status;
}
