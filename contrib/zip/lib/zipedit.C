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
/* zipedit.c	Zip EditView-object				      */
/* Author	TC Peters					      */
/* Information Technology Center	   Carnegie-Mellon University */


/**  SPECIFICATION -- External Facility Suite  *********************************

TITLE	The Zip EditView-object

MODULE	zipedit.c

NOTICE	IBM Internal Use Only

DESCRIPTION
	This is the suite of Methods that support the Zip EditView-object.

    NB: The comment-symbol "===" indicates areas which are:
	    1 - Questionable
	    OR
	    2 - Arbitrary
	    OR
	    3 - Temporary Hacks
   Such curiosities need be resolved prior to Project Completion...

HISTORY
  03/31/88	Created for ATK (TCP)
  10/12/88	Fixed enumeration clash warning (TCP)
  05/01/89	Use symbolic font-names (TCP)
  05/07/89	Restore fix to prevent Background loop (TCP)
  05/26/89	Suppress Hide-Palette - till Form inset used (TCP)
		Deal with WM vs X Background Lightening curiosity
  07/24/89	Terminate-Editing: reset Processors to NULL (TCP)
  10/07/89      Changed chained assignment in InitializeObject to single
                assignment to satisfy MIPS compiler. (zs01)
   08/14/90	Added Show_Enclosure in Update to improve enclosure behavior (SCG)

END-SPECIFICATION  ************************************************************/

#include <andrewos.h>
#include "rect.h"
#include "graphic.H"
#include "view.H"
#include "proctable.H"
#include "menulist.H"
#include "keymap.H"
#include "keystate.H"
#include "im.H"
#include "cursor.H"
#include "bind.H"
#include "message.H"
#include "filetype.H"
#include <ctype.h>
#include "raster.H"
#include "rasterview.H"
#include "zip.H"
#include "zipobject.H"
#include "zipedit.H"
#include "zipview.H"
#include "zipedit.h"

static boolean debug=TRUE;
static class menulist		     *class_menulist;
static class keymap		     *class_keymap;

#define  menu_default		      (1<<0)
#define  menu_app		      (1<<1)
#define  menu_grid_expose	      (1<<10)
#define  menu_grid_hide		      (1<<11)
#define	 menu_points_constrain	      (1<<12)
#define	 menu_points_unconstrain      (1<<13)
#define  menu_object_absolute	      (1<<14)
#define  menu_object_relative	      (1<<15)
#define  menu_coordinates_expose      (1<<16)
#define  menu_coordinates_hide	      (1<<17)
#define  menu_palettes_expose	      (1<<18)
#define  menu_palettes_hide	      (1<<19)
#define  menu_selection		      (1<<20)
#define  menu_background	      (1<<21)
#define  menu_background_hide	      (1<<22)
#define  menu_background_expose	      (1<<23)
#define  menu_pan_alone		      (1<<24)
#define  menu_pan_together	      (1<<25)
#define  menu_stream_expose	      (1<<26)
#define  menu_stream_hide	      (1<<27)
#define  menu_background_select	      (1<<28)
#define  menu_background_unselect     (1<<29)
#define  menu_fit		      (1<<30)
#define	 menu_size_grid		      (1<<31)

#define  pending_redisplay	      (1<<1)
#define  pending_redraw		      (1<<2)
#define  pending_palettes	      (1<<3)
#define  pending_delete		      (1<<4)
#define  pending_grid		      (1<<5)
#define  pending_double_grid	      (1<<6)
#define  pending_halve_grid	      (1<<7)
#define  pending_coordinates	      (1<<8)
#define	 pending_constrain	      (1<<9)

#define  Pending_redisplay	      Action & pending_redisplay
#define  Set_pending_redisplay	      Action |= pending_redisplay
#define  Reset_pending_redisplay      Action ^= pending_redisplay

#define  Pending_redraw		      Action & pending_redraw
#define  Set_pending_redraw	      Action |= pending_redraw
#define  Reset_pending_redraw	      Action ^= pending_redraw

#define  Pending_delete	    	      Action & pending_delete
#define  Set_pending_delete	      Action |= pending_delete
#define  Reset_pending_delete	      Action ^= pending_delete

#define  Pending_coordinates          Action & pending_coordinates
#define  Set_pending_coordinates      Action |= pending_coordinates
#define  Reset_pending_coordinates    Action ^= pending_coordinates

#define  Pending_grid		      Action & pending_grid
#define  Set_pending_grid	      Action |= pending_grid
#define  Reset_pending_grid	      Action ^= pending_grid
#define  Pending_double_grid	      Action & pending_double_grid
#define  Set_pending_double_grid      Action |= pending_double_grid
#define  Reset_pending_double_grid    Action ^= pending_double_grid
#define  Pending_halve_grid	      Action & pending_halve_grid
#define  Set_pending_halve_grid       Action |= pending_halve_grid
#define  Reset_pending_halve_grid     Action ^= pending_halve_grid

#define	 Set_pending_constrain	      Action |= pending_constrain
#define	 Reset_pending_constrain      Action ^= pending_constrain
#define  Pending_constrain            Action & pending_constrain

#define  Pending_palettes	      Action & pending_palettes
#define  Set_pending_palettes	      Action |= pending_palettes
#define  Reset_pending_palettes	      Action ^= pending_palettes















ATKdefineRegistry(zipedit, ATK, zipedit::InitializeClass);
static void Fit_Command( class zipedit	    *self );
static void Duplicate_Command( class zipedit	      *self );
static void To_Front_Command( class zipedit	       *self );
static void To_Rear_Command( class zipedit	       *self );
static void Manipulate_Pane( class zipedit	       *self, int			        action );
static void Delete_Command( class zipedit     *self );
static void Expose_Coordinates_Command( class zipedit      *self );
static void Hide_Coordinates_Command( class zipedit      *self );
static void Expose_Grid_Command( class zipedit      *self );
static void Hide_Grid_Command( class zipedit     *self );
static void Grid_Double_Command( class zipedit      *self );
static void Grid_Halve_Command( class zipedit      *self );
static void Constrain_Points_Command( class zipedit  *self );
static void Unconstrain_Points_Command( class zipedit  *self );
static int Insert_File_By_Name( class zipedit	      *self, char			      *name );
static void Insert_File( class zipedit	      *self );
static void Insert_Reference_Command( class zipedit	       *self );
static void Insert_Stream_Command( class zipedit	       *self );
static void Object_Absolute_Command( class zipedit      *self );
static void Object_Relative_Command( class zipedit         *self );
static long Display_Processor( class zipedit	      *self, zip_type_pane	       pane, long			       action );
static void Background_Command( class zipedit	      *self );
static void Hide_Background_Command( class zipedit	      *self );
static void Expose_Background_Command( class zipedit	      *self );
void zipedit_Display_Background_Pane( class zipedit	      *self, zip_type_pane	       pane );
static void Lighten_Background( class zipedit	      *self );
static void Lighten_Background_Command( class zipedit	      *self );
static void Darken_Background_Command( class zipedit	      *self );
static void Select_Background_Command( class zipedit	      *self );
static void Unselect_Background_Command( class zipedit	      *self );
static void Build_Menu();
static void Accept_Character( class zipedit	      *self, char			       c );
static void Pending_Delete( class zipedit	     *self, zip_type_pane	      pane );
void zipedit_Expose_Selection_Menu( class zipedit	    *self );
void zipedit_Hide_Selection_Menu( class zipedit	    *self );
static void Pending_Palettes( class zipedit	    *self, zip_type_pane	     pane );
static void Pending_Coordinates( class zipedit	    *self, zip_type_pane	     pane );
static void Pending_Grid( class zipedit	    *self, zip_type_pane	     pane );
static void Pending_Constrain( class zipedit	    *self, zip_type_pane	     pane );
static void Pending_Double_Grid( class zipedit	    *self, zip_type_pane	     pane );
static void Pending_Redisplay( class zipedit	    *self, zip_type_pane	     pane );
static void Pending_Redraw( class zipedit	    *self, zip_type_pane	     pane );
static void Pending_Halve_Grid( class zipedit	    *self, zip_type_pane	     pane );
int zipedit_Prepare_Editing_Control( class zipedit		  *self, zip_type_pane		   pane );
int zipedit_Reset_Editing_Control( class zipedit		  *self, zip_type_pane		   pane );
void zipedit_Reset_Editing_Selection( class zipedit	       *self, zip_type_pane		   pane );


boolean 
zipedit::InitializeClass( )
    {
  struct proctable_Entry    *proc;
  char				      string[2];

/*===debug=1;===*/
  /*IN(zipedit::InitializeClass );*/
  class_keymap = new keymap;
  proc = proctable::DefineProc( "self-insert", (proctable_fptr) Accept_Character,
				&zipedit_ATKregistry_ , NULL, "Enter Character" );
  string[1] = 0;
  for ( *string = ' '; *string < 127; (*string)++ )
    (class_keymap)->BindToKey(  string, proc, *string );
  *string = '\010';  (class_keymap)->BindToKey(  string, proc, *string );
  *string = '\177';  (class_keymap)->BindToKey(  string, proc, *string );
  *string = '\012';  (class_keymap)->BindToKey(  string, proc, *string );
  *string = '\015';  (class_keymap)->BindToKey(  string, proc, *string );
  class_menulist = new menulist;
  Build_Menu();
  OUT(zipedit::InitializeClass );
  return TRUE;
  }


zipedit::zipedit( )
      {
	ATKinit;
	class zipedit *self=this;

  IN(zipedit::zipedit );
  Action = (long)view_NoMouseEvent;
  Data = NULL;
  View = NULL;
  this->keyboard_processor = NULL;
  PendingProcessor = NULL;
  KeyboardAnchor = PendingAnchor = 0;
  IconFont = PointsFont = DotsFont = NULL;
  EnclosedFigures = NULL;
  DuplicateSelection = false;
  GridExposed = false;
  PointsConstrained = false;
  CoordinatesExposed = false;
  PalettesExposed = false;
  BackgroundExposed = false;
  BackgroundSelected = false;
  BackgroundLightened = false;
  EnclosureExposed = false;
  ForegroundPanning = false;
  PriorX = PriorY = 0;
  FontFamily = 0;
  FontHeight = 12;
  FontBold = FontItalic = 0;
  FontVertical = zip_middle;
  FontHorizontal = zip_center;
  OUT(zipedit::zipedit );
  THROWONFAILURE( TRUE);
  }

zipedit::~zipedit( )
      {
	ATKinit;
	class zipedit *self=this;

  IN(zipedit::~zipedit );
  DEBUGst( Pane-name, Pane->zip_pane_name );
/*===*/
  if ( Menu )	    delete Menu ;
  if ( KeyState )   delete KeyState ;
  OUT(zipedit::~zipedit );
  }


extern void Show_Enclosure( class zipedit *self, zip_type_pane pane );

void 
zipedit::Update( )
    {
    class zipedit *self=this;
  IN(zipedit::Update);
  DEBUGst( Pane-name, PANE->zip_pane_name );
  if ( Pending_delete )		    Pending_Delete( this, PANE );
  if ( Pending_coordinates )	    Pending_Coordinates( this, PANE );
  if ( Pending_grid )		    Pending_Grid( this, PANE );
  if ( Pending_constrain )          Pending_Constrain( this, PANE );
  if ( Pending_double_grid )	    Pending_Double_Grid( this, PANE );
  if ( Pending_halve_grid )	    Pending_Halve_Grid( this, PANE );
  if ( Pending_palettes )	    Pending_Palettes( this, PANE );
  if ( Pending_redisplay )    	    Pending_Redisplay( this, PANE );
  if ( Pending_redraw )    	    Pending_Redraw( this, PANE );
  if ( EnclosureExposed )
    Show_Enclosure( this, PANE );
  OUT(zipedit::Update);
  }

long
zipedit::Set_Data_Object( class zip	    	       *data_object )
      {
    class zipedit *self=this;
  IN(zipedit::Set_Data_Object);
  Data = data_object;
  OUT(zipedit::Set_Data_Object);
  return zip_ok;
  }

long
zipedit::Set_View_Object( class zipview    	       *view_object )
      {
    class zipedit *self=this;
  IN(zipedit::Set_View_Object);
  View = view_object;
  KeyState = keystate::Create( this, class_keymap );
  KeyState->next = NULL;
  (View)->PostKeyState(  KeyState );
  Menu = (class_menulist)->DuplicateML(  View );
  (Menu)->SetMask(  ((Menu )->GetMask( ) | menu_default |
	menu_grid_expose | menu_coordinates_expose | menu_stream_expose |
	menu_points_constrain | menu_background_select | menu_fit |
	menu_object_relative)); /* Set zip to be in absolute mode by default */
  OUT(zipedit::Set_View_Object);
  return zip_ok;
  }

void
zipedit::Set_Debug( boolean state )
      {
  IN(zipedit::Set_Debug);
  debug = state;
  OUT(zipedit::Set_Debug);
  }

/*===
static void
Group_Command( self )
  struct zipedit	     *self;
  {
  zip_type_figure	      figure = NULL, peer_figure = NULL;
  zip_type_image		      image;
  char				      name[512];
  static long			      name_serial = 1;
  zip_type_pane	      pane = PANE;

  IN(Group_Command);
  sprintf( name, "ZIP-GROUP-%d", name_serial++ );
  if ( CurrentImage )
    zip_Create_Inferior_Image( Data, &image, name, CurrentStream, CurrentImage );
  image->zip_image_attributes.zip_image_attribute_group = true;
  while ( figure = zipedit_Next_Selected_Figure( self, PANE, figure ) )
    {
    DEBUGst(Grouping Figure-name,figure->zip_figure_name );
    zip_Unhook_Figure( Data, figure );
    if ( peer_figure )
      zip_Hook_Figure( Data, figure, peer_figure );
      else
      {
      image->zip_image_figure_anchor = figure;
      figure->zip_figure_image = image;
      figure->zip_figure_state.zip_figure_state_unhooked = false;
      }
    peer_figure = figure;
    }
  OUT(Group_Command);
  }

static void
Ungroup_Command( self )
  struct zipedit	      *self;
  {
  zip_type_figure	      figure;

  IN(Ungroup_Command);
  if ( figure = zipedit_Next_Selected_Figure( self, PANE, NULL ) )
    figure->zip_figure_image->zip_image_attributes.zip_image_attribute_group = false;
  OUT(Ungroup_Command);
  }
===*/

extern void zipedit_Cancel_Enclosure( class zipedit *self, zip_type_pane pane );
extern void zipedit_Enclose_Figure( class zipedit *self, zip_type_figure figure, zip_type_pane pane );

static void
Fit_Command( class zipedit	    *self )
    {
  zip_type_pane	    pane = PANE;
  zip_type_point	    x, y;
  float		    scale, EW, EH;

  IN(Fit_Command);
  if ( CurrentImage )
    {
    if ( CurrentFigure  &&  !EnclosureExposed )
      {
      zipedit_Enclose_Figure( self, CurrentFigure, pane );
      zipedit_Reset_Editing_Selection( self, pane );
      CurrentFigure = NULL;
      BuildPending = 0;
      } 
    scale = PANE->zip_pane_scale;
    EW = abs(EnclosureWidth);
    EH = abs(EnclosureHeight);
    DEBUGgt(Old Scale,scale);
    DEBUGgt(Enclosure-Width,EW);
    DEBUGdt(Pane-Width,(View)->Pane_Width(  pane ));
    DEBUGgt(Pct-A,EW / (View)->Pane_Width(  pane ));
    DEBUGgt(Pct-B,1.0/(EW / (View)->Pane_Width(  pane )));
    if ( EW > EH )
      scale *= (1.0/(EW / (View)->Pane_Width(  pane )));
      else
      scale *= (1.0/(EH / (View)->Pane_Height(  pane )));
    DEBUGgt(New Scale,scale);
    x = (View)->X_Pixel_To_Point(  PANE, NULL, EnclosureLeft + EnclosureWidth/2 );
    y = (View)->Y_Pixel_To_Point(  PANE, NULL, EnclosureTop  + EnclosureHeight/2 );
    zipedit_Cancel_Enclosure( self, pane );
    (View)->Scale_Pane_To_Point(  PANE, x, y, scale,
			    zip_center|zip_middle );
    }
  OUT(Fit_Command);
  }


static void
Duplicate_Command( class zipedit	      *self )
    {
  zip_type_pane	      pane = PANE;

  IN(Duplicate_Command);
  if ( CurrentImage )
    {
    if ( CurrentFigure  &&  !EnclosureExposed )
      {
      zipedit_Enclose_Figure( self, CurrentFigure, pane );
      zipedit_Reset_Editing_Selection( self, pane );
      CurrentFigure = NULL;
      BuildPending = 0;
      }
    DuplicateSelection = true;
    (View)->Announce(  ". . . Move Duplication to Desired Position." );
    }
  OUT(Duplicate_Command);
  }
/*===
static void
Replicate_Command( self )
  struct zipedit	      *self;
  {

  }

static void
Cut_Command( self )
  struct zipedit	      *self;
  {

  }

static void
Copy_Command( self )
  struct zipedit	      *self;
  {

  }
===*/


extern zip_type_figure zipedit_Next_Selected_Figure( class zipedit *self, zip_type_pane pane, zip_type_figure figure );

static void
To_Front_Command( class zipedit	       *self )
    {
  zip_type_pane	      pane = PANE;
  zip_type_figure	      figure = NULL,
				      peer_figure = CurrentImage->zip_image_figure_anchor;

  while ( peer_figure  &&  peer_figure->zip_figure_next )
    peer_figure = peer_figure->zip_figure_next;
  while ( ( figure = zipedit_Next_Selected_Figure( self, PANE, figure ) ) )
    {
    if ( figure != peer_figure )
      {
      DEBUGst(Fronting Figure-name,figure->zip_figure_name );
      (Data)->Unhook_Figure(  figure );
      if ( peer_figure )
        (Data)->Hook_Figure(  figure, peer_figure );
        else
        {
        CurrentImage->zip_image_figure_anchor = figure;
        figure->zip_figure_image = CurrentImage;
        figure->zip_figure_state.zip_figure_state_unhooked = false;
        }
      peer_figure = figure;
      }
    }
  Set_pending_redraw;
  (View)->WantUpdate(  View );
  }

static void
To_Rear_Command( class zipedit	       *self )
    {
  zip_type_pane	      pane = PANE;
  zip_type_figure	      figure = NULL,
				      peer_figure = NULL;

  while ( ( figure = zipedit_Next_Selected_Figure( self, PANE, figure ) ) )
    {
    if ( figure != peer_figure )
      {
      DEBUGst(Rearing Figure-name,figure->zip_figure_name );
      (Data)->Unhook_Figure(  figure );
      if ( peer_figure )
        (Data)->Hook_Figure(  figure, peer_figure );
        else
        {
        figure->zip_figure_next = CurrentImage->zip_image_figure_anchor;
        CurrentImage->zip_image_figure_anchor = figure;
        figure->zip_figure_image = CurrentImage;
        figure->zip_figure_state.zip_figure_state_unhooked = false;
        }
      peer_figure = figure;
      }
    }
  Set_pending_redraw;
  (View)->WantUpdate(  View );
  }
/*===
static void
Foreward_Command( self )
  struct zipedit	      *self;
  {

  }

static void
Rearward_Command( self )
  struct zipedit	      *self;
  {

  }
===*/

static void
Manipulate_Pane( class zipedit	       *self, int			        action )
      {

  IN(Manipulate_Pane);
  Action |= action;
  (View)->WantUpdate(  View );
  OUT(Manipulate_Pane);
  }

static void
Delete_Command( class zipedit     *self )	      {  Manipulate_Pane( self, pending_delete );  }

static void
Expose_Coordinates_Command( class zipedit      *self )   {  Manipulate_Pane( self, pending_coordinates );  }

static void
Hide_Coordinates_Command( class zipedit      *self )   {  Manipulate_Pane( self, pending_coordinates );  }

static void
Expose_Grid_Command( class zipedit      *self )    {  Manipulate_Pane( self, pending_grid );  }

static void
Hide_Grid_Command( class zipedit     *self )      {  Manipulate_Pane( self, pending_grid );  }

static void
Grid_Double_Command( class zipedit      *self )   { if ( GridExposed | PointsConstrained )  Manipulate_Pane( self, pending_double_grid ); }

static void
Grid_Halve_Command( class zipedit      *self )    { if ( GridExposed | PointsConstrained )  Manipulate_Pane( self, pending_halve_grid ); }

static void
Constrain_Points_Command( class zipedit  *self ) {  Manipulate_Pane( self, pending_constrain ); }

static void
Unconstrain_Points_Command( class zipedit  *self ) {  Manipulate_Pane( self, pending_constrain ); }

/*===
static void
Expose_Palettes_Command( self )	    struct zipedit     *self;
  {  Manipulate_Pane( self, pending_palettes );  }

static void
Hide_Palettes_Command( self )	    struct zipedit     *self;
  {  Manipulate_Pane( self, pending_palettes );  }
===*/

static
int Insert_File_By_Name( class zipedit	      *self, char			      *name )
      {
  int			      status = zip_ok;
  char				      msg[512];
  char				      full_name[257];

  IN(Insert_File_By_Name);
  DEBUGst( Name, name );
  filetype::CanonicalizeFilename( full_name, name, 256 );
  DEBUGst( Full-name, full_name );
  SetStreamModified;
  if ( (Data)->Open_Stream(  &self->data_object->stream, full_name, zip_default ) )
    {
    DEBUG( Open_Stream Failed);
    sprintf( msg, "ZipEdit ERROR: Failed to Insert '%s'", full_name );
    (View)->Announce(  msg );
    }
    else
    {
    DEBUG( Open_Stream Successful);
    (View)->Announce(  "Done" );
    if ( ( status = (Data)->Read_Stream(  Data->stream ) ) )
    {
    DEBUG( Read_Stream Failed);
    sprintf( msg, "ZipEdit ERROR: Failed to Insert '%s'", full_name );
    (View)->Announce(  msg );
    }
    else
    if ( ( status = (View)->Set_Pane_Stream(  PANE, Data->stream ) ) )
      {
      DEBUGdt( ZipEdit ERROR: Set_Pane_Stream Status, status);
      }
      else
      {
      DEBUG( Set_Pane_Stream successful);
      Set_pending_redisplay;
      (View)->WantUpdate(  View );
      }
    }
  OUT(Insert_File_By_Name);
  return  status;
  }

static void
Insert_File( class zipedit	      *self )
    {
  char				     *reply;

  IN(Insert_File);
  if ( (View)->Query_File_Name(  "Enter File Name: ", &reply ) == zip_ok )
    {
    if ( reply  &&  *reply )
      Insert_File_By_Name( self, reply );
    }
  OUT(Insert_File);
  }

static void
Insert_Reference_Command( class zipedit	       *self )
    {
  IN(Insert_Reference_Command);
  Insert_File( self );
  OUT(Insert_Reference_Command);
  }

static void
Insert_Stream_Command( class zipedit	       *self )
    {
  IN(Insert_Stream_Command);
  Insert_File( self );
/*===
  free( Data->stream_file_name );
  Data->stream_file_name = NULL;
===*/
  OUT(Insert_Stream_Command);
  }

static void
Object_Absolute_Command( class zipedit      *self )
    {
  IN(Object_Absolute_Command);
  self->data_object->object_width = self->view_object->block.width;
  self->data_object->object_height = self->view_object->block.height;
  DEBUGdt(ObjectWidth,self->data_object->object_width);
  DEBUGdt(ObjectHeight,self->data_object->object_height);
  (self->view_object)->Set_Pane_Object_Width(  PANE, self->data_object->object_width );
  (self->view_object)->Set_Pane_Object_Height(  PANE, self->data_object->object_height );
  (Menu)->SetMask(  ((self->menu )->GetMask( ) & ~menu_object_absolute)
			    | menu_object_relative );
  (self->view_object)->PostMenus(  self->menu );
  (self->view_object)->Display_Pane(  PANE );
  OUT(Object_Absolute_Command);
  }

static void
Object_Relative_Command( class zipedit         *self )
    {
  IN(Object_Relative_Command);
  self->data_object->object_width = self->data_object->object_height = 0;
  (self->view_object)->Set_Pane_Object_Width(  PANE, 0 );
  (self->view_object)->Set_Pane_Object_Height(  PANE, 0 );
  (Menu)->SetMask(  ((self->menu )->GetMask( ) & ~menu_object_relative)
			    | menu_object_absolute );
  (self->view_object)->PostMenus(  self->menu );
  (self->view_object)->Display_Pane(  PANE );
  OUT(Object_Relative_Command);
  }

static long
Display_Processor( class zipedit	      *self, zip_type_pane	       pane, long			       action )
        {
  IN(Display_Processor);
  zipedit_Display_Background_Pane( self, pane );
  if ( ! BackgroundSelected )
    {
    (View)->Set_Pane_Display_Processor(  pane, NULL, 0 );
    (View)->Draw_Pane(  pane );
    (View)->Set_Pane_Display_Processor(  pane, (zip_panedisplayproc) Display_Processor, (long) self );
    }
  OUT(Display_Processor);
  return  zip_ok;
  }

static void
Background_Command( class zipedit	      *self )
    {
  char				     *reply;
  long			      status;
  char				      msg[512];
  zip_type_pane	      pane = PANE;
  FILE			     *file;

  IN(Background_Command);
   if ( (View)->Query_File_Name(  "Enter Background File Name: ", &reply ) == zip_ok )
    {
    if ( reply  &&  *reply )
      {
      DEBUGst(Reply,reply);
      if ( (status = (View)->Create_Nested_Pane(  &BackgroundPane,
			    "ZIP-BACKGROUND-PANE", PANE, zip_default )) == zip_ok )
	{
	(View )->Use_Working_Pane_Cursors( );
	BackgroundData = new raster;
	BackgroundView = new rasterview;
	(BackgroundView)->SetDataObject(  BackgroundData );
	file = fopen( reply, "r" );
	(BackgroundData)->Read(  file, 0 );
	BackgroundExposed = true;
	zipedit_Display_Background_Pane( self, PANE );
        (View)->Draw_Pane(  PANE );
	(View)->Set_Pane_Display_Processor(  pane, (zip_panedisplayproc) Display_Processor, (long) self );
	(View )->Use_Normal_Pane_Cursors( );
	}
      if ( status == zip_ok )
	{
	(View)->Announce(  "Done" );
	(Menu)->SetMask(  (Menu )->GetMask( ) |
			    menu_background | menu_background_expose |
			    menu_background_select |  menu_pan_together );
	(View)->PostMenus(  Menu );
	}
	else
	{
	sprintf( msg, "ZipEdit ERROR: Failed to Set Background '%s', Status = %ld", reply, status );
	(View)->Announce(  msg );
	}
      }
    }
  OUT(Background_Command);
  }

static void
Hide_Background_Command( class zipedit	      *self )
    {
  IN(Hide_Background_Command);
  (Menu)->SetMask(  ((Menu )->GetMask( ) & ~menu_background_expose) |
			   menu_background_hide );
  (View)->PostMenus(  Menu );
  BackgroundExposed = false;
  (View)->Remove_Pane(  PANE->zip_pane_edit->zip_pane_edit_background_pane );
  (View)->Display_Pane(  PANE );
  OUT(Hide_Background_Command);
  }

static void
Expose_Background_Command( class zipedit	      *self )
    {
  IN(Expose_Background_Command);
  (Menu)->SetMask(  ((Menu )->GetMask( ) & ~menu_background_hide)
			    | menu_background_expose );
  (View)->PostMenus(  Menu );
  BackgroundExposed = true;
  zipedit_Display_Background_Pane( self, PANE );
  (View)->Draw_Pane(  PANE );
  OUT(Expose_Background_Command);
  }

void zipedit_Display_Background_Pane( class zipedit	      *self, zip_type_pane	       pane )
      {
  long			      left = (View)->Pane_Left(  pane )+1,
				      top =  (View)->Pane_Top(  pane )+1,
				      width = (View)->Pane_Width(  pane )-2,
				      height = (View)->Pane_Height(  pane )-2;

  IN(zipedit_Display_Background_Pane);
  DEBUGst(Pane-name,pane->zip_pane_name);
  if ( pane  &&  BackgroundPane  &&  BackgroundExposed )
    {
    (View)->Set_Pane_Clip_Area(  pane );
    (View)->SetTransferMode(  graphic_WHITE );
    (View)->EraseRectSize(  left, top, width, height );
    (BackgroundView)->LinkTree(  View );
    (BackgroundView)->InsertViewSize(  View, left, top, width, height );
    (BackgroundView)->FullUpdate(  view_FullRedraw, left, top, width, height );
    if ( BackgroundLightened )
      Lighten_Background( self );
    }
  OUT(zipedit_Display_Background_Pane);
  }

static
void Lighten_Background( class zipedit	      *self )
    {
  zip_type_pane	      pane = PANE;
  long			      left = (View)->Pane_Left(  pane )+1,
				      top =  (View)->Pane_Top(  pane )+1,
				      width = (View)->Pane_Width(  pane )-2,
				      height = (View)->Pane_Height(  pane )-2;

  IN(Lighten_Background_Command);
  if ( BackgroundExposed )
    {
    if ( strcmp( (View )->GetWindowManagerType( ), "AndrewWM" ) == 0 )
      (View)->SetTransferMode(  graphic_WHITE );
      else
      (View)->SetTransferMode(  graphic_AND );
    (View)->FillTrapezoid(  left,top,width, left,top+height,width,
      (View)->Define_Graphic(  (Data)->Define_Font(  ShadeFontName, NULL ), '5' ) );
    if ( !BackgroundSelected )
      {
      (View)->Set_Pane_Display_Processor(  pane, NULL, 0 );
      (View)->Draw_Pane(  PANE );
      (View)->Set_Pane_Display_Processor(  pane, (zip_panedisplayproc) Display_Processor, (long) self );
      }
    }
  OUT(Lighten_Background_Command);
  }

static void
Lighten_Background_Command( class zipedit	      *self )
    {
  IN(Lighten_Background_Command);
  if ( BackgroundExposed )
    {
    BackgroundLightened = true;
    Lighten_Background( self );
    }
  OUT(Lighten_Background_Command);
  }

static void
Darken_Background_Command( class zipedit	      *self )
    {
  IN(Darken_Background_Command);
  if ( BackgroundExposed )
    {
    BackgroundLightened = false;
/*===*/  Expose_Background_Command( self );
    }
  OUT(Darken_Background_Command);
  }

static void
Select_Background_Command( class zipedit	      *self )
    {
  zip_type_pane	      pane = PANE;

  IN(Select_Background_Command);
  BackgroundSelected = BackgroundExposed = true;
  (Menu)->SetMask(  ((Menu )->GetMask( ) & ~menu_background_select)
			    | menu_background_unselect );
  zipedit_Display_Background_Pane( self, PANE );
  Abeyant = on;
  (BackgroundView)->WantInputFocus(  BackgroundView );
  im::ForceUpdate();
  OUT(Select_Background_Command);
  }

static void
Unselect_Background_Command( class zipedit	      *self )
    {
  IN(Unselect_Background_Command);
  (View)->WantInputFocus(  View );
  im::ForceUpdate();
  (Menu)->SetMask(  ((Menu )->GetMask( ) & ~menu_background_unselect)
			    | menu_background_select );
  (Menu )->ClearChain( );
  (View)->PostMenus(  Menu );
  Abeyant = off;
  BackgroundSelected = false;
  (View)->Draw_Pane(  PANE );
  OUT(Unselect_Background_Command);
  }

/*===
static void
Page_New_Command( self )
  struct zipedit	     *self;
  {
  char				     *page_name = "ZIP_PAGE_IMAGE_nnn";
  zip_type_image	      root_image =
	    Pane->zip_pane_current_stream->zip_stream_image_anchor;
  zip_type_image		      page_image;
  zip_type_figure		      page_figure;
  zip_type_figure	      figure;
  long			      status = zip_ok;

  IN(Page_New_Command);
  if ( Data->page_count == 0 )
    {
    DEBUG(Page-count Zero);
    if ( status = zip_Create_Inferior_Image( Data, &page_image, "ZIP_PAGE_IMAGE_001",
      Pane->zip_pane_current_stream, root_image ) )
      {
      DEBUGdt(Status,status);
      }
      else
      {
      zip_Create_Figure( Data, &page_figure, "ZIP_PAGE_FIGURE_001",
		         zip_rectangle_figure*//*===!/, page_image, NULL );
      zip_Set_Figure_Point( Data, page_figure, zip_figure_origin_point,
			    root_image->zip_image_stream->zip_stream_least_x,
			    root_image->zip_image_stream->zip_stream_greatest_y );
      zip_Set_Figure_Point( Data, page_figure, zip_figure_auxiliary_point,
			    root_image->zip_image_stream->zip_stream_greatest_x,
			    root_image->zip_image_stream->zip_stream_least_y );
      figure = root_image->zip_image_figure_anchor;
      root_image->zip_image_figure_anchor = NULL;
      page_figure->zip_figure_next = figure;
      while ( figure )
        {
        DEBUGst(Figure-name,figure->zip_figure_name);
        figure->zip_figure_image = page_image;
        figure = figure->zip_figure_next;
        }
      Data->page_count++;
      }
    }
  if ( status == zip_ok )
    {
    sprintf( page_name, "ZIP_PAGE_IMAGE_%03d", ++Data->page_count );
    DEBUGst(Page-name,page_name);
    zip_Create_Inferior_Image( Data, &page_image, page_name,
	    Pane->zip_pane_current_stream, root_image );
    sprintf( page_name, "ZIP_PAGE_FIGURE_%03d", Data->page_count );
    zip_Create_Figure( Data, &page_figure, page_name,
		       zip_rectangle_figure*//*===!/, page_image, NULL );
    zip_Set_Figure_Point( Data, page_figure, zip_figure_origin_point,-1000,1400 );
    zip_Set_Figure_Point( Data, page_figure, zip_figure_auxiliary_point, 1000,-1400 );
    zipview_Display_Image( View, page_image, Pane );
    }
  OUT(Page_New_Command);
  }

static void
Page_Delete_Command( self )
  struct zipedit	     *self;
  {
  IN(Page_Delete_Command);

  OUT(Page_Delete_Command);
  }

static void
Pan_Alone_Command( self )
  struct zipedit	     *self;
  {
  IN(Pan_Alone_Command);

  OUT(Pan_Alone_Command);
  }

static void
Pan_Together_Command( self )
  struct zipedit	     *self;
  {
  IN(Pan_Together_Command);

  OUT(Pan_Together_Command);
  }
===*/

/*===
static void
Expose_Stream_Command( self )
  struct zipedit	     *self;
  {
  IN(Expose_Stream);

  OUT(Expose_Stream);
  }

static void
Hide_Stream_Command( self )
  struct zipedit	     *self;
  {
  IN(Hide_Stream_Command);

  OUT(Hide_Stream_Command);
  }

static void
ReRead_Stream_Command( self )
  struct zipedit	     *self;
  {
  IN(ReRead_Stream_Command);

  OUT(ReRead_Stream_Command);
  }
===*/

static struct bind_Description	      bound_menu[] =
{
{ "zipedit-insert-stream",  "\033Is",	0,	"File,Insert Stream",	0,
	menu_default | menu_background_select,
	(proctable_fptr) Insert_Stream_Command,	    "...",	NULL },
{ "zipedit-insert-reference","\033Ir",	0,	"File,Insert Reference",0,
	menu_default | menu_background_select,
	(proctable_fptr) Insert_Reference_Command,   "...", NULL },
/*===
{ "zipedit-expose-stream", "File,Expose Stream",	"",
	(proctable_fptr) Expose_Stream_Command,"...",	menu_app | menu_stream_expose | menu_background_select },
{ "zipedit-hide-stream", "File,Hide Stream",	"",
	(proctable_fptr) Hide_Stream_Command,"...",	menu_app | menu_stream_hide | menu_background_select },
{ "zipedit-reread-stream", "File,ReRead Stream",	"",
	(proctable_fptr) ReRead_Stream_Command,"...",	menu_app | menu_stream_hide | menu_background_select },
===*/
{ "zipedit-background",	    "\033BN",	0,	"Pane,Background~90",	    0,
	menu_default | menu_background_select,
	(proctable_fptr) Background_Command,	    "...",	NULL },
{ "zipedit-hide-background","\033BH",	0,	"Background~97,Hide~10",    0,
	menu_background_expose | menu_background_select,
	(proctable_fptr) Hide_Background_Command,    "...",	NULL },
{ "zipedit-expose-background","\033BE",	0,	"Background~97,Expose~10",  0,
	menu_background_hide | menu_background_select,
	(proctable_fptr) Expose_Background_Command,  "...",	NULL },
{ "zipedit-lighten-background","\033BL",    0,	"Background~97,Lighten~20", 0,
	menu_background | menu_background_select,
	(proctable_fptr) Lighten_Background_Command, "...",	NULL },
{ "zipedit-darken-background","\033BD",	0,	"Background~97,Darken~21",  0,
	menu_background | menu_background_select ,
	(proctable_fptr) Darken_Background_Command,  "...",	NULL},
{ "zipedit-select-background","\033BS",	0,	"Background~97,Select~30",  0,
	menu_background | menu_background_select,
	(proctable_fptr) Select_Background_Command,  "...",	NULL },
{ "zipedit-unselect-background","\033BU",0,	"Background~97,Unselect~30",0,
	menu_background | menu_background_unselect,
	(proctable_fptr) Unselect_Background_Command,"...",	NULL },
/*===
{ "zipedit-pan-alone", "Foreground~98,Pan Alone~30",	"",
	Pan_Alone_Command,  "...",	menu_pan_together | menu_background_select },
{ "zipedit-pan-together", "Foreground~98,Pan Together~30",	"",
	Pan_Together_Command,  "...",	menu_pan_alone | menu_background_select },
===*/

{ "zipedit-delete",	"\033SD",	0,	"Selection,Delete~10",	    0,
	menu_selection | menu_background_select,
	(proctable_fptr) Delete_Command,		    "...",	NULL },
/*===
{ "zipedit-group",   "Selection,Group~20",	"",
	(proctable_fptr) Group_Command,"...",	 menu_selection | menu_background_select },
{ "zipedit-ungroup", "Selection,Un-Group~21",	"",
	(proctable_fptr) Ungroup_Command,"...", menu_selection | menu_background_select },
===*/
{ "zipedit-duplicate",	"\033SM",	0,	"Selection,Duplicate~30",   0,
	menu_selection | menu_background_select,
	(proctable_fptr) Duplicate_Command,	    "...",	NULL },
/*===
{ "zipedit-replicate", "Selection,Replicate~31",	"",
	(proctable_fptr) Replicate_Command,"...", menu_selection | menu_background_select },
{ "zipedit-copy", "Selection,Copy~32",	"",
	(proctable_fptr) Copy_Command,"...", menu_selection | menu_background_select },
{ "zipedit-cut", "Selection,Cut~33",	"",
	(proctable_fptr) Cut_Command,"...", menu_selection | menu_background_select },
===*/
{ "zipedit-to-front",	"\033SF",   0,		"Selection,To Front~40",    0,
	menu_selection | menu_background_select,
	(proctable_fptr) To_Front_Command,	    "...",	NULL },
{ "zipedit-to-rear",	"\033SR",   0,		"Selection,To Rear~41",	    0,
	menu_selection | menu_background_select,
	(proctable_fptr) To_Rear_Command,	    "...",	NULL },
{ "zipedit-fit",	"\033SZ",   0,		"Selection,Fit~50",	    0,
	menu_selection | menu_background_select | menu_fit,
	(proctable_fptr) Fit_Command,	    "...",	NULL },
/*===
{ "zipedit-foreward", "Selection,Foreward~42",	"",
	(proctable_fptr) Foreward_Command,"...", menu_selection | menu_background_select },
{ "zipedit-rearward", "Selection,Rearward~43",	"",
	(proctable_fptr) Rearward_Command,"...", menu_selection | menu_background_select },

{ "zipedit-palettes-expose","\033PE",	0,	"Pane,Expose Palettes~80",  0,
	menu_palettes_expose | menu_background_select,
	(proctable_fptr) Expose_Palettes_Command,    "...",  NULL },
{ "zipedit-palettes-hide",  "\033PH",	0,	"Pane,Hide Palettes~80",    0,
	menu_palettes_hide | menu_background_select,
	(proctable_fptr) Hide_Palettes_Command,	    "...",  NULL },
===*/
{ "zipedit-grid-expose",    "\033GE",	0,      "Grid~50,Expose~10",	    0,
	menu_grid_expose | menu_background_select,
	(proctable_fptr) Expose_Grid_Command,	    "...",  NULL },
{ "zipedit-grid-hide",	    "\033GH",	0,	"Grid~50,Hide~10",	    0,
	menu_grid_hide | menu_background_select,
	(proctable_fptr) Hide_Grid_Command,	    "...",  NULL },
{ "zipedit-grid-double",    "\033GD",	0,	"Grid~50,Double~20",	    0,
	menu_size_grid | menu_background_select,
	(proctable_fptr) Grid_Double_Command,	    "...",  NULL },
{ "zipedit-grid-halve",	    "\033GL",	0,	"Grid~50,Halve~21",	    0,
	menu_size_grid | menu_background_select,
	(proctable_fptr) Grid_Halve_Command,	    "...",  NULL },

/*  Added for Constrain and Unconstrain */

{ "zipedit-points-constrain", "\033GCC", 0, "Grid~50,Constrain To Grid~40",  0,
        menu_points_constrain | menu_background_select,
	(proctable_fptr) Constrain_Points_Command,    "...", NULL },
{ "zipedit-points-unconstrain", "\033GCU", 0, "Grid~50,Unconstrain From Grid~40",0,
        menu_points_unconstrain | menu_background_select,
	(proctable_fptr) Unconstrain_Points_Command,    "...", NULL },

{ "zipedit-coordinates-expose","\033GCE",0, "Grid~50,Expose Coordinates~30", 0,
	menu_coordinates_expose | menu_background_select,
	(proctable_fptr) Expose_Coordinates_Command, "...",  NULL },
{ "zipedit-coordinates-hide","\033GCH",	0,  "Grid~50,Hide Coordinates~30",  0,
	menu_coordinates_hide | menu_background_select,
	(proctable_fptr) Hide_Coordinates_Command,   "...",  NULL },
{ "zipedit-absolute",	    "\033OA",	0,  "Object~70,Make Absolute~10",   0,
	menu_object_absolute | menu_background_select,
	(proctable_fptr) Object_Absolute_Command,    "...",  NULL },
{ "zipedit-relative",	    "\033OR",	0,  "Object~70,Make Relative~10",   0,
	menu_object_relative | menu_background_select,
	(proctable_fptr) Object_Relative_Command,    "...",  NULL },
/*===
{ "zipedit-new-page",	    "Page,New~30",	"",
	(proctable_fptr) Page_New_Command,"...",	menu_default | menu_background_select },
{ "zipedit-delete-page",    "Page,Delete~31",	"",
	(proctable_fptr) Page_Delete_Command,"...", menu_default | menu_background_select },
===*/
NULL
};

static
void Build_Menu()
  {
  IN(Build_Menu);
  bind::BindList( bound_menu, class_keymap, class_menulist, &zipedit_ATKregistry_  );
  OUT(Build_Menu);
  }

static void
Accept_Character( class zipedit	      *self, char			       c )
      {
  IN(Accept_Character);
  DEBUGct(C,c);
  if ( self->keyboard_processor )
    {
    DEBUG(>>> Keyboard Processor);
    (*self->keyboard_processor)( KeyboardAnchor, PANE, c, view_NoMouseEvent, 0, 0, 0 );
    DEBUG(<<< Keyboard Processor);
    }
  OUT(Accept_Character);
  }

static
void Pending_Delete( class zipedit	     *self, zip_type_pane	      pane )
      {
  zip_type_figure	     figure;

  IN(Pending_Delete);
  Reset_pending_delete;
  if ( pane->zip_pane_current_image  &&
       pane->zip_pane_edit->zip_pane_edit_selection_level >= ImageSelection )
    (self)->Delete_Image(  pane->zip_pane_current_image, pane );
    else
    if ( pane->zip_pane_current_figure  &&
	 pane->zip_pane_edit->zip_pane_edit_selection_level >= PointSelection )
      (self)->Delete_Figure(  pane->zip_pane_current_figure, pane );
  figure = zipedit_Next_Selected_Figure( self, pane, NULL );
  while ( figure )
    {
    (self)->Delete_Figure(  figure, pane );
    figure = zipedit_Next_Selected_Figure( self, pane, figure );
    }
  zipedit_Cancel_Enclosure( self, pane );
  (View)->Draw_Pane(  pane );
  zipedit_Hide_Selection_Menu( self );
  (Data)->NotifyObservers(  0 );
  OUT(Pending_Delete);
  }

void zipedit_Expose_Selection_Menu( class zipedit	    *self )
    {
  (Menu)->SetMask(  (Menu )->GetMask( ) | menu_selection );
  (View)->PostMenus(  Menu );
  }

void zipedit_Hide_Selection_Menu( class zipedit	    *self )
    {
  (Menu)->SetMask(  (Menu )->GetMask( ) & ~menu_selection );
  (View)->PostMenus(  Menu );
  }

static
void Pending_Palettes( class zipedit	    *self, zip_type_pane	     pane )
      {
  long			    mask = (Menu )->GetMask( );

  IN(Pending_Palettes);
  if ( ( PalettesExposed = ! PalettesExposed ) )
    {
    (self)->Expose_Palettes(  pane );
    mask = (mask & ~menu_palettes_expose) | menu_palettes_hide;
    }
    else
    {
    (self)->Hide_Palettes(  pane );
    mask = (mask & ~menu_palettes_hide) | menu_palettes_expose;
    }
  (Menu)->SetMask(  mask );
  (View)->PostMenus(  Menu );
  Reset_pending_palettes;
  OUT(Pending_Palettes);
  }

static
void Pending_Coordinates( class zipedit	    *self, zip_type_pane	     pane )
      {
  long			    mask = (Menu )->GetMask( );

  IN(Pending_Coordinates);
  if ( ( CoordinatesExposed = ! CoordinatesExposed ) )
    {
    (self)->Expose_Pane_Coordinates(  pane );
    mask = (mask & ~menu_coordinates_expose) | menu_coordinates_hide;
    }
    else
    {
    (self)->Hide_Pane_Coordinates(  pane );
    mask = (mask & ~menu_coordinates_hide) | menu_coordinates_expose;
    }
  (Menu)->SetMask(  mask );
  (View)->PostMenus(  Menu );
  Reset_pending_coordinates;
  OUT(Pending_Coordinates);
  }

static
void Pending_Grid( class zipedit	    *self, zip_type_pane	     pane )
      {
  long			    mask = (Menu )->GetMask( );

  IN(Pending_Grid);
  if ( ( GridExposed = ! GridExposed ))
    {
    (self)->Expose_Pane_Grid(  pane );
    mask = (mask & ~menu_grid_expose) | menu_grid_hide | menu_size_grid;
    }
    else
    {
    (self)->Hide_Pane_Grid(  pane );
    mask = (mask & ~menu_grid_hide) | menu_grid_expose;
    if (!PointsConstrained) mask = mask & ~menu_size_grid;
    }
  (Menu)->SetMask(  mask );
  (View)->PostMenus(  Menu );
  Reset_pending_grid;
  OUT(Pending_Grid);
  }

static
void Pending_Constrain( class zipedit	    *self, zip_type_pane	     pane )
      {
  long			    mask = (Menu )->GetMask( );

  IN(Pending_Constrain);
  if ( ( PointsConstrained = ! PointsConstrained ) )
    {
    (self)->Constrain_Points( pane );
    mask = (mask & ~menu_points_constrain) | menu_points_unconstrain;
    mask = mask | menu_size_grid;
    }
    else
    {
    (self)->Unconstrain_Points( pane );
    mask = (mask & ~menu_points_unconstrain) | menu_points_constrain;
    if (!GridExposed) mask = mask & ~menu_size_grid;
    }
  (Menu)->SetMask(  mask );
  (View)->PostMenus(  Menu );
  Reset_pending_constrain;
  OUT(Pending_Constrain);
  }

static
void Pending_Double_Grid( class zipedit	    *self, zip_type_pane	     pane )
      {
  IN(Pending_Double_Grid);
  (self)->Double_Pane_Grid(  pane );
  Reset_pending_double_grid;
  OUT(Pending_Double_Grid);
  }

static
void Pending_Redisplay( class zipedit	    *self, zip_type_pane	     pane )
      {
  IN(Pending_Redisplay);
  Reset_pending_redisplay;
  (View)->Display_Pane(  pane );
  OUT(Pending_Redisplay);
  }

static
void Pending_Redraw( class zipedit	    *self, zip_type_pane	     pane )
      {
  IN(Pending_Redraw);
  Reset_pending_redraw;
  (View)->Draw_Pane(  pane );
  OUT(Pending_Redraw);
  }

static
void Pending_Halve_Grid( class zipedit	    *self, zip_type_pane	     pane )
      {
  IN(Pending_Halve_Grid);
  (self)->Halve_Pane_Grid(  pane );
  Reset_pending_halve_grid;
  OUT(Pending_Halve_Grid);
  }


extern int zipedit_Redisplay_Edit_Pane( class zipedit *self, zip_type_pane pane );

long
zipedit::Redisplay_Panes( zip_type_pane		   pane )
      {
  IN(zipedit::Redisplay_Panes);
  DEBUGst(Pane-name,pane->zip_pane_name);
  (this->view_object)->Redisplay_Panes( );
  zipedit_Redisplay_Edit_Pane( this, pane );
  OUT(zipedit::Redisplay_Panes);
  return  zip_ok;
  }

long
zipedit::Handle_Editing( zip_type_pane		   pane )
      {
  IN(zipedit::Handle_Editing);
  DEBUGst(Pane-name,pane->zip_pane_name);

  OUT(zipedit::Handle_Editing);
/*===*/return  zip_failure;
  }

long
zipedit::Initialize_Editing( zip_type_pane		   pane )
      {
  int				  status = zip_success;
  long			          mask = (this->menu )->GetMask( );

  IN(zipedit::Initialize_Editing);
  DEBUGst(Pane-name,pane->zip_pane_name);
  if ( pane )
    {
    zipedit_Prepare_Editing_Control( this, pane );
    (this)->Expose_Palettes(  pane );
    (this->view_object)->PostMenus(  this->menu );
    this->keystate->next = NULL;
    (this->view_object)->PostKeyState(  this->keystate );

    if ( this->states.grid_exposed )
    {
	mask = (mask & ~menu_grid_expose) | menu_grid_hide;
    }
    else
    {
	mask = (mask & ~menu_grid_hide) | menu_grid_expose;
    }
  }

  if ( this->states.points_constrained )
    {
    mask = (mask & ~menu_points_constrain) | menu_points_unconstrain;
    }
    else
    {
    mask = (mask & ~menu_points_unconstrain) | menu_points_constrain;
    }
  (this->menu)->SetMask(  mask );
  (this->view_object)->PostMenus(  this->menu );


  OUT(zipedit::Initialize_Editing);
  return status;
  }

long
zipedit::Terminate_Editing( zip_type_pane		   pane )
      {
  int				  status = zip_success;

  IN(zipedit::Terminate_Editing);
  DEBUGst(Pane-name,pane->zip_pane_name);
  if ( pane )
    {
    if ( this->keyboard_processor )
      (*this->keyboard_processor)( this->keyboard_anchor, this->view_object->pane, 0, view_NoMouseEvent, 0, 0, 0 );
    this->keyboard_processor = NULL;
    this->pending_processor = NULL;
    (this->view_object)->Reset_Pane_Display_Processor(  pane );
    zipedit_Cancel_Enclosure( this, pane );
    zipedit_Hide_Selection_Menu( this );
    zipedit_Reset_Editing_Selection( this, pane );
/*=== avoid ???===*/
/*
    pane->zip_pane_state.zip_pane_state_coordinates_exposed = false;
    pane->zip_pane_edit->zip_pane_edit_grid_exposed = false;
      pane->zip_pane_edit->zip_pane_edit_coordinate_grid = 1;

*/

    if ( this->states.palettes_exposed )
      (this)->Hide_Palettes(  pane );
    zipedit_Reset_Editing_Control( this, pane );
    (this->menu)->UnchainML(  0 );
    (this->view_object)->PostMenus(  this->menu );
    }
  OUT(zipedit::Terminate_Editing);
  return status;
  }

int
zipedit_Prepare_Editing_Control( class zipedit		  *self, zip_type_pane		   pane )
      {
  int				  status = zip_success;

  IN(zipedit_Prepare_Editing_Control);
  if ( pane->zip_pane_edit == NULL )
    if ( (pane->zip_pane_edit = (zip_type_pane_edit)
		 calloc( 1, sizeof(struct zip_pane_edit ) )) == NULL )
      {
      status = zip_insufficient_pane_space;
/*=== provide message for failed allocation of pane_edit structure*/
      }
  DEBUGdt(Status,status);
  OUT(zipedit_Prepare_Editing_Control);
  return status;
  }

int
zipedit_Reset_Editing_Control( class zipedit		  *self, zip_type_pane		   pane )
      {
  IN(zipedit_Reset_Editing_Control);
/*=== consider whether state should be left alive for next return into editing... ===*/
  if ( PriorPane )
    (View)->Invert_Pane(  PriorPane );
  PriorPane = NULL;
  pane->zip_pane_edit->zip_pane_edit_last_point_id = 0;
  pane->zip_pane_edit->zip_pane_edit_beginning = 0;
  pane->zip_pane_edit->zip_pane_edit_build_constraint = 0;
  pane->zip_pane_edit->zip_pane_edit_build_pending = 0;
  pane->zip_pane_edit->zip_pane_edit_build_figure = 0;
  pane->zip_pane_edit->zip_pane_edit_selection_level = 0;
/*  pane->zip_pane_edit->zip_pane_edit_coordinate_grid = 1;
  pane->zip_pane_edit->zip_pane_edit_grid_status = 0;
  pane->zip_pane_edit->zip_pane_edit_constrain_points = 0; 
  pane->zip_pane_edit->zip_pane_edit_mark_point_delta = 0;
  pane->zip_pane_edit->zip_pane_edit_mark_point_spacing = 0; */

  OUT(zipedit_Reset_Editing_Control);
  return zip_success;
  }

void zipedit_Reset_Editing_Selection( class zipedit	       *self, zip_type_pane		   pane )
      {
  IN(zipedit_Reset_Editing_Selection);
  BuildPending = 0;
  Building = true;
  if ( CurrentFigure ) {
    if ( SelectionLevel >= ImageSelection )
      (self)->Normalize_Image_Points(  CurrentImage, pane );
      else
      (self)->Normalize_Figure_Points(  CurrentFigure, pane );
  }
  CurrentFigure = NULL;
  SelectionLevel = 0;
  (View)->Set_Pane_Cursor(  pane, 'A', CursorFontName );
  if ( PriorPane )
    {
    if ( FigurePalette )
       (View)->Invert_Pane(  PriorPane );
    (View)->Set_Pane_Inverted(  PriorPane, false );
    PriorPane = NULL;
    }
  OUT(zipedit_Reset_Editing_Selection);
  }
