/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

/**  SPECIFICATION -- External Facility Suite  *********************************

TITLE	The Sched View-object Program

MODULE	schedv.c

VERSION	0.0

DESCRIPTION
	This is the suite of Methods that support the Sched View-object.

HISTORY
  01/20/89	Created (TCP)
  08/24/89	Upgrade to Suite V1.0 interface (TCP)

END-SPECIFICATION  ************************************************************/

#include <andrewos.h>
#include <environ.H>
#include <graphic.H>
#include <fontdesc.H>
#include <observable.H>
#include <bind.H>
#include <menulist.H>
#include <keymap.H>
#include <keystate.H>
#include <view.H>
#include <message.H>
#include <rect.h>
#include <sched.H>
#include <schedv.H>
#include <suite.H>
#include <zip.H>
#include <zipview.H>

static boolean debug=FALSE;
static class menulist		     *class_menulist;
static class keymap		     *class_keymap;

struct NO_DLL_EXPORT schedv_private {
   static void Extend_Button( class schedv  *self, class suite  *suite, struct suite_item  *item, long type, enum view::MouseAction       action, long x, long y, long clicks );
   static void Split_Button( class schedv *self, class suite *suite, struct suite_item *item, long type, enum view::MouseAction       action, long x, long y, long clicks );
   static void Clear_Button( class schedv  *self, class suite  *suite, struct suite_item *item, long type, enum view::MouseAction       action, long x, long y, long clicks );
   static void Save_Button( class schedv *self, class suite *suite, struct suite_item *item, long type, enum view::MouseAction       action, long x, long y, long clicks );
   static void Print_Button( class schedv *self, class suite  *suite, struct suite_item	 *item, long type, enum view::MouseAction       action, long x, long y, long clicks );
   static void Quit_Button( class schedv *self, class suite *suite, struct suite_item *item, long type, enum view::MouseAction       action, long x, long y, long clicks );
   static void Debug_Command( class schedv  *self );
   static void Quit_Command( class schedv *self );

   static void Initialize( class schedv  *self );
   static void Handle_Slot_Hit( class schedv *self, zip_type_figure   slot_figure );
   static void Remember_Slot_Hit( class schedv  *self, zip_type_figure slot_figure );
   static void Move_Slot( class schedv *self, zip_type_figure slot_figure );
   static void Normalize_Previous_Slot_Figure( class schedv *self );
   static void Normalize_Current_Slot_Figure( class schedv  *self );
   static long Exceptions( class schedv *self, char *facility, long  status  );
};

static /*===*/class schedv *SELF;

#define  InputFocus		    (self->input_focus)
#define  ScheduleStream		    (self->stream)
#define  Zip			    (self->data->zipp)
#define  ZipView		    (self->zipviewp)
#define  ChartPane		    (self->chart_pane)
#define  Modified		    (self->modified)
#define  Tracking		    (self->tracking)
#define  PendingQuestion	    (self->pending_question)
#define  PendingDuplicate	    (self->pending_duplicate)
#define	 Block			    (&self->block)
#define  Left			    (self->block.left)
#define  Top			    (self->block.top)
#define  Width			    (self->block.width)
#define  Height			    (self->block.height)
#define  Bottom			    (Top + Height)
#define	 ChartBlock		    (&self->chart_block)
#define  ChartLeft		    (self->chart_block.left)
#define  ChartTop		    (self->chart_block.top)
#define  ChartWidth		    (self->chart_block.width)
#define  ChartHeight		    (self->chart_block.height)

#define  CurrentSlotFigure	    (self->current_slot_figure)
#define  PreviousSlotFigure	    (self->previous_slot_figure)
#define  CurrentTextFigure	    (self->current_text_figure)
#define  PreviousTextFigure	    (self->previous_text_figure)
#define  ControlButtons		    (self->control_buttons)
#define  ControlButtonHeight	    (40)
#define  ButtonHeight		    (ControlButtonHeight)
#define  ButtonTop		    (Bottom - ControlButtonHeight)
#define  ControlButtonTop	    (ButtonTop)
#define  Menu			    (self->menu)
#define  Keystate		    (self->keystate)


ATKdefineRegistry(schedv, view, schedv::InitializeClass);

void
schedv::Set_Debug( boolean mode )
{
    debug = mode;
}

boolean
schedv::InitializeClass( )
{
static const struct bind_Description	 menu[] = {
{   "schedv-quit",		"\033q",    0,	    "Quit~99",		0,  0,
    (proctable_fptr) schedv_private::Quit_Command,		"Quit",		    NULL},
{   "schedv-debug",		"\033z",    0,	    "DEBUG~88",		0,  0,
    (proctable_fptr) schedv_private::Debug_Command,		"Debug",	    NULL},
NULL
};


    IN(schedv_InitializeClass);
    class_menulist = new menulist;
    class_keymap = new keymap;
    bind::BindList( menu, class_keymap, class_menulist, &schedv_ATKregistry_  );
    OUT(schedv_InitializeClass);
    return TRUE;
}

schedv::schedv( )
{
    ATKinit;
    class schedv *self=this;
    SELF=this;
    IN(schedv_InitializeObject);
    ChartPane = NULL;
    this->data = NULL;
    ZipView = new zipview;
    Menu = (class_menulist)->DuplicateML(  this );
    Keystate = keystate::Create(this, class_keymap);
    InputFocus = Modified = Tracking = PendingQuestion = PendingDuplicate = false;
    ScheduleStream = NULL;
    CurrentSlotFigure = CurrentTextFigure =
      PreviousSlotFigure = PreviousTextFigure = NULL;
    OUT(schedv_InitializeObject);
    THROWONFAILURE(TRUE);
}

schedv::~schedv( )
{
    ATKinit;
    class schedv *self=this;

    if(Keystate) delete Keystate;
    if(Menu) delete Menu;
    if(ZipView) {
	(ZipView)->UnlinkTree();
	(ZipView)->Destroy();
	ZipView = NULL;
    }
}

void
schedv::SetDataObject( class dataobject *data )
{
    class schedv *self=this;
    this->data = (class sched *) data;
    (ZipView)->SetDataObject( Zip );
    (Zip)->Set_general_Exception_Handler(  (zip_generalexceptfptr) schedv_private::Exceptions );
}

void
schedv::ReceiveInputFocus( )
{
    class schedv *self=this;
    IN(schedv_ReceiveInputFocus);
    InputFocus = true;
    Keystate->next = NULL;
    (this)->PostKeyState( Keystate );
    (this)->PostMenus(  Menu );
    OUT(schedv_ReceiveInputFocus);
}

void
schedv::LoseInputFocus( )
{
    class schedv *self=this;
    IN(schedv_LoseInputFocus);
    InputFocus = false;
    OUT(schedv_LoseInputFocus);
}

void
schedv::FullUpdate( enum view::UpdateType type, long left , long  top , long  width , long   height )
{
    class schedv *self=this;
    IN(schedv_FullUpdate);
    if ( type == view::FullRedraw || type == view::LastPartialRedraw )
    {
	(this)->GetVisualBounds(  Block );
	ChartLeft = Left;   ChartTop = Top;
	ChartWidth = Width; ChartHeight = Height - ButtonHeight;
	if ( ChartPane == NULL )
	    schedv_private::Initialize( this );
	(ZipView)->InsertViewSize(  this, ChartLeft, ChartTop, ChartWidth, ChartHeight );
	(ZipView)->FullUpdate(  type, 0, 0, ChartWidth, ChartHeight );
	(ControlButtons)->InsertViewSize(  this, Left, ControlButtonTop,
					 Width, ControlButtonHeight );
	(ControlButtons)->FullUpdate(  type, 0, 0, Width, ControlButtonHeight );
    }
    OUT(schedv_FullUpdate);
}

void schedv_private::Initialize( class schedv *self )
{
#if 0
static const suite_Specification  extend_button[] =  {
  suite_ItemCaption( "Extend" ),
  suite_ItemHitHandler( schedv::Extend_Button ),
  0
};

static const suite_Specification  split_button[] = {
  suite_ItemCaption( "Split" ),
  suite_ItemHitHandler( schedv::Split_Button ),
  0
};
#endif

static const suite_Specification  clear_button[] = {
  suite_ItemCaption( "Clear" ),
  suite_ItemHitHandler( schedv_private::Clear_Button ),
  0
};

static const suite_Specification  save_button[] = {
  suite_ItemCaption( "Save" ),
  suite_ItemHitHandler( schedv_private::Save_Button ),
  0
};

static const suite_Specification print_button[] = {
  suite_ItemCaption( "Print" ),
  suite_ItemHitHandler( schedv_private::Print_Button ),
  0
};

static const suite_Specification quit_button[] =
  {
  suite_ItemCaption( "Quit" ),
  suite_ItemHitHandler( schedv_private::Quit_Button ),
  0
};

static const suite_Specification control_buttons[] = {
  suite_ItemCaptionFontName( "andysans10b" ),
/*===
  suite_Item( extend_button ),
  suite_Item( split_button ),
===*/
  suite_Item( clear_button ),
  suite_Item( save_button ),
  suite_Item( print_button ),
  suite_Item( quit_button ),
  suite_Arrangement( suite_Row ),
  0
};

    const UNUSED char *reply; // used with DB=1

    IN(Initialize);
    (ZipView)->LinkTree(  self );
    reply = (ZipView )->GetWindowManagerType( );
    DEBUGst(Window Manager,reply);
    (ZipView )->Use_Working_Pane_Cursors( );
    (ZipView)->Create_Pane(  &ChartPane, "Chart-Pane", ChartBlock, zip_opaque );
    (ZipView)->Set_Pane_Cursor(  ChartPane, 'A', "aptcsr20" );
    (ZipView)->Set_Pane_Stream(  ChartPane, ScheduleStream = self->data->stream );
    ControlButtons = suite::Create( control_buttons, (long) self );
    (ControlButtons)->LinkTree(  self );
    (ZipView )->Use_Normal_Pane_Cursors( );
    OUT(Initialize);
}

class view *
schedv::Hit( enum view::MouseAction action, long  x , long y , long clicks )
{
    class schedv *self=this;
    class view		     *hit = (class view *) this;
    zip_type_figure	      fig;

    IN(schedv_Hit);
    DEBUGdt(Action,action);
    if ( PendingQuestion  &&  (action == view::LeftDown  ||  action == view::RightDown) )
    {
	PendingQuestion = false;
	message::CancelQuestion( this );
	(ZipView)->Announce(  " " );
	schedv_private::Normalize_Current_Slot_Figure( this );
    }
    if ( PendingDuplicate &&  action == view::LeftDown )
	schedv_private::Normalize_Previous_Slot_Figure( this );
    if ( y > ButtonTop )
    { DEBUG(Buttons ::Hit);
    if ( action == view::RightUp && PreviousTextFigure != NULL )
	schedv_private::Normalize_Previous_Slot_Figure( this );
    hit = (ControlButtons)->Hit(  action,
				(ControlButtons)->EnclosedXToLocalX(  x ),
				(ControlButtons)->EnclosedYToLocalY(  y ), clicks );
    }
    else
    { DEBUG(Chart ::Hit);
    if ( !InputFocus  &&  (action == view::LeftDown ||
			   action == view::RightDown ||
			   action == view::RightUp) )
	(this)->WantInputFocus(  this );
    if ( InputFocus )
    { DEBUG(Have InputFocus);
    if ( (fig = (ZipView)->Within_Which_Figure(  x, y ))  &&
	(Zip)->Figure_Name(  fig ) )
	switch ( action )
	{
	    case view::LeftDown:
		if ( fig )
		    schedv_private::Handle_Slot_Hit( this, fig );
		break;
	    case view::RightDown:
		if ( fig )
		    schedv_private::Remember_Slot_Hit( this, fig );
		else
		    if ( PendingDuplicate )
			schedv_private::Normalize_Previous_Slot_Figure( this ); 
		break;
	    case view::RightUp:
		if ( fig )
		    schedv_private::Move_Slot( this, fig );
		else
		    if ( PreviousSlotFigure )
			schedv_private::Normalize_Previous_Slot_Figure( this );
		break;
	    default:
	        break;
	}
    }
    }
    OUT(schedv_Hit);
    return  hit;
}

void schedv_private::Handle_Slot_Hit( class schedv *self, zip_type_figure slot_figure )
{
    char				      reply[512], string[512];
    long			      shade;
    zip_type_figure	      text_figure;

    IN(Handle_Slot_Hit);
    CurrentSlotFigure = slot_figure;
    DEBUGst(Slot Figure Name,(Zip)->Figure_Name(slot_figure));
    sprintf( string, "%sText", (Zip)->Figure_Name(  slot_figure) );
    if ( ( text_figure = (Zip)->Figure(  string ) ) )
    {  DEBUGst(Text Figure Name,(Zip)->Figure_Name(text_figure));
    CurrentTextFigure = text_figure;
    shade = (Zip)->Figure_Shade(  slot_figure );
    (Zip)->Set_Figure_Shade(  slot_figure, 100 );
    (ZipView)->Draw_Figure(  slot_figure, ChartPane );
    sprintf( string, "Enter  %s:  ", (Zip)->Figure_Name(  slot_figure ) );
    PendingQuestion = true;
    if ( message::AskForString( self, 0, string,
			       (Zip)->Figure_Text(  text_figure ), reply, sizeof reply ) )
    { DEBUG(YANK-AWAY);message::CancelQuestion( self );}
    else
    { DEBUGst(Reply,reply);
    PendingQuestion = false;
    const char *fig_text = (Zip)->Figure_Text(text_figure);
    if(!fig_text) fig_text = "";
    if ( strcmp( reply, fig_text ) )
    {
	Modified = true;
	(ZipView)->Clear_Figure(  text_figure, ChartPane );
	(Zip)->Set_Figure_Text(  text_figure, reply );
    }
    if ( *reply )
	shade = 10;
    else
	shade = 0;
    (ZipView)->Clear_Figure(  slot_figure, ChartPane );
    (Zip)->Set_Figure_Shade(  slot_figure, shade );
    (ZipView)->Draw_Figure(  slot_figure, ChartPane );
    (ZipView)->Draw_Figure(  text_figure, ChartPane );
    (ZipView)->Announce(  " " );
    }
    }
    OUT(Handle_Slot_Hit);
}

void schedv_private::Remember_Slot_Hit( class schedv  *self, zip_type_figure slot_figure )
{
    char				      string[512];
    zip_type_figure	      text_figure;

    IN(Remember_Slot_Hit);

    DEBUGst(Slot Figure Name,(Zip)->Figure_Name(slot_figure));
    sprintf( string, "%sText", (Zip)->Figure_Name(  slot_figure) );
    if ( !PendingDuplicate && (text_figure = (Zip)->Figure(  string )) )
	if ( (Zip)->Figure_Text(  text_figure) ) 
	{  DEBUGst(Text Figure Name,(Zip)->Figure_Name(text_figure));
	PreviousSlotFigure = slot_figure;
	PreviousTextFigure = text_figure;
	(Zip)->Set_Figure_Shade(  slot_figure, 50 );
	(ZipView)->Draw_Figure(  slot_figure, ChartPane );
	(ZipView)->Draw_Figure(  text_figure, ChartPane );
	}
    OUT(Remember_Slot_Hit);
}

void schedv_private::Move_Slot( class schedv *self, zip_type_figure slot_figure )
{
    char				      string[512];
    zip_type_figure	      text_figure;

    IN(Move_Slot);
    if ( PreviousSlotFigure != NULL )
    {
	if (PreviousSlotFigure == slot_figure )
	    PendingDuplicate = true;
	else 
	{
	    DEBUGst(Slot Figure Name,(Zip)->Figure_Name(slot_figure));
	    sprintf( string, "%sText", (Zip)->Figure_Name(  slot_figure) );
	    if ( ( text_figure = (Zip)->Figure(  string ) ) )
	    {  DEBUGst(Text Figure Name,(Zip)->Figure_Name(text_figure));
	    Modified = true;
	/* Copy into Target from Previous  */  
	    if ( (Zip)->Figure_Text(  text_figure ) )
		(ZipView)->Clear_Figure(  text_figure, ChartPane );
	    (Zip)->Set_Figure_Text(  text_figure, (Zip)->Figure_Text(  PreviousTextFigure )  );
	    (ZipView)->Clear_Figure(  slot_figure, ChartPane );
	    (Zip)->Set_Figure_Shade(  slot_figure, 10 );
	    (ZipView)->Draw_Figure(  slot_figure, ChartPane );
	    (ZipView)->Draw_Figure(  text_figure, ChartPane );
	    if ( !PendingDuplicate )
	    {   
	  /* Clear out Previous figure  */ 
		(Zip)->Set_Figure_Text(  PreviousTextFigure, NULL  );
		(ZipView)->Clear_Figure(  PreviousSlotFigure, ChartPane );
		(Zip)->Set_Figure_Shade(  PreviousSlotFigure, 0 );
		(ZipView)->Draw_Figure(  PreviousSlotFigure, ChartPane );
		PreviousSlotFigure = NULL;
		PreviousTextFigure = NULL;
	    }
	    }
	    else
		Normalize_Previous_Slot_Figure( self );
	}
    }  /* first if */
    OUT(Move_Slot);
}

void schedv_private::Normalize_Previous_Slot_Figure( class schedv *self )
{
    IN(Normalize_Previous_Slot_Figure);
    (ZipView)->Clear_Figure(  PreviousSlotFigure, ChartPane );
    (Zip)->Set_Figure_Shade(  PreviousSlotFigure, 10 );
    (ZipView)->Draw_Figure(  PreviousSlotFigure, ChartPane );
    (ZipView)->Draw_Figure(  PreviousTextFigure, ChartPane );
    PreviousSlotFigure = PreviousTextFigure = NULL;
    PendingDuplicate = false;
    OUT(Normalize_Previous_Slot_Figure);
}

void schedv_private::Normalize_Current_Slot_Figure( class schedv  *self )
{
    long			      shade = 0;

    IN(Normalize_Current_Slot_Figure);
    if ( CurrentSlotFigure  &&  (Zip)->Figure_Shade(  CurrentSlotFigure ) == 100 )
    {
	if ( (Zip)->Figure_Text(  CurrentTextFigure ) )
	    shade = 10;
	(ZipView)->Clear_Figure(  CurrentSlotFigure, ChartPane );
	(Zip)->Set_Figure_Shade(  CurrentSlotFigure, shade );
	(ZipView)->Draw_Figure(  CurrentSlotFigure, ChartPane );
	(ZipView)->Draw_Figure(  CurrentTextFigure, ChartPane );
	PreviousSlotFigure = PreviousTextFigure = NULL;
	PendingDuplicate = false;
    }
    OUT(Normalize_Current_Slot_Figure);
}

void schedv_private::Extend_Button( class schedv *self, class suite *suite, struct suite_item  *item, long type, enum view::MouseAction       action, long x, long y, long clicks )
{
    IN(Extend_Button);
    // unimplemented?
    (self)->WantInputFocus(  self );
    OUT(Extend_Button);
}

void schedv_private::Split_Button( class schedv *self, class suite  *suite, struct suite_item *item, long type, enum view::MouseAction       action, long x, long y, long clicks )
{
    IN(Split_Button);
    // unimplemented?
    (self)->WantInputFocus(  self );
    OUT(Split_Button);
}

void schedv_private::Clear_Button( class schedv *self, class suite  *suite, struct suite_item  *item, long type, enum view::MouseAction       action, long x, long y, long clicks )
{
    zip_type_image	      img = (Zip)->Image_Root(  ScheduleStream );
    zip_type_figure	      fig,
    slot_figure;
    char				      name[512];

    IN(Clear_Button);
    if ( type == suite_ItemObject  &&  action == view::LeftUp )
    {
	fig = (Zip)->Figure_Root(  img );
	while ( fig )
	{
	    if ( (Zip)->Figure_Name(  fig )  &&  (Zip)->Figure_Text(  fig ) )
	    { DEBUGst(Text,(Zip)->Figure_Text(  fig ));
	    Modified = true;
	    (Zip)->Set_Figure_Text(  fig, NULL );
	    strcpy( name, (Zip)->Figure_Name(  fig ) );
	    name[strlen( name ) - 4] = 0;
	    DEBUGst(Name,name);
	    slot_figure = (Zip)->Figure(  name );
	    (ZipView)->Clear_Figure(  slot_figure, ChartPane );
	    (Zip)->Set_Figure_Shade(  slot_figure, 0 );
	    (ZipView)->Draw_Figure(  slot_figure, ChartPane );
	    }
	    if ( (fig = (Zip)->Next_Figure(  fig )) == NULL )
	    {
		img = (Zip)->Next_Image(  img );
		fig = (Zip)->Figure_Root(  img );
	    }
	}
    }
    (self)->WantInputFocus(  self );
    OUT(Clear_Button);
}

void schedv_private::Save_Button( class schedv	  *self, class suite  *suite, struct suite_item  *item, long type, enum view::MouseAction action, long x, long y, long clicks )
{
    char				      msg[512];
    long			      status;

    IN(Save_Button);
    if ( type == suite_ItemObject  &&  action == view::LeftUp )
    {
	(ZipView )->Use_Working_Pane_Cursors( );
	if ( (status = (Zip)->Write_Stream(  ScheduleStream )) == zip_ok )
	    sprintf( msg, "Wrote File '%s'", ScheduleStream->zip_stream_name );
	else
	    sprintf( msg, "Error Writing File '%s'  (%ld)", ScheduleStream->zip_stream_name, status );
	Modified = false;
	(ZipView)->Announce(  msg );
	(ZipView )->Use_Normal_Pane_Cursors( );
    }
    (self)->WantInputFocus(  self );
    OUT(Save_Button);
}

void schedv_private::Print_Button( class schedv  *self, class suite  *suite, struct suite_item *item, long type, enum view::MouseAction       action, long x, long y, long clicks )
{
    char				      msg[512];
    long			      status;
    FILE			     *file;

    IN(Print_Button);
    if ( type == suite_ItemObject  &&  action == view::LeftUp )
    {
	(ZipView )->Use_Working_Pane_Cursors( );
	file = fopen( "tmp_print", "w" );
	(ZipView)->Set_Print_Language(  "PostScript" );
	(ZipView)->Set_Print_Processor(  "PostScript" );
	(ZipView)->Set_Print_Level(  1 );
	(ZipView)->Set_Print_File(  file );
	(ZipView)->Set_Print_Dimensions(  8.5, 11.0 );
	if ( (status = (ZipView)->Print_Stream(  ScheduleStream, ChartPane )) == zip_ok )
	{
	    fclose( file );
	    system( "print -Tnative tmp_print" );
	    sprintf( msg, "Printed File '%s'", ScheduleStream->zip_stream_name );
	}
	else
	    sprintf( msg, "Error Printing File '%s'  (%ld)", ScheduleStream->zip_stream_name, status );
	(ZipView)->Announce(  msg );
	(ZipView )->Use_Normal_Pane_Cursors( );
    }
    (self)->WantInputFocus(  self );
    OUT(Print_Button);
}

void schedv_private::Quit_Button( class schedv *self, class suite  *suite, struct suite_item	 *item, long type, enum view::MouseAction       action, long x, long y, long clicks )
{
    static const char		     * const choices[] =
    {"Cancel", "Save", "Save & Quit", "Quit Anyway", 0};
    long				       response = 0;
    UNUSED long  result; /* only used with debug */

    IN(Quit_Button);
    if ( type == suite_ItemObject  &&  action == view::LeftUp )
    {
	if ( Modified )
	{
	    result =  message::MultipleChoiceQuestion(
						      self, 0, "Outstanding Modifications:", 0, &response, choices, NULL );
	    DEBUGdt(Result,result);
	    DEBUGdt(Response,response);
	    switch ( response )
	    {
		case 0:	    break;
		case 1:	    Save_Button( self, suite, item, type, action, x, y, clicks ); break;
		case 2:	    Save_Button( self, suite, item, type, action, x, y, clicks );
		case 3:	    exit(0);			break;
		default:    break;
	    }
	}
	else  exit(0);
    }
    (self)->WantInputFocus(  self );
    OUT(Quit_Button);
}

void
schedv_private::Debug_Command( class schedv *self )
{
    IN(Debug_Command);
    debug = !debug;
    OUT(Debug_Command);
}

void
schedv_private::Quit_Command( class schedv *self )
{
    IN(Quit_Command);
    Quit_Button( self, NULL, NULL, suite_ItemObject, view::LeftUp, 0, 0, 0 );
    OUT(Quit_Command);
}

long
schedv_private::Exceptions( class schedv *self, char *facility, long   status  )
{
    char				      msg[512];

    IN(Exceptions);
/*===*/
    self = SELF;
    sprintf( msg, "Exception  Status = %ld  Facility = %s", status, facility );
    (ZipView)->Announce(  msg );
    OUT(Exceptions);
    return  0;
}
