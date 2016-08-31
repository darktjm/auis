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

/* zipoelli.c	Zip Object -- Ellipse					      */
/* Author	TC Peters						      */
/* Information Technology Center		   Carnegie-Mellon University */

/**  SPECIFICATION -- Internal Facility Suite  *********************************

TITLE	Zip Object -- Ellipse

MODULE	zipoelli.c

NOTICE	IBM Internal Use Only

DESCRIPTION

.......................

HISTORY
  04/13/88	Created (TC Peters)
  05/01/89	Use symbolic font-names (TCP)
  08/24/89	Remove excess SetTransferMode() activity in Draw() (SCG)
  08/25/89	Modify Build to handle non-refresh of pane on build completion (SCG)
   08/16/90	Add Ensure_Attribute usage on Draw and Print (SCG)

END-SPECIFICATION  ************************************************************/


#include <andrewos.h>
#include <view.H>
#include "zip.H"
#include "zipview.H"
#include "zipedit.H"
#include "zipprint.H"
#include "zipobject.H"
#include "zipoelli.H"

ATKdefineRegistry(zipoelli, zipocirc, NULL);

static long Draw( class zipoelli		  *self, zip_type_figure		   figure, zip_type_pane		   pane, long				   action );
static void Compute_Handle_Positions( class zipoelli		  *self, zip_type_figure		   figure, zip_type_pane		   pane, zip_type_pixel		  *X1 , zip_type_pixel		  *X2 , zip_type_pixel		  *X3 , zip_type_pixel		  *Y1 , zip_type_pixel		  *Y2 , zip_type_pixel		  *Y3 );


char
zipoelli::Object_Icon( )
    {
  IN(zipoelli::Object_Icon);
  OUT(zipoelli::Object_Icon);
  return  'K';
  }

char
zipoelli::Object_Icon_Cursor( )
    {
  IN(zipoelli::Object_Icon_Cursor);
  OUT(zipoelli::Object_Icon_Cursor);
  return  'H';
  }

char
zipoelli::Object_Datastream_Code( )
    {
  IN(zipoelli::Object_Datastream_Code);
  OUT(zipoelli::Object_Datastream_Code);
  return  'L';
  }

long
zipoelli::Show_Object_Properties( zip_type_pane		   pane, zip_type_figure		   figure )
        {
  (this->view_object)->Announce(  "Draw Ellipse from Center outward." );
  return  zip_ok;
  }

long
zipoelli::Build_Object( zip_type_pane		   pane, enum view_MouseAction				   action , long				   x , long				   y , long				   clicks, zip_type_point		   X , zip_type_point		   Y )
          {
  long				  status = zip_ok;
  zip_type_figure		  figure;

  IN(zipoelli::Build_Object);
  switch ( action )
    {
    case view_LeftDown:
      if ( (status =
	(this->data_object)->Create_Figure(  &pane->zip_pane_current_figure, NULL, zip_ellipse_figure,
			   pane->zip_pane_current_image, NULL )) == zip_ok )
        {
        (this)->Set_Object_Point(  pane->zip_pane_current_figure,
		zip_figure_origin_point, X, Y );
	pane->zip_pane_current_figure->zip_figure_zoom_level =
	    pane->zip_pane_zoom_level;
	pane->zip_pane_edit->zip_pane_edit_last_point_id = zip_figure_auxiliary_point;
	(this->data_object)->Set_Figure_Shade(  CurrentFigure,
			      pane->zip_pane_edit->zip_pane_edit_current_shade );
	(this->view_object)->Set_Pane_Painting_Mode(  pane, zipview_paint_inverted );
	}
      break;
    case view_LeftUp:
      if ( figure = pane->zip_pane_current_figure )
        {
	(this->view_object)->Set_Pane_Painting_Mode(  pane, zip_default );
	if ( figure_x_points(0) == 0  ||  figure_y_points(0) == 0 )
	  {
	  (this->edit_object)->Delete_Figure(  figure, pane );

	  }
          else
          {
          (this->view_object)->Draw_Figure(  figure, pane );
	  }
	}
        break;
    case view_LeftMovement:
      if ( figure = pane->zip_pane_current_figure )
	{
	(this->view_object)->Draw_Figure(  figure, pane );
	if ( X == figure->zip_figure_point.zip_point_x )
	  X += (Y - figure_y_point)/2;
        (this)->Set_Object_Point(  figure, zip_figure_auxiliary_point, X, 0 );
	if ( Y == figure_y_point )
	  Y += (X - figure_x_point)/2;
        (this)->Set_Object_Point(  figure, zip_figure_auxiliary_point + 1, 0, Y );
	(this->view_object)->Draw_Figure(  figure, pane );
	}
      break;
    }
  OUT(zipoelli::Build_Object);
  return  status;
  }

long
zipoelli::Read_Object( zip_type_figure		   figure )
      {
  long				  status = zip_ok;

  IN(zipoelli::Read_Object);
  status = (this->data_object)->Read_Figure(  figure );
  (this->data_object)->Set_Image_Extrema(  figure->zip_figure_image,
			 figure_x_point - abs(figure_x_points(0)),
			 figure_y_point + abs(figure_y_points(0)));
  (this->data_object)->Set_Image_Extrema(  figure->zip_figure_image,
			 figure_x_point + abs(figure_x_points(0)),
			 figure_y_point - abs(figure_y_points(0)));
  (this->data_object)->Set_Stream_Extrema(  figure->zip_figure_image->zip_image_stream,
			  figure->zip_figure_image );
  OUT(zipoelli::Read_Object);
  return  status;
  }

long
zipoelli::Write_Object( zip_type_figure		   figure )
      {
  long				  status = zip_ok;

  IN(zipoelli::Write_Object);
  status = (this->data_object)->Write_Figure(  figure );
  OUT(zipoelli::Write_Object);
  return  status;
  }

long
zipoelli::Draw_Object( zip_type_figure		   figure, zip_type_pane		   pane )
        {
  long				  status = zip_ok;

  IN(zipoelli::Draw_Object);
  if ( (this->view_object)->Condition(  pane, figure, zip_draw ) )
    status = Draw( this, figure, pane, zip_draw );
  OUT(zipoelli::Draw_Object);
  return  status;
  }

long
zipoelli::Clear_Object( zip_type_figure		   figure, zip_type_pane		   pane )
        {
  long				  status = zip_ok;

  IN(zipoelli::Clear_Object);
  if ( (this->view_object)->Condition(  pane, figure, zip_clear ) )
    status = Draw( this, figure, pane, zip_clear );
  OUT(zipoelli::Clear_Object);
  return  status;
  }

static
long Draw( class zipoelli		  *self, zip_type_figure		   figure, zip_type_pane		   pane, long				   action )
          {
  long				  status = zip_ok;
  int				  x_start_end, minor_radius, major_radius;
  unsigned char		  shade;

  IN(Draw);
  x_start_end = (self->view_object)->X_Point_To_Pixel(  pane, figure,
                                          figure_x_point + figure_x_points(0) );
  minor_radius = abs( x_start_end - window_x_point );
  major_radius = abs( (self->view_object)->Y_Point_To_Pixel(  pane, figure,
            	              figure_y_point + figure_y_points(0)) - window_y_point );
  if ( figure->zip_figure_mode.zip_figure_mode_shaded )
    { DEBUGdt(Shade,figure->zip_figure_fill.zip_figure_shade);
    if ( self->view_object->mouse_action != view_LeftMovement  &&  action == zip_draw )
      {
      /* Shade of '0' means Transparent --- Shade of '1' means White */
      if ( (shade = figure->zip_figure_fill.zip_figure_shade) >= 1  &&
	    shade <= 100 )
	{
	if ( (shade = ('0' + ((shade + 10) / 10)) - 1) > '9' )  shade = '9';
	DEBUGdt(Shade-index,shade);
	(self->view_object)->Ensure_Fill_Attributes(  figure );
	(self->view_object)->FillOvalSize(  window_x_point - minor_radius, window_y_point - major_radius,
				(minor_radius << 1)+1, (major_radius << 1)+1,
				(self->view_object)->Define_Graphic( (self->data_object)->Define_Font(
				     ShadeFontName, NULL ), shade ) );
	}
      }
      else
      if ( action == zip_clear )
	{
	(self->view_object)->FillOvalSize(  window_x_point - minor_radius, window_y_point - major_radius,
			      (minor_radius << 1)+1, (major_radius << 1)+1, graphic_WHITE );
	}
    }
  if ( (self->view_object)->Ensure_Line_Attributes(  figure ) == zip_ok )
    (self->view_object)->DrawOvalSize(  window_x_point - minor_radius, window_y_point - major_radius, minor_radius << 1, major_radius << 1 );
  if ( ExposePoints )	    (self)->Expose_Object_Points(  figure, pane );
  if ( HighlightPoints )    (self)->Highlight_Object_Points(  figure, pane );
  OUT(Draw);
  return  status;
  }

long
zipoelli::Print_Object( zip_type_figure  figure, zip_type_pane  pane )
{
  class zipoelli *self=this;
  long				  status = zip_ok;

  IN(zipoelli::Print_Object);
  (this->print_object)->Set_Shade(  figure->zip_figure_fill.zip_figure_shade ); 
  (this->print_object)->Ensure_Line_Attributes(  figure );
  (this->print_object)->Draw_Ellipse(  print_x_point, print_y_point,
			print_x_lengths(0), print_y_lengths(0) );
  OUT(zipoelli::Print_Object);
  return  status;
  }

long
zipoelli::Proximate_Object_Points( zip_type_figure		   figure, zip_type_pane		   pane, zip_type_pixel		   x , zip_type_pixel		   y )
          {
  int				  point = 0;
  zip_type_pixel			  X1, X2, X3, Y1, Y2, Y3;

  IN(zipoelli::Proximate_Object_Points);
  Compute_Handle_Positions( this, figure, pane, &X1, &X2, &X3, &Y1, &Y2, &Y3 );
  if ( (this->view_object)->Proximate_Figure_Point(  pane, figure, X2, Y2, x, y ) )
    point = zip_figure_origin_point;	    /* Origin */
  else
  if ( (this->view_object)->Proximate_Figure_Point(  pane, figure, X3, Y2, x, y ) )
    point = zip_figure_auxiliary_point;     /* 3 O'Clock */
  else
  if ( (this->view_object)->Proximate_Figure_Point(  pane, figure, X2, Y3, x, y ) )
    point = zip_figure_auxiliary_point + 1; /* 6 O'Clock */
  else
  if ( (this->view_object)->Proximate_Figure_Point(  pane, figure, X1, Y2, x, y ) )
    point = zip_figure_auxiliary_point + 2; /* 9 O'Clock */
  else
  if ( (this->view_object)->Proximate_Figure_Point(  pane, figure, X2, Y1, x, y ) )
    point = zip_figure_auxiliary_point + 3; /* 12 O'Clock */
  else
  if ( (this->view_object)->Proximate_Figure_Point(  pane, figure, X1, Y1, x, y ) )
    point = zip_figure_auxiliary_point + 4; /* Upper-Left */
  else
  if ( (this->view_object)->Proximate_Figure_Point(  pane, figure, X3, Y1, x, y ) )
    point = zip_figure_auxiliary_point + 5; /* Upper-Right */
  else
  if ( (this->view_object)->Proximate_Figure_Point(  pane, figure, X3, Y3, x, y ) )
    point = zip_figure_auxiliary_point + 6; /* Lower-Right */
  else
  if ( (this->view_object)->Proximate_Figure_Point(  pane, figure, X1, Y3, x, y ) )
    point = zip_figure_auxiliary_point + 7; /* Lower-Left */

  OUT(zipoelli::Proximate_Object_Points);
  return  point;
  }

boolean
zipoelli::Enclosed_Object( zip_type_figure		   figure, zip_type_pane		   pane, zip_type_pixel		   x , zip_type_pixel		   y , zip_type_pixel		   w , zip_type_pixel		   h )
          {
  boolean			  enclosed = false;
  zip_type_pixel			  X1, X2, X3, Y1, Y2, Y3;

  IN(zipoelli::Enclosed_Object);
  Compute_Handle_Positions( this, figure, pane, &X1, &X2, &X3, &Y1, &Y2, &Y3 );
  if ( X1 > x  &&  Y1 > y  &&  X3 < (x + w)  &&  Y3 < (y + h) )
    enclosed = true;
  OUT(zipoelli::Enclosed_Object);
  return  enclosed;
  }

long
zipoelli::Object_Enclosure( zip_type_figure		   figure, zip_type_pane		   pane, zip_type_pixel		  *x , zip_type_pixel		  *y , zip_type_pixel		  *w , zip_type_pixel		  *h )
          {
  zip_type_pixel			  X1, X2, X3, Y1, Y2, Y3;

  IN(zipoelli::Object_Enclosure);
  Compute_Handle_Positions( this, figure, pane, &X1, &X2, &X3, &Y1, &Y2, &Y3 );
  *x = X1;  *y = Y1;  *w = abs(X3 - X1);  *h = abs(Y3 - Y1);
  OUT(zipoelli::Object_Enclosure);
  return  zip_ok;
  }

long
zipoelli::Highlight_Object_Points( zip_type_figure		   figure, zip_type_pane		   pane )
        {
  zip_type_pixel			  X1, X2, X3, Y1, Y2, Y3;
  long				  status = zip_ok;

  IN(zipoelli::Highlight_Object_Points);
  Compute_Handle_Positions( this, figure, pane, &X1, &X2, &X3, &Y1, &Y2, &Y3 );
  (this->edit_object)->Highlight_Handles(  pane, X1, X2, X3, Y1, Y2, Y3 );
  OUT(zipoelli::Highlight_Object_Points);
  return  status;
  }

long
zipoelli::Normalize_Object_Points( zip_type_figure		   figure, zip_type_pane		   pane )
        {
  zip_type_pixel			  X1, X2, X3, Y1, Y2, Y3;
  long				  status = zip_ok;

  IN(zipoelli::Normalize_Object_Points);
  Compute_Handle_Positions( this, figure, pane, &X1, &X2, &X3, &Y1, &Y2, &Y3 );
  (this->edit_object)->Normalize_Handles(  pane, X1, X2, X3, Y1, Y2, Y3 );
  OUT(zipoelli::Normalize_Object_Points);
  return  status;
  }

long
zipoelli::Expose_Object_Points( zip_type_figure		   figure, zip_type_pane		   pane )
        {
  long				  status = zip_ok;

  IN(zipoelli::Expose_Object_Points);
  (this->edit_object)->Expose_Point(  pane, figure, figure_x_point, figure_y_point );
  (this->edit_object)->Expose_Point(  pane, figure, figure_x_point + figure_x_points(0), figure_y_point );
  (this->edit_object)->Expose_Point(  pane, figure, figure_x_point, figure_y_point + figure_x_points(0) );
  (this->edit_object)->Expose_Point(  pane, figure, figure_x_point - figure_x_points(0), figure_y_point );
  (this->edit_object)->Expose_Point(  pane, figure, figure_x_point, figure_y_point - figure_x_points(0) );

  OUT(zipoelli::Expose_Object_Points);
  return  status;
  }

long
zipoelli::Hide_Object_Points( zip_type_figure		   figure, zip_type_pane		   pane )
        {
  long				  status = zip_ok;

  IN(zipoelli::Hide_Points);
  (this->edit_object)->Hide_Point(  pane, figure, figure_x_point, figure_y_point );
  (this->edit_object)->Hide_Point(  pane, figure, figure_x_point + figure_x_points(0), figure_y_point );
  (this->edit_object)->Hide_Point(  pane, figure, figure_x_point, figure_y_point + figure_x_points(0) );
  (this->edit_object)->Hide_Point(  pane, figure, figure_x_point - figure_x_points(0), figure_y_point );
  (this->edit_object)->Hide_Point(  pane, figure, figure_x_point, figure_y_point - figure_x_points(0) );
  OUT(zipoelli::Hide_Points);
  return  status;
  }

long
zipoelli::Set_Object_Point( zip_type_figure		   figure, int				   point, zip_type_point		   x , zip_type_point		   y )
          {
  long				  status = zip_ok;
  zip_type_point		  x_radius, y_radius, Rx, Ry,
					  X1, X2, X3,
					  Y1, Y2, Y3;

  IN(zipoelli::Set_Object_Point);
  if ( figure->zip_figure_points == NULL  &&
       (status = (this->data_object)->Allocate_Figure_Points_Vector(  &figure->zip_figure_points )) == zip_ok )
    {
    figure->zip_figure_points->zip_points_count = 1;
    figure_x_point = x;
    figure_y_point = y;
    figure_x_points(0) = figure_y_points(0) = 0;
    }
  x_radius = abs( x - figure_x_point );
  y_radius = abs( y - figure_y_point );
  Rx = figure_x_points(0);
  Ry = figure_y_points(0);
  X2 = figure_x_point;
  X1 = X2 - Rx;
  X3 = X2 + Rx;
  Y2 = figure_y_point;
  Y1 = Y2 + Ry;
  Y3 = Y2 - Ry;
  if ( status == zip_ok )
    switch ( point )
      {
      case zip_figure_origin_point:
        figure_x_point = x;
        figure_y_point = y;
	break;
      case zip_figure_auxiliary_point:	    /*  3 O'Clock */
      case zip_figure_auxiliary_point + 2:  /*  9 O'Clock */
	figure_x_points(0) = x_radius;
	break;
      case zip_figure_auxiliary_point + 1:  /*  6 O'Clock */
      case zip_figure_auxiliary_point + 3:  /* 12 O'Clock */
	figure_y_points(0) = y_radius;
	break;
      case zip_figure_auxiliary_point + 4:  /* Upper-Left */
	figure_x_point = X3 - x_radius;
	figure_y_point = Y3 + y_radius;
	figure_x_points(0) = x_radius;
	figure_y_points(0) = y_radius;
        break;
      case zip_figure_auxiliary_point + 5:  /* Upper-Right */
	figure_x_point = X1 + x_radius;
	figure_y_point = Y3 + y_radius;
	figure_x_points(0) = x_radius;
	figure_y_points(0) = y_radius;
	break;
      case zip_figure_auxiliary_point + 6:  /* Lower-Right */
	figure_x_point = X1 + x_radius;
	figure_y_point = Y1 - y_radius;
	figure_x_points(0) = x_radius;
	figure_y_points(0) = y_radius;
	break;
      case zip_figure_auxiliary_point + 7:  /* Lower-Left */
	figure_x_point = X3 - x_radius;
	figure_y_point = Y1 - y_radius;
	figure_x_points(0) = x_radius;
	figure_y_points(0) = y_radius;
	break;
      default:
	status = zip_failure; /*=== zip_invalid_point_position ===*/
      }
/*=== SET EXTREMA === */

  OUT(zipoelli::Set_Object_Point);
  return  status;
  }

long
zipoelli::Adjust_Object_Point_Suite( zip_type_figure		   figure, zip_type_point		   x_delta , zip_type_point		   y_delta )
        {
  long				  status = zip_ok;

  IN(zipoelli::Adjust_Object_Point_Suite);
  figure_x_point += x_delta;
  figure_y_point += y_delta;
  (this->data_object)->Set_Image_Extrema(  figure->zip_figure_image, figure_x_point, figure_y_point );
  (this->data_object)->Set_Stream_Extrema(  figure->zip_figure_image->zip_image_stream,
			    figure->zip_figure_image );
  OUT(zipoelli::Adjust_Object_Point_Suite);
  return  status;
  }

static
void Compute_Handle_Positions( class zipoelli		  *self, zip_type_figure		   figure, zip_type_pane		   pane, zip_type_pixel		  *X1 , zip_type_pixel		  *X2 , zip_type_pixel		  *X3 , zip_type_pixel		  *Y1 , zip_type_pixel		  *Y2 , zip_type_pixel		  *Y3 )
          {
  *X1 = (self->view_object)->X_Point_To_Pixel(  pane, figure, figure_x_point - figure_x_points(0) );
  *X2 = window_x_point;
  *X3 = (self->view_object)->X_Point_To_Pixel(  pane, figure, figure_x_point + figure_x_points(0) );
  *Y1 = (self->view_object)->Y_Point_To_Pixel(  pane, figure, figure_y_point + figure_y_points(0) );
  *Y2 = window_y_point;
  *Y3 = (self->view_object)->Y_Point_To_Pixel(  pane, figure, figure_y_point - figure_y_points(0) );
  }
