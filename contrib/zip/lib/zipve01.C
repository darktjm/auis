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

/* zipve01.c	Zip EditView-object  -- Grids			      */
/* Author	TC Peters					      */
/* Information Technology Center	   Carnegie-Mellon University */



/**  SPECIFICATION -- External Facility Suite  *********************************

TITLE	The Zip EditView-object  --  Grids

MODULE	zipve01.c

NOTICE	IBM Internal Use Only

DESCRIPTION
	This is the suite of Methods that support the Editing facilities
	of the Zip View-object.

    NB: The comment-symbol "===" indicates areas which are:
	    1 - Questionable
	    OR
	    2 - Arbitrary
	    OR
	    3 - Temporary Hacks
    Such curiosities need be resolved prior to Project Completion...


HISTORY
  03/31/88	Created (TCP)
  01/18/88	Improve Grid speed by not drawing any dots beyond 1st level (TCP)
   09/26/89	Fix point delta calculations in Draw_Pane_Grid() (SCG)

END-SPECIFICATION  ************************************************************/

#include <andrewos.h>
#include "view.H"
#include "fontdesc.H"
#include "zip.H"
#include "zipview.H"
#include "zipedit.H"
#include "zipedit.h"
#include "zipobject.H"
#include "zipiff00.h"

static void Clear_Pane_Mark_Areas( register class zipedit		  *self, register zip_type_pane		   pane );
static void Draw_Pane_Coordinate_Marks( register class zipedit		  *self, register zip_type_pane		   pane );
static void Draw_Pane_Coordinate_Ticks( register class zipedit		  *self, register zip_type_pane		   pane );
long Compute_Pane_Coordinate_Deltas( register class zipedit		  *self, register zip_type_pane		   pane );


long
zipedit::Draw_Pane_Coordinates( register zip_type_pane		   pane )
      {
  class zipedit *self=this;
  IN(zipedit::Draw_Pane_Coordinates);
  pane->zip_pane_state.zip_pane_state_coordinates_exposed = true;
  pane->zip_pane_border_thickness = ZIP_pane_coordinate_thickness;
/*===  zipedit_Compute_Pane_Stretch_Factors( self, pane );===*/
  (this->view_object)->Set_Clip_Area(  pane, PaneLeft, PaneTop, PaneWidth, PaneHeight );
  (this->view_object)->SetTransferMode(  graphic_BLACK );
  (this->view_object)->MoveTo(  PaneLeft, PaneTop );
  if ( (View )->GetLineWidth( ) != 1 )
    (this->view_object)->SetLineWidth(  1 );
  (this->view_object)->DrawLineTo(  PaneRight - 1, PaneTop );
  (this->view_object)->DrawLineTo(  PaneRight - 1, PaneBottom - 1 );
  (this->view_object)->DrawLineTo(  PaneLeft, PaneBottom - 1 );
  (this->view_object)->DrawLineTo(  PaneLeft, PaneTop );
  (this->view_object)->MoveTo(  PaneLeft + ZIP_pane_coordinate_thickness - 1,
		  PaneTop + ZIP_pane_coordinate_thickness - 1);
  (this->view_object)->DrawLineTo(  PaneRight - ZIP_pane_coordinate_thickness,
		      PaneTop + ZIP_pane_coordinate_thickness - 1 );
  (this->view_object)->DrawLineTo(  PaneRight - ZIP_pane_coordinate_thickness,
		      PaneBottom - ZIP_pane_coordinate_thickness );
  (this->view_object)->DrawLineTo(  PaneLeft + ZIP_pane_coordinate_thickness - 1,
		      PaneBottom - ZIP_pane_coordinate_thickness );
  (this->view_object)->DrawLineTo(  PaneLeft + ZIP_pane_coordinate_thickness - 1,
		      PaneTop + ZIP_pane_coordinate_thickness - 1);
  Compute_Pane_Coordinate_Deltas( self, pane );
  Draw_Pane_Coordinate_Marks( self, pane );
  (self)->Draw_Pane_Grid(  pane );
  (this->view_object)->Set_Clip_Area(   pane, PaneLeft + ZIP_pane_coordinate_thickness + 2,
		       PaneTop  + ZIP_pane_coordinate_thickness + 2,
		       PaneWidth - (2 * ZIP_pane_coordinate_thickness) - 2,
		       PaneHeight- (2 * ZIP_pane_coordinate_thickness) - 2 );
  OUT(zipedit::Draw_Pane_Coordinates);
  return zip_success;
  }

long zipedit::Draw_Pane_Grid( register zip_type_pane	   pane )
{
    register long			  i, j, k, point_spacing;
    register float		  X, Y, GX, GY, point_delta_SMSD;
    register float		    grid_factor, delta;
    register long                   X_int, Y_int, r, c;
    register float		  SM, SD, point_delta,L, T, R, B;
    float			  x_center, y_center;
    long                          n;
    class fontdesc		 *current_font = (this->view_object)->GetFont( );
   char				  big_dot = 'C';
   register class graphic	 *black = (this->view_object)->BlackPattern( );;
   class zipedit *self=this;

  IN(zipedit::Draw_Pane_Grid);
    if ( pane->zip_pane_edit->zip_pane_edit_grid_exposed )
    {
	if (pane->zip_pane_edit->zip_pane_edit_coordinate_grid == 0)
	    pane->zip_pane_edit->zip_pane_edit_coordinate_grid = 1;
    Compute_Pane_Coordinate_Deltas( self, pane );
    point_spacing = pane->zip_pane_edit->zip_pane_edit_mark_point_spacing;
    point_delta = pane->zip_pane_edit->zip_pane_edit_mark_point_delta;
    (this->view_object)->Set_Clip_Area(  pane,
	 PaneLeft + pane->zip_pane_border_thickness + 2,
	 PaneTop  + pane->zip_pane_border_thickness + 2,
	 PaneWidth - (2 * pane->zip_pane_border_thickness) - 2,
	 PaneHeight- (2 * pane->zip_pane_border_thickness) - 2 );
    if ( self->dots_font == NULL )
      self->dots_font = (class fontdesc *) (this->data_object)->Define_Font(  zip_dot_font_name, NULL );
    (this->view_object)->SetFont(  self->dots_font );
    if ( pane->zip_pane_zoom_level >= 0 )
      SM = pane->zip_pane_stretch_multiplier * (pane->zip_pane_zoom_level + 1);
      else
      SM = pane->zip_pane_stretch_multiplier / (abs( pane->zip_pane_zoom_level ) + 1);
    SD = pane->zip_pane_stretch_divisor;
	/*    point_delta_SMSD = (point_delta * SM) / SD; */
	delta =  (point_delta_SMSD = (point_delta * SM) / SD)/
	  pane->zip_pane_edit->zip_pane_edit_coordinate_grid; 
    L = PaneLeft;
    R = PaneRight;
    T = PaneTop;
    B = PaneBottom;
    x_center = pane->zip_pane_x_origin + pane->zip_pane_x_offset;
    y_center = pane->zip_pane_y_origin - pane->zip_pane_y_offset;
    (this->view_object)->SetTransferMode(  graphic_BLACK );
	grid_factor = pane->zip_pane_edit->zip_pane_edit_coordinate_grid; 

	/* Do the center dot */
	(this->view_object)->MoveTo(  (long)x_center, (long)y_center );
	(this->view_object)->DrawText(  &big_dot, 1, 0 );

	/*==	if (x_center > y_center)

	    n = (long)(x_center / delta ) + 1;

	else

	    n = (long)(y_center / delta ) + 1; ==*/

	for ( i = 0; x_center+i*delta <= R ||
	  x_center-i*delta >= L ||
	  y_center+i*delta <= B ||
	  y_center-i*delta >= T;
	  i++ )
      {
      for ( j = -i; j <= i; j++ )
	{
		X_int = (long) (X = x_center + (j * delta ));
/*		if ( X_int < x_center ) X++; === FUDGE ===*/
	for ( k = -i; k <= i; k++ )
	  {
		    Y_int = (long) (Y = y_center - (k * delta ));
/*		    if ( Y_int < y_center ) Y++; === FUDGE ===*/
		    if ( ((j == -i  ||  j == i) || (k == -i  ||  k == i)) &&
			X_int >= L  &&
			X_int <= R  &&
			Y_int >= T  &&
			Y_int <= B )
	    (this->view_object)->FillRectSize(  X_int, Y_int, 1,1, black );
	    }
	  }
	}

    (this->view_object)->SetFont(  current_font );
    }
  OUT(zipedit::Draw_Pane_Grid);
  return zip_success;
  }

static
void Clear_Pane_Mark_Areas( register class zipedit		  *self, register zip_type_pane		   pane )
      {
  (self->view_object)->SetTransferMode(  graphic_WHITE );
  (self->view_object)->FillRectSize(  PaneLeft + 1, PaneTop + 1,
    PaneWidth - 2, pane->zip_pane_border_thickness - 2, (View )->WhitePattern( ) );
  (self->view_object)->FillRectSize(  PaneRight - pane->zip_pane_border_thickness + 1,
    PaneTop + 1,
    pane->zip_pane_border_thickness - 2, PaneHeight - 2, (View )->WhitePattern( ) );
  (self->view_object)->FillRectSize(  PaneLeft + 1, PaneBottom - pane->zip_pane_border_thickness + 1,
    PaneWidth - 2, pane->zip_pane_border_thickness - 2, (View )->WhitePattern( ) );
  (self->view_object)->FillRectSize(  PaneLeft + 1, PaneTop + 1,
    pane->zip_pane_border_thickness - 2, PaneHeight - 2, (View )->WhitePattern( ) );
  (self->view_object)->SetTransferMode(  graphic_BLACK );
  }

static
void Draw_Pane_Coordinate_Marks( register class zipedit		  *self, register zip_type_pane		   pane )
      {
  register class fontdesc		 *current_font = (View )->GetFont( );

  IN(Draw_Pane_Coordinate_Marks);
  (self->view_object)->Set_Clip_Area(  pane, PaneLeft, PaneTop, PaneWidth, PaneHeight );
  Clear_Pane_Mark_Areas( self, pane );
  Draw_Pane_Coordinate_Ticks( self, pane );
  (self->view_object)->Set_Clip_Area(   pane, PaneLeft + ZIP_pane_coordinate_thickness + 2,
		       PaneTop  + ZIP_pane_coordinate_thickness + 2,
		       PaneWidth - (2 * ZIP_pane_coordinate_thickness) - 2,
		       PaneHeight- (2 * ZIP_pane_coordinate_thickness) - 2 );
  (self->view_object)->SetFont(  current_font );
  OUT(Draw_Pane_Coordinate_Marks);
  }

static
void Draw_Pane_Coordinate_Ticks( register class zipedit		  *self, register zip_type_pane		   pane )
      {
  register float			  SM, SD;
  register int				  i, center, middle, edge;
  char					  point_value[256];
  short				     font = 0;

  IN(Draw_Pane_Coordinate_Ticks);
  (self->data_object)->Define_Font(  ZIP_pane_coordinate_font_name, &font );
  (self->view_object)->SetFont(  self->data_object->fonts->zip_fonts_vector[font].zip_fonts_table_font );
  center = PaneLeft + pane->zip_pane_x_offset + PaneWidth/2;
  middle = PaneTop + ZIP_pane_coordinate_thickness/2;
  edge = PaneTop + ZIP_pane_coordinate_thickness - 3;
  (self->view_object)->Set_Clip_Area(   pane, PaneLeft + ZIP_pane_coordinate_thickness,
		       PaneTop, PaneWidth - (2 * ZIP_pane_coordinate_thickness),
		       ZIP_pane_coordinate_thickness );
  if ( pane->zip_pane_zoom_level >= 0 )
    SM = pane->zip_pane_stretch_multiplier * (pane->zip_pane_zoom_level + 1);
    else
    SM = pane->zip_pane_stretch_multiplier / (abs( pane->zip_pane_zoom_level ) + 1);
  SD = pane->zip_pane_stretch_divisor;
  for ( i = 0;
	center + (i * pane->zip_pane_edit->zip_pane_edit_mark_point_spacing) <=
		 PaneRight  ||
	center - (i * pane->zip_pane_edit->zip_pane_edit_mark_point_spacing) >=
		 PaneLeft;
	i++ )
    {
    sprintf( point_value, "%d", i * (int)pane->zip_pane_edit->zip_pane_edit_mark_point_delta );
    (self->view_object)->MoveTo(  (int)(center +
		       ((i * pane->zip_pane_edit->zip_pane_edit_mark_point_delta) * SM / SD)),  middle );
    (self->view_object)->DrawString(  point_value,
			((i%2) ? view_ATTOP : view_ATBOTTOM) | view_BETWEENLEFTANDRIGHT );
    if ( i == 0 )
      (self->view_object)->FillRectSize(  center - 1, edge - 2, 3, 5, (View )->WhitePattern( ) ); 
      else
      (self->view_object)->FillRectSize(  (int)(center +
		        ((i * pane->zip_pane_edit->zip_pane_edit_mark_point_delta) * SM / SD)),
		      edge, 1,3, (View )->WhitePattern( ) ); 
    (self->view_object)->MoveTo(  (int)(center - ((i * pane->zip_pane_edit->zip_pane_edit_mark_point_delta) * SM / SD)),
		   middle );
    (self->view_object)->DrawString(  point_value,
			((i%2) ? view_ATTOP : view_ATBOTTOM) | view_BETWEENLEFTANDRIGHT );
    (self->view_object)->FillRectSize(  (int)(center - ((i * pane->zip_pane_edit->zip_pane_edit_mark_point_delta) * SM / SD)),
		    edge, 1, 3, (View )->WhitePattern( ) ); 
    }
  center = PaneLeft + ZIP_pane_coordinate_thickness/2;
  middle = PaneTop - pane->zip_pane_y_offset + PaneHeight/2;
  edge = PaneLeft + ZIP_pane_coordinate_thickness - 3;
  (self->view_object)->Set_Clip_Area(   pane, PaneLeft,
		       PaneTop + ZIP_pane_coordinate_thickness,
		       ZIP_pane_coordinate_thickness,
		       PaneHeight - (2 * ZIP_pane_coordinate_thickness ) );
  for ( i = 0;
	middle + (i * pane->zip_pane_edit->zip_pane_edit_mark_point_spacing) <=
	     PaneBottom  ||
	middle - (i * pane->zip_pane_edit->zip_pane_edit_mark_point_spacing) >=
	     PaneTop;
	i++ )
    {
    sprintf( point_value, "%d", i * (int)pane->zip_pane_edit->zip_pane_edit_mark_point_delta );
    (self->view_object)->MoveTo(   center,
		    (int)(middle + ((i * pane->zip_pane_edit->zip_pane_edit_mark_point_delta) * SM / SD)) );
    (self->view_object)->DrawString(  point_value,
		    view_BETWEENTOPANDBOTTOM | view_BETWEENLEFTANDRIGHT );
    if ( i == 0 )
      (self->view_object)->FillRectSize(  edge - 2, middle - 1, 5, 3, (View )->WhitePattern( ) ); 
      else
      (self->view_object)->FillRectSize(  edge,
	 (int)(middle + ((i * pane->zip_pane_edit->zip_pane_edit_mark_point_delta) * SM / SD)),
	 3, 1, (View )->WhitePattern( ) ); 
    (self->view_object)->MoveTo(  center,
		    (int)(middle - ((i * pane->zip_pane_edit->zip_pane_edit_mark_point_delta) * SM / SD))  );
    (self->view_object)->DrawString(  point_value, view_BETWEENTOPANDBOTTOM | view_BETWEENLEFTANDRIGHT );
    (self->view_object)->FillRectSize(  edge,
	 (int)(middle - ((i * pane->zip_pane_edit->zip_pane_edit_mark_point_delta) * SM / SD)),
	 3,1, (View )->WhitePattern( ) ); 
    }
  (self->view_object)->Set_Clip_Area(   pane, PaneLeft + ZIP_pane_coordinate_thickness + 2,
		       PaneTop  + ZIP_pane_coordinate_thickness + 2,
		       PaneWidth - (2 * ZIP_pane_coordinate_thickness) - 2,
		       PaneHeight- (2 * ZIP_pane_coordinate_thickness) - 2 );
  OUT(Draw_Pane_Coordinate_Ticks);
  }

/*static If self isn't static then I can access it from Set_Constrains */
long Compute_Pane_Coordinate_Deltas( register class zipedit		  *self, register zip_type_pane		   pane )
      {
  register float			  SM, SD, delta = 1.0, limit = 10.0;
  register int				  done = false, low;

  if ( pane->zip_pane_zoom_level >= 0 )
    SM = pane->zip_pane_stretch_multiplier * (pane->zip_pane_zoom_level + 1);
    else
    SM = pane->zip_pane_stretch_multiplier / (abs( pane->zip_pane_zoom_level ) + 1);
  SD = pane->zip_pane_stretch_divisor;
  if ( pane->zip_pane_attributes.zip_pane_attribute_stream_source )
    low = (long) (((pane->zip_pane_source.zip_pane_stream->zip_stream_greatest_x / 20.0) * SM) / SD);
  else
  if ( pane->zip_pane_attributes.zip_pane_attribute_image_source )
    low = (long) (((pane->zip_pane_source.zip_pane_image->zip_image_greatest_x / 20.0) * SM) / SD);
  else /*=== deal with figure extrema ===*/
    low = (long) ((20.0 * SM) / SD);
/*=== make intelligent ===*/
  pane->zip_pane_edit->zip_pane_edit_mark_point_delta = 100.0;

  if ( (pane->zip_pane_edit->zip_pane_edit_mark_point_spacing = (long) (((pane->zip_pane_edit->zip_pane_edit_mark_point_delta * SM) / SD ))) < low )
    while ( ! done )
      {
      for ( pane->zip_pane_edit->zip_pane_edit_mark_point_delta = delta;
          pane->zip_pane_edit->zip_pane_edit_mark_point_delta <= limit;
	  pane->zip_pane_edit->zip_pane_edit_mark_point_delta += delta )
	{
        if ( (pane->zip_pane_edit->zip_pane_edit_mark_point_spacing = (long) (((pane->zip_pane_edit->zip_pane_edit_mark_point_delta * SM) / SD ))) >=
		 low  &&
	       ((int)pane->zip_pane_edit->zip_pane_edit_mark_point_delta % 2) == 0 )
	  done = true;
	}
        delta = limit;
        limit = 10.0 * limit;
      }
  return zip_success;
  }

void zipedit::Lighten_Pane( register zip_type_pane		   pane, register char				   density )
        {
  class zipedit *self=this;
  (this->view_object)->Set_Pane_Clip_Area(  pane );
  (this->view_object)->SetTransferMode(  graphic_WHITE );
  if ( density == 0 )
    density = 'G';
  (this->view_object)->FillTrapezoid( 
	PaneLeft, PaneTop, PaneWidth,
	PaneLeft, PaneBottom, PaneWidth,
(this->view_object)->WhitePattern()/*===tile===*/ );
  (View )->FlushGraphics( );
  (this->view_object)->SetTransferMode(  graphic_BLACK );
  }

