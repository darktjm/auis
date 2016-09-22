/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

/* zipv000.c	Zip View-object	-- general			      */
/* Author	TC Peters					      */
/* Information Technology Center	   Carnegie-Mellon University */


/**  SPECIFICATION -- External Facility Suite  *********************************

TITLE	The Zip View-object -- general	

MODULE	zipv000.c

NOTICE	IBM Internal Use Only

DESCRIPTION
	This is the suite of Methods that support general facilities
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
  05/15/89	Improve performance by pre-checking TransferMode (TCP)
		Also avoid unnecessary SetClippingRect usage
   08/30/89	Adjust zipview_Condition to use graphic_SOURCE and ~graphic_SOURCE instread of graphic_BLACK and graphic_WHITE (SCG)
   08/16/90	Add {Ensure,Normalize}_{Fill,Line}_Attributes methods (SCG)
  09/20/90	Optimize usage of FG/BG Color setting (SCG)

END-SPECIFICATION  ************************************************************/

#include <andrewos.h>
#include "view.H"
#include "fontdesc.H"
#include "zip.H"
#include "zipview.H"
#include "zipedit.H"
#include "zipprint.H"
#include "zipobject.H"

#undef Data
#define  Data			(self->zipobject::data_object
#undef View
#define  View			(self)
#define  PaneExceptionHandler	(self->pane_exception_handler)

extern NO_DLL_EXPORT void zipview_Mark_Pane_Exposed( class zipview		  *self, zip_type_pane		   pane );
extern NO_DLL_EXPORT void zipview_Mark_Pane_Hidden( class zipview		  *self, zip_type_pane		   pane );


class graphic *
zipview::Define_Graphic( class fontdesc		  *font2, unsigned char pattern )
        {
  class graphic		 *graphic2 = NULL;
/*===*/
#define max_table_size  50
int			  i;
static struct zipv000table
  {
  class fontdesc		 *font2;
  unsigned char			  pattern;
  class graphic		 *graphic2;
  }				  localtable[max_table_size];
  class zipview *self=this

  IN(zipview::Define_Graphic);
  DEBUGct(Pattern,pattern);
  if ( font2 )
   for ( i = 0; i < max_table_size; i++ )
    {
    if ( font2 == localtable[i].font2  &&  pattern == localtable[i].pattern )
      {
      DEBUGdt(Old Entry,i);
      graphic2 = localtable[i].graphic2;
      break;
      }
    if ( localtable[i].font2 == NULL )
      {
      DEBUGdt(New Entry,i);
      localtable[i].font2 = font2;
      localtable[i].pattern = pattern;
      localtable[i].graphic2 = graphic2 =
	(font2)->CvtCharToGraphic(  (self )->GetDrawable( ), pattern );
      break;
      }
    }
/*===*/
  DEBUGdt(I,i);
  DEBUGxt(Graphic,graphic2);
  OUT(zipview::Define_Graphic);
  return  graphic2;
  }

class fontdesc *
zipview::Contextual_Figure_Font( zip_type_figure		   figure )
      {
  class fontdesc		 *font;

  IN(zipview::Contextual_Figure_Font);
  if ( figure->zip_figure_font )
    font = this->data_object->fonts->zip_fonts_vector
	[figure->zip_figure_font].zip_fonts_table_font;
  else 
  if ( figure->zip_figure_image->zip_image_font )
    font = this->data_object->fonts->zip_fonts_vector
	[figure->zip_figure_image->zip_image_font].zip_fonts_table_font;
  else
  if ( ( font = (this->data_object)->Superior_Image_Font(  figure->zip_figure_image->zip_image_superior ) ) )
    {}
  else
  if ( figure->zip_figure_image->zip_image_stream->zip_stream_font )
    font = this->data_object->fonts->zip_fonts_vector
	[figure->zip_figure_image->zip_image_stream->zip_stream_font].zip_fonts_table_font;
  else
    font = (class fontdesc *)(this->data_object)->Define_Font(  "andy12", NULL );
  DEBUGxt(Font,font);
  OUT(zipview::Contextual_Figure_Font);
  return  font;
  }

class fontdesc *
zipview::Select_Contextual_Figure_Font( zip_type_figure		   figure )
      {
  class fontdesc		 *font;
  class zipview *self=this;

  IN(zipview::Select_Contextual_Figure_Font);
  if ( ( font = (self)->Contextual_Figure_Font(  figure ) ) )
    (self)->SetFont(  font );
  DEBUGxt(Font,font);
  OUT(zipview::Select_Contextual_Figure_Font);
  return  font;
  }

void
zipview::Ensure_Fill_Attributes( zip_type_figure		 figure )
      {
  double				red, green, blue;
  const char				*def_bg, *def_fg;
  class zipview *self=this;

  if ( (self )->DisplayClass( ) & graphic_Color )
  {
      def_fg = def_bg = NULL;
      graphic::GetDefaultColors(&def_fg, &def_bg);
      if ( (this->data_object)->Contextual_Figure_FillFG_Color(  figure, &red, &green, &blue ) == zip_ok ) (this)->SetFGColor(  red, green, blue );
      else
	 (self)->SetForegroundColor( def_fg, 0, 0, 0);
    if ( (this->data_object)->Contextual_Figure_FillBG_Color(  figure, &red, &green, &blue ) == zip_ok ) (this)->SetBGColor(  red, green, blue );
    else
	 (self)->SetBackgroundColor( def_bg, 65535, 65535, 65535);
  }
  }

void
zipview::Normalize_Fill_Attributes( )
    {
  const char				*def_bg, *def_fg;
  class zipview *self=this;

  if ( (self )->DisplayClass( ) & graphic_Color )
    {
      def_fg = def_bg = NULL;
      graphic::GetDefaultColors(&def_fg, &def_bg);
      (self)->SetForegroundColor( def_fg, 0, 0, 0);
      (self)->SetBackgroundColor( def_bg, 65535, 65535, 65535);
    }
  }

long
zipview::Ensure_Line_Attributes( zip_type_figure		 figure )
      {
  unsigned char		lwidth;
  double				red, green, blue;
  long				status = zip_failure;
  char					*pattern = NULL;
  int					offset;
  short					dashtype, value;
  const char				*def_bg, *def_fg;
  class zipview *self=this;

  if (( lwidth = (this->data_object)->Contextual_Figure_Line_Width(  figure )) > 0  ||
       self->mouse_action == view_LeftMovement  )
    {
    status = zip_ok;
    (self)->SetLineWidth(  lwidth );
    if ( (self )->DisplayClass( ) & graphic_Color )
    {
	def_fg = def_bg = NULL;
	graphic::GetDefaultColors(&def_fg, &def_bg);
	if ( (this->data_object)->Contextual_Figure_Line_Color(  figure, &red, &green, &blue ) == zip_ok )
	  (self)->SetFGColor(  red, green, blue );
        else (self)->SetForegroundColor(  def_fg, 0, 0, 0 );
    }
    (this->data_object)->Contextual_Figure_Line_Dash(  figure, &pattern, &offset, &dashtype );
    if ( pattern ) (self)->SetLineDash(  pattern, offset, dashtype );
    else
    {
      (self)->GetLineDash( NULL, NULL, &dashtype );
      if ( dashtype != graphic_LineSolid )
        (self)->SetLineDash(  NULL, 0, graphic_LineSolid );
    }
    if (( value = (this->data_object)->Contextual_Figure_Line_Cap(  figure )) != -1 )
      (self)->SetLineCap(  value );
    else (self)->SetLineCap(  graphic_CapButt );
    if (( value = (this->data_object)->Contextual_Figure_Line_Join(  figure )) != -1 )
      (self)->SetLineJoin(  value );
    else (self)->SetLineJoin(  graphic_JoinMiter );
    }
    return status;
  }

void
zipview::Normalize_Line_Attributes( )
    {
  const char				*def_bg, *def_fg;
  class zipview *self=this;

  IN( zipview::Normalize_Line_Attributes )
  (self)->SetLineWidth(  1 );
  if ( (self )->DisplayClass( ) & graphic_Color ) {
      def_fg = def_bg = NULL;
      graphic::GetDefaultColors(&def_fg, &def_bg);
      (self)->SetForegroundColor(  def_fg, 0, 0, 0);
  }
  (self)->SetLineDash(  NULL, 0, graphic_LineSolid );
  (self)->SetLineCap(  graphic_CapButt );
  (self)->SetLineJoin(  graphic_JoinMiter );
  OUT( zipview::Normalize_Line_Attributes )
  }

boolean
zipview::Condition( zip_type_pane		   pane, zip_type_figure		   figure, int				   action )
          {
  boolean			  condition = true;
  class zipview *self=this;

  IN(zipview::Condition);
  if (( pane && figure && figure->zip_figure_zoom_level <= pane->zip_pane_zoom_level )
       || ( pane && !figure ))
    {
    switch ( action )
      {
      case  zip_draw:
	DEBUG(DRAW);
        if ( pane->zip_pane_state.zip_pane_state_inverted )
	  {
	  if ( (self )->GetTransferMode( ) != ~graphic_SOURCE )
            (self)->SetTransferMode(  ~graphic_SOURCE );
	  }
	else
        if ( pane->zip_pane_state.zip_pane_state_paint_inverted )
	  {
	  if ( (self )->GetTransferMode( ) != graphic_INVERT )
	    (self)->SetTransferMode(  graphic_INVERT );
	  }
	else
        if ( pane->zip_pane_state.zip_pane_state_paint_copy )
	  {
	  if ( (self )->GetTransferMode( ) != graphic_COPY )
	    (self)->SetTransferMode(  graphic_COPY );
	  }
	else
	  {
  	  if ( (self )->GetTransferMode( ) != graphic_SOURCE )
	    (self)->SetTransferMode(  graphic_SOURCE );
	  }
	break;
      case  zip_clear:
	DEBUG(CLEAR);
        if ( ! pane->zip_pane_state.zip_pane_state_inverted )
	  {
	  if ( (self )->GetTransferMode( ) != graphic_WHITE )
            (self)->SetTransferMode(  graphic_WHITE );
	  }
	else
	  {
	  if ( (self )->GetTransferMode( ) != graphic_BLACK )
	    (self)->SetTransferMode(  graphic_BLACK );
	  }
	break;
      }
    }
    else  condition = false;
  OUT(zipview::Condition);
  return condition;
  }

void zipview::Set_Clip_Area( zip_type_pane		   pane, long				   l , long				   t , long				   w , long				   h )
        {
  struct rectangle			  rectangle;
  class zipview *self=this;

  IN(zipview::Set_Clip_Area);
  rectangle.left   = l;
  rectangle.top    = t;
  rectangle.width  = w;
  rectangle.height = h;
  (self)->SetClippingRect(  &rectangle );
  OUT(zipview::Set_Clip_Area);
  }

void zipview::Set_Pane_Clip_Area( zip_type_pane		   pane )
      {
  struct rectangle			  rect, clip;
  class zipview *self=this;

  IN(zipview::Set_Pane_Clip_Area);
  rect.left   = pane->zip_pane_left + pane->zip_pane_border_thickness;
  rect.top    = pane->zip_pane_top  + pane->zip_pane_border_thickness;
  rect.width  = (self)->Pane_Width(  pane ) -
			 (2 * pane->zip_pane_border_thickness);
  rect.height = (self)->Pane_Height( pane ) -
			 (2 * pane->zip_pane_border_thickness);
  (self)->GetClippingRect(  &clip );
  if ( rect.left  != clip.left   ||  rect.top    != clip.top  ||
       rect.width != clip.width  ||  rect.height != clip.height )
    {
    (self)->SetClippingRect(  &rect );
    }
  OUT(zipview::Set_Pane_Clip_Area);
  }

void zipview::Reset_Pane_Clip_Area( zip_type_pane		   pane )
      {
  class zipview *self=this;

  IN(zipview::Reset_Pane_Clip_Area);
  (self )->ClearClippingRect( );
  OUT(zipview::Reset_Pane_Clip_Area);
  }

void zipview_Mark_Pane_Exposed( class zipview		  *self, zip_type_pane		   pane )
      {
  IN(zipview_Mark_Pane_Exposed);
  pane->zip_pane_state.zip_pane_state_exposed = true;
  pane->zip_pane_state.zip_pane_state_hidden = false;
  pane->zip_pane_state.zip_pane_state_removed = false;
  OUT(zipview_Mark_Pane_Exposed);
  }

void zipview_Mark_Pane_Hidden( class zipview		  *self, zip_type_pane		   pane )
      {
  IN(zipview_Mark_Pane_Hidden);
  pane->zip_pane_state.zip_pane_state_exposed = false;
  pane->zip_pane_state.zip_pane_state_hidden = true;
  pane->zip_pane_state.zip_pane_state_removed = false;
  OUT(zipview_Mark_Pane_Hidden);
  }

boolean
zipview::Proximate_Figure_Point( zip_type_pane		   pane, zip_type_figure		   figure, zip_type_pixel		   X , zip_type_pixel		   Y , zip_type_pixel		   x , zip_type_pixel		   y )
          {
  long				  status = false;

  IN(zipview::Proximate_Figure_Point);
  if ( X >= (x - zip_proximal_distance)  &&
       X <= (x + zip_proximal_distance)  &&
       Y >= (y - zip_proximal_distance)  &&
       Y <= (y + zip_proximal_distance)  )
    status = true;
  OUT(zipview::Proximate_Figure_Point);
  return status;
  }

long
zipview::Proximate_Enclosure( zip_type_pane		   pane, zip_type_pixel		   left , zip_type_pixel		   top , zip_type_pixel		   right , zip_type_pixel		   bottom, zip_type_pixel		   x , zip_type_pixel		   y )
          {
  long				  status = false;

  IN(zipview::Proximate_Enclosure);
  if ( x >= left  &&  x <= right  &&
       y >= top   &&  y <= bottom )
    status = true;
  OUT(zipview::Proximate_Enclosure);
  return status;
  }

long
zipview::Try_Pane_Exception_Handler( zip_type_pane		   pane )
      {
  class zipview *self=this;

  IN(zipview::Try_Pane_Exception_Handler);
  if ( PaneExceptionHandler )
    {
    (*PaneExceptionHandler)( self, pane );
    OUT(zipview::Try_Pane_Exception_Handler);
    return true;
    }
    else
    {
    OUT(zipview::Try_Pane_Exception_Handler);
    return (this->data_object )->Try_general_Exception_Handler( );
    }
  }
