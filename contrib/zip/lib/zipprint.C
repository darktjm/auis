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

/*LIBS:  -lm
*/

/*
 * P_R_P_Q_# (C) COPYRIGHT IBM CORPORATION 1988
 * LICENSED MATERIALS - PROPERTY OF IBM
 * REFER TO COPYRIGHT INSTRUCTIONS FORM NUMBER G120-2083
 */
/* zipprint.c	Zip PrintView-object				      */
/* Author	TC Peters					      */
/* Information Technology Center	   Carnegie-Mellon University */


/**  SPECIFICATION -- External Facility Suite  *********************************

TITLE	The Zip PrintView-object

MODULE	zipprint.c

NOTICE	IBM Internal Use Only

DESCRIPTION
	This is the suite of Methods that support the Zip PrintView-object.

    NB: The comment-symbol "===" indicates areas which are:
	    1 - Questionable
	    OR
	    2 - Arbitrary
	    OR
	    3 - Temporary Hacks
   Such curiosities need be resolved prior to Project Completion...

HISTORY
  03/31/88	Created for ATK (TCP)

END-SPECIFICATION  ************************************************************/

#include <andrewos.h>
#include <stdio.h>
#include <math.h>
#include "graphic.H"
#include "view.H"
#include "im.H"
#include "rect.h"
#include "environ.H"
#include <ctype.h>
#include "fontdesc.H"
#include "zip.H"
#include "zipview.H"
#include "zipobject.H"
#include "zipprint.H"
#include <util.h>

static boolean debug=TRUE;
#define	 Objects(i)		   ((self->view_object->objects)[i])

#define  PelWidth		(Printing->zip_printing_pel_width)
#define  PelHeight		(Printing->zip_printing_pel_height)
#define  PelResolution		(Printing->zip_printing_resolution)
#define  InchWidth		(Printing->zip_printing_inch_width)
#define  InchHeight		(Printing->zip_printing_inch_height)
#define	 UserWidth		Printing->zip_printing_user_width
#define	 UserHeight		Printing->zip_printing_user_height

ATKdefineRegistry(zipprint, ATK, NULL);

static long Print_Figure( register class zipprint	        *self, register zip_type_figure		 figure, register zip_type_pane		 pane );
static long Print_Image( register class zipprint *self, register zip_type_image image, register zip_type_pane pane );
static int Print_Inferior_Image( register class zipprint	        *self, register zip_type_image	         image, register zip_type_pane		 pane );
static void Compute_Printing_Slug_Stretch_Factors( register class zipprint	          *self, register zip_type_pane		   pane );
static void Set_Printing_Characteristics( register class zipprint	          *self, register zip_type_pane		   pane, register zip_type_figure		   figure );

extern int zipprint_Write_Print_Datastream_Header( class zipprint *self );
extern int zipprint_Write_Print_Datastream_Trailer( class zipprint *self );


zipprint::zipprint( )
      {
  IN(zipprint_InitializeObject );
/*===*/
  OUT(zipprint_InitializeObject );
  THROWONFAILURE( TRUE);
  }

zipprint::~zipprint( )
      {
  class zipprint *self=this;
  IN(zipprint_FinalizeObject );
/*===*/
  if ( Printing )  free( Printing );
  OUT(zipprint_FinalizeObject );
  }

void
zipprint::Set_Data_Object( register class zip		       *data_object )
      {
  IN(zipprint_Set_Data_Object );
  this->data_object = data_object;
  OUT(zipprint_Set_Data_Object );
  }

void
zipprint::Set_View_Object( register class zipview	      *view_object )
      {
  class zipprint *self=this;
  IN(zipprint_Set_View_Object);
  this->view_object = view_object;
  if ( Printing =
		(zip_type_printing) calloc( 1, sizeof(struct zip_printing) ) )
    {
    Printing->zip_printing_resolution = zip_printing_resolution_default; 
    Printing->zip_printing_width_percent  = 100;
    Printing->zip_printing_height_percent = 100;
    Printing->zip_printing_x_origin_percent = 50;
    Printing->zip_printing_y_origin_percent = 50;
    Printing->zip_printing_inch_width = zip_printing_page_width_default;
    Printing->zip_printing_inch_height = zip_printing_page_height_default;
    Printing->zip_printing_pel_width =
	zip_printing_resolution_default * zip_printing_page_width_default;
    Printing->zip_printing_pel_height =
	zip_printing_resolution_default * zip_printing_page_height_default;
    }
  OUT(zipprint_Set_View_Object);
  }

void
zipprint::Set_Debug( boolean  state )
      {
  IN(zipprint_Set_Debug);
  debug = state;
  OUT(zipprint_Set_Debug);
  }

long
zipprint::Set_Print_Language( register const char				  *language )
      {
  class zipprint *self=this;
  register char language_code = 0;

  IN(zipprint_Set_Print_Language);
  DEBUGst(Language,language);
  if ( apt_MM_Compare( language, "PostScript" ) == 0 )
    language_code = zip_postscript;
  else
  if ( apt_MM_Compare( language, "Troff" ) == 0 )
    language_code = zip_troff;
  Printing->zip_printing_language = language_code;
  OUT(zipprint_Set_Print_Language);
  return  zip_ok;
  }

long
zipprint::Set_Print_Processor( register const char				  *processor )
      {
  class zipprint *self=this;
  register char				  processor_code = 0;

  IN(zipprint_Set_Print_Processor);
  DEBUGst(Processor,processor);
  if ( apt_MM_Compare( processor, "PostScript" ) == 0 )
    processor_code = zip_postscript;
  else
  if ( apt_MM_Compare( processor, "troff" ) == 0 )
    processor_code = zip_troff;
  Printing->zip_printing_processor = processor_code;
  OUT(zipprint_Set_Print_Processor);
  return zip_ok;
  }

long
zipprint::Set_Print_Level( register long				   level )
      {
  class zipprint *self=this;
  IN(zipprint_Set_Print_Level);
  DEBUGdt(Level,level);
  Printing->zip_printing_level = level;
  OUT(zipprint_Set_Print_Level);
  return zip_ok;
  }

long
zipprint::Set_Print_File( register FILE				  *file )
      {
  class zipprint *self=this;
  IN(zipprint_Set_Print_File);
  Printing->zip_printing_file = file;
  OUT(zipprint_Set_Print_File);
  return zip_ok;
  }

long
zipprint::Set_Print_Resolution( register long				   resolution )
      {
  class zipprint *self=this;
  IN(zipprint_Set_Print_Resolution);
  DEBUGdt(Resolution,resolution);
  Printing->zip_printing_resolution = resolution;
  OUT(zipprint_Set_Print_Resolution);
  return zip_ok;
  }

long
zipprint::Set_Print_Dimensions( register float			   inch_width , register float			   inch_height )
      {
  class zipprint *self=this;
  IN(zipprint_Set_Print_Dimensions);
  DEBUGgt(Inch-Width,inch_width);
  DEBUGgt(Inch-Height,inch_height);
  Printing->zip_printing_inch_width = inch_width;
  Printing->zip_printing_inch_height = inch_height;

  /* UPDATE ZIP OBJECT DEFAULT PRINT SIZE INFO */
  this->data_object->def_inch_width = inch_width;
  this->data_object->def_inch_height = inch_height;

  OUT(zipprint_Set_Print_Dimensions);  
  return zip_ok;
  }

void
zipprint::Set_Default_Print_Size( register class zip  *data )
        {
      class zipprint  *self=this;
      IN(zipprint_Set_Default_Print_Size);

      (self)->Set_Print_Dimensions( data->def_inch_width, data->def_inch_height);
      DEBUGgt(zipprint::Default Inch Width, data->def_inch_width);
      DEBUGgt(zipprint::Default Inch Height, data->def_inch_height);

      OUT(zipprint_Set_Default_Print_Size);
  }


  long
zipprint::Set_Print_Size( float  width , float height)
 {
      IN(zipprint_Set_Print_Size);
      (this)->Set_Print_Dimensions( width, height);
      this->states.user_def_size = 1;
      DEBUGgt(Set Print Inch Width, width);
      DEBUGgt(Set Print Inch Height, height);

      OUT(zipprint_Set_Print_Size);
      return zip_ok;
  }


long
zipprint::Set_Print_Coordinates( register zip_type_percent x_origin , register zip_type_percent y_origin , register zip_type_percent width , register zip_type_percent height )
{
  class zipprint *self=this;
  int					  status = zip_success;

  IN(zipprint_Set_Print_Coordinates);
  DEBUGdt(X-Origin-Pct,x_origin);
  DEBUGdt(Y-Origin-Pct,y_origin);
  DEBUGdt(Width-Pct,width);
  DEBUGdt(Height-Pct,height);
  Printing->zip_printing_x_origin_percent = x_origin;
  Printing->zip_printing_y_origin_percent = y_origin;
  Printing->zip_printing_height_percent = height;
  Printing->zip_printing_width_percent = width;
  OUT(zipprint_Set_Print_Coordinates);
  return status;
  }

long
zipprint::Set_Print_Orientation( register long				   orientation )
      {
  class zipprint *self=this;
  IN(zipprint_Set_Print_Orientation);
  Printing->zip_printing_orientation = orientation;
  OUT(zipprint_Set_Print_Orientation);
  return zip_ok;
  }

long
zipprint::Print_Figure( register zip_type_figure		 figure, register zip_type_pane		 pane )
        {
  register long			        status = zip_ok;

  IN(zipprint_Print_Figure);
  if ( figure )
    {
    Compute_Printing_Slug_Stretch_Factors( this, pane );
    zipprint_Write_Print_Datastream_Header( this );
    ::Print_Figure( this, figure, pane );
    zipprint_Write_Print_Datastream_Trailer( this );
    }
  OUT(zipprint_Print_Figure);
  return  status;
  }

static
long Print_Figure( register class zipprint	        *self, register zip_type_figure		 figure, register zip_type_pane		 pane )
        {
  register long			        status = zip_ok;

  IN(::Print_Figure);
  if ( ! figure->zip_figure_state.zip_figure_state_deleted )
/*===figure->zip_figure_visibility == zip_figure_exposed===*/
    {
    Set_Printing_Characteristics( self, pane, figure );
    status = (Objects(figure->zip_figure_type))->Print_Object(  figure, pane );
    }
  OUT(::Print_Figure);
  return  status;
  }


long
zipprint::Print_Image( register zip_type_image		 image, register zip_type_pane		 pane )
        {
  register long			        status = zip_ok;

  IN(zipprint_Print_Image);
  if ( image )
    {
    Compute_Printing_Slug_Stretch_Factors( this, pane );
    zipprint_Write_Print_Datastream_Header( this );
    ::Print_Image( this, image, pane );
    zipprint_Write_Print_Datastream_Trailer( this );
    }
  OUT(zipprint_Print_Image);
  return  status;
  }

static
long Print_Image( register class zipprint *self, register zip_type_image image, register zip_type_pane pane )
{
  register long			        status = zip_ok;
  register zip_type_figure		figure_ptr;

  IN(Print_Image);
  if ( 1 /*===image->zip_image_visibility == zip_image_exposed===*/ )
    {
    figure_ptr = image->zip_image_figure_anchor;
    while ( figure_ptr  &&  status == zip_success )
      {
      status = ::Print_Figure( self, figure_ptr, pane );
      figure_ptr = figure_ptr->zip_figure_next;
     }
    if ( image->zip_image_inferior  &&  status == zip_success )
      status = Print_Inferior_Image( self, image->zip_image_inferior, pane );
    }
  OUT(Print_Image);
  return  status;
  }

static int
Print_Inferior_Image( register class zipprint	        *self, register zip_type_image	         image, register zip_type_pane		 pane )
        {
  register int			        status = zip_success;
  register zip_type_figure		figure_ptr;

  IN(Print_Inferior_Image);
  if ( 1 /*===image->zip_image_visibility == zip_image_exposed===*/ )
    {
    figure_ptr = image->zip_image_figure_anchor;
    while ( figure_ptr  &&  status == zip_success )
      {
      status = ::Print_Figure( self, figure_ptr, pane );
      figure_ptr = figure_ptr->zip_figure_next;
      }
    if ( image->zip_image_inferior  &&  status == zip_success )
      status = Print_Inferior_Image( self, image->zip_image_inferior, pane );
    if ( image->zip_image_peer  &&  status == zip_success )
      status = Print_Inferior_Image( self, image->zip_image_peer, pane );
    }
  OUT(Print_Inferior_Image);
  return status;
  }

long
zipprint::Print_Stream( register zip_type_stream		 stream, register zip_type_pane		 pane )
        {
  class zipprint *self=this;
  register long	  status = zip_ok;

  IN(zipprint_Print_Stream);
  if ( stream )
    {
    Compute_Printing_Slug_Stretch_Factors( this, pane );
    zipprint_Write_Print_Datastream_Header( this );
    ::Print_Image( this, (this->data_object)->Image_Root(  stream ), pane );
    zipprint_Write_Print_Datastream_Trailer( this );
    }
  OUT(zipprint_Print_Stream);
  return  status;
  }

long
zipprint::Print_Pane( register zip_type_pane		 pane )
      {
  register long			        status = zip_ok;

  IN(zipprint_Print_Pane);
  if ( pane )
    {
    if ( pane->zip_pane_attributes.zip_pane_attribute_stream_source  &&
         pane->zip_pane_source.zip_pane_stream  &&
         pane->zip_pane_source.zip_pane_stream->zip_stream_image_anchor )
      status = (this)->Print_Stream(  pane->zip_pane_source.zip_pane_stream, pane );
    else
    if ( pane->zip_pane_attributes.zip_pane_attribute_image_source  &&
         pane->zip_pane_source.zip_pane_image )
      status = (this)->Print_Image(  pane->zip_pane_source.zip_pane_image, pane );
    else
    if ( pane->zip_pane_attributes.zip_pane_attribute_figure_source  &&
         pane->zip_pane_source.zip_pane_figure )
      status = (this)->Print_Figure(  pane->zip_pane_source.zip_pane_figure, pane );
    else status = zip_failure; /*=== s/b missing  pane-source ===*/
    }
  OUT(zipprint_Print_Pane);
  return  status;
  }

static
void Compute_Printing_Slug_Stretch_Factors( register class zipprint *self, register zip_type_pane pane )
{
  register long				  greatest_x=1, least_x=1, greatest_y=1, least_y=1;

  IN(Compute_Printing_Slug_Stretch_Factors);
  if ( pane->zip_pane_attributes.zip_pane_attribute_stream_source )
    {
    greatest_x = pane->zip_pane_source.zip_pane_stream->zip_stream_greatest_x;
    least_x    = pane->zip_pane_source.zip_pane_stream->zip_stream_least_x;
    greatest_y = pane->zip_pane_source.zip_pane_stream->zip_stream_greatest_y;
    least_y    = pane->zip_pane_source.zip_pane_stream->zip_stream_least_y;
    }
  else
  if ( pane->zip_pane_attributes.zip_pane_attribute_image_source )
    {
    greatest_x = pane->zip_pane_source.zip_pane_image->zip_image_greatest_x;
    least_x    = pane->zip_pane_source.zip_pane_image->zip_image_least_x;
    greatest_y = pane->zip_pane_source.zip_pane_image->zip_image_greatest_y;
    least_y    = pane->zip_pane_source.zip_pane_image->zip_image_least_y;
    }
  PelWidth  = (long) (PelResolution * InchWidth);
  PelHeight = (long) (PelResolution * InchHeight);
  if ( PelWidth  * ((greatest_y - least_y) + 1) >
       PelHeight * ((greatest_x - least_x) + 1) )
    {
    Printing->zip_printing_stretch_multiplier =
	 PelHeight;
    Printing->zip_printing_stretch_divisor    =
	 (greatest_y - least_y) + 1; 
    }
    else
    {
    Printing->zip_printing_stretch_multiplier =
	 PelWidth;
    Printing->zip_printing_stretch_divisor =
	 (greatest_x - least_x) + 1; 
    }
  OUT(Compute_Printing_Slug_Stretch_Factors);
  }

static
void Set_Printing_Characteristics( register class zipprint	          *self, register zip_type_pane		   pane, register zip_type_figure		   figure )
        {
  IN(Set_Printing_Characteristics);
  if ( pane->zip_pane_zoom_level >= 0 )
    Printing->zip_printing_stretch_zoom_multiplier = pane->zip_pane_scale *
	 (Printing->zip_printing_stretch_multiplier
         * (pane->zip_pane_zoom_level - figure->zip_figure_zoom_level + 1 ));
    else
    Printing->zip_printing_stretch_zoom_multiplier = pane->zip_pane_scale *
	 (Printing->zip_printing_stretch_multiplier
        / (((abs(pane->zip_pane_zoom_level - figure->zip_figure_zoom_level)) > 0 ) ?
            (abs(pane->zip_pane_zoom_level - figure->zip_figure_zoom_level)): 2));
  OUT(Set_Printing_Characteristics);
  }
/*=== === ===*/

