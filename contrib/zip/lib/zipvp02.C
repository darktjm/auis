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

#ifndef NORCSID
#define NORCSID
static char rcsid[]="$Header: /afs/cs.cmu.edu/project/atk-src-C++/contrib/zip/lib/RCS/zipvp02.C,v 1.4 1993/08/04 21:15:05 rr2b Stab74 $";
#endif


 

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
static int Blit_Pane( register class zipview *self, zip_type_pane pane, int x_offset, int y_offset );


long
zipview::Pan_Pane( register zip_type_pane		   pane, register zip_type_pixel		   x_offset , register zip_type_pixel		   y_offset )
        {
  register int				  status = zip_success;
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
zipview::Pan_Pane_To_Edge( register zip_type_pane		   pane, register long				   edge )
        {
  register int				  status = zip_success;
  register long				  x_offset = 0, y_offset = 0;
  register zip_type_stream		  stream;
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
zipview::Handle_Panning( register zip_type_pane		   pane, register long				   x , register long				   y , register long				   x_delta , register long				   y_delta )
        {
return zip_ok;
  }

long
zipview::Initiate_Panning( register zip_type_pane		   pane, register long				   x , register long				   y , register long				   mode )
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
zipview::Continue_Panning( register zip_type_pane		   pane, register long				   x , register long				   y )
        {
  register long				  precision =
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
zipview::Terminate_Panning( register zip_type_pane		   pane, register long				   x , register long				   y, register long				  *x_delta , register long				  *y_delta, register long				   draw )
            {
  register long				  precision =
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
Blit_Pane( register class zipview		  *self, register zip_type_pane		   pane, register int				   x_offset , register int				   y_offset )
        {
  struct rectangle			  rectangle;
  struct point				  point;
  register class graphic		 *graphic_op;

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
