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

/* zipocapt.c	Zip Object -- Captions					      */
/* Author	TC Peters						      */
/* Information Technology Center		   Carnegie-Mellon University */

/**  SPECIFICATION -- Internal Facility Suite  *********************************

TITLE	Zip Object -- Captions

MODULE	zipocapt.c

NOTICE	IBM Internal Use Only

DESCRIPTION

.......................

HISTORY
  04/13/88	Created (TC Peters)
  05/01/89	Use symbolic font-names (TCP)
  06/08/89	Use Drawable in fontdesc_StringSize (TCP)
  07/24/89	Drop leading/trailing newline(s) upon build completion (TCP)
   08/14/90	Use Ensure_Line_Attributes on Draw (SCG)

END-SPECIFICATION  ************************************************************/

#include <andrewos.h>
#include <im.H>
#include <fontdesc.H>
#include "zip.H"
#include "zipview.H"
#include "zipedit.H"
#include "zipprint.H"
#include "zipobject.H"
#include "zipocapt.H"

ATKdefineRegistry(zipocapt, zipobject, NULL);

static enum view_MouseAction Accept_Caption_Character( class zipocapt *self, zip_type_pane pane, char c, enum view_MouseAction action, long x, long y, long clicks );
static long Draw( class zipocapt *self, zip_type_figure figure, zip_type_pane pane );
static void Compute_Handle_Positions( class zipocapt *self, zip_type_figure figure, zip_type_pane pane, zip_type_pixel *X1, zip_type_pixel *X2, zip_type_pixel *X3, zip_type_pixel *Y1, zip_type_pixel *Y2, zip_type_pixel *Y3 );


char
zipocapt::Object_Icon( )
    {
  IN(zipocapt_Object_Icon);
  OUT(zipocapt_Object_Icon);
  return  'A';
  }

char
zipocapt::Object_Icon_Cursor( )
    {
  IN(zipocapt_Object_Icon_Cursor);
  OUT(zipocapt_Object_Icon_Cursor);
  return  'D';
  }

char
zipocapt::Object_Datastream_Code( )
    {
  IN(zipocapt_Object_Datastream_Code);
  OUT(zipocapt_Object_Datastream_Code);
  return  'A';
  }

long
zipocapt::Show_Object_Properties( zip_type_pane		   pane, zip_type_figure		   figure )
        {
  (this->view_object)->Announce(  "Draw Caption by Selecting Start-point, then Typing." );
  return  zip_ok;
  }

long
zipocapt::Build_Object( zip_type_pane pane, enum view_MouseAction action, long x, long y, long clicks, zip_type_point X, zip_type_point Y )
            {
  zip_type_figure			  figure;
  long				  status = zip_ok;
  char					  text[4];

  IN(zipocapt_Build_Object);
  switch ( action )
    {
    case view_LeftDown:
      if ( (status =
	(this->data_object)->Create_Figure(  &figure, NULL, zip_caption_figure,
			   pane->zip_pane_current_image, NULL )) == zip_ok )
        {
	pane->zip_pane_current_figure = figure;
        (this)->Set_Object_Point(  figure, 1, X, Y );
	text[0] = '|'; text[1] = 0;
        figure->zip_figure_font = pane->zip_pane_current_font;
        figure->zip_figure_mode = pane->zip_pane_current_mode;
        figure->zip_figure_zoom_level = pane->zip_pane_zoom_level;
	ZIP_Select_Figure_Font( this, figure );
        (this->data_object)->Set_Figure_Text(  figure, text );
        (this->view_object)->Draw_Figure(  figure, pane );
        (this->edit_object)->Set_Keyboard_Processor(  (long) this, (zip_keyboardfptr) Accept_Caption_Character );
        (this->view_object)->Set_Pane_Cursor(  pane, 'D', CursorFontName );
	}
      break;
    case view_LeftUp:
    case view_LeftMovement:
      break;
    }
  OUT(zipocapt_Build_Object);
  return  status;
  }

static enum view_MouseAction
Accept_Caption_Character( class zipocapt		  *self, zip_type_pane		   pane, char				   c, enum view_MouseAction	   action, long				   x , long				   y , long				   clicks )
            {
  zip_type_figure		  figure = pane->zip_pane_current_figure;
  char					  text[4097];/*===*/
  char				 *text_cursor;

  IN(Accept_Caption_Character)
  (self->edit_object)->Normalize_Figure_Points(  figure, pane );
  if ( action == view_LeftUp  ||  action == view_LeftMovement )
    action = view_NoMouseEvent;
    else
    {
    strcpy( text, figure->zip_figure_datum.zip_figure_text );/*===*/
    text_cursor = strlen( text ) + &text[0];
    (self->view_object)->Clear_Figure(  figure, pane );
    switch ( c )
      {
      case 0:        DEBUG(Finished Typing);
	text_cursor--;
        if ( (text_cursor - text) >= 0  &&  *text_cursor == '|' )
	  *text_cursor-- = 0;
	while ( (text_cursor - text) > 0 )
	  if ( *(text_cursor-1) == '\\'  &&  *text_cursor == 'n' )
	    {*(text_cursor-1) = 0; text_cursor -= 2;}
	    else  break;
        if ( *text == 0 )
	  {
	  (self->edit_object)->Delete_Figure(  figure, pane );
	  figure = NULL;
	  }
        (self->edit_object)->Set_Keyboard_Processor(  0, NULL );
        break;
      case '\012':
      case '\015':   DEBUG(NewLine);
	if ( text_cursor > text+1 )
	  {
          *(text_cursor-1) = '\\';
          *text_cursor = 'n';
          *(text_cursor+1) = '|';
          *(text_cursor+2) = 0;
	  }
        break;
      case '\010':
      case '\177':   DEBUG(BackSpace or Delete);
        if ( text_cursor > text+1 )
          { DEBUG(Not at front);
	  if ( (text_cursor - text) > 2  &&
	       *(text_cursor-3) == '\\'  &&  *(text_cursor-2) == 'n' )
	    text_cursor--;
          *(text_cursor-2) = '|';
          *(text_cursor-1) = 0;
          }
        break;
      default:      DEBUG(Regular);
        *(text_cursor-1) = c;
        *text_cursor = '|';
        *(text_cursor+1) = 0;
      }
    if ( *text )
      {
      (self->data_object)->Set_Figure_Text(  figure, text );
      (self->view_object)->Draw_Figure(  figure, pane );
      (self->edit_object)->Highlight_Figure_Points(  figure, pane );
      }
    }
   OUT(Accept_Caption_Character);
  return  action;
  }

long
zipocapt::Draw_Object( zip_type_figure		   figure, zip_type_pane		   pane )
        {
  long				  status = zip_ok;

  IN(zipocapt_Draw_Object);
  if ( (this->view_object)->Condition(  pane, figure, zip_draw ) )
    status = Draw( this, figure, pane );
  OUT(zipocapt_Draw_Object);
  return  status;
  }

long
zipocapt::Clear_Object( zip_type_figure		   figure, zip_type_pane		   pane )
        {
  long				  status = zip_ok;

  IN(zipocapt_Clear_Object);
  if ( (this->view_object)->Condition(  pane, figure, zip_clear ) )
    status = Draw( this, figure, pane );
  OUT(zipocapt_Clear_Object);
  return  status;
  }

static
long Draw( class zipocapt		  *self, zip_type_figure		   figure, zip_type_pane		   pane )
        {
  long			  status = zip_ok;
  class fontdesc	 *font, *current_font = (self->view_object )->GetFont( );
  char			 *text = NULL;
  long			  mode = 0, x, y, y_increment, 
			  left, left_offset = 0, top_offset, width;
  long			  xp, yp;
  long			  transfer_mode;
  char			  buffer[4097];
  char			 *cursor, *buffer_ptr;

  IN(Draw);
  font = (self->view_object)->Select_Contextual_Figure_Font(  figure );
  if ( figure->zip_figure_datum.zip_figure_text )
    text = figure->zip_figure_datum.zip_figure_text;
  else
  if ( figure->zip_figure_image->zip_image_text )
    text = figure->zip_figure_image->zip_image_text;
  else
  if ( text = (self->data_object)->Superior_Image_Text(  figure->zip_figure_image->zip_image_superior ) )
    {}
  else
  if ( figure->zip_figure_image->zip_image_stream->zip_stream_text )
    text = figure->zip_figure_image->zip_image_stream->zip_stream_text;

  x = window_x_point;  y = window_y_point;
  y_increment = (font )->GetFontSize( ) + 5/*===fudge.s/b FontNewLine===*/;
  left = 2;  top_offset = -y_increment/2;
  if ( figure->zip_figure_mode.zip_figure_mode_top )
    { mode |= graphic_ATTOP; top_offset = 0; }
  else
  if ( figure->zip_figure_mode.zip_figure_mode_middle )
    { mode |= graphic_BETWEENTOPANDBOTTOM; top_offset = -y_increment/2; }
  else
  if ( figure->zip_figure_mode.zip_figure_mode_baseline )
    { mode |= graphic_BETWEENTOPANDBASELINE; top_offset = -y_increment/2; }
  else
  if ( figure->zip_figure_mode.zip_figure_mode_bottom )
    { mode |= graphic_ATBOTTOM; top_offset = -y_increment; }

  if ( figure->zip_figure_mode.zip_figure_mode_left )
    { mode |= graphic_ATLEFT; left = 1; }
  if ( figure->zip_figure_mode.zip_figure_mode_center )
    { mode |= graphic_BETWEENLEFTANDRIGHT; left = 2; }
  if ( figure->zip_figure_mode.zip_figure_mode_right )
    { mode |= graphic_ATRIGHT; left = 3; }

  if ( text )
    {
    cursor = text;
    (self->view_object)->Ensure_Line_Attributes(  figure );
    while ( *cursor )
      {
      buffer_ptr = buffer;
      while ( *cursor  &&  !(*cursor == '\\'  &&  *(cursor+1) == 'n') )
        *buffer_ptr++ = *cursor++;
      *buffer_ptr = 0;
      if ( figure->zip_figure_mode.zip_figure_mode_halo  &&
	    (transfer_mode = (self->view_object )->GetTransferMode( )) == graphic_BLACK )
	{
        width = (font)->StringSize(  (self->view_object)->GetDrawable(), buffer, &xp, &yp );
	DEBUGdt(Width,width);
	switch ( left )
	  {
	  case 1: left_offset = 0;	    break;
	  case 2: left_offset = -width/2;   break;
	  case 3: left_offset = -width;	    break;
	  }
        (self->view_object)->SetTransferMode(  graphic_WHITE );
        (self->view_object)->EraseRectSize(  x + left_offset, y + top_offset,
			       width + 2, y_increment );
        (self->view_object)->SetTransferMode(  transfer_mode );
	}
      (self->view_object)->MoveTo(  x, y );
      DEBUGdt(X,x);  DEBUGdt(Y,y);
      (self->view_object)->DrawString(  buffer, mode );
      DEBUGxt(Mode,mode);
      if ( *cursor == '\\' )
	{
	cursor++;
	if ( *cursor == 'n' )
	  {
	  cursor++;
	  y += y_increment;
	  }
	}
      }
    }
  if ( ExposePoints )
    (self)->Expose_Object_Points(  figure, pane );
  if ( HighlightPoints )
    (self)->Highlight_Object_Points(  figure, pane );
  (self->view_object)->SetFont(  current_font );
  OUT(Draw);
  return  status;
  }

long
zipocapt::Print_Object( zip_type_figure figure, zip_type_pane	 pane )
{
  class zipocapt *self=this;
  char			*text = NULL;
  class fontdesc	*font = NULL;
  int			 y, y_increment;
  char			 buffer[4097];
  char			*cursor, *buffer_ptr;
  long			 status = zip_ok;
  long			 mode = 0;

  IN(zipocapt_Print_Object);
  font = (this->view_object)->Contextual_Figure_Font(  figure );
  if ( figure->zip_figure_datum.zip_figure_text )
    text = figure->zip_figure_datum.zip_figure_text;
  else
  if ( figure->zip_figure_image->zip_image_text )
    text = figure->zip_figure_image->zip_image_text;
  else
  if ( text = (this->data_object)->Superior_Image_Text(  figure->zip_figure_image->zip_image_superior ) )
    {}
  else
  if ( figure->zip_figure_image->zip_image_stream->zip_stream_text )
    text = figure->zip_figure_image->zip_image_stream->zip_stream_text;

  if ( text )
    {
    mode =  ((figure->zip_figure_mode.zip_figure_mode_top)      ? zip_top      : 0) |
	    ((figure->zip_figure_mode.zip_figure_mode_middle)   ? zip_middle   : 0) |
	    ((figure->zip_figure_mode.zip_figure_mode_baseline) ? zip_baseline : 0) |
	    ((figure->zip_figure_mode.zip_figure_mode_bottom)   ? zip_bottom   : 0) |
	    ((figure->zip_figure_mode.zip_figure_mode_left)     ? zip_left     : 0) |
	    ((figure->zip_figure_mode.zip_figure_mode_center)   ? zip_center   : 0) |
	    ((figure->zip_figure_mode.zip_figure_mode_right)    ? zip_right    : 0);
    (this->print_object)->Change_Font(  font );
    cursor = text;
    y = print_y_point;
    y_increment = 100 * (font)->GetFontSize( );
    DEBUGdt(Y-Increment,y_increment);
    while ( *cursor )
      {
      buffer_ptr = buffer;
      while ( *cursor  &&  !(*cursor == '\\'  &&  *(cursor+1) == 'n') )
        *buffer_ptr++ = *cursor++;
      *buffer_ptr = 0;
      (this->print_object)->Draw_String(  print_x_point, y, buffer, mode );
      if ( *cursor == '\\' )
	{
	cursor++;
	if ( *cursor == 'n' )
	  {
	  cursor++;
	  y += y_increment;
	  }
	}
      }
    (this->print_object )->Restore_Font( ); 
    }
  OUT(zipocapt_Print_Object);
  return  status;
  }

long
zipocapt::Proximate_Object_Points( zip_type_figure		   figure, zip_type_pane		   pane, zip_type_pixel		   x , zip_type_pixel		   y )
          {
  int				  point = 0;
  zip_type_pixel			  X1, X2, X3, Y1, Y2, Y3;

  IN(zipocapt_Proximate_Object_Points);
  Compute_Handle_Positions( this, figure, pane, &X1, &X2, &X3, &Y1, &Y2, &Y3 );
  if ( (this->view_object)->Proximate_Figure_Point(  pane, figure, X1, Y1, x, y ) )
    point = 9;	    /* Upper Left Corner */
  else
  if ( (this->view_object)->Proximate_Figure_Point(  pane, figure, X3, Y3, x, y ) )
    point = 5;	    /* Lower Right Corner */
  else
  if ( (this->view_object)->Proximate_Figure_Point(  pane, figure, X3, Y1, x, y ) )
    point = 3; /* Upper Right Corner */
  else
  if ( (this->view_object)->Proximate_Figure_Point(  pane, figure, X1, Y3, x, y ) )
    point = 7; /* Lower Left Corner */
  else
  if ( (this->view_object)->Proximate_Figure_Point(  pane, figure, X2, Y1, x, y ) )
    point = 2; /* 12 O'Clock */
  else
  if ( (this->view_object)->Proximate_Figure_Point(  pane, figure, X3, Y2, x, y ) )
    point = 4; /* 3 O'Clock */
  else
  if ( (this->view_object)->Proximate_Figure_Point(  pane, figure, X2, Y3, x, y ) )
    point = 6; /* 6 O'Clock */
  else
  if ( (this->view_object)->Proximate_Figure_Point(  pane, figure, X1, Y2, x, y ) )
    point = 8; /* 9 O'Clock */
  else
  if ( (this->view_object)->Proximate_Figure_Point(  pane, figure, X2, Y2, x, y ) )
    point = 1; /* Center */
  else
  if ( (this->view_object)->Proximate_Enclosure(  pane, X1, Y1, X3, Y3, x, y ) )
    point = 1; /* Center */
  OUT(zipocapt_Proximate_Object_Points);
  return  point;
  }

boolean
zipocapt::Enclosed_Object( zip_type_figure		   figure, zip_type_pane		   pane, zip_type_pixel		   x , zip_type_pixel		   y , zip_type_pixel		   w , zip_type_pixel		   h )
          {
  boolean			  enclosed = false;
  zip_type_pixel			  X1, X2, X3, Y1, Y2, Y3;

  IN(zipocapt_Enclosed_Object);
  Compute_Handle_Positions( this, figure, pane, &X1, &X2, &X3, &Y1, &Y2, &Y3 );
  if ( X1 > x  &&  Y1 > y  &&  X2 < (x + w)  &&  Y2 < (y + h) )
    enclosed = true;
  OUT(zipocapt_Enclosed_Object);
  return  enclosed;
  }

long
zipocapt::Object_Enclosure( zip_type_figure		   figure, zip_type_pane		   pane, zip_type_pixel		  *x , zip_type_pixel		  *y , zip_type_pixel		  *w , zip_type_pixel		  *h )
          {
  zip_type_pixel			  X1, X2, X3, Y1, Y2, Y3;

  IN(zipocapt_Object_Enclosure);
  Compute_Handle_Positions( this, figure, pane, &X1, &X2, &X3, &Y1, &Y2, &Y3 );
  *x = X1;  *y = Y1;  *w = abs(X3 - X1);  *h = abs(Y3 - Y1);
  OUT(zipocapt_Object_Enclosure);
  return  zip_ok;
  }

long
zipocapt::Highlight_Object_Points( zip_type_figure		   figure, zip_type_pane		   pane )
        {
  zip_type_pixel			  X1, X2, X3, Y1, Y2, Y3;
  long				  status = zip_ok;

  IN(zipocapt_Highlight_Object_Points);
  Compute_Handle_Positions( this, figure, pane, &X1, &X2, &X3, &Y1, &Y2, &Y3 );
  (this->edit_object)->Highlight_Handles(  pane, X1, X2, X3, Y1, Y2, Y3 );
  OUT(zipocapt_Highlight_Object_Points);
  return  status;
  }

long
zipocapt::Normalize_Object_Points( zip_type_figure		   figure, zip_type_pane		   pane )
        {
  zip_type_pixel			  X1, X2, X3, Y1, Y2, Y3;
  long				  status = zip_ok;

  IN(zipocapt_Normalize_Object_Points);
  Compute_Handle_Positions( this, figure, pane, &X1, &X2, &X3, &Y1, &Y2, &Y3 );
  (this->edit_object)->Normalize_Handles(  pane, X1, X2, X3, Y1, Y2, Y3 );
  OUT(zipocapt_Normalize_Object_Points);
  return  status;
  }

long
zipocapt::Expose_Object_Points( zip_type_figure		   figure, zip_type_pane		   pane )
        {
  long				  status = zip_ok;

  IN(zipocapt_Expose_Object_Points);
  (this->edit_object)->Expose_Point(  pane, figure, figure_x_point, figure_y_point );
  OUT(zipocapt_Expose_Object_Points);
  return  status;
  }

long
zipocapt::Hide_Object_Points( zip_type_figure		   figure, zip_type_pane		   pane )
        {
  long				  status = zip_ok;

  IN(zipocapt_Hide_Points);
  (this->edit_object)->Hide_Point(  pane, figure, figure_x_point, figure_y_point );
  OUT(zipocapt_Hide_Points);
  return  status;
  }

long
zipocapt::Set_Object_Point( zip_type_figure figure, int point, zip_type_point x, zip_type_point y )
          {
  long				  status = zip_ok;

  IN(zipocapt_Set_Object_Point);
  switch ( point )
      {
      case 1: /* Center */
        figure_x_point = x; figure_y_point = y;
        break;
      case 2: /* 12 O'Clock */
        figure_x_point = x; figure_y_point = y;
	break;
      case 3: /* Upper Right Corner */
        figure_x_point = x; figure_y_point = y;
        break;
      case 4: /* 3 O'Clock */
        figure_x_point = x; figure_y_point = y;
	break;
      case 5: /* Lower Right Corner */
        figure_x_point = x; figure_y_point = y;
	break;
      case 6: /* 6 O'Clock */
        figure_x_point = x; figure_y_point = y;
	break;
      case 7: /* Lower Left Corner */
        figure_x_point = x; figure_y_point = y;
	break;
      case 8: /* 9 O'Clock */
        figure_x_point = x; figure_y_point = y;
	break;
      case 9: /* Upper Left Corner */
        figure_x_point = x; figure_y_point = y;
	break;
      default: status = zip_failure; /*=== zip_invalid_point_type ===*/
      }
    if ( status == zip_ok )
      {
      (this->data_object)->Set_Image_Extrema(  figure->zip_figure_image, x, y );
      (this->data_object)->Set_Stream_Extrema(  figure->zip_figure_image->zip_image_stream,
			      figure->zip_figure_image );
      }
  OUT(zipocapt_Set_Object_Point);
  return  status;
  }

long
zipocapt::Adjust_Object_Point_Suite( zip_type_figure		   figure, zip_type_point		   x_delta , zip_type_point		   y_delta )
        {
  long				  status = zip_ok;

  IN(zipocapt_Adjust_Object_Point_Suite);
  figure_x_point += x_delta;
  figure_y_point += y_delta;
  (this->data_object)->Set_Image_Extrema(  figure->zip_figure_image, figure_x_point, figure_y_point );
  (this->data_object)->Set_Stream_Extrema(  figure->zip_figure_image->zip_image_stream,
			    figure->zip_figure_image );
  OUT(zipocapt_Adjust_Object_Point_Suite);
  return  status;
  }

static
void Compute_Handle_Positions( class zipocapt		  *self, zip_type_figure		   figure, zip_type_pane		   pane, zip_type_pixel		  *X1 , zip_type_pixel		  *X2 , zip_type_pixel		  *X3, zip_type_pixel		  *Y1 , zip_type_pixel		  *Y2 , zip_type_pixel		  *Y3 )
          {
  long					  width = 0, height = 0,
					  font_height, w, h;
  class fontdesc		 *font;
  zip_type_pixel		  x, y;
  long				  line_count = 0;
  char					  string[4096];
  char				 *text = figure->zip_figure_datum.zip_figure_text,
					 *line = NULL, *line_max = string + sizeof string;

  x = (self->view_object)->X_Point_To_Pixel(  pane, figure, figure_x_point );
  y = (self->view_object)->Y_Point_To_Pixel(  pane, figure, figure_y_point );
  font_height = (font =
	 (self->view_object)->Select_Contextual_Figure_Font(  figure ) )->GetFontSize( ) + 5
					 /*===fudge. CREATE FontNewLine===*/;

  while ( text  &&  *text  &&  line != line_max )
    {
    line = string;
    while ( *text  &&  line != line_max )
      {
      if ( *text == '\\' )
	{
	if ( *(text+1) == '\\' )  text++;
	if ( *(text+1) == 'n' )
    	  {text++; break;}
	  else *line++ = *text++;
	}
	else *line++ = *text++;
      }
    *line = 0;
    (font)->StringSize(  (self->view_object)->GetDrawable(), string, &w, &h );
    if ( w > width )  width = w;
    line_count++;
    }
  height = line_count * font_height;
  /*===*/  width += 5; /* Fudge */

  if (  figure->zip_figure_mode.zip_figure_mode_left )
    *X1 = x;
    else
    if (  figure->zip_figure_mode.zip_figure_mode_center )
    *X1 = x - width/2;
    else
    if (  figure->zip_figure_mode.zip_figure_mode_right )
    *X1 = x - width;

  if (  figure->zip_figure_mode.zip_figure_mode_top )
    *Y1 = y;
    else
    if (  figure->zip_figure_mode.zip_figure_mode_middle )
    *Y1 = y - font_height/2;
    else
    if (  figure->zip_figure_mode.zip_figure_mode_baseline )
    *Y1 = y - font_height/2; /*===*/
    else
    if (  figure->zip_figure_mode.zip_figure_mode_bottom )
    *Y1 = y - font_height; /*===*/
    else
    *Y1 = y - font_height/2; /*===*/

  *X2 = *X1 + width/2;
  *X3 = *X1 + width;
  *Y2 = *Y1 + height/2;
  *Y3 = *Y1 + height;
  }



long
zipocapt::Set_Object_Font( zip_type_figure		   figure, short			   font )
        {
  IN(zipocapt_Set_Object_Font);
  figure->zip_figure_font = font;
  OUT(zipocapt_Set_Object_Font);
  return  zip_ok;
  }
