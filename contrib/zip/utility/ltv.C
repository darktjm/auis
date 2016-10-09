/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

/**  SPECIFICATION -- External Facility Suite  *********************************

TITLE	The LightTable View-object Program

MODULE	ltv.c

NOTICE	IBM Internal Use Only

DESCRIPTION
	This is the suite of Methods that support the LightTable View-object.

HISTORY
  10/10/88	Created (TCP)
  08/02/89	Accomodate Suite interface upgrade -- suite_ChangeItemCaption (TCP)
  08/25/89	More Accomodations (TCP)
  08/29/89	More Accomodations (TCP)
  08/31/89	Change Data to Datum (TCP)

END-SPECIFICATION  ************************************************************/

#include <andrewos.h>
#include <im.H>
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
#include <lt.H>
#include <ltv.H>
#include <suite.H>
#include <zip.H>
#include <zipview.H>
#include <zipobject.H>
#include <rasterview.H>

static boolean debug=FALSE;
static class menulist		     *class_menulist;
static class keymap		     *class_keymap;



static /*===*/class ltv *SELF;

#define  tolerance		    5

#define  enclosure_cursor	    'P'
#define  normal_cursor		    'A'
#define  invisible_cursor	    '@'
#define  build_cursor		    '/'

#define  hide_background	    (1<<0)
#define  expose_background	    (1<<1)
#define  pan_foreground		    (1<<2)
#define  pan_together		    (1<<3)

#define  InputFocus		    (self->input_focus)
#define  StreamLocal		    (self->stream)
#define  IMAGE			    (self->image)
#define  Figure			    (self->figure)
#define  Point			    (self->point)
#define  Zip			    (self->data->zipp)
#define  ZipView		    (self->zipviewp)
#define  ForegroundPane		    (self->foreground_pane)
#define  Build			    (self->build)
#define  Building		    (self->building)
#define  Modifying		    (self->modifying)
#define  Modified		    (self->modified)
#define  Tracking		    (self->tracking)
#define	 Block			    (&self->block)
#define  Left			    (self->block.left)
#define  Top			    (self->block.top)
#define  Width			    (self->block.width)
#define  Height			    (self->block.height)
#define  Bottom			    (Top + self->block.height)
#define  ForegroundPanning	    (self->foreground_panning)
#define  BackgroundPane		    (self->background_pane)
#define  BackgroundLight	    (self->background_light)
#define  BackgroundExposed	    (self->background_exposed)
#define  BackgroundXOffset	    (self->background_x_offset)
#define  BackgroundYOffset	    (self->background_y_offset)
#define  BackgroundLeft		    (self->background_left)
#define  BackgroundLeftX	    (self->background_left + self->background_x_offset )
#define  BackgroundTop		    (self->background_top)
#define  BackgroundTopY		    (self->background_top + self->background_y_offset )
#define  BackgroundWidth	    (self->data->background_width)
#define  BackgroundHeight	    (self->data->background_height)
#define  Raster			    (self->data->rasterp)
#define  RasterView		    (self->rasterviewp)
#define  LeftNameItem		    (self->left_name_item)
#define  RightNameItem		    (self->right_name_item)
#define  Buttons		    (self->buttons)
#define  ButtonHeight		    (63)
#define  ButtonTop		    (Top + Height - ButtonHeight)
#define  Menu			    (self->menu)
#define  Keystate		    (self->keystate)
#define  InitialX		    (self->initial_x_pixel)
#define  InitialY		    (self->initial_y_pixel)
#define  WM			    (self->wm_type)
#define  WMWM			    (self->wm_type == 1)
#define  XWM			    (self->wm_type == 2)
#define  TentativeName		    (self->tentative)
#define  EnclosureExposed	    (self->enclosure_exposed)
#define  EnclosureLeft		    (self->enclosure.left)
#define  EnclosureTop		    (self->enclosure.top)
#define  EnclosureWidth		    (self->enclosure.width)
#define  EnclosureHeight	    (self->enclosure.height)

struct NO_DLL_EXPORT ltv_private {
  static void Begin_Chain_Button( class ltv *self, class suite  *suite, struct suite_item     *item, long type, enum view_MouseAction  action, long x, long y, long clicks );
  static void End_Chain_Button( class ltv  *self, class suite  *suite, struct suite_item	      *item, long type, enum view_MouseAction action, long x, long y, long clicks );
  static void End_Chain( class ltv *self );
  static void Delete_Chain_Button( class ltv *self, class suite *suite, struct suite_item	      *item, long type, enum view_MouseAction action, long x, long y, long clicks );
  static void Rename_Chain_Button( class ltv *self, class suite *suite, struct suite_item	      *item, long type, enum view_MouseAction action, long x, long y, long clicks );
  static void Left_Chain_Button( class ltv *self, class suite *suite, struct suite_item	      *item, long type, enum view_MouseAction action, long x, long y, long clicks );
  static void Right_Chain_Button( class ltv *self, class suite *suite, struct suite_item	      *item, long type, enum view_MouseAction action, long x, long y, long clicks );

  static void Detect( class ltv  *self, class suite *suite, struct suite_item *item, long    datum );
  static void Initialize( class ltv  *self );
  static void Build_Chain( class ltv  *self, enum view_MouseAction action, long  x, long	 y, long clicks );
  static int Which_Figure_Point( class ltv *self, zip_type_figure figure, zip_type_pane       pane, zip_type_pixel x , zip_type_pixel y );
  static void Modify_Chain( class ltv *self, enum view_MouseAction action, long x, long	 y, long clicks );
  static void Track_Enclosure( class ltv *self, enum view_MouseAction action, long x, long y , long  clicks );
  static void Cancel_Enclosure( class ltv *self );
  static void Clear_Enclosure( class ltv *self );
  static void Draw_Enclosure( class ltv  *self );
  static void Invert_Enclosure( class ltv *self );
  static long Neighbor( class ltv *self, zip_type_pixel x , zip_type_pixel y, zip_type_figure    *figure, zip_type_point *X , zip_type_point *Y, long  *point );
  static void Normalize_Command( class ltv *self );
  static void Zoom_In_Command( class ltv  *self );
  static void Zoom_Out_Command( class ltv *self );
  static void Scale_Pane( class ltv *self, float  scale );
  static void Scale_Normal_Command( class ltv  *self );
  static void Scale_Smaller_Command( class ltv *self );
  static void Scale_Smaller_10_Command( class ltv *self );
  static void Scale_Larger_Command( class ltv *self );
  static void Scale_Larger_10_Command( class ltv *self );
  static void Scale_Half_Command( class ltv *self );
  static void Scale_Double_Command( class ltv *self );
  static void Pan_Foreground_Command( class ltv	 *self );
  static void Pan_Together_Command( class ltv *self );
  static void Fit_Command( class ltv *self );
  static void Show_Chain_Names( class ltv *self );
  static void Clear_Chain_Names( class ltv    *self );
  static long Rename_Exception( class ltv *self, char *facility, long  status  );
  static void Name_Chain( class ltv *self );
  static void Split_Chain_Name( class ltv *self, zip_type_figure figure, char **right_name, char **left_name  );
  static void Passivate( class ltv *self, long datum );
  static void Activate( class ltv *self, long datum );
  static void Lighten_Background_Command( class ltv *self );
  static void Lighten_Background( class ltv *self );
  static void Darken_Background_Command( class ltv *self );
  static void Hide_Background_Command( class ltv *self );
  static void Expose_Background_Command( class ltv *self );
  static void Show_Background( class ltv  *self );
  static void Save_Command( class ltv *self );
  static void Print_Command( class ltv *self );
  static void Quit_Command( class ltv *self );
  static void Debug_Command( class ltv  *self );
  static void Build_Menu();
  static long Exceptions( class ltv *self, char *facility, long  status  );
};

#define  ChangeItemCaption(o,i,c) \
      (o)->ChangeItemAttribute(  i, suite_ItemCaption(c) )

#define  right_code	    1
#define  left_code	    2
#define  begin_code	    3
#define  end_code	    4
#define  rename_code	    5
#define  delete_code	    6

static suite_Specification		    begin_button[] =
  {
  suite_ItemCaption( "Begin Chain" ),
  suite_ItemDatum( begin_code ),
  suite_ItemHitHandler( ltv_private::Begin_Chain_Button ),
  0
  };

static suite_Specification		    end_button[] =
  {
  suite_ItemCaption( "End Chain" ),
  suite_ItemDatum( end_code ),
  suite_ItemHitHandler( ltv_private::End_Chain_Button ),
  0
  };

static suite_Specification		    rename_button[] =
  {
  suite_ItemCaption( "Rename Chain" ),
  suite_ItemDatum( rename_code ),
  suite_ItemHitHandler( ltv_private::Rename_Chain_Button ),
  0
  };

static suite_Specification		    delete_button[] =
  {
  suite_ItemCaption( "Delete Chain" ),
  suite_ItemDatum( delete_code ),
  suite_ItemHitHandler( ltv_private::Delete_Chain_Button ),
  0
  };

static suite_Specification		    right_name_button[] =
  {
  suite_ItemTitleCaption( "Right:" ),
  suite_ItemDatum( right_code ),
  suite_ItemTitlePlacement( suite_Left ),
  suite_ItemTitleFontName( "andysans10b" ),
  suite_ItemCaptionFontName( "andy10b" ),
/*===  suite_ItemType( suite_ReadWrite ),*/
  suite_ItemHitHandler( ltv_private::Right_Chain_Button ),
  0
  };

static suite_Specification		    left_name_button[] =
  {
  suite_ItemTitleCaption( "Left:" ),
  suite_ItemDatum( left_code ),
  suite_ItemTitlePlacement( suite_Left ),
  suite_ItemTitleFontName( "andysans10b" ),
  suite_ItemCaptionFontName( "andy10b" ),
/*===  suite_ItemType( suite_ReadWrite ),*/
  suite_ItemHitHandler( ltv_private::Left_Chain_Button ),
  0
  };

static suite_Specification		    ltv_buttons[] =
  {
  suite_ItemCaptionFontName( "andysans10b" ),
  suite_Item( begin_button ),
  suite_Item( end_button ),
  suite_Item( rename_button ),
  suite_Item( delete_button ),
  suite_Item( right_name_button ),
  suite_Item( left_name_button ),
/*===  suite_Arrangement( suite_Row ),*/
  suite_Arrangement( suite_Balanced | suite_Matrix ),
  0
  };


ATKdefineRegistry(ltv, view, ltv::InitializeClass);

void
ltv::Set_Debug( boolean mode )
{
    class ltv *self=this;
    debug = mode;
    (ZipView)->Set_Debug(  debug );
}

boolean
ltv::InitializeClass( )
{
    IN(ltv_InitializeClass);
    class_menulist = new menulist;
    class_keymap = new keymap;
    ltv_private::Build_Menu();
    OUT(ltv_InitializeClass);
    return TRUE;
}

ltv::ltv( )
{
    ATKinit;
    class ltv *self=this;

    SELF=this;
    IN(ltv_InitializeObject);
    ForegroundPane = BackgroundPane = NULL;
    this->data = NULL;
    ZipView = new zipview;
    (ZipView)->Set_Debug( debug );
    RasterView = new rasterview;
    Buttons = suite::Create( ltv_buttons, (long) self );
    Menu = (class_menulist)->DuplicateML(  this );
    Keystate = keystate::Create(this, class_keymap);
    InputFocus = BackgroundExposed = true;
    BackgroundLight = 0;
    Building = Build = Modifying = Modified = ForegroundPanning =
      Tracking = EnclosureExposed = false;
    BackgroundXOffset = BackgroundYOffset = 0;
    EnclosureLeft = EnclosureTop = EnclosureWidth = EnclosureHeight = 0;
    LeftNameItem = RightNameItem = NULL;
    StreamLocal = NULL;
    IMAGE = NULL;
    Figure = NULL;
    Point = 0;
    WM = 0;
    OUT(ltv_InitializeObject);
    THROWONFAILURE(  TRUE);
}

ltv::~ltv( )
{
    ATKinit;
    class ltv *self=this;
    if(Keystate) delete Keystate;
    if(Menu) delete Menu;
    if(ZipView) {
	(ZipView)->UnlinkTree();
	(ZipView)->Destroy();
	ZipView = NULL;
    }
    if(Buttons) {
	(Buttons)->UnlinkTree();
	(Buttons)->Destroy();
	Buttons = NULL;
    }
}

void
ltv::SetDataObject( class dataobject *data )
{
    class ltv *self=this;
    this->data = (class lt *) data;
    (ZipView)->SetDataObject(  Zip );
    (this->data)->AddObserver(  this );
    (Zip)->Set_general_Exception_Handler( (zip_generalexceptfptr) ltv_private::Exceptions );
}

void
ltv::ReceiveInputFocus( )
{
    class ltv *self=this;
    IN(ltv_ReceiveInputFocus);
    InputFocus = true;
    (Menu)->SetMask(  hide_background | pan_foreground );
    Keystate->next = NULL;
    (this)->PostKeyState( Keystate );
    (this)->PostMenus(  Menu );
    (this)->WantUpdate( this );
    OUT(ltv_ReceiveInputFocus);
}

void
ltv::LoseInputFocus( )
{
    class ltv *self=this;
    IN(ltv_LoseInputFocus);
    InputFocus = false;
    OUT(ltv_LoseInputFocus);
}

view_DSattributes 
ltv::DesiredSize( long given_width , long  given_height, enum view_DSpass pass, long *desired_width , long *desired_height )
{
    IN(ltv_DesiredSize);
    *desired_width  = 50;
    *desired_height = 100;
    OUT(ltv_DesiredSize);
    return  view_Fixed;
}

void
ltv::FullUpdate( enum view_UpdateType type, long left , long top , long width , long height )
{
    class ltv *self=this;
    IN(ltv_FullUpdate);
    if ( type == view_FullRedraw || type == view_LastPartialRedraw )
    {
	ltv_private::Cancel_Enclosure( this );
	(this)->GetVisualBounds(  Block );
	DEBUGdt(Left,Left); DEBUGdt(Top,Top);
	DEBUGdt(Width,Width); DEBUGdt(Height,Height);
	(ZipView)->InsertViewSize(  this, Left, Top, Width, Height - ButtonHeight );
	(Buttons)->InsertViewSize(  this, Left, Bottom - ButtonHeight, Width, ButtonHeight );
	(Buttons)->FullUpdate(  type, 0, 0, Width, ButtonHeight );
	if ( Figure == NULL )
	{
	    ltv_private::Passivate( this, delete_code );
	    ltv_private::Passivate( this, rename_code );
	    ltv_private::Passivate( this, end_code );
	}
	BackgroundLeft = (Left + Width/2) - BackgroundWidth/2;
	BackgroundTop  = (Top + Height/2) - BackgroundHeight/2;
	ltv_private::Show_Background( this );
	(ZipView )->Use_Normal_Pane_Cursors( );
    }
    OUT(ltv_FullUpdate);
}

void ltv_private::Detect( class ltv *self, class suite *suite, struct suite_item *item, long datum )
{
    IN(Detect);
    switch ( (Buttons/*===*/)->ItemAttribute(  item, suite_itemdatum ) )
    {
	case  left_code:
	    LeftNameItem = item;
	    break;
	case  right_code:
	    RightNameItem = item;
	    break;
    }
    OUT(Detect);
}

void ltv_private::Initialize( class ltv  *self )
{
    long		status = 0;
    zip_type_image     root_image;
    boolean	      satisified = false;
    char	     *reply, stream_name[512], raster_name[512];
    const char       *wm;

    IN(Initialize);
    wm = (ZipView )->GetWindowManagerType( );
    DEBUGst(Window Manager,wm);
    if ( strcmp( "AndrewWM", wm ) == 0 )   WM = 1;  else  WM = 2;
    (ZipView )->Use_Working_Pane_Cursors( );
    (ZipView)->Create_Pane(  &ForegroundPane, "Foreground-Pane", Block, zip_opaque );
    (ZipView)->Set_Pane_Cursor(  ForegroundPane, normal_cursor, "aptcsr20" );
    (ZipView)->Set_Pane_Panning_Precision(  ForegroundPane, 1 );
     if ( self->data->foreground_stream == NULL )
    {
	(ZipView)->Use_Normal_Pane_Cursors( );
	while ( ! satisified )
	{
	    if ( (ZipView)->Query_File_Name(  "Enter Zip file-name: ", &reply ) )
	    {
/*===*/printf("ERROR -- Exiting\n");
/*===*/exit (-1);
	    }
	    else  strcpy( stream_name, reply );
	    if ( (ZipView)->Query_File_Name(  "Enter Raster file-name: ", &reply ) )
	    {
/*===*/printf("ERROR -- Exiting\n");
/*===*/exit (-1);
	    }
	    else  strcpy( raster_name, reply );
	    (ZipView)->Announce(  "Thanks." );
	    if ( (self->data)->Read_Visuals(  stream_name, raster_name ) )
	    {
		(ZipView)->Query(  "Error Reading Files. Continue?: ", "Y", &reply );
		if ( reply  &&  (*reply == 'n' || *reply == 'N' ) )
		    exit (-1);
	    }
	    else satisified = true;
	}
	(ZipView )->Use_Working_Pane_Cursors( );
    }
    (ZipView)->Set_Pane_Stream(  ForegroundPane, StreamLocal = self->data->foreground_stream );
    (ZipView)->Set_Pane_Object_Width(  ForegroundPane, BackgroundWidth );
    (ZipView)->Set_Pane_Object_Height(  ForegroundPane, BackgroundHeight );
    if ( ( IMAGE = (Zip)->Image(  "Chains" ) ) )
    { DEBUG(Chains Image Exists);
    }
    else
    { DEBUG(Chains Image Non-Existant);
    root_image = (Zip)->Image(  "ZIP_ROOT_IMAGE" );
    if ( ( status = (Zip)->Create_Inferior_Image(  &IMAGE, "Chains", StreamLocal, root_image ) ) )
    { DEBUG(ERROR -- Create Chains Image);
/*===*/printf("ERROR -- Failed to create Chains Image (Status %ld)\n",status );
    }
    }
    (RasterView)->SetDataObject( (class dataobject *) Raster );
    (Buttons)->Apply( (suite_applyfptr) Detect, (long) self, 0 );
    (ZipView)->Use_Normal_Pane_Cursors( );
    OUT(Initialize);
}

void
ltv::ObservedChanged( class observable *changed, long  value )
{
    IN(ltv_ObservedChanged);
    (this)->WantUpdate( this );
    OUT(ltv_ObservedChanged);
}

void
ltv::Update( )
{
    class ltv* self=this;
    IN(ltv_Update);
    if ( ForegroundPane == NULL ) {
	ltv_private::Initialize( this );
	ltv_private::Show_Background( this );
    }
    OUT(ltv_Update);
}

class view *
ltv::Hit( enum view_MouseAction action, long  x , long  y , long clicks )
{
    long x_delta, y_delta;
    class view *hit = (class view *) this;
    class ltv *self=this;

    IN(ltv_Hit);
    if ( x < 0 ) x = 0;
    if ( y < 0 ) y = 0;
    if ( x > Left+Width ) x = Left+Width;
    if ( y > Top+Height ) y = Top+Height;
    if ( y < ButtonTop  &&  !InputFocus  &&  action == view_LeftDown )
	(this)->WantInputFocus(  this );
    if ( InputFocus )
    { DEBUG(InputFocus);
    if ( y > ButtonTop )
    { DEBUG(Buttons ::Hit);
    hit = (Buttons)->Hit(  action,
			 (Buttons)->EnclosedXToLocalX(  x ),
			 (Buttons)->EnclosedYToLocalY(  y ), clicks );
    }
    else
    {
	switch ( action )
	{
	    case view_LeftDown:
		ltv_private::Clear_Chain_Names( this );
	    case view_LeftMovement:
	    case view_LeftUp:
		if ( Building )
		{ DEBUG(Building);
		ltv_private::Build_Chain( this, action, x, y, clicks );
		}
		else
		{ DEBUG(NotBuilding);
		ltv_private::Passivate( this, end_code );
		if ( Modifying  ||  (action == view_LeftDown  &&  
				     (Figure = (ZipView)->Which_Pane_Figure(  x, y, ForegroundPane ))) )
		{ DEBUGst(Figure-name, Figure->zip_figure_name);
		ltv_private::Modify_Chain( this, action, x, y, clicks );
		}
		else
		{
		    (ZipView)->Announce(  " " );
		    ltv_private::Track_Enclosure( this, action, x, y, clicks );
		}
		}
		if ( Figure  &&  !Building )
		{
		    ltv_private::Activate( this, delete_code );
		    ltv_private::Activate( this, rename_code );
		}
		else
		{
		    ltv_private::Passivate( this, delete_code );
		    ltv_private::Passivate( this, rename_code );
		}
		break;
	    case view_RightDown:
		ltv_private::Cancel_Enclosure( this );
		(ZipView)->Initiate_Panning(  ForegroundPane, x, y, 0 );
		break;
	    case view_RightMovement:
		(ZipView)->Continue_Panning(  ForegroundPane, x, y );
		break;
	    case view_RightUp:
		(ZipView)->Terminate_Panning(  ForegroundPane, x, y, &x_delta, &y_delta, 0 );
		if ( abs(x_delta)  ||  abs(y_delta) )
		{
		    if ( !ForegroundPanning )
		    {
			BackgroundXOffset += x_delta;
			BackgroundYOffset += y_delta;
		    }
		    else  (ZipView)->Clear_Pane(  ForegroundPane );
		    (ZipView)->Pan_Pane(  ForegroundPane, x_delta, -y_delta );
		    ltv_private::Show_Background( this );
		}
		(ZipView )->Use_Normal_Pane_Cursors( );
		break;
	    default:
		break;
	}
    }
    }
    OUT(ltv_Hit);
    return  hit;
}

void ltv_private::Build_Chain( class ltv *self, enum view_MouseAction action, long	x , long y , long clicks )
{
    long		      X, Y, status = 0;
    long neighbor = 0;
    zip_type_figure	      neighbor_figure;
    zip_type_point	      neighbor_x = 0, neighbor_y = 0;
    long			      neighbor_point;
    char			      msg[512], *reply;

    IN(Build_Chain);
    X = (ZipView)->X_Pixel_To_Point(  ForegroundPane, NULL, x );
    Y = (ZipView)->Y_Pixel_To_Point(  ForegroundPane, NULL, y );
    if ( action != view_LeftMovement )
	neighbor = Neighbor( self, x, y, &neighbor_figure, &neighbor_x, &neighbor_y, &neighbor_point );
    switch ( action )
    {
	case view_LeftDown:
	    DEBUG(LeftDown);
	    (ZipView)->Set_Pane_Cursor(  ForegroundPane, invisible_cursor, "aptcsr20" );
	    if ( Build )
	    { DEBUG(Begin Build);
	    Build = false;
	    InitialX = x;  InitialY = y;
	    Clear_Chain_Names( self );
	    if ( ( status = (Zip)->Create_Figure(  &Figure, NULL, zip_poly_line_figure, IMAGE, NULL ) ) )
	    { DEBUGdt(ERROR,status);
	    Figure = NULL;
	    }
	    else
	    { DEBUG(Success);
	    Point = 2;
	    if ( neighbor )
	    { DEBUG( Snuggle to Neighbor);
	    sprintf( msg, "Beginning At Chain '%s'", (Zip)->Figure_Name(  neighbor_figure ) );
	    (ZipView)->Announce(  msg );
	    X = neighbor_x;
	    Y = neighbor_y;
	    }
	    (Zip)->Set_Figure_Point(  Figure, 1, X, Y );
	    (Zip)->Set_Figure_Point(  Figure, 2, X, Y );
	    }
	    }
	    else
	    { DEBUG(Continue Building);
	    if ( Figure )
		++Point;
	    }
	    break;
	case view_LeftMovement:
	    DEBUG(LeftMovement);
	    break;
	case view_LeftUp:
	    DEBUG(LeftUp);
	    (ZipView)->Set_Pane_Cursor(  ForegroundPane, build_cursor, "icon12" );
	    if ( Figure  &&  neighbor )
	    {
		if ( neighbor_figure == Figure  &&  Point > 3  &&
		    abs(x - InitialX) < tolerance  &&  abs(y - InitialY) < tolerance )
		{ DEBUG(Snuggle to own Origin Point);
		X = Figure->zip_figure_point.zip_point_x;
		Y = Figure->zip_figure_point.zip_point_y;
		while ( (ZipView)->Query(  "At Current Chain Origin.  End Chain?: ", "Y", &reply ) );
		if ( reply  &&  (*reply == 'Y' || *reply == 'y' ) )
		    End_Chain( self );
		else  (ZipView)->Announce(  " ");
		}
		else
		    if ( neighbor_figure != Figure )
		    { DEBUG(Snuggle to Neighbor);
		    X = neighbor_x;
		    Y = neighbor_y;
		    sprintf( msg, "At Chain '%s'.  End Current Chain?: ",
			    (Zip)->Figure_Name(  neighbor_figure ) );
		    while ( (ZipView)->Query(  msg, "Y", &reply ) );
		    if ( reply  &&  (*reply == 'Y' || *reply == 'y' ) )
			End_Chain( self );
		    else  (ZipView)->Announce(  " ");
		    }
	    }
	    break;
	default:
	    break;
    }
    if ( status == 0  &&  Figure )
    {
	(ZipView)->Set_Pane_Painting_Mode(  ForegroundPane, zipview_paint_inverted );
	(ZipView)->Draw_Figure(  Figure, ForegroundPane );
	(Zip)->Set_Figure_Point(  Figure, Point, X, Y );
	(ZipView)->Draw_Figure(  Figure, ForegroundPane );
	(ZipView)->Set_Pane_Painting_Mode(  ForegroundPane, 0 );
    }
    OUT(Build_Chain);
}

 /*=========*/
int
ltv_private::Which_Figure_Point( class ltv	*self, zip_type_figure figure, zip_type_pane  pane, zip_type_pixel x , zip_type_pixel y )
{
    long		      p;
    static class zipobject    *PO;

    IN(Which_Figure_Point);
    if ( PO == NULL )
    {
	PO = (class zipobject *)ATK::NewObject( "zipoplin" );
	(PO)->Set_Data_Object(  Zip );
	(PO)->Set_View_Object(  ZipView );
    }
    p = (PO)->Proximate_Object_Points(  figure, pane, x, y );
    DEBUGdt(Point,p);
    OUT(Which_Figure_Point);
    return  p;
}


void ltv_private::Modify_Chain( class ltv *self, enum view_MouseAction action, long	 x , long y , long clicks )
{
    long		      X, Y, status = 0;
    static long		      down_x, down_y, down_X, down_Y, moved;

    IN(Modify_Chain);
    X = (ZipView)->X_Pixel_To_Point(  ForegroundPane, NULL, x );
    Y = (ZipView)->Y_Pixel_To_Point(  ForegroundPane, NULL, y );
    switch ( action )
    {
	case view_LeftDown:
	    DEBUG(LeftDown);
	    Cancel_Enclosure( self );
	    moved = false;
	    down_x = x;  down_y = y;
	    (ZipView)->Set_Pane_Cursor(  ForegroundPane, invisible_cursor, "aptcsr20" );
	    (ZipView)->Announce(  " " );
	    if (( Point = /*===zipv==*/Which_Figure_Point( self, Figure, ForegroundPane, x, y ) ))
	    {
		Modifying = Modified = true;
		Show_Chain_Names( self );
		if ( Point == 1 )
		{
		    down_X = Figure->zip_figure_point.zip_point_x;
		    down_Y = Figure->zip_figure_point.zip_point_y;
		}
		else
		{
		    down_X = Figure->zip_figure_points->zip_points[Point-2].zip_point_x;
		    down_Y = Figure->zip_figure_points->zip_points[Point-2].zip_point_y;
		}
	    }
	    break;
	case view_LeftMovement:
	    DEBUG(LeftMovement);
	    if ( abs(x - down_x) > tolerance  ||  abs(y - down_y) > tolerance )
		moved = true;
	    break;
	case view_LeftUp:
	    DEBUG(LeftUp);
	    (ZipView)->Set_Pane_Cursor(  ForegroundPane, normal_cursor, "aptcsr20" );
	    Modifying = false;
	    if ( abs(x - down_x) < tolerance  &&  abs(y - down_y) < tolerance )
	    {X = down_X;  Y = down_Y;}
	    break;
	default:
	    break;
    }
    if ( status == 0  &&  Figure  &&  Point )
    {
	(ZipView)->Set_Pane_Painting_Mode(  ForegroundPane, zipview_paint_inverted );
	(ZipView)->Draw_Figure(  Figure, ForegroundPane );
	if ( moved )
	    (Zip)->Set_Figure_Point(  Figure, Point, X, Y );
	(ZipView)->Draw_Figure(  Figure, ForegroundPane );
	(ZipView)->Set_Pane_Painting_Mode(  ForegroundPane, 0 );
    }
    OUT(Modify_Chain);
}

void ltv_private::Track_Enclosure( class ltv *self, enum view_MouseAction action, long x , long		       y , long clicks )
{
    IN(Track_Enclosure);
    Clear_Enclosure( self );
    switch ( action )
    {
	case view_LeftDown:
	    Tracking = true;
	    EnclosureExposed = false;
	    (ZipView)->Set_Pane_Cursor(  ForegroundPane, enclosure_cursor, "aptcsr20" );
	    EnclosureLeft = x;
	    EnclosureTop = y;
	    EnclosureWidth = EnclosureHeight = 0;
	    break;
	case view_LeftMovement:
	    if ( Tracking  &&
		(abs(x - EnclosureLeft) > tolerance  ||  abs(y - EnclosureTop) > tolerance)  )
	    {
		EnclosureExposed = true;
		EnclosureWidth  = x - EnclosureLeft;
		EnclosureHeight = y - EnclosureTop;
	    }
	    break;
	case view_LeftUp:
	    Tracking = false;
	    (ZipView)->Set_Pane_Cursor(  ForegroundPane, normal_cursor, "aptcsr20" );
	    if ( EnclosureExposed )
	    {
		EnclosureWidth  = x - EnclosureLeft;
		EnclosureHeight = y - EnclosureTop;
		if ( EnclosureWidth < 0 )
		{
		    EnclosureLeft += EnclosureWidth;
		    EnclosureWidth = abs(EnclosureWidth);
		}
		if ( EnclosureHeight < 0 )
		{
		    EnclosureTop += EnclosureHeight;
		    EnclosureHeight = abs(EnclosureHeight);
		}
	    }
	    break;
	default:
	    break;
    }
    Draw_Enclosure( self );
    OUT(Track_Enclosure);
}

void ltv_private::Cancel_Enclosure( class ltv  *self )
{
    IN(Cancel_Enclosure);
    Clear_Enclosure( self );
    EnclosureExposed = false;
    EnclosureLeft = EnclosureTop = EnclosureWidth = EnclosureHeight = 0;
    OUT(Cancel_Enclosure);
}

void ltv_private::Clear_Enclosure( class ltv *self )
{
    IN(Clear_Enclosure);
    Invert_Enclosure( self );
    OUT(Clear_Enclosure);
}

void ltv_private::Draw_Enclosure( class ltv	 *self )
{
    IN(Draw_Enclosure);
    Invert_Enclosure( self );
    OUT(Draw_Enclosure);
}

void ltv_private::Invert_Enclosure( class ltv *self )
{
    IN(Invert_Enclosure);
    if ( EnclosureExposed )
    {
	(self)->SetTransferMode(  graphic::INVERT );
	(self)->DrawRectSize(  EnclosureLeft, EnclosureTop, EnclosureWidth, EnclosureHeight );
	(self)->DrawRectSize(  EnclosureLeft-1, EnclosureTop-1, EnclosureWidth+2, EnclosureHeight+2 );
	(self )->FlushGraphics( );
    }
    OUT(Invert_Enclosure);
}

/*******************************************************************\

    Neighbor is recognized iff the point nearest to the pixel x,y is
    either a Start or and End Point of that neighbor.

\*******************************************************************/
long ltv_private::Neighbor( class ltv *self, zip_type_pixel x , zip_type_pixel y, zip_type_figure    *figure, zip_type_point *X , zip_type_point *Y, long *point )
{
    long		      status = false;

    IN(Neighbor);
    *point = 0;
    if ( ( *figure = (ZipView)->Which_Pane_Figure(  x, y, ForegroundPane ) ) )
    { DEBUGst(Near Figure,(*figure)->zip_figure_name);
    if ( ( *point = Which_Figure_Point( self, *figure, ForegroundPane, x, y ) ) )
    {
/*===zipview_Figure_Point...*/
	if ( *point == 1 )
	{
	    status = true;
	    *X = (*figure)->zip_figure_point.zip_point_x;
	    *Y = (*figure)->zip_figure_point.zip_point_y;
	}
	else
	{
	    if ( (*figure)->zip_figure_points->zip_points_count == (*point - 1)  )
	    {
		status = true;
		*X = (*figure)->zip_figure_points->zip_points[*point-2].zip_point_x;
		*Y = (*figure)->zip_figure_points->zip_points[*point-2].zip_point_y;
	    }
	}
/*===*/
    }
    }
    OUT(Neighbor);
    return status;
}

void
ltv_private::Normalize_Command( class ltv *self )
{
    IN(Normalize_Command);
    Cancel_Enclosure( self );
    (ZipView)->Normalize_Pane(  ForegroundPane );
    BackgroundXOffset = BackgroundYOffset = 0;
    (ZipView)->Clear_Pane(  ForegroundPane );
    Show_Background( self );
    OUT(Normalize_Command);
}

void
ltv_private::Zoom_In_Command( class ltv *self )
{
    IN(Zoom_In_Command);
    Cancel_Enclosure( self );
    (ZipView)->Set_Pane_Zoom_Level(  ForegroundPane,
				   (ZipView)->Pane_Zoom_Level(  ForegroundPane ) + 1 );
    if ( BackgroundExposed )
	Show_Background( self );
    else
	(ZipView)->Display_Pane(  ForegroundPane );
    OUT(Zoom_In_Command);
}

void
ltv_private::Zoom_Out_Command( class ltv *self )
{
    IN(Zoom_Out_Command);
    Cancel_Enclosure( self );
    if ( (ZipView)->Pane_Zoom_Level(  ForegroundPane ) > 0 )
    {
	(ZipView)->Set_Pane_Zoom_Level(  ForegroundPane,
				       (ZipView)->Pane_Zoom_Level(  ForegroundPane ) - 1 );
	if ( BackgroundExposed )
	    Show_Background( self );
	else
	    (ZipView)->Display_Pane(  ForegroundPane );
    }
    else (ZipView)->Announce(  "Already at Zoom Level Zero" );
    OUT(Zoom_Out_Command);
}

void
ltv_private::Scale_Pane( class ltv *self, float  scale )
{
/*  float	       x, y; */

    IN(Scale_Pane);
    DEBUGgt( Scale, scale);
    Cancel_Enclosure( self );
    (ZipView)->Set_Pane_Scale(  ForegroundPane,
			      (ZipView)->Pane_Scale(  ForegroundPane ) + scale );
/*===
  x = zipview_X_Pixel_To_Point( ZipView, ForegroundPane, NULL,
    zipview_Pane_Left( ZipView, ForegroundPane ) +
		      (zipview_Pane_Width( ZipView, ForegroundPane )/2) );
  y = zipview_Y_Pixel_To_Point( ZipView, ForegroundPane, NULL,
    zipview_Pane_Top( ZipView, ForegroundPane ) +
		     (zipview_Pane_Height( ZipView, ForegroundPane )/2) );
  zipview_Scale_Pane_To_Point( ZipView, ForegroundPane, x, y,
    Scale, zip_center|zip_middle );
===*/
    if ( BackgroundExposed )
	Show_Background( self );
    else
	(ZipView)->Display_Pane(  ForegroundPane );
    OUT(Scale_Pane);
}

void
ltv_private::Scale_Normal_Command( class ltv  *self )
{
    (ZipView)->Set_Pane_Scale(  ForegroundPane, 1.0 );
    Scale_Pane( self, 0.0 );
}

void
ltv_private::Scale_Smaller_Command( class ltv *self )
{  Scale_Pane( self, -0.01 );  }

void
ltv_private::Scale_Smaller_10_Command( class ltv *self )
{  Scale_Pane( self, -0.1 );  }

void
ltv_private::Scale_Larger_Command( class ltv *self )
    {  Scale_Pane( self, 0.01 );  }

void
ltv_private::Scale_Larger_10_Command( class ltv *self )
    {  Scale_Pane( self, 0.1 );  }

void
ltv_private::Scale_Half_Command( class ltv  *self )
{
    IN(Scale_Half_Command);
    (ZipView)->Set_Pane_Scale(  ForegroundPane,
			      (ZipView)->Pane_Scale(  ForegroundPane ) * 0.5 );
    Scale_Pane( self, 0.0 );
    OUT(Scale_Half_Command);
}

void
ltv_private::Scale_Double_Command( class ltv  *self )
{
    IN(Scale_Double_Command);
    (ZipView)->Set_Pane_Scale(  ForegroundPane,
			      (ZipView)->Pane_Scale(  ForegroundPane ) * 2.0 );
    Scale_Pane( self, 0.0 );
    OUT(Scale_Double_Command);
}

void
ltv_private::Pan_Foreground_Command( class ltv *self )
{
    IN(Pan_Foreground_Command);
    (Menu)->SetMask(  ((Menu )->GetMask( ) & ~pan_foreground) | pan_together );
    (self)->PostMenus(  Menu );
    ForegroundPanning = true;
    OUT(Pan_Foreground_Command);
}

void
ltv_private::Pan_Together_Command( class ltv *self )
{
    IN(Pan_Together_Command);
    Cancel_Enclosure( self );
    (Menu)->SetMask(  ((Menu )->GetMask( ) & ~pan_together) | pan_foreground );
    (self)->PostMenus(  Menu );
    ForegroundPanning = false;
    OUT(Pan_Together_Command);
}

void
ltv_private::Fit_Command( class ltv *self )
{
    float	      scale, EW, EH;
    zip_type_point     x, y;

    IN(Fit_Command);
    if ( EnclosureExposed )
    {
	scale = (ZipView)->Pane_Scale(  ForegroundPane );
	EW = abs( EnclosureWidth);  EH = abs( EnclosureHeight );
	if ( EW > EH )
	    scale *= (1.0 / (EW / (ZipView)->Pane_Width(  ForegroundPane )));
	else
	    scale *= (1.0 / (EH / (ZipView)->Pane_Height(  ForegroundPane )));
	x = (ZipView)->X_Pixel_To_Point(  ForegroundPane, NULL, EnclosureLeft + EnclosureWidth/2 );
	y = (ZipView)->Y_Pixel_To_Point(  ForegroundPane, NULL, EnclosureTop + EnclosureHeight/2 );
	Cancel_Enclosure( self );
	(ZipView)->Scale_Pane_To_Point(  ForegroundPane, x, y, scale, zip_center | zip_middle );
	Show_Background( self );
    }
    else  (ZipView)->Announce(  "No Selected Region" );
    OUT(Fit_Command);
}

void ltv_private::Show_Chain_Names( class ltv *self )
{
    char			     *left_name, *right_name;

    IN(Show_Chain_Names);
    if ( Figure->zip_figure_name )
    {
	Split_Chain_Name( self, Figure, &right_name, &left_name );
	if ( LeftNameItem )
	    ChangeItemCaption( Buttons, LeftNameItem, left_name );
	if ( RightNameItem )
	    ChangeItemCaption( Buttons, RightNameItem, right_name );
	im::ForceUpdate();
    }
    else  (ZipView)->Announce(  "Un-named Chain." );
    OUT(Show_Chain_Names);
}

void ltv_private::Clear_Chain_Names( class ltv  *self )
{
    IN(Clear_Chain_Names);
    if ( LeftNameItem )
	ChangeItemCaption( Buttons, LeftNameItem, "" );
    if ( RightNameItem )
	ChangeItemCaption( Buttons, RightNameItem, "" );
/*===*/ (ZipView)->Announce(  " " );
OUT(Clear_Chain_Names);
}

void ltv_private::Begin_Chain_Button( class ltv  *self, class suite   *suite, struct suite_item     *item, long type, enum view_MouseAction  action, long x, long y, long clicks )
{
    IN(Begin_Chain_Button);
    if ( type == suite_ItemObject  &&  action == view_LeftUp )
    {
	(self)->WantInputFocus(  self );
	Cancel_Enclosure( self );
	(ZipView)->Set_Pane_Cursor(  ForegroundPane, build_cursor, "icon12" );
	Building = Build = true;
	(ZipView)->Announce(  " ");
	Passivate( self, begin_code );
	Activate( self, end_code  );
	Passivate( self, delete_code );
	Passivate( self, rename_code );
    }
    OUT(Begin_Chain_Button);   
}

void ltv_private::End_Chain_Button( class ltv *self, class suite  *suite, struct suite_item	      *item, long type, enum view_MouseAction action, long x, long y, long clicks )
{
    IN(End_Chain_Button);
    if ( Building  &&  type == suite_ItemObject  &&  action == view_LeftUp )
    {
	(self)->WantInputFocus(  self );
	End_Chain( self );
    }
    OUT(End_Chain_Button);  
}

void ltv_private::End_Chain( class ltv  *self )
{
    IN(End_Chain);
    Passivate( self, end_code );
    Activate( self, begin_code );
    Activate( self, rename_code );
    Activate( self, delete_code );
    (ZipView)->Set_Pane_Cursor(  ForegroundPane, normal_cursor, "aptcsr20" );
    Building = false;
    Name_Chain( self );
    OUT(End_Chain);
}

void ltv_private::Delete_Chain_Button( class ltv *self, class suite  *suite, struct suite_item	      *item, long type, enum view_MouseAction action, long x, long y, long clicks )
{
    IN(Delete_Chain_Button);
    Building = false;
    if ( Figure )
    {
	if ( type == suite_ItemObject  &&  action == view_LeftUp )
	{
	    (self)->WantInputFocus(  self );
	    (ZipView)->Clear_Figure(  Figure, ForegroundPane );
	    (Zip)->Destroy_Figure(  Figure );
	    Figure = NULL;
	    Clear_Chain_Names( self );
	    Passivate( self, delete_code );
	    Passivate( self, rename_code );
	}
    }
    else  (ZipView)->Announce(  "No Chain Selected." );
    OUT(Delete_Chain_Button);
}

long
ltv_private::Rename_Exception( class ltv  *self, char *facility, long  status  )
{
    char				      msg[512];
    char				      chain_name[512];
    char			     *ptr;
    boolean		      duplicate = true;
    long			      number = 0;

    IN(Exceptions);
    self = SELF; /*===*/
     (Zip)->Set_general_Exception_Handler(  NULL );
     if ( status == zip_duplicate_figure_name )
     {
	 strcpy( msg, TentativeName );
	 DEBUGst(Original Chain-name,msg);
	 if ( ( ptr = strchr( msg, '[' ) ) )
	     *ptr = 0;
	 while ( duplicate )
	 {
	     sprintf( chain_name, "%s[%ld]", msg, number );
	     DEBUGst(New Chain-name,chain_name);
	     if ( (Zip)->Set_Figure_Name(  Figure, chain_name ) == zip_ok )
		 duplicate = false;
	     else  number++;
	 }
	 sprintf( msg, "Duplicate Chain Name.  Re-named to '%s'", chain_name );
     }
     else
	 sprintf( msg, "Rename-Exception  Facility = %s  Status = %ld", facility, status );
     (ZipView)->Announce(  msg );
     (Zip)->Set_general_Exception_Handler(  (zip_generalexceptfptr) Exceptions );
     OUT(Exceptions);
     return  0;
}

void ltv_private::Rename_Chain_Button( class ltv *self, class suite *suite, struct suite_item	      *item, long type, enum view_MouseAction action, long x, long y, long clicks )
{
    IN(Rename_Chain_Button);
    if ( type == suite_ItemObject  &&  action == view_LeftUp )
    {
	(self)->WantInputFocus(  self );
	Name_Chain( self );
    }
    OUT(Rename_Chain_Button); 
}

void ltv_private::Left_Chain_Button( class ltv *self, class suite *suite, struct suite_item	      *item, long type, enum view_MouseAction action, long x, long y, long clicks )
{
    IN(Left_Chain_Button);
    if ( type == suite_ItemObject  &&  action == view_LeftUp )
    {
/*===*/
	(self)->WantInputFocus(  self );
	(Buttons)->NormalizeItem(  (Buttons)->ItemOfDatum(  left_code  ) );
    }
    OUT(Left_Chain_Button); 
}

void ltv_private::Right_Chain_Button( class ltv *self, class suite	  *suite, struct suite_item	      *item, long type, enum view_MouseAction   action, long x, long y, long clicks )
{
    IN(Right_Chain_Button);
    if ( type == suite_ItemObject  &&  action == view_LeftUp )
    {
/*===*/
	(self)->WantInputFocus(  self );
	(Buttons)->NormalizeItem(  (Buttons)->ItemOfDatum(  right_code  ) );
    }
    OUT(Right_Chain_Button); 
}

void ltv_private::Name_Chain( class ltv  *self )
{
    char			     *reply, *left_name, *right_name;

    IN(Name_Chain);
    if ( Figure )
    {
	Split_Chain_Name( self, Figure, &right_name, &left_name );
	while ( (ZipView)->Query(  "Enter Chain-Right Name: ",
				 right_name, &reply ) );
	strcpy( right_name, reply );
	while ( (ZipView)->Query(  "Enter Chain-Left Name: ",
				 left_name, &reply ) );
	strcpy( left_name, reply );
	(Zip)->Set_general_Exception_Handler(   (zip_generalexceptfptr) Rename_Exception );
	sprintf( TentativeName, "%s,%s", right_name, left_name );
	if ( (Zip)->Set_Figure_Name(  Figure, TentativeName ) == zip_ok )
	{
	    Modified = true;
	    (ZipView)->Announce(  " " );
	    Show_Chain_Names( self );
	    (Buttons)->NormalizeItem(  (Buttons)->ItemOfDatum(  rename_code  ) );
	}
    }
    else  (ZipView)->Announce(  "No Chain Selected." );
    OUT(Name_Chain);
}

void ltv_private::Split_Chain_Name( class ltv *self, zip_type_figure  figure, char		     **right_name, char **left_name  )
{
    static char		      left[257], right[257],
    full[257], *comma;

    *left = *right = 0;
    *left_name = left;
    *right_name = right;
    strcpy( full, figure->zip_figure_name );
    if ( ( comma = strchr( full, ',' ) ) )
    {
	*comma = 0;
	strcpy( right, full );
	strcpy( left, comma + 1 );
    }
}

void ltv_private::Passivate( class ltv *self, long  datum )
{
    (Buttons)->PassivateItem(  (Buttons)->ItemOfDatum(  datum ) );
}

void ltv_private::Activate( class ltv	      *self, long		       datum )
{
    (Buttons)->ActivateItem(  (Buttons)->ItemOfDatum(  datum ) );
}

void
ltv_private::Lighten_Background_Command( class ltv  *self )
{
    IN(Lighten_Background_Command);
    if ( BackgroundLight == 0 ) {
	if ( WMWM ) BackgroundLight = '4';  else BackgroundLight = '6';
    }
    if ( BackgroundLight > '1'  &&  BackgroundLight < '9' )
    {
	if ( WMWM )
	    BackgroundLight += 1;
	else
	    BackgroundLight -= 1;
	Lighten_Background( self );
    }
    else  (ZipView)->Announce(  "Already as Light as Possible." );
    OUT(Lighten_Background_Command);
}

void ltv_private::Lighten_Background( class ltv *self )
{
    if ( WMWM )
	(self)->SetTransferMode(  graphic::WHITE );
    else
	(self)->SetTransferMode(  graphic::AND );
    (self)->FillTrapezoid(  Left, Top, Width,  Left, Bottom-ButtonHeight,Width,
			  (ZipView)->Define_Graphic( 
						    (Zip)->Define_Font(  "zipshades16", NULL ), BackgroundLight ) );
    (ZipView )->Redraw_Panes( );
    if ( EnclosureExposed )
	Draw_Enclosure( self );
}

void
ltv_private::Darken_Background_Command( class ltv	*self )
{
    IN(Darken_Background_Command);
    BackgroundLight = 0;
    Show_Background( self );
    if ( EnclosureExposed )
	Draw_Enclosure( self );
    OUT(Darken_Background_Command);
}

void
ltv_private::Hide_Background_Command( class ltv  *self )
{
    IN(Hide_Background_Command);
    BackgroundExposed = false;
    (ZipView)->Clear_Pane(  ForegroundPane );
    (ZipView )->Redisplay_Panes( );
    if ( EnclosureExposed )
	Draw_Enclosure( self );
    (Menu)->SetMask(  ((Menu )->GetMask( ) & ~hide_background) | expose_background );
    (self)->PostMenus(  Menu );
    OUT(Hide_Background_Command);
}

void
ltv_private::Expose_Background_Command( class ltv *self )
{
    IN(Expose_Background_Command);
    BackgroundExposed = true;
    Show_Background( self );
    if ( EnclosureExposed )
	Draw_Enclosure( self );
    (Menu)->SetMask(  ((Menu )->GetMask( ) & ~expose_background) | hide_background  );
    (self)->PostMenus(  Menu );
    OUT(Expose_Background_Command);
}

void ltv_private::Show_Background( class ltv *self )
{
    long		      height = (Height - ButtonHeight) - BackgroundTopY;

    IN(Show_Background);
    if ( BackgroundExposed )
    {
	(RasterView)->InsertViewSize(  self,
				     BackgroundLeftX, BackgroundTopY, BackgroundWidth, height );
	(RasterView)->FullUpdate(  view_FullRedraw,
				 BackgroundLeftX, BackgroundTopY, BackgroundWidth, height );
	if ( BackgroundLight )
	    Lighten_Background( self );
	else
	    (ZipView )->Redraw_Panes( );
    }
    else  (ZipView )->Redraw_Panes( );
    OUT(Show_Background);
}

void
ltv_private::Save_Command( class ltv *self )
{
    char			      msg[512];
    long		      status;

    IN(Save_Command);
    if ( (status = (Zip)->Write_Stream(  StreamLocal )) == zip_ok )
	sprintf( msg, "Wrote File '%s'", StreamLocal->zip_stream_name );
    else
	sprintf( msg, "Error Writing File '%s'  (%ld)", StreamLocal->zip_stream_name, status );
    Modified = false;
    (ZipView)->Announce(  msg );
    OUT(Save_Command);
}

void
ltv_private::Print_Command( class ltv  *self )
{
    char			      msg[512];
    long		      status;
    FILE		     *file;

    IN(Print_Command);
    (ZipView )->Use_Working_Pane_Cursors( );
    file = fopen( "tmp_print", "w" );
    (ZipView)->Set_Print_Language(  "PostScript" );
    (ZipView)->Set_Print_Processor(  "PostScript" );
    (ZipView)->Set_Print_Level(  1 );
    (ZipView)->Set_Print_File(  file );
    (ZipView)->Set_Print_Dimensions(  8.5, 11.0 );
    if ( (status = (ZipView)->Print_Stream(  StreamLocal, ForegroundPane )) == zip_ok )
    {
	fclose( file );
	system( "print -Tnative tmp_print" );
	sprintf( msg, "Printed File '%s'", StreamLocal->zip_stream_name );
    }
    else
	sprintf( msg, "Error Printing File '%s'  (%ld)", StreamLocal->zip_stream_name, status );
    (ZipView)->Announce(  msg );
    (ZipView )->Use_Normal_Pane_Cursors( );
    OUT(Print_Command);
}

void
ltv_private::Quit_Command( class ltv  *self )
{
    static const char	     * const choices[] =
    {"Cancel", "Save", "Save & Quit", "Quit Anyway", 0};
    long			      response = 0;
    long UNUSED result; // used with DB=1

    IN(Quit_Command);
    if ( Modified )
    {
	result =  message::MultipleChoiceQuestion(
						  self, 0, "Outstanding Modifications:", 0, &response, choices, NULL );
	DEBUGdt(Result,result);
	DEBUGdt(Response,response);
	switch ( response )
	{
	    case 0:
		break;
	    case 1:
		Save_Command( self );
		break;
	    case 2:
		Save_Command( self );
	    case 3:
		exit(0);
		break;
	    default:
		break;
	}
    }
    else  exit(0);
    OUT(Quit_Command);
}

void
ltv_private::Debug_Command( class ltv *self )
{
    IN(Debug_Command);
    debug = !debug;
    (Zip)->Set_Debug(  debug );
    (ZipView)->Set_Debug(  debug );
    OUT(Debug_Command);
}

static struct bind_Description	      menu[] = {
{   "ltv-save",		"\033s",    0,	    "Save~10",		0,  0,
    (proctable_fptr) ltv_private::Save_Command,		"Save",		    NULL},
{   "ltv-print",		"\033p",    0,	    "Print~20",		0,  0,
    (proctable_fptr) ltv_private::Print_Command,		"Print",	    NULL},
{   "ltv-quit",		"\033q",    0,	    "Quit~99",		0,  0,
    (proctable_fptr) ltv_private::Quit_Command,		"Quit",		    NULL},
{   "ltv-debug",		"\033z",    0,	    "DEBUG~88",		0,  0,
    (proctable_fptr) ltv_private::Debug_Command,		"Debug",	    NULL},

{   "ltv-hide-background",	"\033x",    0,	    "Background~20,Hide",0,  hide_background,
    (proctable_fptr) ltv_private::Hide_Background_Command,	"Hide Background",  NULL},
{   "ltv-expose-background",	"\033y",    0,	    "Background~20,Expose",0,  expose_background,
    (proctable_fptr) ltv_private::Expose_Background_Command,	"Expose Background", NULL},
{   "ltv-lighten-background","\033l",    0,	    "Background~20,Lighten",0,  0,
    (proctable_fptr) ltv_private::Lighten_Background_Command,	"Lighten Background", NULL},
{   "ltv-darken-background",	"\033d",    0,	    "Background~20,Darken",0,  0,
    (proctable_fptr) ltv_private::Darken_Background_Command,	"Darken Background", NULL},

{   "ltv-normalize",		"\033b",    0,	    "Pane~30,Normalize~00",0,  0,
    (proctable_fptr) ltv_private::Normalize_Command,		"Normalize",	    NULL},
{   "ltv-zoom_in",		"\033b",    0,	    "Pane~30,Zoom-In~10",	0,  0,
    (proctable_fptr) ltv_private::Zoom_In_Command,		"Zoom In",	    NULL},
{   "ltv-zoom_out",		"\033b",    0,	    "Pane~30,Zoom-Out~11",	0,  0,
    (proctable_fptr) ltv_private::Zoom_Out_Command,		"Zoom Out",	    NULL},
{   "ltv-pan_foreground",	"\033b",    0,	    "Pane~30,Pan Foreground~30",0,  pan_foreground,
    (proctable_fptr) ltv_private::Pan_Foreground_Command,	"Pan Foreground",    NULL},
{   "ltv-pan_together",	"\033b",    0,	    "Pane~30,Pan Together~30",	0,  pan_together,
    (proctable_fptr) ltv_private::Pan_Together_Command,	"Pan Together",	    NULL},
{   "ltv-fit",		"\033b",    0,	    "Pane~30,Fit~40",	0,  0,
    (proctable_fptr) ltv_private::Fit_Command,		"Fit",    NULL},

{   "ltv-scale-normal",	"\033s0",   0,	    "Pane Scale~40,Normal~00",0, 0,
    (proctable_fptr) ltv_private::Scale_Normal_Command,	"...",		    NULL },
{   "ltv-scale-100s",	"\033ssh",  0,	    "Pane Scale~40,100th Smaller~10",0,0,
    (proctable_fptr) ltv_private::Scale_Smaller_Command,	"...",		    NULL },
{   "ltv-scale-10s",		"\033sst",  0,	    "Pane Scale~40,10th Smaller~11", 0,0,
    (proctable_fptr) ltv_private::Scale_Smaller_10_Command,	"...",		    NULL },
{   "ltv-scale-half",        "\033sh",   0,	    "Pane Scale~40,Half Size~12", 0,0,
    (proctable_fptr) ltv_private::Scale_Half_Command,		"...",		    NULL },
{   "ltv-scale-100l",        "\033slh",  0,	    "Pane Scale~40,100th Larger~20", 0, 0,
    (proctable_fptr) ltv_private::Scale_Larger_Command,	"...",		     NULL },
{   "ltv-scale-10l",		"\033slt",  0,	    "Pane Scale~40,10th Larger~21",  0, 0,
    (proctable_fptr) ltv_private::Scale_Larger_10_Command,	"...",		    NULL },
{   "ltv-scale-double",	"\033sd",   0,	    "Pane Scale~40,Double Size~22",  0,0,
    (proctable_fptr) ltv_private::Scale_Double_Command,	"...",		    NULL },
NULL
};


void ltv_private::Build_Menu()
{
    IN(Build_Menu);
    bind::BindList( ::menu, class_keymap, class_menulist, &ltv_ATKregistry_  );
    OUT(Build_Menu);
}

long
ltv_private::Exceptions( class ltv *self, char *facility, long  status  )
{
    char			      msg[512];

    IN(Exceptions);
/*===*/
    self = SELF;
    sprintf( msg, "Exception  Status = %ld  Facility = %s", status, facility );
    (ZipView)->Announce(  msg );
    OUT(Exceptions);
    return  0;
}


void
ltv::LinkTree( class view *parent )
{
    class ltv *self=this;
    (this)->view::LinkTree( parent );
    if((this)->GetIM()) {
	if ( ZipView )
	    (ZipView)->LinkTree( (class view *) this);
	if ( RasterView )
	    (RasterView)->LinkTree( (class view *) this);
	if ( Buttons )
	    (Buttons)->LinkTree( (class view *) this);
   }
}
