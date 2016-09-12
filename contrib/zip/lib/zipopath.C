/**  SPECIFICATION -- Internal Facility Suite  *********************************

TITLE	Zip Object -- Path

MODULE	zipopath.c

NOTICE	IBM Internal Use Only

DESCRIPTION

.......................

HISTORY
  04/13/88	Created (TC Peters)
  05/01/89	Use symbolic font-names (TCP)
  05/15/89	Improve performance by pre-checking TransferMode (TCP)
  08/24/89	Remove excess SetTransferMode() activity in Draw() (SCG)
  09/08/89	Fix Draw() to handle rubberbanding in both Builds and Edit Modifications (SCG)
  07/12/90	Have Object_Point return first coordinate (SCG)
  08/16/90	Use Ensure_Attributes on Draw and Print. Add Contains method (SCG)

END-SPECIFICATION  ************************************************************/


#include <andrewos.h>
#include <view.H>
#include "zip.H"
#include "zipview.H"
#include "zipedit.H"
#include "zipprint.H"
#include "zipobject.H"
#include "zipopath.H"

ATKdefineRegistry(zipopath, zipobject, NULL);

static long Draw( class zipopath		  *self, zip_type_figure		   figure, zip_type_pane		   pane, short			   action );
static long signum(long  a);
static long same( zip_type_point_pair		 p1 , zip_type_point_pair		 p2 , zip_type_point_pair		 p3 , zip_type_point_pair		 p4 );
static boolean intersect( zip_type_point_pair		 p1 , zip_type_point_pair		 p2 , zip_type_point_pair		 p3 , zip_type_point_pair		 p4 );


char
zipopath::Object_Icon( )
    {
  IN(zipopath::Object_Icon);
  OUT(zipopath::Object_Icon);
  return  'F';
  }

char
zipopath::Object_Icon_Cursor( )
    {
  IN(zipopath::Object_Icon_Cursor);
  OUT(zipopath::Object_Icon_Cursor);
  return  'L';
  }

char
zipopath::Object_Datastream_Code( )
    {
  IN(zipopath::Object_Datastream_Code);
  OUT(zipopath::Object_Datastream_Code);
  return  'H';
  }

long
zipopath::Show_Object_Properties( zip_type_pane		   pane, zip_type_figure		   figure )
        {
  (this->view_object)->Announce(  "Draw Path from Start to Finish in free-form." );
  return  zip_ok;
  }

long
zipopath::Build_Object( zip_type_pane		   pane, enum view_MouseAction				   action , long				   x , long				   y , long				   clicks, zip_type_point		   X , zip_type_point		   Y )
          {
  long				  status = zip_ok;
  static long				  initial_x, initial_y,
					  initial_X, initial_Y,
					  prior_X, prior_Y;

  IN(zipopath::Build_Object);
  switch ( action )
    {
    case view_LeftDown:
      initial_x = x;  initial_y = y;
      initial_X = prior_X = X;  initial_Y = prior_Y = Y;
      if ( (status = (this->data_object)->Create_Figure(  &CurrentFigure, NULL,
		zip_path_figure, CurrentImage, NULL )) == zip_success )
        {
        (this)->Set_Object_Point(  CurrentFigure,
		zip_figure_origin_point, X, Y );
  	CurrentFigure->zip_figure_zoom_level =
	    pane->zip_pane_zoom_level;
	pane->zip_pane_edit->zip_pane_edit_build_figure = false;
	pane->zip_pane_edit->zip_pane_edit_last_point_id = zip_figure_origin_point;
	(this->data_object)->Set_Figure_Shade(  CurrentFigure,
			      pane->zip_pane_edit->zip_pane_edit_current_shade );
	}
      break;
    case view_LeftUp:
      if ( pane->zip_pane_edit->zip_pane_edit_last_point_id == zip_figure_origin_point )
	{
	(this->edit_object)->Delete_Figure(  CurrentFigure, pane );
	break;
	}
      if ( abs(initial_x - x) <= 10  &&  abs(initial_y - y) <= 10 )
	{ DEBUG(Close Path); X = initial_X;  Y = initial_Y; }
      /* Fall-thru */
    case view_LeftMovement:
      if ( CurrentFigure )
	{
	if ( X != prior_X  ||  Y != prior_Y )
	  {
          (this)->Set_Object_Point(  CurrentFigure,
		     ++pane->zip_pane_edit->zip_pane_edit_last_point_id,
		     prior_X = X, prior_Y = Y );
          (this->view_object)->Draw_Figure(  CurrentFigure, pane );
	  pane->zip_pane_edit->zip_pane_edit_build_figure = false;
	  }
	}
      break;
    default:
      break;
    }
  OUT(zipopath::Build_Object);
  return  status;
  }

long
zipopath::Draw_Object( zip_type_figure		   figure, zip_type_pane		   pane )
        {
  long				  status = zip_ok;

  if ( (this->view_object)->Condition(  pane, figure, zip_draw ) )
    status = Draw( this, figure, pane, zip_draw );
  OUT(zipopath::Draw_Object);
  return  status;
  }

long
zipopath::Clear_Object( zip_type_figure		   figure, zip_type_pane		   pane )
        {
  long				  status = zip_ok;

  IN(zipopath::Clear_Object);
  if ( (this->view_object)->Condition(  pane, figure, zip_clear ) )
    status = Draw( this, figure, pane, zip_clear );
  OUT(zipopath::Clear_Object);
  return  status;
  }

static
long Draw( class zipopath		  *self, zip_type_figure		   figure, zip_type_pane		   pane, short			   action )
          {
  long				  status = zip_ok, i, count, id;
  unsigned char		  shade, allocated = false,
					  closure = false;
  struct point				  point_vector[101];
  struct point			 *points = point_vector;

  IN(Draw);
  count = figure->zip_figure_points->zip_points_count + 1;
  DEBUGdt(Point-count,count);
  if ( count > 100 )
    {
    allocated = true;
    points = (struct point *) malloc( count * sizeof(struct point) );
    }
  points[0].x = window_x_point;  points[0].y = window_y_point;
  DEBUGdt(  X,points[0].x); DEBUGdt(  Y,points[0].y);
  for ( i = 1; i < count; i++ )
    {
    points[i].x = window_x_points(i-1);
    points[i].y = window_y_points(i-1);
    DEBUGdt(X,points[i].x); DEBUGdt(Y,points[i].y);
    }
  if ( points[0].x == points[count-1].x  &&  points[0].y == points[count-1].y )
    closure = true;
  if ( closure  && (shade = (self->data_object)->Contextual_Figure_Shade(  figure )) )
    {
    if ( self->view_object->mouse_action != view_LeftMovement  &&  action == zip_draw )
      {
      if ( (shade = ('0' + ((shade + 10) / 10)) - 1) > '9' )  shade = '9';
      DEBUGdt(Shade-index,shade);
      (self->view_object)->Ensure_Fill_Attributes(  figure );
      (self->view_object)->FillPolygon(  points, count, (self->view_object)->Define_Graphic(  (self->data_object)->Define_Font(  ShadeFontName, NULL ), shade ) );
      }
      else
      if ( action == zip_clear )
	{
        (self->view_object)->FillPolygon(  points, count,
          (self->view_object)->Define_Graphic(  (self->data_object)->Define_Font(  ShadeFontName, NULL ), shade ) );
	}
    }
  if ( (self->view_object)->Ensure_Line_Attributes(  figure ) == zip_ok )
    {
    if ( pane->zip_pane_edit && pane->zip_pane_state.zip_pane_state_paint_inverted )
     {
	id = pane->zip_pane_edit->zip_pane_edit_last_point_id - 1;
        if ( id == count - 1 )
	{
	  (self->view_object)->MoveTo(  points[id - 1].x, points[id - 1].y );
	  (self->view_object)->DrawLineTo(  points[id].x, points[id].y );
	  if ( closure ) (self->view_object)->DrawLineTo(  points[0].x, points[0].y );
	}
        else
	{
          if ( id )
          {
  	    (self->view_object)->MoveTo(  points[id - 1].x, points[id - 1].y );
	    (self->view_object)->DrawLineTo(  points[id].x, points[id].y );
	    if ( id + 1  < count )
	      (self->view_object)->DrawLineTo(  points[id + 1].x, points[id + 1].y );
	  }
          else
	  {
	    (self->view_object)->MoveTo(  points[1].x, points[1].y );
  	    (self->view_object)->DrawLineTo(  points[0].x, points[0].y );
            if ( closure )
  	     (self->view_object)->DrawLineTo(  points[count - 2].x, points[count - 2].y );
	  }
	}
     }
     else if ( closure ) (self->view_object)->DrawPolygon(  points, count );
     else (self->view_object)->DrawPath(  points, count );
    }
  if ( ExposePoints )
    (self)->Expose_Object_Points(  figure, pane );
  if ( HighlightPoints )
    (self)->Highlight_Object_Points(  figure, pane );
  if ( allocated )   free( points );
  OUT(Draw);
  return  status;
  }

long
zipopath::Print_Object( zip_type_figure figure, zip_type_pane pane )
{
  class zipopath *self=this;
  long				  pc, status = zip_ok;
  zip_type_point_pairs	    		  points = NULL;

  IN(zipopath::Print_Object);
  if ( (status = (this->data_object)->Allocate_Figure_Points_Vector(  &points )) == zip_success )
    {
    points->zip_points_count = 0;
    for ( pc = 0; pc < figure->zip_figure_points->zip_points_count; pc++ )
      {
      if ( status == zip_success )
        {
        points->zip_points[pc].zip_point_x = print_x_points(pc);
        points->zip_points[pc].zip_point_y = print_y_points(pc);
        points->zip_points_count++;
        status = (this->data_object)->Enlarge_Figure_Points_Vector(  &points );
        }
      }
    (this->print_object)->Set_Shade(  figure->zip_figure_fill.zip_figure_shade );
    (this->print_object)->Ensure_Line_Attributes(  figure );
    (this->print_object)->Draw_Multi_Line(  figure->zip_figure_points->zip_points_count,
			      print_x_point, print_y_point, points );
    free( points );
    }
  OUT(zipopath::Print_Object);
  return  status;
  }

long
zipopath::Within_Object( zip_type_figure figure, zip_type_pane pane, zip_type_pixel x, zip_type_pixel y )
{
  long				  distance = -1,
					  X1, Y1, X2, Y2,
					  X, Y, XA, XB, YA, YB;

  IN(zipopath::Within_Object);
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
  OUT(zipopath::Within_Object);
  return  distance;
  }

long
zipopath::Proximate_Object_Points( zip_type_figure		   figure, zip_type_pane		   pane, zip_type_pixel		   x , zip_type_pixel		   y )
          {
  int				  i, point = 0;

  IN(zipopath::Proximate_Object_Points);

  if ( (this->view_object)->Proximate_Figure_Point(  pane, figure,
				       window_x_point, window_y_point, x, y ) )
    point = zip_figure_origin_point;
    else
    for ( i = 0;
	   i < figure->zip_figure_points->zip_points_count;
	    i++ )
      if (  (this->view_object)->Proximate_Figure_Point(  pane, figure,
				     window_x_points(i),
				     window_y_points(i), x, y ) )
	{
	point = zip_figure_auxiliary_point + i;
        break;
	}

  OUT(zipopath::Proximate_Object_Points);
  return  point;
  }

boolean
zipopath::Enclosed_Object( zip_type_figure		   figure, zip_type_pane		   pane, zip_type_pixel		   x , zip_type_pixel		   y , zip_type_pixel		   w , zip_type_pixel		   h )
          {
  boolean			  enclosed = true;
  long				  i, right = x + w, bottom = y + h;
  zip_type_pixel		  X, Y;

  IN(zipopath::Enclosed_Object);
  X = window_x_point;  Y = window_y_point;
  if ( X < x  ||  Y < y  ||  X > right  ||  Y > bottom )
    enclosed = false;
  for ( i = 0; enclosed == true  &&
	i < figure->zip_figure_points->zip_points_count; i++ )
    {
    X = window_x_points(i);  Y = window_y_points(i);
    if ( X < x  ||  Y < y  ||  X > right  ||  Y > bottom )
      enclosed = false;
    }
  OUT(zipopath::Enclosed_Object);
  return  enclosed;
  }

long
zipopath::Object_Enclosure( zip_type_figure figure, zip_type_pane pane, zip_type_pixel *x, zip_type_pixel *y, zip_type_pixel *w, zip_type_pixel *h )
{
  zip_type_pixel		  max_x, min_x, max_y, min_y, X, Y;
  long				  i;

  IN(zipopath::Object_Enclosure);
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
  OUT(zipopath::Object_Enclosure);
  return  zip_ok;
  }

long
zipopath::Highlight_Object_Points( zip_type_figure		   figure, zip_type_pane		   pane )
        {
  long				  i, status = zip_ok;

  IN(zipopath::Highlight_Object_Points);
  (this->edit_object)->Highlight_Point(  pane, window_x_point, window_y_point );
  for ( i = 0; i < figure->zip_figure_points->zip_points_count; i++ )
    (this->edit_object)->Highlight_Point(  pane, window_x_points(i), window_y_points(i) );
  OUT(zipopath::Highlight_Object_Points);
  return  status;
  }

long
zipopath::Normalize_Object_Points( zip_type_figure		   figure, zip_type_pane		   pane )
        {
  long				  i, status = zip_ok;

  IN(zipopath::Normalize_Object_Points);
  (this->edit_object)->Normalize_Point(  pane, window_x_point, window_y_point );
  for ( i = 0; i < figure->zip_figure_points->zip_points_count; i++ )
    (this->edit_object)->Normalize_Point(  pane, window_x_points(i), window_y_points(i) );
  OUT(zipopath::Normalize_Object_Points);
  return  status;
  }

long
zipopath::Expose_Object_Points( zip_type_figure		   figure, zip_type_pane		   pane )
        {
  long				  i, status = zip_ok;

  IN(zipopath::Expose_Object_Points);
  (this->edit_object)->Expose_Point(  pane, figure, figure_x_point, figure_y_point );
  for ( i = 0; i < figure->zip_figure_points->zip_points_count; i++ )
    (this->edit_object)->Expose_Point(  pane, figure, figure_x_points(i), figure_y_points(i) );
  OUT(zipopath::Expose_Object_Points);
  return  status;
  }

long
zipopath::Hide_Object_Points( zip_type_figure		   figure, zip_type_pane		   pane )
        {
  long				  i, status = zip_ok;

  IN(zipopath::Hide_Points);
  (this->edit_object)->Hide_Point(  pane, figure, figure_x_point, figure_y_point );
  for ( i = 0; i < figure->zip_figure_points->zip_points_count; i++ )
    (this->edit_object)->Hide_Point(  pane, figure, figure_x_points(i), figure_y_points(i) );
  OUT(zipopath::Hide_Points);
  return  status;
  }

long
zipopath::Set_Object_Point( zip_type_figure		   figure, int				   point, zip_type_point		   x , zip_type_point		   y )
          {
  long				  status = zip_ok;

  IN(zipopath::Set_Object_Point);
  if ( point == zip_figure_origin_point )
    {
    if ( (this->data_object)->Contextual_Figure_Shade(  figure ) &&
         figure->zip_figure_points  &&
	 figure->zip_figure_points->zip_points_count > 2  &&
         figure->zip_figure_points->zip_points
	  [figure->zip_figure_points->zip_points_count - 1].zip_point_x ==
	    figure->zip_figure_point.zip_point_x  &&
         figure->zip_figure_points->zip_points
	  [figure->zip_figure_points->zip_points_count - 1].zip_point_y ==
	    figure->zip_figure_point.zip_point_y
	)
      { DEBUG(Closed Filled Figure);
      figure->zip_figure_points->zip_points
	[figure->zip_figure_points->zip_points_count - 1].zip_point_x = x;
      figure->zip_figure_points->zip_points
	[figure->zip_figure_points->zip_points_count - 1].zip_point_y = y;
      }
    figure->zip_figure_point.zip_point_x = x;
    figure->zip_figure_point.zip_point_y = y;
    }
  else
  if ( point == zip_figure_auxiliary_point )
    {
    if ( figure->zip_figure_points == NULL )
      if ( (status = (this->data_object)->Allocate_Figure_Points_Vector(
		     &figure->zip_figure_points )) == zip_success )
	figure->zip_figure_points->zip_points_count = 1;
    if ( status == zip_success )
      {
      figure->zip_figure_points->zip_points[0].zip_point_x = x; 
      figure->zip_figure_points->zip_points[0].zip_point_y = y;
      }
    }
  else
  if ( (point - zip_figure_auxiliary_point) <
       figure->zip_figure_points->zip_points_count )
    {
    figure->zip_figure_points->zip_points[point -
	zip_figure_auxiliary_point].zip_point_x = x;
    figure->zip_figure_points->zip_points[point -
	zip_figure_auxiliary_point].zip_point_y = y;
    }
  else
  if ( (point - zip_figure_auxiliary_point) == figure->zip_figure_points->zip_points_count )
    {
    if ( (status = (this->data_object)->Enlarge_Figure_Points_Vector(
	     &figure->zip_figure_points )) == zip_success )
      {
      figure->zip_figure_points->zip_points[point - zip_figure_auxiliary_point].zip_point_x = x;
      figure->zip_figure_points->zip_points[point - zip_figure_auxiliary_point].zip_point_y = y;
      figure->zip_figure_points->zip_points_count++;
      }
    }
  else status = zip_failure; /*=== zip_invalid_point_type (more than 1 beyond allocation) ===*/
  if ( status == zip_success )
    {
    (this->data_object)->Set_Image_Extrema(  figure->zip_figure_image, x, y );
    (this->data_object)->Set_Stream_Extrema(  figure->zip_figure_image->zip_image_stream,
			    figure->zip_figure_image );
    }
  OUT(zipopath::Set_Object_Point);
  return  status;
  }

long
zipopath::Object_Point( zip_type_figure		   figure, long				   point, zip_type_point		   *x , zip_type_point		   *y )
          {
  long				  status = zip_ok;

  IN(zipopath::Object_Point);
  if ( figure )
    { /* just return the first coordinate */
    *x = figure_x_point;
    *y = figure_y_point;
    }
  else *x = *y = 0;
  OUT(zipopath::Object_Point);
  return  status;
  }

long
zipopath::Adjust_Object_Point_Suite( zip_type_figure		   figure, zip_type_point		   x_delta , zip_type_point		   y_delta )
        {
  long				  i, status = zip_ok;

  IN(zipopath::Adjust_Object_Point_Suite);
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
  OUT(zipopath::Adjust_Object_Point_Suite);
  return  status;
  }

long
zipopath::Set_Object_Shade( zip_type_figure		   figure, unsigned char shade )
      {
  IN(zipopath::Set_Object_Shade);
  figure->zip_figure_fill.zip_figure_shade = shade;
  if ( shade >= 1  &&  shade <= 100 )
    figure->zip_figure_mode.zip_figure_mode_shaded = on;
    else
    figure->zip_figure_mode.zip_figure_mode_shaded = off;
  OUT(zipopath::Set_Object_Shade);
  return  zip_ok;
  }

static long
signum(long  a)
{
  if (a < 0) return -1;
  else if (a > 0) return 1;
  else return 0;
}

static long
same( zip_type_point_pair		 p1 , zip_type_point_pair		 p2 , zip_type_point_pair		 p3 , zip_type_point_pair		 p4 )
{
long				dx, dx1, dx2, dy, dy1, dy2;

  dx = p2.zip_point_x - p1.zip_point_x;
  dx1 = p3.zip_point_x - p1.zip_point_x;
  dx2 = p4.zip_point_x - p2.zip_point_x;
  dy = p2.zip_point_y - p1.zip_point_y;
  dy1 = p3.zip_point_y - p1.zip_point_y;
  dy2 = p4.zip_point_y - p2.zip_point_y;
  return (signum(dx*dy1 - dy*dx1)*signum(dx*dy2 - dy*dx2 ));
}

static boolean
intersect( zip_type_point_pair		 p1 , zip_type_point_pair		 p2 , zip_type_point_pair		 p3 , zip_type_point_pair		 p4 )
{
boolean			status = FALSE;

  if (( same( p1, p2, p3, p4 ) <= 0 ) && ( same( p3, p4, p1, p2 ) <= 0 )) status = TRUE;
  return status;
}

/* Closed poly-lines need to be normalized, to work in all cases */
boolean
zipopath::Contains( zip_type_figure		 figure, zip_type_pane			 pane, zip_type_pixel			 x, zip_type_pixel			 y )
{
long				i, j = 0, count = 0;
boolean			status = FALSE;
zip_type_point_pair		p1, p2, p3, p4;

  IN( zipopath::Contains );
  p1.zip_point_x = x;
  p1.zip_point_y = p2.zip_point_y = y;
  p2.zip_point_x = 10000;
  for( i = 1; i < figure->zip_figure_points->zip_points_count + 1; i++ )
  {
    p3.zip_point_x = p4.zip_point_x = window_x_points(i-1);
    p3.zip_point_y = p4.zip_point_y = window_y_points(i-1);
    if ( intersect( p3, p4, p1, p2 ) == FALSE )
    {
      if ( j == 0 )
      {
        p4.zip_point_x = window_x_point;
        p4.zip_point_y = window_y_point;
      }
      else
      {
        p4.zip_point_x = window_x_points(j-1);
        p4.zip_point_y = window_y_points(j-1);
      }
      j = i;
      if ( intersect( p3, p4, p1, p2 ) == TRUE ) count++;
    }
  }
  if ( count % 2 == 1 ) status = TRUE;
  OUT( zipopath::Contains );
  return status;
}
