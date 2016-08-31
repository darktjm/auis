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
 * P_R_P_Q_# (C) COPYRIGHT IBM CORPORATION 1987
 * LICENSED MATERIALS - PROPERTY OF IBM
 * REFER TO COPYRIGHT INSTRUCTIONS FORM NUMBER G120-2083
 */
/* zipv.c	Zip View-object					      */
/* Author	TC Peters					      */
/* Information Technology Center	   Carnegie-Mellon University */


/**  SPECIFICATION -- External Facility Suite  *********************************

TITLE	The Zip View-object

MODULE	zipv.c

NOTICE	IBM Internal Use Only

DESCRIPTION
	This is the suite of Methods that support the Zip View-object.

    NB: The comment-symbol "===" indicates areas which are:
	    1 - Questionable
	    OR
	    2 - Arbitrary
	    OR
	    3 - Temporary Hacks
   Such curiosities need be resolved prior to Project Completion...

HISTORY
  06/16/87	Created (TCP)
  08/06/87	Once more to the breach, dear friends ...
  10/30/87	Remove old ifdef on WM and remove include of mrl.ih (TCP)
  11/09/87	Support "Absolute" as well as "Relative" sizing (TCP)
		Cope with resizing of View-rectangle when not via Menu
		Utilize menu-mask to update Menu-items
  11/12/87	Fix destroying of Null Text figure building (TCP)
  11/25/87	Add more xxx_Destroys to Finalize (TCP)
  12/02/87	Suppress View Menu-card & Flip/Flop Pane Menu items (TCP)
  03/31/88	Revised for ATK (TCP)
  10/12/88	Fix enumeration clash warning (TCP)
  10/19/88	Fix PostKeyState (set keystate->next NULL again) (TCP) 
  10/27/88	Add preference ZipManualRefresh (TCP)
  11/18/88	Check NULL arg in PostKeyState (TCP)
	              Ensure non-null ptr on Query facilities
  05/26/89	Suppress Page feature - till Form inset used (TCP)
  07/12/89	Check for Im existence in Announce (TCP)
   08/16/90	Add Normalize_Line_Attributes in {Highlight,Normalize}_View (SCG)

END-SPECIFICATION  ************************************************************/

#include <andrewos.h>
#include <stdio.h>
#include "environ.H"
#include "graphic.H"
#include "view.H"
#include "proctable.H"
#include "menulist.H"
#include "keymap.H"
#include "keystate.H"
#include "im.H"
#include "cursor.H"
#include "bind.H"
#include "rect.h"
#include "message.H"
#include "completion.H"
#include "filetype.H"
#include <ctype.h>
#include "fontdesc.H"
#include "zip.H"
#include "zipview.H"
#include "zipedit.H"
#include "zipprint.H"
#include "zipobject.H"

static boolean debug=TRUE;
#define	 Data			      (self->data_object)
#define	 View			      self
#define	 Editor			      self->edit_object
#define	 Printer		      self->print_object
#define	 DataObjects(i)		      ((Data->objects)[i])
#define	 Objects(i)		      ((self->objects)[i])
#define	 Pane			      self->pane
#define  PanningPrecision	      self->panning_precision
#define  CurrentPane		      self->current_pane
#define  CurrentPage		      self->current_page
#define	 Edge			      self->edge
#define  Block			      self->block
#define  BlockTop		      self->block.top
#define  BlockLeft		      self->block.left
#define  BlockHeight		      self->block.height
#define  BlockWidth		      self->block.width
#define  Editing		      self->states.editing
#define  FirstTime		      self->states.first_time
#define  Abeyant		      self->states.abeyant
#define  Application		      self->states.application
#define  Action			      self->action
#define  Menu			      self->menu
#define  Keystate		      self->keystatep
#define  Cursor			      self->cursor
#define	 MouseAction		      self->mouse_action
#define	 MouseX			      self->mouse_x
#define	 MouseY			      self->mouse_y
#define	 MouseClicks		      self->mouse_clicks
#define  InputFocus		      self->states.inputfocus

static class menulist		     *class_menulist;
static class keymap		     *class_keymap;
/*===static struct keystate	     *class_keystate;*/

#define  menu_default		     (1<<0)
#define  menu_app		     (1<<1)
#define  menu_edit		     (1<<2)
#define  menu_browse		     (1<<3)
#define  menu_portrait		     (1<<4)
#define  menu_landscape		     (1<<5)

#define  pending_hit		      (1<<1)
#define  pending_redisplay	      (1<<2)
#define  pending_redraw		      (1<<3)
#define  pending_edit		      (1<<4)
#define  pending_browse		      (1<<5)
#define  pending_receiveinputfocus    (1<<6)
#define  pending_loseinputfocus	      (1<<7)
#define  pending_pan		      (1<<8)
#define  pending_scrollbars	      (1<<9)
#define  pending_zoom		      (1<<10)
#define  pending_scale		      (1<<11)

#define  Pending_hit		      Action & pending_hit
#define  Set_pending_hit	      Action |= pending_hit
#define  Reset_pending_hit	      Action ^= pending_hit
#define  Pending_redisplay	      Action & pending_redisplay
#define  Set_pending_redisplay	      Action |= pending_redisplay
#define  Reset_pending_redisplay      Action ^= pending_redisplay
#define  Pending_redraw		      Action & pending_redraw
#define  Set_pending_redraw	      Action |= pending_redraw
#define  Reset_pending_redraw	      Action ^= pending_redraw
#define  Pending_edit		      Action & pending_edit
#define  Set_pending_edit	      Action |= pending_edit
#define  Reset_pending_edit	      Action ^= pending_edit
#define  Pending_browse		      Action & pending_browse
#define  Set_pending_browse	      Action |= pending_browse
#define  Reset_pending_browse	      Action ^= pending_browse
#define  Pending_receiveinputfocus       Action & pending_receiveinputfocus
#define  Set_pending_receiveinputfocus	 Action |= pending_receiveinputfocus
#define  Reset_pending_receiveinputfocus Action ^= pending_receiveinputfocus
#define  Pending_loseinputfocus		 Action & pending_loseinputfocus
#define  Set_pending_loseinputfocus	 Action |= pending_loseinputfocus
#define  Reset_pending_loseinputfocus    Action ^= pending_loseinputfocus
#define  Pending_pan		      Action & pending_pan
#define  Set_pending_pan	      Action |= pending_pan
#define  Reset_pending_pan	      Action ^= pending_pan
#define  Pending_scrollbars	      Action & pending_scrollbars
#define  Set_pending_scrollbars       Action |= pending_scrollbars
#define  Reset_pending_scrollbars     Action ^= pending_scrollbars
#define  Pending_zoom		      Action & pending_zoom
#define  Set_pending_zoom	      Action |= pending_zoom
#define  Reset_pending_zoom           Action ^= pending_zoom
#define  Pending_scale		      Action & pending_scale
#define  Set_pending_scale	      Action |= pending_scale
#define  Reset_pending_scale          Action ^= pending_scale


#ifdef Printing
#undef Printing
#endif
#define Printing self->printing

/* Added for set print size command */
#define	 PelWidth			      Printing->zip_printing_pel_width
#define	 PelHeight			      Printing->zip_printing_pel_height
#define	 PelResolution			      Printing->zip_printing_resolution
#define	 InchWidth			      Printing->zip_printing_inch_width
#define	 InchHeight			      Printing->zip_printing_inch_height
#define	 UserWidth			      Printing->zip_printing_user_width
#define	 UserHeight			      Printing->zip_printing_user_height

ATKdefineRegistry(zipview, view, zipview::InitializeClass);

static void Initialize_Printing( class zipview	      *self );
static void DEBUG_Command( class zipview	       *self );
static void Edit_Command( class zipview	       *self );
static void Browse_Command( class zipview      *self );
static void Portrait_Command( class zipview      *self );
static void Landscape_Command( class zipview        *self );
static void Print_To_File_Command( class zipview        *self );
static void Set_Print_Size_Command(class zipview      *self);
static void Zoom_Pane( class zipview	       *self, int			        factor );
static void Zoom_Normal_Command( class zipview   *self );
static void Zoom_In_Command( class zipview      *self );
static void Zoom_In5_Command( class zipview      *self );
static void Zoom_Out5_Command( class zipview      *self );
static void Zoom_Out_Command( class zipview      *self );
static void Normalize_Pane_Command( class zipview	       *self );
static void Refresh_Pane_Command( class zipview	       *self );
static void Pan_Pane( class zipview	       *self, int			        edge );
static void Center_Pane_Command( class zipview	       *self );
static void Top_Pane_Command( class zipview      *self );
static void Bottom_Pane_Command( class zipview      *self );
static void Left_Pane_Command( class zipview      *self );
static void Right_Pane_Command( class zipview      *self );
static void Scale_Pane( class zipview	       *self, float		        scale );
static void Scale_Normal_Command( class zipview	       *self );
static void Scale_Smaller_Command( class zipview      *self );
static void Scale_Smaller_10_Command( class zipview      *self );
static void Scale_Larger_Command( class zipview      *self );
static void Scale_Larger_10_Command( class zipview      *self );
static void Scale_Half_Command( class zipview	       *self );
static void Scale_Double_Command( class zipview	       *self );
static void Build_Menu();
static void Pending_Hit( class zipview	    *self );
static void Pending_Redisplay( class zipview	    *self );
static void Pending_Redraw( class zipview	    *self );
static void Pending_Zoom( class zipview	    *self );
static void Pending_Scale( class zipview	    *self );
static void Pending_Pan( class zipview	    *self );
static void Pending_ReceiveInputFocus( class zipview	    *self );
static void Pending_LoseInputFocus( class zipview	    *self );
static void Pending_Edit( class zipview	    *self );
static void Pending_Browse( class zipview	    *self );
static void Highlight_View( class zipview	      *self );
static void Normalize_View( class zipview	      *self );
static void Prepare_Default_Pane( class zipview	       *self );
static int Prepare_Default_Stream( class zipview	       *self );
static void zipview_y_getinfo( class zipview	      *self, struct range		      *total , struct range		      *seen , struct range		      *dot );
static long zipview_y_whatisat( class zipview	      *self, long			       coordinate , long			       outof );
static void  zipview_y_setframe( class zipview	      *self, int			       position, long			       coordinate , long			       outof );
static void zipview_y_endzone( class zipview	      *self, int			       zone , int			       action );
static void zipview_x_getinfo( class zipview	      *self, struct range		      *total , struct range		      *seen , struct range		      *dot );
static long zipview_x_whatisat( class zipview	      *self, long			       coordinate , long			       outof );
static void  zipview_x_setframe( class zipview	      *self, int			       position, long			       coordinate , long			       outof );
static void zipview_x_endzone( class zipview	      *self, int			       zone , int			       action );

boolean 
zipview::InitializeClass( )
    {
  IN(zipview_InitializeClass );
  class_menulist = new menulist;
  class_keymap = new keymap;
  Build_Menu();
  OUT(zipview_InitializeClass );
  return TRUE;
  }


zipview::zipview( )
      {
	ATKinit;
  class zipview *self=this;
  IN(zipview_InitializeObject);
  this->options.manual_refresh =
    environ::GetProfileSwitch( "ZipManualRefresh", false );
  Application = Abeyant = Editing = InputFocus = false;
  Data = NULL;
  Editor = NULL;
  Printer = NULL;
  PaneAnchor = NULL;
  Printing = NULL;
/*===  PaneExceptionHandler = NULL;*/
  PanningPrecision = 0;
  FirstTime = true;
  Pane = CurrentPane = NULL;
  CurrentPage = 1;
  Action = 0;
  MouseAction = view_NoMouseEvent;
  this->objects = NULL;
  BlockTop = BlockLeft = BlockHeight = BlockWidth = 0;
  this->imPtr = NULL;/*=== WHY DONE ? ===*/
  Menu = (class_menulist)->DuplicateML(  this );
  (Menu)->SetView(  this );
  Keystate = keystate::Create( this, class_keymap );
  Cursor = cursor::Create( this );
  (Cursor)->SetStandard(  Cursor_Gunsight );
  OUT(zipview_InitializeObject);
  THROWONFAILURE( TRUE);
  }

zipview::~zipview( )
      {
	ATKinit;
  class zipview *self=this;
  IN(zipview_FinalizeObject);
  if ( Cursor )	    delete Cursor ;
  if ( Menu )	    delete Menu ;
  if ( Keystate )   delete Keystate ;
/*===*/
  OUT(zipview_FinalizeObject);
  }

void
zipview::PostKeyState( class keystate	      *keystate )
      {
  class keystate	     *keys;
  class zipview *self=this;
  IN(zipview_PostKeyState);
  if ( keystate != Keystate  &&  Editing  &&  Editor->keystate )
    { DEBUG(Editor-keystate);
    keys = Editor->keystate;
    }
    else
    { DEBUG(View-keystate);
    keys = Keystate;
    }
  if ( keys  &&  keystate != keys )
    { DEBUG(Different);
    keys->next = NULL;
    if ( keystate )
      (keystate)->AddBefore(  keys );
    }
  (this)->view::PostKeyState(  keystate );
  OUT(zipview_PostKeyState);
  }

void
zipview::PostMenus( class menulist	      *menulist )
      {
  class zipview *self=this;
  class menulist	     *menu = Menu;

  IN(zipview_PostMenus);
  (menu )->ClearChain( );
  if ( menulist != menu )
    { DEBUG(Different);
    if ( InputFocus )
      (menu)->ChainAfterML(  menulist, (long) menulist );
      else
      { DEBUG(Not InputFocus);
      menu = menulist;
      if ( Editing  &&  Editor->menu ) /*===dirty===*/
        (menu)->ChainAfterML(  Editor->menu, (long) Editor->menu );
      }
    }
  (this)->view::PostMenus(  menu );
  OUT(zipview_PostMenus);
  }

void
zipview::SetDataObject( class dataobject		       *data )
      {
  int			       i;
  class zipview *self = this;
  IN(zipview_SetDataObject);
  Data = (class zip *) data;
  this->objects = (class zipobject **) calloc( 28, sizeof(class zipobject *) );
  for ( i = 1; DataObjects(i); i++ )
    {
    DEBUGst(Object-name,(DataObjects(i) )->GetTypeName( ));
    Objects(i) = (class zipobject *) 
	ATK::NewObject( (DataObjects(i) )->GetTypeName( ) );
    (Objects(i))->Set_Data_Object(  Data );
    (Objects(i))->Set_View_Object(  this );
    }
  DEBUGdt(Desired-width, this->data_object->desired_view_width);
  DEBUGdt(Desired-height, this->data_object_desired_view_height);
  if ( !(this->data_object->desired_view_width) )   this->data_object->desired_view_width  = zipview_default_block_width;
  if ( !(this->data_object->desired_view_height) )  this->data_object->desired_view_height = zipview_default_block_height;
  DEBUG(super-::SetDataObject);
  (this)->view::SetDataObject(  data );
  OUT(zipview_SetDataObject );
  }

view_DSattributes
zipview::DesiredSize( long			       given_width , long			       given_height,
		      enum view_DSpass	       pass, long			      *desired_width , long			      *desired_height )
          {
  view_DSattributes     result = view_WidthFlexible |
					       view_HeightFlexible;
  class zipview *self=this;
  IN(zipview_DesiredSize);
  DEBUGdt( Given Width, given_width);
  DEBUGdt( Given Height, given_height);
  DEBUGdt( Pass, pass);
  DEBUGdt( Pending-Action,Action);
  *desired_width  = this->data_object->desired_view_width;
  *desired_height = this->data_object->desired_view_height;
  DEBUGdt( Desired Width,  *desired_width);
  DEBUGdt( Desired Height, *desired_height);
  DEBUGxt( Result-code, result );
  OUT(zipview_DesiredSize);
  return result;
  }

void
zipview::ObservedChanged( class observable	      *changed, long			       value )
        {
  class zipview *self=this;
  IN(zipview_ObservedChanged);
  if ( this != (class zipview *) value )
    {
    DEBUG(Not Self Observed);
    Set_pending_redraw;
    (this)->WantUpdate(  this );
    }
  OUT(zipview_ObservedChanged);
  }

void
zipview::SetOptions( int			        options )
      {
  IN(zipview_SetOptions );
  DEBUGxt( Options, options);
/*===*/
  OUT(zipview_SetOptions );
  }

void 
zipview::ReceiveInputFocus( )
    {
  class zipview *self=this;
  IN(zipview_ReceiveInputFocus );
  DEBUGxt(Action,Action);
  Keystate->next = NULL;
  (this)->PostKeyState(  Keystate );
  if ( ! Editing )
    {
    (Menu)->SetMask(  ((Application) ? menu_app : 0) | 
		      menu_default | menu_browse | menu_portrait );
    }
    else
    { DEBUG(Editing);
    Editor->keystate->next = NULL;
    (this)->PostKeyState(  Editor->keystate );
    }
  (Cursor)->SetStandard(  Cursor_Arrow );
  (this)->PostCursor(  &Block, Cursor );
  Set_pending_receiveinputfocus;
  (this)->WantUpdate(  this );
  (this)->PostMenus(  Menu );
  OUT(zipview_ReceiveInputFocus );
  }

void
zipview::LoseInputFocus( )
    {
  class zipview *self=this;
  IN(zipview_LoseInputFocus );
  DEBUGxt(Action,Action);
  if ( ! Abeyant )
    {
    if ( Editing )
      Pending_Browse( this );
    (this)->RetractViewCursors(  this );
    (Cursor)->SetStandard(  Cursor_Gunsight );
    (this)->PostCursor(  &Block, Cursor );
    Set_pending_loseinputfocus;
    (this)->WantUpdate(  this );
    }
  InputFocus = false;
  OUT(zipview_LoseInputFocus );
  }

void
zipview::WantUpdate( class view		      *requestor )
      {
  IN(zipview_WantUpdate);
/*===*/
  (this)->view::WantUpdate(  requestor );
/*===*/
  OUT(zipview_WantUpdate);
  }

void 
zipview::FullUpdate( enum view_UpdateType	       type, long			       left , long			       top , long			       width , long			       height )
        {
  class zipview *self=this;
  IN(zipview_FullUpdate);
  DEBUGdt( Type, type);
  DEBUGlt( Left,  left);   DEBUGlt( Top,   top);
  if ( type == view_FullRedraw || type == view_LastPartialRedraw )
    {
    (this)->GetLogicalBounds(  &Block );
    this->data_object->desired_view_width  = BlockWidth;
    this->data_object->desired_view_height = BlockHeight;
    if ( FirstTime )
      {
      FirstTime = false;
      if ( this->data_object->stream_anchor == NULL )
	Prepare_Default_Stream( this );
      if ( this->pane_anchor == NULL )
	Prepare_Default_Pane( this );
      }
    if ( Editing )
      (Editor)->Redisplay_Panes(  Pane );
      else
      (this )->Redisplay_Panes( );
    (this )->Update( );
    }
  else
  if ( type == view_MoveNoRedraw )
    {
    DEBUG(Move-No-Redraw);
    (this )->Use_Normal_Pane_Cursors( );
    }
  if ( InputFocus )
    Highlight_View( this );
    else
    {
    (Cursor)->SetStandard(  Cursor_Gunsight );
    (this)->PostCursor(  &Block, Cursor );
    }
  OUT(zipview_FullUpdate);
  }

void 
zipview::Update( )
    {
  class zipview *self=this;
  IN(zipview_Update);
  DEBUGxt(Action,Action);
  if ( Pending_hit )		    Pending_Hit( this );
  if ( Pending_redisplay )	    Pending_Redisplay( this );
  if ( Pending_redraw )		    Pending_Redraw( this );
  if ( Pending_pan )		    Pending_Pan( this );
  if ( Pending_zoom )		    Pending_Zoom( this );
  if ( Pending_scale )		    Pending_Scale( this );
  if ( Pending_receiveinputfocus )  Pending_ReceiveInputFocus( this );
  if ( Pending_loseinputfocus )	    Pending_LoseInputFocus( this );
  if ( Pending_edit )		    Pending_Edit( this );
  if ( Pending_browse )		    Pending_Browse( this );
/*  if ( Pending_setprintsize )	    Pending_SetPrintSize( self ); */
  if ( Editing )		    (Editor )->Update( );
  OUT(zipview_Update);
  }

class view *
zipview::Hit( enum view_MouseAction       action, long			       x , long			       y , long			       clicks )
        {
  zip_type_pane	      pane;
  zip_type_figure	      figure;
  class view		     *view = NULL;
  class zipview *self=this;

  IN(zipview_Hit );
  if ( (CurrentPane = pane = (this)->Which_Pane(  x, y ))  &&  !Abeyant )
    {
    if ( action == view_LeftDown  ||  action == view_RightDown )
      (View)->Announce(  " " );
    MouseAction = action;  MouseX = x;  MouseY = y;  MouseClicks = clicks;
    if ( pane->zip_pane_hit_processor )
      { DEBUG(>>> Hit Processor);
      view = (class view *)
	(*pane->zip_pane_hit_processor)( pane->zip_pane_hit_processor_anchor,
					 pane, action, x, y, clicks );
      DEBUG(<<< Hit Processor);
      }
    else
    if ( Editing )
      {
      view = (class view *) this;
      (Editor)->Accept_Hit(  pane, action, x, y, clicks );
      }
    else
    if ( figure = (this)->Within_Which_Figure(  x, y ) )
      {  DEBUG(Within a Figure);
      if ( !InputFocus  ||  action == view_LeftDown )
        view = (Objects(figure->zip_figure_type))->Object_Hit( 
				   figure, action, x, y, clicks );
      }
    if ( view == NULL )
      {
      if ( InputFocus )
	{ DEBUG(Have InputFocus);
	if ( action == view_RightDown  ||  action == view_RightMovement  ||
	     action == view_RightUp )
	  { DEBUG(RightButton);
	  Set_pending_hit;
	  view = (class view *) this;
	  (this )->Update( );
	  }
	}
	else
	{ DEBUG(Not InputFocus);
	if ( action == view_LeftDown ) 
	  { DEBUG(Grab InputFocus);
	  (this)->WantInputFocus(  this );
	  view = (class view *) this;
	  }
	}
      }
    }
    else  /* Abeyant...*/
    {
    if ( Editing  &&  CurrentPane )
      {
      view = (class view *) this;
      (Editor)->Accept_Hit(  pane, action, x, y, clicks );
      }
      else  DEBUG(No Pane ::Hit);
    }
  MouseAction = view_NoMouseEvent;
  OUT(zipview_Hit );
  return  view;
  }

void
zipview::Print( FILE			      *file, const char			      *processor, const char			      *format, boolean		       top_level )
            {
/*  float		      inch_width, inch_height; */
  class zipview *self=this;

  IN(zipview_Print);
  DEBUGst(Processor, processor);
  DEBUGst(Format, format);
  DEBUGdt(Level, top_level);
  (this)->Set_Print_Processor(  processor );
  (this)->Set_Print_Language(  format );
  (this)->Set_Print_Level(  top_level );
  (this)->Set_Print_File(  file );
/*===*/
/*
#define pixels_per_inch	80.0
  if ( !(inch_width = this->data_object->desired_view_width) )
    inch_width = BlockWidth;
  if ( !(inch_height = this->data_object_desired_view_height) )
    inch_height = BlockHeight;
*/

/*
  inch_width = Data->def_inch_width;
  inch_height = Data->def_inch_height;
*/
  
  if ( top_level ) {
      if ( Printing->zip_printing_orientation == zip_landscape )
	  (this)->Set_Print_Dimensions(  10.5, 7.5 );
  }  else  
      (this)->Set_Print_Dimensions(  Data->def_inch_width, Data->def_inch_height );


  if ( this->pane_anchor == NULL )
    Prepare_Default_Pane( this );
  (this)->Print_Pane(  Pane );
/*===*/
  OUT(zipview_Print);
  }

long
zipview::Set_Print_Language( const char				  *language )
      {
  long				  status;
  class zipview *self=this;

  IN(zipview_Set_Print_Language);
  Initialize_Printing( this );
  status = (Printer)->Set_Print_Language(  language );
  OUT(zipview_Set_Print_Language);
  return  status;
  }

long
zipview::Set_Print_Processor( const char				  *processor )
      {
  long				  status;
  class zipview *self=this;

  IN(zipview_Set_Print_Processor);
  Initialize_Printing( this );
  status = (Printer)->Set_Print_Processor(  processor );
  OUT(zipview_Set_Print_Processor);
  return  status;
  }

long
zipview::Set_Print_Level( long				   level )
      {
  long				  status;
  class zipview *self=this;

  IN(zipview_Set_Print_Level);
  Initialize_Printing( this );
  status = (Printer)->Set_Print_Level(  level );
  OUT(zipview_Set_Print_Level);
  return  status;
  }

long
zipview::Set_Print_File( FILE				  *file )
      {
  long				  status;
  class zipview *self=this;

  IN(zipview_Set_Print_File);
  Initialize_Printing( this );
  status = (Printer)->Set_Print_File(  file );
  OUT(zipview_Set_Print_File);
  return  status;
  }

long
zipview::Set_Print_Resolution( long				   resolution )
      {
  long				  status;
  class zipview *self=this;

  IN(zipview_Set_Print_Resolution);
  Initialize_Printing( this );
  status = (Printer)->Set_Print_Resolution(  resolution );
  OUT(zipview_Set_Print_Resolution);
  return  status;
  }

long
zipview::Set_Print_Dimensions( float			   inch_width , float			   inch_height )
      {
  long				  status;
  class zipview *self=this;

  IN(zipview_Set_Print_Dimensions);
  Initialize_Printing( this );
  status = (Printer)->Set_Print_Dimensions(  inch_width, inch_height );
  OUT(zipview_Set_Print_Dimensions);  
  return  status;
  }

long
zipview::Set_Print_Coordinates( zip_type_percent		   x_origin , zip_type_percent		   y_origin , zip_type_percent		   width , zip_type_percent		   height )
      {
  int					  status = zip_success;
  class zipview *self=this;

  IN(zipview_Set_Print_Coordinates);
  Initialize_Printing( this );
  status = (Printer)->Set_Print_Coordinates(  x_origin, y_origin, width, height );
  OUT(zipview_Set_Print_Coordinates);
  return status;
  }

long
zipview::Set_Print_Orientation( long				   orientation )
      {
  int					  status = zip_success;
  class zipview *self=this;

  IN(zipview_Set_Print_Orientation);
  Initialize_Printing( this );
  status = (Printer)->Set_Print_Orientation(  orientation );
  OUT(zipview_Set_Print_Orientation);
  return status;
  }

long
zipview::Print_Figure( zip_type_figure		 figure, zip_type_pane		 pane )
        {
  long			        status = zip_ok;
  class zipview *self=this;

  IN(zipview_Print_Figure);
  Initialize_Printing( this );
  status = (Printer)->Print_Figure(  figure, pane );
  OUT(zipview_Print_Figure);
  return  status;
  }

long
zipview::Print_Image( zip_type_image		 image, zip_type_pane		 pane )
        {
  long			        status;
  class zipview *self=this;

  IN(zipview_Print_Image);
  Initialize_Printing( this );
  status = (Printer)->Print_Image(  image, pane );
  OUT(zipview_Print_Image);
  return  status;
  }

long
zipview::Print_Stream( zip_type_stream		 stream, zip_type_pane		 pane )
        {
  long			        status = zip_ok;
  class zipview *self=this;

  IN(zipview_Print_Stream);
  Initialize_Printing( this );
  status = (Printer)->Print_Stream(  stream, pane );
  OUT(zipview_Print_Stream);
  return  status;
  }

long
zipview::Print_Pane( zip_type_pane		 pane )
      {
  long			        status;
  class zipview *self=this;

  IN(zipview_Print_Pane);
  Initialize_Printing( this );
  status = (Printer)->Print_Pane(  pane );
  OUT(zipview_Print_Pane);
  return  status;
  }

static
void Initialize_Printing( class zipview	      *self )
    {
  int			      i;

  IN(Initialize_Printing);
  if ( Printer == NULL )
    {
    Printer = new zipprint;
    (Printer)->Set_Data_Object(  Data );
    (Printer)->Set_View_Object(  self );
    (Printer)->Set_Default_Print_Size( Data );		    /* Get Default Print Size from ZIP data object */
    (Printer)->Set_Debug(  debug );
    for ( i = 1; Objects(i); i++ )/*===*/
      (Objects(i))->Set_Print_Object(  Printer );
    }
  OUT(Initialize_Printing);
  }

void
zipview::Set_Debug( boolean  state )
      {
  IN(zipview_Set_Debug);
  debug = state;
  OUT(zipview_Set_Debug);
  }

static void
DEBUG_Command( class zipview	       *self )
    {
  IN(DEBUG_Command);
  debug = !debug;
  (Data)->Set_Debug(  debug );
  if ( Printer )
    (Printer)->Set_Debug(  debug );
  if ( Editor )
    (Editor)->Set_Debug(  debug );
  OUT(DEBUG_Command);
  }

static void
Edit_Command( class zipview	       *self )
    {
  IN(Edit_Command);
  Set_pending_edit;
  (self)->WantUpdate(  self );
  OUT(Edit_Command);
  }

static void
Browse_Command( class zipview      *self )
    {
  IN(Browse_Command);
  Set_pending_browse;
  (self)->WantUpdate(  self );
  OUT(Browse_Command);
  }

static void
Portrait_Command( class zipview      *self )
    {
  IN(Portrait_Command);
  (Menu)->SetMask(  ((Menu )->GetMask( ) & ~menu_landscape) | menu_portrait );
  (self)->PostMenus(  Menu );
  (self)->Set_Print_Orientation(  zip_portrait );
  OUT(Portrait_Command);
  }

static void
Landscape_Command( class zipview        *self )
    {
  IN(Landscape_Command);
  (Menu)->SetMask(  ((Menu )->GetMask( ) & ~menu_portrait) | menu_landscape );
  (self)->PostMenus(  Menu );
  (self)->Set_Print_Orientation(  zip_landscape );
  Printer->states.user_def_size = 0;
  OUT(Landscape_Command);
  }

static void
Print_To_File_Command( class zipview        *self )
    {
  char			        *reply;
  FILE			*file;
  char				 msg[512];

  IN(Print_To_File_Command);
  /* Would like to prompt with source stream-name -- currently infeasible */
  if ( (self)->Query(  "Enter Print-File Name: ", NULL, &reply ) == zip_ok  &&
       reply  &&  *reply )
    {
    if ( file = fopen( reply, "w" ) )
      {
      sprintf( msg, "Generating PostScript Print-File '%s'", reply );
      (self)->Announce(  msg );
      (self )->Use_Working_Pane_Cursors( );
      (self)->Print(  file, "PostScript", "PostScript", true );
      fclose( file );
      sprintf( msg, "Wrote PostScript Print-File '%s'", reply );
      (self)->Announce(  msg );
      (self )->Use_Normal_Pane_Cursors( );
      }
      else
      {
      sprintf( msg, "Unable to Open '%s'", reply );
      (self)->Announce(  msg );
      }
    }
  OUT(Print_To_File_Command);
  }

/*===
static void
Page_First_Command( self )
  struct zipview     *self;
  {
  IN(Page_First_Command);
  if ( CurrentPage > 1 )
    {
    CurrentPage = 1;
    zipview_Display_Image( self, zip_Image( Data, "ZIP_PAGE_IMAGE_001" ), Pane );
    zipview_Announce( View, "Page 1" );
    }
    else    zipview_Announce( View, "At First Page." );
  OUT(Page_First_Command);
  }

static void
Page_Last_Command( self )
  struct zipview     *self;
  {
  char			      *page_image_name = "ZIP_PAGE_IMAGE_nnn";

  IN(Page_Last_Command);
  if ( CurrentPage < Data->page_count )
    {
    CurrentPage = Data->page_count;
    sprintf( page_image_name, "ZIP_PAGE_IMAGE_%03d", CurrentPage );
    DEBUGst(Page-name,page_image_name);
    zipview_Display_Image( self, zip_Image( Data, page_image_name ), Pane );
    sprintf( page_image_name, "Page %d", CurrentPage );
    zipview_Announce( View, page_image_name );
    }
    else    zipview_Announce( View, "At Last Page." );
  OUT(Page_Last_Command);
  }

static void
Page_Next_Command( self )
  struct zipview     *self;
  {
  char			      *page_image_name = "ZIP_PAGE_IMAGE_nnn";
  zip_type_image      image;

  IN(Page_Next_Command);
  sprintf( page_image_name, "ZIP_PAGE_IMAGE_%03d", ++CurrentPage );
  DEBUGst(Page-name,page_image_name);
  if ( image = zip_Image( Data, page_image_name ) )
    {
    zipview_Display_Image( self, image, Pane );
    sprintf( page_image_name, "Page %d", CurrentPage );
    zipview_Announce( View, page_image_name );
    }
    else
    {
    CurrentPage--;
    zipview_Announce( View, "At Last Page." );
    }
  OUT(Page_Next_Command);
  }

static void
Page_Prior_Command( self )
  struct zipview     *self;
  {
  char			      *page_image_name = "ZIP_PAGE_IMAGE_nnn";

  IN(Page_Prior_Command);
  if ( CurrentPage > 1 )
    {
    sprintf( page_image_name, "ZIP_PAGE_IMAGE_%03d", --CurrentPage );
    DEBUGst(Page-name,page_image_name);
    zipview_Display_Image( self, zip_Image( Data, page_image_name ), Pane );
    sprintf( page_image_name, "Page %d", CurrentPage );
    zipview_Announce( View, page_image_name );
    }
    else    zipview_Announce( View, "At First Page." );
  OUT(Page_Prior_Command);
  }
===*/

static void
Set_Print_Size_Command(class zipview      *self)
{
    char request[150], *answer = NULL;
    char w[5], h[5];
    float width, height, aspect;
    long status;

    IN(SetPrintSize_Command);

    Initialize_Printing(self);

    /* Data->def_inch_{width,height} has the defaults for 
	setting the print size.  This method will modify
	them accordingly so that the next time the defaults will
	be the user specified sizes.
	*/


    if (Printing->zip_printing_orientation == zip_landscape ) {
	width = 10.5;
	height = 7.5;
    }
    else {

	width = Data->def_inch_width;
	height = Data->def_inch_height;
    }	  


    /* calculate aspect ratio */
    aspect = width / height;

    /*foobar*/

    sprintf(request, "Width now %3.2f in., Enter new Width[constrain to height]  ", width);
    status = (View)->AskOrCancel(request, NULL, &answer);
    if ((status == zip_ok) && answer) {
	if ((sscanf(answer, "%f", &width) != 1) || (width <= 0.0)) {

	    (View)->Announce( "Invalid width entered, keeping old width and height");
	    return;
	}

	/* do new height */
	answer = NULL;
	/*calculate new default height to keep aspect ratio */
	height = width / aspect;

	sprintf(request, "Enter new Height [%3.2f in.]  ", height);
	status = (View)->AskOrCancel(request, NULL, &answer);
	if ((status == zip_ok) && answer) {
	    if ((sscanf(answer, "%f", &height) != 1) || (height <= 0.0)) {

		(View)->Announce( "Invalid height entered, keeping old width and height");
		return;
	    }
	}
	else
	{

	    /* Check to see if canceled from getting new height */
	    if (status == zip_failure) return;
	}
    }
    else
    {
	/* Check to see if canceled from getting new width */
	if (status == zip_failure) return;

	/* status == zip_ok && answer == NULL */
	/* constrain to height */
	answer = NULL;
	sprintf(request, "Width constrained to height, Enter new Height [%3.2f in.]  ", height);
	status = (View)->AskOrCancel(request, NULL, &answer);
	if (answer && (status == zip_ok)) {
	    if ((sscanf(answer, "%f", &height) != 1) || (height <= 0.0)) {


		(View)->Announce( "Invalid height entered, keeping old width and height");
		return;
	    }
	}
	else
	{
	    /* Check to see if canceled from getting new constraining height */
	    if (status == zip_failure) return;
	}
	/* calculate new width keeping aspect ratio */
	width = height * aspect;
    }



    if ((status = (Printer)->Set_Print_Size(width, height)) == zip_ok) {

	sprintf(request, "Print size is %3.2f in. x %3.2f in. %s", width, height,
		((abs((width/height)-aspect) > .1) ? " -- Note: aspect ratio has changed" : ""));
	(View)->Announce( request);
	UserWidth = width;
	UserHeight = height;
	Portrait_Command(self);
    }
    else
    {
	(View)->Announce( "Sorry - Cannot change print size now");
    }

    OUT(SetPrintSize_Command);  
    return;
}



/*===*/static long ZOOM_LEVEL;
static void
Zoom_Pane( class zipview	       *self, int			        factor )
      {
  IN(::Zoom_Pane);
  Set_pending_zoom;
  if ( factor != 0 )
    ZOOM_LEVEL = Pane->zip_pane_zoom_level + factor;
    else
    ZOOM_LEVEL = 0;
  (self)->WantUpdate(  self );
  OUT(::Zoom_Pane);
  }

static void
Zoom_Normal_Command( class zipview   *self )    {  ::Zoom_Pane( self, 0 );  }

static void
Zoom_In_Command( class zipview      *self )    {  ::Zoom_Pane( self, 1 );  }

static void
Zoom_In5_Command( class zipview      *self )    {  ::Zoom_Pane( self, 5 );  }

static void
Zoom_Out5_Command( class zipview      *self )    {  ::Zoom_Pane( self, -5 );  }

static void
Zoom_Out_Command( class zipview      *self )    {  ::Zoom_Pane( self, -1 );  }


static void
Normalize_Pane_Command( class zipview	       *self )
    {
  IN(Normalize_Pane_Command);
  (self)->Normalize_Pane(  Pane );
  Set_pending_redisplay;
  (self)->WantUpdate(  self );
  OUT(Normalize_Pane_Command);
  }

static void
Refresh_Pane_Command( class zipview	       *self )
    {
  IN(Refresh_Pane_Command);
  Set_pending_redisplay;
  (self)->WantUpdate(  self );
  OUT(Refresh_Pane_Command);
  }

static void
Pan_Pane( class zipview	       *self, int			        edge )
      {
  Edge = edge;
  Set_pending_pan;
  (self)->WantUpdate(  self );
  }

static void
Center_Pane_Command( class zipview	       *self )
    {
  IN(Center_Pane_Command);
  Pane->zip_pane_x_offset = 0;
  Pane->zip_pane_y_offset = 0;
  Pane->zip_pane_x_origin_offset = Pane->zip_pane_x_origin;
  Pane->zip_pane_y_origin_offset = Pane->zip_pane_y_origin;
  Set_pending_redisplay;
  (self)->WantUpdate(  self );
  OUT(Center_Pane_Command);
  }

static void
Top_Pane_Command( class zipview      *self )    {  ::Pan_Pane( self, zipview_pane_top_edge );  }

static void
Bottom_Pane_Command( class zipview      *self )    {  ::Pan_Pane( self, zipview_pane_bottom_edge );  }

static void
Left_Pane_Command( class zipview      *self )    {  ::Pan_Pane( self, zipview_pane_left_edge );  }

static void
Right_Pane_Command( class zipview      *self )    {  ::Pan_Pane( self, zipview_pane_right_edge );  }

/*===*/static float SCALE;
static void
Scale_Pane( class zipview	       *self, float		        scale )
      {
  IN(::Scale_Pane);
  DEBUGgt( Scale, scale);
  Set_pending_scale;
  SCALE = (self)->Pane_Scale(  Pane ) + scale;
  (self)->WantUpdate(  self );
  OUT(::Scale_Pane);
  }

static void
Scale_Normal_Command( class zipview	       *self )
    {
  Set_pending_scale;
  SCALE = 1.0;
  (self)->WantUpdate(  self );
  }

static void
Scale_Smaller_Command( class zipview      *self )    {  ::Scale_Pane( self, -0.01 );  }

static void
Scale_Smaller_10_Command( class zipview      *self )    {  ::Scale_Pane( self, -0.1 );  }

static void
Scale_Larger_Command( class zipview      *self )    {  ::Scale_Pane( self, 0.01 );  }

static void
Scale_Larger_10_Command( class zipview      *self )    {  ::Scale_Pane( self, 0.1 );  }

static void
Scale_Half_Command( class zipview	       *self )
    {
  IN(Scale_Half_Command);
  Set_pending_scale;
  SCALE = (self)->Pane_Scale(  Pane ) * 0.5;
  (self)->WantUpdate(  self );
  OUT(Scale_Half_Command);
  }

static void
Scale_Double_Command( class zipview	       *self )
    {
  IN(Scale_Double_Command);
  Set_pending_scale;
  SCALE = (self)->Pane_Scale(  Pane ) * 2.0;
  (self)->WantUpdate(  self );
  OUT(Scale_Double_Command);
  }

static struct bind_Description 	      bound_menu[] =
{
{ "zipview-compose",	    "\033e",    0,	"Compose",		0,  menu_browse,
	(proctable_fptr) Edit_Command,		"Switch to Compose-mode",   NULL },
{ "zipview-browse",	    "\033b",    0,	"Browse",		0,  menu_edit,
	(proctable_fptr) Browse_Command,		"Switch to Browse-mode",   NULL },
{ "zipview-normalize",	    "\033n",    0,	"Pane~20,Normalize~10",	0,  menu_default,
	(proctable_fptr) Normalize_Pane_Command, "...",   NULL },
{ "zipview-refesh",	    "\033r",    0,	"Pane~20,Refresh~11",	0,  menu_default,
	(proctable_fptr) Refresh_Pane_Command,   "...",   NULL },
{ "zipview-center",	    "\033PC",    0,	"Pane~20,Center~20",	0,  menu_default,
	(proctable_fptr) Center_Pane_Command,	"...",   NULL },
{ "zipview-pane-top",	    "\033PT",    0,	"Pane~20,Top~30",	0,  menu_default,
	(proctable_fptr) Top_Pane_Command,   	"...",   NULL },
{ "zipview-pane-bottom",    "\033PB",	0,      "Pane~20,Bottom~31",	0,  menu_default,
	(proctable_fptr) Bottom_Pane_Command,	"...",   NULL },
{ "zipview-pane-left",	    "\033PL",	0,	"Pane~20,Left~32",	0,  menu_default,
	(proctable_fptr) Left_Pane_Command,	"...",   NULL },
{ "zipview-pane-right",	    "\033PR",	0,      "Pane~20,Right~33",	0,  menu_default,
	(proctable_fptr) Right_Pane_Command,	"...",   NULL },
{ "zipview-zoom-zero",	    "\033z0",	0,      "Zoom~30,Normal~10",	0,  menu_default,
	(proctable_fptr) Zoom_Normal_Command,	"...",   NULL },
{ "zipview-zoom-in",	    "\033zi",	0,      "Zoom~30,In~20",	0,  menu_default,
	(proctable_fptr) Zoom_In_Command,	"...",   NULL },
{ "zipview-zoom-in5",	    "\033zi5",	0,	"Zoom~30,In +5~21",	0,  menu_default,
	(proctable_fptr) Zoom_In5_Command,	"...",   NULL },    
{ "zipview-zoom-out",	    "\033zo",	0,      "Zoom~30,Out~30",	0,  menu_default,
	(proctable_fptr) Zoom_Out_Command,	"...",   NULL },
{ "zipview-zoom-out5",	    "\033zo5",	0,	"Zoom~30,Out -5~31",	0,  menu_default,
	(proctable_fptr) Zoom_Out5_Command,	"...",   NULL },
{ "zipview-scale-normal",   "\033s0",	0,      "Scale~40,Normal~10",	0,  menu_default,
	(proctable_fptr) Scale_Normal_Command,	"...",   NULL },
{ "zipview-scale-100s",	    "\033ssh",0,      "Scale~40,100th Smaller~20",0,	menu_default,
	(proctable_fptr) Scale_Smaller_Command,	"...",   NULL },
{ "zipview-scale-10s",	    "\033sst",	0,	"Scale~40,10th Smaller~21", 0,	menu_default,
	(proctable_fptr) Scale_Smaller_10_Command,"...",   NULL },
{ "zipview-scale-half",	    "\033sh",	0,	"Scale~40,Half Size~22",    0,	menu_default,
	(proctable_fptr) Scale_Half_Command,	"...",   NULL },
{ "zipview-scale-100l",	    "\033slh",0,	"Scale~40,100th Larger~30", 0,	menu_default,
	(proctable_fptr) Scale_Larger_Command,	"...",   NULL },
{ "zipview-scale-10l",	    "\033slt",	0,	"Scale~40,10th Larger~31",  0,	menu_default,
	(proctable_fptr) Scale_Larger_10_Command,"...",   NULL },
{ "zipview-scale-double",   "\033sd",	0,	"Scale~40,Double Size~32",  0,	menu_default,
	(proctable_fptr) Scale_Double_Command,	"...",   NULL },
{ "zipview-print_portrait", "\033pp",	0,	"Printing~50,Portrait~10",  0,	menu_app | menu_landscape,
	(proctable_fptr) Portrait_Command,	"...",   NULL },
{ "zipview-print_landscape", "\033pl",	0,	"Printing~50,Landscape~10", 0,	menu_app | menu_portrait,
	(proctable_fptr) Landscape_Command,	"...",   NULL },
{ "zipview-print_to_file",  "\033pf",	0,	"Printing~50,To File ...~20",	0,  menu_app,
	(proctable_fptr) Print_To_File_Command,	"...",   NULL },
{ "zipview-set_print_size",  "...",	0,	"Printing~50,Set Print Size~30",0,  menu_default,
	(proctable_fptr) Set_Print_Size_Command,	"...",   NULL },
/*===
{ "zipview-next-page",	    " ",	0,	"Page~50,Next~10",	0,  menu_default,
	(proctable_fptr) Page_Next_Command,	"...",   NULL },
{ "zipview-next-page",	    "n",	0,	"Page~50,Next~10",	0,  menu_default,
	(proctable_fptr) Page_Next_Command,	"...",   NULL },
{ "zipview-prior-page",	    "p",	0,	"Page~50,Prior~11",	0,  menu_default,
	(proctable_fptr) Page_Prior_Command,	"...",   NULL },
{ "zipview-first-page",	    "f",	0,	"Page~50,First~20",	0,  menu_default,
	(proctable_fptr) Page_First_Command,	"...",   NULL },
{ "zipview-last-page",	    "l",	0,	"Page~50,Last~21",	0,  menu_default,
	(proctable_fptr) Page_Last_Command,	"...",   NULL },
===*/
{ "zipview-DEBUG",	    "D",	0,	"",			0,  menu_default,
	(proctable_fptr) DEBUG_Command,		"...",   NULL },
NULL
};

static
void Build_Menu()
  {
  IN(Build_Menu);
  bind::BindList( bound_menu, class_keymap, class_menulist, &zipview_ATKregistry_  );
  OUT(Build_Menu);
  }

static
void Pending_Hit( class zipview	    *self )
    {
  IN(Pending_Hit);
  Reset_pending_hit;
  if ( InputFocus )
    switch ( MouseAction )
      {
      case view_RightDown:
	(self)->Initiate_Panning(  Pane, MouseX, MouseY, 0/*===*/ );
	break;
      case view_RightMovement:
	(self)->Continue_Panning(  Pane, MouseX, MouseY );
	break;
      case view_RightUp:
	Normalize_View( self );
	(self)->Terminate_Panning(  Pane, MouseX, MouseY, 0, 0, 1 );
	Highlight_View( self );
	break;
      }
  OUT(Pending_Hit);
  }

static
void Pending_Redisplay( class zipview	    *self )
    {
  IN(Pending_Redisplay);
  DEBUGxt(Action, Action);
  Reset_pending_redisplay;
  if ( (self)->Display_Pane(  Pane ) )
    {DEBUG(ZipView Redisplay ERROR);}
  if ( InputFocus )
    Highlight_View( self );
  OUT(Pending_Redisplay);
  }

static
void Pending_Redraw( class zipview	    *self )
    {
  IN(Pending_Redraw);
  DEBUGxt(Action, Action);
  Reset_pending_redraw;
  if ( (self)->Draw_Pane(  Pane ) )
    {DEBUG(ZipView Redraw ERROR);}
  if ( InputFocus )
    Highlight_View( self );
  OUT(Pending_Redraw);
  }

static
void Pending_Zoom( class zipview	    *self )
    {
  zip_type_point	    x, y;

  IN(Pending_Zoom);
  Reset_pending_zoom;
  x = (self)->X_Pixel_To_Point(  Pane, NULL,
    (self)->Pane_Left(  Pane ) + ((self)->Pane_Width(  Pane )/2) );
  y = (self)->Y_Pixel_To_Point(  Pane, NULL,
    (self)->Pane_Top(  Pane ) + ((self)->Pane_Height(  Pane )/2) );
  (self)->Zoom_Pane_To_Point(  Pane, x, y,
    ZOOM_LEVEL, zip_center|zip_middle );
  OUT(Pending_Zoom);
  }

static
void Pending_Scale( class zipview	    *self )
    {
  zip_type_point	    x, y;

  IN(Pending_Scale);
  Reset_pending_scale;
  x = (self)->X_Pixel_To_Point(  Pane, NULL,
    (self)->Pane_Left(  Pane ) + ((self)->Pane_Width(  Pane )/2) );
  y = (self)->Y_Pixel_To_Point(  Pane, NULL,
    (self)->Pane_Top(  Pane ) + ((self)->Pane_Height(  Pane )/2) );
  (self)->Scale_Pane_To_Point(  Pane, x, y,
    SCALE, zip_center|zip_middle );
  OUT(Pending_Scale);
  }

static
void Pending_Pan( class zipview	    *self )
    {
  IN(Pending_Pan);
  Reset_pending_pan;
  Normalize_View( self );
  (self)->Pan_Pane_To_Edge(  Pane, Edge );
  Highlight_View( self );
  OUT(Pending_Pan);
  }

static
void Pending_ReceiveInputFocus( class zipview	    *self )
    {
  IN(Pending_ReceiveInputFocus);
  Reset_pending_receiveinputfocus;
  InputFocus = true;
  Highlight_View( self );
  OUT(Pending_ReceiveInputFocus);
  }

static
void Pending_LoseInputFocus( class zipview	    *self )
    {
  IN(Pending_LoseInputFocus);
  Reset_pending_loseinputfocus;
  Normalize_View( self );
  OUT(Pending_LoseInputFocus);
  }

static
void Pending_Edit( class zipview	    *self )
    {
  long			    i;

  IN(Pending_Edit);
  Reset_pending_edit;
  (self )->Use_Working_Pane_Cursors( );
  if ( Editor == NULL )
    {
    Editor = new zipedit;
    (Editor)->Set_Debug(  debug );
    (Editor)->Set_Data_Object(  Data );
    (Editor)->Set_View_Object(  self );
    for ( i = 1; Objects(i); i ++ )
      (Objects(i))->Set_Edit_Object(  Editor );
    }
  if ( (Editor)->Set_Palettes(  Pane,
	zip_name_palette | zip_shade_palette | zip_attribute_palette |
	zip_figure_palette | zip_font_palette ) )
    {DEBUG(ZipView ERROR: Set_Palettes);}
  Editing = true;
  (Menu)->SetMask(  ((Menu )->GetMask( ) & ~menu_browse) | menu_edit );
  (self)->PostMenus(  Menu );
  if ( (Editor)->Initialize_Editing(  Pane ) )
    {DEBUG(ZipView ERROR: Initialize_Editing);
    Editing = false;
    (Menu)->SetMask(  ((Menu )->GetMask( ) & ~menu_edit) | menu_browse );
    (self)->PostMenus(  Menu );
    }
  Highlight_View( self );
  (self )->Use_Normal_Pane_Cursors( );
  OUT(Pending_Edit);
  }

static
void Pending_Browse( class zipview	    *self )
    {
  IN(Pending_Browse);
Set_pending_browse;
  Reset_pending_browse;
  Editing = false;
  (Editor)->Terminate_Editing(  Pane );
  (Menu)->SetMask(  ((Menu )->GetMask( ) & ~menu_edit) | menu_browse );
  (self)->PostMenus(  Menu );
  Highlight_View( self );
  OUT(Pending_Browse);
  }

static
void Highlight_View( class zipview	      *self )
    {
  IN(Highlight_View);
  (self)->SetTransferMode(  graphic_BLACK );
  (self )->Normalize_Line_Attributes( );
  (self)->DrawRectSize(  0, 0, (self )->GetLogicalWidth(  ) - 1,
				    (self )->GetLogicalHeight( ) - 1);
  OUT(Highlight_View);
  }

static
void Normalize_View( class zipview	      *self )
    {
  IN(Normalize_View);
  (self)->SetTransferMode(  graphic_WHITE );
  (self )->Normalize_Line_Attributes( );
  (self)->DrawRectSize(  0, 0, (self )->GetLogicalWidth(  ) - 1,
				    (self )->GetLogicalHeight( ) - 1);
  OUT(Normalize_View);
  }

static
void Prepare_Default_Pane( class zipview	       *self )
    {
  long			      status = zip_ok;
  char				      pane_name[257];
  static int			      pane_number = 1;

  IN(Prepare_Default_Pane);
  sprintf( pane_name, "ZipView-Pane-%02d", pane_number++ );
  (self)->Create_Pane(  &Pane, pane_name, &Block, zip_opaque );
/*
  if ( ObjectWidth )	zipview_Set_Pane_Object_Width(  self, Pane, ObjectWidth );
  if ( ObjectHeight )	zipview_Set_Pane_Object_Height( self, Pane, ObjectHeight );
*/

  /*
    Since we are starting in absolute mode, set the object dimensions
  to that of the pane.
  */

  (self)->Set_Pane_Object_Width(   Pane, self->data_object->object_width = (self)->Pane_Width( Pane) );
  (self)->Set_Pane_Object_Height(  Pane, self->data_object->object_height = (self)->Pane_Height( Pane) );

  if ( Data->page_count )
    status = (self)->Set_Pane_Image(  Pane, (self->data_object)->Image(  "ZIP_PAGE_IMAGE_001" ) );
    else
    status = (self)->Set_Pane_Stream(  Pane, self->data_object->stream );
  OUT(Prepare_Default_Pane);
  }

static int
Prepare_Default_Stream( class zipview	       *self )
    {
  long			      status = zip_ok;
  char				      stream_name[257];
  static int			      stream_number = 1;
  const char			      source[] =
	"#Default Stream Header\n";

  IN(Prepare_Default_Stream);
  sprintf( stream_name, "ZipView-Created-Stream-%02d", stream_number++ );
  if ( status = (Data)->Create_Stream(  &self->data_object->stream, stream_name, zip_default ) )
    {DEBUGdt( ZipView ERROR: Create-Stream Status, status);}

  if ( status = (Data)->Set_Stream_Source(  self->data_object->stream, source ) )
  {DEBUGdt( ZipView ERROR: Set-Stream-Source Status, status);}

  /* Set the stream extrema based on the print size since anything outside
     will not appear on the physical page anyhow
  */

     self->data_object->stream->zip_stream_least_x = (zip_type_point) (-Data->def_inch_width * 
       72.0 / 2.0);
     DEBUGlt(Extrema Least X, STREAM->zip_stream_least_x);

     self->data_object->stream->zip_stream_greatest_x = (zip_type_point) (Data->def_inch_width *
       72.0 / 2.0);
     DEBUGlt(Extrema Greatest X, STREAM->zip_stream_greatest_x);

     self->data_object->stream->zip_stream_least_y = (zip_type_point) (-Data->def_inch_height *
       72.0 / 2.0);
     DEBUGlt(Extrema Least Y, self->data_object->stream->zip_stream_least_y);

     self->data_object->stream->zip_stream_greatest_y = (zip_type_point) (Data->def_inch_height *
       72.0 / 2.0);
     DEBUGlt(Extrema Greatest Y, self->data_object->stream->zip_stream_greatest_y);

  Set_pending_edit;
  OUT(Prepare_Default_Stream);
  return status;
  }

long
zipview::Query( const char			       *query , const char			       *default_response , char			       **response )
      {
  long			      status = zip_ok;
  static char			      buffer[512];
  class zipview *self=this;

  IN(zipview_Query);
  Abeyant = true;
  *buffer = 0;
  *response = buffer;
  if ( (message::AskForString( this, 0, query, default_response,
	 buffer, sizeof buffer ) == -1)  ||  *buffer == 0 )
    {
    (this)->Announce(  "Cancelled." );
    status = zip_failure;
    }
  DEBUGst(Buffer,buffer);
  Abeyant = false;
  OUT(zipview_Query);
  return  status;
  }

long
zipview::Query_File_Name( const char			      *query, char			     **response )
        {
  enum message_CompletionCode  result;
  static char			      path[257];
  static char			      buffer[513];
  long			      status = zip_ok;
  class zipview *self=this;

  IN(zipview_Query_File_Name);
  Abeyant = true;
  *buffer = 0;
  *response = buffer;
  result = (enum message_CompletionCode) completion::GetFilename( this, 
			  query,			/* prompt */
			  path,				/* initial string */
			  buffer,			/* working area */
			  sizeof buffer - 1,		/* size of working area */
			  0,				/* want file, not directory */
			  1 );				/* must be existing file */
  DEBUGdt(Result,result);
  DEBUGst(File-name,buffer);
  if ( (result != message_Complete  &&  result != message_CompleteValid)  ||
       *buffer == 0 ||
	buffer[strlen(buffer)-1] == '/' )
    {
    status = zip_failure;
    (this)->Announce(  "Cancelled." );
    }
    else /* Preserve path for next time around */
    {
    strncpy( path, buffer, sizeof(path) - 1 );
    path[sizeof(path) - 1] = 0;
    }
  Abeyant = false;
  OUT(zipview_Query_File_Name);
  return  status;
  }

long
zipview::Announce( const char			       *message )
      {
  long			      status = zip_ok;

  IN(zipview_Announce);
  if ( this->imPtr )
    {
    if ( message::DisplayString( this, 0, message ) == -1 )
      status = zip_failure;
    im::ForceUpdate();
    }
    else  fprintf( stderr, "Zip: %s\n", message );
  OUT(zipview_Announce);
  return  status;
  }


long
zipview::AskOrCancel( const char  *request , const char  *def , char  **answer)
{
    long		      status = zip_ok;
    static char			      buffer[512];
    class zipview *self=this;

    Abeyant = true;
    *buffer = 0;
    if (message::AskForString( self, 0, request, def, buffer, sizeof buffer ) == -1)
    {
	status = zip_failure;
	(self)->Announce( "Canceled.");
    }
    else
    {
	if (*buffer != 0) *answer = buffer;
	else *answer = NULL;
	status = zip_ok;
    }
    Abeyant = false;
    return status;
}




/* The Scroll-Bar facilities */

#include "scroll.H"

static const struct scrollfns vertical_scroll_interface = {
    (scroll_getinfofptr) zipview_y_getinfo,
    (scroll_setframefptr) zipview_y_setframe,
    (scroll_endzonefptr) zipview_y_endzone,
    (scroll_whatfptr) zipview_y_whatisat
};

static const struct scrollfns horizontal_scroll_interface = {
    (scroll_getinfofptr) zipview_x_getinfo,
    (scroll_setframefptr) zipview_x_setframe,
    (scroll_endzonefptr) zipview_x_endzone,
    (scroll_whatfptr) zipview_x_whatisat
};



class view *
zipview::GetApplicationLayer( )
    {
  class scroll	     *view;
  class zipview *self=this;

  IN(zipview_GetApplicationLayer);
  Application = true;

  /* Change zip default printing size */
  if (!Data->size_from_file) {
      Data->def_inch_width = 7.5;
      Data->def_inch_height = 10.5;
      DEBUGgt(Application Inch Width, Data->def_inch_width);
      DEBUGgt(Application Inch Height, Data->def_inch_height);
  }

  view = scroll::Create( this, scroll_LEFT | scroll_BOTTOM );
  (view)->SetView(  this );
  OUT(zipview_GetApplicationLayer);
  return  (class view *) view;
  }


const void *
zipview::GetInterface( const char		      *interface_name )
      {
  const struct scrollfns	     *interface = NULL;

  IN(zipview_GetInterface);
  if ( strcmp( interface_name, "scroll,vertical" ) == 0 )
    interface = &vertical_scroll_interface;
    else
    if ( strcmp( interface_name, "scroll,horizontal" ) == 0 )
      interface = &horizontal_scroll_interface;
  OUT(zipview_GetInterface);
  return interface;
  }

static void
zipview_y_getinfo( class zipview	      *self, struct range		      *total , struct range		      *seen , struct range		      *dot )
      {
  long			      height, top, bottom, x;

  IN(zipview_y_getinfo);
  total->beg = total->end = dot->beg = dot->end = seen->beg = seen->end = 0;
  if ( Pane )
    {
  top = (self)->Y_Point_To_Pixel(  Pane, NULL /*===*/,
	  (self->data_object)->Image_Greatest_Y(  (self->data_object)->Image_Root(  self->data_object->stream ) ) );
  bottom = (self)->Y_Point_To_Pixel(  Pane, NULL /*===*/,
	     (self->data_object)->Image_Least_Y(  (self->data_object)->Image_Root(  self->data_object->stream ) ) );
  if ( top > bottom )
    {
    x = top;
    top = bottom;
    bottom = x;
    }
  height = bottom - top;
  total->end = height;
  seen->beg = (top >= (self)->Pane_Top(  Pane )) ? 0 : -top;
  seen->end = (bottom <= (self)->Pane_Bottom(  Pane )) ?
		height : height - (bottom - (self)->Pane_Bottom(  Pane ));
  DEBUGdt( Pane Top, (self)->Pane_Top(  Pane ) );
  DEBUGdt( Pane Bottom, (self)->Pane_Bottom(  Pane ) );
  DEBUGdt( Imagery Top, top);
  DEBUGdt( Imagery Bottom, bottom);
  DEBUGdt( Imagery Height, height);
  DEBUGdt( Total-beg, total->beg);
  DEBUGdt( Total-end, total->end);
  DEBUGdt( Seen-beg, seen->beg);
  DEBUGdt( Seen-end, seen->end);
    }
  OUT(zipview_y_getinfo);
  }


static long
zipview_y_whatisat( class zipview	      *self, long			       coordinate , long			       outof )
      {
  long			      value;

  IN(zipview_y_whatisat);
  DEBUGlt( Coordinate,coordinate);
  DEBUGlt( Outof, outof);
  DEBUGdt( y-offset,Pane->zip_pane_y_offset);
  value = coordinate + Pane->zip_pane_y_offset;
  DEBUGlt( Value,value);
  OUT(zipview_y_whatisat);
  return value;
  }

static void 
zipview_y_setframe( class zipview	      *self, int			       position, long			       coordinate , long			       outof )
        {
  IN(zipview_y_setframe);
  DEBUGdt( Position, position );
  DEBUGlt( Coordinate, coordinate );
  DEBUGlt( Outof, outof );
  DEBUGdt( y-offset(1),Pane->zip_pane_y_offset);
  Pane->zip_pane_y_offset = position - coordinate;
  DEBUGdt( y-offset(2),Pane->zip_pane_y_offset);
  Set_pending_redisplay;
  (self)->WantUpdate( self);
  OUT(zipview_y_setframe);
  }


static void
zipview_y_endzone( class zipview	      *self, int			       zone , int			       action )
      {
  IN(zipview_y_endzone);
  DEBUGdt( Zone,zone);
  DEBUGdt( Action,action);
  if ( zone == scroll_TOPENDZONE )
    {
    DEBUG( Top);
    ::Pan_Pane( self, zipview_pane_top_edge );
    }
    else
    {
    DEBUG( Bottom);
    ::Pan_Pane( self, zipview_pane_bottom_edge );
    }
  OUT(zipview_y_endzone);
  }

static void
zipview_x_getinfo( class zipview	      *self, struct range		      *total , struct range		      *seen , struct range		      *dot )
      {
  long			      width, left, right, x;

  IN(zipview_x_getinfo);
  total->beg = total->end = dot->beg = dot->end = seen->beg = seen->end = 0;
  if ( Pane )
    {
  left = (self)->X_Point_To_Pixel(  Pane, NULL /*===*/,
	  (self->data_object)->Image_Least_X(  (self->data_object)->Image_Root(  self->data_object->stream ) ) );
  right = (self)->X_Point_To_Pixel(  Pane, NULL /*===*/,
	     (self->data_object)->Image_Greatest_X(  (self->data_object)->Image_Root(  self->data_object->stream ) ) );
  if ( left > right )
    {
    x = left;
    left = right;
    right = x;
    }
  width = right - left;
  total->end = width;
  seen->beg = (left >= (self)->Pane_Left(  Pane )) ? 0 : -left;
  seen->end = (right <= (self)->Pane_Right(  Pane )) ?
		width : width - (right - (self)->Pane_Right(  Pane ));
  DEBUGdt( Pane Left, (self)->Pane_Left(  Pane ) );
  DEBUGdt( Pane Right, (self)->Pane_Right(  Pane ) );
  DEBUGdt( Imagery Left, left);
  DEBUGdt( Imagery Right, right);
  DEBUGdt( Imagery Width, width);
  DEBUGdt( Total-beg, total->beg);
  DEBUGdt( Total-end, total->end);
  DEBUGdt( Seen-beg, seen->beg);
  DEBUGdt( Seen-end, seen->end);
    }
  OUT(zipview_x_getinfo);
  }


static long
zipview_x_whatisat( class zipview	      *self, long			       coordinate , long			       outof )
      {
  long			      value;

  IN(zipview_x_whatisat);
  DEBUGlt( Coordinate,coordinate);
  DEBUGlt( Outof, outof);
  DEBUGdt( x-offset,Pane->zip_pane_x_offset);
  value = coordinate + Pane->zip_pane_x_offset;
  DEBUGlt( Value,value);
  OUT(zipview_x_whatisat);
  return value;
  }

static void 
zipview_x_setframe( class zipview	      *self, int			       position, long			       coordinate , long			       outof )
        {
  IN(zipview_x_setframe);
  DEBUGdt( Position, position );
  DEBUGlt( Coordinate, coordinate );
  DEBUGlt( Outof, outof );
  DEBUGdt( x-offset(1),Pane->zip_pane_x_offset);
  Pane->zip_pane_x_offset = coordinate - position;
  DEBUGdt( x-offset(2),Pane->zip_pane_x_offset);
  Set_pending_redisplay;
  (self)->WantUpdate( self);
  OUT(zipview_x_setframe);
  }


static void
zipview_x_endzone( class zipview	      *self, int			       zone , int			       action )
      {
  IN(zipview_x_endzone);
  DEBUGdt( Zone,zone);
  DEBUGdt( Action,action);
  if ( zone == scroll_TOPENDZONE )
    {
    DEBUG( Left);
    ::Pan_Pane( self, zipview_pane_left_edge );
    }
    else
    {
    DEBUG( Right);
    ::Pan_Pane( self, zipview_pane_right_edge );
    }
  OUT(zipview_x_endzone);
  }
/*=== === ===*/
