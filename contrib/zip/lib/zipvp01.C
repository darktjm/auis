/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

/* zipvp01.c	Zip View-object	-- Pane	Cursors			      */
/* Author	TC Peters					      */
/* Information Technology Center	   Carnegie-Mellon University */



/**  SPECIFICATION -- External Facility Suite  *********************************

TITLE	The Zip View-object -- Pane Cursors	

MODULE	zipvp01.c

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
  05/01/89	Use symbolic font-names (TCP)

END-SPECIFICATION  ************************************************************/

#include <andrewos.h>
#include "view.H"
#include "im.H"
#include "cursor.H"
#include "fontdesc.H"
#include "zip.H"
#include "zipview.H"
#include "zipedit.H"
#include "zipprint.H"
#include "zipobject.H"

#undef Data
#define	 Data			      (self->zipobject::data_object)
#undef View
#define	 View			      (self)


long
zipview::Set_Pane_Cursor( zip_type_pane		   pane, char				   icon, const char				  *font_name )
          {
  int				  status = zip_ok;
  class fontdesc		 *old_font;
  class zipview *self=this;

  IN(zipview_Set_Pane_Cursor);
  if ( pane )
    {
    if ( pane->zip_pane_cursor_glyph == NULL )
      pane->zip_pane_cursor_glyph = cursor::Create( self );
    if ( ! font_name )
      font_name = CursorFontName;
    old_font = pane->zip_pane_cursor_font;
    pane->zip_pane_cursor_font = (class fontdesc *)
	(self->data_object)->Define_Font(  font_name, NULL );
    if ( pane->zip_pane_cursor_font  &&
	  (old_font != pane->zip_pane_cursor_font ||
	   pane->zip_pane_cursor_icon != icon) )
	{
	pane->zip_pane_cursor_icon = icon;
        (pane->zip_pane_cursor_glyph)->SetGlyph( 
			 pane->zip_pane_cursor_font, pane->zip_pane_cursor_icon );

	}
    (self)->Post_Pane_Cursor(  pane, pane->zip_pane_cursor_glyph );
    }
    else  status = zip_pane_non_existent;
  ZIP_STATUS(this->data_object);
  OUT(zipview_Set_Pane_Cursor);
  return status;
  }

long
zipview::Use_Working_Pane_Cursors( )
    {
  static  class cursor			 *glyph = NULL;
  class zipview *self=this;
  IN(zipview::Use_Working_Pane_Cursors);
/*===  zipview_Use_Alternate_Pane_Cursors( self, 'H', "icon12" );===*/
  if ( glyph == NULL )
    {
    glyph = cursor::Create( self );
    (glyph)->SetGlyph(  (self->data_object)->Define_Font(  "icon12", NULL ), 'H' );
    }
  ((self )->GetIM( ))->SetWindowCursor(  glyph );
  OUT(zipview::Use_Working_Pane_Cursors);
  return zip_ok;
  }

long
zipview::Use_Normal_Pane_Cursors( )
    {
  class zipview *self=this;
  zip_type_pane_chain	      pane_chain = PaneAnchor;

  IN(zipview::Use_Normal_Pane_Cursors);
  ((self)->GetIM( ))->SetWindowCursor(  NULL );
  while ( pane_chain )
    {
    if ( pane_chain->zip_pane_chain_ptr->zip_pane_state.zip_pane_state_exposed )
      (self)->Post_Pane_Cursor(  pane_chain->zip_pane_chain_ptr,
			        pane_chain->zip_pane_chain_ptr->zip_pane_cursor_glyph );
    pane_chain = pane_chain->zip_pane_chain_next;
    }
  OUT(zipview::Use_Normal_Pane_Cursors);
  return zip_ok;
  }

long
zipview::Use_Alternate_Pane_Cursors( char			       icon, char			      *font_name )
        {
  class zipview *self=this;
  class fontdesc	     *font;
  zip_type_pane_chain	      pane_chain = PaneAnchor;
  static  class cursor	    	     *glyph = NULL;
  int			      status = zip_ok;

  IN(zipview::Use_Alternate_Pane_Cursors);
  if ( glyph == NULL )
    glyph = cursor::Create( self );
  font = (class fontdesc *)(self->data_object)->Define_Font(  font_name, NULL );
  (glyph)->SetGlyph(  font, icon );
  while ( pane_chain )
    {
    if ( pane_chain->zip_pane_chain_ptr->zip_pane_state.zip_pane_state_exposed )
      (self)->Post_Pane_Cursor(  pane_chain->zip_pane_chain_ptr, glyph );
    pane_chain = pane_chain->zip_pane_chain_next;
    }
  ZIP_STATUS(this->data_object);
  OUT(zipview::Use_Alternate_Pane_Cursors);
  return zip_ok;
  }

void zipview::Post_Pane_Cursor( zip_type_pane	       pane, class cursor	      *glyph )
        {
  struct  rectangle		      rectangle;
  class zipview *self=this;

  IN(zipview::Post_Pane_Cursor);
  if ( pane->zip_pane_state.zip_pane_state_exposed )
    {
    DEBUG(Exposed);
    rectangle.left   = (self)->Pane_Left(  pane );
    rectangle.top    = (self)->Pane_Top(  pane );
    rectangle.width  = (self)->Pane_Width(  pane );
    rectangle.height = (self)->Pane_Height(  pane );
    if ( (glyph )->IsPosted( ) )
      (self)->RetractCursor(  glyph );
    (self)->PostCursor(  &rectangle, glyph );
    }
  OUT(zipview::Post_Pane_Cursor);
  }
