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
/* zipvp03.c	Zip View-object	-- Display/Draw Facilities	      */
/* Author	TC Peters					      */
/* Information Technology Center	   Carnegie-Mellon University */


/**  SPECIFICATION -- External Facility Suite  *********************************

TITLE	The Zip View-object -- Display/Draw Facilities	

MODULE	zipvp03.c

NOTICE	IBM Internal Use Only

DESCRIPTION
	This is the suite of Methods that support the Pane Display/Draw facilities
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
  10/18/88	Add use of Pre/Post/Processors to Draw as well as Display (TCP)
  07/12/89	Invoke Stream_Visible to optimize Draw_Auxiliary_Streams (SCG)
  08/30/89	Use zipview_Condition when drawing Pane Borders (SCG)
   08/16/90	Add Normalize_Line_Attributres in Draw_Pane_Border (SCG)

END-SPECIFICATION  ************************************************************/

#include <andrewos.h>
#include "view.H"
#include "fontdesc.H"
#include "zip.H"
#include "zipview.H"
#include "zipedit.H"
#include "zipprint.H"
#include "zipobject.H"


#define	 Data			      (self->zipobject::data_object)
#define	 View			      (self)
#define	 Edit			      (self->edit_object)

#define  PaneLeft		      ((self)->Pane_Left(  pane ))
#define  PaneTop		      ((self)->Pane_Top(  pane ))
#define  PaneWidth		      ((self)->Pane_Width(  pane ))
#define  PaneHeight		      ((self)->Pane_Height(  pane ))
#define  PaneBottom		      ((self)->Pane_Bottom(  pane ))
#define  PaneRight		      ((self)->Pane_Right(  pane ))


static int Show_Pane( register class zipview		  *self, register zip_type_pane		   pane, register long				   action );
static int Pane_Suite_Member( register class zipview		  *self, register zip_type_pane		   major_pane, register zip_type_pane		   candidate_pane );
static int Draw_Auxiliary_Streams( register class zipview		  *self, register zip_type_pane		   pane );
void zipview_Draw_Pane_Border( register class zipview		  *self, register zip_type_pane		   pane );
int zipview_Preserve_Overlay( register class zipview		  *self, register zip_type_pane		   pane );
int zipview_Restore_Overlay( register class zipview		  *self, register zip_type_pane		   pane );

extern void zipview_Compute_Pane_Stretch_Factors( register class zipview      *self, register zip_type_pane             pane );

long
zipview::Display_Pane( register zip_type_pane		   pane )
      {
  register int				  status = zip_ok;
  class zipview *self=this;

  IN(zipview_Display_Pane);
  if ( pane )
    {
    if ( (self)->Pane_Overlaying(  pane )  &&
         ! (self)->Pane_Exposed(  pane ) )
      zipview_Preserve_Overlay( self, pane );
    if ( ! (self)->Pane_Transparent(  pane ) )
      (self)->Clear_Pane(  pane );
    zipview_Compute_Pane_Stretch_Factors( self, pane );
    status = Show_Pane( self, pane, zip_display_action );
    }
    else  status = zip_pane_non_existent;
  ZIP_STATUS(this->data_object);
  OUT(zipview_Display_Pane);
  return status;
  }

long
zipview::Draw_Pane( register zip_type_pane		   pane )
      {
  register int				  status = zip_ok;
  class zipview *self=this;

  IN(zipview_Draw_Pane);
  if ( pane )
    {
    status = Show_Pane( self, pane, zip_draw_action );
    }
    else 
    status = zip_pane_non_existent;
  ZIP_STATUS(this->data_object);
  OUT(zipview_Draw_Pane);
  return status;
  }

extern void zipview_Mark_Pane_Exposed( register class zipview                 *self, register zip_type_pane             pane );

static int
Show_Pane( register class zipview		  *self, register zip_type_pane		   pane, register long				   action )
        {
  register int				  status = zip_ok;

  IN(Show_Pane);
  zipview_Mark_Pane_Exposed( self, pane );
  (self)->Set_Pane_Clip_Area(  pane );
  if ( pane->zip_pane_display_preprocessor )
    { DEBUG(>>> Display Preprocessor);
    status = (*pane->zip_pane_display_preprocessor)
		( pane->zip_pane_display_processor_anchor, pane, action );
    DEBUG(<<< Display Preprocessor);
    }
  if ( pane->zip_pane_display_processor )
    { DEBUG(>>> Display Processor);
    status = (*pane->zip_pane_display_processor)
		( pane->zip_pane_display_processor_anchor, pane, action );
    DEBUG(<<< Display Processor);
    }
    else
      {
      Draw_Auxiliary_Streams( self, pane );
      if ( pane->zip_pane_attributes.zip_pane_attribute_stream_source  &&
           pane->zip_pane_source.zip_pane_stream  &&
           pane->zip_pane_source.zip_pane_stream->zip_stream_image_anchor )
        status = (self)->Draw_Stream(  pane->zip_pane_source.zip_pane_stream, pane );
      else
      if ( pane->zip_pane_attributes.zip_pane_attribute_image_source  &&
           pane->zip_pane_source.zip_pane_image  &&
           pane->zip_pane_source.zip_pane_image->zip_image_figure_anchor )
        status = (self)->Draw_Image(  pane->zip_pane_source.zip_pane_image, pane );
      else
      if ( pane->zip_pane_attributes.zip_pane_attribute_figure_source  &&
           pane->zip_pane_source.zip_pane_figure )
        status = (self)->Draw_Figure(  pane->zip_pane_source.zip_pane_figure, pane );
      else
        {
        status = (self)->Clear_Pane(  pane ); /*=== missing  pane-source ===*/
        }
      if ( status == zip_ok )
        if ( pane->zip_pane_border_thickness )
          zipview_Draw_Pane_Border( self, pane );
	  else
	  { DEBUG(Border Zero);
	  if ( pane->zip_pane_state.zip_pane_state_coordinates_exposed )
            (Edit)->Draw_Pane_Coordinates(  pane );
            else
            if ( pane->zip_pane_edit  &&  pane->zip_pane_edit->zip_pane_edit_grid_exposed
		 && self->states.editing)
              (Edit)->Draw_Pane_Grid(  pane );
	  }
      if ( status == zip_ok  &&  pane->zip_pane_cursor_glyph )
        (self)->Post_Pane_Cursor(  pane, pane->zip_pane_cursor_glyph );
      (self )->FlushGraphics( );
      }
  if ( pane->zip_pane_display_postprocessor )
    { DEBUG(>>> Display Postprocessor);
    status = (*pane->zip_pane_display_postprocessor)
		( pane->zip_pane_display_processor_anchor, pane, action );
    DEBUG(<<< Display Postprocessor);
    }
  OUT(Show_Pane);
  return  status;
  }


extern long zipview_Recoordinate_Panes( register class zipview                *self );

long
zipview::Redisplay_Panes( )
    {
  class zipview *self=this;
  register int				  status = zip_ok;
  register zip_type_pane_chain		  pane_link = PaneAnchor;

  IN(zipview_Redisplay_Panes);
  status = zipview_Recoordinate_Panes( self );
  while ( pane_link  &&  status == zip_ok )
    {
    if ( ! ((self)->Pane_Hidden(   pane_link->zip_pane_chain_ptr ) ||
	    (self)->Pane_Removed(  pane_link->zip_pane_chain_ptr ) ) )
      {
      (self)->Set_Pane_Exposed(  pane_link->zip_pane_chain_ptr, false );
      status = (self)->Display_Pane(  pane_link->zip_pane_chain_ptr );
      }
    pane_link = pane_link->zip_pane_chain_next;
    }
  (self )->FlushGraphics( );
  ZIP_STATUS(this->data_object);
  OUT(zipview_Redisplay_Panes);
  return status;
  }

long
zipview::Redraw_Panes( )
    {
  class zipview *self=this;
  register int				  status = zip_ok;
  register zip_type_pane_chain		  pane_link =
			    PaneAnchor;

  IN(zipview_Redraw_Panes);
  zipview_Recoordinate_Panes( self );
  while ( pane_link  &&  status == zip_ok )
    {
    if ( ! ((self)->Pane_Hidden(   pane_link->zip_pane_chain_ptr ) ||
	    (self)->Pane_Removed(  pane_link->zip_pane_chain_ptr ) ) )
      {
      (self)->Set_Pane_Exposed(  pane_link->zip_pane_chain_ptr, false );
      status = (self)->Draw_Pane(  pane_link->zip_pane_chain_ptr );
      }
    pane_link = pane_link->zip_pane_chain_next;
    }
  (self )->FlushGraphics( );
  ZIP_STATUS(this->data_object);
  OUT(zipview_Redraw_Panes);
  return status;
  }

long
zipview::Redisplay_Pane_Suite( register zip_type_pane		   pane )
      {
  class zipview *self=this;
  register int				  status = zip_ok;
  register zip_type_pane_chain		  pane_link =
			    PaneAnchor;

  IN(zipview_Redisplay_Pane_Suite);
  zipview_Recoordinate_Panes( self );
  while ( pane_link  &&  status == zip_ok )
    {
    if ( Pane_Suite_Member( self, pane, pane_link->zip_pane_chain_ptr ) )
      {
      (self)->Set_Pane_Exposed(  pane_link->zip_pane_chain_ptr, false );
      status = (self)->Display_Pane(  pane_link->zip_pane_chain_ptr );
      }
    pane_link = pane_link->zip_pane_chain_next;
    }
  (self )->FlushGraphics( );
  ZIP_STATUS(this->data_object);
  OUT(zipview_Redisplay_Pane_Suite);
  return status;
  }

long
zipview::Redraw_Pane_Suite( register zip_type_pane		   pane )
      {
  class zipview *self=this;
  register int				  status = zip_ok;
  register zip_type_pane_chain		  pane_link = PaneAnchor;

  IN(zipview_Redraw_Pane_Suite);
  zipview_Recoordinate_Panes( self );
  while ( pane_link  &&  status == zip_ok )
    {
    if ( Pane_Suite_Member( self, pane, pane_link->zip_pane_chain_ptr ) )
      {
      (self)->Set_Pane_Exposed(  pane_link->zip_pane_chain_ptr, false );
      status = (self)->Draw_Pane(  pane_link->zip_pane_chain_ptr );
      }
    pane_link = pane_link->zip_pane_chain_next;
    }
  (self )->FlushGraphics( );
  ZIP_STATUS(this->data_object);
  OUT(zipview_Redraw_Pane_Suite);
  return status;
  }

static int
Pane_Suite_Member( register class zipview		  *self, register zip_type_pane		   major_pane, register zip_type_pane		   candidate_pane )
        {
  register int				  status = false;
  register zip_type_pane		  superior_pane;

  IN(Pane_Suite_Member);
  if ( candidate_pane == major_pane  &&
       !((self)->Pane_Hidden(   candidate_pane ) ||
	 (self)->Pane_Removed(  candidate_pane ) ) )
    status = true;
    else
    {
    if ( candidate_pane->zip_pane_attributes.zip_pane_attribute_pane_area  &&
	 !((self)->Pane_Hidden(   candidate_pane ) ||
	   (self)->Pane_Removed(  candidate_pane ) ) )
      {
      if ( candidate_pane->zip_pane_area.zip_pane_pane == major_pane )
	status = true;
	{
	superior_pane = candidate_pane->zip_pane_area.zip_pane_pane;
	while ( superior_pane )
	  {
	  if ( superior_pane->zip_pane_area.zip_pane_pane == major_pane )
	    {
	    status = true;
	    break;
	    }
	    else
	    if ( superior_pane->zip_pane_attributes.zip_pane_attribute_pane_area )
	      superior_pane = superior_pane->zip_pane_area.zip_pane_pane;
	      else break;
	  }
	}
      }
    }
  OUT(Pane_Suite_Member);
  return status;
  }

static int
Draw_Auxiliary_Streams( register class zipview		  *self, register zip_type_pane		   pane )
      {
  register int				  status = zip_ok;
  register zip_type_pane_auxiliary_stream ptr;

  IN(Draw_Auxiliary_Streams);
  if ( pane->zip_pane_auxiliary_stream )
    {
    (self)->Set_Pane_Clip_Area(  pane );
    ptr = pane->zip_pane_auxiliary_stream;
    while ( status == zip_ok  &&  ptr )
      {
      if ( ptr->zip_pane_auxiliary_stream_ptr  &&
	   ptr->zip_pane_auxiliary_stream_visibility != zip_pane_hidden  &&
           (self)->Stream_Visible(  ptr->zip_pane_auxiliary_stream_ptr, pane ) == TRUE &&
	  (status =  (self)->Draw_Image(  ptr->zip_pane_auxiliary_stream_ptr->
				    zip_stream_image_anchor, pane )) == zip_ok )
        {
	ptr->zip_pane_auxiliary_stream_visibility = zip_pane_exposed;
	if ( pane->zip_pane_auxiliary_stream->zip_pane_auxiliary_stream_density )
	  (Edit)->Lighten_Pane(  pane,
	     pane->zip_pane_auxiliary_stream->zip_pane_auxiliary_stream_density );
	}
      ptr = ptr->zip_pane_auxiliary_stream_next;
      }
    }
  DEBUGdt(Status,status);
  OUT(Draw_Auxiliary_Streams);
  return status;
  }

void zipview_Draw_Pane_Border( register class zipview		  *self, register zip_type_pane		   pane )
      {
  register class graphic		 *graphic;
  register int				  thickness =
				    pane->zip_pane_border_thickness;
  register short		          font = 
				    pane->zip_pane_border_font;
  register char				  pattern =
				    pane->zip_pane_border_pattern;

  IN(zipview_Draw_Pane_Border);
  if ( thickness > 0  &&
      ! pane->zip_pane_state.zip_pane_state_coordinates_exposed )
    { DEBUGdt(Thickness,thickness);
    (self)->Set_Clip_Area(  pane, PaneLeft, PaneTop,
			   PaneWidth, PaneHeight );
    (self)->Set_Pane_Painting_Mode(  pane, zip_default );
    (self )->Normalize_Line_Attributes( );
    (self)->Condition(  pane, NULL, zip_draw );
    if ( pane->zip_pane_border_pattern )
      {
      graphic = (self)->Define_Graphic( 
	    self->data_object->fonts->zip_fonts_vector[font].zip_fonts_table_font,
	    pattern );
      (self)->FillTrapezoid(  /* Top */
	PaneLeft, PaneTop, PaneWidth,
	PaneLeft, PaneTop + thickness,	PaneWidth, graphic );
      (self)->FillTrapezoid(  /* Bottom */
	PaneLeft, PaneBottom - thickness, PaneWidth,
	PaneLeft, PaneBottom, PaneWidth, graphic );
      (self)->FillTrapezoid(  /* Left */
	PaneLeft, PaneTop + thickness, thickness,
	PaneLeft, PaneBottom - thickness, thickness, graphic );
      (self)->FillTrapezoid(  /* Right */
	PaneRight - thickness, PaneTop + thickness, thickness,
	PaneRight - thickness, PaneBottom - thickness, thickness, graphic );
      }
      else
      {
      (self)->FillRectSize( 
	 PaneLeft, PaneTop,
	 PaneWidth, thickness, (self )->BlackPattern( ) );
      (self)->FillRectSize( 
	 PaneRight - thickness, PaneTop,
	 thickness, PaneHeight, (self )->BlackPattern( ) );
      (self)->FillRectSize( 
	 PaneLeft, PaneBottom - thickness,
	 PaneWidth, thickness, (self )->BlackPattern( ) );
      (self)->FillRectSize( 
	 PaneLeft, PaneTop,
	 thickness, PaneHeight, (self )->BlackPattern( ) );
      }
    if ( pane->zip_pane_edit  &&  pane->zip_pane_edit->zip_pane_edit_grid_exposed
	 && self->states.editing )
      (Edit)->Draw_Pane_Grid(  pane );
/*    zipview_SetTransferMode( self, graphic_BLACK ); */
    (self)->Set_Pane_Clip_Area(  pane );
    }
    else
    { DEBUG(Coordinates/Grid);
    if ( pane->zip_pane_state.zip_pane_state_coordinates_exposed )
      (Edit)->Draw_Pane_Coordinates(  pane );
      else
      if ( pane->zip_pane_edit  &&  pane->zip_pane_edit->zip_pane_edit_grid_exposed
	   && self->states.editing)
        (Edit)->Draw_Pane_Grid(  pane );
    }
  OUT(zipview_Draw_Pane_Border);
  }


extern void zipview_Mark_Pane_Hidden( register class zipview           *self, register zip_type_pane            pane );

long
zipview::Hide_Pane( register zip_type_pane		   pane )
      {
  register int						  status = zip_ok;
  class zipview *self=this;

  IN(zipview_Hide_Pane);
  if ( pane )
    {
    if ( (self)->Pane_Exposed(  pane ) )
      {
      (self)->Set_Pane_Clip_Area(  pane );
/*===      ZIP_WM_DefineRegion( pane->zip_pane_region_id, 0, 0, 0,0 );===*/
/*===      ZIP_WM_ForgetRegion( pane->zip_pane_region_id );===*/
/*===
      if ( pane->zip_pane_saved_region_id )
        {
       ZIP_WM_DefineRegion( pane->zip_pane_saved_region_id, 0, 0, 0,0 );
        ZIP_WM_ForgetRegion( pane->zip_pane_saved_region_id ); 
        }
===*/
/*===      pane->zip_pane_saved_region_id = ZIP_Region_ID( pane );===*/
      (self)->Set_Pane_Clip_Area(  pane );
/*===      ZIP_WM_SaveRegion( pane->zip_pane_saved_region_id, 
		       PaneLeft,  PaneTop,
		       PaneWidth, PaneHeight );===*/
      (self)->Set_Pane_Clip_Area(  pane );
      if ( 0 /*===pane->zip_pane_other_region_id===*/ )
        zipview_Restore_Overlay( self, pane );
        else
        {
        (self)->SetTransferMode(  graphic_WHITE );
        (self)->FillRectSize( 
		    PaneLeft, PaneTop,
		    PaneWidth, PaneHeight,
		    (self )->WhitePattern( ) /*===*/ );
        (self)->SetTransferMode(  graphic_BLACK );
        }
      zipview_Mark_Pane_Hidden( self, pane );
      }
    }
    else  status = zip_pane_non_existent;
  (self )->FlushGraphics( );
  ZIP_STATUS(this->data_object);
  OUT(zipview_Hide_Pane);
  return status;
  }

long
zipview::Expose_Pane( register zip_type_pane		   pane )
      {
  register int				  status = zip_ok;
  class zipview *self=this;

  IN(zipview_Expose_Pane);
  if ( pane )
    {
    if ( (self)->Pane_Hidden(  pane ) )
      {
      (self)->Set_Pane_Clip_Area(  pane );
      if ( (self)->Pane_Overlaying(  pane ) )
        zipview_Preserve_Overlay( self, pane );
      (self)->Set_Pane_Clip_Area(  pane );
      (self)->SetTransferMode(  graphic_COPY );
/*===      ZIP_WM_RestoreRegion( ZIP_Pane_Saved_Region( pane ),
		    PaneLeft(pane), PaneTop(pane) );===*/
      (self)->SetTransferMode(  graphic_BLACK );
      (self)->Set_Pane_Clip_Area(  pane );
      zipview_Mark_Pane_Exposed( self, pane );
      }
    } 
    else  status = zip_pane_non_existent;
  (self )->FlushGraphics( );
  ZIP_STATUS(this->data_object);
  OUT(zipview_Expose_Pane);
  return status;
  }

int
zipview_Preserve_Overlay( register class zipview		  *self, register zip_type_pane		   pane )
{
    IN(zipview_Preserve_Overlay);
    /*===
      if ( pane->zip_pane_other_region_id )
      {
	  ZIP_WM_DefineRegion( ZIP_Pane_Saved_Region( pane ), 0, 0, 0,0 );
	  ZIP_WM_ForgetRegion( pane->zip_pane_other_region_id );
      }
      pane->zip_pane_other_region_id = ZIP_Region_ID( pane );
      zipview_Set_Clip_Area( self, pane,
			     PaneLeft(pane),
			     PaneTop(pane),
			     PaneWidth(pane),
			     PaneHeight(pane) );
      ZIP_WM_SaveRegion( pane->zip_pane_other_region_id, 
			 PaneLeft, PaneTop,
			 PaneWidth, PaneHeight );
      zipview_Set_Pane_Clip_Area( self, pane );
      ===*/
    OUT(zipview_Preserve_Overlay);
    return(0);
}


int
zipview_Restore_Overlay( register class zipview		  *self, register zip_type_pane		   pane )
{
    IN(zipview_Restore_Overlay);
    /*===
      zipview_Set_Clip_Area( self, pane,
			     PaneLeft(pane),  PaneTop(pane),
			     PaneWidth(pane), PaneHeight(pane) );
      zipview_SetTransferMode( self, graphic_COPY );
      ZIP_WM_RestoreRegion( pane->zip_pane_other_region_id,
			    PaneLeft(pane), PaneTop(pane) );
      zipview_SetTransferMode( self, graphic_BLACK );
      zipview_Set_Pane_Clip_Area( self, pane );
      ZIP_WM_DefineRegion( pane->zip_pane_other_region_id, 0, 0, 0,0 );
      ZIP_WM_ForgetRegion( pane->zip_pane_other_region_id );
      pane->zip_pane_other_region_id = NULL;
      ===*/
    OUT(zipview_Restore_Overlay);
    return(0);
}

