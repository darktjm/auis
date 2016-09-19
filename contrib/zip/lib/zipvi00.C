/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

/*
 * P_R_P_Q_# (C) COPYRIGHT IBM CORPORATION 1988
 * LICENSED MATERIALS - PROPERTY OF IBM
 * REFER TO COPYRIGHT INSTRUCTIONS FORM NUMBER G120-2083
 */
/* zipvi00.c	Zip View-object	-- Images			      */
/* Author	TC Peters					      */
/* Information Technology Center	   Carnegie-Mellon University */


/*
    $Log: zipvi00.C,v $
 * Revision 1.3  1993/06/17  04:28:00  rr2b
 * made apt_MM_compare a macro which uses ULstrcmp
 * to avoid multiple definitions of apt_MM_compare.
 *
 * Revision 1.2  1993/06/09  17:45:37  gk5g
 * functional C++ version
 *
 * Revision 1.3  1992/12/15  21:58:46  rr2b
 * more disclaimerization fixing
 *
 * Revision 1.2  1992/12/15  04:00:36  rr2b
 * disclaimerization
 *
 * Revision 1.1  1992/10/06  18:33:00  susan
 * Initial revision
 *
 * Revision 2.8  1991/09/12  16:45:25  bobg
 * Update copyright notice and rcsid
 *
 * Revision 2.7  1991/09/10  20:52:14  gk5g
 * Changes in support of SGI_4d platform.
 * Mostly added forward delcarations.
 *
 * Revision 2.6  1989/07/20  13:13:13  sg08
 * introduced Image_Visible method
 *
 * Revision 2.5  89/02/17  18:10:31  ghoti
 * ifdef/endif,etc. label fixing - courtesy of Ness
 * 
 * Revision 2.4  89/02/08  16:52:46  ghoti
 * change copyright notice
 * 
 * Revision 2.3  89/02/07  21:03:04  ghoti
 * first pass porting changes: filenames and references to them
 * 
 * Revision 2.2  88/11/16  18:35:22  tom
 * Optimize Draw Figure by doing it directly from Image.
 * 
 * Revision 2.1  88/09/27  18:20:40  ghoti
 * adjusting rcs #
 * 
 * Revision 1.2  88/09/15  17:52:43  ghoti
 * copyright fix
 * 
 * Revision 1.1  88/09/14  17:48:05  tom
 * Initial revision
 * 
*/

/**  SPECIFICATION -- External Facility Suite  *********************************

TITLE	The Zip View-object -- Images

MODULE	zipvi00.c

NOTICE	IBM Internal Use Only

DESCRIPTION
	This is the suite of Methods that support the Image facilities
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
  10/26/88	Optimize Draw Figure by doing it directly (TCP)
  07/12/89	Added Image_Visible method (SCG)

END-SPECIFICATION  ************************************************************/

#include <andrewos.h>
#include "view.H"
#include "zip.H"
#include "zipview.H"
#include "zipobject.H"

#define	 Data			      (self->data_object)
#define	 View			      (self)
#define  Objects(i)		    ((self->objects)[i])

static int Draw_Inferior_Image( class zipview		  *self, zip_type_image		   image, zip_type_pane		   pane );
static int Draw_Figure( class zipview		  *self, zip_type_figure		   figure, zip_type_pane		   pane );
static int Clear_Inferior_Image( class zipview		  *self, zip_type_image		   image, zip_type_pane		   pane );
static int Hide_Inferior_Image( class zipview		  *self, zip_type_image		   image, zip_type_pane		   pane );
static int Expose_Inferior_Image( class zipview		  *self, zip_type_image		   image, zip_type_pane		   pane );


long
zipview::Display_Image( zip_type_image		   image, zip_type_pane		   pane )
        {
  int				  status = zip_success;
  class zipview *self=this;

  IN(zipview::Display_Image);
  if ( image  &&  pane )
    {
    (self)->Set_Pane_Image(  pane, image );
    status = (self)->Display_Pane(  pane );
    }
    else 
    {
    if ( pane == NULL )
      status = zip_pane_non_existent;
      else
      status = zip_image_non_existent;
    }
  (self )->FlushGraphics( );
  ZIP_STATUS(this->data_object);
  OUT(zipview::Display_Image);
  return status;
  }

long
zipview::Draw_Image( zip_type_image		   image, zip_type_pane		   pane )
        {
  int				  status = zip_success;
  zip_type_figure		  figure_ptr;
  class zipview *self=this;

  IN(zipview::Draw_Image);
  if ( image  &&  pane )
    {
    if ( image->zip_image_visibility != zip_image_hidden )
      {
      (self)->Set_Pane_Clip_Area(  pane );
      image->zip_image_visibility = zip_image_exposed;
      figure_ptr = image->zip_image_figure_anchor;
      while ( figure_ptr  &&  status == zip_success )
        {
	  status = ::Draw_Figure( self, figure_ptr, pane );
        figure_ptr = figure_ptr->zip_figure_next;
        }
      }
    if ( image->zip_image_inferior  &&  status == zip_success )
      status = Draw_Inferior_Image( self, image->zip_image_inferior, pane );
    }
    else 
    {
    if ( pane == NULL )
      status = zip_pane_non_existent;
      else
      status = zip_image_non_existent;
    }
  (self )->FlushGraphics( );
  ZIP_STATUS(this->data_object);
  OUT(zipview::Draw_Image);
  return status;
  }

static int
Draw_Inferior_Image( class zipview		  *self, zip_type_image		   image, zip_type_pane		   pane )
        {
  int				  status = zip_success;
  zip_type_figure		  figure_ptr;

  IN(Draw_Inferior_Image);
  if ( image->zip_image_visibility != zip_image_hidden )
    {
    image->zip_image_visibility = zip_image_exposed;
    figure_ptr = image->zip_image_figure_anchor;
    while ( figure_ptr  &&  status == zip_success )
      {
	status = ::Draw_Figure( self, figure_ptr, pane );
      figure_ptr = figure_ptr->zip_figure_next;
      }
    }
  if ( image->zip_image_inferior  &&  status == zip_success )
    status = Draw_Inferior_Image( self, image->zip_image_inferior, pane );
  if ( image->zip_image_peer  &&  status == zip_success )
    status = Draw_Inferior_Image( self, image->zip_image_peer, pane );
  OUT(Draw_Inferior_Image);
  return status;
  }

static int
Draw_Figure( class zipview		  *self, zip_type_figure		   figure, zip_type_pane		   pane )
        {
  long				  status = zip_ok;

  if ( figure->zip_figure_visibility != zip_figure_hidden  &&
     !figure->zip_figure_state.zip_figure_state_deleted )
    {
    figure->zip_figure_visibility = zip_figure_exposed;
    status = (Objects(figure->zip_figure_type))->Draw_Object(  figure, pane );
    }
  return  status;
  }

long
zipview::Clear_Image( zip_type_image		   image, zip_type_pane		   pane )
        {
  int				  status = zip_success;
  zip_type_figure		  figure_ptr;
  class zipview *self=this;

  IN(zipview::Clear_Image);
  if ( image  &&  pane )
    {
    if ( image->zip_image_visibility == zip_image_exposed )
      {
      image->zip_image_visibility = 0;
      figure_ptr = image->zip_image_figure_anchor;
      while ( figure_ptr  &&  status == zip_success )
        {
        status = (self)->Clear_Figure(  figure_ptr, pane );
        figure_ptr = figure_ptr->zip_figure_next;
        }
      }
    if ( image->zip_image_inferior  &&  status == zip_success )
      status = Clear_Inferior_Image( self, image->zip_image_inferior, pane );
    }
    else 
    {
    if ( pane == NULL )
      status = zip_pane_non_existent;
      else
      status = zip_image_non_existent;
    }
  (self )->FlushGraphics( );
  ZIP_STATUS(this->data_object);
  OUT(zipview::Clear_Image);
  return status;
  }

static int
Clear_Inferior_Image( class zipview		  *self, zip_type_image		   image, zip_type_pane		   pane )
        {
  int				  status = zip_success;
  zip_type_figure		  figure_ptr;

  IN(Clear_Inferior_Image);
  if ( image->zip_image_visibility == zip_image_exposed )
    {
    image->zip_image_visibility = 0;
    figure_ptr = image->zip_image_figure_anchor;
    while ( figure_ptr  &&  status == zip_success )
      {
      status =(self)->Clear_Figure(  figure_ptr, pane );
      figure_ptr = figure_ptr->zip_figure_next;
      }
    }
  if ( image->zip_image_inferior  &&  status == zip_success )
    status = Clear_Inferior_Image( self, image->zip_image_inferior, pane );
  if ( image->zip_image_peer  &&  status == zip_success )
    status = Clear_Inferior_Image( self, image->zip_image_peer, pane );
  OUT(Clear_Inferior_Image);
  return status;
  }

long
zipview::Hide_Image( zip_type_image		   image, zip_type_pane		   pane )
        {
  int				  status = zip_success;
  zip_type_figure		  figure_ptr;
  class zipview *self=this;

  IN(zipview::Hide_Image);
  if ( image )
    {
    image->zip_image_visibility = zip_image_hidden;
    figure_ptr = image->zip_image_figure_anchor;
    while ( figure_ptr  &&  status == zip_success )
      {
      status = (self)->Hide_Figure(  figure_ptr, pane );
      figure_ptr = figure_ptr->zip_figure_next;
      }
    if ( image->zip_image_inferior  &&  status == zip_success )
      status = Hide_Inferior_Image( self, image->zip_image_inferior, pane );
    }
    else 
    status = zip_image_non_existent;
  (self )->FlushGraphics( );
  ZIP_STATUS(this->data_object);
  OUT(zipview::Hide_Image);
  return status;
  }

static int
Hide_Inferior_Image( class zipview		  *self, zip_type_image		   image, zip_type_pane		   pane )
        {
  int				  status = zip_success;
  zip_type_figure		  figure_ptr;

  IN(Hide_Inferior_Image);
  image->zip_image_visibility = zip_image_hidden;
  figure_ptr = image->zip_image_figure_anchor;
  while ( figure_ptr  &&  status == zip_success )
    {
    status = (self)->Hide_Figure(  figure_ptr, pane );
    figure_ptr = figure_ptr->zip_figure_next;
    }
  if ( image->zip_image_inferior  &&  status == zip_success )
    status = Hide_Inferior_Image( self, image->zip_image_inferior, pane );
  if ( image->zip_image_peer  &&  status == zip_success )
    status = Hide_Inferior_Image( self, image->zip_image_peer, pane );
  OUT(Hide_Inferior_Image);
  return status;
  }

long
zipview::Expose_Image( zip_type_image		   image, zip_type_pane		   pane )
        {
  int				  status = zip_success;
  zip_type_figure		  figure_ptr;
  class zipview *self=this;

  IN(zipview::Expose_Image);
  if ( image )
    {
    image->zip_image_visibility = zip_image_exposed;
    figure_ptr = image->zip_image_figure_anchor;
    while ( figure_ptr  &&  status == zip_success )
      {
      status = (self)->Expose_Figure(  figure_ptr, pane );
      figure_ptr = figure_ptr->zip_figure_next;
      }
    if ( image->zip_image_inferior  &&  status == zip_success )
      status = Expose_Inferior_Image( self, image->zip_image_inferior, pane );
    }
    else 
    status = zip_image_non_existent;
  (self )->FlushGraphics( );
  ZIP_STATUS(this->data_object);
  OUT(zipview::Expose_Image);
  return status;
  }

static int
Expose_Inferior_Image( class zipview		  *self, zip_type_image		   image, zip_type_pane		   pane )
        {
  int				  status = zip_success;
  zip_type_figure		  figure_ptr;

  IN(Expose_Inferior_Image);
  image->zip_image_visibility = zip_image_exposed;
  figure_ptr = image->zip_image_figure_anchor;
  while ( figure_ptr  &&  status == zip_success )
    {
    status = (self)->Expose_Figure(  figure_ptr, pane );
    figure_ptr = figure_ptr->zip_figure_next;
    }
  if ( image->zip_image_inferior  &&  status == zip_success )
    status = Expose_Inferior_Image( self, image->zip_image_inferior, pane );
  if ( image->zip_image_peer  &&  status == zip_success )
    status = Expose_Inferior_Image( self, image->zip_image_peer, pane );
  OUT(Expose_Inferior_Image);
  return status;
  }

zip_type_image
zipview::Which_Image( long				   x , long				   y )
      {
  zip_type_image		  image = NULL;
  zip_type_figure		  figure;
  int				  status = zip_success;
  class zipview *self=this;

  IN(zippview_Which_Image);
  if ( ( figure = (self)->Which_Figure(  x, y ) ) )
    image = figure->zip_figure_image;
  ZIP_STATUS(this->data_object);
  OUT(zippview_Which_Image);
  return image;
  }

boolean
zipview::Image_Visible( zip_type_image		     image, zip_type_pane		     pane )
        {
  boolean			    status = FALSE;
  class zipview *self=this;

  IN( zipview::Image_Visible );
  if ( image && pane )
    {
    if ( !(( (Data)->Image_Greatest_X(  image ) <
          (self)->X_Pixel_To_Point(  pane, NULL, (self)->Pane_Left(  pane ))) ||
        ( (Data)->Image_Greatest_Y(  image ) <
          (self)->Y_Pixel_To_Point(  pane, NULL, (self)->Pane_Bottom(  pane ))) ||
        ( (Data)->Image_Least_X(  image ) >
          (self)->X_Pixel_To_Point(  pane, NULL, (self)->Pane_Right(  pane ))) ||
        ( (Data)->Image_Least_Y(  image ) >
          (self)->Y_Pixel_To_Point(  pane, NULL, (self)->Pane_Top(  pane )))))
      status = TRUE;
    }
  OUT( zipview::Image_Visible );
  return status;
  }
