/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

/* zipve03.c	Zip EditView-object  -- Hit Handling		      */
/* Author	TC Peters					      */
/* Information Technology Center	   Carnegie-Mellon University */


/**  SPECIFICATION -- External Facility Suite  *********************************

TITLE	The Zip EditView-object  --  Hit Handling

MODULE	zipve03.c

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
  10/19/88	Reduce frequency of re-drawing Pane (Draw_Pane) (TCP)
  05/01/89	Use symbolic font-names (TCP)
  08/28/89	Suppress Draw_Pane() after building each figure build (SCG)
   08/16/90	Remove old color and line_style attribute copying from Duplicate_Selection (SCG)

END-SPECIFICATION  ************************************************************/

#include <andrewos.h>
#include <math.h>
#include "view.H"
#include "rasterview.H"
#include "fontdesc.H"
#include "zip.H"
#include "zipview.H"
#include "zipobject.H"
#include "zipedit.H"
#include "zipedit.h"

#define  InitialX		    (self->prior_x)
#define  InitialY		    (self->prior_y)


static int RBDT( class zipedit *self, zip_type_pane pane, long x, long y );
static int RBDM( class zipedit *self, zip_type_pane pane, long x, long y );
static int RBUT( class zipedit *self, zip_type_pane pane, long x, long y );
static int Handle_Edit_Selection( class zipedit		  *self, zip_type_pane		   pane, enum view_MouseAction	   action, long				   x , long				   y , long				   clicks );
static int Handle_Edit_Selection_Modification( class zipedit	      *self, zip_type_pane	       pane, enum view_MouseAction       action, long			       x , long			       y , long			       clicks );
static int Edit_Modification_LBDT( class zipedit *self, zip_type_pane pane, zip_type_pixel	       x, zip_type_pixel y );
static int Edit_Modification_LBDM( class zipedit *self, zip_type_pane pane, zip_type_pixel x, zip_type_pixel  y );
static int Edit_Modification_LBUT( class zipedit *self, zip_type_pane pane, zip_type_pixel  x, zip_type_pixel  y );
zip_type_figure zipedit_Next_Selected_Figure( class zipedit	      *self, zip_type_pane	       pane, zip_type_figure	       figure );
static void Move_Selection( class zipedit	      *self, zip_type_pane	       pane );
static void Duplicate_Selection( class zipedit	      *self, zip_type_pane	       pane, zip_type_point	       x_delta , zip_type_point	       y_delta );
void zipedit_Cancel_Enclosure( class zipedit	      *self, zip_type_pane	       pane );
static boolean Within_Enclosure( class zipedit	      *self, zip_type_pane	       pane, zip_type_pixel	       x , zip_type_pixel	       y );
static void Draw_Enclosure_Shadow( class zipedit	      *self, zip_type_pane	       pane, long			       x , long			       y );
static void Clear_Enclosure_Shadow( class zipedit	      *self, zip_type_pane	       pane );
static void Show_Enclosure_Shadow( class zipedit	      *self, zip_type_pane	       pane );
void zipedit_Enclose_Figure( class zipedit	      *self, zip_type_figure	       figure, zip_type_pane	       pane );
static int Check_Enclosure( class zipedit	      *self, zip_type_pane	       pane, zip_type_image	       image, long			       count );
static void Draw_Enclosure( class zipedit	      *self, zip_type_pane	       pane, long			       x , long			       y );
static void Clear_Enclosure( class zipedit	      *self, zip_type_pane	       pane );
void Show_Enclosure( class zipedit	      *self, zip_type_pane	       pane );
static void Enclosure_Bounds( class zipedit	      *self, zip_type_pane	       pane, zip_type_pixel	      *L , zip_type_pixel	      *T , zip_type_pixel	      *W , zip_type_pixel	      *H );
static void Show_Names( class zipedit	       *self, zip_type_pane	        pane );
static void Show_Point( class zipedit	      *self, zip_type_pane	       pane, zip_type_figure	       figure, int			       point );
static void Show_Font( class zipedit	      *self, zip_type_pane	       pane, char			      *font_name );
static void Clear_Font( class zipedit	      *self, zip_type_pane	       pane );
static void Set_Constraints( class zipedit	      *self, zip_type_pane	       pane, zip_type_pixel	       x  , zip_type_pixel	       y, zip_type_point	      *X , zip_type_point	      *Y );


int
zipedit::Accept_Hit( zip_type_pane	hit_pane, enum view_MouseAction   action , long x , long y , long clicks )
{
  class zipedit *self=this;
  int			      status = zip_ok;
  zip_type_pane	      pane;

  IN(zipedit_Accept_Hit);
  DEBUGst(HitPane-name,hit_pane->zip_pane_name);
  pane = View->pane;
  DEBUGst(Pane-name,pane->zip_pane_name);
  DEBUGdt(Abeyant,Abeyant);
  if ( hit_pane  &&  !Abeyant )
    { DEBUG(Not Abeyant);
    if ( Moving )  hit_pane = EditPane;
    if ( hit_pane == EditPane  ||  hit_pane == BackgroundPane )
      { DEBUG(EditPane Selected);
	hit_pane = EditPane;
        if ( this->keyboard_processor )
          {
	  DEBUG(>>> Keyboard Processor);
	  action = ((*this->keyboard_processor)( KeyboardAnchor, pane, 0, action, x, y, clicks ));
	  DEBUG(<<< Keyboard Processor);
	  }
        switch ( action )
          {
          case view_RightDown:     status = RBDT( self, hit_pane, x, y); break;
          case view_RightMovement: status = RBDM( self, hit_pane, x, y); break;
          case view_RightUp:       status = RBUT( self, hit_pane, x, y ); break;
          default:  status = Handle_Edit_Selection( self, hit_pane, action, x, y, clicks );
	  }
      }
    else    if ( hit_pane == FontFamilyPane )
      status = zipedit_Handle_Font_Family_Selection( self, hit_pane, action, x, y, clicks );
    else    if ( hit_pane == FontHeightPane )
      status = zipedit_Handle_Font_Height_Selection( self, hit_pane, action, x, y, clicks );
    else    if ( hit_pane == FontItalicPane )
      status = zipedit_Handle_Font_Italic_Selection( self, hit_pane, action, x, y, clicks );
    else    if ( hit_pane == FontBoldPane )
      status = zipedit_Handle_Font_Bold_Selection(   self, hit_pane, action, x, y, clicks );
    else    if ( hit_pane == FontSamplePane )
      status = zipedit_Handle_Font_Sample_Selection( self, hit_pane, action, x, y, clicks );
/*===
    else    if ( hit_pane == HierarchyPane )
      status = zipedit_Handle_Hierarchy_Selection( self, hit_pane, action, x, y, clicks );
===*/
    else
    if ( x >= (View)->Pane_Left(  FiguresPane )   &&
	 x <= (View)->Pane_Right(  FiguresPane )  &&  
	 y >= (View)->Pane_Top(  FiguresPane )    &&  
	 y <= (View)->Pane_Bottom(  FiguresPane ) &&
	 hit_pane != FiguresPane )
      { DEBUG(Figure Palette Hit);
      Building = true;
      status = zipedit_Handle_Figure_Palette_Hit( self, hit_pane, action, x, y, clicks );
      }
    else
    if ( x >= (View)->Pane_Left(  ShadesPane )   &&
	 x <= (View)->Pane_Right(  ShadesPane )  &&  
	 y >= (View)->Pane_Top(  ShadesPane )    &&  
	 y <= (View)->Pane_Bottom(  ShadesPane ) )
      { DEBUG(Shade Palette Hit);
      status = zipedit_Handle_Shade_Palette_Hit( self, hit_pane, action, x, y, clicks );
      }
    }
    else  if ( BackgroundSelected ) 
      { DEBUG(BackgroundPane Hit);
      (BackgroundView)->Hit(  action,
			(BackgroundView)->EnclosedXToLocalX(  x ),
			(BackgroundView)->EnclosedYToLocalY(  y ), clicks );
      }
  OUT(zipedit_Accept_Hit);
  return status;
  }

static zip_type_point	      initial_x_point, initial_y_point,
			      final_x_point, final_y_point;
extern void zipedit_Reset_Editing_Selection( class zipedit         *self, zip_type_pane               pane );

static int
RBDT( class zipedit *self, zip_type_pane pane, long x, long y )
  {
  int			      status = zip_ok;

  IN(RBDT);
  if ( self->keyboard_processor )
    (*self->keyboard_processor)( KeyboardAnchor, pane, 0, view_NoMouseEvent, 0, 0, 0 );
  zipedit_Reset_Editing_Selection( self, pane );
  zipedit_Cancel_Enclosure( self, pane );
  if ( ForegroundPinning )
    {
/*===    status = zipview_Handle_Pinning( View, pane, x, y, NULL, NULL );*/
    status = zip_ok;
    }
    else
    {
    if ( ForegroundPanning )
      Set_Constraints( self, pane, x, y, &initial_x_point, &initial_y_point );
    status = (View)->Initiate_Panning(  pane, x, y, 0 /*===*/ );
    }
  OUT(RBDT);
  return status;
  }

static int
RBDM( class zipedit *self, zip_type_pane pane, long x, long y )
  {
  int			      status = zip_ok;

  IN(RBDM);
  status = (View)->Continue_Panning(  pane, x, y );
  OUT(RBDM);
  return status;
  }

extern void zipedit_Display_Background_Pane( class zipedit        *self, zip_type_pane            pane );

static int
RBUT( class zipedit *self, zip_type_pane pane, long x, long y )
{
  int			      status = zip_ok;
  zip_type_image	      image =
			CurrentStream->zip_stream_image_anchor;
  long				      x_delta, y_delta;

  IN(RBUT);
  DEBUGst(Pane-name,pane->zip_pane_name);
  if ( ForegroundPanning )
    { DEBUG(ForeGround Panning);
    status = (View)->Terminate_Panning(  pane, x, y, NULL, NULL, 0 );
    Set_Constraints( self, pane, x, y, &final_x_point, &final_y_point );
    if ( final_x_point - initial_x_point  ||  final_y_point - initial_y_point )
      {
      (View)->Clear_Image(  image, pane );
/*===      CurrentStream->zip_stream_pseudo_x_offset +=
	            final_x_point - initial_x_point;
      CurrentStream->zip_stream_pseudo_y_offset +=
	            final_y_point - initial_y_point;===*/
      (Data)->Adjust_Image_Point_Suite(  image,
		    final_x_point - initial_x_point,
		    final_y_point - initial_y_point/*===, true*/ );
      (View)->Draw_Image(  image, pane );
      }
    }
    else
    { DEBUG(Not ForeGround Panning);
    if ( BackgroundPane )
      { DEBUG(BackgroundPane Exists);
      DEBUGst(BackgroundPane-name,BackgroundPane->zip_pane_name);
      (View)->Terminate_Panning(  pane, x, y, &x_delta, &y_delta, 0 );
      if ( x_delta  ||  y_delta )
	{
        BackgroundPane->zip_pane_x_offset += x_delta;
        BackgroundPane->zip_pane_y_offset += -y_delta;
        BackgroundPane->zip_pane_x_origin_offset = 
	  BackgroundPane->zip_pane_x_origin + BackgroundPane->zip_pane_x_offset;
        BackgroundPane->zip_pane_y_origin_offset =
	  BackgroundPane->zip_pane_y_origin - BackgroundPane->zip_pane_y_offset;
	if ( BackgroundExposed )
	  { DEBUG(BackgroundPane Exposed);
	  pane->zip_pane_x_offset += x_delta;
	  pane->zip_pane_y_offset += -y_delta;
	  pane->zip_pane_x_origin_offset =
	    pane->zip_pane_x_origin + pane->zip_pane_x_offset;
	  pane->zip_pane_y_origin_offset =
	    pane->zip_pane_y_origin - pane->zip_pane_y_offset;
          zipedit_Display_Background_Pane( self, pane );
          (View)->Draw_Pane(  pane );
	  }
	  else
	  { DEBUG(BackgroundPane Not Exposed);
	  (View)->Pan_Pane(  pane, x_delta, -y_delta );
	  }
	}
      }
      else  status = (View)->Terminate_Panning(  pane, x, y, NULL, NULL, 1 );
    }
  OUT(RBUT);
  return status;
  }

extern void zipedit_Expose_Selection_Menu( class zipedit        *self );


static int
Handle_Edit_Selection( class zipedit		  *self, zip_type_pane		   pane, enum view_MouseAction	   action, long				   x , long				   y , long				   clicks )
          {
  long				  status = zip_ok;
  long					  X, Y;

  IN(Handle_Edit_Selection);
  if ( BuildPending == 0 )
    status = Handle_Edit_Selection_Modification( self, pane, action, x, y, clicks );
    else
    {
    DEBUG(Build Pending);
    zipedit_Cancel_Enclosure( self, pane );
    if ( CurrentFigure ) {
      if ( SelectionLevel >= ImageSelection )
        (self)->Normalize_Image_Points(  CurrentImage, pane );
        else
        (self)->Normalize_Figure_Points(  CurrentFigure, pane );
    }
    SelectionLevel = 0;
    (self)->Set_Pane_Highlight_Icon(  pane, zip_figure_point_icon );

    Set_Constraints( self, pane, x, y, &X, &Y );
    if ( ( X - LastPointX )  ||  ( Y - LastPointY )  ||
	 ! (action == view_LeftMovement) )
      {
      LastPixelX = x;      LastPixelY = y;
      LastPointX = X;      LastPointY = Y;
      if ( action == view_LeftDown )
	{ DEBUG(LeftDown);
	/* Make Cursor disappear -- this currently crashes PS/2 AIX X Server 9/28/89  */
	EditingIcon(pane) = pane->zip_pane_cursor_icon;
	(View)->Set_Pane_Cursor(  pane, '@', CursorFontName );
	}
      status = (Objects(BuildPending))->Build_Object(  pane, action, x, y, clicks, X, Y );
      if ( action == view_LeftUp )
	{ DEBUG(LeftUp);
	(View)->Set_Pane_Cursor(  pane, EditingIcon(pane), CursorFontName );
	if ( CurrentFigure )
	  {
/*===	  if ( !ManualRefresh )
	    zipview_Draw_Pane( View, pane ); ===*/
	  (self)->Highlight_Figure_Points(  CurrentFigure, pane );
          zipedit_Expose_Selection_Menu( self );
	  }
	}
      }
    }
  OUT(Handle_Edit_Selection);
  return status;
  }

static int
Handle_Edit_Selection_Modification( class zipedit	      *self, zip_type_pane	       pane, enum view_MouseAction       action, long			       x , long			       y , long			       clicks )
          {
  int			      status = zip_ok;

  IN(Handle_Edit_Selection_Modification);
  switch ( action )
    {
    case view_LeftDown:
      status = Edit_Modification_LBDT( self, pane, x, y );
      break;
    case view_LeftUp:
      status = Edit_Modification_LBUT( self, pane, x, y );
      break;
    case view_LeftMovement:
      status = Edit_Modification_LBDM( self, pane, x, y );
      break;
    default:
      ;
    }
  OUT(Handle_Edit_Selection_Modification);
  return status;
  }

extern void zipedit_Hide_Selection_Menu( class zipedit          *self );

static int
Edit_Modification_LBDT( class zipedit *self, zip_type_pane pane, zip_type_pixel  x , zip_type_pixel  y )
{
    long			      status = zip_ok;
    zip_type_figure	      figure = NULL;
    long			      point = 0;
    struct timeval		      dummy_time;
    static long			      prior_time, this_time;

    IN(Edit_Modification_LBDT);
    Moving = false;
    InitialX = x;  InitialY = y;
    gettimeofday( &dummy_time, NULL );
    this_time = (dummy_time.tv_sec * 1000) + (dummy_time.tv_usec / 1000);
    (View)->Set_Pane_Cursor(  pane, 'A', CursorFontName );
    EnclosureShadowStartX = EnclosureShadowStartY =
      EnclosureShadowLastX = EnclosureShadowLastY = -1;
    if ( !Within_Enclosure( self, pane, x, y ) )
    { DEBUG(Not Within Enclosure);
    zipedit_Cancel_Enclosure( self, pane );
    EnclosureLeft = x; EnclosureTop = y; EnclosureWidth = EnclosureHeight = 0;
    if ( CurrentFigure  &&  CurrentFigure->zip_figure_visibility == zip_figure_exposed  &&
	(Objects(CurrentFigure->zip_figure_type))->Proximate_Object_Points( CurrentFigure, pane, x, y ) )
	figure = CurrentFigure;
    if ( figure  ||  (figure = (View)->Which_Pane_Figure(  x, y, pane )) )
    {
	zipedit_Expose_Selection_Menu( self );
	point = (self)->Which_Figure_Point(  figure, pane, x, y );
    }
    if ( figure == NULL  ||  figure != CurrentFigure )
    {
	if ( SelectionLevel >= ImageSelection )
	    (self)->Normalize_Image_Points(  CurrentImage, pane );
	else
	    (self)->Normalize_Figure_Points(  CurrentFigure, pane );
	if ( figure )
	    SelectionLevel = PointSelection;
	else
	    SelectionLevel = 0;
	(self)->Set_Pane_Highlight_Icon(  pane, zip_figure_point_icon );
    }
    if ( figure  &&  figure == CurrentFigure  &&
	point  == pane->zip_pane_edit->zip_pane_edit_last_point_id  &&
	(this_time - prior_time) < ZIP_double_click_parameter )
    {
	if ( ++SelectionLevel == ImageSelection )
	{
	    (self)->Normalize_Figure_Points(  CurrentFigure, pane );
	    CurrentImage = figure->zip_figure_image;
	    (self)->Set_Pane_Highlight_Icon(  pane, zip_image_point_suite_icon );
	}
	else
	{
	    if ( SelectionLevel >= ImageSelection  &&  CurrentImage )
	    {
		(self)->Normalize_Image_Points(  CurrentImage, pane );
		if ( CurrentImage->zip_image_superior )
		    CurrentImage = CurrentImage->zip_image_superior;
		(self)->Set_Pane_Highlight_Icon(  pane, zip_image_point_suite_icon );
	    }
	    else
	    {
		(self)->Normalize_Figure_Points(  CurrentFigure, pane );
		(self)->Set_Pane_Highlight_Icon(  pane, zip_figure_point_suite_icon );
	    }
	}
    }
    else
    {
	SelectionLevel = PointSelection;
	(self)->Set_Pane_Highlight_Icon(  pane, zip_figure_point_icon );
	if ( figure )
	{
	    CurrentFigure = figure;
	    CurrentImage  = figure->zip_figure_image;
	    CurrentStream = figure->zip_figure_image->zip_image_stream;
/*===      CurrentPane   = pane;*/
	}
	else
	{
	    zipedit_Hide_Selection_Menu( self );
	    CurrentFigure = NULL;
	    if ( CurrentImage )
		CurrentStream = CurrentImage->zip_image_stream;
	    else
	    {
		if ( pane->zip_pane_attributes.zip_pane_attribute_stream_source  &&
		    pane->zip_pane_source.zip_pane_stream )
		    CurrentStream = pane->zip_pane_source.zip_pane_stream;
		else
		    if ( pane->zip_pane_attributes.zip_pane_attribute_image_source  &&
			pane->zip_pane_source.zip_pane_image )
			CurrentStream = pane->zip_pane_source.zip_pane_image->zip_image_stream;
		    else
			if ( pane->zip_pane_attributes.zip_pane_attribute_figure_source  &&
			    pane->zip_pane_source.zip_pane_figure )
			    CurrentStream = pane->zip_pane_source.zip_pane_figure->zip_figure_image->zip_image_stream;
	    }
	}
    }
    pane->zip_pane_edit->zip_pane_edit_beginning = true;
    LastPixelX = x;  LastPixelY = y;
    if ( figure )
    {
	(View)->Set_Pane_Cursor(  pane, '@', CursorFontName ); /* Make Cursor disappear */
	Set_Constraints( self, pane, LastPixelX, LastPixelY, &LastPointX, &LastPointY );
	if ( SelectionLevel >= ImageSelection )
	    (self)->Highlight_Image_Points(  CurrentImage, pane );
	else
	    (self)->Highlight_Figure_Points(  CurrentFigure, pane );
	pane->zip_pane_edit->zip_pane_edit_last_point_id = point;	 
	Show_Names( self, pane );
	Show_Point( self, pane, figure, point );
/*=== should check upward also */
/*=== don't we have a macro for addressing font-name ? */
	if ( figure->zip_figure_font )
	    Show_Font( self, pane, self->data_object->fonts->zip_fonts_vector[figure->zip_figure_font].
		      zip_fonts_table_name );
	else
	    Clear_Font( self, pane );
    }
    }
    else
    { DEBUG(Within Enclosure);
    Draw_Enclosure_Shadow( self, pane, x, y );
    }
    prior_time = this_time;
    OUT(Edit_Modification_LBDT);
    return status;
}

static int
Edit_Modification_LBDM( class zipedit *self, zip_type_pane pane, zip_type_pixel  x , zip_type_pixel  y )
{
    int			      status = zip_ok;
    long				      X, Y;

    IN(Edit_Modification_LBDM);
    Moving = true;
    if ( EnclosureExposed )
    { DEBUG(Within Enclosure);
    Draw_Enclosure_Shadow( self, pane, x, y );
    }
    else
    { DEBUG(Outside Enclosure);
    if ( CurrentFigure )
    {
	Set_Constraints( self, pane, x, y, &X, &Y );
	if ( abs(X - LastPointX)  ||  abs(Y - LastPointY) )
	{
	    if ( pane->zip_pane_edit->zip_pane_edit_beginning ) {
		if ( SelectionLevel >= ImageSelection )
		    (self)->Normalize_Image_Points(  CurrentImage, pane );
		else
		    (self)->Normalize_Figure_Points(  CurrentFigure, pane );
	    }
	    (View)->Set_Pane_Painting_Mode(  pane, zipview_paint_inverted );
	    switch ( SelectionLevel )
	    {
		case PointSelection:
		    if ( pane->zip_pane_edit->zip_pane_edit_beginning )
			(View)->Clear_Figure(  CurrentFigure, pane );
		    else
			(View)->Draw_Figure(  CurrentFigure, pane );
		    (Data)->Set_Figure_Point(  CurrentFigure,
					     pane->zip_pane_edit->zip_pane_edit_last_point_id, X, Y );
		    (View)->Draw_Figure(  CurrentFigure, pane );
		    break;
		case FigureSelection:
		    if ( pane->zip_pane_edit->zip_pane_edit_beginning )
			(View)->Clear_Figure(  CurrentFigure, pane );
		    else
			(View)->Draw_Figure(  CurrentFigure, pane );
		    (Data)->Adjust_Figure_Point_Suite(  CurrentFigure,
						      X - LastPointX,
						      Y - LastPointY );
		    (View)->Draw_Figure(  CurrentFigure, pane );
		    break;
		default:
		    if ( pane->zip_pane_edit->zip_pane_edit_beginning )
			(View)->Clear_Image(  CurrentImage, pane );
		    else
			(View)->Draw_Image(  CurrentImage, pane );
		    (Data)->Adjust_Image_Point_Suite(  CurrentImage,
						     X - LastPointX, Y - LastPointY );
		    (View)->Draw_Image(  CurrentImage, pane );
	    }
	    (View)->Set_Pane_Painting_Mode(  pane, zip_default );
	    Show_Point( self, pane, CurrentFigure,
		       pane->zip_pane_edit->zip_pane_edit_last_point_id );
	    LastPointX = X;  LastPointY = Y;
	    pane->zip_pane_edit->zip_pane_edit_beginning = false;
	}
    }
    else
    { DEBUG(Enclosuring);
    Draw_Enclosure( self, pane, x, y );
    }
    }
    LastPixelX = x;  LastPixelY = y;
    OUT(Edit_Modification_LBDM);
    return status;
}

static int
Edit_Modification_LBUT( class zipedit *self, zip_type_pane pane, zip_type_pixel  x , zip_type_pixel  y )
{
    IN(Edit_Modification_LBUT);
    Moving = false;
    if ( CurrentFigure )
    {
	pane->zip_pane_edit->zip_pane_edit_beginning = false;
	Show_Point( self, pane, CurrentFigure, pane->zip_pane_edit->zip_pane_edit_last_point_id );
	if ( SelectionLevel >= ImageSelection )
	    (self)->Normalize_Image_Points(  CurrentImage, pane );
	else
	    (self)->Normalize_Figure_Points(  CurrentFigure, pane );
	if ( !ManualRefresh  &&  (abs(x - InitialX) > 3  ||  abs(y - InitialY) > 3) )
	    (View)->Draw_Pane(  pane );
	if ( SelectionLevel >= ImageSelection )
	    (self)->Highlight_Image_Points(  CurrentImage, pane );
	else
	    (self)->Highlight_Figure_Points(  CurrentFigure, pane );
    }
    else
    {
	if ( CurrentStream  &&  EnclosureWidth )
	{
	    EnclosureExposed = true;
	    Draw_Enclosure_Shadow( self, pane, x, y );
	    if ( EnclosedFigures ||
		Check_Enclosure( self, pane, CurrentStream->zip_stream_image_anchor, 0 ) )
	    {
		zipedit_Expose_Selection_Menu( self );
		Move_Selection( self, pane );
	    }
	    else
	    {
		zipedit_Cancel_Enclosure( self, pane );
	    }
	    EnclosureShadowStartX = EnclosureShadowStartY =
	      EnclosureShadowLastX = EnclosureShadowLastY = -1;
	}
    }
    (View)->Set_Pane_Cursor(  pane, 'A', CursorFontName );
    OUT(Edit_Modification_LBUT);
    return  zip_ok;
}

zip_type_figure
zipedit_Next_Selected_Figure( class zipedit	      *self, zip_type_pane	       pane, zip_type_figure	       figure )
        {
  zip_type_figure	      next = NULL;
  long			      i = 0;

  IN(zipedit_Next_Selected_Figure);
  if ( figure  &&  figure != CurrentFigure )
    { DEBUG(Nth selection);
    while ( EnclosedFigures  &&  EnclosedFigures[i] )
      if ( EnclosedFigures[i] == figure )
	{
	next = EnclosedFigures[i+1];
	break;
	}
	else  i++;
    }
    else
    { DEBUG(Initial selection);
    if ( EnclosedFigures )
      next = EnclosedFigures[0];
      else
      if ( figure == NULL )
	next = CurrentFigure;
    }
  DEBUGxt(Next,next);
  OUT(zipedit_Next_Selected_Figure);
  return  next;
  }

static
void Move_Selection( class zipedit	      *self, zip_type_pane	       pane )
      {
  zip_type_point	      X_delta, Y_delta;
  zip_type_point		      X1, Y1, X2, Y2;
  long				      L, T, W, H,
				      i = 0;

  IN(Move_Selection);
  if ( EnclosedFigures  &&  EnclosureShadowStartX > 0  &&
	( abs(EnclosureShadowStartX - EnclosureShadowLastX) > 2  ||
	  abs(EnclosureShadowStartY - EnclosureShadowLastY) > 2 ) )
    {
    Clear_Enclosure_Shadow( self, pane );
    Set_Constraints( self, pane, EnclosureShadowStartX, EnclosureShadowStartY, &X1, &Y1 );
    Set_Constraints( self, pane, EnclosureShadowLastX,  EnclosureShadowLastY,  &X2, &Y2 );
    X_delta = X2 - X1;
    DEBUGdt(X-delta,X_delta);
    Y_delta = Y2 - Y1;
    DEBUGdt(Y-delta,Y_delta);
    if ( DuplicateSelection )
      {
      Duplicate_Selection( self, pane, X_delta, Y_delta );
      Show_Enclosure( self, pane );
      }
      else
      {
      while( EnclosedFigures[i] )
        {
        DEBUGst(Adjust Figure,EnclosedFigures[i]->zip_figure_name);
        (self)->Normalize_Figure_Points(  EnclosedFigures[i], pane );
        (Data)->Adjust_Figure_Point_Suite(  EnclosedFigures[i], X_delta, Y_delta );
        i++;
     	}
      Enclosure_Bounds( self, pane, &L, &T, &W, &H );
      (View)->SetTransferMode(  graphic_WHITE );
      (View)->EraseRectSize(  L-2, T-2, W+4, H+4 );
      EnclosureLeft = L + (EnclosureShadowLastX - EnclosureShadowStartX);
      EnclosureTop  = T + (EnclosureShadowLastY - EnclosureShadowStartY);
      EnclosureHeight = H; EnclosureWidth = W;
      }
    (View)->Draw_Pane(  pane );
    Show_Enclosure( self, pane );
    }
  OUT(Move_Selection);
  }

static
void Duplicate_Selection( class zipedit	      *self, zip_type_pane	       pane, zip_type_point	       x_delta , zip_type_point	       y_delta )
        {
  zip_type_figure	      original_figure = NULL;
  zip_type_figure		      new_figure, peer_figure = NULL;
/*===
  char				      name[512];
  static long			      name_serial = 1;
===*/
  long			      i;

  IN(Duplicate_Selection);
  peer_figure = CurrentImage->zip_image_figure_anchor;
  while ( peer_figure  &&  peer_figure->zip_figure_next )
    peer_figure = peer_figure->zip_figure_next;
  while ( ( original_figure = zipedit_Next_Selected_Figure( self, PANE, original_figure ) ) )
    { DEBUGst(Duplicating Figure-name,original_figure->zip_figure_name );
    (self)->Normalize_Figure_Points(  original_figure, pane );
    (Data)->Create_Figure(  &new_figure, NULL, original_figure->zip_figure_type,
		       CurrentImage, peer_figure );
    if ( original_figure->zip_figure_name )
	{
/*=== generate non-duplicate name*/
	}
    new_figure->zip_figure_datum.zip_figure_anchor =
	original_figure->zip_figure_datum.zip_figure_anchor;
    new_figure->zip_figure_font = original_figure->zip_figure_font;
    new_figure->zip_figure_state = original_figure->zip_figure_state;
    new_figure->zip_figure_state.zip_figure_state_points_highlighted = off;
    new_figure->zip_figure_visibility = original_figure->zip_figure_visibility;
    new_figure->zip_figure_mode = original_figure->zip_figure_mode;
    new_figure->zip_figure_style = original_figure->zip_figure_style;
    new_figure->zip_figure_line_width = original_figure->zip_figure_line_width;
/* === duplicate proper line style and color attributes here === */
/*    new_figure->zip_figure_line_style = original_figure->zip_figure_line_style;
    new_figure->zip_figure_color = original_figure->zip_figure_color; */
    new_figure->zip_figure_fill.zip_figure_shade =
	original_figure->zip_figure_fill.zip_figure_shade;
    new_figure->zip_figure_zoom_level = original_figure->zip_figure_zoom_level;
    new_figure->zip_figure_detail_level = original_figure->zip_figure_detail_level;
    new_figure->zip_figure_point.zip_point_x = original_figure->zip_figure_point.zip_point_x;
    new_figure->zip_figure_point.zip_point_y = original_figure->zip_figure_point.zip_point_y;
    if ( original_figure->zip_figure_points )
      {
      new_figure->zip_figure_points = (zip_type_point_pairs)
	malloc( sizeof(long) + (original_figure->zip_figure_points->zip_points_count *
	    sizeof(struct zip_point_pair)) );
      new_figure->zip_figure_points->zip_points_count =
	original_figure->zip_figure_points->zip_points_count;
      DEBUGdt(Points-count,new_figure->zip_figure_points->zip_points_count);
      for ( i = 0; i < original_figure->zip_figure_points->zip_points_count; i++ )
        {
        new_figure->zip_figure_points->zip_points[i].zip_point_x = 
	    original_figure->zip_figure_points->zip_points[i].zip_point_x;
        new_figure->zip_figure_points->zip_points[i].zip_point_y = 
	    original_figure->zip_figure_points->zip_points[i].zip_point_y;
        }
      }
    (Data)->Adjust_Figure_Point_Suite(  new_figure, x_delta, y_delta );
    peer_figure = new_figure;
    }
  OUT(Duplicate_Selection);
  }

void zipedit_Cancel_Enclosure( class zipedit	      *self, zip_type_pane	       pane )
      {
  long			      i = 0;

  IN(zipedit_Cancel_Enclosure);
  zipedit_Hide_Selection_Menu( self );
  if ( EnclosureExposed )
    { DEBUG(Exposed);
    Clear_Enclosure( self, pane );
    }
  if ( EnclosedFigures )
    { DEBUG(Enclosed Figure to Normalize);
    while ( EnclosedFigures[i] )
      {
      (self)->Normalize_Figure_Points(  EnclosedFigures[i], pane );
      i++;
      }
    free( EnclosedFigures );
    EnclosedFigures = NULL;
    }
  DuplicateSelection = false;
  EnclosureLeft = EnclosureTop = EnclosureWidth = EnclosureHeight = 0;
  EnclosureShadowStartX = EnclosureShadowStartY =
    EnclosureShadowLastX = EnclosureShadowLastY = -1;
  OUT(zipedit_Cancel_Enclosure);
  }

static
boolean Within_Enclosure( class zipedit	      *self, zip_type_pane	       pane, zip_type_pixel	       x , zip_type_pixel	       y )
        {
  boolean		      within = false;
  long				      L, T, W, H;

  IN(Within_Enclosure);
  Enclosure_Bounds( self, pane, &L, &T, &W, &H );
  if ( EnclosureExposed  &&
       x > L  &&  x < L + W  &&  y > T  &&  y < T + H )
    within = true;
  OUT(Within_Enclosure);
  return  within;
  }

static
void Draw_Enclosure_Shadow( class zipedit	      *self, zip_type_pane	       pane, long			       x , long			       y )
        {
  IN(Draw_Enclosure_Shadow);
  if ( EnclosureShadowStartX < 0 )
    {
    EnclosureShadowStartX = EnclosureShadowLastX = x;
    EnclosureShadowStartY = EnclosureShadowLastY = y;
    }
    else
    {
    Clear_Enclosure_Shadow( self, pane );
    EnclosureShadowLastX = x;
    EnclosureShadowLastY = y;
    Show_Enclosure_Shadow( self, pane );
    }
  OUT(Draw_Enclosure_Shadow);
  }

static
void Clear_Enclosure_Shadow( class zipedit	      *self, zip_type_pane	       pane )
      {
  IN(Clear_Enclosure_Shadow);
  if ( EnclosureShadowLastX > 0 )
    Show_Enclosure_Shadow( self, pane );
  OUT(Clear_Enclosure_Shadow);
  }

static
void Show_Enclosure_Shadow( class zipedit	      *self, zip_type_pane	       pane )
      {
  long				      L, T, W, H;

  IN(Show_Enclosure_Shadow);
  if ( abs(EnclosureShadowStartX - EnclosureShadowLastX) > 2  ||
       abs(EnclosureShadowStartY - EnclosureShadowLastY) > 2 )
    {
    Enclosure_Bounds( self, pane, &L, &T, &W, &H );
/*===*/
(View)->SetTransferMode(  graphic_INVERT );
if ( (View )->GetLineWidth( ) != 1 )
  (View)->SetLineWidth(  1 );
(View)->DrawRectSize( 
L + (EnclosureShadowLastX - EnclosureShadowStartX),
T + (EnclosureShadowLastY - EnclosureShadowStartY),
W, H );
(View )->FlushGraphics( );
/*===*/
    }
  OUT(Show_Enclosure_Shadow);
  }

void zipedit_Enclose_Figure( class zipedit	      *self, zip_type_figure	       figure, zip_type_pane	       pane )
        {
  IN(zipedit_Enclose_Figure);
  if ( figure )
    {
    (self)->Normalize_Figure_Points(  figure, pane );
    (Objects(figure->zip_figure_type))->Object_Enclosure(  figure, pane,
		&EnclosureLeft, &EnclosureTop, &EnclosureWidth, &EnclosureHeight );
    EnclosureLeft   -= 4;
    EnclosureTop    -= 4;
    EnclosureWidth  += 6;
    EnclosureHeight += 6;
    EnclosureExposed = true;
    Show_Enclosure( self, pane );
    Check_Enclosure( self, pane, CurrentImage, 0 );
    }
  OUT(zipedit_Enclose_Figure);
  }

static
int Check_Enclosure( class zipedit	      *self, zip_type_pane	       pane, zip_type_image	       image, long			       count )
          {
  zip_type_figure	      figure;
  long				      L, T, W, H;

  IN(Check_Enclosure);
  Enclosure_Bounds( self, pane, &L, &T, &W, &H );
  if ( image )
    {
    DEBUGst(Image-name,image->zip_image_name);
    if ( count == 0  &&  EnclosedFigures )
      {
      free( EnclosedFigures );
      EnclosedFigures = NULL;
      }
    figure = image->zip_image_figure_anchor;
    while ( figure )
      {
      DEBUGst(Figure-name,figure->zip_figure_name);
      if ( (figure->zip_figure_visibility == zip_figure_exposed)  &&
	   (Objects(figure->zip_figure_type))->Enclosed_Object(  figure, pane,
		L, T, W, H ) )
        { DEBUG(Figure is Enclosed);
        (self)->Highlight_Figure_Points(  figure, pane );
        if ( EnclosedFigures )
	  { DEBUG(Enclosed Figures exist);
	  EnclosedFigures = (zip_type_figure *) realloc( EnclosedFigures,
			    (count + 2) * sizeof(zip_type_figure *) );
	  EnclosedFigures[count] = figure;
	  EnclosedFigures[count+1] = NULL;
	  count++;
	  }
	  else
	  { DEBUG(First Enclosed Figure);
	  EnclosedFigures = (zip_type_figure *) calloc( 2, sizeof(zip_type_figure *) );
	  EnclosedFigures[0] = figure;
	  EnclosedFigures[1] = NULL;
	  count = 1;
	  }
        }
      figure = figure->zip_figure_next;
      }
    count = Check_Enclosure( self, pane, image->zip_image_inferior, count );
    count = Check_Enclosure( self, pane, image->zip_image_peer, count );
    }
  DEBUGdt(Count,count);
  OUT(Check_Enclosure);
  return  count;
  }

static
void Draw_Enclosure( class zipedit	      *self, zip_type_pane	       pane, long			       x , long			       y )
        {
  IN(Draw_Enclosure);
  if ( x > ((View)->Pane_Left(    pane ) + 2)  &&
       x < ((View)->Pane_Right(   pane ) - 3)  &&  
       y > ((View)->Pane_Top(     pane ) + 2)  &&  
       y < ((View)->Pane_Bottom(  pane ) - 3) )
    {
    Clear_Enclosure( self, pane );
    EnclosureWidth  = x - EnclosureLeft;
    EnclosureHeight = y - EnclosureTop;
    Show_Enclosure( self, pane );
    }
  OUT(Draw_Enclosure);
  }

static
void Clear_Enclosure( class zipedit	      *self, zip_type_pane	       pane )
      {
  IN(Clear_Enclosure);
  EnclosureExposed = false;
  if ( EnclosureWidth > 2  ||  EnclosureWidth < -2 )
    {
    Show_Enclosure( self, pane );
    }
  OUT(Clear_Enclosure);
  }

void Show_Enclosure( class zipedit	      *self, zip_type_pane	       pane )
      {
  long				     L, T, W, H;

  IN(Show_Enclosure);
  Enclosure_Bounds( self, pane, &L, &T, &W, &H );
  if ( W > 2  &&  H > 2 )
    {
/*===*/
(View)->SetTransferMode(  graphic_INVERT );
if ( (View )->GetLineWidth( ) != 1 )
  (View)->SetLineWidth(  1 );
(View)->DrawRectSize(  L, T, W, H );
(View)->DrawRectSize(  L-1, T-1, W+2, H+2 );
(View )->FlushGraphics( );
/*===*/
    }
  OUT(Show_Enclosure);
  }

static
void Enclosure_Bounds( class zipedit	      *self, zip_type_pane	       pane, zip_type_pixel	      *L , zip_type_pixel	      *T , zip_type_pixel	      *W , zip_type_pixel	      *H )
        {
  zip_type_pixel	      left = EnclosureLeft,  top = EnclosureTop,
				      width = EnclosureWidth, height = EnclosureHeight;
  IN(Enclosure_Bounds);
  if ( width  < 0 )
    { *L = left + width;  *W = -width; }
    else
    { *L = left;  *W = width; }
  if ( height < 0 )
    { *T = top  + height; *H = -height; }
    else
    { *T = top; *H = height; }
  OUT(Enclosure_Bounds);
  }

static
void Show_Names( class zipedit	       *self, zip_type_pane	        pane )
      {
  zip_type_figure	       name_figure;

  if ( PalettesExposed  &&  NamePalette )
    {
    name_figure = (Data)->Figure(  ZIP_current_figure_text_name );
    (View)->Clear_Figure(  name_figure, NamesPane );
    (Data)->Set_Figure_Text(  name_figure, (Data)->Figure_Name(  CurrentFigure ) );
    (View)->Draw_Figure(  name_figure, NamesPane );

    name_figure = (Data)->Figure(  ZIP_current_image_text_name );
    (View)->Clear_Figure(  name_figure, NamesPane );
    (Data)->Set_Figure_Text(  name_figure, (Data)->Image_Name(  CurrentImage ) );
    (View)->Draw_Figure(  name_figure, NamesPane );

    name_figure = (Data)->Figure(  ZIP_current_stream_text_name );
    (View)->Clear_Figure(  name_figure, NamesPane );
    (Data)->Set_Figure_Text(  name_figure, (Data)->Stream_Name(  CurrentStream ) );
    (View)->Draw_Figure(  name_figure, NamesPane );
    }
  }

static
void Show_Point( class zipedit	      *self, zip_type_pane	       pane, zip_type_figure	       figure, int			       point )
          {
  zip_type_figure	      name_figure;
  char				      msg[257];
  zip_type_point		      X, Y;

  IN(Show_Point);
  if ( PalettesExposed  &&  AttributePalette )
    {
    (Objects(figure->zip_figure_type))->Object_Point(  figure, point, &X, &Y );
    name_figure = (Data)->Figure(  ZIP_current_x_point_text_name );
    (View)->Clear_Figure(  name_figure, AttributesPane );
    sprintf( msg, "%ld", X );
    (Data)->Set_Figure_Text(  name_figure, msg);
    (View)->Draw_Figure(  name_figure, AttributesPane );

    name_figure = (Data)->Figure(  ZIP_current_y_point_text_name );
    (View)->Clear_Figure(  name_figure, AttributesPane );
    sprintf( msg, "%ld", Y );
    (Data)->Set_Figure_Text(  name_figure, msg);
    (View)->Draw_Figure(  name_figure, AttributesPane );
    }
  OUT(Show_Point);
  }

static
void Show_Font( class zipedit	      *self, zip_type_pane	       pane, char			      *font_name )
        {
  zip_type_figure	      name_figure;

  IN(Show_Font);
  if ( PalettesExposed  &&  AttributePalette )
    {
    name_figure = (Data)->Figure(  ZIP_current_font_text_name );
    (View)->Clear_Figure(  name_figure, AttributesPane );
    (Data)->Set_Figure_Text(  name_figure, font_name );
    (View)->Draw_Figure(  name_figure, AttributesPane );
    }
  OUT(Show_Font);
  }

static
void Clear_Font( class zipedit	      *self, zip_type_pane	       pane )
      {
  zip_type_figure	      name_figure;

  IN(Clear_Font);
  if ( PalettesExposed  &&  AttributePalette )
    {
    name_figure = (Data)->Figure(  ZIP_current_font_text_name );
    (View)->Clear_Figure(  name_figure, AttributesPane );
    (Data)->Set_Figure_Text(  name_figure, "" );
    }
  OUT(Clear_Font);
  }

static
void Set_Constraints( class zipedit	      *self, zip_type_pane	       pane, zip_type_pixel	       x  , zip_type_pixel	       y, zip_type_point	      *X , zip_type_point	      *Y )
          {
  double		      point_offset, point_delta;
    double gsize = 1.0;

  IN(Set_Constraints);
  *X = (View)->X_Pixel_To_Point(  pane, NULL, x );
  *Y = (View)->Y_Pixel_To_Point(  pane, NULL, y );
    if ( pane->zip_pane_edit->zip_pane_edit_constrain_points )
    {

	if (pane->zip_pane_edit->zip_pane_edit_coordinate_grid)
	    gsize = pane->zip_pane_edit->zip_pane_edit_coordinate_grid;

	point_delta = pane->zip_pane_edit->zip_pane_edit_mark_point_delta / gsize;

point_offset = abs(abs((double)*X) -
			((int)(abs((double)*X) / point_delta) * point_delta));
    if ( point_offset > point_delta/2.0 )
      if ( *X > 0 )
        *X += (long) (point_delta - point_offset);
	else
        *X -= (long) (point_delta - point_offset);
      else
      if ( *X > 0 )
        *X -= (long) point_offset;
	else
        *X += (long) point_offset;
    point_offset = abs(abs((double)*Y) -
			((int)(abs((double)*Y) / point_delta) * point_delta));
    if ( point_offset > point_delta/2.0 )
      if ( *Y > 0 )
        *Y += (long) (point_delta - point_offset);
	else
        *Y -= (long) (point_delta - point_offset);
      else
      if ( *Y > 0 )
        *Y -= (long) point_offset;
	else
        *Y += (long) point_offset;
    }
  OUT(Set_Constraints);
  }
