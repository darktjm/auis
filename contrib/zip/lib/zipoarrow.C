/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

/* zipoarrw.c	Zip Object -- Arrow					      */
/* Author	TC Peters						      */
/* Information Technology Center		   Carnegie-Mellon University */

/**  SPECIFICATION -- Internal Facility Suite  *********************************

TITLE	Zip Object -- Arrow

MODULE	zipoarrw.c

NOTICE	IBM Internal Use Only

DESCRIPTION

.......................

HISTORY
  04/13/88	Created (TC Peters)
  11/18/88	Recognize Line-width (TCP)
  05/01/89	Use symbolic font-names (TCP)
  05/16/89	Destroy zero-length arrows when built (TCP)
   08/14/90	Use Ensure_Line_Attributes on Draw and Print (SCG)

END-SPECIFICATION  ************************************************************/

#include <andrewos.h>
#include <math.h>
#include <view.H>
#include <environ.H>
#include "zip.H"
#include "zipview.H"
#include "zipedit.H"
#include "zipprint.h"
#include "zipprint.H"
#include "zipobject.H"
#include "zipoarrow.H"


ATKdefineRegistry(zipoarrow, zipobject, NULL);
static int Draw( class zipoarrow		  *self, zip_type_figure		   figure, zip_type_pane		   pane );
static void Draw_Basic_Style( class zipoarrow		  *self, zip_type_figure		   figure, zip_type_pane		   pane, char				   fill );
static void Draw_Basic_Body( class zipoarrow		  *self, zip_type_figure		   figure, zip_type_pane		   pane, zip_type_pixel		  *current_x , zip_type_pixel		  *current_y , zip_type_pixel		  *prior_x , zip_type_pixel		  *prior_y );
static void Feather_Points( zip_type_pixel		   current_x , zip_type_pixel		   current_y , zip_type_pixel		   prior_x , zip_type_pixel		   prior_y, zip_type_pixel		   *x1 , zip_type_pixel		   *y1 , zip_type_pixel		   *x2 , zip_type_pixel		   *y2 );


zipoarrow::zipoarrow( )
      {
  IN(zipoarrow_InitializeObject);
  this->tolerance = environ::GetProfileInt( "ZipCreateTolerance", 10 );
  DEBUGdt(Tolerance,this->tolerance);
  OUT(zipoarrow_InitializeObject);
  THROWONFAILURE(  true);
  }

char
zipoarrow::Object_Icon( )
    {
  IN(zipoarrow::Object_Icon);
  OUT(zipoarrow::Object_Icon);
  return  'O';
  }

char
zipoarrow::Object_Icon_Cursor( )
    {
  IN(zipoarrow::Object_Icon_Cursor);
  OUT(zipoarrow::Object_Icon_Cursor);
  return  'c';
  }

char
zipoarrow::Object_Datastream_Code( )
    {
  IN(zipoarrow::Object_Datastream_Code);
  OUT(zipoarrow::Object_Datastream_Code);
  return  'O';
  }

long
zipoarrow::Show_Object_Properties( zip_type_pane		   pane, zip_type_figure		   figure )
        {
  (this->view_object)->Announce(  "Draw Arrow from Tail to Head." );
  return  zip_ok;
  }

long
zipoarrow::Build_Object( zip_type_pane		   pane, enum view_MouseAction				   action , long				   x , long				   y , long				   clicks, zip_type_point		   X , zip_type_point		   Y )
          {
  long				  status = zip_ok;
  static zip_type_point			  prior_X, prior_Y;
  static zip_type_pixel			  prior_x, prior_y;
  zip_type_figure		  figure;

  IN(zipoarrow::Build_Object);
  if ( action == (long)view_LeftDown  &&
       (abs(x - prior_x) < this->tolerance  &&  abs(y - prior_y) < this->tolerance) )
    {
    action = view_NoMouseEvent;
    CurrentFigure = NULL;
    prior_x = prior_y = prior_X = prior_Y = 0;
    pane->zip_pane_edit->zip_pane_edit_build_figure = true;
    }
  switch ( action )
    {
    case view_LeftDown:
      prior_x = x;  prior_y = y;
      if ( pane->zip_pane_edit->zip_pane_edit_build_figure  ||
	   CurrentFigure == NULL )
	{
	prior_X = X;  prior_Y = Y;
	if ( (status = (this->data_object)->Create_Figure(  &CurrentFigure, NULL,
		zip_arrow_figure, CurrentImage, NULL )) == zip_ok )
          {
          (this)->Set_Object_Point(  CurrentFigure, zip_figure_origin_point, X, Y );
          (this)->Set_Object_Point(  CurrentFigure, zip_figure_auxiliary_point, X, Y );
  	  CurrentFigure->zip_figure_zoom_level = pane->zip_pane_zoom_level;
	  pane->zip_pane_edit->zip_pane_edit_build_figure = false;
	  pane->zip_pane_edit->zip_pane_edit_last_point_id = zip_figure_auxiliary_point;
	  }
	}
	else
	{
	(this->view_object)->Set_Pane_Painting_Mode(  pane, zipview_paint_inverted );
	(this->view_object)->Draw_Figure(  CurrentFigure, pane );
	if ( X != prior_X  ||  Y != prior_Y )
	  {
	  if ( pane->zip_pane_edit->zip_pane_edit_last_point_id == zip_figure_auxiliary_point  &&
	       CurrentFigure->zip_figure_points->zip_points[0].zip_point_x ==
		 CurrentFigure->zip_figure_point.zip_point_x &&
	       CurrentFigure->zip_figure_points->zip_points[0].zip_point_y ==
		 CurrentFigure->zip_figure_point.zip_point_y )
	      pane->zip_pane_edit->zip_pane_edit_last_point_id--;
          (this)->Set_Object_Point(  CurrentFigure,
		   ++pane->zip_pane_edit->zip_pane_edit_last_point_id,
		   prior_X = X, prior_Y = Y );
	  (this->view_object)->Draw_Figure(  CurrentFigure, pane );
	  (this->view_object)->Set_Pane_Painting_Mode(  pane, zip_default );
	  }
	(this->view_object)->Set_Pane_Painting_Mode(  pane, zip_default );
	}
      break;
    case view_LeftUp:
      if ( ( figure = CurrentFigure ) )
	{
	if ( figure_x_point == figure_x_points(0)  &&
	     figure_y_point == figure_y_points(0) )
	  {
	  (this->edit_object)->Delete_Figure(  figure, pane );
          break;
	  }
	  else
	  {
          prior_x = x;  prior_y = y;
          (this->view_object)->Draw_Figure(  CurrentFigure, pane );
	  }
	}
      break;
    case view_LeftMovement:
      if ( CurrentFigure  &&  (X != prior_X  ||  Y != prior_Y) )
	{
	(this->view_object)->Set_Pane_Painting_Mode(  pane, zipview_paint_inverted );
	(this->view_object)->Draw_Figure(  CurrentFigure, pane );
        (this)->Set_Object_Point(  CurrentFigure,
	    pane->zip_pane_edit->zip_pane_edit_last_point_id, X, Y );
	(this->view_object)->Draw_Figure(  CurrentFigure, pane );
	(this->view_object)->Set_Pane_Painting_Mode(  pane, zip_default );
	}
      break;
    default:
      break;
    }
  OUT(zipoarrow::Build_Object);
  return  status;
  }

long
zipoarrow::Draw_Object( zip_type_figure		   figure, zip_type_pane		   pane )
        {
  long				  status = zip_ok;

  if ( (this->view_object)->Condition(  pane, figure, zip_draw ) )
    status = Draw( this, figure, pane );
  OUT(zipoarrow::Draw_Object);
  return  status;
  }

long
zipoarrow::Clear_Object( zip_type_figure		   figure, zip_type_pane		   pane )
        {
  long				  status = zip_ok;

  IN(zipoarrow::Clear_Object);
  if ( (this->view_object)->Condition(  pane, figure, zip_clear ) )
    status = Draw( this, figure, pane );
  OUT(zipoarrow::Clear_Object);
  return  status;
  }

static
int Draw( class zipoarrow		  *self, zip_type_figure		   figure, zip_type_pane		   pane )
        {
  long				  status = zip_ok;

  IN(Draw);
  if ( (self->view_object)->Ensure_Line_Attributes(  figure ) == zip_ok )
    {
    switch ( figure->zip_figure_style )
      {
      case 0: Draw_Basic_Style(self, figure, pane, 0 );   break;
      case 1: Draw_Basic_Style(self, figure, pane, '9' );    break;
      case 2: Draw_Basic_Style(self, figure, pane, '0' );    break;
      }
    if ( ExposePoints )    (self)->Expose_Object_Points(  figure, pane );
    if ( HighlightPoints ) (self)->Highlight_Object_Points(  figure, pane );
    }
  OUT(Draw);
  return  status;
  }

static
void Draw_Basic_Style( class zipoarrow *self, zip_type_figure		   figure, zip_type_pane pane, char fill )
{
  long					  current_x, current_y, prior_x, prior_y;
  struct point				  points[3];
  short			  transfer_mode;

  IN(Draw_Basic_Style);
  Draw_Basic_Body( self, figure, pane, &current_x, &current_y, &prior_x, &prior_y );
  if ( prior_x != current_x  ||  prior_y != current_y )
    {
    Feather_Points( current_x, current_y, prior_x, prior_y,
		    &points[0].x, &points[0].y, &points[1].x, &points[1].y );
    points[2].x = current_x; points[2].y = current_y;
    if ( fill )
      {
      transfer_mode = (self->view_object)->GetTransferMode( );
      (self->view_object)->SetTransferMode(  graphic_COPY );
      (self->view_object)->FillPolygon(  points, 3,
	(self->view_object)->Define_Graphic(  (self->data_object)->Define_Font(  ShadeFontName, NULL ), fill ) );
      (self->view_object)->SetTransferMode(  transfer_mode );
      (self->view_object)->DrawLineTo(  points[0].x, points[0].y );
      (self->view_object)->DrawLineTo(  points[1].x, points[1].y );
      (self->view_object)->DrawLineTo(  current_x, current_y );
      }
      else
      {
      (self->view_object)->DrawLineTo(  points[0].x, points[0].y );
      (self->view_object)->MoveTo(  current_x, current_y );
      (self->view_object)->DrawLineTo(  points[1].x, points[1].y  );
      }
    }
  OUT(Draw_Basic_Style);
  }

static
void Draw_Basic_Body( class zipoarrow		  *self, zip_type_figure		   figure, zip_type_pane		   pane, zip_type_pixel		  *current_x , zip_type_pixel		  *current_y , zip_type_pixel		  *prior_x , zip_type_pixel		  *prior_y )
          {
  long				  i;

  *current_x = *current_y = 0;
  (self->view_object)->MoveTo(  *prior_x = window_x_point, *prior_y = window_y_point );
  for ( i = 0; i < figure->zip_figure_points->zip_points_count; i++ )
    {
    (self->view_object)->DrawLineTo(  *current_x = window_x_points(i),
			      *current_y = window_y_points(i) );
    if ( figure->zip_figure_points->zip_points_count > (i+1) )
      { *prior_x = *current_x;  *prior_y = *current_y; }
    }
  }

static
void Feather_Points( zip_type_pixel		   current_x , zip_type_pixel		   current_y , zip_type_pixel		   prior_x , zip_type_pixel		   prior_y, zip_type_pixel		   *x1 , zip_type_pixel		   *y1 , zip_type_pixel		   *x2 , zip_type_pixel		   *y2 )
    {
  double			  theta, cos_theta, sin_theta;

  IN(Feather_Points);
  DEBUGdt(Current-x,current_x);  DEBUGdt(Current-y,current_y);
  DEBUGdt(Prior-x,prior_x);      DEBUGdt(Prior-y,prior_y);
  theta = atan2( 1.0 * ((current_y - prior_y) ? (current_y - prior_y) : 1),
		 1.0 * ((prior_x - current_x) ? (prior_x - current_x) : 1) );
  DEBUGgt(Theta,theta);
  cos_theta = cos( theta );  DEBUGgt(Cos-Theta,cos_theta);
  sin_theta = sin( theta );  DEBUGgt(Sin-Theta,sin_theta);
  *x1 = (zip_type_pixel) (current_x + (5.0 * cos_theta - 2.5 * sin_theta));  DEBUGdt(X1,x1);
  *y1 = (zip_type_pixel) (current_y - (5.0 * sin_theta + 2.5 * cos_theta));  DEBUGdt(Y1,y1);
  *x2 = (zip_type_pixel) (current_x + (5.0 * cos_theta - (-2.5 * sin_theta)));  DEBUGdt(X2,x2);
  *y2 = (zip_type_pixel) (current_y - (5.0 * sin_theta + (-2.5 * cos_theta)));  DEBUGdt(Y2,y2);
  OUT(Feather_Points);
  }

long
zipoarrow::Print_Object( zip_type_figure figure, zip_type_pane pane )
{
  class zipoarrow *self=this;
  long				  pc, status = zip_ok, x1, y1, x2, y2,
					  current_x = 0, current_y = 0, prior_x, prior_y;
  zip_type_point_pairs	    		  points = NULL;
  double			  theta, cos_theta, sin_theta;
/*===debug=1;===*/
  IN(zipoarrow::Print_Object);
  if ( (status = (this->data_object)->Allocate_Figure_Points_Vector(  &points )) == zip_ok )
    {
    points->zip_points_count = 0;
    prior_x = print_x_point;  prior_y = print_y_point;
    for ( pc = 0; pc < figure->zip_figure_points->zip_points_count; pc++ )
      {
      if ( status == zip_ok )
        {
        points->zip_points[pc].zip_point_x = current_x = print_x_points(pc);
        points->zip_points[pc].zip_point_y = current_y = print_y_points(pc);
        points->zip_points_count++;
        status = (this->data_object)->Enlarge_Figure_Points_Vector(  &points );
	if ( figure->zip_figure_points->zip_points_count > (pc+1) )
	  { prior_x = current_x;  prior_y = current_y; }
        }
      }
    DEBUGdt(Count,points->zip_points_count);
    theta = atan2( 1.0 * ((current_y - prior_y) ? (current_y - prior_y) : 1),
		   1.0 * ((prior_x - current_x) ? (prior_x - current_x) : 1) );
    DEBUGgt(Theta,theta);
    cos_theta = cos( theta );  DEBUGgt(Cos-Theta,cos_theta);
    sin_theta = sin( theta );  DEBUGgt(Sin-Theta,sin_theta);
    x1 = (long) (current_x + (750.0 * cos_theta - 375.0 * sin_theta));     DEBUGdt(X1,x1);
    y1 = (long) (current_y - (750.0 * sin_theta + 375.0 * cos_theta));     DEBUGdt(Y1,y1);
    x2 = (long) (current_x + (750.0 * cos_theta - (-375.0 * sin_theta)));  DEBUGdt(X2,x2);
    y2 = (long) (current_y - (750.0 * sin_theta + (-375.0 * cos_theta)));  DEBUGdt(Y2,y2);
    points->zip_points[pc].zip_point_x = x1;
    points->zip_points[pc].zip_point_y = y1;
    pc++;  points->zip_points_count++;
    status = (this->data_object)->Enlarge_Figure_Points_Vector(  &points );
    points->zip_points[pc].zip_point_x = current_x;
    points->zip_points[pc].zip_point_y = current_y;
    pc++;  points->zip_points_count++;
    status = (this->data_object)->Enlarge_Figure_Points_Vector(  &points );
    points->zip_points[pc].zip_point_x = x2;
    points->zip_points[pc].zip_point_y = y2;
    (this->print_object)->Ensure_Line_Attributes(  figure );
    (this->print_object)->Draw_Multi_Line( 
			      figure->zip_figure_points->zip_points_count + 3,
			      print_x_point, print_y_point, points );
    free( points );
    }
  OUT(zipoarrow::Print_Object);
  return  status;
  }

long
zipoarrow::Within_Object( zip_type_figure		   figure, zip_type_pane		   pane, zip_type_pixel		   x , zip_type_pixel		   y )
          {
  long				  distance = -1;

  IN(zipoarrow::Within_Object);
/*===*/
  OUT(zipoarrow::Within_Object);
  return  distance;
  }

boolean
zipoarrow::Enclosed_Object( zip_type_figure		   figure, zip_type_pane		   pane, zip_type_pixel		   x , zip_type_pixel		   y , zip_type_pixel		   w , zip_type_pixel		   h )
          {
  boolean			  enclosed = true;
  zip_type_pixel		  X, Y;
  long				  i;

  IN(zipoarrow::Enclosed_Object);
  X = window_x_point;  Y = window_y_point;
  if ( X < x  ||  X > (x + w)  ||  Y < y  ||  Y > (y + h) )
    enclosed = false;
  for ( i = 0; enclosed == true  &&  i < figure->zip_figure_points->zip_points_count; i++ )
    {
    X = window_x_points(i);  Y = window_y_points(i);
    if (X < x  ||  X > (x + w)  ||  Y < y  ||  Y > (y + h) )
      enclosed = false;
    }
  OUT(zipoarrow::Enclosed_Object);
  return  enclosed;
  }

long
zipoarrow::Object_Enclosure( zip_type_figure		   figure, zip_type_pane		   pane, zip_type_pixel		  *x , zip_type_pixel		  *y , zip_type_pixel		  *w , zip_type_pixel		  *h )
          {
  zip_type_pixel		  max_x, min_x, max_y, min_y, X, Y;
  long				  i;

  IN(zipoarrow::Object_Enclosure);
  max_x = min_x = window_x_point;  max_y = min_y = window_y_point;
  for ( i = 0; i < figure->zip_figure_points->zip_points_count; i++ )
    {
    if ( (X = window_x_points(i)) > max_x )
      max_x = X;
      else
      if ( X < min_x )
        min_x = X;
    if ( (Y = window_y_points(i)) > max_y )
      max_y = Y;
      else
      if ( Y < min_y )
        min_y = Y;
    }
  *x = min_x;  *y = min_y;
  *w = abs(max_x - min_x);
  *h = abs(max_y - min_y);
  OUT(zipoarrow::Object_Enclosure);
  return  zip_ok;
  }

long
zipoarrow::Proximate_Object_Points( zip_type_figure		   figure, zip_type_pane		   pane, zip_type_pixel		   x , zip_type_pixel		   y )
          {
  long				  point = 0, i;

  IN(zipoarrow::Proximate_Object_Points);
  if ( (this->view_object)->Proximate_Figure_Point(  pane, figure,
				       window_x_point, window_y_point, x, y ) )
    point = zip_figure_origin_point;
    else
    for ( i = 0; i < figure->zip_figure_points->zip_points_count; i++ )
      if (  (this->view_object)->Proximate_Figure_Point(  pane, figure,
				     window_x_points(i),
				     window_y_points(i), x, y ) )
	{
	point = zip_figure_auxiliary_point + i;
        break;
	}
  OUT(zipoarrow::Proximate_Object_Points);
  return  point;
  }

long
zipoarrow::Highlight_Object_Points( zip_type_figure		   figure, zip_type_pane		   pane )
        {
  long				  status = zip_ok, i;

  IN(zipoarrow::Highlight_Object_Points);
  (this->edit_object)->Highlight_Point(  pane, window_x_point, window_y_point );
    for ( i = 0; i < figure->zip_figure_points->zip_points_count; i++ )
      (this->edit_object)->Highlight_Point(  pane, window_x_points(i), window_y_points(i) );
  OUT(zipoarrow::Highlight_Object_Points);
  return  status;
  }

long
zipoarrow::Normalize_Object_Points( zip_type_figure		   figure, zip_type_pane		   pane )
        {
  long				  status = zip_ok, i;

  IN(zipoarrow::Normalize_Object_Points);
  (this->edit_object)->Normalize_Point(  pane, window_x_point, window_y_point );
    for ( i = 0; i < figure->zip_figure_points->zip_points_count; i++ )
      (this->edit_object)->Normalize_Point(  pane, window_x_points(i), window_y_points(i) );
  OUT(zipoarrow::Normalize_Object_Points);
  return  status;
  }

long
zipoarrow::Expose_Object_Points( zip_type_figure		   figure, zip_type_pane		   pane )
        {
  long				  status = zip_ok, i;

  IN(zipoarrow::Expose_Object_Points);
  (this->edit_object)->Expose_Point(  pane, figure, figure_x_point, figure_y_point );
  for ( i = 0; i < figure->zip_figure_points->zip_points_count; i++ )
      (this->edit_object)->Expose_Point(  pane, figure, figure_x_points(i), figure_y_points(i) );
  OUT(zipoarrow::Expose_Object_Points);
  return  status;
  }

long
zipoarrow::Hide_Object_Points( zip_type_figure		   figure, zip_type_pane		   pane )
        {
  long				  status = zip_ok, i;

  IN(zipoarrow::Hide_Points);
  (this->edit_object)->Hide_Point(  pane, figure, figure_x_point, figure_y_point );
    for ( i = 0; i < figure->zip_figure_points->zip_points_count; i++ )
     (this->edit_object)->Hide_Point(  pane, figure, figure_x_points(i), figure_y_points(i) );
  OUT(zipoarrow::Hide_Points);
  return  status;
  }

long
zipoarrow::Set_Object_Point( zip_type_figure		   figure, int				   point, zip_type_point		   x , zip_type_point		   y )
          {
  long				  status = zip_ok;

  IN(zipoarrow::Set_Object_Point);
  if ( point == zip_figure_origin_point )
    {
    figure->zip_figure_point.zip_point_x = x;
    figure->zip_figure_point.zip_point_y = y;
    }
  else
  if ( point == zip_figure_auxiliary_point )
    {
    if ( figure->zip_figure_points == NULL )
      if ( (status = (this->data_object)->Allocate_Figure_Points_Vector(  &figure->zip_figure_points )) == zip_ok )
	figure->zip_figure_points->zip_points_count = 1;
    if ( status == zip_ok )
      {
      figure->zip_figure_points->zip_points[0].zip_point_x = x; 
      figure->zip_figure_points->zip_points[0].zip_point_y = y;
      }
    }
  else
  if ( (point - zip_figure_auxiliary_point) < figure->zip_figure_points->zip_points_count )
    {
    figure->zip_figure_points->zip_points[point -
	zip_figure_auxiliary_point].zip_point_x = x;
    figure->zip_figure_points->zip_points[point -
	zip_figure_auxiliary_point].zip_point_y = y;
    }
  else
  if ( (point - zip_figure_auxiliary_point) == figure->zip_figure_points->zip_points_count )
    {
    if ( (status = (this->data_object)->Enlarge_Figure_Points_Vector(  &figure->zip_figure_points )) == zip_ok )
      {
      figure->zip_figure_points->zip_points[point - zip_figure_auxiliary_point].zip_point_x = x;
      figure->zip_figure_points->zip_points[point - zip_figure_auxiliary_point].zip_point_y = y;
      figure->zip_figure_points->zip_points_count++;
      }
    }
  else status = zip_failure; /*=== zip_invalid_point_type (more than 1 beyond allocation ===*/
  if ( status == zip_ok )
    {
    (this->data_object)->Set_Image_Extrema(  figure->zip_figure_image, x, y );
    (this->data_object)->Set_Stream_Extrema(  figure->zip_figure_image->zip_image_stream,
			    figure->zip_figure_image );
    }
  OUT(zipoarrow::Set_Object_Point);
  return  status;
  }

long
zipoarrow::Object_Point( zip_type_figure		   figure, long				   point, zip_type_point		   *x , zip_type_point		   *y )
          {
  long				  status = zip_ok;

  IN(zipoarrow::Object_Point);
/*===*/*x = *y = 0;
  OUT(zipoarrow::Object_Point);
  return  status;
  }

long
zipoarrow::Adjust_Object_Point_Suite( zip_type_figure		   figure, zip_type_point		   x_delta , zip_type_point		   y_delta )
        {
  long				  status = zip_ok, i;

  IN(zipoarrow::Adjust_Object_Point_Suite);
  figure_x_point += x_delta;
  figure_y_point += y_delta;
  for ( i = 0; i < figure->zip_figure_points->zip_points_count; i++ )
    {
    figure->zip_figure_points->zip_points[i].zip_point_x += x_delta;
    figure->zip_figure_points->zip_points[i].zip_point_y += y_delta;
    (this->data_object)->Set_Image_Extrema(  figure->zip_figure_image,
		figure->zip_figure_points->zip_points[i].zip_point_x,
		figure->zip_figure_points->zip_points[i].zip_point_y  );
/*===handle both extrema 7/20/86===*/
/*===have extrema check for REDUCTIONS as well as EXPANSIONS 7/20/86===*/
    (this->data_object)->Set_Stream_Extrema(  figure->zip_figure_image->zip_image_stream,
			    figure->zip_figure_image );
    }
  OUT(zipoarrow::Adjust_Object_Point_Suite);
  return  status;
  }

