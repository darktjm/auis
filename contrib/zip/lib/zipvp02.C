/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

/* zipvp02.c	Zip View-object	-- Panning			      */
/* Author	TC Peters					      */
/* Information Technology Center	   Carnegie-Mellon University */



/**  SPECIFICATION -- External Facility Suite  *********************************

TITLE	The Zip View-object -- Panning	

MODULE	zipvp02.c

NOTICE	IBM Internal Use Only

DESCRIPTION
	This is the suite of Methods that support the Pane Cursor facilities
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
  12/08/88	Utilize panning_precision of pane when panning (TCP)
   08/16/90	Add Normalize_Line_Attributes on Initiate_Panning (SCG)

END-SPECIFICATION  ************************************************************/

#include <andrewos.h>
#include "view.H"
#include "zip.H"
#include "zipview.H"
#include "zipprint.H"
#include "zipobject.H"

#define	 Data			      (self->zipobject::data_object)
#define	 View			      (self)

#define  BorderThickness	      (pane->zip_pane_border_thickness)

#define  panning_factor			  16

static void Set_Crosshairs( class zipview *self, zip_type_pane pane, long x, long y );
static int Blit_Pane( class zipview *self, zip_type_pane pane, int x_offset, int y_offset );


long
zipview::Pan_Pane( zip_type_pane		   pane, zip_type_pixel		   x_offset , zip_type_pixel		   y_offset )
        {
  int				  status = zip_success;
  class zipview *self=this;

  IN(zipview_Pan_Pane);
  DEBUGdt(X-Offset,x_offset);
  DEBUGdt(Y-Offset,y_offset);
  ZIP_EFN(zip_Pan_Pane_EFN);
  if ( pane )
    {
    (self)->Set_Pane_Clip_Area(  pane );
    if ( x_offset == 0  &&  y_offset == 0 )
      {
      Blit_Pane( self, pane, - pane->zip_pane_x_offset,
			    - pane->zip_pane_y_offset );
      pane->zip_pane_x_offset = 0;
      pane->zip_pane_y_offset = 0;
      pane->zip_pane_x_origin_offset = pane->zip_pane_x_origin;
      pane->zip_pane_y_origin_offset = pane->zip_pane_y_origin;
      }
      else
      {
      long  factor = (pane->zip_pane_panning_precision > 0 ) ?
			 pane->zip_pane_panning_precision : 1;
      x_offset = (x_offset / factor) * factor;
      y_offset = (y_offset / factor) * factor;
      pane->zip_pane_x_offset += x_offset;
      pane->zip_pane_y_offset += y_offset;
      pane->zip_pane_x_origin_offset = pane->zip_pane_x_origin + pane->zip_pane_x_offset;
      pane->zip_pane_y_origin_offset = pane->zip_pane_y_origin - pane->zip_pane_y_offset;
      Blit_Pane( self, pane, x_offset, y_offset );
      }
/*===
    if ( pane->zip_pane_state.zip_pane_state_coordinates_exposed )
      ZIP_Draw_Pane_Coordinate_Marks( pane );
    zipview_FlushGraphics( self );
===*/
    (self)->Draw_Pane(  pane );
    }
    else
    {
    status = zip_pane_non_existent;
    }
  (self)->FlushGraphics( );
  ZIP_STATUS(this->data_object);
  OUT(zipview_Pan_Pane);
  return status;
  }

long
zipview::Pan_Pane_To_Edge( zip_type_pane		   pane, long				   edge )
        {
  int				  status = zip_success;
  long				  x_offset = 0, y_offset = 0;
  zip_type_stream		  stream;
  class zipview *self=this;

  IN(zip_Pan_Pane_To_Edge);
  ZIP_EFN(zip_Pan_Pane_To_Edge_EFN);
  if ( pane  &&  (stream = (zip_type_stream)(self)->Pane_Stream(  pane )) )
    {
    (self)->Set_Pane_Clip_Area(  pane );
    if ( edge & zipview_pane_top_edge )
      y_offset =   (long) (((self)->Pane_Y_Origin(  pane ) -
		     pane->zip_pane_y_offset) - stream->zip_stream_greatest_y *
	((pane->zip_pane_zoom_level >= 0) ?
	 pane->zip_pane_stretch_multiplier * (pane->zip_pane_zoom_level + 1)
	 :
	 pane->zip_pane_stretch_multiplier / abs(pane->zip_pane_zoom_level))
	 /
	 pane->zip_pane_stretch_divisor);
    else
    if ( edge & zipview_pane_bottom_edge )
      y_offset =   (long)  ((((self)->Pane_Y_Origin(  pane ) -
	pane->zip_pane_y_offset) - stream->zip_stream_least_y *
	((pane->zip_pane_zoom_level >= 0) ?
	 pane->zip_pane_stretch_multiplier * (pane->zip_pane_zoom_level + 1)
	 :
	 pane->zip_pane_stretch_multiplier / abs(pane->zip_pane_zoom_level))
	 /
	 pane->zip_pane_stretch_divisor) - (self)->Pane_Height(  pane ) + 1);
    if ( edge & zipview_pane_left_edge )
      x_offset = (long)  -(((self)->Pane_X_Origin(  pane ) +
	pane->zip_pane_x_offset) + stream->zip_stream_least_x *
	((pane->zip_pane_zoom_level >= 0) ?
	 pane->zip_pane_stretch_multiplier * (pane->zip_pane_zoom_level + 1)
	 :
	 pane->zip_pane_stretch_multiplier / abs(pane->zip_pane_zoom_level))
         /
	 pane->zip_pane_stretch_divisor);
    else
    if ( edge & zipview_pane_right_edge )
      x_offset = (long) -((((self)->Pane_X_Origin(  pane ) +
	pane->zip_pane_x_offset) + stream->zip_stream_greatest_x *
	((pane->zip_pane_zoom_level >= 0) ?
	 pane->zip_pane_stretch_multiplier * (pane->zip_pane_zoom_level + 1)
	 :
	 pane->zip_pane_stretch_multiplier / abs(pane->zip_pane_zoom_level))
	 /
	 pane->zip_pane_stretch_divisor) - (self)->Pane_Width(  pane ) + 1);
    Blit_Pane( self, pane, x_offset, y_offset );
    pane->zip_pane_x_offset += x_offset;
    pane->zip_pane_y_offset += y_offset;
    pane->zip_pane_x_origin_offset = pane->zip_pane_x_origin + pane->zip_pane_x_offset;
    pane->zip_pane_y_origin_offset = pane->zip_pane_y_origin - pane->zip_pane_y_offset;
/*===
    if ( pane->zip_pane_state.zip_pane_state_coordinates_exposed )
      ZIP_Draw_Pane_Coordinate_Marks( pane );
    zipview_FlushGraphics( self );
===*/
    (self)->Draw_Pane(  pane );
    }
    else
    {
    status = zip_pane_non_existent;
    }
  (self )->FlushGraphics( );
  OUT(zip_Pan_Pane_To_Edge);
  ZIP_STATUS(this->data_object);
  return status;
  }

static zip_type_pixel			  initial_x, initial_y,
					  hair_x, hair_y;


long
zipview::Handle_Panning( zip_type_pane		   pane, long				   x , long				   y , long				   x_delta , long				   y_delta )
        {
return zip_ok;
  }

long
zipview::Initiate_Panning( zip_type_pane		   pane, long				   x , long				   y , long				   mode )
        {
  class zipview *self=this;
  IN(zipview::Initiate_Panning);
  (self )->Normalize_Line_Attributes( );
  Set_Crosshairs( self, pane, initial_x = x, initial_y = y );
  Set_Crosshairs( self, pane, hair_x = x + 1, hair_y = y + 1 );
  OUT(zipview::Initiate_Panning);
  return zip_ok;
  }


long
zipview::Continue_Panning( zip_type_pane		   pane, long				   x , long				   y )
        {
  long				  precision =
				pane->zip_pane_panning_precision;
  class zipview *self=this;

  IN(zipview::Continue_Panning);
  if ( x > hair_x + precision  ||  x < hair_x - precision  ||
       y > hair_y + precision  ||  y < hair_y - precision  )
    {
    Set_Crosshairs( self, pane, hair_x, hair_y );
    Set_Crosshairs( self, pane, hair_x = x, hair_y = y );
    }
  OUT(zipview::Continue_Panning);
  return zip_ok;
  }

long
zipview::Terminate_Panning( zip_type_pane		   pane, long				   x , long				   y, long				  *x_delta , long				  *y_delta, long				   draw )
            {
  long				  precision =
				pane->zip_pane_panning_precision;
  class zipview *self=this;

  IN(zipview::Terminate_Panning);
  Set_Crosshairs( self, pane, initial_x, initial_y );
  Set_Crosshairs( self, pane, hair_x, hair_y );
  if ( x_delta )  *x_delta = 0;
  if ( y_delta )  *y_delta = 0;
  if ( x > initial_x + precision ||  x < initial_x - precision ||
       y > initial_y + precision ||  y < initial_y - precision  )
    {
    if ( draw )
      {
      (self)->Pan_Pane(  pane, x - initial_x, -(y - initial_y) );
      if ( x_delta )
        *x_delta = pane->zip_pane_x_offset;
      if ( y_delta )
        *y_delta = pane->zip_pane_y_offset;
      }
      else
      {
      if ( x_delta )
	*x_delta = x - initial_x;
      if ( y_delta )
	*y_delta = y - initial_y;
      }
    }
  OUT(zipview::Terminate_Panning);
  return zip_ok;
  }

static
void Set_Crosshairs( class zipview *self, zip_type_pane pane, long x , long y )
{
  IN(Set_Crosshairs);
  (self)->SetTransferMode( graphic_INVERT );
  (self)->MoveTo( (self)->Pane_Left( pane ), y );
  if ( (self )->GetLineWidth( ) != 1 )
    (self)->SetLineWidth( 1 );
  (self)->DrawLineTo(  (self)->Pane_Right( pane ), y );
  (self)->MoveTo( x, (self)->Pane_Top( pane ) );
  (self)->DrawLineTo( x, (self)->Pane_Bottom( pane ) );
  (self)->FlushGraphics( );
  OUT(Set_Crosshairs);
  }

static int
Blit_Pane( class zipview		  *self, zip_type_pane		   pane, int				   x_offset , int				   y_offset )
        {
  struct rectangle			  rectangle;
  struct point				  point;
  class graphic		 *graphic_op;

  IN(Blit_Pane);
  (self)->SetTransferMode(  graphic_COPY );
  rectangle.left =
	(x_offset > 0) ?
	   (self)->Pane_Left(  pane ) + BorderThickness
	   :
	   (self)->Pane_Left(  pane ) + BorderThickness + abs(x_offset);
  rectangle.top = 
	(y_offset > 0) ?
	   (self)->Pane_Top(  pane )  + BorderThickness + abs(y_offset)
	   :
	   (self)->Pane_Top(  pane )  + BorderThickness;
  point.x =
	(x_offset > 0) ?
	   (self)->Pane_Left(  pane ) + BorderThickness + abs(x_offset)
	   :
	   (self)->Pane_Left(  pane ) + BorderThickness;
  point.y =
	(y_offset > 0) ?
	   (self)->Pane_Top(  pane )  + BorderThickness
	   :
	   (self)->Pane_Top(  pane )  + BorderThickness + abs(y_offset);
  rectangle.width =
	 (self)->Pane_Width(  pane )  - (abs(x_offset) + (2 * BorderThickness) );
  rectangle.height =
	 (self)->Pane_Height(  pane ) - (abs(y_offset) + (2 * BorderThickness) );
  (self)->BitBlt(  &rectangle, self, &point, NULL );
  if ( pane->zip_pane_state.zip_pane_state_inverted )
    {
    (self)->SetTransferMode(  graphic_BLACK );
    graphic_op = (self )->BlackPattern( );
    }
    else
    {
    (self)->SetTransferMode(  graphic_WHITE );
    graphic_op = (self )->WhitePattern( );
    }
  (self)->FillRectSize(  
	(x_offset > 0) ?
	   (self)->Pane_Left(  pane ) + BorderThickness
	   :
	   (self)->Pane_Right(  pane ) - (BorderThickness + abs(x_offset)),
	(self)->Pane_Top(  pane ) + BorderThickness,
	abs(x_offset),
	(self)->Pane_Height(  pane ) - 2 * BorderThickness, graphic_op );
  (self)->FillRectSize(  
	(self)->Pane_Left(  pane ) + BorderThickness,
	(y_offset > 0) ?
	   (self)->Pane_Bottom(  pane ) - (BorderThickness + abs(y_offset))
	   :
	   (self)->Pane_Top(  pane ) + BorderThickness,
	(self)->Pane_Width(  pane ) - 2 * BorderThickness,
	abs(y_offset), graphic_op );
  (self)->SetTransferMode(  graphic_BLACK );
  OUT(Blit_Pane);
  return zip_success;
  }
