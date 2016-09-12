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

/* ziposym.c	Zip Object -- Symbols					      */
/* Author	TC Peters						      */
/* Information Technology Center		   Carnegie-Mellon University */

/**  SPECIFICATION -- Internal Facility Suite  *********************************

TITLE	Zip Object -- Symbols

MODULE	ziposym.c

NOTICE	IBM Internal Use Only

DESCRIPTION

.......................

HISTORY
  07/10/88	Created (TC Peters)
  11/01/88	Optimize (TCP)
  11/16/88	Fix Symbol-set listing (TCP)
  05/01/89	Use symbolic font-names (TCP)
  05/26/89	Accomodate symbol-set name backward-compatibility (TCP)
  07/13/89	Fix non-view (print) usage (TCP)
   09/15/89	Fix symbol set path corruption (copy AndrewDir()  result), remove excess TransferMode activity in Draw() and DrawSymbol, modify Build accordingly (SCG)
  11/20/89	Fixed bogus dereferencing of argument to strlen() in Filter() (GW Keim)

END-SPECIFICATION  ************************************************************/


#include <andrewos.h>
#include <view.H>
#include <environ.H>
#include <text.H>
#include <textview.H>
#include <fontdesc.H>
#include "zip.H"
#include "zipview.H"
#include "zipedit.H"
#include "zipprint.H"
#include "zipobject.H"
#include "ziposymbol.H"
#include <sys/param.h>
#include <sys/stat.h>
#include <ctype.h>
#include <errno.h>
#include <string.h>

static char				 *symbol_library_path = NULL;

static struct symbol_set	         *symbol_sets = NULL;
static long				  symbol_sets_count;

#define  SVL				 (self->symbol_view_left)
#define  SVT				 (self->symbol_view_top)
#define  SVW				 (self->symbol_view_width)
#define  SVH				 (self->symbol_view_height)
#define  SVR				 (self->symbol_view_right)
#define  SVB				 (self->symbol_view_bottom)
#define  SymbolSelected			 (self->symbol_selected)
#define  SelectedSymbolSetName		 (self->selected_symbol_set_name)
#define  SelectedSymbolIndexName	 (self->selected_symbol_index_name)
#define  SelectedSymbolSet		 (self->selected_symbol_set)
#define  SelectedSymbolSetIndex		 (self->selected_symbol_set_index)
#define  SelectedSymbol			 (self->selected_symbol)
#define  SymbolSetDisplayed		 (self->symbol_set_displayed)
#define  MaxSymbolSetsCount		 100
#define  SymbolSets			 (symbol_sets)
#define  SymbolLibraryPath		 (symbol_library_path)
#define  SymbolLibraries		 (self->library_paths)
#define  CurrentSymbolSet		 (self->current_symbol_set)
#define  CurrentSymbolSetIndex		 (self->current_symbol_set_index)
#define  GrayShade			 (self->gray_shade)
#define  GrayGraphic			 (self->gray_graphic)
#define  LineWidth			 (self->line_width)
#define  LineStyle			 (self->line_style)
#define  LastNumber			 (self->last_number)
#define  OutstandingSurround		 (self->outstanding_surround)
#define  OutstandingLeft		 (self->outstanding_left)
#define  OutstandingTop			 (self->outstanding_top)
#define  OutstandingWidth		 (self->outstanding_width)
#define  OutstandingHeight		 (self->outstanding_height)


ATKdefineRegistry(ziposymbol, ziporect, NULL);

static long Draw( class ziposymbol *self, zip_type_figure figure, zip_type_pane pane );
static long Draw_Symbol( class ziposymbol *self, zip_type_figure figure, zip_type_pane pane, char *algorithm, zip_type_pixel left , zip_type_pixel top , zip_type_pixel width , zip_type_pixel height,  long x_factor , long y_factor, boolean print );
static long Identify_Symbol_Sets( class ziposymbol *self );
static int Identify_Pathed_Symbol_Sets( class ziposymbol *self, const char *path );
static struct symbol_set * Symbol_Set( class ziposymbol *self, char	  *set_name );
static struct symbol * Symbol_Set_Vector( class ziposymbol *self, char  *set_name );
static long Open_Symbol_Set_File( class ziposymbol *self, struct symbol_set*set );
static long Open_File( class ziposymbol *self, struct symbol_set *set );
static int Identify_Paths( class ziposymbol *self, zip_type_paths *paths_ptr );
static char * Symbol_Algorithm( class ziposymbol *self, zip_type_figure figure );
static void Highlight_Set_Name( class ziposymbol *self, zip_type_pane pane, struct symbol_set *set );
static void Draw_Set_Name( class ziposymbol *self, zip_type_pane pane, struct symbol_set *set, class fontdesc *font );
static long Show_Symbol_Dialog( class ziposymbol *self, zip_type_pane pane );
static void Show_Set_Symbols( class ziposymbol *self, zip_type_pane pane, struct symbol_set *set );
static void Highlight_Symbol( class ziposymbol *self, struct symbol_set *set, struct symbol *symbol );
static void Invert_Symbol( class ziposymbol *self, struct symbol *symbol );
static enum view_MouseAction Accept_Property_Hit( class ziposymbol *self, zip_type_pane pane, char c, enum view_MouseAction	   action, long x , long y , long clicks );
static void Decline_Property_Hits( class ziposymbol *self, zip_type_pane pane );
static char * Skip_Colon( char *string );
static char * String( class ziposymbol *self, char *string, char **s );
static char * Number( class ziposymbol *self, char *string, long *n );
static char * Pixel( class ziposymbol	 *self, char *string, long *x , long *y, double M , double D , double XO , double YO, long x_factor , long	 y_factor );


ziposymbol::ziposymbol( )
      {
  class ziposymbol *self=this;

  IN(ziposymbol_InitializeObject);
  SymbolSelected = OutstandingSurround = false;
  *SelectedSymbolSetName = 0;
  *SelectedSymbolIndexName = 0;
  SelectedSymbolSet = CurrentSymbolSet = NULL;
  SelectedSymbolSetIndex = CurrentSymbolSetIndex = 0;
  SelectedSymbol = NULL;
  SymbolSetDisplayed = NULL;
  SymbolLibraries = NULL;
  GrayShade = 0;
  LineStyle = 0;
  LineWidth = 1;
  LastNumber = 0;
  if ( symbol_library_path == NULL )
  {
    const char *p = environ::AndrewDir( "/lib/zip/symbols" );
    if ( p )
	symbol_library_path = strdup( p );
  }
  DEBUGst(LibPath,symbol_library_path);
  OUT(ziposymbol_InitializeObject);
  THROWONFAILURE(  true);
  }

ziposymbol::~ziposymbol( )
      {
  long				  i;
  struct symbol_set		 *sets = SymbolSets;

  IN(ziposymbol_FinalizeObject);
  if ( SymbolSets )
    {
    while ( sets->set_name )
      {
      if ( sets->set_file )
	fclose( sets->set_file );
      if ( sets->set_symbols )
        for ( i = 0; i < sets->set_symbols_count; i++ )
	  {
	  if ( sets->set_symbols[i].symbol_icon )
            free( sets->set_symbols[i].symbol_icon );
  	  if ( sets->set_symbols[i].symbol_name )
            free( sets->set_symbols[i].symbol_name );
	  if ( sets->set_symbols[i].symbol_algorithm )
            free( sets->set_symbols[i].symbol_algorithm );
	  }
      free( sets->set_symbols );
      sets++;
      }
    free( SymbolSets );
    }
  OUT(ziposymbol_FinalizeObject);
  }

char
ziposymbol::Object_Icon( )
    {
  IN(ziposymbol_Object_Icon);
  OUT(ziposymbol_Object_Icon);
  return  'Q';
  }

char
ziposymbol::Object_Icon_Cursor( )
    {
  IN(ziposymbol_Object_Icon_Cursor);
  OUT(ziposymbol_Object_Icon_Cursor);
  return  'B';
  }

char
ziposymbol::Object_Datastream_Code( )
    {
  IN(ziposymbol_Object_Datastream_Code);
  OUT(ziposymbol_Object_Datastream_Code);
  return  'P';
  }

long
ziposymbol::Show_Object_Properties( zip_type_pane		   pane, zip_type_figure		   figure )
        {
  long				  status;

  IN(ziposymbol_Show_Object_Properties);
  status = Show_Symbol_Dialog( this, pane );
  OUT(ziposymbol_Show_Object_Properties);
  return  status;
  }

long
ziposymbol::Build_Object( zip_type_pane pane, enum view_MouseAction action, long x , long y , long clicks, zip_type_point X , zip_type_point Y )
{
  class ziposymbol		 *self=this;
  long				  status = zip_ok;
  zip_type_figure		  figure;
  char				  symbol_string[257];

  IN(ziposymbol_Build_Object);
  switch ( action )
    {
    case view_LeftDown:
      if ( SelectedSymbol )
	{
        sprintf( symbol_string, "%s;%s", SelectedSymbolSetName, SelectedSymbolIndexName );
        DEBUGst(Symbol-string,symbol_string);
        if ( (status =
          (this->data_object)->Create_Figure(  &CurrentFigure, NULL, zip_symbol_figure,
			 CurrentImage, NULL )) == zip_success )
	  {
	  (this->data_object)->Set_Figure_Text(  CurrentFigure, symbol_string );
          (this)->Set_Object_Point(  CurrentFigure, zip_figure_origin_point, X, Y );
          (this)->Set_Object_Point(  CurrentFigure, zip_figure_auxiliary_point, X, Y );
	  CurrentFigure->zip_figure_zoom_level = pane->zip_pane_zoom_level;
          pane->zip_pane_edit->zip_pane_edit_last_point_id = zip_figure_auxiliary_point;
	  (this->data_object)->Set_Figure_Shade(  CurrentFigure,
			        pane->zip_pane_edit->zip_pane_edit_current_shade );
	  (this->view_object)->Set_Pane_Painting_Mode(  pane, zipview_paint_inverted );
	  }
	}
	else  (this->view_object)->Announce(  "No Symbol Selected." );
      break;
    case view_LeftUp:
      (this->view_object)->Set_Pane_Cursor(  pane, 'B', CursorFontName ); /*=== ?? ===*/
      if ( SelectedSymbol  &&  (figure = CurrentFigure) )
	{
	(this->view_object)->Set_Pane_Painting_Mode(  pane, zip_default );
	if ( figure_x_point == figure_x_points(0)  &&
	     figure_y_point == figure_y_points(0) )
	  {
	  (this->edit_object)->Delete_Figure(  figure, pane ); 
	  }
	  else (this->view_object)->Draw_Figure(  CurrentFigure, pane );
	}
        break;
    case view_LeftMovement:
      if ( SelectedSymbol  &&  CurrentFigure )
	{
	(this->view_object)->Draw_Figure(  CurrentFigure, pane );
        (this)->Set_Object_Point(  CurrentFigure, zip_figure_auxiliary_point, X, Y );
	(this->view_object)->Draw_Figure(  CurrentFigure, pane );
	}
      break;
    default:
      break;
    }
  OUT(ziposymbol_Build_Object);
  return  status;
  }

long
ziposymbol::Draw_Object( zip_type_figure		   figure, zip_type_pane		   pane )
        {
  long				  status = zip_ok;

  IN(ziposymbol_Draw_Object);
  if ( (this->view_object)->Condition(  pane, figure, zip_draw ) )
    status = Draw( this, figure, pane );
  OUT(ziposymbol_Draw_Object);
  return  status;
  }

long
ziposymbol::Clear_Object( zip_type_figure		   figure, zip_type_pane		   pane )
        {
  long				  status = zip_ok;

  IN(ziposymbol::Clear_Object);
  if ( (this->view_object)->Condition(  pane, figure, zip_clear ) )
    status = Draw( this, figure, pane );
  OUT(ziposymbol::Clear_Object);
  return  status;
  }

static
long Draw( class ziposymbol *self, zip_type_figure figure, zip_type_pane  pane )
 {
  long				  status = zip_ok;
  short			  left = window_x_point, top = window_y_point,
					  width = window_x_points(0) - left,
					  height = window_y_points(0) - top,
					  x_factor = 1, y_factor = 1,
					  transfer_mode;
  unsigned char		  shade;
  char				 *algorithm;

  IN(Draw);
  if ( width < 0 )
    { DEBUG(X-Flipped);
    left = window_x_points(0);
    width = -width;
    x_factor = -1;
    }
  if ( height < 0 )
    { DEBUG(Y-Flipped);
    top = window_y_points(0);
    height = -height;
    y_factor = -1;
    }
  (self->view_object)->Set_Pane_Clip_Area(  pane );
  transfer_mode = (self->view_object )->GetTransferMode( );
  if ( figure->zip_figure_mode.zip_figure_mode_shaded  &&
       self->view_object->mouse_action != view_LeftMovement  &&
      (shade = figure->zip_figure_fill.zip_figure_shade) >= 1  &&
          shade <= 100 )
    {
    /* Shade of '0' means Transparent --- Shade of '1' means White */
    DEBUGdt(Shade,figure->zip_figure_fill.zip_figure_shade);
    if ( (shade = ('0' + ((shade + 10) / 10)) - 1) > '9' )  shade = '9';
    DEBUGdt(Shade-index,shade);
/*    if ( transfer_mode != graphic_COPY )
      zipview_SetTransferMode( self->view_object, graphic_COPY ); */
    (self->view_object)->FillTrapezoid(  left, top, width, left, top + height, width,
	    (self->view_object)->Define_Graphic( 
		(self->data_object)->Define_Font(  ShadeFontName, NULL ), shade ) );
    }
  if ( OutstandingSurround )
    {
    OutstandingSurround = false;
    if ( transfer_mode != graphic_INVERT )
      (self->view_object)->SetTransferMode(  graphic_INVERT );
    (self->view_object)->DrawRectSize(  OutstandingLeft, OutstandingTop, OutstandingWidth, OutstandingHeight );
    }
  if ( self->view_object->mouse_action == view_LeftMovement )
    {
    OutstandingSurround = true;
    if ( transfer_mode != graphic_WHITE )
      (self->view_object)->SetTransferMode(  graphic_WHITE );
    (self->view_object)->EraseRectSize(  left-1, top-1, width+2, height+2 );
    (self->view_object)->SetTransferMode(  graphic_INVERT );
    (self->view_object)->DrawRectSize(  OutstandingLeft = left, OutstandingTop = top,
				OutstandingWidth = width, OutstandingHeight = height );
    }
    else
    {
    if ( ( algorithm = Symbol_Algorithm( self, figure ) ) )
      {
      if ( (self->view_object )->GetTransferMode( ) != transfer_mode )
        (self->view_object)->SetTransferMode(  transfer_mode );
      Draw_Symbol( self, figure, pane, algorithm,
	         left, top, width, height, x_factor, y_factor, false );
      }
      else  status = zip_failure;
    }
  (self->view_object)->Set_Pane_Clip_Area(  pane );
  if ( ExposePoints )	    (self)->Expose_Object_Points(  figure, pane );
  if ( HighlightPoints )    (self)->Highlight_Object_Points(  figure, pane );
  OUT(Draw);
  return  status;
  }

long
ziposymbol::Print_Object( zip_type_figure figure, zip_type_pane pane )
{
  class ziposymbol *self=this;
  long				  status = zip_ok;
  long				  left, top, width, height,
					  x_factor = 1, y_factor = 1;
  char					 *algorithm;

  IN(ziposymbol::Print_Object);
  left   = print_x_point;		top    = print_y_point;
  width  = print_x_points(0) - left;	height = print_y_points(0) - top;
  if ( figure->zip_figure_mode.zip_figure_mode_shaded )
    {
    DEBUGdt(Shade,figure->zip_figure_fill.zip_figure_shade);
    (this->print_object)->Set_Shade(  figure->zip_figure_fill.zip_figure_shade );
    }
    else (this->print_object)->Set_Shade(  0 /* Transparent */ );
  if ( width < 0 )
    { DEBUG(X-Flipped);
    left = print_x_points(0);
    width = -width;
    x_factor = -1;
    }
  if ( height < 0 )
    { DEBUG(Y-Flipped);
    top = print_y_points(0);
    height = -height;
    y_factor = -1;
    }
  if ( ( algorithm = Symbol_Algorithm( this, figure ) ) )
    Draw_Symbol( this, figure, pane, algorithm, left, top, width, height,
	       x_factor, y_factor, true );
    else  status = zip_failure;
  OUT(ziposymbol::Print_Object);
  return  status;
  }

static
long Draw_Symbol( class ziposymbol  *self, zip_type_figure  figure, zip_type_pane  pane, char *algorithm, zip_type_pixel left , zip_type_pixel  top , zip_type_pixel  width , zip_type_pixel  height, long x_factor , long y_factor, boolean  print )
{
  long				  status = zip_ok, level, mode = 0;
  long					  x, y, n, m, q, r;
  long				  current_x = 0, current_y = 0,
					  start_x, start_y, end_x, end_y,
					  x_diff, y_diff,
					  start_angle = 0, offset_angle;
  char				  operator_c, secondary_operator = 0, shade;
  double			  M, D, XO, YO;
  struct rectangle			  rectangle;
  char				 *buffer_ptr;
  char					  buffer[32000];
  class fontdesc		 *font/*=== = (struct fontdesc *)
					    zip_Define_Font( self->data_object, "andysans10", NULL )===*/;

  IN(Draw_Symbol);
  DEBUGdt(Left,left);  DEBUGdt(Top,top);
  DEBUGdt(Width,width);DEBUGdt(Height,height);
  if ( left   > (x = (self->view_object)->Pane_Left(  pane )) )		x = left;
  if ( top    > (y = (self->view_object)->Pane_Top(  pane )) )		y = top;
  if ( width  < (m = (self->view_object)->Pane_Right(  pane )  - x) )	m = width;
  if ( height < (n = (self->view_object)->Pane_Bottom(  pane ) - y) )	n = height;
  if ( !print )
    (self->view_object)->Set_Clip_Area(  pane, x-1, y-1, m+2, n+2 );
  M = width / 2000.0;  D = height / 2000.0;
  DEBUGgt(M,M);    DEBUGgt(D,D);
  XO = left + width / 2.0; YO = top + height / 2.0;
  DEBUGgt(XO,XO);    DEBUGgt(YO,YO);
  *buffer = 0;
  GrayGraphic = NULL;
  GrayShade = LineStyle = 0;
  LineWidth = 1;
  if ( print )
    (self->print_object)->Set_Line_Width(  1 );
   else   if ( (self->view_object )->GetLineWidth( ) != LineWidth )
    (self->view_object)->SetLineWidth(  LineWidth );
  while ( ( operator_c = *algorithm ) )
    {
    DEBUGct(Operator,operator_c);
    if ( operator_c != '(' )
      {
      secondary_operator = *(algorithm+1);
      DEBUGct(Secondary-Operator,secondary_operator);
      algorithm = Skip_Colon( algorithm );
      if  ( operator_c != 'U' )
        algorithm = Pixel( self, algorithm, &x, &y, M, D, XO, YO, x_factor, y_factor );
      }
    switch( operator_c )
      {
      case 'A':  DEBUG(ArcTo);
	algorithm = Number( self, algorithm, &n );
	DEBUGdt(Mode,n);
	switch ( n )
	  {
	  case 1: n = (y_factor < 0) ?
		      ((x_factor < 0) ? 3 : 2) :
		      ((x_factor < 0) ? 4 : 1); break;
	  case 2: n = (y_factor < 0) ?
		      ((x_factor < 0) ? 4 : 1) :
		      ((x_factor < 0) ? 3 : 2); break;
	  case 3: n = (y_factor < 0) ?
		      ((x_factor < 0) ? 1 : 4) :
		      ((x_factor < 0) ? 2 : 3); break;
	  case 4: n = (y_factor < 0) ?
		      ((x_factor < 0) ? 2 : 3) :
		      ((x_factor < 0) ? 1 : 4); break;
	  }
	rectangle.width  = 2 * abs(x - current_x);
	rectangle.height = 2 * abs(y - current_y);
	offset_angle = 90;
	x_diff = abs(current_x - x);  y_diff = abs(current_y - y);
	start_x = current_x; start_y = current_y; end_x = x; end_y = y;
	switch ( n )
	  {
	  case 1:
	    if ( y < current_y )
	      {
	      rectangle.left   = x - x_diff;
	      rectangle.top    = y;
	      start_x = x; start_y = y; end_x = current_x; end_y = current_y;
	      }
	      else
	      {
	      rectangle.left   = current_x - x_diff;
	      rectangle.top    = current_y;
	      }
	    start_angle = 0;
	    break;
	  case 2:
	    if ( y < current_y )
	      {
	      rectangle.left   = current_x - x_diff;
	      rectangle.top    = y - y_diff;
	      start_x = x; start_y = y; end_x = current_x; end_y = current_y;
	      }
	      else
	      {
	      rectangle.left   = x - x_diff;
	      rectangle.top    = current_y - y_diff;
	      }
	    start_angle = 90;
	    break;
	  case 3:
	    if ( y < current_y )
	      {
	      rectangle.left   = x;
	      rectangle.top    = y - y_diff;
	      }
	      else
	      {
	      rectangle.left   = current_x;
	      rectangle.top    = current_y - y_diff;
	      start_x = x; start_y = y; end_x = current_x; end_y = current_y;
	      }
	    start_angle = 180;
	    break;
	  case 4:
	    if ( y < current_y )
	      {
	      rectangle.left   = current_x;
	      rectangle.top    = y;
	      }
	      else
	      {
	      rectangle.left   = x;
	      rectangle.top    = current_y;
	      start_x = x; start_y = y; end_x = current_x; end_y = current_y;
	      }
	    start_angle = 270;
	    break;
	  default:
	    DEBUGct(Default -- ERROR,*algorithm);
	  }
	if ( print )
	  {
	  (self->print_object)->Draw_Arc( 
		   rectangle.left + rectangle.width/2,
		   rectangle.top  + rectangle.height/2,
		   rectangle.width/2, rectangle.height/2,
		   start_x, start_y, end_x, end_y );
	  (self->print_object)->Move_To(  x, y );
	  }
	  else
	  {
	  (self->view_object)->DrawArc(  &rectangle, start_angle, offset_angle );
	  (self->view_object)->MoveTo(  x, y );
	  }
	break;
      case 'B':  DEBUG(DrawBox);
	algorithm = Pixel( self, algorithm, &m, &n, M, D, XO, YO, x_factor, y_factor );
	if ( print )
	  {
	  (self->print_object)->Set_Shade(  GrayShade );
	  (self->print_object)->Draw_Rectangle(  x, y, m, n );
	  }
	  else
	  {
	  if ( GrayGraphic )
	    {
/*	    if ( zipview_GetTransferMode( self->view_object ) != graphic_COPY )
	      zipview_SetTransferMode( self->view_object, graphic_COPY ); */
	    (self->view_object)->FillTrapezoid(  x+1, y+1, abs(m - x)-1,
					 x+1, n-1, abs(m - x)-1, GrayGraphic );
	    }
/*	  if ( zipview_GetTransferMode( self->view_object ) != graphic_BLACK )
	    zipview_SetTransferMode( self->view_object, graphic_BLACK ); */
	  (self->view_object)->DrawRectSize(  x, y, abs(m - x), abs(n - y) );
	  }
	break;
      case 'C':  DEBUG(DrawCircle);
        algorithm = Number( self, algorithm, &n );
	n = (long) (n * M);
	DEBUGdt(Radius,n);
	if ( n > 0 ) {
	  if ( print )
	    (self->print_object)->Draw_Circle(  x, y, n );
	    else
	    (self->view_object)->DrawOvalSize(  x - n, y - n, n<<1, n<<1 );
	}
	break;
      case 'D':  DEBUG(DrawTo);
	if ( print )
	  (self->print_object)->Draw_Line(  current_x, current_y, x, y );
	  else
	  (self->view_object)->DrawLineTo(  x, y );
	break;
      case 'E':  DEBUG(DrawEllipse);
        algorithm = Number( self, algorithm, &n );
	n = (long) (n * M);
	DEBUGdt(X-Radius,n);
        algorithm = Number( self, algorithm, &m );
	m = (long) (m * D);
	DEBUGdt(Y-Radius,m);
	if ( m > 0  &&  n > 0 ) {
	  if ( print )
	    (self->print_object)->Draw_Ellipse(  x, y, n, m );
	    else
	    {
	    if ( GrayGraphic )
	      (self->view_object)->FillOvalSize(  x-n, y-m, n<<1, m<<1, GrayGraphic );
	    (self->view_object)->DrawOvalSize(  x-n, y-m, n<<1, m<<1 );
	    }
	}
	break;
      case 'F':  DEBUG(SetFont);
	algorithm++;
	buffer_ptr = buffer;
	while ( *algorithm )
	  if ( *algorithm == '"' )   break;
	    else   *buffer_ptr++ = *algorithm++;
	*buffer_ptr = 0;
	if ( *algorithm == '"' )  algorithm++;
	if ( *algorithm == ';' )  algorithm++;
	DEBUGst(Font-name,buffer);
	font = (class fontdesc *) (self->data_object)->Define_Font(  buffer, NULL );
	if ( print )
	  (self->print_object)->Change_Font(  font );
	  else
          (self->view_object)->SetFont(  font );
	break;
      case 'G':  DEBUG(GrayGraphic);
	GrayShade = LastNumber;
	DEBUGdt(GrayShade,GrayShade);
	if ( !print ) {
	 if ( GrayShade > 0 &&  GrayShade <= 100 )
	  {
	  if ( (shade = ('0' + ((GrayShade + 10) / 10)) - 1) > '9' )  shade = '9';
	  DEBUGct(Shade,shade);
	  GrayGraphic = (self->view_object)->Define_Graphic( 
			(self->data_object)->Define_Font(  ShadeFontName, NULL ), shade );
	  }
	  else
	  {
	  GrayGraphic = NULL;
	  GrayShade = 0;
	  }
	}
	break;
      case 'I':  DEBUG(Incorporate);
	{
	struct symbol		 *vector;
	struct symbol_set	 *css = CurrentSymbolSet;
	long			  cssi = CurrentSymbolSetIndex;
	char			 *set_name = CurrentSymbolSet->set_name, *bufptr = buffer;

	algorithm = Pixel( self, algorithm, &m, &n, M, D, XO, YO, x_factor, y_factor );
	algorithm = String( self, algorithm, &bufptr );
	algorithm = Number( self, algorithm, &q );
	vector = Symbol_Set_Vector( self, set_name );
	Draw_Symbol( self, figure, pane, vector[q].symbol_algorithm,
		     x, y, m-x, n-y, x_factor, y_factor, print );
	CurrentSymbolSet = css; CurrentSymbolSetIndex = cssi;
	break;
	}
      case 'L':  DEBUG(Set-Line...);
	if ( secondary_operator == 'W' )
	  { DEBUG(...Width);
	  LineWidth = LastNumber;
	  DEBUGdt(LineWidth,LineWidth);
	  if ( print )
	    {
	    (self->print_object)->Set_Line_Width(  LineWidth ); 
	    }
	    else
	    {
	    if ( (self->view_object )->GetLineWidth( ) != LineWidth )
		(self->view_object)->SetLineWidth(  LineWidth );
	    }
	  }
	else
	if ( secondary_operator == 'S' )
	  { DEBUG(...Style);
	  if ( print )
	    {
/*===*/
	    }
	    else
	    {
/*===*/
	    }
	  }
	break;
      case 'M':  DEBUG(MoveTo);
	if ( print )
	  (self->print_object)->Move_To(  x, y );
	  else
	  (self->view_object)->MoveTo(  x, y );
	break;
      case 'R':  DEBUG(DrawRoundBox);
	algorithm = Pixel( self, algorithm, &m, &n, M, D, XO, YO, x_factor, y_factor );
	algorithm = Number( self, algorithm, &q );
	algorithm = Number( self, algorithm, &r );
	/*===q = r = 5;===*/
	if ( print )
	  (self->print_object)->Draw_Round_Rectangle(  x, y, m, n, q*100, r*100 );
	  else
	  (self->view_object)->DrawRRectSize(  x, y, m - x, n - y, q, r );
	break;
      case 'S':  DEBUG(DrawString);
	algorithm++;
	buffer_ptr = buffer;
	while ( *algorithm )
	  {
	  if ( *algorithm == '"' )
	    break;
	    else   *buffer_ptr++ = *algorithm++;
          }
	*buffer_ptr = 0;
	if ( *algorithm == '"' )  algorithm++;
	if ( *algorithm == ';' )  algorithm++;
	DEBUGst(Stack,buffer);
	if ( print )
	  {
/*===	  zipprint_Change_Font( Print, font );===*/
	  (self->print_object)->Draw_String(  x, y, buffer, mode );
/*===	  zipprint_Restore_Font( self->print_object ); ===*/
	  }
	  else
	  {
	  (self->view_object)->MoveTo(  x, y );
/*===     zipview_SetFont( self->view_object, font );===*/
	  (self->view_object)->DrawString(  buffer,
	    graphic_BETWEENLEFTANDRIGHT | graphic_BETWEENTOPANDBOTTOM );
	  }
	break;
      case '(':  DEBUG(Parenthsize);
	level = 0;
	algorithm++;
	buffer_ptr = buffer;
	while ( *algorithm )
	  {
	  if ( *algorithm == '(' )  level++;
	    else if ( *algorithm == ')' )  level--;
	  if ( level >= 0 )
	    *buffer_ptr++ = *algorithm++;
	    else  break;
          }
	*buffer_ptr = 0;
	if ( *algorithm == ')' )  algorithm++;
	DEBUGst(Stack,buffer);
	break;
      case 'U':  DEBUG(Utilize);
	if ( *buffer )
	  {
          algorithm = Number( self, algorithm, &m );
          algorithm = Number( self,  algorithm, &n );
	  Draw_Symbol( self, figure, pane, buffer, left, top, width, height,
		       m*x_factor, n*y_factor, print );
	  }
	break;
      default:  DEBUGct(Default -- ERROR,*algorithm);
	algorithm++;
	break;
      }
    current_x = x;  current_y = y;
    }
  if ( !print )
    {
    (self->view_object)->Reset_Pane_Clip_Area(  pane );
    if ( (self->view_object )->GetLineWidth( ) != 1 )
      (self->view_object)->SetLineWidth(  1 );
    }
  OUT(Draw_Symbol);
  return  status;
  }
  /****************************************************************/

static
boolean Filter( SCANDIRSELARG_TYPE *entry )
    {
  const char	 *end = entry->d_name + strlen(entry->d_name);

  return ( !(*entry->d_name == '.'  &&
	     (*(entry->d_name+1) == '.'  ||  *(entry->d_name+1) == '\0')) &&
	   !((end - entry->d_name) > 3  &&
	     ((*(end-3) == 'B'  &&  *(end-2) == 'A'  &&  *(end-1) == 'K')  ||
	      (*(end-3) == 'C'  &&  *(end-2) == 'K'  &&  *(end-1) == 'P') ) ) );
  }

static
long Identify_Symbol_Sets( class ziposymbol		  *self )
    {
  long				  status = zip_ok, i;

  IN(Identify_Symbol_Sets);
  if ( SymbolSets == NULL )
    {
    if ( SymbolLibraries == NULL )
      {
      if ( (status = Identify_Paths( self, &SymbolLibraries )) == zip_ok)
        SymbolSets = (struct symbol_set *)
		    calloc( 1, (MaxSymbolSetsCount+1) * sizeof(struct symbol_set) );
      }
    if ( status == zip_ok )
      for ( i = 0; i < SymbolLibraries->zip_paths_count; i++ )
	{
        status = Identify_Pathed_Symbol_Sets( self, SymbolLibraries->zip_paths_vector[i] );
	}
    }
  DEBUGdt(Status,status);
  OUT(Identify_Symbol_Sets);
  return  status;
  }

static int
Identify_Pathed_Symbol_Sets( class ziposymbol		  *self, const char				  *path )
      {
  long				  status = zip_ok, count, i, j;
  char					  msg[1025];
  DIRENT_TYPE				**anchor;
  struct stat				  stats;

  IN(Identify_Pathed_Symbol_Sets);
  DEBUGst(Path,path);
  sprintf( msg, "Accessing Directory '%s'", path );
  (self->view_object)->Announce(  msg );
  if ( stat( path, &stats ) )
    { DEBUG(Accessing ERROR);
    status = zip_failure;
    sprintf( msg, "ERROR Accessing Directory '%s'", path );
    (self->view_object)->Announce(  msg );
    }
    else
    {
    if ( (count = scandir( path, &anchor, SCANDIRSELFUNC(Filter), SCANDIRCOMPFUNC(alphasort) )) < 0 )
      { DEBUG(Scanning ERROR);
      status = zip_failure;
      sprintf( msg, "ERROR Scanning Directory '%s'", path );
      (self->view_object)->Announce(  msg );
      }
      else
      {
      (self->view_object)->Announce(  "Done." );
      DEBUGdt(Count,count);
      if ( count )
	{
	if ( SymbolSets )
	  {
	  for ( i = 0, j = symbol_sets_count;
		i < count  &&  ((i + j) < MaxSymbolSetsCount);
		i++ )
	    {
	    SymbolSets[i+j].set_name = anchor[i]->d_name;
	    DEBUGst(Set-name,SymbolSets[i+j].set_name);
	    SymbolSets[i+j].set_path = path;
	    }
	  symbol_sets_count += count;
	  }
	  else
	  { DEBUG(ERROR: SymbolSets Unallocated);
/*===*/
	  }
	}
      }
    }
  OUT(Identify_Pathed_Symbol_Sets);
  return  status;
  }

static struct symbol_set *
Symbol_Set( class ziposymbol		  *self, char				  *set_name )
      {
  struct symbol_set		 *set = NULL, *sets = SymbolSets;

  IN(Symbol_Set);
  if ( sets == NULL )
    { DEBUG(Not Initialized);
/*===*/
    }
  while ( sets->set_name )
    { DEBUGst(Set-name,sets->set_name);
    if ( strcmp( sets->set_name, set_name ) == 0 )
      { DEBUG(Found Set);
      set = sets;
      break;
      }
    sets++;
    }
  OUT(Symbol_Set);
  return  set;
  }

static struct symbol *
Symbol_Set_Vector( class ziposymbol		  *self, char				  *set_name )
      {
  struct symbol		 *symbol_vector = NULL;
  struct symbol_set		 *sets = SymbolSets;

  IN(Symbol_Set_Vector);
  if ( sets == NULL )
    { DEBUG(Not Initialized);
/*===*/
    }
  while ( sets->set_name )
    { DEBUGst(Set-name,sets->set_name);
    if ( strcmp( sets->set_name, set_name ) == 0 )
      { DEBUG(Found Set);
      if ( sets->set_file == NULL )
	{ DEBUG(Set-file Not Open);
	if ( Open_Symbol_Set_File( self, sets ) )
	  { DEBUG(Open ERROR);
/*===*/
	  break;
	  }
	}
      symbol_vector = sets->set_symbols;
      break;
      }
    sets++;
    }
  OUT(Symbol_Set_Vector);
  return  symbol_vector;
  }

static
long Open_Symbol_Set_File( class ziposymbol		  *self, struct symbol_set		  *set )
      {
  long				  status = zip_ok,
					  length, count = 0;
  char					  buffer[8193];
  boolean			  continuation = false;

  IN(Open_Symbol_Set_File);
  if ( Open_File( self, set ) == zip_ok )
    { DEBUG(Open OK);
    set->set_symbols = (struct symbol *)
	calloc( 256, sizeof(struct symbol) );
/*===use grits===*/
    while ( fgets( buffer, sizeof buffer - 1, set->set_file ) )
      {
      length = strlen( buffer );
      if ( buffer[length-1] == '\n' )
	buffer[length-1] = 0;
      DEBUGst(Buffer,buffer);
      if ( *buffer != '#'  ||  *buffer == '\n' )
	{
	if ( continuation )
	  {
	  set->set_symbols[count].symbol_algorithm = (char *)
	    realloc( set->set_symbols[count].symbol_algorithm,
		     strlen( set->set_symbols[count].symbol_algorithm ) +
		      length+1 );
	  strcat( set->set_symbols[count].symbol_algorithm, buffer );
	  }
	  else
	  {
          set->set_symbols[++count].symbol_algorithm = (char *)
	    malloc( length+1 );
          strcpy( set->set_symbols[count].symbol_algorithm, buffer );
	  }
	if ( buffer[length-2] == '\\' )
	  {
	  set->set_symbols[count].
	    symbol_algorithm[strlen(set->set_symbols[count].symbol_algorithm)-1] = 0;
	  continuation = true;
	  }
	  else
	  continuation = false;
	}
      DEBUGst(Algorithm,set->set_symbols[count].symbol_algorithm);
      }
    set->set_symbols_count = count;
    (self->view_object)->Announce(  "Done." );
/*===use grits===*/
    }
    else
    { DEBUG(Open ERROR);
    status = zip_failure;
    sprintf( buffer, "ERROR Accessing '%s/%s': %s",
	     set->set_path, set->set_name, strerror(errno) );
    (self->view_object)->Announce(  buffer );
    }
  OUT(Open_Symbol_Set_File);
  return  status;
  }

static
long Open_File( class ziposymbol		  *self, struct symbol_set		  *set )
      {
  long				  i, status = zip_failure;
  char					  buffer[512];

  IN(Open_File);
  DEBUGst(Set-name,set->set_name);
  sprintf( buffer, "Accessing '%s/%s'", set->set_path, set->set_name ); 
  DEBUGst(File-name,buffer);
  (self->view_object)->Announce(  buffer );
  sprintf( buffer, "%s/%s", set->set_path, set->set_name );
  errno = 0;
  if ( ( set->set_file = fopen( buffer, "r" ) ) )
    status = zip_ok;
    else
    {
    if ( errno == ENOENT )
      {
      status = -1;
      for ( i = 0; status == -1  &&  i < SymbolLibraries->zip_paths_count; i++ )
	{
	errno = 0;
	sprintf( buffer, "%s/%s", SymbolLibraries->zip_paths_vector[i], set->set_name );
	DEBUGst(Trying,buffer);
	if ( ( set->set_file = fopen( buffer, "r" ) ) )
	  { DEBUG(Success);
	  status = zip_ok;
	  break;
	  }
	if ( status == -1 )
	  status = zip_system_status_value_boundary + errno;
	}
      }
      else  status = zip_system_status_value_boundary + errno;
    }
  DEBUGdt(Status,status);
  OUT(Open_File);
  return  status;
  }

static int
Identify_Paths( class ziposymbol	      *self, zip_type_paths	      *paths_ptr )
      {
  long			      new_path = 1, status = zip_ok;
  char				     *zippath_profile, *zippath_string;

  IN(Identify_Paths);
  if ( (*paths_ptr = (zip_type_paths)
        calloc( 1, sizeof(struct zip_paths) + (10 * sizeof(zip_type_path)) )) != NULL )
    {
    (*paths_ptr)->zip_paths_count = 0;
    if ( (zippath_profile = (char *) environ::GetProfile( "ZipSymbolPaths" ))  ||
         (zippath_profile = (char *) environ::GetProfile( "ZipSymbolPath" )) )
      {
      if ( ( zippath_string = strdup( zippath_profile ) ) )
        {
	DEBUGst(Path-string,zippath_string);
        while ( *zippath_string != '\0'  &&  (*paths_ptr)->zip_paths_count < 10 )
          {
          if ( new_path  &&  *zippath_string != ':' )
            {
            (*paths_ptr)->zip_paths_vector[(*paths_ptr)->zip_paths_count++] =
              zippath_string;
	    new_path = false;
	    }
          if ( *zippath_string  == ':' )
            {
	    *zippath_string = 0;
	    DEBUGst(Path,zippath_string);
            new_path = true;
            }
          zippath_string++;
	  }
	}
	else status = zip_insufficient_stream_space;
      }
    if ( ( (*paths_ptr)->zip_paths_vector[(*paths_ptr)->zip_paths_count] =
         strdup( symbol_library_path ) ) )
    {
      (*paths_ptr)->zip_paths_count++;
    }
      else status = zip_insufficient_stream_space;
    }
    else status = zip_insufficient_stream_space;
  DEBUGdt(Status,status);
  OUT(Identify_Paths);
  return status;
  }

static char *
Symbol_Algorithm( class ziposymbol		  *self, zip_type_figure		   figure )
      {
  long				  symbol_index;
  char				 *s, *t, *algorithm = NULL;
  const char			 *number;
  static char				  string[257], symbol_string[257];
  struct symbol		 *symbol_vector;
  static const char			 * const old_names[] =
    { "Arrows", "arrows",	"Borders", "borders",
      "CMU", "cmu",		"Computers", "computer",
      "DayBooks", "daybooks",
      "FlowChart", "flowchrt",  "Games","games",
      "General", "general",	"Home", "home",
      "ITC", "itc",		"Months", "months",
      "Mathematical", "math",	"Organization", "organize",
      "Shields", "shields",	"Stars", "stars",
      NULL };
  const char		* const *old_name = old_names;

  IN(Symbol_Algorithm);
  if ( Identify_Symbol_Sets( self ) == zip_ok )
    {
    s = figure->zip_figure_datum.zip_figure_text;
    DEBUGst(Text-string,s);
    t = string;
    while ( *s  &&  *s != ';'  &&  *s != '\n' )    *t++ = *s++;
    *t = 0;
    DEBUGst(Original Symbol-set-name,string);
    /* Backward compatibility for symbol-set name change */
    while ( *old_name )
      { DEBUGst(Check,*old_name);
      if ( strcmp( *old_name, string ) == 0 )
	{
	fprintf( stderr, "Zip: SymbolSet '%s' Replaced By '%s'\n",
		    *old_name, *(old_name+1) );
	strcpy( string, *(old_name+1) );
	if ( *s == ';' )  number = ++s;  else number = "1";
	sprintf( symbol_string, "%s;%s", string, number );
	DEBUGst(symbol-string,symbol_string);
	(self->data_object)->Set_Figure_Text(  figure, symbol_string );
	break;
	}
      old_name += 2;
      }
    DEBUGst(Used Symbol-set-name,string);
    if ( ( symbol_vector = Symbol_Set_Vector( self, string ) ) )
      {
      if ( ( CurrentSymbolSet = Symbol_Set( self, string ) ) )
	{
        if ( *s == ';' )    s++;
        t = string;
        while ( *s  &&  *s != ';'  &&  *s != '\n' )    *t++ = *s++;
        *t = 0;
        DEBUGst(Symbol-set-index,string);
        CurrentSymbolSetIndex = symbol_index = atol( string );
        DEBUGdt(Symbol-index,symbol_index);
        algorithm = symbol_vector[symbol_index].symbol_algorithm;
	}
	else
	{ DEBUG(Error);
	fprintf( stderr, "Zip: SymbolSet '%s' Not Found\n", string );
	}
      }
      else
      { DEBUG(ERROR in Symbol-Set-Vector);
/*===*/
      }
    }
    else
    { DEBUG(ERROR Identifying Symbol-Sets);
/*===*/
    }
  DEBUGst(Algorithm,algorithm);
  OUT(Symbol_Algorithm);
  return  algorithm;
  }

static
void Highlight_Set_Name( class ziposymbol		  *self, zip_type_pane		   pane, struct symbol_set		  *set )
        {
  struct symbol_set		 *candidate = SymbolSets;
  class fontdesc		 *normal_font, *highlight_font;

  IN(Highlight_Set_Name);
  normal_font    = (class fontdesc *) (self->data_object)->Define_Font(  "andysans10", NULL );
  highlight_font = (class fontdesc *) (self->data_object)->Define_Font(  "andysans10b", NULL );
  while ( candidate->set_name )
    {
    if ( candidate->set_highlighted  &&  candidate != set )
      { DEBUG(Normalize);
      candidate->set_highlighted = false;
      Draw_Set_Name( self, pane, candidate, normal_font );
      }
    else
    if ( (! candidate->set_highlighted)  &&  candidate == set )
      { DEBUG(Highlight);
      candidate->set_highlighted = true;
      Draw_Set_Name( self, pane, candidate, highlight_font );
      }
    candidate++;
    }
  OUT(Highlight_Set_Name);
  }

static
void Draw_Set_Name( class ziposymbol		  *self, zip_type_pane		   pane, struct symbol_set		  *set, class fontdesc		  *font )
          {
  IN(Draw_Set_Name);
  (self->view_object)->Set_Clip_Area(  pane, SVL, SVT + 40, SVW/3, SVH - 50 );
  (self->view_object)->SetTransferMode(  graphic_WHITE );
  (self->view_object)->EraseRectSize(  set->set_left, set->set_top,
			       set->set_right  - set->set_left,
		               set->set_bottom - set->set_top );
  (self->view_object)->SetTransferMode(  graphic_BLACK );
  (self->view_object)->SetFont(  font );
  (self->view_object)->MoveTo(  set->set_left, set->set_top );
  (self->view_object)->DrawString(  set->set_name, graphic_ATLEFT | graphic_ATTOP );
  OUT(Draw_Set_Name);
  }

static long
Show_Symbol_Dialog( class ziposymbol *self, zip_type_pane pane )
      {
  long				  status = zip_ok, x, y;
  struct symbol_set		 *set;
  class fontdesc		 *normal_font, *highlight_font;

  IN(Show_Symbol_Dialog);
  (self->view_object )->Use_Working_Pane_Cursors( );
  if ( (status = Identify_Symbol_Sets( self )) == zip_ok )
    {
    (self->edit_object)->Set_Keyboard_Processor(  (long) self, (zip_keyboardfptr) Accept_Property_Hit );
    (self->edit_object)->Set_Pending_Processor(  (long) self, (zip_pendingfptr) Show_Symbol_Dialog );
    (self->view_object)->Set_Pane_Clip_Area(  pane );
    SymbolSelected = false;
    SVL = (self->view_object)->Pane_Left(  pane ) + 20;  SVT = (self->view_object)->Pane_Top(  pane ) + 20;
    SVW = (self->view_object)->Pane_Width(  pane ) - 40; SVH = (self->view_object)->Pane_Height(  pane ) - 40;
    SVR = SVL + SVW;  SVB = SVT + SVH;
    (self->view_object)->SetTransferMode(  graphic_WHITE );
    (self->view_object)->EraseRectSize(  SVL-1, SVT-1, SVW+2, SVH+2 );
    (self->view_object)->SetTransferMode(  graphic_BLACK );
    (self->view_object)->DrawRRectSize(  SVL, SVT, SVW-1, SVH-1, 10,10 );
    (self->view_object)->DrawRRectSize(  SVL+1, SVT+1, SVW-3, SVH-3, 10,10 );
    (self->view_object)->MoveTo(  SVL + SVW/3, SVT );
    (self->view_object)->DrawLineTo(  SVL + SVW/3, SVT + SVH-1 );
    (self->view_object)->MoveTo(  SVL, SVT + 40 );
    (self->view_object)->DrawLineTo(  SVL + SVW/3, SVT + 40 );
    (self->view_object)->MoveTo(  SVL + SVW/6, SVT + 25 );
    (self->view_object)->SetFont(  (self->data_object)->Define_Font(  "andysans12b", NULL ) );
    (self->view_object)->Set_Clip_Area(  pane, SVL, SVT + 5, SVW/3, 40 );
    (self->view_object)->DrawString(  "Symbols", graphic_BETWEENLEFTANDRIGHT | graphic_BETWEENTOPANDBOTTOM );
    set = SymbolSets;
    normal_font = (class fontdesc *) (self->data_object)->Define_Font(  "andysans10", NULL );
    highlight_font = (class fontdesc *) (self->data_object)->Define_Font(  "andysans10b", NULL );
    x = SVL + 5;  y = SVT + 54;
    while ( set->set_name )
      { DEBUGst(Display Set-name,set->set_name);
      set->set_left = x;
      set->set_right = SVL + SVW/3; 
      set->set_top = y - 6;
      set->set_bottom = y + 6;
      if ( set->set_highlighted )
        {
        Draw_Set_Name( self, pane, set, highlight_font );
        Show_Set_Symbols( self, pane, set );
        }
        else
        Draw_Set_Name( self, pane, set, normal_font );
      y += 12;
      set++;
      }
    (self->view_object )->Use_Normal_Pane_Cursors( );
    (self->view_object)->Set_Pane_Cursor(  pane, 'a', "icon12" );
    (self->view_object)->Set_Pane_Clip_Area(  pane );
    (self->view_object)->Announce(  "Select a Symbol-Set ..." );
    }
  OUT(Show_Symbol_Dialog);
  return  status;
  }

static
void Show_Set_Symbols( class ziposymbol		  *self, zip_type_pane		   pane, struct symbol_set		  *set )
        {
  struct symbol		 *symbols;
  short			  row = 0, col = 0, x, y,
					  x_increment, y_increment;

  IN(Show_Set_Symbols);
  DEBUGst(Set-name,set->set_name);
  (self->view_object)->Set_Pane_Clip_Area(  pane );
  (self->view_object)->Set_Pane_Cursor(  pane, 'H', "icon12" );
  SymbolSetDisplayed = set;
  if ( set->set_file == NULL )
    Open_Symbol_Set_File( self, set );
  x = SVL + SVW/3 + 4;  y = SVT + 4;
  x_increment = (SVR - x) / 4 - 4;  y_increment = (SVB - y) / 4 - 4;
  (self->view_object)->SetTransferMode(  graphic_WHITE );
  (self->view_object)->EraseRectSize(  x, y, SVR - x - 4, SVB - y - 4 );
  (self->view_object)->SetTransferMode(  graphic_BLACK );
  symbols = set->set_symbols;
  symbols++;
  while ( symbols->symbol_algorithm )
    {
    symbols->symbol_left = x; symbols->symbol_right = x + x_increment;
    symbols->symbol_top  = y; symbols->symbol_bottom = y + y_increment;
    (self->view_object)->DrawRectSize(  x, y, x_increment, y_increment );
    Draw_Symbol( self, NULL, pane, symbols->symbol_algorithm,
		 x+4, y+4, x_increment-6, y_increment-6, 1, 1, false );
    if ( symbols->symbol_highlighted )
      Invert_Symbol( self, symbols );
    if ( ++col == 4 )
      { row++; y += y_increment + 4; col = 0; x = SVL + SVW/3 + 4; }
      else x += x_increment + 4;
    symbols++;
    }
  (self->view_object)->Set_Pane_Clip_Area(  pane );
  (self->view_object)->Set_Pane_Cursor(  pane, 'a', "icon12" );
  OUT(Show_Set_Symbols);
  }

static
void Highlight_Symbol( class ziposymbol		  *self, struct symbol_set		  *set, struct symbol		  *symbol )
        {
  struct symbol		 *candidate = set->set_symbols;

  IN(Highlight_Symbol);
  candidate++;
  while ( candidate->symbol_algorithm )
    {
    if ( candidate->symbol_highlighted  &&  candidate != symbol )
      { DEBUG(Normalize);
      candidate->symbol_highlighted = false;
      Invert_Symbol( self, symbol );
      }
    else
    if ( (! candidate->symbol_highlighted)  &&  candidate == symbol )
      { DEBUG(Highlight);
      candidate->symbol_highlighted = true;
      Invert_Symbol( self, symbol );
      }
    candidate++;
    }
  OUT(Highlight_Symbol);
  }

static
void Invert_Symbol( class ziposymbol		  *self, struct symbol		  *symbol )
      {
  IN(Invert_Symbol);
  (self->view_object)->SetTransferMode(  graphic_INVERT );
  (self->view_object)->FillRectSize(  symbol->symbol_left+2, symbol->symbol_top+2,
			symbol->symbol_right  - symbol->symbol_left - 3,
			symbol->symbol_bottom - symbol->symbol_top - 3,
			(self->view_object )->WhitePattern( ) );
  OUT(Invert_Symbol);
  }

static enum view_MouseAction
Accept_Property_Hit( class ziposymbol		  *self, zip_type_pane		   pane, char				   c, enum view_MouseAction	   action, long				   x , long				   y , long				   clicks )
            {
  struct symbol_set		 *set = SymbolSets;
  struct symbol		 *symbols;
  short			  serial = 1;
  static boolean			  exit_on_up;

  IN(Accept_Property_Hit)
  if ( action == view_LeftDown )
    {
    if ( x >= SVL  &&  x <= SVR &&  y >= SVT   &&  y <= SVB )
      { DEBUG(Inside Symbol-view);
      if ( x <= SVL + SVW/3 )
        { DEBUG(Inside Set-names);
        while ( set->set_name )
          {
          if ( x >= set->set_left  &&  x <= set->set_right  &&
	       y >= set->set_top   &&  y <= set->set_bottom )
	    { DEBUGst(Set-name Hit,set->set_name);
	    strcpy( SelectedSymbolSetName, set->set_name );
	    DEBUGst(Selected Set,SelectedSymbolSetName);
	    SelectedSymbolSet = CurrentSymbolSet = set;
	    SelectedSymbolSetIndex = CurrentSymbolSetIndex = 0;
	    Highlight_Set_Name( self, pane, set );
	    Show_Set_Symbols( self, pane, set );
	    (self->view_object)->Announce(  "Select a Symbol, then Drag an Area ..." );
	    break;
	    }
          set++;
          }
	}
      else
        { DEBUG(Inside Symbol-icons);
        if ( SymbolSetDisplayed )
	  {
          symbols = SymbolSetDisplayed->set_symbols;
          symbols++;
          while ( symbols  &&  serial <= SymbolSetDisplayed->set_symbols_count )
	    {
	    if ( x >= symbols->symbol_left  &&  x <= symbols->symbol_right  &&
	         y >= symbols->symbol_top   &&  y <= symbols->symbol_bottom )
	      { DEBUG(Inside Symbol Icon);
	      sprintf( SelectedSymbolIndexName, "%d", serial );
	      DEBUGst(Selected Index,SelectedSymbolIndexName);
	      SelectedSymbolSetIndex = serial;
	      SelectedSymbol = &SelectedSymbolSet->set_symbols[serial];
	      Highlight_Symbol( self, SelectedSymbolSet, SelectedSymbol );
	      SymbolSelected = true;
	      break;
	      }
	    serial++;  symbols++;
	    }
	  }
	}
      }
      else
      { DEBUG(Outside Symbol-view);
      exit_on_up = true;
      *SelectedSymbolIndexName = 0;
      SelectedSymbol = NULL;
      SelectedSymbolSetIndex = 0;
      (self->view_object)->Announce(  "No Symbol Selected." );
      }
    }
    else
    {
    if ( action == view_LeftUp )
      {
      if ( SymbolSelected  ||  exit_on_up )
        Decline_Property_Hits( self, pane );
      }
      else if ( action == view_NoMouseEvent  ||  action == view_RightDown )
        Decline_Property_Hits( self, pane );
    exit_on_up = false;
    }
  if ( action != view_RightDown )
    action = view_NoMouseEvent;
  OUT(Accept_Property_Hit);
  return  action;
  }

static
void Decline_Property_Hits( class ziposymbol		  *self, zip_type_pane		   pane )
      {
  IN(Decline_Property_Hits);
  (self->view_object)->SetTransferMode(  graphic_WHITE );
  (self->view_object)->EraseRectSize(  SVL, SVT, SVW, SVH );
  (self->edit_object)->Set_Keyboard_Processor(  0, NULL );
  (self->edit_object)->Set_Pending_Processor(  0, NULL );
  (self->view_object)->Set_Pane_Cursor(  pane, 'B', CursorFontName );
  (self->view_object)->Draw_Pane(  pane );
  OUT(Decline_Property_Hits);
  }

static char *
Skip_Colon( char				  *string )
    {
  while ( *string  &&  *string != ':' )  string++;
  if ( *string )     string++;
  return  string;
  }

static char *
String( class ziposymbol *self, char *string, char **s )
{
  static char			  extracted_string[257];
  char				 *p = extracted_string;

  IN(String);
  *extracted_string = 0;
  *s = extracted_string;
  while ( *string  &&  *string != ','  &&  *string != '\n' )
    {
    *p++ = *string++;
    }
  *p = 0;
  if ( *string  &&  (*string == ','  ||  *string == '\n') )  string++;
  DEBUGst(Extracted,extracted_string );
  OUT(String);
  return  string;
  }

static char *
Number( class ziposymbol		  *self, char				  *string, long				  *n )
        {
  char					  number_string[257];
  char				 *p = number_string;
  boolean			  number_found = false;

  IN(Number);
  while ( *string  &&
        (((*string - '0') >= 0  && (*string - '0') <= 9 )  ||
	   *string == '-'  ||  *string == '+' ) )
    {
    *p++ = *string++;
    number_found = true;
    }
  *p = 0;
  DEBUGst(Number-string,number_string);
  if ( number_found )
    {
    if ( *string )  string++;
    *n = LastNumber = atol( number_string );
    }
    else  *n = 0;
  DEBUGdt(N,*n );
  OUT(Number);
  return  string;
  }

static char *
Pixel( class ziposymbol *self, char *string, long *x , long *y, double	 M , double D , double XO , double YO, long  x_factor , long y_factor )
{
  long					  X, Y;

  IN(Pixel);
  string = Number( self, string, &X );
  string = Number( self, string, &Y );
  *x = (long) (XO + (X * x_factor) * M);
  *y = (long) (YO - (Y * y_factor) * D);
  DEBUGdt(X-Pixel,*x );
  DEBUGdt(Y-Pixel,*y );
  OUT(Pixel);
  return  string;
  }
