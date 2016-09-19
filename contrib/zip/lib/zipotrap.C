/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

/* zipotrap.c	Zip Object -- Trapezoid					      */
/* Author	TC Peters						      */
/* Information Technology Center		   Carnegie-Mellon University */

/**  SPECIFICATION -- Internal Facility Suite  *********************************

TITLE	Zip Object -- Trapezoid

MODULE	zipotrap.c

NOTICE	IBM Internal Use Only

DESCRIPTION

.......................

	    HANDLES

         X1****X2****X3		Y1
         *            *
        *              *
      X4        X5      X6	Y2
      *                  *
     *                    *
   X7************X8********X9   Y3


HISTORY
  04/13/88	Created (TC Peters)
  10/27/88	Optimize drawing times (TCP)
   06/14/89	Made Draw() use Shade attribute rather than pattern attribute,
                            plus symbolic font name (SCG)
   08/24/89	Remove excess SetTransferMode() activity in Draw() (SCG)

END-SPECIFICATION  ************************************************************/


#include <andrewos.h>
#include <view.H>
#include <fontdesc.H>
#include "zip.H"
#include "zipview.H"
#include "zipedit.H"
#include "zipprint.H"
#include "zipobject.H"
#include "zipotrap.H"


ATKdefineRegistry(zipotrap, zipobject, NULL);

static long Draw( class zipotrap *self, zip_type_figure figure, zip_type_pane pane );
static void Compute_Handle_Positions( class zipotrap		  *self, zip_type_figure		   figure, zip_type_pane		   pane, zip_type_pixel		  *X1 , zip_type_pixel		  *X2 , zip_type_pixel		  *X3 , zip_type_pixel		  *X4 , zip_type_pixel		  *X5 , zip_type_pixel		  *X6 , zip_type_pixel		  *X7 , zip_type_pixel		  *X8 , zip_type_pixel		  *X9, zip_type_pixel		  *Y1 , zip_type_pixel		  *Y2 , zip_type_pixel		  *Y3 );


char
zipotrap::Object_Icon( )
    {
  IN(zipotrap::Object_Icon);
  OUT(zipotrap::Object_Icon);
/*===  return  'P';*/
  return  0;
  }

char
zipotrap::Object_Icon_Cursor( )
    {
  IN(zipotrap::Object_Icon_Cursor);
  OUT(zipotrap::Object_Icon_Cursor);
/*===  return  'A';*/
  return  0;
  }

char
zipotrap::Object_Datastream_Code( )
    {
  IN(zipotrap::Object_Datastream_Code);
  OUT(zipotrap::Object_Datastream_Code);
  return  'F';
  }

long
zipotrap::Build_Object( zip_type_pane		   pane, enum view_MouseAction				   action , long				   x , long				   y , long				   clicks, zip_type_point		   X , zip_type_point		   Y )
          {
  int				  status = zip_success;
  static zip_type_point			  initial_Y;
  zip_type_figure		  figure;

  IN(zipotrap::Build_Object);
  switch ( action )
    {
    case view_LeftDown:
      if ( (status =
	(this->data_object)->Create_Figure(  &CurrentFigure, NULL, zip_trapezoid_figure,
			   CurrentImage, NULL )) == zip_success )
        {
        (this)->Set_Object_Point(  CurrentFigure,	zip_figure_origin_point, X, Y );
        (this)->Set_Object_Point(  CurrentFigure,	zip_figure_auxiliary_point, X, Y );
	CurrentFigure->zip_figure_zoom_level =  pane->zip_pane_zoom_level;
	(this->data_object)->Set_Figure_Pattern(  CurrentFigure, '5' /*===*/ );
	(this->data_object)->Set_Figure_Font(  CurrentFigure, "shape10" /*===*/ );
	initial_Y = Y;
	}
      break;
    case view_LeftUp:
      if ( ( figure = CurrentFigure ) )
        {
	if ( figure_x_point == 0  ||  figure_y_point == 0 )
	  {
	  (this->edit_object)->Delete_Figure(  figure, pane );
	  break;
	  }
	}
      /* Fall-thru */
    case view_LeftMovement:
      if ( CurrentFigure )
	{
	(this->view_object)->Set_Pane_Painting_Mode(  pane, zipview_paint_inverted );
	(this->view_object)->Draw_Figure(  CurrentFigure, pane );
        (this)->Set_Object_Point(  CurrentFigure,
		zip_figure_auxiliary_point + 1, X, initial_Y );
        (this)->Set_Object_Point(  CurrentFigure,
		zip_figure_auxiliary_point + 2, X, Y );
	(this->view_object)->Draw_Figure(  CurrentFigure, pane );
	(this->view_object)->Set_Pane_Painting_Mode(  pane, zip_default );
	}
      break;
    default:
      break;
    }
  OUT(zipotrap::Build_Object);
  return  status;
  }

long
zipotrap::Draw_Object( zip_type_figure		   figure, zip_type_pane		   pane )
        {
  long				  status = zip_ok;

  IN(zipotrap::Draw_Object);
/*===
  if ( zipview_Condition( this->view_object, pane, figure, zip_draw ) )
    status = Draw( self, figure, pane );
===*/
  if ( figure->zip_figure_zoom_level <= pane->zip_pane_zoom_level )
    status = Draw( this, figure, pane );
  OUT(zipotrap::Draw_Object);
  return  status;
  }

long
zipotrap::Clear_Object( zip_type_figure		   figure, zip_type_pane		   pane )
        {
  long				  status = zip_ok;

  IN(zipotrap::Clear_Object);
  if ( (this->view_object)->Condition(  pane, figure, zip_clear ) )
    status = Draw( this, figure, pane );
  OUT(zipotrap::Clear_Object);
  return  status;
  }

static
long Draw( class zipotrap *self, zip_type_figure figure, zip_type_pane pane )
{
  long				  status = zip_ok, shade;
  long	i, L1, L2, X1, Y1, X2, Y2,
			  pane_left = (self->view_object)->Pane_Left(  pane ),
			  pane_top = (self->view_object)->Pane_Top(  pane ),
			  pane_right = (self->view_object)->Pane_Right(  pane ),
			  pane_bottom = (self->view_object)->Pane_Bottom(  pane );
  class graphic		 *graphic;
  float			  SM = pane->zip_pane_stretch_zoom_multiplier,
					  SD = pane->zip_pane_stretch_divisor;

  IN(Draw);
  if ( ( shade = (self->data_object)->Contextual_Figure_Shade(  figure )))
    {
    if ( (shade = ('0' + ((shade + 10) / 10)) - 1) > '9' )  shade = '9';
    graphic = (self->view_object)->Define_Graphic(  (self->data_object)->Define_Font(  ShadeFontName, NULL ), shade );    X1 = window_x_points(0);
    X2 = window_x_points(1);
    Y1 = window_y_points(0);
    Y2 = window_y_points(1);
    L1 = (long) (figure->zip_figure_point.zip_point_x * SM / SD);
    L2 = (long) (figure->zip_figure_point.zip_point_y * SM / SD);
    (self->view_object)->FillTrapezoid(  X1, Y1, L1, X2, Y2, L2, graphic );
    for ( i = 2; i < figure->zip_figure_points->zip_points_count; i += 3  )
      {
      X1 = window_x_points(i + 1);
      X2 = window_x_points(i + 2);
      if ( X1 > pane_right  &&  X2 > pane_right )
        continue;
      Y1 = window_y_points(i + 1);
      Y2 = window_y_points(i + 2);
      if ( Y1 > pane_bottom ||  Y2 < pane_top )
        continue;
      L1 = (long) (figure_x_points(i) * SM / SD);
      L2 = (long) (figure_y_points(i) * SM / SD);
      if ( (X1 + L1) < pane_left  &&  (X2 + L2) < pane_left )
        continue;
      (self->view_object)->FillTrapezoid(  X1, Y1, L1, X2, Y2, L2, graphic );
      }
    if ( ExposePoints )
      (self)->Expose_Object_Points(  figure, pane );
    if ( HighlightPoints )
      (self)->Highlight_Object_Points(  figure, pane );
    }
  OUT(Draw);
  return  status;
  }

long
zipotrap::Print_Object( zip_type_figure figure, zip_type_pane  pane )
{
  class zipotrap *self=this;
  long				  status = zip_ok;
  char					  pattern = 0;
  int					  i;

  IN(zipotrap::Print_Object);
  if ( ( pattern = (this->data_object)->Contextual_Figure_Pattern(  figure ) ) )
  {
      long x1,y1,x2,y2,xlen,ylen;

      x1 = print_x_points(0);
      y1 = print_y_points(0);
      x2 = print_x_points(1);
      y2 = print_y_points(1);
      xlen = print_x_length;
      ylen = print_y_length;

    (this->print_object)->Fill_Trapezoid(  x1, y1, x2, y2, xlen, ylen, pattern);

    for ( i = 2; i < figure->zip_figure_points->zip_points_count; i += 3  )  {
      x1 = print_x_points(i+1);
      y1 = print_y_points(i+1);
      x2 = print_x_points(i+2);
      y2 = print_y_points(i+2);
      xlen = print_x_lengths(i);
      ylen = print_y_lengths(i);

     (this->print_object)->Fill_Trapezoid(  x1, y1, x2, y2, xlen, ylen, pattern);
    }
    }
    
  OUT(zipotrap::Print_Object);
  return  status;
  }

long
zipotrap::Proximate_Object_Points( zip_type_figure		   figure, zip_type_pane		   pane, zip_type_pixel		   x , zip_type_pixel		   y )
          {
  int				  point = 0;
  zip_type_pixel			  X1, X2, X3, X4, X5, X6, X7, X8, X9, Y1, Y2, Y3;

  IN(zipotrap::Proximate_Object_Points);
  Compute_Handle_Positions( this, figure, pane,
    &X1, &X2, &X3, &X4, &X5, &X6, &X7, &X8, &X9, &Y1, &Y2, &Y3 );
  if ( (this->view_object)->Proximate_Figure_Point(  pane, figure, X1, Y1, x, y ))
    point = zip_figure_origin_point;		/* Upper Left Corner */
  else
  if ( (this->view_object)->Proximate_Figure_Point(  pane, figure, X7, Y3, x, y ) )
    point = zip_figure_auxiliary_point;		/* Lower Left Corner */
  else
  if ( (this->view_object)->Proximate_Figure_Point(  pane, figure, X3, Y1, x, y ))
    point = zip_figure_auxiliary_point + 1;	/* Upper Right Corner */
  else
  if ( (this->view_object)->Proximate_Figure_Point(  pane, figure, X9, Y3, x, y ))
    point = zip_figure_auxiliary_point + 2;	/* Lower Right Corner */
  else
  if ( (this->view_object)->Proximate_Figure_Point(  pane, figure, X2, Y1, x, y ))
    point = zip_figure_auxiliary_point + 3;	/* 12 O'Clock */
  else
  if ( (this->view_object)->Proximate_Figure_Point(  pane, figure, X6, Y2, x, y ))
    point = zip_figure_auxiliary_point + 4;	/* 3 O'Clock */
  else
  if ( (this->view_object)->Proximate_Figure_Point(  pane, figure, X8, Y3, x, y ))
    point = zip_figure_auxiliary_point + 5;	/* 6 O'Clock */
  else
  if ( (this->view_object)->Proximate_Figure_Point(  pane, figure, X4, Y2, x, y ))
    point = zip_figure_auxiliary_point + 6;	/* 9 O'Clock */
  else
  if ( (this->view_object)->Proximate_Figure_Point(  pane, figure, X5, Y2, x, y ))
    point = zip_figure_auxiliary_point + 7;	/* Center */
  OUT(zipotrap::Proximate_Object_Points);
  return  point;
  }

long
zipotrap::Highlight_Object_Points( zip_type_figure		   figure, zip_type_pane		   pane )
        {
  zip_type_pixel			  X1, X2, X3, X4, X5, X6, X7, X8, X9, Y1, Y2, Y3;
  long				  status = zip_ok;

  IN(zipotrap::Highlight_Object_Points);
  Compute_Handle_Positions( this, figure, pane,
    &X1, &X2, &X3, &X4, &X5, &X6, &X7, &X8, &X9, &Y1, &Y2, &Y3 );
  (this->edit_object)->Highlight_Point(  pane, X1, Y1 );
  (this->edit_object)->Highlight_Point(  pane, X2, Y1 );
  (this->edit_object)->Highlight_Point(  pane, X3, Y1 );
  (this->edit_object)->Highlight_Point(  pane, X4, Y2 );
  (this->edit_object)->Highlight_Point(  pane, X5, Y2 );
  (this->edit_object)->Highlight_Point(  pane, X6, Y2 );
  (this->edit_object)->Highlight_Point(  pane, X7, Y3 );
  (this->edit_object)->Highlight_Point(  pane, X8, Y3 );
  (this->edit_object)->Highlight_Point(  pane, X9, Y3 );
  OUT(zipotrap::Highlight_Object_Points);
  return  status;
  }

long
zipotrap::Normalize_Object_Points( zip_type_figure		   figure, zip_type_pane		   pane )
        {
  zip_type_pixel			  X1, X2, X3, X4, X5, X6, X7, X8, X9, Y1, Y2, Y3;
  long				  status = zip_ok;

  IN(zipotrap::Normalize_Object_Points);
  Compute_Handle_Positions( this, figure, pane,
    &X1, &X2, &X3, &X4, &X5, &X6, &X7, &X8, &X9, &Y1, &Y2, &Y3 );
  (this->edit_object)->Normalize_Point(  pane, X1, Y1 );
  (this->edit_object)->Normalize_Point(  pane, X2, Y1 );
  (this->edit_object)->Normalize_Point(  pane, X3, Y1 );
  (this->edit_object)->Normalize_Point(  pane, X4, Y2 );
  (this->edit_object)->Normalize_Point(  pane, X5, Y2 );
  (this->edit_object)->Normalize_Point(  pane, X6, Y2 );
  (this->edit_object)->Normalize_Point(  pane, X7, Y3 );
  (this->edit_object)->Normalize_Point(  pane, X8, Y3 );
  (this->edit_object)->Normalize_Point(  pane, X9, Y3 );
  OUT(zipotrap::Normalize_Object_Points);
  return  status;
  }

long
zipotrap::Expose_Object_Points( zip_type_figure		   figure, zip_type_pane		   pane )
        {
  long				  status = zip_ok;

  IN(zipotrap::Expose_Object_Points);
  (this->edit_object)->Expose_Point(  pane, figure, figure_x_point, figure_y_point );
  OUT(zipotrap::Expose_Object_Points);
  return  status;
  }

long
zipotrap::Hide_Object_Points( zip_type_figure		   figure, zip_type_pane		   pane )
        {
  long				  status = zip_ok;

  IN(zipotrap::Hide_Points);
  (this->edit_object)->Hide_Point(  pane, figure, figure_x_point, figure_y_point );
  OUT(zipotrap::Hide_Points);
  return  status;
  }

long
zipotrap::Set_Object_Point( zip_type_figure figure, int point, zip_type_point x , zip_type_point y )
          {
  long				  status = zip_ok;
  long				  delta;

  IN(zipotrap::Set_Object_Point);
  if ( figure->zip_figure_points == NULL )
    if ( (status = (this->data_object)->Allocate_Figure_Points_Vector(  &figure->zip_figure_points )) == zip_success )
      if ( (status = (this->data_object)->Enlarge_Figure_Points_Vector(  &figure->zip_figure_points )) == zip_success )
	{
	figure->zip_figure_points->zip_points_count = 2;
	figure_x_point = 0;
	figure_y_point = 0;
	figure_x_points(0) = x;
	figure_y_points(0) = y;
	figure_x_points(1) = x;
	figure_y_points(1) = y;
	}
  if ( status == zip_success )
    {
    switch ( point )
      {
      case zip_figure_origin_point:		/* 1 => Upper Left */
	delta = x - figure_x_points(0);
	figure_x_point += abs( delta );
	figure_x_points(0) += delta;
	delta = y - figure_y_points(0);
	figure_y_points(0) += delta;
        break;
      case zip_figure_auxiliary_point:		/* 2 => Lower Left */
	delta = x - figure_x_points(1);
	figure_y_point += abs( delta );
	figure_x_points(1) += delta;
	delta = y - figure_y_points(1);
	figure_y_points(1) += delta;
	break;
      case zip_figure_auxiliary_point + 1:	/* 3 => Upper Right */
	delta = x - (figure_x_points(0) + figure_x_point );
	figure_x_point += abs( delta );
	delta = y - figure_y_points(0);
	figure_y_points(0) += delta;
        break;
      case zip_figure_auxiliary_point + 2:	/* 4 => Lower Right */
	delta = x - (figure_x_points(1) + figure_y_point );
	figure_y_point += abs( delta );
	delta = y - figure_y_points(1);
	figure_y_points(1) += delta;
	break;
      default:
	status = zip_failure;
      }
    if ( status == zip_success )
      {
      (this->data_object)->Set_Image_Extrema(  figure->zip_figure_image,
			     figure->zip_figure_point.zip_point_x +
				       figure->zip_figure_points->zip_points[0].zip_point_x,
			     figure->zip_figure_points->zip_points[0].zip_point_y );
      (this->data_object)->Set_Image_Extrema(  figure->zip_figure_image,
			     figure->zip_figure_point.zip_point_y +
				       figure->zip_figure_points->zip_points[1].zip_point_x,
			     figure->zip_figure_points->zip_points[1].zip_point_y );

      (this->data_object)->Set_Stream_Extrema(  figure->zip_figure_image->zip_image_stream,
			      figure->zip_figure_image );
      }
    }
    if ( status == zip_success )
      {
      (this->data_object)->Set_Image_Extrema(  figure->zip_figure_image, x, y );
/*===handle both extrema 7/20/86===*/
/*===have extrema check for REDUCTIONS as well as EXPANSIONS 7/20/86===*/
      (this->data_object)->Set_Stream_Extrema(  figure->zip_figure_image->zip_image_stream,
			      figure->zip_figure_image );
      }
  OUT(zipotrap::Set_Object_Point);
  return  status;
  }

long
zipotrap::Adjust_Object_Point_Suite( zip_type_figure		   figure, zip_type_point		   x_delta , zip_type_point		   y_delta )
        {
  long				  status = zip_ok;

  IN(zipotrap::Adjust_Object_Point_Suite);
  figure_x_point += x_delta;
  figure_y_point += y_delta;
  figure_x_points(0) += x_delta;
  figure_y_points(0) += y_delta;
  (this->data_object)->Set_Image_Extrema(  figure->zip_figure_image, figure_x_point, figure_y_point );
  (this->data_object)->Set_Stream_Extrema(  figure->zip_figure_image->zip_image_stream,
			    figure->zip_figure_image );
  OUT(zipotrap::Adjust_Object_Point_Suite);
  return  status;
  }

static
void Compute_Handle_Positions( class zipotrap		  *self, zip_type_figure		   figure, zip_type_pane		   pane, zip_type_pixel		  *X1 , zip_type_pixel		  *X2 , zip_type_pixel		  *X3 , zip_type_pixel		  *X4 , zip_type_pixel		  *X5 , zip_type_pixel		  *X6 , zip_type_pixel		  *X7 , zip_type_pixel		  *X8 , zip_type_pixel		  *X9, zip_type_pixel		  *Y1 , zip_type_pixel		  *Y2 , zip_type_pixel		  *Y3 )
          {
  *X1 = window_x_points(0);
  *X3 = (self->view_object)->X_Point_To_Pixel(  pane, figure, figure_x_points(0) + figure_x_point );
  *X2 = *X1 + ((*X3 - *X1) / 2);

  *X7 = window_x_points(1);
  *X9 = (self->view_object)->X_Point_To_Pixel(  pane, figure, figure_x_points(1) + figure_y_point );
  *X8 = *X7 + ((*X9 - *X7) / 2);

  *X4 = *X7 + ((*X1 - *X7) / 2);
  *X6 = *X9 + ((*X3 - *X9) / 2);
  *X5 = *X4 + ((*X6 - *X4) / 2);

  *Y1 = window_y_points(0);
  *Y3 = window_y_points(1);
  *Y2 = *Y1 + ((*Y3 - *Y1) / 2);
  }
