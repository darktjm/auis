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

/*
 * P_R_P_Q_# (C) COPYRIGHT IBM CORPORATION 1988
 * LICENSED MATERIALS - PROPERTY OF IBM
 * REFER TO COPYRIGHT INSTRUCTIONS FORM NUMBER G120-2083
 */
/* zipve00.c	Zip EditView-object				      */
/* Author	TC Peters					      */
/* Information Technology Center	   Carnegie-Mellon University */


/**  SPECIFICATION -- External Facility Suite  *********************************

TITLE	The Zip EditView-object

MODULE	zipve00.c

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
  10/26/88	Optimize point Highlight/Normalize (TCP)
  06/08/89	Use Drawable in fontdesc_StringSize (TCP)
   08/22/89	Flag Figures regardless of visibility on Highlight/Normalize (SCG)

END-SPECIFICATION  ************************************************************/

#include <andrewos.h>
#include "view.H"
#include "im.H"
#include "fontdesc.H"
#include "zip.H"
#include "zipview.H"
#include "zipedit.H"
#include "zipobject.H"
#include "zipprint.h"
#include "zipprint.H"

#define PaneViewObj (self->view_object)

long Highlight_Pane_Points( class zipedit		  *self, zip_type_pane		   pane );
long Normalize_Pane_Points( class zipedit		  *self, zip_type_pane		   pane );
static int Delete_Inferior_Image( class zipedit		  *self, zip_type_image		   image, zip_type_pane		   pane );
static int Undelete_Inferior_Image( class zipedit		  *self, zip_type_image		   image, zip_type_pane		   pane );
int Highlight_Inferior_Image_Points( class zipedit		  *self, zip_type_image		   image, zip_type_pane		   pane );
static int Normalize_Inferior_Image_Points( class zipedit		  *self, zip_type_image		   image, zip_type_pane		   pane );
long Delete_Stream( class zipedit		  *self, zip_type_stream		   stream, zip_type_pane		   pane );
long Undelete_Stream( class zipedit		  *self, zip_type_stream		   stream, zip_type_pane		   pane );
long Highlight_Stream_Points( class zipedit		  *self, zip_type_stream		   stream, zip_type_pane		   pane );
long Normalize_Stream_Points( class zipedit		  *self, zip_type_stream		   stream, zip_type_pane		   pane );
long Hide_Stream_Points( class zipedit		  *self, zip_type_stream		   stream, zip_type_pane		   pane );
long Expose_Stream_Points( class zipedit		  *self, zip_type_stream		   stream, zip_type_pane		   pane );
static boolean Ratify_Highlighting( class zipedit		  *self, zip_type_pane		   pane, zip_type_pixel		   x , zip_type_pixel		   y );
static boolean Ratify_Normalizing( class zipedit		  *self, zip_type_pane		   pane, zip_type_pixel		   x , zip_type_pixel		   y );


long
zipedit::Set_Pane_Highlight_Icon( zip_type_pane		   pane, char				   icon )
        {
  long				  status = zip_ok;

  IN(zipedit::Set_Pane_Highlight_Icon);
  if ( pane )
    pane->zip_pane_highlight_icon = icon;
    else 
    status = zip_pane_non_existent;
  ZIP_STATUS(this->data_object);
  OUT(zipedit::Set_Pane_Highlight_Icon);
  return status;
  }

long
Highlight_Pane_Points( class zipedit		  *self, zip_type_pane		   pane )
      {
return zip_ok;
  }

long
Normalize_Pane_Points( class zipedit		  *self, zip_type_pane		   pane )
      {
return zip_ok;
  }

long
zipedit::Hide_Pane_Points( zip_type_pane		   pane )
      {
  long				  status = zip_ok;

  IN(zipedit::Hide_Pane_Points);
  if ( pane )
    {
    if ( pane->zip_pane_state.zip_pane_state_points_exposed )
      {
      pane->zip_pane_state.zip_pane_state_points_exposed = false;
      status = (this->view_object)->Display_Pane(  pane );
      }
      else  status = zip_pane_not_exposed; /*=== pane_points ===*/
    }
    else  status = zip_pane_non_existent;
  ZIP_STATUS(this->data_object);
  OUT(zipedit::Hide_Pane_Points);
  return status;
  }

long
zipedit::Expose_Pane_Points( zip_type_pane		   pane )
      {
  long				  status = zip_ok;

  IN(zipedit::Expose_Pane_Points);
  if ( pane )
    {
    if ( ! pane->zip_pane_state.zip_pane_state_points_exposed )
      {
      pane->zip_pane_state.zip_pane_state_points_exposed = true;
      status = (this->view_object)->Draw_Pane(  pane );
      }
      else status = zip_pane_not_hidden; /*=== pane_points ===*/
    } 
    else  status = zip_pane_non_existent;
  ZIP_STATUS(this->data_object);
  OUT(zipedit::Expose_Pane_Points);
  return status;
  }

long
zipedit::Expose_Pane_Grid( zip_type_pane		   pane )
      {
  long				  status = zip_ok;

  IN(zipedit::Expose_Pane_Grid);
  if ( pane )
    {
    if ( pane->zip_pane_edit->zip_pane_edit_grid_exposed == 0 )
      {
      pane->zip_pane_edit->zip_pane_edit_grid_exposed = 1;
      status = (this)->Draw_Pane_Grid(  pane );
      }
    }
    else  status = zip_pane_non_existent;
  ZIP_STATUS(this->data_object);
  OUT(zipedit::Expose_Pane_Grid);
  return status;
  }

long
zipedit::Hide_Pane_Grid( zip_type_pane		   pane )
      {
  long				  status = zip_ok;

  IN(zipedit::Hide_Pane_Grid);
  if ( pane )
    {
    if ( pane->zip_pane_edit->zip_pane_edit_grid_exposed != 0 )
      {
      pane->zip_pane_edit->zip_pane_edit_grid_exposed = 0;
      if (pane->zip_pane_edit->zip_pane_edit_constrain_points == 0)
	  pane->zip_pane_edit->zip_pane_edit_coordinate_grid = 1;
      status = (this->view_object)->Display_Pane(  pane );
      }
    }
    else  status = zip_pane_non_existent;
  ZIP_STATUS(this->data_object);
  OUT(zipedit::Hide_Pane_Grid);
  return status;
  }

extern long Compute_Pane_Coordinate_Deltas( class zipedit *self, zip_type_pane  pane );

long
zipedit::Constrain_Points( zip_type_pane  pane )
      {
  class zipedit *self=this;
  long				  status = zip_ok;

  IN(zipedit::Constrain_Points);
  if ( pane )
    {
      pane->zip_pane_edit->zip_pane_edit_constrain_points = 1;
      if (pane->zip_pane_edit->zip_pane_edit_coordinate_grid == 0)
	  pane->zip_pane_edit->zip_pane_edit_coordinate_grid = 1;
     Compute_Pane_Coordinate_Deltas( self, pane );
      
    }
    else  status = zip_pane_non_existent;
  ZIP_STATUS(self->data_object);
  OUT(zipedit::Constrain_points);
  return status;
  }

long
zipedit::Unconstrain_Points( zip_type_pane  pane )
      {
  class zipedit *self=this;
  long				  status = zip_ok;

  IN(zipedit::Unconstrain_Points);
  if ( pane )
    {
      pane->zip_pane_edit->zip_pane_edit_constrain_points = 0;
      if (pane->zip_pane_edit->zip_pane_edit_grid_exposed == 0)
	  pane->zip_pane_edit->zip_pane_edit_coordinate_grid = 1;
    }
    else  status = zip_pane_non_existent;
  ZIP_STATUS(self->data_object);
  OUT(zipedit::Unconstrain_Points);
  return status;
  }


long
zipedit::Halve_Pane_Grid( zip_type_pane		   pane )
      {
  long				  status = zip_ok;
  class zipedit *self=this;
  IN(zipedit::Halve_Pane_Grid);
  if ( pane )
    {
    if ( pane->zip_pane_edit->zip_pane_edit_coordinate_grid >= 2 )
      {
      pane->zip_pane_edit->zip_pane_edit_coordinate_grid /= 2;
      if ( pane->zip_pane_edit->zip_pane_edit_grid_exposed != 0 )
	  status = (this->view_object)->Display_Pane(  pane );
      else
	  status = Compute_Pane_Coordinate_Deltas( self, pane );
      }
    }
    else  status = zip_pane_non_existent;
  ZIP_STATUS(this->data_object);
  OUT(zipedit::Halve_Pane_Grid);
  return status;
  }

long
zipedit::Double_Pane_Grid( zip_type_pane		   pane )
      {
  long				  status = zip_ok;
  class zipedit *self=this;

  IN(zipedit::Double_Pane_Grid);
  if ( pane )
    {
    if ( pane->zip_pane_edit->zip_pane_edit_coordinate_grid != 0 )
      {
      pane->zip_pane_edit->zip_pane_edit_coordinate_grid *= 2;
      if ( pane->zip_pane_edit->zip_pane_edit_grid_exposed != 0 )
	  status = (self)->Draw_Pane_Grid(  pane );
      else
	  status = Compute_Pane_Coordinate_Deltas( self, pane );
      }
    }
    else  status = zip_pane_non_existent;
  ZIP_STATUS(this->data_object);
  OUT(zipedit::Double_Pane_Grid);
  return status;
  }

long
zipedit::Expose_Pane_Coordinates( zip_type_pane		   pane )
      {
  long				  status = zip_ok;

  IN(zipedit::Expose_Pane_Coordinates);
  if ( pane )
    {
    if ( ! pane->zip_pane_state.zip_pane_state_coordinates_exposed )
      {
      pane->zip_pane_state.zip_pane_state_coordinates_exposed = true;
      pane->zip_pane_preserved_border_thickness = pane->zip_pane_border_thickness;
      pane->zip_pane_border_thickness = ZIP_pane_coordinate_thickness;
      status = (this->view_object)->Display_Pane(  pane );
      }
    } 
    else  status = zip_pane_non_existent;
  ZIP_STATUS(this->data_object);
  OUT(zipedit::Expose_Pane_Coordinates);
  return status;
  }

long
zipedit::Hide_Pane_Coordinates( zip_type_pane		   pane )
      {
  long				  status = zip_ok;

  IN(zipedit::Hide_Pane_Coordinates);
  if ( pane )
    {
    if ( pane->zip_pane_state.zip_pane_state_coordinates_exposed )
      {
      pane->zip_pane_state.zip_pane_state_coordinates_exposed = false;
      pane->zip_pane_border_thickness = pane->zip_pane_preserved_border_thickness;
      status = (this->view_object)->Display_Pane(  pane );
      }
    }
    else  status = zip_pane_non_existent;
  ZIP_STATUS(this->data_object);
  OUT(zipedit::Hide_Pane_Coordinates);
  return status;
  }

long
zipedit::Delete_Figure( zip_type_figure		   figure, zip_type_pane		   pane )
        {
  long				  status= zip_ok;
  class zipedit *self=this;

  IN(zipedit::Delete_Figure);
  if ( figure )
    {
    if ( pane  &&  figure->zip_figure_visibility == zip_figure_exposed )
      {
      (this->view_object)->Set_Pane_Clip_Area(  pane );
      if ( figure->zip_figure_state.zip_figure_state_points_highlighted )
        (self)->Normalize_Figure_Points(  figure, pane );
      status = (Objects(figure->zip_figure_type))->Clear_Object(  figure, pane );
      }
    figure->zip_figure_visibility = zip_figure_hidden;
    figure->zip_figure_state.zip_figure_state_deleted = true;
    if ( figure == CurrentFigure )  CurrentFigure = NULL;
    (Data )->SetModified( );
    figure->zip_figure_image->zip_image_stream->zip_stream_states.zip_stream_state_modified = 1;
    }
    else  status = zip_figure_non_existent;
  ZIP_STATUS(this->data_object);
  OUT(zipedit::Delete_Figure);
  return status;
  }

long
zipedit::Undelete_Figure( zip_type_figure		   figure, zip_type_pane		   pane )
        {
   long				  status= zip_ok;
   class zipedit *self=this;
  IN(zipedit::Undelete_Figure);
  if ( figure )
    {
    if ( pane  &&  figure->zip_figure_visibility == zip_figure_hidden )
      {
      (this->view_object)->Set_Pane_Clip_Area(  pane );
      status = (Objects(figure->zip_figure_type))->Draw_Object(  figure, pane );
      figure->zip_figure_visibility = zip_figure_exposed;
      }
    figure->zip_figure_state.zip_figure_state_deleted = false;
    }
    else  status = zip_figure_non_existent;
  ZIP_STATUS(this->data_object);
  OUT(zipedit::Undelete_Figure);
  return status;
 }

long
zipedit::Which_Figure_Point( zip_type_figure		   figure, zip_type_pane		  pane, long				   x , long				   y )
          {
  long				  point = 0;
  long				  status = zip_ok;
  class zipedit *self=this;

  IN(zipedit::Which_Figure_Point);
  if ( figure )
    {
    DEBUGst(Figure-name,figure->zip_figure_name);
    DEBUGdt(Figure-type,figure->zip_figure_type);
    point = (Objects(figure->zip_figure_type))->Proximate_Object_Points( 
					       figure, pane, x, y );
    }
  OUT(zipedit::Which_Figure_Point);
  ZIP_STATUS(this->data_object);
  return point;
  }

long
zipedit::Highlight_Figure_Points( zip_type_figure		   figure, zip_type_pane		   pane )
        {
  long				  status = zip_ok;
  class zipedit *self=this;
  IN(zipedit::Highlight_Figure_Points);
  if ( pane  &&  figure  &&  figure->zip_figure_visibility == zip_figure_exposed  &&
       ! figure->zip_figure_state.zip_figure_state_points_highlighted )
    { 
    (this->view_object)->Set_Pane_Clip_Area(  pane );
    status = (Objects(figure->zip_figure_type))->Highlight_Object_Points(  figure, pane );
    }
  if ( figure ) figure->zip_figure_state.zip_figure_state_points_highlighted = true;
  ZIP_STATUS(this->data_object);
  OUT(zipedit::Highlight_Figure_Points);
  return  status;
  }

long
zipedit::Normalize_Figure_Points( zip_type_figure		   figure, zip_type_pane		   pane )
        {
  long				  status = zip_ok;
  class zipedit *self=this;
  IN(zipedit::Normalize_Figure_Points);
  if ( pane  &&  figure  &&  figure->zip_figure_visibility == zip_figure_exposed  &&
       figure->zip_figure_state.zip_figure_state_points_highlighted )
    {
    (this->view_object)->Set_Pane_Clip_Area(  pane );
    status = (Objects(figure->zip_figure_type))->Normalize_Object_Points(  figure, pane );
    }
  if ( figure ) figure->zip_figure_state.zip_figure_state_points_highlighted = false;
  ZIP_STATUS(this->data_object);
  OUT(zipedit::Normalize_Figure_Points);
  return zip_ok;
  }

long
zipedit::Hide_Figure_Points( zip_type_figure		   figure, zip_type_pane		   pane )
        {
  long				  status = zip_ok;

  IN(zipedit::Hide_Figure_Points);
  if ( pane  &&  figure  &&  figure->zip_figure_state.zip_figure_state_points_exposed )
    {
    (this->view_object)->Set_Pane_Clip_Area(  pane );
    figure->zip_figure_state.zip_figure_state_points_exposed = false;
    (this->view_object)->Display_Pane(  pane );
    }
  ZIP_STATUS(this->data_object);
  OUT(zipedit::Hide_Figure_Points);
  return zip_ok;
  }

long
zipedit::Expose_Figure_Points( zip_type_figure		   figure, zip_type_pane		   pane )
        {
  long				  status = zip_ok;

  IN(zipedit::Expose_Figure_Points);
  if ( pane  &&  figure  &&  ! figure->zip_figure_state.zip_figure_state_points_exposed )
    {
    (this->view_object)->Set_Pane_Clip_Area(  pane );
    figure->zip_figure_state.zip_figure_state_points_exposed = true;
    (this->view_object)->Draw_Figure(  figure, pane );
    }
  ZIP_STATUS(this->data_object);
  OUT(zipedit::Expose_Figure_Points);
  return zip_ok;
  }

long
zipedit::Delete_Image( zip_type_image		   image, zip_type_pane		   pane )
        {
  long				  status = zip_ok;
  zip_type_figure		  figure_ptr;
  class zipedit *self=this;
  IN(zipedit::Delete_Image);
  if ( image )
    {
    (this->view_object)->Set_Pane_Clip_Area(  pane );
    image->zip_image_visibility = zip_image_hidden;
    image->zip_image_state.zip_image_state_deleted = true;
    figure_ptr = image->zip_image_figure_anchor;
    while ( figure_ptr  &&  status == zip_ok )
      {
      status = (self)->Delete_Figure(  figure_ptr, pane );
      figure_ptr = figure_ptr->zip_figure_next;
      }
      if ( image->zip_image_inferior  &&  status == zip_ok )
    status = Delete_Inferior_Image( self, image->zip_image_inferior, pane );
    }
    else
    status = zip_image_non_existent;
  ZIP_STATUS(this->data_object);
  OUT(zipedit::Delete_Image);
  return status;
  }

static int
Delete_Inferior_Image( class zipedit		  *self, zip_type_image		   image, zip_type_pane		   pane )
        {
  long				  status = zip_ok;
  zip_type_figure		  figure_ptr;

  IN(Delete_Inferior_Image);
  image->zip_image_visibility = zip_image_hidden;
  image->zip_image_state.zip_image_state_deleted = true;
  figure_ptr = image->zip_image_figure_anchor;
  while ( figure_ptr  &&  status == zip_ok )
    {
    status = (self)->Delete_Figure(  figure_ptr, pane );
    figure_ptr = figure_ptr->zip_figure_next;
    }
  if ( image->zip_image_inferior  &&  status == zip_ok )
    status = Delete_Inferior_Image( self, image->zip_image_inferior, pane );
  if ( image->zip_image_peer  &&  status == zip_ok )
    status = Delete_Inferior_Image( self, image->zip_image_peer, pane );
  OUT(Delete_Inferior_Image);
  return status;
  }

long
zipedit::Undelete_Image( zip_type_image		   image, zip_type_pane		   pane )
        {
  long						  status = zip_ok;
  zip_type_figure				  figure_ptr;
  class zipedit *self=this;

  IN(zipedit::Undelete_Image);
  if ( image )
    {
    (this->view_object)->Set_Pane_Clip_Area(  pane );
    image->zip_image_visibility = zip_image_exposed;
    image->zip_image_state.zip_image_state_deleted = false;
    figure_ptr = image->zip_image_figure_anchor;
    while ( figure_ptr  &&  status == zip_ok )
      {
      status = (self)->Undelete_Figure(  figure_ptr, pane );
      figure_ptr = figure_ptr->zip_figure_next;
      }
    if ( image->zip_image_inferior  &&  status == zip_ok )
      status = Undelete_Inferior_Image( self, image->zip_image_inferior, pane );
    }
    else
    status = zip_image_non_existent;
  ZIP_STATUS(this->data_object);
  OUT(zipedit::Undelete_Image);
  return status;
  }

static int
Undelete_Inferior_Image( class zipedit		  *self, zip_type_image		   image, zip_type_pane		   pane )
        {
  long				  status = zip_ok;
  zip_type_figure		  figure_ptr;

  IN(Undelete_Inferior_Image);
  image->zip_image_visibility = zip_image_exposed;
  image->zip_image_state.zip_image_state_deleted = false;
  figure_ptr = image->zip_image_figure_anchor;
  while ( figure_ptr  &&  status == zip_ok )
    {
    status = (self)->Undelete_Figure(  figure_ptr, pane );
    figure_ptr = figure_ptr->zip_figure_next;
    }
  if ( image->zip_image_inferior  &&  status == zip_ok )
    status = Undelete_Inferior_Image( self, image->zip_image_inferior, pane );
  if ( image->zip_image_peer  &&  status == zip_ok )
    status = Undelete_Inferior_Image( self, image->zip_image_peer, pane );
  OUT(Undelete_Inferior_Image);
  return status;
  }

long
zipedit::Highlight_Image_Points( zip_type_image		   image, zip_type_pane		   pane )
        {
  zip_type_figure		  figure_ptr;
  long				  status = zip_ok;
  class zipedit *self=this;

  IN(zipedit::Highlight_Image_Points);
  if ( image  &&  pane  &&   image->zip_image_visibility == zip_image_exposed )
    {
    (this->view_object)->Set_Pane_Clip_Area(  pane );
    figure_ptr = image->zip_image_figure_anchor;
    while ( figure_ptr  &&  status == zip_ok )
      {
      status = (self)->Highlight_Figure_Points(  figure_ptr, pane );
      figure_ptr = figure_ptr->zip_figure_next;
      }
    if ( image->zip_image_inferior  &&  status == zip_ok )
      status = Highlight_Inferior_Image_Points( self, image->zip_image_inferior, pane );
    }
  ZIP_STATUS(this->data_object);
  OUT(zipedit::Highlight_Image_Points);
  return status;
  }


int
Highlight_Inferior_Image_Points( class zipedit		  *self, zip_type_image		   image, zip_type_pane		   pane )
        {
  zip_type_figure		  figure_ptr;
  long				  status = zip_ok;

  IN(zip_Highlight_Inferior_Image_Points);
  if ( image->zip_image_visibility == zip_image_exposed )
    {
    figure_ptr = image->zip_image_figure_anchor;
    while ( figure_ptr  &&  status == zip_ok )
      {
      status = (self)->Highlight_Figure_Points(  figure_ptr, pane );
      figure_ptr = figure_ptr->zip_figure_next;
      }
    if ( image->zip_image_inferior  &&  status == zip_ok )
      status = Highlight_Inferior_Image_Points( self, image->zip_image_inferior, pane );
    if ( image->zip_image_peer  &&  status == zip_ok )
      status = Highlight_Inferior_Image_Points( self, image->zip_image_peer, pane );
    }
  ZIP_STATUS(self->data_object);
  OUT(zip_Highlight_Inferior_Image_Points);
  return status;
  }

long
zipedit::Normalize_Image_Points( zip_type_image		   image, zip_type_pane		   pane )
        {
  zip_type_figure		  figure_ptr;
  long				  status = zip_ok;
  class zipedit *self=this;

  IN(zip_Normalize_Image_Points);
  if ( image  &&  pane  &&  image->zip_image_visibility == zip_image_exposed )
    {
    (this->view_object)->Set_Pane_Clip_Area(  pane );
    figure_ptr = image->zip_image_figure_anchor;
    while ( figure_ptr  &&  status == zip_ok )
      {
      status = (self)->Normalize_Figure_Points(  figure_ptr, pane );
      figure_ptr = figure_ptr->zip_figure_next;
      }
    if ( image->zip_image_inferior  &&  status == zip_ok )
      status = Normalize_Inferior_Image_Points( self, image->zip_image_inferior, pane );
    }
  ZIP_STATUS(this->data_object);
  OUT(zip_Normalize_Image_Points);
  return zip_ok;
  }

static int
Normalize_Inferior_Image_Points( class zipedit		  *self, zip_type_image		   image, zip_type_pane		   pane )
        {
  zip_type_figure		  figure_ptr;
  long				  status = zip_ok;

  IN(Normalize_Inferior_Image_Points);
  if ( image->zip_image_visibility == zip_image_exposed )
    {
    figure_ptr = image->zip_image_figure_anchor;
    while ( figure_ptr  &&  status == zip_ok )
      {
      status = (self)->Normalize_Figure_Points(  figure_ptr, pane );
      figure_ptr = figure_ptr->zip_figure_next;
      }
    if ( image->zip_image_inferior  &&  status == zip_ok )
      status = Normalize_Inferior_Image_Points( self, image->zip_image_inferior, pane );
    if ( image->zip_image_peer  &&  status == zip_ok )
      status = Normalize_Inferior_Image_Points( self, image->zip_image_peer, pane );
    }
  ZIP_STATUS(self->data_object);
  OUT(Normalize_Inferior_Image_Points);
  return status;
  }

long
zipedit::Hide_Image_Points( zip_type_image		   image, zip_type_pane		   pane )
        {
  zip_type_figure		  figure_ptr;
  long				  status = zip_ok;
  class zipedit *self=this;

  IN(zip_Hide_Image_Points);
  if ( image  &&  pane )
    {
    (this->view_object)->Set_Pane_Clip_Area(  pane );
    figure_ptr = image->zip_image_figure_anchor;
    while ( figure_ptr )
      {
      (self)->Hide_Figure_Points(  figure_ptr, pane );
      figure_ptr = figure_ptr->zip_figure_next;
      }
    }
  ZIP_STATUS(this->data_object);
  OUT(zip_Hide_Image_Points);
  return zip_ok;
  }

long
zipedit::Expose_Image_Points( zip_type_image		   image, zip_type_pane		   pane )
        {
  zip_type_figure		  figure_ptr;
  long				  status = zip_ok;
  class zipedit *self=this;

  IN(zip_Expose_Image_Points);
  if ( image  &&  pane )
    {
    (this->view_object)->Set_Pane_Clip_Area(  pane );
    figure_ptr = image->zip_image_figure_anchor;
    while ( figure_ptr )
      {
      (self)->Expose_Figure_Points(  figure_ptr, pane );
      figure_ptr = figure_ptr->zip_figure_next;
      }
    }
  ZIP_STATUS(this->data_object);
  OUT(zip_Expose_Image_Points);
  return zip_ok;
  }


long
Delete_Stream( class zipedit		  *self, zip_type_stream		   stream, zip_type_pane		   pane )
        {
return zip_ok;
  }

long
Undelete_Stream( class zipedit		  *self, zip_type_stream		   stream, zip_type_pane		   pane )
        {
return zip_ok;
  }

long
Highlight_Stream_Points( class zipedit		  *self, zip_type_stream		   stream, zip_type_pane		   pane )
        {
return zip_ok;
  }

long
Normalize_Stream_Points( class zipedit		  *self, zip_type_stream		   stream, zip_type_pane		   pane )
        {
return zip_ok;
  }

long
Hide_Stream_Points( class zipedit		  *self, zip_type_stream		   stream, zip_type_pane		   pane )
        {
return zip_ok;
  }

long
Expose_Stream_Points( class zipedit		  *self, zip_type_stream		   stream, zip_type_pane		   pane )
        {
return zip_ok;
  }

void zipedit::Expose_Point( zip_type_pane		   pane, zip_type_figure		   figure, zip_type_pixel		   x , zip_type_pixel		   y )
{
  class zipedit *self=this;
  char					  points[100];
  long					  xp, yp;
  class fontdesc		 *current_font = (View)->GetFont( );

  IN(zipedit::Expose_Point);
  sprintf( points, "(%ld,%ld)", x, y );  /*=== optimize ===*/
  (this->view_object)->SetTransferMode(  graphic_BLACK );
  (this->view_object)->MoveTo( (long) (((OriginX) + (x * (Flip) * (Multiplier) / (Divisor))) - 3), (long) (((OriginY) - (y * (Flop) * (Multiplier) / (Divisor))) - 3 ));
  (this->view_object)->DrawLineTo(  (long) (((OriginX) + (x * (Flip) * (Multiplier) / (Divisor))) + 3), (long) (((OriginY) - (y * (Flop) * (Multiplier) / (Divisor))) - 3 ));
  (this->view_object)->DrawLineTo( (long) (((OriginX) + (x * (Flip) * (Multiplier) / (Divisor))) + 3), (long) (((OriginY) - (y * (Flop) * (Multiplier) / (Divisor))) + 3 ));
  (this->view_object)->DrawLineTo( (long) (((OriginX) + (x * (Flip) * (Multiplier) / (Divisor))) - 3), (long) (((OriginY) - (y * (Flop) * (Multiplier) / (Divisor))) + 3 ));
  (this->view_object)->DrawLineTo( (long) (((OriginX) + (x * (Flip) * (Multiplier) / (Divisor))) - 3), (long) (((OriginY) - (y * (Flop) * (Multiplier) / (Divisor))) - 3 ));
  if ( PointsFont == NULL )
    {
    char			      family_name[257];
    long			      font_style;
    long			      font_size;

    fontdesc::ExplodeFontName( zip_points_font_name, family_name,
			      sizeof( family_name ), &font_style, & font_size );
    PointsFont = fontdesc::Create( family_name, font_style, font_size );
    }
  (this->view_object)->SetFont(  PointsFont );
  ((View )->GetFont( ))->StringSize(  (this->view_object)->GetDrawable(), points, &xp, &yp );
  (this->view_object)->SetTransferMode(  graphic_WHITE );
  (this->view_object)->FillRectSize( (long) (((OriginX) + (x * (Flip) * (Multiplier) / (Divisor))) + 5),  (long) (((OriginY) - (y * (Flop) * (Multiplier) / (Divisor))) - 5), xp , 10, (View)->WhitePattern( ) );
			 /*=== compute width and height ===*/
  (this->view_object)->SetTransferMode(  graphic_BLACK );
  (this->view_object)->MoveTo( (long) (5 + ((OriginX) + (x * (Flip) * (Multiplier) / (Divisor)))), (long) ((OriginY) - (y * (Flop) * (Multiplier) / (Divisor)) ));
  (this->view_object)->DrawString( points, graphic_ATLEFT | graphic_BETWEENTOPANDBOTTOM );
  (this->view_object)->MoveTo( (long) ((OriginX) + (x * (Flip) * (Multiplier) / (Divisor))),
		(long) ((OriginY) - (y * (Flop) * (Multiplier) / (Divisor))) );
  (this->view_object)->SetFont(  current_font );
  IN(zipedit::Expose_Point);
  }


void zipedit::Hide_Point( zip_type_pane		   pane, zip_type_figure		   figure, zip_type_pixel		   x , zip_type_pixel		   y )
          {
/*=== needed ?  ===*/
  }

long
zipedit::Highlight_Handles( zip_type_pane		   pane, zip_type_pixel		   X1 , zip_type_pixel		   X2 , zip_type_pixel		   X3 , zip_type_pixel		   Y1 , zip_type_pixel		   Y2 , zip_type_pixel		   Y3 )
{
  class zipedit *self=this;
  IN(zipedit::Highlight_Handles);
  (self)->Highlight_Point(   pane, X1, Y1 );
  (self)->Highlight_Handle(  pane, X1, Y2 );
  (self)->Highlight_Point(   pane, X1, Y3 );
  (self)->Highlight_Handle(  pane, X2, Y1 );
  (self)->Highlight_Point(   pane, X2, Y2 );
  (self)->Highlight_Handle(  pane, X2, Y3 );
  (self)->Highlight_Point(   pane, X3, Y1 );
  (self)->Highlight_Handle(  pane, X3, Y2 );
  (self)->Highlight_Point(   pane, X3, Y3 );
  OUT(zipedit::Highlight_Handles);
  return zip_ok;
  }

struct highlights
  {
  zip_type_pixel	     x, y;
  };

static struct highlights    *Highlights;
static long		     highlights_count;

static boolean
Ratify_Highlighting( class zipedit		  *self, zip_type_pane		   pane, zip_type_pixel		   x , zip_type_pixel		   y )
        {
  long				  i;
  boolean			  ratified = true;

  IN(Ratify_Highlighting);
  if ( x > 0  &&  y > 0 )
    {
    for ( i = 0; i < highlights_count; i++ )
      if ( abs(x - Highlights[i].x ) < 5  &&  abs(y - Highlights[i].y ) < 5 )
	{ DEBUG(Not Ratified);
	ratified = false;
	break;
	}
    if ( ratified )
      { DEBUG(Ratified);
      if ( highlights_count )
	Highlights = (struct highlights *) realloc( Highlights,
			++highlights_count * sizeof(struct highlights) );
	else
	Highlights = (struct highlights *) malloc( ++highlights_count * sizeof(struct highlights) );
      Highlights[highlights_count-1].x = x;
      Highlights[highlights_count-1].y = y;
      }
    }
  OUT(Ratify_Highlighting);
  return  ratified;
  }

static boolean
Ratify_Normalizing( class zipedit		  *self, zip_type_pane		   pane, zip_type_pixel		   x , zip_type_pixel		   y )
        {
  long				  i;
  boolean			  discard = true;
  boolean			  ratified = false;

  IN(Ratify_Normalizing);
  if ( x > 0  &&  y > 0 )
    {
    for ( i = 0; i < highlights_count; i++ )
      if ( x == Highlights[i].x  &&  y == Highlights[i].y )
	{ DEBUG(Ratified);
	Highlights[i].x = -1;
	ratified = true;
	break;
	}
    if ( ratified )
      { DEBUG(Attempt Purge);
      for ( i = 0; i < highlights_count; i++ )
        if ( Highlights[i].x > 0 )
	  {
	  discard = false;
	  break;
	  }
      if ( discard )
        { DEBUG(Do Purge);
        free( Highlights );
        highlights_count = 0;
        Highlights = NULL;
        }
      }
    }
  OUT(Ratify_Normalizing);
  return  ratified;
  }

long
zipedit::Highlight_Point( zip_type_pane		   pane, zip_type_pixel		   x , zip_type_pixel		   y )
        {
  class zipedit *self=this;
  IN(zipedit_Highlight_Point);
  if ( Ratify_Highlighting( self, pane, x, y ) )
    {
    if ( (View )->GetTransferMode( ) != graphic_INVERT )
      (this->view_object)->SetTransferMode(  graphic_INVERT );
    if ( IconFont == NULL )
      {
      char			      family_name[257];
      long			      font_style;
      long			      font_size;

      DEBUG(Create Icon Font);
      fontdesc::ExplodeFontName( zip_icon_font_name, family_name,
				sizeof( family_name ), &font_style, & font_size );
      IconFont = fontdesc::Create( family_name, font_style, font_size );
      }
    if ( (View )->GetFont( ) != IconFont )
      (this->view_object)->SetFont(  IconFont );
    (this->view_object)->MoveTo(  x, y );
    (this->view_object)->DrawText(  &pane->zip_pane_highlight_icon, 1, 0 );
    }
  OUT(zipedit_Highlight_Point);
  return zip_ok;
  }

long
zipedit::Highlight_Handle( zip_type_pane		   pane, zip_type_pixel		   x , zip_type_pixel		   y )
        {
  class zipedit *self=this;
  IN(zipedit_Highlight_Handle);
  if ( Ratify_Highlighting( self, pane, x, y ) )
    {
    if ( (View )->GetTransferMode( ) != graphic_INVERT )
      (this->view_object)->SetTransferMode(  graphic_INVERT );
    (this->view_object)->MoveTo(  x, y );
    (this->view_object)->FillRectSize(  x - 2, y - 2, 5, 5, (View )->WhitePattern( ) );
    }
  OUT(zipedit_Highlight_Handle);
  return zip_ok;
  }

long
zipedit::Normalize_Handles( zip_type_pane		   pane, zip_type_pixel		   X1 , zip_type_pixel		   X2 , zip_type_pixel		   X3 , zip_type_pixel		   Y1 , zip_type_pixel		   Y2 , zip_type_pixel		   Y3 )
{
  class zipedit *self=this;
  IN(zipedit::Normalize_Handles);
  (self)->Normalize_Point(   pane, X1, Y1 );
  (self)->Normalize_Handle(  pane, X1, Y2 );
  (self)->Normalize_Point(   pane, X1, Y3 );
  (self)->Normalize_Handle(  pane, X2, Y1 );
  (self)->Normalize_Point(   pane, X2, Y2 );
  (self)->Normalize_Handle(  pane, X2, Y3 );
  (self)->Normalize_Point(   pane, X3, Y1 );
  (self)->Normalize_Handle(  pane, X3, Y2 );
  (self)->Normalize_Point(   pane, X3, Y3 );
  OUT(zipedit::Normalize_Handles);
  return zip_ok;
  }


long
zipedit::Normalize_Point( zip_type_pane		   pane, zip_type_pixel		   x , zip_type_pixel		   y )
{
  class zipedit *self=this;
  IN(zipedit::Normalize_Point);
  if ( Ratify_Normalizing( self, pane, x, y ) )
    {
    if ( (View )->GetTransferMode( ) != graphic_INVERT )
      (this->view_object)->SetTransferMode(  graphic_INVERT );
    if ( IconFont == NULL )
      {
      char			      family_name[257];
      long			      font_style;
      long			      font_size;

      fontdesc::ExplodeFontName( zip_icon_font_name, family_name,
				sizeof( family_name ), &font_style, & font_size );
      IconFont = fontdesc::Create( family_name, font_style, font_size );
      }
    if ( (View )->GetFont( ) != IconFont )
      (this->view_object)->SetFont(  IconFont );
    (this->view_object)->MoveTo(  x, y );
    (this->view_object)->DrawText(  &pane->zip_pane_highlight_icon, 1, 0 );
    }
  OUT(zipedit::Normalize_Point);
  return zip_ok;
  }


long
zipedit::Normalize_Handle( zip_type_pane		   pane, zip_type_pixel		   x , zip_type_pixel		   y )
        {
  class zipedit *self=this;
  IN(zipedit::Highlight_Handle);
  if ( Ratify_Normalizing( self, pane, x, y ) )
    {
    if ( (View )->GetTransferMode( ) != graphic_INVERT )
      (this->view_object)->SetTransferMode(  graphic_INVERT );
    (this->view_object)->MoveTo(  x, y );
    (this->view_object)->FillRectSize(  x - 2, y - 2, 5, 5, (View )->WhitePattern( ) );
    }
  OUT(zipedit::Normalize_Handle);
  return zip_ok;
  }
