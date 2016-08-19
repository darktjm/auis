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
static char rcsid[]="$Header: /afs/cs.cmu.edu/project/atk-src-C++/contrib/zip/lib/RCS/zipofcapt.C,v 1.5 1993/09/30 19:51:01 rr2b Stab74 $";
#endif

/* zipofcap.c	Zip Object -- Flexible Captions				      */
/* Author	TC Peters						      */
/* Information Technology Center		   Carnegie-Mellon University */

/**  SPECIFICATION -- Internal Facility Suite  *********************************

TITLE	Zip Object -- Flexible Captions

MODULE	zipofcap.c

NOTICE	IBM Internal Use Only

DESCRIPTION

.......................

HISTORY
  06/23/88	Created (TC Peters)
  05/01/89	Use symbolic font-names (TCP)

END-SPECIFICATION  ************************************************************/

#include <andrewos.h>
#include "fontdesc.H"
#include "zip.H"
#include "zipview.H"
#include "zipedit.H"
#include "zipprint.H"
#include "zipobject.H"
#include "zipofcapt.H"

ATKdefineRegistry(zipofcapt, zipocapt, NULL);

static enum view_MouseAction Accept_Caption_Character( long anchor, zip_type_pane pane, char c, enum view_MouseAction action, long x, long y, long clicks );
static long Draw( class zipofcapt *self, zip_type_figure figure, zip_type_pane pane );
static void Compute_Handle_Positions( class zipofcapt *self, zip_type_figure figure, zip_type_pane pane, zip_type_pixel *X1, zip_type_pixel *X2 , zip_type_pixel *X3, zip_type_pixel *Y1 , zip_type_pixel *Y2 , zip_type_pixel *Y3 );

char
zipofcapt::Object_Icon( )
    {
  IN(zipofcapt::Object_Icon);
  OUT(zipofcapt::Object_Icon);
/*===  return  'B';===*/
  return  0;
  }

char
zipofcapt::Object_Icon_Cursor( )
    {
  IN(zipofcapt::Object_Icon_Cursor);
  OUT(zipofcapt::Object_Icon_Cursor);
  return  'B';
  }

char
zipofcapt::Object_Datastream_Code( )
    {
  IN(zipofcapt::Object_Datastream_Code);
  OUT(zipofcapt::Object_Datastream_Code);
  return  'B';
  }

long
zipofcapt::Build_Object( zip_type_pane pane, enum view_MouseAction action, long x , long y , long clicks, zip_type_point X , zip_type_point Y )
{
  zip_type_figure			figure;
  long					status = zip_ok;
  char					text[4];
/*===debug=1;*/
  IN(zipofcapt::Build_Object);
  switch ( action )
    {
    case view_LeftDown:
      (this->view_object)->Set_Pane_Cursor(  pane, 'C', CursorFontName );
      if ( (status =
        (this->data_object)->Create_Figure(  &CurrentFigure, NULL, zip_flexible_caption_figure,
			   CurrentImage, NULL )) == zip_ok )
	{
        (this)->Set_Object_Point(  CurrentFigure, 9, X, Y );
        (this)->Set_Object_Point(  CurrentFigure, 5, X, Y );
	text[0] = '|'; text[1] = '\0';
        CurrentFigure->zip_figure_font = pane->zip_pane_current_font;
        CurrentFigure->zip_figure_mode = pane->zip_pane_current_mode;
	ZIP_Select_Figure_Font( this, CurrentFigure );
        (this->data_object)->Set_Figure_Text(  CurrentFigure, text );
	CurrentFigure->zip_figure_zoom_level = pane->zip_pane_zoom_level;
        pane->zip_pane_edit->zip_pane_edit_last_point_id = 5;
	(this->data_object)->Set_Figure_Shade(  CurrentFigure,
			      pane->zip_pane_edit->zip_pane_edit_current_shade );
	}
      break;
    case view_LeftUp:
      if ( figure = CurrentFigure )
	{
	if ( figure_x_point == figure_x_points(0)  &&
	     figure_y_point == figure_y_points(0) )
	  {
	  (this->edit_object)->Delete_Figure(  figure, pane );
          break;
	  }
	  else  (this->edit_object)->Set_Keyboard_Processor(  (long) this, (zip_keyboardfptr) Accept_Caption_Character );
	}
      /* Fall-thru */
    case view_LeftMovement:
      if ( CurrentFigure )
	{
        (this->edit_object)->Normalize_Figure_Points(  CurrentFigure, pane );
	(this->view_object)->Clear_Figure(  CurrentFigure, pane );
        (this)->Set_Object_Point(  CurrentFigure, 5, X, Y );
	(this->view_object)->Draw_Figure(  CurrentFigure, pane );
	if ( action != view_LeftUp )
	  (this->edit_object)->Highlight_Figure_Points(  CurrentFigure, pane );
	}
      break;
    }
  OUT(zipofcapt::Build_Object);
  return  status;
  }

static enum view_MouseAction
Accept_Caption_Character( long anchor, zip_type_pane pane, char c, enum view_MouseAction action, long	 x, long y, long clicks )
{
  class zipofcapt		  *self = (class zipofcapt *) anchor;
  zip_type_figure		  figure = pane->zip_pane_current_figure;
  char					  text[4097];/*===*/
  char				 *text_cursor;

  IN(Accept_Caption_Character)
  if ( action == view_LeftUp  ||  action == view_LeftMovement )
    action = view_NoMouseEvent;
    else if ( CurrentFigure )
      { DEBUG(Current Figure Exists);
      strcpy( text, figure->zip_figure_datum.zip_figure_text );/*===*/
      text_cursor = strlen( text ) + &text[0];
      (self->view_object)->Clear_Figure(  figure, pane );
      switch ( c )
        {
	case 0:	    DEBUG(Finished Typing);
          if ( *text  &&  *(text_cursor-1) == '|' )
	    *(text_cursor-1) = 0;
          if ( *text == 0 )
	    (self->edit_object)->Delete_Figure(  figure, pane );
          (self->edit_object)->Set_Keyboard_Processor(  0, NULL );
          break;
        case '\012':
	case '\015':	DEBUG(NewLine);
          *(text_cursor-1) = '\\';
          *text_cursor = 'n';
          *(text_cursor+1) = '|';
          *(text_cursor+2) = 0;
          break;
        case '\010':
	case '\177':	DEBUG(BackSpace|Delete);
          if ( text_cursor > text+1 )
            { DEBUG(Not at front);
	    if ( (text_cursor - text) > 2  &&
	       *(text_cursor-3) == '\\'  &&  *(text_cursor-2) == 'n' )
	      text_cursor--;
            *(text_cursor-2) = '|';
            *(text_cursor-1) = 0;
            }
          break;
        default:   DEBUG(Regular);
          *(text_cursor-1) = c;
          *text_cursor = '|';
          *(text_cursor+1) = 0;
      }
    if ( *text )
      {
      (self->data_object)->Set_Figure_Text(  figure, text );
      (self->view_object)->Draw_Figure(  figure, pane );
      }
    }
    else
    {
    (self->edit_object)->Set_Keyboard_Processor(  0, NULL );
    }
  OUT(Accept_Caption_Character);
  return  action;
  }

long
zipofcapt::Draw_Object( zip_type_figure		   figure, zip_type_pane		   pane )
        {
  long				  status = zip_ok;

  IN(zipofcapt::Draw_Object);
  if ( (this->view_object)->Condition(  pane, figure, zip_draw ) )
    status = Draw( this, figure, pane );
  OUT(zipofcapt::Draw_Object);
  return  status;
  }

long
zipofcapt::Clear_Object( zip_type_figure		   figure, zip_type_pane		   pane )
        {
  long				  status = zip_ok;

  IN(zipofcapt::Clear_Object);
  if ( (this->view_object)->Condition(  pane, figure, zip_clear ) )
    status = Draw( this, figure, pane );
  OUT(zipofcapt::Clear_Object);
  return  status;
  }

static
long Draw( class zipofcapt		  *self, zip_type_figure		   figure, zip_type_pane		   pane )
        {
  long				  status = zip_ok;
  class fontdesc		 *font, *current_font =
					    (self->view_object )->GetFont( );
  char					 *text = NULL;
  long				  mode = 0,
					  width, wide, widest = 0,
					  height, tallest = 0,
					  x, y, y_increment, lines = 1;
  char					  buffer[4097], widest_buffer[4097];
  char				 *cursor, *buffer_ptr, *font_family;
  long				  font_style, font_size, prior_font_size,
					  adjusted = false,
					  too_small = 0, too_big = 0,
					  too_wide = 0, too_tall = 0,
					  too_narrow = 0, too_short = 0;
  zip_type_pixel			  X1, X2, X3, Y1, Y2, Y3, xp, yp;
/*===debug=1;*/
  IN(Draw);
  Compute_Handle_Positions( self, figure, pane, &X1, &X2, &X3, &Y1, &Y2, &Y3 );
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
  if ( text )
    {
    mode |= (figure->zip_figure_mode.zip_figure_mode_top)      ? graphic_ATTOP      : 0;
    mode |= (figure->zip_figure_mode.zip_figure_mode_middle)   ? graphic_BETWEENTOPANDBOTTOM   : 0;
    mode |= (figure->zip_figure_mode.zip_figure_mode_baseline) ? graphic_BETWEENTOPANDBASELINE : 0;
    mode |= (figure->zip_figure_mode.zip_figure_mode_bottom)   ? graphic_ATBOTTOM   : 0;
    mode |= (figure->zip_figure_mode.zip_figure_mode_left)     ? graphic_ATLEFT     : 0;
    mode |= (figure->zip_figure_mode.zip_figure_mode_center)   ? graphic_BETWEENLEFTANDRIGHT   : 0;
    mode |= (figure->zip_figure_mode.zip_figure_mode_right)    ? graphic_ATRIGHT    : 0;
    cursor = text;
    while ( *cursor )
      {
      if ( *cursor++ == '\\' )
	if ( *cursor++ == 'n' ) lines++;
      }
    DEBUGdt(Lines,lines);
    font = (self->view_object)->Select_Contextual_Figure_Font(  figure );
    font_family = (font )->GetFontFamily( );
    font_style = (font )->GetFontStyle( );
    width = abs(window_x_points(0) - window_x_point);    DEBUGdt(Width,width);
    height = abs(window_y_points(0) - window_y_point);   DEBUGdt(Height,height);
    cursor = text;
    while ( *cursor )
      {
      buffer_ptr = buffer;
      while ( *cursor  &&  !(*cursor == '\\'  &&  *(cursor+1) == 'n') )
        *buffer_ptr++ = *cursor++;
      if ( *cursor == '\\' )
	{
	cursor++;
	if ( *cursor == 'n' )
	  cursor++;
	}
      *buffer_ptr = 0;
      if ( (wide = (font)->StringSize(  (self->view_object)->GetDrawable( ), buffer, &xp, &yp )) > widest )
	{
	widest = wide;
	strcpy( widest_buffer, buffer );
	}
      }
    font = fontdesc::Create( font_family, font_style, font_size = 4 );
    (self->view_object)->SetFont(  font );
    while ( (! adjusted)  &&  font_size >= 4  &&  font_size <= 34/*===*/ )
      {
      prior_font_size = font_size;;
      DEBUGdt(Prior-Font-size,prior_font_size);
      widest = (font)->StringSize(  (self->view_object)->GetDrawable( ), widest_buffer, &xp, &yp );
      DEBUGdt(Widest,widest);
      tallest = lines * font_size;
      DEBUGdt(Tallest,tallest);
      if ( ! too_small )
	{ DEBUG(Too Big...);
	if ( widest > (width + 2) )       { DEBUG(Too Wide);  too_wide++; }
	if ( tallest > (height + 2) )     { DEBUG(Too Tall);  too_tall++; }
	if ( too_wide  ||  too_tall )
	  {
	  too_big++; too_wide = too_tall = false;
	  font = fontdesc::Create( font_family, font_style, font_size -= 2 );
          }
	}
      if ( ! too_big )
	{ DEBUG(Too Small...)
	if ( widest < (width - 2) )	  { DEBUG(Too Narrow);  too_narrow++; }
	if ( tallest < (height - 2) )	  { DEBUG(Too Short);   too_short++;  }
	if ( too_narrow  &&  too_short )
	  {
	  too_small++; too_narrow = too_short = false;
	  font = fontdesc::Create( font_family, font_style, font_size += 2 );
	  }
	}
      if ( !too_small  &&  !too_big )
        adjusted = true;
      (self->view_object)->SetFont(  font );
      if ( (font_size = (font )->GetFontSize( )) == prior_font_size )
	{ DEBUG(No change);
	adjusted = true;
	}
      }
    (self->view_object)->SetFont(  font );
    DEBUGdt(Final Font-size,(font )->GetFontSize( ));
    y = Y2;  x = X2;
    y_increment = (font )->GetFontSize( ) + 5 /*===fudge.s/b FontNewLine===*/;
    if ( figure->zip_figure_mode.zip_figure_mode_top )
      y = Y1;
    else
    if ( figure->zip_figure_mode.zip_figure_mode_middle )
      y = y - ((lines>>1) * y_increment);
    else
    if ( figure->zip_figure_mode.zip_figure_mode_bottom )
      y = Y3;
    if ( figure->zip_figure_mode.zip_figure_mode_left )
      x = X1;
    else
    if ( figure->zip_figure_mode.zip_figure_mode_right )
      x = X3;
    (self->view_object)->Set_Clip_Area(  pane, window_x_point, window_y_point, width, height );
    cursor = text;
    while ( *cursor )
      {
      buffer_ptr = buffer;
      while ( *cursor  &&  !(*cursor == '\\'  &&  *(cursor+1) == 'n') )
        *buffer_ptr++ = *cursor++;
      *buffer_ptr = 0;
      (self->view_object)->MoveTo(  x, y );
      (self->view_object)->DrawString(  buffer, mode );
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
    (self->view_object)->Set_Pane_Clip_Area(  pane );
{
/*===hokey: should have fontdesc facility===*/
char font_style_name[5];
    *font_style_name = 0;
    if ( font_style & fontdesc_Bold )
      strcat( font_style_name, "b" );
    if ( font_style & fontdesc_Italic )
      strcat( font_style_name, "i" );
    sprintf( buffer, "%s%s%ld", font_family, font_style_name, font_size );
}
    DEBUGst(Font-name,buffer);
    (self->data_object)->Set_Figure_Font(  figure, buffer );
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
zipofcapt::Set_Object_Point( zip_type_figure		   figure, int				   point, zip_type_point		   x , zip_type_point		   y )
          {
  long				  status = zip_ok;

  IN(zipofcapt::Set_Object_Point);
  if ( figure->zip_figure_points == NULL  &&
       (status = (this->data_object)->Allocate_Figure_Points_Vector(
		     &figure->zip_figure_points )) == zip_ok )
    figure->zip_figure_points->zip_points_count = 1;
  if ( status == zip_ok )
    {
    switch ( point )
      {
      case 1: /* Center */
	(this)->Adjust_Object_Point_Suite(  figure,
	    x - (figure_x_point + (figure_x_points(0) - figure_x_point)/2),
	    y - (figure_y_point - (figure_y_point - figure_y_points(0))/2) );
        break;
      case 2: /* 12 O'Clock */
	figure_y_points(0) -= y - figure_y_point;
	figure_y_point = y;
	break;
      case 3: /* Upper Right Corner */
        figure_x_points(0) = x;
	figure_y_point = y;
        break;
      case 4: /* 3 O'Clock */
	figure_x_point -= x - figure_x_points(0);
	figure_x_points(0) = x;
	break;
      case  5: /* Lower Right Corner */
        figure_x_points(0) = x;
        figure_y_points(0) = y;
	break;
      case 6: /* 6 O'Clock */
	figure_y_point += figure_y_points(0) - y;
	figure_y_points(0) = y;
	break;
      case 7: /* Lower Left Corner */
        figure_x_point = x;
	figure_y_points(0) = y;
	break;
      case 8: /* 9 O'Clock */
	figure_x_points(0) += figure_x_point - x;
	figure_x_point = x;
	break;
      case 9: /* Upper Left Corner */
        figure_x_point = x;
        figure_y_point = y;
	break;
      default: status = zip_failure; /*=== zip_invalid_handle ===*/
      }
    if ( status == zip_ok )
      {
      (this->data_object)->Set_Image_Extrema(  figure->zip_figure_image, x, y );
/*===handle both extrema 7/20/86===*/
/*===have extrema check for REDUCTIONS as well as EXPANSIONS 7/20/86===*/
      (this->data_object)->Set_Stream_Extrema(  figure->zip_figure_image->zip_image_stream,
			      figure->zip_figure_image );
      }
    }
  OUT(zipofcapt::Set_Object_Point);
  return  status;
  }

long
zipofcapt::Proximate_Object_Points( zip_type_figure		   figure, zip_type_pane		   pane, zip_type_pixel		   x , zip_type_pixel		   y )
          {
  int				  point = 0;
  zip_type_pixel			  X1, X2, X3, Y1, Y2, Y3;

  IN(zipofcapt::Proximate_Object_Points);
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
  OUT(zipofcapt::Proximate_Object_Points);
  return  point;
  }

boolean
zipofcapt::Enclosed_Object( zip_type_figure		   figure, zip_type_pane		   pane, zip_type_pixel		   x , zip_type_pixel		   y , zip_type_pixel		   w , zip_type_pixel		   h )
          {
  boolean			  enclosed = false;
  zip_type_pixel			  X1, X2, X3, Y1, Y2, Y3;

  IN(zipofcapt::Enclosed_Object);
  Compute_Handle_Positions( this, figure, pane, &X1, &X2, &X3, &Y1, &Y2, &Y3 );
  if ( X1 > x  &&  Y1 > y  &&  X2 < (x + w)  &&  Y2 < (y + h) )
    enclosed = true;
  OUT(zipofcapt::Enclosed_Object);
  return  enclosed;
  }

long
zipofcapt::Object_Enclosure( zip_type_figure		   figure, zip_type_pane		   pane, zip_type_pixel		  *x , zip_type_pixel		  *y , zip_type_pixel		  *w , zip_type_pixel		  *h )
          {
  zip_type_pixel			  X1, X2, X3, Y1, Y2, Y3;

  IN(zipofcapt::Object_Enclosure);
  Compute_Handle_Positions( this, figure, pane, &X1, &X2, &X3, &Y1, &Y2, &Y3 );
  *x = X1;  *y = Y1;  *w = abs(X3 - X1);  *h = abs(Y3 - Y1);
  OUT(zipofcapt::Object_Enclosure);
  return  zip_ok;
  }

long
zipofcapt::Highlight_Object_Points( zip_type_figure		   figure, zip_type_pane		   pane )
        {
  zip_type_pixel			  X1, X2, X3, Y1, Y2, Y3;

  IN(zipofcapt::Highlight_Object_Points);
  Compute_Handle_Positions( this, figure, pane, &X1, &X2, &X3, &Y1, &Y2, &Y3 );
  (this->edit_object)->Highlight_Handles(  pane, X1, X2, X3, Y1, Y2, Y3 );
  OUT(zipofcapt::Highlight_Object_Points);
  return  zip_ok;
  }

long
zipofcapt::Normalize_Object_Points( zip_type_figure		   figure, zip_type_pane		   pane )
        {
  zip_type_pixel			  X1, X2, X3, Y1, Y2, Y3;

  IN(zipofcapt::Normalize_Object_Points);
  Compute_Handle_Positions( this, figure, pane, &X1, &X2, &X3, &Y1, &Y2, &Y3 );
  (this->edit_object)->Normalize_Handles(  pane, X1, X2, X3, Y1, Y2, Y3 );
  OUT(zipofcapt::Normalize_Object_Points);
  return  zip_ok;
  }

long
zipofcapt::Adjust_Object_Point_Suite( zip_type_figure		   figure, zip_type_point		   x_delta , zip_type_point		   y_delta )
        {
  long				  status = zip_ok;

  IN(zipofcapt::Adjust_Object_Point_Suite);
  figure_x_point += x_delta;
  figure_y_point += y_delta;
  figure_x_points(0) += x_delta;
  figure_y_points(0) += y_delta;
  (this->data_object)->Set_Image_Extrema(  figure->zip_figure_image, figure_x_point, figure_y_point );
  (this->data_object)->Set_Image_Extrema(  figure->zip_figure_image, figure_x_points(0), figure_y_points(0) );
/*===have extrema check for REDUCTIONS as well as EXPANSIONS 7/20/86===*/
  (this->data_object)->Set_Stream_Extrema(  figure->zip_figure_image->zip_image_stream,
			    figure->zip_figure_image );
  OUT(zipofcapt::Adjust_Object_Point_Suite);
  return  status;
  }

static
void Compute_Handle_Positions( class zipofcapt		  *self, zip_type_figure		   figure, zip_type_pane		   pane, zip_type_pixel		  *X1 , zip_type_pixel		  *X2 , zip_type_pixel		  *X3, zip_type_pixel		  *Y1 , zip_type_pixel		  *Y2 , zip_type_pixel		  *Y3 )
          {
  *X1 = (self->view_object)->X_Point_To_Pixel(  pane, figure, figure_x_point );
  *X2 = *X1 + (window_x_points(0) - window_x_point)/2;
  *X3 = (self->view_object)->X_Point_To_Pixel(  pane, figure, figure_x_points(0) );
  *Y1 = (self->view_object)->Y_Point_To_Pixel(  pane, figure, figure_y_point );
  *Y2 = *Y1 + (window_y_points(0) - window_y_point)/2;
  *Y3 = (self->view_object)->Y_Point_To_Pixel(  pane, figure, figure_y_points(0) );
  }
