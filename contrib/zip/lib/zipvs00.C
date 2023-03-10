/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

/*
 * P_R_P_Q_# (C) COPYRIGHT IBM CORPORATION 1988
 * LICENSED MATERIALS - PROPERTY OF IBM
 * REFER TO COPYRIGHT INSTRUCTIONS FORM NUMBER G120-2083
 */
/* zipvs00.c	Zip View-object	-- Stream			      */
/* Author	TC Peters					      */
/* Information Technology Center	   Carnegie-Mellon University */


/**  SPECIFICATION -- External Facility Suite  *********************************

TITLE	The Zip View-object -- Stream

MODULE	zipvs00.c

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
  07/12/89	Added Stream_Visible method (SCG)

END-SPECIFICATION  ************************************************************/

#include <andrewos.h>
#include "view.H"
#include "zip.H"
#include "zipview.H"
#include "zipifm00.h"

#define	 Data			      (self->data_object)
#define	 View			      (self)


long
zipview::Display_Stream( zip_type_stream		   stream, zip_type_pane		   pane )
        {
  class zipview *self=this;
  int				  status = zip_success;

  IN(zipview::Display_Stream);
  if ( pane  &&  stream )
    {
    (self)->Set_Pane_Stream(  pane, stream );
    status = (self)->Display_Pane(  pane );
    }
    else 
    {
    if ( pane == NULL )
      status = zip_pane_non_existent;
      else
      status = zip_stream_non_existent;
    }
  (self )->FlushGraphics( );
  ZIP_STATUS(this->data_object);
  OUT(zipview::Display_Stream);
  return status;
  }

long
zipview::Draw_Stream( zip_type_stream		   stream, zip_type_pane		   pane )
        {
  class zipview *self=this;
  int				  status = zip_success;

  IN(zipview::Draw_Stream);
  if ( pane  &&  stream )
    {
    status = (self)->Draw_Image(  stream->zip_stream_image_anchor, pane );
    }
    else 
    {
    if ( pane == NULL )
      status = zip_pane_non_existent;
      else
      status = zip_stream_non_existent;
    }
  (self)->FlushGraphics( );
  ZIP_STATUS(this->data_object);
  OUT(zipview::Draw_Stream);
  return status;
  }

long
zipview::Clear_Stream( zip_type_stream		   stream, zip_type_pane		   pane )
        {
return zip_ok;/*===*/
  }

long
zipview::Hide_Stream( zip_type_stream   stream,zip_type_pane pane )
        {
return zip_ok;/*===*/
  }

long
zipview::Expose_Stream( zip_type_stream  stream, zip_type_pane  pane )
        {
return zip_ok;/*===*/
  }

zip_type_stream
zipview::Which_Stream( long	 x , long  y )
{
  class zipview *self=this;
  int				  status = zip_success;
  zip_type_stream		  stream = NULL;
  zip_type_pane		  pane;

  IN(zip_Which_Stream);
  if ( (pane = (self)->Which_Pane(  x, y )) ) {
    if ( pane->zip_pane_attributes.zip_pane_attribute_stream_source )
      stream = pane->zip_pane_source.zip_pane_stream;
    else
    if ( pane->zip_pane_attributes.zip_pane_attribute_image_source )
      stream = pane->zip_pane_source.zip_pane_image->zip_image_stream;
    else
    if ( pane->zip_pane_attributes.zip_pane_attribute_figure_source )
      stream = pane->zip_pane_source.zip_pane_figure->zip_figure_image->zip_image_stream;
  }
  ZIP_STATUS(this->data_object);
  OUT(zip_Which_Stream);
  return stream;
  }

zip_type_stream
zipview::Within_Which_Stream( long x , long y )
      {
return zip_ok;/*===*/
  }

boolean
zipview::Stream_Visible( zip_type_stream stream, zip_type_pane		     pane )
        {
  class zipview *self=this;
  boolean			    status = FALSE;

  IN( zipview::Stream_Visible );
  if ( stream && pane )
    status = (self)->Image_Visible(  (Data)->Image_Root(  stream ), pane );
  OUT( zipview::Stream_Visible );
  return status;
  }
