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
/* zipvf00.c	Zip View-object	-- Figures			      */
/* Author	TC Peters					      */
/* Information Technology Center	   Carnegie-Mellon University */


/**  SPECIFICATION -- External Facility Suite  *********************************

TITLE	The Zip View-object -- Figures

MODULE	zipvf00.c

NOTICE	IBM Internal Use Only

DESCRIPTION
	This is the suite of Methods that support the Figure facilities
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
  11/01/88	Correct Which_Figure to check to exposed state (TCP)
  05/15/89	Improve performance by dropping FlushGraphics (TCP)
   06/14/89	Allow NULL Pane arg to Hide|Expose_Figure (SCG)
  07/12/89	Added Figure_Visible method (SCG)
   08/20/90	Suppress usage of unimplemented Object_Visible() (wait for "Intersects" method) (SCG)

END-SPECIFICATION  ************************************************************/

#include <andrewos.h>
#include "view.H"
#include "zip.H"
#include "zipview.H"
#include "zipobject.H"

#define	 Data			      (self->data_object)
#define	 View			      (self)
#define  Objects(i)		    ((self->objects)[i])


static zip_type_figure Which_Figure( class zipview		  *self, zip_type_pixel		   x , zip_type_pixel		   y, zip_type_pane		   pane );


long
zipview::Display_Figure( zip_type_figure		   figure, zip_type_pane		   pane )
        {
  int				  status = zip_success;
  class zipview *self=this;

  IN(zipview::Display_Figure);
  if ( figure  &&  pane )
    {
    (self)->Set_Pane_Figure(  pane, figure );
    status = (self)->Display_Pane(  pane );
    }
    else
    if ( pane == NULL )
      status = zip_pane_non_existent;
      else
      status = zip_stream_non_existent;
  ZIP_STATUS(this->data_object);
  OUT(zipview::Display_Figure);
  return status;
  }

long
zipview::Draw_Figure( zip_type_figure		   figure, zip_type_pane		   pane )
        {
  int				  status = zip_ok;
  class zipview *self=this;
  IN(zipview::Draw_Figure);
  if ( figure  &&  pane  &&  figure->zip_figure_visibility != zip_figure_hidden  &&
       !figure->zip_figure_state.zip_figure_state_deleted )
    {
    (self)->Set_Pane_Clip_Area(  pane );
    figure->zip_figure_visibility = zip_figure_exposed;
    status = (Objects(figure->zip_figure_type))->Draw_Object(  figure, pane );
    }
    else 
    if ( pane == NULL )
      status = zip_pane_non_existent;
    else
    if ( figure == NULL )
      status = zip_figure_non_existent;
  ZIP_STATUS(this->data_object);
  OUT(zipview::Draw_Figure);
  return status;
  }

long
zipview::Clear_Figure( zip_type_figure		   figure, zip_type_pane		   pane )
        {
  int				  status = zip_ok;
  class zipview *self=this;
  IN(zipview::Clear_Figure);
  if ( figure  &&  pane  &&  figure->zip_figure_visibility == zip_figure_exposed  &&
       !figure->zip_figure_state.zip_figure_state_deleted )
    {
    (self)->Set_Pane_Clip_Area(  pane );
    figure->zip_figure_visibility = 0;
    status = (Objects(figure->zip_figure_type))->Clear_Object(  figure, pane );
    }
    else 
    if ( pane == NULL )
      status = zip_pane_non_existent;
    else
    if ( figure == NULL )
      status = zip_figure_non_existent;
  ZIP_STATUS(this->data_object);
  OUT(zipview::Clear_Figure);
  return status;
  }

long
zipview::Hide_Figure( zip_type_figure		   figure, zip_type_pane		   pane )
        {
  int				  status = zip_ok;
  class zipview *self=this;
  IN(zipview::Hide_Figure);
  if ( figure &&
       !figure->zip_figure_state.zip_figure_state_deleted )
    {
    if ( pane )
      {
      (self)->Set_Pane_Clip_Area(  pane );
      if ( figure->zip_figure_visibility == zip_figure_exposed )
        status = (Objects(figure->zip_figure_type))->Clear_Object(  figure, pane );
      }
    figure->zip_figure_visibility = zip_figure_hidden;
    }
    else
    if ( figure == NULL )
      status = zip_figure_non_existent;
  ZIP_STATUS(this->data_object);
  OUT(zipview::Hide_Figure);
  return status;
  }

long
zipview::Expose_Figure( zip_type_figure		   figure, zip_type_pane		   pane )
        {
  int				  status = zip_ok;
  class zipview *self=this;  
  IN(zipview::Expose_Figure);
  if ( figure &&
       !figure->zip_figure_state.zip_figure_state_deleted )
    {
    if ( pane )
      {
      (self)->Set_Pane_Clip_Area(  pane );
      if ( figure->zip_figure_visibility != zip_figure_exposed )
        status = (Objects(figure->zip_figure_type))->Draw_Object(  figure, pane );
      }
    figure->zip_figure_visibility = zip_figure_exposed;
    }
    else
    if ( figure == NULL )
      status = zip_figure_non_existent;
  ZIP_STATUS(this->data_object);
  OUT(zipview::Expose_Figure);
  return status;
  }

static zip_type_figure
Which_Figure( class zipview		  *self, zip_type_pixel		   x , zip_type_pixel		   y, zip_type_pane		   pane )
        {
  zip_type_figure		  figure_ptr, figure = NULL;
  zip_type_image		  image;
  zip_type_stream		  stream = NULL;

  IN(Which_Figure);
  DEBUGst(Pane-name,pane->zip_pane_name);
  if ( pane->zip_pane_attributes.zip_pane_attribute_stream_source )
      stream = pane->zip_pane_source.zip_pane_stream;
  else
  if ( pane->zip_pane_attributes.zip_pane_attribute_image_source )
      stream = pane->zip_pane_source.zip_pane_image->zip_image_stream;
  else
  if ( pane->zip_pane_attributes.zip_pane_attribute_figure_source )
      stream = pane->zip_pane_source.zip_pane_figure->zip_figure_image->zip_image_stream;
  if ( stream )
    {
    image = stream->zip_stream_image_anchor;
    DEBUGst(Image-name,image->zip_image_name);
    while ( image )
      {
      figure_ptr = image->zip_image_figure_anchor;
      while ( figure_ptr )
	{
	DEBUGst(Figure-name,figure_ptr->zip_figure_name);
        if ( figure_ptr->zip_figure_visibility == zip_figure_exposed  &&
	     (Objects(figure_ptr->zip_figure_type))->Proximate_Object_Points( 
						figure_ptr, pane, x, y ) )
	  {
	  figure = figure_ptr;
	  goto found;
	  }
        figure_ptr = figure_ptr->zip_figure_next;
	}
      image = (Data)->Next_Image(  image );
      }
    }
  found:
  OUT(Which_Figure);
  return  figure;
  }

zip_type_figure
zipview::Which_Figure( zip_type_pixel		   x , zip_type_pixel		   y )
      {
  zip_type_figure		  figure = NULL;
  zip_type_pane		  pane;
  class zipview *self=this;
  IN(zipview::Which_Figure);
  if ( ( pane = (self)->Which_Pane(  x, y ) ) )
    {
    DEBUGst(Pane-name,pane->zip_pane_name);
    figure = ::Which_Figure( self, x, y, pane );
    }
  OUT(zipview::Which_Figure);
  return  figure;
  }

zip_type_figure
zipview::Which_Pane_Figure( zip_type_pixel		   x , zip_type_pixel		   y, zip_type_pane		   pane )
        {
  zip_type_figure		  figure = NULL;
  class zipview *self=this;
  IN(zipview::Which_Pane_Figure);
  DEBUGst(Pane-name,pane->zip_pane_name);
  figure = ::Which_Figure( self, x, y, pane );
  OUT(zipview::Which_Pane_Figure);
  return  figure;
  }

zip_type_figure
zipview::Within_Which_Figure( long			       x , long			       y )
      {
  zip_type_stream	      stream;
  zip_type_image	      image;
  zip_type_figure	      figure, figure_ptr = NULL;
  zip_type_pane	      pane;
  long			      distance, best_distance = 999999;
  class zipview *self=this;
  IN(zipview::Within_Which_Figure);
  if ( (pane  = (self)->Which_Pane(  x, y ))  &&
       (stream = (self)->Pane_Stream(  pane ))
     ) 
    {
    DEBUGst(Pane-name,pane->zip_pane_name);
    DEBUGst(Stream-name,stream->zip_stream_name);
    image = stream->zip_stream_image_anchor;
    while ( image )
      {
      DEBUGst(Image-name,image->zip_image_name);
      figure = image->zip_image_figure_anchor;
      while( figure )
	{
        DEBUGst(Figure-name,figure->zip_figure_name);
	if ( !figure->zip_figure_state.zip_figure_state_deleted )
	  if ( (distance = (Objects(figure->zip_figure_type))->Within_Object(
		 figure, pane, x, y )) > -1 )
	    {
	    if ( distance < best_distance  ||  figure->zip_figure_mode.zip_figure_mode_shaded )
	      { DEBUGdt(Best-distance,distance);
	      best_distance = distance;
              figure_ptr = figure;
	      }
	    }
	figure = figure->zip_figure_next;
	}
      image = (Data)->Next_Image(  image );
      }
    }
  OUT(zipview::Within_Which_Figure);
  return  figure_ptr;
  }

boolean
zipview::Figure_Visible( zip_type_figure		     figure, zip_type_pane		     pane )
        {
  boolean			    status = FALSE;
  class zipview *self=this;
  IN( zipview::Figure_Visible );
  if ( figure && pane )
    {
      if ( (self)->Image_Visible(  (Data)->Figure_Image(  figure ), pane )
             == FALSE )
        status = FALSE;
        else status = TRUE; /* === ===*/
/*         status = zipobject_Object_Visible( Objects( figure->zip_figure_type ), figure, pane ); */
    }
  OUT( zipview::Figure_Visible );
  return status;
  }
