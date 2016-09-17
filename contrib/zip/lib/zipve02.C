/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'     *
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
/* zipve02.c	Zip EditView-object  -- Palettes		      */
/* Author	TC Peters					      */
/* Information Technology Center	   Carnegie-Mellon University */


/**  SPECIFICATION -- External Facility Suite  *********************************

TITLE	The Zip EditView-object  --  Palettes

MODULE	zipve02.c

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
  10/13/88	Fix Mode Setting on Captions (TCP)
  05/01/89	Use symbolic font-names (TCP)
  08/22/89	Re-Highlight handles on Change_Shade (SCG)
   08/16/90	Improve Re-Highlight handles on Change_Shade, to work all the time (SCG)

END-SPECIFICATION  ************************************************************/


#include <andrewos.h>
#include <view.H>
#include <fontdesc.H>
#include <zip.H>
#include "zipifm00.h"
#include <zipview.H>
#include <zipobject.H>
#include <zipedit.H>
#include <zipedit.h>




int zipedit_Redisplay_Edit_Pane( class zipedit		  *self, zip_type_pane		   pane );
int zipedit_Handle_Shade_Palette_Hit( class zipedit		  *self, zip_type_pane		   pane, enum view_MouseAction	   action, long				   x , long				   y , long				   clicks );
static long Change_Shade( class zipedit		  *self, zip_type_pane		   pane, zip_type_figure		   figure, long				   shade );
int zipedit_Handle_Figure_Palette_Hit( class zipedit		  *self, zip_type_pane		   pane, enum view_MouseAction				   action , int				   x , int				   y , int				   clicks );
static int Figure_Palette_LBDT( class zipedit		  *self, zip_type_pane		   icon_pane, int				   x , int				   y , int				   clicks );
static int Create_Name_Palette( class zipedit		  *self, zip_type_pane		   containing_pane, zip_type_pane		   pane, zip_type_pane		  *palette );
static int Create_Font_Palette( class zipedit		  *self, zip_type_pane		   containing_pane, zip_type_pane		   pane, zip_type_pane		  *palette );
static int Create_Font_Icon( class zipedit		  *self, zip_type_pane		   containing_pane , zip_type_pane		   pane , zip_type_pane		  *palette, 		  const char				  *source, char				   cursor, int				   x , int				   y , int				   width , int				   height, const char *name );
void zipedit_Expose_Font_Palette( class zipedit		  *self, zip_type_pane		   pane );
long zipedit_Hide_Font_Palette( class zipedit		  *self, zip_type_pane		   pane );
static int Create_Shade_Palette( class zipedit		  *self, zip_type_pane containing_pane, zip_type_pane		   pane, zip_type_pane		  *palette );
void zipedit_Expose_Shade_Palette( class zipedit		  *self, zip_type_pane		   pane );
void zipedit_Hide_Shade_Palette( class zipedit		  *self, zip_type_pane		   pane );
static int Create_Figure_Palette( class zipedit		  *self, zip_type_pane		   containing_pane, zip_type_pane		   pane, zip_type_pane		  *palette );
void zipedit_Expose_Figure_Palette( class zipedit		  *self, zip_type_pane		   pane );
void zipedit_Hide_Figure_Palette( class zipedit		  *self, zip_type_pane		   pane );
static int Create_Figure_Icon( class zipedit		  *self, zip_type_pane		   containing_pane, zip_type_pane		   editing_pane, zip_type_pane		  *palette, long				   type, 		    char				   icon, const char				  *icon_font_name, char				   cursor, const char				  *cursor_font_name, 		    long				   serial , long				   count );
static int Create_Attribute_Palette( class zipedit		  *self, zip_type_pane			   containing_pane, zip_type_pane			   pane, zip_type_pane			  *palette );
static int Create_TL_Palette( class zipedit		  *self, zip_type_pane		   containing_pane, zip_type_pane		   pane, zip_type_pane		  *palette );
void zipedit_Expose_TL_Palette( class zipedit		  *self, zip_type_pane		   pane );
void zipedit_Hide_TL_Palette( class zipedit		  *self, zip_type_pane		   pane );
static int Create_TR_Palette( class zipedit		  *self, zip_type_pane		   containing_pane, zip_type_pane		   pane, zip_type_pane		  *palette );
void zipedit_Expose_TR_Palette( class zipedit		  *self, zip_type_pane		   pane );
void zipedit_Hide_TR_Palette( class zipedit		  *self, zip_type_pane		   pane );
static int Create_BL_Palette( class zipedit		  *self, zip_type_pane			   containing_pane, zip_type_pane			   pane, zip_type_pane			  *palette );
void zipedit_Expose_BL_Palette( class zipedit		  *self, zip_type_pane		   pane );
void zipedit_Hide_BL_Palette( class zipedit		  *self, zip_type_pane		   pane );
static int Create_BR_Palette( class zipedit		  *self, zip_type_pane			   containing_pane, zip_type_pane			   pane, zip_type_pane			  *palette );
void zipedit_Expose_BR_Palette( class zipedit		  *self, zip_type_pane		   pane );
void zipedit_Hide_BR_Palette( class zipedit		  *self, zip_type_pane		   pane );
static int Create_Palette_Surround( class zipedit  *self, zip_type_pane  pane, zip_type_pane  *palette, const char *name, int x_origin , int   y_origin , int width , int height );
int zipedit_Handle_Font_Family_Selection( class zipedit *self, zip_type_pane pane, enum view_MouseAction  action, long  x , long  y , long  clicks );
int zipedit_Handle_Font_Height_Selection( class zipedit	      *self, zip_type_pane	       pane, enum view_MouseAction       action, long			       x , long			       y , long			       clicks );
int zipedit_Handle_Font_Italic_Selection( class zipedit	      *self, zip_type_pane	       pane, int			       action , long			       x , long			       y , long			       clicks );
int zipedit_Handle_Font_Bold_Selection( class zipedit	      *self, zip_type_pane	       pane, enum view_MouseAction       action, long			       x , long			       y , long			       clicks );
int zipedit_Handle_Font_Sample_Selection( class zipedit	      *self, zip_type_pane	       pane, enum view_MouseAction       action, long			       x , long			       y , long			       clicks );
static void Set_Sample( class zipedit	      *self, zip_type_pane	       pane, boolean		       draw_pane );
static boolean Change_Figure_Font_And_Mode( class zipedit		  *self, zip_type_pane		   pane, zip_type_figure		   figure, long				   font , long				   mode );

extern int zipedit_Prepare_Editing_Control( class zipedit *self, zip_type_pane pane );

long
zipedit::Set_Palettes( zip_type_pane	       pane, int			       palette_mode )
        {
  int			      status = zip_success;
  class zipedit *self=this;

  IN(zipedit::Set_Palettes);
  if ( pane )
    {
    if ( (status = zipedit_Prepare_Editing_Control( self, pane )) == zip_ok )
      PaletteMode = palette_mode;
    }
  ZIP_STATUS(self->data_object);
  DEBUGdt(Status,status);
  OUT(zipedit::Set_Palettes);
  return status;
  }

long
zipedit::Expose_Palettes( zip_type_pane	       pane )
      {
  class zipedit *self=this;
  int			      status = zip_ok;
  char			     *client_data;
  class fontdesc	     *current_font =
					(this->view_object )->GetFont( );
  zip_type_pane	      container;
  static int			      container_number = 1;
  char				      container_full_name[257];
  long			      modified;

  IN(zipedit::Expose_Palette);
  DEBUGst(Pane-name,pane->zip_pane_name);
  if ( pane  &&  pane->zip_pane_edit )
    {
    if ( PalettePanes == NULL )
      {
      status = (this->view_object)->Create_Nested_Pane( 
		     &pane->zip_pane_edit->zip_pane_edit_containing_pane,
		     "ContainingPane", pane, 0 );
      container = pane->zip_pane_edit->zip_pane_edit_containing_pane;
      bcopy( pane, container, sizeof(struct zip_pane) );
      container->zip_pane_source.zip_pane_stream = NULL;
      sprintf( container_full_name, "%s-CONTAINER-%02d",
	       pane->zip_pane_name, container_number );
      container->zip_pane_name = strdup( container_full_name );
      DEBUG(Name Set);
      if ( (PalettePanes = (struct zip_pane_palettes *)
		calloc( 1, sizeof(struct zip_pane_palettes) +
			 PalettePaneCount * sizeof(zip_type_pane) )) == NULL )
	{
	DEBUG(Unable to Allocate Palette-Panes);
	status = zip_insufficient_pane_space;
/*===	apt_Acknowledge( "Insufficient Space for Palettes" );===*/
	}
      DEBUG(Allocated Palette-Panes);
      PalettePanes->zip_pane_palette_count = PalettePaneCount;
      modified = (Data )->GetModified( );
      if ( status != zip_ok  ||
        (status = Create_Name_Palette(	    self, container, pane, &NamesPane ))	||
        (status = Create_Font_Palette(	    self, container, pane, &FontsPane ))	||
        (status = Create_Figure_Palette(    self, container, pane, &FiguresPane ))	||
        (status = Create_Shade_Palette(	    self, container, pane, &ShadesPane ))	||
        (status = Create_Attribute_Palette( self, container, pane, &AttributesPane ))	||
	(status = Create_TL_Palette(	    self, container, pane, &TLPane ))		||
	(status = Create_TR_Palette(	    self, container, pane, &TRPane ))		||
        (status = Create_BL_Palette(	    self, container, pane, &BLPane ))		||
	(status = Create_BR_Palette(	    self, container, pane, &BRPane ))  )
	{ /*=== handle error ===*/}
      Data->modified = modified;
      (this->view_object)->Remove_Pane(  container );
      }

    if ( status == zip_ok )
      {
      client_data = pane->zip_pane_client_data;
      pane->zip_pane_client_data = (char *)
		calloc( 1, sizeof(struct ZIP_edit_client_data) );
      PriorClientData	     = client_data;
      PriorXOrigin         = pane->zip_pane_x_origin_percent;
      PriorYOrigin         = pane->zip_pane_y_origin_percent;
      PriorWidth           = pane->zip_pane_width_percent;
      PriorHeight          = pane->zip_pane_height_percent;
      PriorBorderFont      = pane->zip_pane_border_font;
      PriorBorderPattern   = pane->zip_pane_border_pattern;
      PriorBorderThickness = pane->zip_pane_border_thickness;
      EditPane = pane;
      if ( FigurePalette )
	{
        zipedit_Expose_Figure_Palette( self, pane );
	(this->view_object)->Display_Pane(  TRPane );
	(this->view_object)->Display_Pane(  BRPane );
	}
      if ( NamePalette )
	{
        (this->view_object)->Display_Pane(  NamesPane );
	(this->view_object)->Display_Pane(  TLPane );
	(this->view_object)->Display_Pane(  TRPane );
	}
      if ( FontPalette )
	{
        zipedit_Expose_Font_Palette( self, pane );
	(this->view_object)->Display_Pane(  TLPane );
	(this->view_object)->Display_Pane(  TRPane );
	}
      if ( ShadePalette )
	{
        zipedit_Expose_Shade_Palette( self, pane );
	(this->view_object)->Display_Pane(  BLPane );
	(this->view_object)->Display_Pane(  BRPane );
	}
      if ( AttributePalette )
	{
        (this->view_object)->Display_Pane(  AttributesPane );
        (this->view_object)->Display_Pane(  TLPane );
	(this->view_object)->Display_Pane(  BLPane );
	}
      (self)->Align_Pane(  pane );
      zipedit_Redisplay_Edit_Pane( self, pane );
      (this->view_object)->SetFont(  current_font ); (this->view_object )->FlushGraphics( );
      }
    PalettesExposed = true;
    }
  ZIP_STATUS(this->data_object);
  OUT(zipedit::Expose_Palettes);
  return status;
  }

long
zipedit::Hide_Palettes( zip_type_pane	       pane )
      {
  int			      status = zip_ok;
  class zipedit *self=this;
  IN(zipedit::Hide_Palettes);
  DEBUGst(Pane-name,pane->zip_pane_name);
  if ( pane )
    {
    zipedit_Hide_Font_Palette( self, pane );
    zipedit_Hide_Figure_Palette( self, pane );
    zipedit_Hide_Shade_Palette( self, pane );
    (this->view_object)->Remove_Pane(  NamesPane );
    (this->view_object)->Remove_Pane(  AttributesPane );
    (this->view_object)->Remove_Pane(  TLPane );
    (this->view_object)->Remove_Pane(  TRPane );
    (this->view_object)->Remove_Pane(  BLPane );
    (this->view_object)->Remove_Pane(  BRPane );
    DEBUG(Palettes Removed);
    if ( PriorPane )
      (this->view_object)->Set_Pane_Inverted(  PriorPane, false );
    (this->view_object)->Set_Pane_Coordinates(  pane,
	PriorXOrigin, PriorYOrigin, PriorWidth, PriorHeight );
    (this->view_object)->Set_Pane_Border(  pane, NULL,
	PriorBorderPattern, PriorBorderThickness );
    pane->zip_pane_border_font = PriorBorderFont;
    pane->zip_pane_client_data = PriorClientData;
    (this->view_object)->Display_Pane(  pane );
    PalettesExposed = false;
    }
  ZIP_STATUS(this->data_object);
  OUT(zipedit::Hide_Palettes);
  return status;
  }

void zipedit::Align_Pane( zip_type_pane		   pane )
      {
/*===
  char					 *p = NULL;
  zip_type_stream			  stream = zipview_Pane_Stream( View, pane );
  int					  count, x_offset, y_offset, pseudo_x, pseudo_y;
  float					  scale;
===*/
  IN(zipedit::Align_Pane);
/*===
  if ( stream && pane->zip_pane_auxiliary_stream )
    {
    p = zip_Image_Name( Data, zip_Image_Inferior( Data, stream->zip_stream_image_anchor ));
    if ( p )
      {
      count = sscanf( p, "%f:%d:%d:%d:%d", &scale, &x_offset, &y_offset,
                        &pseudo_x, &pseudo_y );
      if ( count == 5 )
        {
        pane->zip_pane_x_offset = x_offset;
        pane->zip_pane_y_offset = y_offset;
        zipview_Set_Pane_Scale( this->view_object, pane, scale );
        stream->zip_stream_pseudo_x_offset = pseudo_x;
        stream->zip_stream_pseudo_y_offset = pseudo_y; */
/*=== investigate args
        zip_Adjust_Image_Point_Suite( Data, stream->zip_stream_image_anchor, NULL,
                                      pseudo_x, pseudo_y, true );
===!/
	}
      }
    }
===*/
  OUT(zipedit::Align_Pane);
  }

extern void zipedit_Display_Background_Pane( class zipedit        *self, zip_type_pane            pane );
extern void Show_Enclosure( class zipedit *self, zip_type_pane pane );

int
zipedit_Redisplay_Edit_Pane( class zipedit		  *self, zip_type_pane		   pane )
      {
  int				  status = zip_ok;
  int				  pad_width = 100, pad_height = 100,
					  pad_x_center = 50, pad_y_center = 50;

  IN(zipedit_Redisplay_Edit_Pane);
  if ( FigurePalette )	{ pad_width  -= 10;  pad_x_center -= 5; }
  if ( ShadePalette )	{ pad_height -= 10;  pad_y_center -= 5; }
  if ( AttributePalette ){ pad_width  -= 10;  pad_x_center += 5; }
  if ( NamePalette  ||  FontPalette ) { pad_height -= 10;  pad_y_center += 5; }
  (self->view_object)->Set_Pane_Coordinates(  pane, pad_x_center, pad_y_center,
				pad_width, pad_height );
  (self->view_object)->Set_Pane_Border(  pane, NULL, 0, 1 );
  (self->view_object)->Clear_Pane(  pane );
  if ( BackgroundPane  &&  !(self->view_object)->Pane_Removed(  BackgroundPane ) )
    zipedit_Display_Background_Pane( self, pane );
  (self->view_object)->Draw_Pane(  pane );
  if ( EnclosureExposed )
    Show_Enclosure( self, pane );
  if ( PendingProcessor )
    self->pending_processor((class ziposymbol *)self->pending_anchor, pane );
  OUT(zipedit_Redisplay_Edit_Pane)
  return status;
  }


extern zip_type_figure zipedit_Next_Selected_Figure( class zipedit *self, zip_type_pane pane, zip_type_figure figure );

int
zipedit_Handle_Shade_Palette_Hit( class zipedit		  *self, zip_type_pane		   pane, enum view_MouseAction	   action, long				   x , long				   y , long				   clicks )
          {
  long				  status = zip_ok, shade = 0, changed = false;
  zip_type_figure		  figure, display, 
					  selector, transparent;
  char				 *ptr;

  IN(zipedit_Handle_Shade_Palette_Hit);
  if ( action == view_LeftDown  &&
       (figure = (self->view_object)->Within_Which_Figure(  x, y )) )
    {
    DEBUGst(Figure-name,figure->zip_figure_name);
    display  = (self->data_object)->Figure(  "ZIP-SHADE-DISPLAY" );
    selector = (self->data_object)->Figure(  "ZIP-SHADE-SELECTOR" );
    transparent = (self->data_object)->Figure(  "ZIP-SHADE-TRANSPARENT" );
    (self->view_object)->Clear_Figure(   selector, pane );
    (self->view_object)->Hide_Figure(   transparent, pane );
    if ( ( ptr = (char *) index( figure->zip_figure_name, '=' ) ) )
      { DEBUG(Change Shade);
      shade = atoi( ++ptr );
      DEBUGdt(Shade,shade);
      (self->data_object)->Set_Figure_Shade(  display, shade );
      (self->view_object)->Draw_Figure(   display, pane );
      selector->zip_figure_point.zip_point_x = figure->zip_figure_point.zip_point_x +
	((figure->zip_figure_points->zip_points[0].zip_point_x -
	    figure->zip_figure_point.zip_point_x) / 2);
      selector->zip_figure_point.zip_point_y = figure->zip_figure_point.zip_point_y +
	((figure->zip_figure_points->zip_points[0].zip_point_y -
	    figure->zip_figure_point.zip_point_y) / 2);
      (self->view_object)->Draw_Figure(   selector, pane );
      EditingPane(pane)->zip_pane_edit->zip_pane_edit_current_shade = shade;
      }
      else
      {
      if ( figure == display  ||  figure == transparent )
	{ DEBUG(Become Transparent);
        (self->view_object)->Clear_Figure(  display, pane );
        (self->data_object)->Set_Figure_Shade(  display, 0 );
        (self->view_object)->Draw_Figure(   display, pane );
        (self->view_object)->Expose_Figure(  transparent, pane );
        EditingPane(pane)->zip_pane_edit->zip_pane_edit_current_shade = shade = 0; 
	}
      }
    figure = NULL;
    while ( ( figure = zipedit_Next_Selected_Figure( self, EditingPane(pane), figure ) ) )
      if ( Change_Shade( self, EditingPane(pane), figure, shade ) )
	changed = true;
    if ( changed )
      {
      Show_Enclosure( self, EditingPane(pane) );
      (self->view_object)->Draw_Pane(  EditingPane(pane) );
      figure = NULL;
      while ( ( figure = zipedit_Next_Selected_Figure( self, EditingPane(pane), figure )) )
        (self)->Highlight_Figure_Points(  figure, EditingPane(pane));
      Show_Enclosure( self, EditingPane(pane) );
      }
    }
  OUT(zipedit_Handle_Shade_Palette_Hit);
  return status;
  }

static
long Change_Shade( class zipedit		  *self, zip_type_pane		   pane, zip_type_figure		   figure, long				   shade )
          {
  long				  changed = false;

  IN(Change_Shade);
  (self)->Normalize_Figure_Points(  figure, pane );
  if ( figure->zip_figure_mode.zip_figure_mode_shaded )
    (self->view_object)->Clear_Figure(  figure, pane );
  if ( (Objects(figure->zip_figure_type))->Set_Object_Shade(
	 figure, shade ) == zip_ok )
    changed = true;
  OUT(Change_Shade);
  return  changed;
  }

int
zipedit_Handle_Figure_Palette_Hit( class zipedit		  *self, zip_type_pane		   pane, enum view_MouseAction				   action , int				   x , int				   y , int				   clicks )
        {
  int				  status = zip_ok;

  IN(zipedit_Handle_Figure_Palette_Hit);
  switch ( action )
    {
    case view_LeftDown:
      Figure_Palette_LBDT( self, pane, x, y, clicks );
      break;
    case view_LeftUp:
    case view_LeftMovement:
    case view_RightDown:
    case view_RightUp:
    case view_RightMovement:
    default: ;
    }
  OUT(zipedit_Handle_Figure_Palette_Hit);
  return status;
  }

extern void zipedit_Cancel_Enclosure( class zipedit *self, zip_type_pane pane );
extern void zipedit_Hide_Selection_Menu( class zipedit          *self );

static int
Figure_Palette_LBDT( class zipedit		  *self, zip_type_pane		   icon_pane, int				   x , int				   y , int				   clicks )
        {
  int				  status = zip_ok;
  zip_type_pane		  pane = EditingPane(icon_pane);

  IN(Figure_Palette_LBDT);
  if ( icon_pane->zip_pane_client_data )
    {
    if ( self->keyboard_processor )
      (*self->keyboard_processor)( KeyboardAnchor, pane, 0, view_NoMouseEvent, 0, 0, 0 );
    zipedit_Cancel_Enclosure( self, pane );
    zipedit_Hide_Selection_Menu( self );
    if ( CurrentFigure ) {
      if ( SelectionLevel >= ImageSelection )
      (self)->Normalize_Image_Points(  CurrentImage, pane );
      else
      (self)->Normalize_Figure_Points(  CurrentFigure, pane );
    }
    SelectionLevel = 0;
    (self)->Set_Pane_Highlight_Icon(  pane, zip_figure_point_icon );

    pane = icon_pane;
    (self->view_object)->Invert_Pane(  FigurePane );
    (self->view_object)->Invert_Pane(  FigurePane = pane );
    PendingFigureType = EditingType(pane);
    (self->view_object)->Set_Pane_Cursor(  EditingPane(pane), EditingIcon(pane), CursorFontName );
    status = (Objects(PendingFigureType))->Show_Object_Properties(  EditingPane(pane), NULL );
    }
  (self->view_object )->FlushGraphics( );
  OUT(Figure_Palette_LBDT);
  return status;
  }

static int
Create_Name_Palette( class zipedit		  *self, zip_type_pane		   containing_pane, zip_type_pane		   pane, zip_type_pane		  *palette )
          {
  int				  status = zip_ok;
  zip_type_stream			  stream;
  static const char			  source[] =
"{ZIP_NAME_STREAM\n"
"Fandysans10b\n"
"*A;-590,75\nT Stream:\n"
"*A;-300,75\nNZIP_current_stream_name\nFandysans10\n"
"*A;-590,0\nT Image:\n"
"*A;-300,0\nNZIP_current_image_name\nFandysans10\n"
"*A;-590,-75\nT Figure:\n"
"*A;-300,-75\nNZIP_current_figure_name\nFandysans10\n"
"}";

  IN(Create_Name_Palette);
  Create_Palette_Surround( self, containing_pane, palette, "NAME", 30,5,40,10 );
  (self->data_object)->Create_Stream(  &stream, NULL, zip_default );
  (self->data_object)->Set_Stream_Source(  stream, source );
  (self->view_object)->Set_Pane_Stream(  *palette, stream );
  (self->view_object)->Remove_Pane(  *palette );
  OUT(Create_Name_Palette);
  return status;
  }

static int
Create_Font_Palette( class zipedit		  *self, zip_type_pane		   containing_pane, zip_type_pane		   pane, zip_type_pane		  *palette )
          {
  int				  status = zip_ok;
  static const char			  family[] =
"{ZIP_FONT_FAMILY\n*A;0,0\nNfont_catalog_family\nMMC\nFandysans10\nTSerif\n}";
  static const char				  height[] =
"{ZIP_FONT_HEIGHT\n*A;0,0\nNfont_catalog_height\nMMC\nFandysans10\nT12\n}";
  static const char				  italic[] =
"{ZIP_FONT_ITALIC\n*A;0,0\nNfont_catalog_italic\nMMC\nFandysans10\nTItalic\n}";
  static const char				  bold[] =
"{ZIP_FONT_BOLD\n*A;0,0\nNfont_catalog_bold\nMMC\nFandysans10\nTBold\n}";
  static const char				  sample[] =
"{ZIP_FONT_SAMPLE\n*A;0,0\nNfont_catalog_sample\nMMC\nFandy12\nTSample\n*C;0,50\n>0,-50\n*C;-100,0\n>100,0\n}";

  IN(Create_Font_Palette);
  if (
    (status = Create_Palette_Surround( self, containing_pane, palette,  
		"FONT-SURROUND", 70,5,40,10 )) != zip_ok  ||
    (status = Create_Font_Icon( self, containing_pane, pane, &FontFamilyPane,
		family, '2', 55, 3, 6, 5, "FONT-FAMILY" )) != zip_ok  || 
    (status = Create_Font_Icon( self, containing_pane, pane, &FontHeightPane,
		height, '2', 65, 3, 4, 5, "FONT-HEIGHT" )) != zip_ok  || 
    (status = Create_Font_Icon( self, containing_pane, pane, &FontItalicPane,
		italic, '2', 55, 7, 6, 5, "FONT-ITALIC" )) != zip_ok  || 
    (status = Create_Font_Icon( self, containing_pane, pane, &FontBoldPane,
		bold,   '2', 65, 7, 6, 5, "FONT-BOLD" )) != zip_ok  || 
    (status = Create_Font_Icon( self, containing_pane, pane, &FontSamplePane,
		sample, '2', 80, 5, 19, 8, "FONT-SAMPLE" )) != zip_ok  || 
    (status = zipedit_Hide_Font_Palette( self, pane )) != zip_ok 
    )
    {}
    else
    {
    (self->data_object)->Define_Font(  "andy12", &pane->zip_pane_current_font );
    pane->zip_pane_current_mode.zip_figure_mode_middle = on;
    pane->zip_pane_current_mode.zip_figure_mode_center = on;
    }
  OUT(Create_Font_Palette);
  return status;
  }

static int
Create_Font_Icon( class zipedit		  *self, zip_type_pane		   containing_pane , zip_type_pane		   pane , zip_type_pane		  *palette,
		  const char				  *source, char				   cursor, int				   x , int				   y , int				   width , int				   height, const char *name )
                {
  int				  status = zip_ok;
  zip_type_stream			  stream;

  IN(Create_Font_Icon);
  if (
    (status = (self->view_object)->Create_Nested_Pane(  palette, name, containing_pane, zip_default )) != zip_ok  ||
    (status = (self->view_object)->Set_Pane_Coordinates(  *palette, x, y,
		 width, height )) != zip_ok  ||
    (status = (self->view_object)->Set_Pane_Cursor(  *palette, cursor, NULL )) != zip_ok  ||
    (status = (self->data_object)->Create_Stream(  &stream, NULL, zip_default )) != zip_ok  ||
    (status = (self->data_object)->Set_Stream_Source(  stream, source )) != zip_ok  ||
    (status = (self->view_object)->Set_Pane_Stream(  *palette, stream )) != zip_ok
    )
    {DEBUG(ERROR);}
    else
    {
    (*palette)->zip_pane_client_data = (char *)
		 calloc( 1, sizeof(struct ZIP_edit_client_data) );
    EditingIcon(*palette) = cursor;
    EditingPane(*palette) = pane;
    }
  OUT(Create_Font_Icon);
  return status;
  }

void zipedit_Expose_Font_Palette( class zipedit		  *self, zip_type_pane		   pane )
      {
  IN(zipedit_Expose_Font_Palette);
  (self->view_object)->Display_Pane(  FontsPane );
  (self->view_object)->Display_Pane(  FontFamilyPane );
  (self->view_object)->Display_Pane(  FontHeightPane );
  (self->view_object)->Display_Pane(  FontItalicPane );
  (self->view_object)->Display_Pane(  FontBoldPane );
  (self->view_object)->Display_Pane(  FontSamplePane );
  OUT(zipedit_Expose_Font_Palette);
  }

long zipedit_Hide_Font_Palette( class zipedit		  *self, zip_type_pane		   pane )
      {
  IN(zipedit_Hide_Font_Palette);
  (self->view_object)->Remove_Pane(  FontsPane );
  (self->view_object)->Remove_Pane(  FontFamilyPane );
  (self->view_object)->Remove_Pane(  FontHeightPane );
  (self->view_object)->Remove_Pane(  FontItalicPane );
  (self->view_object)->Remove_Pane(  FontBoldPane );
  (self->view_object)->Remove_Pane(  FontSamplePane );
  OUT(zipedit_Hide_Font_Palette);
  return  zip_ok;
  }

static int
Create_Shade_Palette( class zipedit		  *self, zip_type_pane containing_pane, zip_type_pane		   pane, zip_type_pane		  *palette )
        {
  int				  status = zip_ok;
  zip_type_stream			  stream;
  static const char			  source[] =
"{\n"
"*J;-1100,0\n"
"NZIP-SHADE-SELECTOR\n"
">77,0\n"
"*G;-1200,100\n"
">-1000,-100\n"
"*G;-1175,75\n"
"NZIP-SHADE-DISPLAY\n"
">-1025,-75\n"
"*A;-1100,0\n"
"NZIP-SHADE-TRANSPARENT\n"
"Fzipicn20\n"
"T2\n"
"*G;-900,50\n"
"#White\n"
"NZIP-SHADE=01\n"
"G1\n"
">-800,-50\n"
"*G;-700,50\n"
"NZIP-SHADE=10\n"
"G10\n"
">-600,-50\n"
"*G;-500,50\n"
"NZIP-SHADE=20\n"
"G20\n"
">-400,-50\n"
"*G;-300,50\n"
"NZIP-SHADE=30\n"
"G30\n"
">-200,-50\n"
"*G;-100,50\n"
"NZIP-SHADE=40\n"
"G40\n"
">0,-50\n"
"*G;100,50\n"
"NZIP-SHADE=50\n"
"G50\n"
">200,-50\n"
"*G;300,50\n"
"NZIP-SHADE=60\n"
"G60\n"
">400,-50\n"
"*G;500,50\n"
"NZIP-SHADE=70\n"
"G70\n"
">600,-50\n"
"*G;700,50\n"
"NZIP-SHADE=80\n"
"G80\n"
">800,-50\n"
"*G;900,50\n"
"NZIP-SHADE=100\n"
"#Black\n"
"G100\n"
">1000,-50\n"
"#*G;1100,100\n"
"#NZIP-SHADE=100\n"
"#G100\n"
"#>1200,-100\n"
"}";


  IN(Create_Shade_Palette);
  Create_Palette_Surround( self, containing_pane, palette, "SHADE-SURROUND", 50, 95, 80, 10 );
  (*palette)->zip_pane_client_data = (char *)
	 calloc( 1, sizeof(struct ZIP_edit_client_data) );
  EditingPane(*palette) = pane;
  (self->data_object)->Create_Stream(  &stream, NULL, zip_default );
  (self->data_object)->Set_Stream_Source(  stream, source );
  (self->view_object)->Set_Pane_Stream(  *palette, stream );
  zipedit_Hide_Shade_Palette( self, pane );
  OUT(Create_Shade_Palette);
  return status;
  }

void zipedit_Expose_Shade_Palette( class zipedit		  *self, zip_type_pane		   pane )
      {
  IN(zipedit_Expose_Shade_Palette);
  (self->view_object)->Display_Pane(  ShadesPane );
  OUT(zipedit_Expose_Shade_Palette);
  }

void zipedit_Hide_Shade_Palette( class zipedit		  *self, zip_type_pane		   pane )
      {
  IN(zipedit_Hide_Shade_Palette);
  (self->view_object)->Remove_Pane(  ShadesPane );
  OUT(zipedit_Hide_Shade_Palette);
  }

#define FigureIconPane(i)   PalettePanes->zip_pane_palette_vector[20+i]
static int
Create_Figure_Palette( class zipedit		  *self, zip_type_pane		   containing_pane, zip_type_pane		   pane, zip_type_pane		  *palette )
          {
  long				  status = zip_ok, i;
  long				  icon_count = 0,
					  serial = 0;

  IN(Create_Figure_Palette);
  Create_Palette_Surround( self, containing_pane, palette, "FIGURE-SURROUND", 95, 50, 10, 80 );
  for ( i = 1; Objects(i); i++ )
    {
    DEBUGct(Icon,(Objects(i) )->Object_Icon( ));
    DEBUGst(Icon-Font-Name,(Objects(i) )->Object_Icon_Font_Name( ));
    DEBUGct(Cursor,(Objects(i) )->Object_Icon_Cursor( ));
    DEBUGst(Cursor-Font-Name,(Objects(i) )->Object_Icon_Cursor_Font_Name( ));
    DEBUGct(DataStream-Code,(Objects(i) )->Object_Datastream_Code( ));
    if ( (Objects(i) )->Object_Icon( ) )
      icon_count++;
      else {DEBUGdt(No Icon,i);}
    }
  DEBUGdt(Icon-count,icon_count);
  for ( i = 1; Objects(i); i++ )
    {
    if ( (Objects(i) )->Object_Icon( ) )
      Create_Figure_Icon( self, containing_pane, pane,
	&FigureIconPane(i),
	(Objects(i) )->Object_Datastream_Code( ) - '@',
	(Objects(i) )->Object_Icon( ),
	(Objects(i) )->Object_Icon_Font_Name( ),
	(Objects(i) )->Object_Icon_Cursor( ),
	(Objects(i) )->Object_Icon_Cursor_Font_Name( ),
	++serial, icon_count );
    }
  DEBUGdt(Serial,serial);
  zipedit_Hide_Figure_Palette( self, pane );
  OUT(Create_Figure_Palette);
  return status;
  }

void zipedit_Expose_Figure_Palette( class zipedit		  *self, zip_type_pane		   pane )
      {
  long				  i;

  IN(zipedit_Expose_Figure_Palette);
  (self->view_object)->Display_Pane(  FiguresPane );
  for ( i = 1; Objects(i); i++ )
    if ( FigureIconPane(i) )
      (self->view_object)->Display_Pane(  FigureIconPane(i) );
  OUT(zipedit_Expose_Figure_Palette);
  }


void zipedit_Hide_Figure_Palette( class zipedit		  *self, zip_type_pane		   pane )
      {
  long				  i;

  IN(zipedit_Hide_Figure_Palette);
  (self->view_object)->Remove_Pane(  FiguresPane );
  for ( i = 1; Objects(i); i++ )
    if ( FigureIconPane(i) )
      (self->view_object)->Remove_Pane(  FigureIconPane(i) );
  OUT(zipedit_Hide_Figure_Palette);
  }

static int
Create_Figure_Icon( class zipedit		  *self, zip_type_pane		   containing_pane, zip_type_pane		   editing_pane, zip_type_pane		  *palette, long				   type,
		    char				   icon, const char				  *icon_font_name, char				   cursor, const char				  *cursor_font_name,
		    long				   serial , long				   count )
                      {
  long				  status = zip_ok,
					  percentage = 80/count;
  zip_type_stream			  stream;
  char					  source[257];

  IN(Create_Figure_Icon);
  sprintf( source, "Figure_Icon_%c", cursor );
  (self->view_object)->Create_Nested_Pane(  palette, source, containing_pane, zip_default );
  (self->view_object)->Set_Pane_Coordinates(  *palette, 95, serial * percentage + 8, 8, percentage );
  (self->view_object)->Set_Pane_Cursor(  *palette, cursor, cursor_font_name );
  (*palette)->zip_pane_client_data = (char *)
	 calloc( 1, sizeof(struct ZIP_edit_client_data) );
  EditingType(*palette) = type;
  EditingIcon(*palette) = cursor;
  EditingPane(*palette) = editing_pane;
  (self->data_object)->Create_Stream(  &stream, NULL, zip_default );
  sprintf( source, "*A;0,0\nT%c\nF%s\n", icon, icon_font_name );
  (self->data_object)->Set_Stream_Source(  stream, source ); 
  (self->view_object)->Set_Pane_Stream(  *palette, stream );
  OUT(Create_Figure_Icon);
  return status;
  }

static int
Create_Attribute_Palette( class zipedit		  *self, zip_type_pane			   containing_pane, zip_type_pane			   pane, zip_type_pane			  *palette )
          {
  int					  status = zip_ok;
  zip_type_stream				  stream;
  static const char				  source[] =
"{ZIP_ATTRIBUTES\n"
"Fandysans10b\n"
"##*G;-120,1000\n##>120,-1000\n"
"\n"
"*C;-100,1000\n>-100,1000\n"
"*A;0,900\nTX-Point\nMMC\n"
"*A;0,850\nNZIP_current_x_point_name\nFandysans10\nMMC\n"
"\n"
"*A;0,700\nTY-Point\nMMC\n"
"*A;0,650\nNZIP_current_y_point_name\nFandysans10\nMMC\n"
"\n"
"*A;0,500\nTFont\nMMC\n"
"*A;0,450\nNZIP_current_font_name\nFandysans10\nMMC\n"
"*C;100,-1000\n>100,-1000\n"
"}";

  Create_Palette_Surround( self, containing_pane, palette, "ATTRIBUTE-SURROUND", 5,50,10,80 );
  (self->data_object)->Create_Stream(  &stream, NULL, zip_default );
  (self->data_object)->Set_Stream_Source(  stream, source );
  (self->view_object)->Set_Pane_Stream(  *palette, stream );
  (self->view_object)->Remove_Pane(  AttributesPane );
  return status;
  }

static int
Create_TL_Palette( class zipedit		  *self, zip_type_pane		   containing_pane, zip_type_pane		   pane, zip_type_pane		  *palette )
          {
  int				  status = zip_ok;
  zip_type_stream			  stream;
  static const char			  source[] =
"{\n"
"Fandysans16\n"
"##*1;0,0\n##T*\n"
"}";

  Create_Palette_Surround( self, containing_pane, palette, "TL-SURROUND", 5,5,10,10 );
  (self->data_object)->Create_Stream(  &stream, NULL, zip_default );
  (self->data_object)->Set_Stream_Source(  stream, source );
  (self->view_object)->Set_Pane_Stream(  *palette, stream );
  (self->view_object)->Remove_Pane(  TLPane );
  return status;
  }

void zipedit_Expose_TL_Palette( class zipedit		  *self, zip_type_pane		   pane )
      {
  (self->view_object)->Display_Pane(  TLPane );
  }

void zipedit_Hide_TL_Palette( class zipedit		  *self, zip_type_pane		   pane )
      {
  (self->view_object)->Remove_Pane(  TLPane );
  }

static int
Create_TR_Palette( class zipedit		  *self, zip_type_pane		   containing_pane, zip_type_pane		   pane, zip_type_pane		  *palette )
          {
  int				  status = zip_ok;
  zip_type_stream			  stream;
  static const char			  source[] =
"{\n"
"Fandysans10b\n"
"##*1;0,0\n##T*\n"
"}";

  Create_Palette_Surround( self, containing_pane, palette, "TR-SURROUND", 95, 5,10,10 ); 
  (self->data_object)->Create_Stream(  &stream, NULL, zip_default );
  (self->data_object)->Set_Stream_Source(  stream, source );
  (self->view_object)->Set_Pane_Stream(  *palette, stream );
  (self->view_object)->Remove_Pane(  TRPane );
  return status;
  }

void zipedit_Expose_TR_Palette( class zipedit		  *self, zip_type_pane		   pane )
      {
  (self->view_object)->Display_Pane(  TRPane );
  }

void zipedit_Hide_TR_Palette( class zipedit		  *self, zip_type_pane		   pane )
      {
  (self->view_object)->Remove_Pane(  TRPane );
  }

static int
Create_BL_Palette( class zipedit		  *self, zip_type_pane			   containing_pane, zip_type_pane			   pane, zip_type_pane			  *palette )
          {
  int					  status = zip_ok;
  zip_type_stream				  stream;
  static const char				  source[] =
"{\n"
"Fandysans10b\n"
"##*1;0,0\n##T*\n"
"}";

  Create_Palette_Surround( self, containing_pane, palette, "BL-SURROUND", 5,95, 10,10 );
  (self->data_object)->Create_Stream(  &stream, NULL, zip_default ); 
  (self->data_object)->Set_Stream_Source(  stream, source );
  (self->view_object)->Set_Pane_Stream(  *palette, stream );
  (self->view_object)->Remove_Pane(  BLPane );
  return status;
  }

void zipedit_Expose_BL_Palette( class zipedit		  *self, zip_type_pane		   pane )
      {
  (self->view_object)->Display_Pane(  BLPane );
  }

void zipedit_Hide_BL_Palette( class zipedit		  *self, zip_type_pane		   pane )
      {
  (self->view_object)->Remove_Pane(  BLPane );
  }

static int
Create_BR_Palette( class zipedit		  *self, zip_type_pane			   containing_pane, zip_type_pane			   pane, zip_type_pane			  *palette )
          {
  int					  status = zip_ok;
  zip_type_stream				  stream;
  static const char				  source[] =
"{\n"
"Fandysans10b\n"
"##*1;0,0\n##T*\n"
"}";

  Create_Palette_Surround( self, containing_pane, palette, "BR-SURROUND", 95,95,10,10 );
  (self->data_object)->Create_Stream(  &stream, NULL, zip_default );
  (self->data_object)->Set_Stream_Source(  stream, source );
  (self->view_object)->Set_Pane_Stream(  *palette, stream );
  (self->view_object)->Remove_Pane(  BRPane );
  return status;
  }

void zipedit_Expose_BR_Palette( class zipedit		  *self, zip_type_pane		   pane )
      {
  (self->view_object)->Display_Pane(  BRPane );
  }

void zipedit_Hide_BR_Palette( class zipedit		  *self, zip_type_pane		   pane )
      {
  (self->view_object)->Remove_Pane(  BRPane );
  }

static int
Create_Palette_Surround( class zipedit		  *self, zip_type_pane		   pane, zip_type_pane		  *palette, const char *name,
			    int				   x_origin , int				   y_origin , int				   width , int				   height )
          {
  int				  status = zip_ok;
  zip_type_stream			  stream;
  static const char			  source[] =
"{\n"
"Fandysans10b\n"
"##*1;0,0\n"
"##T*\n"
"}";

  IN(Create_Palette_Surround);
  (self->view_object)->Create_Nested_Pane(  palette, name, pane, zip_default );
  (self->view_object)->Set_Pane_Coordinates(  *palette, x_origin, y_origin, width, height );
  (self->view_object)->Set_Pane_Cursor(  *palette, 'A', CursorFontName );
  (self->view_object)->Set_Pane_Border(  *palette, 0, 0, 2 );
  (*palette)->zip_pane_client_data = (char *)
	calloc( 1, sizeof(struct ZIP_edit_client_data) );
  EditingType( *palette ) = 0;
  EditingIcon( *palette ) = 'A';
  (self->data_object)->Create_Stream(  &stream, NULL, zip_default );
  (self->data_object)->Set_Stream_Source(  stream, source );
  (self->view_object)->Set_Pane_Stream(  *palette, stream );
  OUT(Create_Palette_Surround);
  return status;
  }

int
zipedit_Handle_Font_Family_Selection( class zipedit	      *self, zip_type_pane	       pane, enum view_MouseAction       action, long			       x , long			       y , long			       clicks )
          {
  long			      status = zip_success;
  zip_type_figure	      figure;

  IN(zipedit_Handle_Font_Family_Selection);
  if ( action == view_LeftDown )
    {
    (self->view_object)->Clear_Pane(  pane );
    figure = (self->data_object)->Figure(  "font_catalog_family" );
    if ( ( FontFamily = ! FontFamily ) )
	{
	(self->data_object)->Set_Figure_Text(  figure, "Sans" );
	}
	else
	{
	(self->data_object)->Set_Figure_Text(  figure, "Serif" );
	}
    (self->view_object)->Draw_Figure(  figure, pane );
    }
  OUT(zipedit_Handle_Font_Family_Selection);
  return status;
  }

int
zipedit_Handle_Font_Height_Selection( class zipedit	      *self, zip_type_pane	       pane, enum view_MouseAction       action, long			       x , long			       y , long			       clicks )
          {
  long			      status = zip_success;
  zip_type_figure	      figure;
  char				      msg[257];

  IN(zipedit_Handle_Font_Height_Selection);
  if ( action == view_LeftDown  ||  action == view_RightDown )
    {
    (self->view_object)->Clear_Pane(  pane );
    figure = (self->data_object)->Figure(  "font_catalog_height" );
    if ( action == view_LeftDown )
      {
      if ( FontHeight < 144 )
	FontHeight += 2;
	else
	FontHeight = 2;
        sprintf( msg, "%2ld", FontHeight );
      (self->data_object)->Set_Figure_Text(  figure, msg);
      }
    else
    if ( action == view_RightDown )
      {
      if ( FontHeight > 2 )
	FontHeight -= 2;
	else
	FontHeight = 144;
        sprintf( msg, "%2ld", FontHeight );
      (self->data_object)->Set_Figure_Text(  figure, msg);
      }
    (self->view_object)->Draw_Figure(  figure, pane );
    }
  OUT(zipedit_Handle_Font_Height_Selection);
  return status;
  }

int
zipedit_Handle_Font_Italic_Selection( class zipedit *self, zip_type_pane	       pane, enum view_MouseAction action , long x , long y , long clicks )
{
  int			      status = zip_success;
  zip_type_figure	      figure;

  IN(zipedit_Handle_Font_Italic_Selection);
  if ( action == (int)view_LeftDown )
    {
    (self->view_object)->Clear_Pane(  pane );
    figure = (self->data_object)->Figure(  "font_catalog_italic" );
    FontItalic = ! FontItalic;
    (self->data_object)->Set_Figure_Font(  figure, (FontItalic) ?
			 "andysans10b" : "andysans10" );
    (self->view_object)->Draw_Figure(  figure, pane );
    }
  OUT(zipedit_Handle_Font_Italic_Selection);
  return status;
  }

int
zipedit_Handle_Font_Bold_Selection( class zipedit	      *self, zip_type_pane	       pane, enum view_MouseAction       action, long			       x , long			       y , long			       clicks )
          {
  long			      status = zip_success;
  zip_type_figure	      figure;

  IN(zipedit_Handle_Font_Bold_Selection);
  if ( action == view_LeftDown )
    {
    (self->view_object)->Clear_Pane(  pane );
    figure = (self->data_object)->Figure(  "font_catalog_bold" );
    FontBold = ! FontBold;
    (self->data_object)->Set_Figure_Font(  figure, (FontBold) ?
			 "andysans10b" : "andysans10" );
    (self->view_object)->Draw_Figure(  figure, pane );
    }
  OUT(zipedit_Handle_Font_Bold_Selection);
  return status;
  }

int
zipedit_Handle_Font_Sample_Selection( class zipedit	      *self, zip_type_pane	       pane, enum view_MouseAction       action, long			       x , long			       y , long			       clicks )
          {
  long			      status = zip_ok, reshow = false;

  IN(zipedit_Handle_Font_Sample_Selection);
  if ( action == view_LeftDown )
    {
    PriorX = x;  PriorY = y;
    Set_Sample( self, pane, false );
    }
  else
    if ( action == view_LeftMovement )
      {
      (self->view_object)->Set_Pane_Cursor(  pane, '@', CursorFontName ); /* Make Cursor disappear */
      if ( x < (PriorX - 5) )
	{
	switch ( FontHorizontal )
	  {
	  case zip_left:    FontHorizontal = zip_center;  break;
	  case zip_center:  FontHorizontal = zip_right;   break;
	  case zip_right:   break;
	  default:	    FontHorizontal = zip_left;    break;
	  }
	PriorX = x;  PriorY = y; reshow = true;
	}
      else  if ( x > (PriorX + 5) )
	{
	switch ( FontHorizontal )
	  {
	  case zip_right:   FontHorizontal = zip_center;  break;
	  case zip_center:  FontHorizontal = zip_left;    break;
	  case zip_left:    break;
	  default:	    FontHorizontal = zip_left;    break;;
	  }
	PriorX = x;  PriorY = y; reshow = true;
	}
      else  if ( y < (PriorY - 5) )
	{
	switch ( FontVertical )
	  {
	  case zip_top :     FontVertical = zip_middle;    break;
	  case zip_middle:   FontVertical = zip_baseline;  break;
	  case zip_baseline: FontVertical = zip_bottom;    break;
	  case zip_bottom:   break;
	  default:	     FontVertical = zip_bottom;    break;
	  }
	PriorX = x;  PriorY = y; reshow = true;
	}
      else  if ( y > (PriorY + 5) )
	{
	switch ( FontVertical )
	  {
	  case zip_bottom:   FontVertical = zip_baseline;  break;
	  case zip_baseline: FontVertical = zip_middle;    break;
	  case zip_middle:   FontVertical = zip_top;       break;
	  case zip_top:      break;
	  default:	     FontVertical = zip_bottom;    break;
	  }
	PriorX = x;  PriorY = y; reshow = true;
	}
      }
  if (  action == view_LeftUp  ||
       ((action == view_LeftMovement)  &&  reshow ) )
    Set_Sample( self, pane, action == view_LeftUp );
  if ( action == view_LeftUp )
    (self->view_object)->Set_Pane_Cursor(  pane, '2', CursorFontName );
  OUT(zipedit_Handle_Font_Sample_Selection);
  return status;
  }

static
void Set_Sample( class zipedit	      *self, zip_type_pane	       pane, boolean		       draw_pane )
        {
  zip_type_figure	      figure;
  char				      msg[257];
  long			      changed = false;

  IN(Set_Sample);
  figure = (self->data_object)->Figure(  "font_catalog_sample" );
  sprintf( msg, "%s%ld%s%s",
	 (FontFamily) ? "andysans" : "andy",
	 FontHeight,
	 (FontBold) ? "b" : "",
	 (FontItalic) ? "i" : "" );
  (self->data_object)->Set_Figure_Font(  figure, msg );
  (self->data_object)->Set_Figure_Mode(  figure, FontHorizontal | FontVertical );
  EditingPane(pane)->zip_pane_current_font = figure->zip_figure_font;
  EditingPane(pane)->zip_pane_current_mode = figure->zip_figure_mode;
  (self->view_object)->Display_Pane(  pane );
  figure = NULL;
  while ( ( figure = zipedit_Next_Selected_Figure( self, EditingPane(pane), figure ) ) )
    if ( Change_Figure_Font_And_Mode( self, EditingPane(pane), figure,
		EditingPane(pane)->zip_pane_current_font, FontHorizontal | FontVertical ) )
      changed = true;
  if ( changed  &&  draw_pane)
    {
    Show_Enclosure( self, EditingPane(pane) );
    (self->view_object)->Draw_Pane(  EditingPane(pane) );
    Show_Enclosure( self, EditingPane(pane) );
    }
  OUT(Set_Sample);
  }

static
boolean Change_Figure_Font_And_Mode( class zipedit		  *self, zip_type_pane		   pane, zip_type_figure		   figure, long				   font , long				   mode )
          {
  boolean			  changed = false;

  IN(Change_Figure_Font_And_Mode);
  (self)->Normalize_Figure_Points(  figure, pane );
  if ( figure->zip_figure_font )
    (self->view_object)->Clear_Figure(  figure, pane );
  if ( (Objects(figure->zip_figure_type))->Set_Object_Font(
	 figure, font ) == zip_ok )
    {
    changed = true;
/*===ADD Set_Object_Mode===*/
if ( figure->zip_figure_type == zip_caption_figure )
(self->data_object)->Set_Figure_Mode(  figure, mode );
    }
  OUT(Change_Figure_Font_And_Mode);
  return  changed;
  }
