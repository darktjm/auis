/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'     *
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

/* zipopoly.c	Zip Object -- Polygon				      */
/* Author	TC Peters						      */
/* Information Technology Center		   Carnegie-Mellon University */

/**  SPECIFICATION -- Internal Facility Suite  *********************************

TITLE	Zip Object -- Polygon

MODULE	zipopoly.c

NOTICE	IBM Internal Use Only

DESCRIPTION

.......................

HISTORY
  04/13/88	Created (TC Peters)
  05/01/89	Use symbolic font-names (TCP)
  08/24/89	Remove excess SetTransferMode() activity in Draw() (SCG)
  08/24/89	Modify Build to handle non-refresh of pane on build completion (SCG)
   08/16/90	Use Ensure_Attribute on Draw and Print (SCG)

END-SPECIFICATION  ************************************************************/

#include <andrewos.h>
#include <math.h>

#include <view.H>
#include "zip.H"
#include "zipview.H"
#include "zipedit.H"
#include "zipprint.H"
#include "zipobject.H"
#include "zipopolygon.H"

static long				  sides;


ATKdefineRegistry(zipopolygon, zipocirc, NULL);

#if (defined(MACH) && defined(i386))
int calltosavecompile (zip_type_pane  pane, zip_type_figure  figure, class zipopolygon  *self);
#endif
static long Draw( class zipopolygon  *self, zip_type_figure   figure, zip_type_pane   pane, short   action );


char
zipopolygon::Object_Icon( )
    {
  IN(zipopolygon::Object_Icon);
  OUT(zipopolygon::Object_Icon);
  return  'N';
  }

char
zipopolygon::Object_Icon_Cursor( )
    {
  IN(zipopolygon::Object_Icon_Cursor);
  OUT(zipopolygon::Object_Icon_Cursor);
  return  'G';
  }

char
zipopolygon::Object_Datastream_Code( )
    {
  IN(zipopolygon::Object_Datastream_Code);
  OUT(zipopolygon::Object_Datastream_Code);
  return  'E';
  }

long
zipopolygon::Show_Object_Properties( zip_type_pane		   pane, zip_type_figure		   figure )
        {
  char					 *response;

  (this->view_object)->Set_Pane_Cursor(  pane, '@', IconFontName );
  sides = 0;
  CurrentFigure = NULL;
  (this->view_object)->Query(  "Enter Number of Polygon Sides [3]: ",
					NULL, &response );
  DEBUGst(Response,response);
  sides = atoi( response );
  DEBUGdt(Sides,sides);
  if ( sides < 3 )  sides = 3;
  (this->view_object)->Announce(  "Draw Polygon from Center outward." );
  (this->view_object)->Set_Pane_Cursor(  pane, 'G', CursorFontName );
  return  zip_ok;
  }

long
zipopolygon::Build_Object( zip_type_pane		   pane, enum view_MouseAction				   action , long				   x , long				   y , long				   clicks, zip_type_point		   X , zip_type_point		   Y )
          {
  long				  status = zip_ok,
					  radial_point = 0;
  zip_type_figure		  figure;

  IN(zipopolygon::Build_Object);
  switch ( action )
    {
    case view_LeftDown:
      if ( (status =
        (this->data_object)->Create_Figure(  &CurrentFigure, NULL, zip_polygon_figure,
			   CurrentImage, 0 )) == zip_success )
	{ DEBUG(Created);
        (this)->Set_Object_Point(  CurrentFigure,
				      zip_figure_origin_point, X, Y );
	(this)->Set_Object_Point(  CurrentFigure,
				      zip_figure_auxiliary_point, X, Y );
	figure = CurrentFigure;
	figure_x_points(0) = figure_y_points(0) = 0;
	figure->zip_figure_zoom_level = pane->zip_pane_zoom_level;
        pane->zip_pane_edit->zip_pane_edit_last_point_id = zip_figure_auxiliary_point;
	(this->data_object)->Set_Figure_Shade(  figure,
			      pane->zip_pane_edit->zip_pane_edit_current_shade );
	(this->view_object)->Set_Pane_Painting_Mode(  pane, zipview_paint_inverted );
	}
      break;
    case view_LeftUp:
      if ( ( figure = CurrentFigure ) )
	{
	DEBUGdt(Radius,figure_x_points(0));
	DEBUGdt(Sides,figure_y_points(0));
        (this->view_object)->Set_Pane_Painting_Mode(  pane, zip_default );
	if ( figure_x_points(0) == 0  ||  figure_y_points(0) < 3 )
	  {
	  (this->view_object)->Clear_Figure(  figure, pane );
	  (this->data_object)->Destroy_Figure(  figure );
	  CurrentFigure = NULL;
	  }
          else
          {
	    (this->view_object)->Draw_Figure(  figure, pane );
	  }
	}
	break;
    case view_LeftMovement:
      if ( (figure = CurrentFigure)  &&  status == zip_ok )
	{
	(this->view_object)->Draw_Figure(  figure, pane );
        if ( abs( X - figure_x_point ) < abs( Y - figure_y_point ) )
	  radial_point = figure_x_point + abs( Y - figure_y_point );
	  else
	  radial_point = X;
        (this)->Set_Object_Point(  figure,
				      zip_figure_auxiliary_point, radial_point, 0 );
	figure_y_points(0) = sides;
	(this->view_object)->Draw_Figure(  figure, pane );
	}
      break;
    default:
      break;
    }
  OUT(zipopolygon::Build_Object);
  return  status;
  }

long
zipopolygon::Draw_Object( zip_type_figure		   figure, zip_type_pane		   pane )
        {
  long				  status = zip_ok;

  IN(zipopolygon::Draw_Object);
  if ( (this->view_object)->Condition(  pane, figure, zip_draw ) )
    status = Draw( this, figure, pane, zip_draw );
  OUT(zipopolygon::Draw_Object);
  return  status;
  }

long
zipopolygon::Clear_Object( zip_type_figure		   figure, zip_type_pane		   pane )
        {
  long				  status = zip_ok;

  IN(zipopolygon::Clear_Object);
  if ( (this->view_object)->Condition(  pane, figure, zip_clear ) )
    status = Draw( this, figure, pane, zip_clear );
  OUT(zipopolygon::Clear_Object);
  return  status;
  }

#if (defined(MACH) && defined(i386))
int calltosavecompile (zip_type_pane  pane, zip_type_figure  figure, class zipopolygon  *self)
{
return (abs( (this->view_object)->X_Point_To_Pixel(  pane, figure,
		figure_x_point + figure_x_points(0) ) - window_x_point ));
}
#endif

static
long Draw( class zipopolygon  *self, zip_type_figure   figure, zip_type_pane   pane, short   action )
{
  long				  status = zip_ok, i,
					  sides, radius;
  unsigned char			pattern = 0, shade,
                                          allocated = false;
  short			  even = true;
  struct point				  point_vector[9];
  struct point			 *points = point_vector;
  double			  theta = 0.0, angle;

  IN(Draw);
  DEBUGdt(Sides,sides);
  if ( (sides = figure_y_points(0)) >= 3 )
    {
    if ( sides > 8 )
      {
      allocated = true;
      points = (struct point *) malloc( (sides + 1) * sizeof(struct point) );
      }
    if ( sides % 2 )
      { DEBUG(Odd);
      even = false;
      theta = -M_PI/2;
      }
#if (defined(MACH) && defined(i386))
    radius = calltosavecompile(pane,figure,self);
#else
    radius = abs( (self->view_object)->X_Point_To_Pixel(  pane, figure,
		figure_x_point + figure_x_points(0) ) - window_x_point );
#endif
    points[0].x = points[sides].x = window_x_point + ((even) ? radius : 0);
    DEBUGdt(X,points[0].x);
    points[0].y = points[sides].y = window_y_point - ((even) ? 0 : radius);
    DEBUGdt(Y,points[0].y);
    angle = (2.0 * M_PI) / sides;  DEBUGgt(Angle,angle);
    for ( i = 1; i < sides; i++ )
      {
      theta += angle;  DEBUGgt(Theta,theta);
      points[i].x = (long) (window_x_point + radius * cos(theta)); DEBUGdt(X,points[i].x);
      points[i].y = (long) (window_y_point + radius * sin(theta)); DEBUGdt(Y,points[i].y);
      }
    if ( (figure->zip_figure_mode.zip_figure_mode_shaded  ||
          figure->zip_figure_mode.zip_figure_mode_patterned) )
      {
      if ( self->view_object->mouse_action != view_LeftMovement  &&  action == zip_draw )
        {
        if ( figure->zip_figure_mode.zip_figure_mode_patterned  &&
	     (pattern = (self->data_object)->Contextual_Figure_Pattern(  figure )) )
          { DEBUGct(Pattern,pattern);
          (self->view_object)->FillPolygon(  points, sides + 1,
	      (self->view_object)->Define_Graphic( 
	        (self->view_object)->Select_Contextual_Figure_Font(  figure ), pattern ));
          }
          else
          /* Shade of '0' means Transparent --- Shade of '1' means White */
          if ( (shade = (self->data_object)->Contextual_Figure_Shade(  figure )) >= 1  &&
	        shade <= 100 )
	    { DEBUGdt(Shade,shade);
	    if ( (shade = ('0' + ((shade + 10) / 10)) - 1) > '9' )  shade = '9';
	    DEBUGdt(Shade-index,shade);
            (self->view_object)->Ensure_Fill_Attributes(  figure );
	    (self->view_object)->FillPolygon(  points, sides + 1, 
	      (self->view_object)->Define_Graphic(  (self->data_object)->Define_Font(  ShadeFontName, NULL ), shade ) );
	    }
        }
        else
        if ( action == zip_clear )
	  { DEBUG(Clear Action);
	  (self->view_object)->FillPolygon(  points, sides + 1, graphic_WHITE );
	  }
      }
      if ( (self->view_object)->Ensure_Line_Attributes(  figure ) == zip_ok )
      {
      (self->view_object)->DrawPolygon(  points, sides + 1 );
      if ( ExposePoints )
        (self)->Expose_Object_Points(  figure, pane );
      if ( HighlightPoints )
        (self)->Highlight_Object_Points(  figure, pane );
      }
    if ( allocated )   free( points );
    }
  OUT(Draw);
  return  status;
  }

long
zipopolygon::Print_Object( zip_type_figure figure, zip_type_pane pane )
{
  class zipopolygon *self=this;
  long				  pc, status = zip_ok,
					  sides, radius, even = true;
  zip_type_point_pairs	    		  points = NULL;
  double			  theta = 0.0, angle;

  IN(zipopolygon::Print_Object);
  sides = figure_y_points(0);
  radius = print_x_lengths(0);
  if ( (status = (this->data_object)->Allocate_Figure_Points_Vector(  &points )) == zip_ok )
    {
    points->zip_points_count = 0;
    angle = (2.0 * M_PI) / sides;
    if ( sides % 2 )
      {
      even = false;
      theta = -M_PI/2;
      }
    for ( pc = 0; pc < sides - 1  &&  status == zip_ok; pc++ )
      {
      if ( status == zip_success )
        {
        theta += angle;
        points->zip_points[pc].zip_point_x = (long) (print_x_point + radius * cos(theta));
        points->zip_points[pc].zip_point_y = (long) (print_y_point + radius * sin(theta));
        points->zip_points_count++;
        status = (this->data_object)->Enlarge_Figure_Points_Vector(  &points );
        }
      }
    points->zip_points[sides-1].zip_point_x = print_x_point + ((even) ? radius : 0);
    points->zip_points[sides-1].zip_point_y = print_y_point - ((even) ? 0 : radius);
    (this->print_object)->Set_Shade(  figure->zip_figure_fill.zip_figure_shade ); 
    (this->print_object)->Ensure_Line_Attributes(  figure );
    (this->print_object)->Draw_Multi_Line(  sides,
			      print_x_point + ((even) ? radius : 0),
			      print_y_point - ((even) ? 0 : radius), points );
    free( points );
    }
  OUT(zipopolygon::Print_Object);
  return  status;
  }
