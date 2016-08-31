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

/* zipoarc.c	Zip Object -- Arc					      */
/* Author	TC Peters						      */
/* Information Technology Center		   Carnegie-Mellon University */

/**  SPECIFICATION -- Internal Facility Suite  *********************************

TITLE	Zip Object -- Arc

MODULE	zipoarc.c

NOTICE	IBM Internal Use Only

DESCRIPTION

.......................

HISTORY
  04/13/88	Created (TC Peters)
  11/18/88	Recognize Line-width (TCP)
   08/14/90	Use Ensure_Line_Attributes on Draw and Print (SCG)

END-SPECIFICATION  ************************************************************/

#include <andrewos.h>
#include <math.h>
#include "zip.H"
#include "zipview.H"
#include "zipobject.H"
#include "zipedit.H"
#include "zipprint.H"
#include "zipoarc.H"

/*LIBS: -lm
*/
static char two = 2; /* To quieten Compiler */

ATKdefineRegistry(zipoarc, zipobject, NULL);
static long Draw( class zipoarc		  *self, zip_type_figure		   figure, zip_type_pane		   pane );
static void Set_Points( zip_type_figure		   figure, float			   x_center , float			   y_center , float			   x_radius , float			   y_radius, 		float			   xs_delta , float			   ys_delta , float			   xe_delta , float			   ye_delta );
static void Compute_Handle_Positions( class zipoarc		  *self, zip_type_figure		   figure, zip_type_pane		   pane, zip_type_pixel		  *X1 , zip_type_pixel		  *X2 , zip_type_pixel		  *X3 , zip_type_pixel		  *Y1 , zip_type_pixel		  *Y2 , zip_type_pixel		  *Y3, zip_type_pixel		  *XS , zip_type_pixel		  *YS , zip_type_pixel		  *XE , zip_type_pixel		  *YE );


char
zipoarc::Object_Icon( )
    {
  IN(zipoarc::Object_Icon);
  OUT(zipoarc::Object_Icon);
  return  'L';
  }


char
zipoarc::Object_Icon_Cursor( )
    {
  IN(zipoarc::Object_Icon_Cursor);
  OUT(zipoarc::Object_Icon_Cursor);
  return  'P';
  }

char
zipoarc::Object_Datastream_Code( )
    {
  IN(zipoarc::Object_Datastream_Code);
  OUT(zipoarc::Object_Datastream_Code);
  return  'M';
  }

long
zipoarc::Show_Object_Properties( zip_type_pane		   pane, zip_type_figure		   figure )
        {
  (this->view_object)->Announce(  "Draw Arc Clockwise: Noon to 3:00, 3:00 to 6:00, etc." );
  return  zip_ok;
  }

long
zipoarc::Build_Object( zip_type_pane		   pane, enum view_MouseAction				   action , long				   x , long				   y , long				   clicks, zip_type_point		   X , zip_type_point		   Y )
          {
  long				  status = zip_ok;
  zip_type_figure		  figure;
  zip_type_point		  X_origin = 0, Y_origin = 0,
					  X_start, Y_start;

  IN(zipoarc::Build_Object);
  switch ( action )
    {
    case view_LeftDown:
      if ( (status =
	(this->data_object)->Create_Figure(  &pane->zip_pane_current_figure, NULL, zip_arc_figure,
			   pane->zip_pane_current_image, NULL )) == zip_ok )
        {
	figure = pane->zip_pane_current_figure;
        (this)->Set_Object_Point(  figure,
		zip_figure_origin_point, X, Y ); /* Center-point */
	figure->zip_figure_zoom_level = pane->zip_pane_zoom_level;
	pane->zip_pane_edit->zip_pane_edit_last_point_id = zip_figure_auxiliary_point;
	}
      break;
    case view_LeftUp:
      if ( figure = pane->zip_pane_current_figure )
        {
	if ( figure_x_points(two) == 0  ||  figure_y_points(two) == 0 )
	  {
	  (this->edit_object)->Delete_Figure(  figure, pane );
          break;
	  }
	}
      /* Fall-thru */
    case view_LeftMovement:
      if ( figure = pane->zip_pane_current_figure )
	{
	(this->view_object)->Set_Pane_Painting_Mode(  pane, zipview_paint_inverted );
	(this->view_object)->Draw_Figure(  figure, pane );
	X_start = figure_x_points(0);  Y_start = figure_y_points(0);
	if ( X_start < X  &&  Y_start < Y )
	  { X_origin = X;       Y_origin = Y_start; }
	else
	if ( X_start < X  &&  Y_start > Y )
	  { X_origin = X_start; Y_origin = Y; }
	else
	if ( X_start > X  &&  Y_start > Y )
	  { X_origin = X;       Y_origin = Y_start; }
	else
	if ( X_start > X  &&  Y_start < Y )
	  { X_origin = X_start; Y_origin = Y; }
	/* Origin */	figure_x_point = X_origin;
			figure_y_point = Y_origin;
	/* End */	figure_x_points(1) = X;
			figure_y_points(1) = Y;
	/* Radii */	figure_x_points(two) = abs(X_start - X);
			figure_y_points(two) = abs(Y_start - Y);
	(this->view_object)->Draw_Figure(  figure, pane );
	(this->view_object)->Set_Pane_Painting_Mode(  pane, zip_default );
	}
      break;
    }
  OUT(zipoarc::Build_Object);
  return  status;
  }

long
zipoarc::Draw_Object( zip_type_figure		   figure, zip_type_pane		   pane )
        {
  long				  status = zip_ok;

  IN(zipoarc::Draw_Object);
  if ( (this->view_object)->Condition(  pane, figure, zip_draw ) )
    status = Draw( this, figure, pane );
  OUT(zipoarc::Draw_Object);
  return  status;
  }

long
zipoarc::Clear_Object( zip_type_figure		   figure, zip_type_pane		   pane )
        {
  long				  status = zip_ok;

  IN(zipoarc::Clear_Object);
  if ( (this->view_object)->Condition(  pane, figure, zip_clear ) )
    status = Draw( this, figure, pane );
  OUT(zipoarc::Clear_Object);
  return  status;
  }

static
long Draw( class zipoarc		  *self, zip_type_figure		   figure, zip_type_pane		   pane )
        {
  long				  status = zip_ok;
  long				  x_radius, y_radius;
  long				  left, top, width, height;
  short			  start_angle, offset_angle;
  double			  theta, angle, x, y;
  unsigned char		  lwidth;

  IN(Draw); /*=== CLEAN UP THIS OLD MIGRATORY MESS... ===*/
  if ( (self->view_object)->Ensure_Line_Attributes(  figure ) == zip_ok )
    {
    x_radius = abs( (self->view_object)->X_Point_To_Pixel(  pane, figure,
     figure_x_point + figure_x_points(two) ) - window_x_point );
    y_radius = abs( (self->view_object)->Y_Point_To_Pixel(  pane, figure,
     figure_y_point + figure_y_points(two) ) - window_y_point );
    if ( x_radius  &&  y_radius )
      {
      left =  window_x_point - x_radius;
      top  =  window_y_point - y_radius;
      width  = 2 * x_radius;    height = 2 * y_radius;
      x = window_x_points(0) - window_x_point;
      y = window_y_point - window_y_points(0);
      theta = atan2( y, x);
      start_angle = (short) (angle = -(360.0 * (theta / (2.0 * 3.14159))) + 90.0);
      x = window_x_points(1) - window_x_point;
      y = window_y_point - window_y_points(1);
      theta = atan2( y, x );
      offset_angle = (short) ((-(360.0 * (theta / (2.0 * 3.14159))) + 90.0) - angle);
      if ( offset_angle <= 0 )  offset_angle += 360;
      (self->view_object)->DrawArcSize(  left, top, width, height, start_angle, offset_angle );
      (self->view_object)->MoveTo(  window_x_points(1), window_y_points(1) );
      if ( ExposePoints )    (self)->Expose_Object_Points(  figure, pane );
      if ( HighlightPoints ) (self)->Highlight_Object_Points(  figure, pane );
      }
    }
  OUT(Draw);
  return  status;
  }

long
zipoarc::Print_Object( zip_type_figure		   figure, zip_type_pane		   pane )
        {
  class zipoarc *self=this;
  long	status = zip_ok;
  long x, y, xlen, ylen, x0, y0, x1, y1;

  IN(zipoarc::Print_Object);
  x = print_x_point;
  y = print_y_point;
  xlen = print_x_lengths(two);
  ylen = print_y_lengths(two);
  x0 = print_x_points(0);
  y0 = print_y_points(0);
  x1 = print_x_points(1);
  y1 = print_y_points(1);
  (this->print_object)->Ensure_Line_Attributes(  figure );
  (this->print_object)->Draw_Arc(  x, y, abs(xlen), abs(ylen), x0, y0, x1, y1);
  OUT(zipoarc::Print_Object);
  return  status;
  }

long
zipoarc::Proximate_Object_Points( zip_type_figure		   figure, zip_type_pane		   pane, zip_type_pixel		   x , zip_type_pixel		   y )
          {
  int				  point = 0;
  zip_type_pixel			  X1, X2, X3, Y1, Y2, Y3, XS, YS, XE, YE;

  IN(zipoarc::Proximate_Object_Points);
  Compute_Handle_Positions( this, figure, pane,
                	   &X1, &X2, &X3, &Y1, &Y2, &Y3, &XS, &YS, &XE, &YE );
  if ( (this->view_object)->Proximate_Figure_Point(  pane, figure, X2, Y2, x, y ))
    point = zip_figure_origin_point;	    /* 1 -- Origin */
  else
  if ( (this->view_object)->Proximate_Figure_Point(  pane, figure, XS, YS, x, y ))
    point = zip_figure_auxiliary_point;	    /* 2 -- Start */
  else
  if ( (this->view_object)->Proximate_Figure_Point(  pane, figure, XE, YE, x, y ))
    point = zip_figure_auxiliary_point + 1; /* 3 -- End */
  else
  if ( (this->view_object)->Proximate_Figure_Point(  pane, figure, X3, Y2, x, y ))
    point = zip_figure_auxiliary_point + 2; /* 4 -- 3 O'Clock */
  else
  if ( (this->view_object)->Proximate_Figure_Point(  pane, figure, X2, Y3, x, y ))
    point = zip_figure_auxiliary_point + 3; /* 5 -- 6 O'Clock */
  else
  if ( (this->view_object)->Proximate_Figure_Point(  pane, figure, X1, Y2, x, y ))
    point = zip_figure_auxiliary_point + 4; /* 6 -- 9 O'Clock */
  else
  if ( (this->view_object)->Proximate_Figure_Point(  pane, figure, X2, Y1, x, y ))
    point = zip_figure_auxiliary_point + 5; /* 7 -- 12 O'Clock */
  else
  if ( (this->view_object)->Proximate_Figure_Point(  pane, figure, X1, Y1, x, y ))
    point = zip_figure_auxiliary_point + 6; /* 8 -- Upper-Left */
  else
  if ( (this->view_object)->Proximate_Figure_Point(  pane, figure, X3, Y1, x, y ))
    point = zip_figure_auxiliary_point + 7; /* 9 -- Upper-Right */
  else
  if ( (this->view_object)->Proximate_Figure_Point(  pane, figure, X3, Y3, x, y ))
    point = zip_figure_auxiliary_point + 8; /* 10 -- Lower-Right */
  else
  if ( (this->view_object)->Proximate_Figure_Point(  pane, figure, X1, Y3, x, y ))
    point = zip_figure_auxiliary_point + 9; /* 11 -- Lower-Left */
  OUT(zipoarc::Proximate_Object_Points);
  return  point;
  }

boolean
zipoarc::Enclosed_Object( zip_type_figure		   figure, zip_type_pane		   pane, zip_type_pixel		   x , zip_type_pixel		   y , zip_type_pixel		   w , zip_type_pixel		   h )
          {
  boolean			  enclosed = false;
  zip_type_pixel			  X1, X2, X3, Y1, Y2, Y3, XS, YS, XE, YE;

  IN(zipoarc::Enclosed_Object);
  Compute_Handle_Positions( this, figure, pane,
                	   &X1, &X2, &X3, &Y1, &Y2, &Y3, &XS, &YS, &XE, &YE );
  if ( X1 > x  &&  Y1 > y  &&  X3 < (x + w)  &&  Y3 < (y + h) )
    enclosed = true;
  OUT(zipoarc::Enclosed_Object);
  return  enclosed;
  }

long
zipoarc::Object_Enclosure( zip_type_figure		   figure, zip_type_pane		   pane, zip_type_pixel		  *x , zip_type_pixel		  *y , zip_type_pixel		  *w , zip_type_pixel		  *h )
          {
  zip_type_pixel			  X1, X2, X3, Y1, Y2, Y3, XS, YS, XE, YE;

  IN(zipoarc::Object_Enclosure);
  Compute_Handle_Positions( this, figure, pane,
			    &X1, &X2, &X3, &Y1, &Y2, &Y3, &XS, &YS, &XE, &YE );
  *x = X1;  *y = Y1;  *w = abs(X3 - X1);  *h = abs(Y3 - Y1);
  OUT(zipoarc::Object_Enclosure);
  return  zip_ok;
  }

long
zipoarc::Highlight_Object_Points( zip_type_figure		   figure, zip_type_pane		   pane )
        {
  zip_type_pixel			  X1, X2, X3, Y1, Y2, Y3, XS, YS, XE, YE;
  long				  status = zip_ok;

  IN(zipoarc::Highlight_Object_Points);
  Compute_Handle_Positions( this, figure, pane,
                	    &X1, &X2, &X3, &Y1, &Y2, &Y3, &XS, &YS, &XE, &YE );
  (this->edit_object)->Highlight_Handles(  pane, X1, X2, X3, Y1, Y2, Y3 );
  if ( XS != X1  &&  XS != X2  &&  XS != X3 )
    (this->edit_object)->Highlight_Point(  pane, XS, YS );
  if ( XE != X1  &&  XE != X2  &&  XE != X3 )
    (this->edit_object)->Highlight_Point(  pane, XE, YE );
  OUT(zipoarc::Highlight_Object_Points);
  return  status;
  }

long
zipoarc::Normalize_Object_Points( zip_type_figure		   figure, zip_type_pane		   pane )
        {
  zip_type_pixel			  X1, X2, X3, Y1, Y2, Y3, XS, YS, XE, YE;
  long				  status = zip_ok;

  IN(zipoarc::Normalize_Object_Points);
  Compute_Handle_Positions( this, figure, pane,
                	   &X1, &X2, &X3, &Y1, &Y2, &Y3, &XS, &YS, &XE, &YE );
  (this->edit_object)->Normalize_Handles(  pane, X1, X2, X3, Y1, Y2, Y3 );
  if ( XS != X1  &&  XS != X2  &&  XS != X3 )
    (this->edit_object)->Normalize_Point(  pane, XS, YS );
  if ( XE != X1  &&  XE != X2  &&  XE != X3 )
    (this->edit_object)->Normalize_Point(  pane, XE, YE );
  OUT(zipoarc::Normalize_Object_Points);
  return  status;
  }

long
zipoarc::Expose_Object_Points( zip_type_figure		   figure, zip_type_pane		   pane )
        {
  long				  status = zip_ok;

  IN(zipoarc::Expose_Object_Points);
  (this->edit_object)->Expose_Point(  pane, figure, figure_x_point, figure_y_point );
  OUT(zipoarc::Expose_Object_Points);
  return  status;
  }

long
zipoarc::Hide_Object_Points( zip_type_figure		   figure, zip_type_pane		   pane )
        {
  long				  status = zip_ok;

  IN(zipoarc::Hide_Points);
  (this->edit_object)->Hide_Point(  pane, figure, figure_x_point, figure_y_point );
  OUT(zipoarc::Hide_Points);
  return  status;
  }

static
void Set_Points( zip_type_figure		   figure, float			   x_center , float			   y_center , float			   x_radius , float			   y_radius,
		float			   xs_delta , float			   ys_delta , float			   xe_delta , float			   ye_delta )
      {
  figure_x_point = (long) x_center;
  figure_y_point = (long) y_center;
  figure_x_points(0) = (long) ((x_center - x_radius) + xs_delta);
  figure_y_points(0) = (long) ((y_center + y_radius) - ys_delta);
  figure_x_points(1) = (long) ((x_center - x_radius) + xe_delta);
  figure_y_points(1) = (long) ((y_center + y_radius) - ye_delta);
  figure_x_points(two) = (long) abs(x_radius);
  figure_y_points(two) = (long) abs(y_radius);
  }

long
zipoarc::Set_Object_Point( zip_type_figure		   figure, int				   point, zip_type_point		   x , zip_type_point		   y )
          {
  long				  status = zip_ok;
  float			  x_radius, y_radius, span,
					  X1, X2, X3, Y1, Y2, Y3,
					  XS, YS, XE, YE,
					  XS_delta, YS_delta, XE_delta, YE_delta;
  double			  theta;

  IN(zipoarc::Set_Object_Point);
  if ( figure->zip_figure_points == NULL )
    if ( (status = (this->data_object)->Allocate_Figure_Points_Vector(  &figure->zip_figure_points )) == zip_ok )
      if ( (status = (this->data_object)->Enlarge_Figure_Points_Vector(  &figure->zip_figure_points )) == zip_ok  &&
	   (status = (this->data_object)->Enlarge_Figure_Points_Vector(  &figure->zip_figure_points )) == zip_ok )
        {
	figure->zip_figure_points->zip_points_count = 3;
        figure_x_point = x;	/* Center  (Origin) */
        figure_y_point = y;
        figure_x_points(0) = x;	/* Start */
        figure_y_points(0) = y;
        figure_x_points(1) = x;	/* End */
        figure_y_points(1) = y;
        figure_x_points(two) = figure_y_points(two) = 0;	/* Radii */
	}
  x_radius = abs( x - figure_x_point );
  y_radius = abs( y - figure_y_point );
  X2 = figure_x_point;
  X1 = X2 - figure_x_points(two);
  X3 = X2 + figure_x_points(two);
  Y2 = figure_y_point;
  Y1 = Y2 + figure_y_points(two);
  Y3 = Y2 - figure_y_points(two);
  XS = figure_x_points(0);
  YS = figure_y_points(0);
  XE = figure_x_points(1);
  YE = figure_y_points(1);
  if ( (span = X3 - X1) == 0 )  span = 1;
  XS_delta = ((XS - X1)/span) * (2*x_radius);
  XE_delta = ((XE - X1)/span) * (2*x_radius);
  if ( (span = Y3 - Y1) == 0 )  span = 1;
  YS_delta = ((YS - Y1)/span) * (2*y_radius);
  YE_delta = ((YE - Y1)/span) * (2*y_radius);
  if ( status == zip_ok  &&  x_radius  &&  y_radius )
    {
    switch ( point )
      {
      case zip_figure_origin_point:	    /* 1 -- Center X/Y */
	(this)->Adjust_Object_Point_Suite(  figure,
		x - figure_x_point, y - figure_y_point );
        break;
      case zip_figure_auxiliary_point:	    /* 2 -- Start X/Y */
	theta = atan2( 1.0 * (y - Y2), 1.0 * (x - X2) );
	figure_x_points(0) = (long) (X2 + (figure_x_points(two) * cos( theta )));
	figure_y_points(0) = (long) (Y2 + (figure_y_points(two) * sin( theta )));
	break;
      case zip_figure_auxiliary_point + 1:  /* 3 -- End X/Y */
	theta = atan2( 1.0 * (y - Y2), 1.0 * (x - X2) );
	figure_x_points(1) = (long) (X2 + (figure_x_points(two) * cos( theta )));
	figure_y_points(1) = (long) (Y2 + (figure_y_points(two) * sin( theta )));
	break;
      case zip_figure_auxiliary_point + 2:  /* 4 --  3 O'Clock */
      case zip_figure_auxiliary_point + 4:  /* 6 --  9 O'Clock */
	figure_x_points(0) = (long) ((X2 - x_radius) + XS_delta);
	figure_x_points(1) = (long) ((X2 - x_radius) + XE_delta);
	figure_x_points(two) = (long) x_radius;
	break;
      case zip_figure_auxiliary_point + 3:  /* 5 --  6 O'Clock */
      case zip_figure_auxiliary_point + 5:  /* 7 -- 12 O'Clock */
	figure_y_points(0) = (long) ((Y2 + y_radius) - YS_delta);
	figure_y_points(1) = (long) ((Y2 + y_radius) - YE_delta);
	figure_y_points(two) = (long) y_radius;
	break;
      case zip_figure_auxiliary_point + 6:  /* 8  -- Upper-Left  */
	Set_Points( figure, X3 - x_radius, Y3 + y_radius,
		x_radius, y_radius, XS_delta, YS_delta,	XE_delta, YE_delta );
	break;
      case zip_figure_auxiliary_point + 7:  /* 9  -- Upper-Right */
	Set_Points( figure, X1 + x_radius, Y3 + y_radius,
		x_radius, y_radius, XS_delta, YS_delta,	XE_delta, YE_delta );
	break;
      case zip_figure_auxiliary_point + 8:  /* 10 -- Lower-Right */
	Set_Points( figure, X1 + x_radius, Y1 - y_radius,
		x_radius, y_radius, XS_delta, YS_delta,	XE_delta, YE_delta );
	break;
      case zip_figure_auxiliary_point + 9:  /* 11 -- Lower-Left  */
	Set_Points( figure, X3 - x_radius, Y1 - y_radius,
		x_radius, y_radius, XS_delta, YS_delta,	XE_delta, YE_delta );
	break;
      default:
        status = zip_failure; /*=== zip_invalid_point_type ===*/
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
  OUT(zipoarc::Set_Object_Point);
  return  status;
  }

long
zipoarc::Adjust_Object_Point_Suite( zip_type_figure		   figure, zip_type_point		   x_delta , zip_type_point		   y_delta )
        {
  long				  status = zip_ok;

  IN(zipoarc::Adjust_Object_Point_Suite);
  figure_x_point += x_delta;
  figure_y_point += y_delta;
  figure_x_points(0) += x_delta;
  figure_y_points(0) += y_delta;
  figure_x_points(1) += x_delta;
  figure_y_points(1) += y_delta;
  (this->data_object)->Set_Image_Extrema(  figure->zip_figure_image, figure_x_point, figure_y_point );
  (this->data_object)->Set_Stream_Extrema(  figure->zip_figure_image->zip_image_stream,
			    figure->zip_figure_image );
  OUT(zipoarc::Adjust_Object_Point_Suite);
  return  status;
  }

static
void Compute_Handle_Positions( class zipoarc		  *self, zip_type_figure		   figure, zip_type_pane		   pane, zip_type_pixel		  *X1 , zip_type_pixel		  *X2 , zip_type_pixel		  *X3 , zip_type_pixel		  *Y1 , zip_type_pixel		  *Y2 , zip_type_pixel		  *Y3, zip_type_pixel		  *XS , zip_type_pixel		  *YS , zip_type_pixel		  *XE , zip_type_pixel		  *YE )
          {
  *X1 = (self->view_object)->X_Point_To_Pixel(  pane, figure, figure_x_point - figure_x_points(two) );
  *X2 = window_x_point;
  *X3 = (self->view_object)->X_Point_To_Pixel(  pane, figure, figure_x_point + figure_x_points(two) );
  *Y1 = (self->view_object)->Y_Point_To_Pixel(  pane, figure, figure_y_point + figure_y_points(two) );
  *Y2 = window_y_point;
  *Y3 = (self->view_object)->Y_Point_To_Pixel(  pane, figure, figure_y_point - figure_y_points(two) );
  *XS = (self->view_object)->X_Point_To_Pixel(  pane, figure, figure_x_points(0) );
  *YS = (self->view_object)->Y_Point_To_Pixel(  pane, figure, figure_y_points(0) );
  *XE = (self->view_object)->X_Point_To_Pixel(  pane, figure, figure_x_points(1) );
  *YE = (self->view_object)->Y_Point_To_Pixel(  pane, figure, figure_y_points(1) );
  }
