/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

/* zipoline.c	Zip Object -- Line					      */
/* Author	TC Peters						      */
/* Information Technology Center		   Carnegie-Mellon University */

/**  SPECIFICATION -- Internal Facility Suite  *********************************

TITLE	Zip Object -- Line

MODULE	zipoline.c

NOTICE	IBM Internal Use Only

DESCRIPTION

.......................

HISTORY
  04/13/88	Created (TC Peters)
   08/16/90	Add Ensure_Attribute usage on Draw and Print. Add Contains method (SCG)

END-SPECIFICATION  ************************************************************/

#include <andrewos.h>
#include "zip.H"
#include "zipview.H"
#include "zipedit.H"
#include "zipprint.H"
#include "zipobject.H"
#include "zipoline.H"

ATKdefineRegistry(zipoline, zipobject, NULL);

static long Draw( class zipoline		  *self, zip_type_figure		   figure, zip_type_pane		   pane );
static void Compute_Handle_Positions( class zipoline		  *self, zip_type_figure		   figure, zip_type_pane		   pane, zip_type_pixel		  *X1 , zip_type_pixel		  *X2 , zip_type_pixel		  *X3 , zip_type_pixel		  *Y1 , zip_type_pixel		  *Y2 , zip_type_pixel		  *Y3 );


char
zipoline::Object_Icon( )
    {
  IN(zipoline::Object_Icon);
  OUT(zipoline::Object_Icon);
  return  'C';
  }

char
zipoline::Object_Icon_Cursor( )
    {
  IN(zipoline::Object_Icon_Cursor);
  OUT(zipoline::Object_Icon_Cursor);
  return  'I';
  }

char
zipoline::Object_Datastream_Code( )
    {
  IN(zipoline::Object_Datastream_Code);
  OUT(zipoline::Object_Datastream_Code);
  return  'C';
  }

long
zipoline::Show_Object_Properties( zip_type_pane		   pane, zip_type_figure		   figure )
        {
  (this->view_object)->Announce(  "Draw Line from Left to Right." );
  return  zip_ok;
  }

long
zipoline::Build_Object( zip_type_pane		   pane, enum view_MouseAction				   action , long				   x , long				   y , long				   clicks, zip_type_point		   X , zip_type_point		   Y )
          {
  long				  status = zip_ok;
  zip_type_figure		  figure;

  IN(zipoline::Build_Object);
  switch ( action )
    {
    case view_LeftDown:
      if ( (status =
	(this->data_object)->Create_Figure(  &CurrentFigure, NULL, zip_line_figure,
			   CurrentImage, NULL )) == zip_success )
        {
        (this)->Set_Object_Point(  CurrentFigure, 9, X, Y );
        (this)->Set_Object_Point(  CurrentFigure, 5, X, Y );
	CurrentFigure->zip_figure_zoom_level = pane->zip_pane_zoom_level;
        pane->zip_pane_edit->zip_pane_edit_last_point_id = zip_figure_auxiliary_point;
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
	}
      /* Fall-thru */
    case view_LeftMovement:
      if ( CurrentFigure )
	{
	(this->view_object)->Set_Pane_Painting_Mode(  pane, zipview_paint_inverted );
	(this->view_object)->Draw_Figure(  CurrentFigure, pane );
        (this)->Set_Object_Point(  CurrentFigure, 5, X, Y );
	(this->view_object)->Draw_Figure(  CurrentFigure, pane );
	(this->view_object)->Set_Pane_Painting_Mode(  pane, zip_default );
	}
      break;
    default:
      break;
    }
  OUT(zipoline::Build_Object);
  return  status;
  }

long
zipoline::Draw_Object( zip_type_figure		   figure, zip_type_pane		   pane )
        {
  long				  status = zip_ok;

  IN(zipoline::Draw_Object);
  if ( (this->view_object)->Condition(  pane, figure, zip_draw ) )
    status = Draw( this, figure, pane );
  OUT(zipoline::Draw_Object);
  return  status;
  }

long
zipoline::Clear_Object( zip_type_figure		   figure, zip_type_pane		   pane )
        {
  long				  status = zip_ok;

  IN(zipoline::Clear_Object);
  if ( (this->view_object)->Condition(  pane, figure, zip_clear ) )
    status = Draw( this, figure, pane );
  OUT(zipoline::Clear_Object);
  return  status;
  }

static
long Draw( class zipoline		  *self, zip_type_figure		   figure, zip_type_pane		   pane )
        {
  long				  status = zip_ok;

  IN(Draw);
  if ( (self->view_object)->Ensure_Line_Attributes(  figure ) == zip_ok )
    {
    (self->view_object)->MoveTo(  window_x_point, window_y_point );
    (self->view_object)->DrawLineTo(  window_x_points(0), window_y_points(0) );
    if ( ExposePoints )
      (self)->Expose_Object_Points(  figure, pane );
    if ( HighlightPoints )
      (self)->Highlight_Object_Points(  figure, pane );
    }
  OUT(Draw);
  return  status;
  }

long
zipoline::Print_Object( zip_type_figure figure, zip_type_pane pane )
{
  class zipoline *self=this;
  long				  status = zip_ok;

  IN(zipoline::Print_Object);
  (this->print_object)->Ensure_Line_Attributes(  figure );
  (this->print_object)->Draw_Line(  print_x_point, print_y_point,
		      print_x_points( 0 ), print_y_points( 0 ) );
  OUT(zipoline::Print_Object);
  return  status;
}

long
zipoline::Proximate_Object_Points( zip_type_figure		   figure, zip_type_pane		   pane, zip_type_pixel		   x , zip_type_pixel		   y )
          {
  int				  point = 0;
  zip_type_pixel			  X1, X2, X3, Y1, Y2, Y3;

  IN(zipoline::Proximate_Object_Points);
  Compute_Handle_Positions( this, figure, pane, &X1, &X2, &X3, &Y1, &Y2, &Y3 );
  /* Ordered thus to give preference to moving an End-point */
  /* Upper-Left Corner */
  if ( (this->view_object)->Proximate_Figure_Point(  pane, figure, X1, Y1, x, y )) point = 9;
  else	/* Lower_Right Corner */
  if ( (this->view_object)->Proximate_Figure_Point(  pane, figure, X3, Y3, x, y )) point = 5;
  else 	/* Center */
  if ( (this->view_object)->Proximate_Figure_Point(  pane, figure, X2, Y2, x, y )) point = 1;
  else	/* 12 O'Clock */
  if ( (this->view_object)->Proximate_Figure_Point(  pane, figure, X2, Y1, x, y )) point = 2;
  else	/* Upper-Right Corner */
  if ( (this->view_object)->Proximate_Figure_Point(  pane, figure, X3, Y1, x, y )) point = 3;
  else	/* 3 O'Clock */
  if ( (this->view_object)->Proximate_Figure_Point(  pane, figure, X3, Y2, x, y )) point = 4;
  else	/* 6 O'Clock */
  if ( (this->view_object)->Proximate_Figure_Point(  pane, figure, X2, Y3, x, y )) point = 6;
  else	/* Lower-Left Corner */
  if ( (this->view_object)->Proximate_Figure_Point(  pane, figure, X1, Y3, x, y )) point = 7;
  else	/* 9 O'Clock */
  if ( (this->view_object)->Proximate_Figure_Point(  pane, figure, X1, Y2, x, y )) point = 8;
  OUT(zipoline::Proximate_Object_Points);
  return  point;
  }

boolean
zipoline::Enclosed_Object( zip_type_figure		   figure, zip_type_pane		   pane, zip_type_pixel		   x , zip_type_pixel		   y , zip_type_pixel		   w , zip_type_pixel		   h )
          {
  boolean			  enclosed = false;
  zip_type_pixel		  X1, Y1, X2, Y2;

  IN(zipoline::Enclosed_Object);
  X1 = window_x_point;	    Y1 = window_y_point;
  X2 = window_x_points(0);  Y2 = window_y_points(0);
  if ( X1 > x  &&  Y1 > y  &&  X2 < (x + w)  &&  Y2 < (y + h) )
    enclosed = true;
  OUT(zipoline::Enclosed_Object);
  return  enclosed;
  }

long
zipoline::Object_Enclosure( zip_type_figure		   figure, zip_type_pane		   pane, zip_type_pixel		  *x , zip_type_pixel		  *y , zip_type_pixel		  *w , zip_type_pixel		  *h )
          {
  zip_type_pixel			  X1, X2, X3, Y1, Y2, Y3;

  IN(zipoline::Object_Enclosure);
  Compute_Handle_Positions( this, figure, pane, &X1, &X2, &X3, &Y1, &Y2, &Y3 );
  *x = X1;  *y = Y1;  *w = abs(X3 - X1);  *h = abs(Y3 - Y1);
  OUT(zipoline::Object_Enclosure);
  return  zip_ok;
  }

long
zipoline::Highlight_Object_Points( zip_type_figure		   figure, zip_type_pane		   pane )
        {
  zip_type_pixel			  X1, X2, X3, Y1, Y2, Y3;
  long				  status = zip_ok;

  IN(zipoline::Highlight_Object_Points);
  Compute_Handle_Positions( this, figure, pane, &X1, &X2, &X3, &Y1, &Y2, &Y3 );
  (this->edit_object)->Highlight_Handles(  pane, X1, X2, X3, Y1, Y2, Y3 );
  OUT(zipoline::Highlight_Object_Points);
  return  status;
  }

long
zipoline::Normalize_Object_Points( zip_type_figure		   figure, zip_type_pane		   pane )
        {
  zip_type_pixel			  X1, X2, X3, Y1, Y2, Y3;
  long				  status = zip_ok;

  IN(zipoline::Normalize_Object_Points);
  Compute_Handle_Positions( this, figure, pane, &X1, &X2, &X3, &Y1, &Y2, &Y3 );
  (this->edit_object)->Normalize_Handles(  pane, X1, X2, X3, Y1, Y2, Y3 );
  OUT(zipoline::Normalize_Object_Points);
  return  status;
  }

long
zipoline::Expose_Object_Points( zip_type_figure		   figure, zip_type_pane		   pane )
        {
  long				  status = zip_ok;

  IN(zipoline::Expose_Object_Points);
  (this->edit_object)->Expose_Point(  pane, figure, figure_x_point,     figure_y_point );
  (this->edit_object)->Expose_Point(  pane, figure, figure_x_points(0), figure_y_points(0) );
  OUT(zipoline::Expose_Object_Points);
  return  status;
  }

long
zipoline::Hide_Object_Points( zip_type_figure		   figure, zip_type_pane		   pane )
        {
  long				  status = zip_ok;

  IN(zipoline::Hide_Points);
  (this->edit_object)->Hide_Point(  pane, figure, figure_x_point,     figure_y_point );
  (this->edit_object)->Hide_Point(  pane, figure, figure_x_points(0), figure_y_points(0) );
  OUT(zipoline::Hide_Points);
  return  status;
  }

static
void Compute_Handle_Positions( class zipoline		  *self, zip_type_figure		   figure, zip_type_pane		   pane, zip_type_pixel		  *X1 , zip_type_pixel		  *X2 , zip_type_pixel		  *X3 , zip_type_pixel		  *Y1 , zip_type_pixel		  *Y2 , zip_type_pixel		  *Y3 )
          {
  *X1 = (self->view_object)->X_Point_To_Pixel(  pane, figure, figure_x_point );
  *X2 = *X1 + (window_x_points(0) - window_x_point)/2;
  *X3 = (self->view_object)->X_Point_To_Pixel(  pane, figure, figure_x_points(0) );
  *Y1 = (self->view_object)->Y_Point_To_Pixel(  pane, figure, figure_y_point );
  *Y2 = *Y1 + (window_y_points(0) - window_y_point)/2;
  *Y3 = (self->view_object)->Y_Point_To_Pixel(  pane, figure, figure_y_points(0) );
  }

boolean
zipoline::Contains( zip_type_figure		 figure, zip_type_pane		 pane, zip_type_pixel		 x , zip_type_pixel		 y )
        
  {
  boolean			status = FALSE;
  int				x1, y1, x2, y2;

  IN( zipoline::Contains )
  x1 = window_x_point;
  y1 = window_y_point;
  x2 = window_x_points(0);
  y2 = window_y_points(0);
  if (( x >= zipmin( x1, x2 )) && ( x <= zipmax( x1, x2 )) &&
      ( y >= zipmin( y1, y2 )) && ( y <= zipmax( y1, y2 )) &&
      (( y - y1 )*( x2 - x1 ) - ( y2 - y1 )*( x - x1 )) == 0 )
    status = TRUE;
  OUT( zipoline::Contains )
  return status;
  }
