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

/**  SPECIFICATION -- External Facility Suite  *********************************

TITLE	The Org View-object

MODULE	orgv.c

VERSION	1.0

AUTHOR	TC Peters
	Information Technology Center, Carnegie-Mellon University 

DESCRIPTION
	This is the suite of Methods that support the Org View-object.

    NB: The comment-symbol "===" indicates areas which are:
	    1 - Questionable
	    OR
	    2 - Arbitrary
	    OR
	    3 - Temporary Hacks
    Such curiosities need be resolved prior to Project Completion...


HISTORY
  01/19/89	Created (TCP)
  05/02/89	Introduce additional Attributes (TCP)
  05/04/89	Remove ForceUpdate in FullUpdate to prevent recursion in EZ (TCP)
  05/09/89	Improve placement of InputFocus Highlight lines (TCP)
  05/18/89	Have SetDataObject do super_SetDataObject (TCP)
  05/24/89	Change control-actions (TCP)
		Only access controls if exposed.
		Add Expose/Hide Description
		Force Write to DataStream to have ATK brackets
  05/25/89	Add Vertical/Horizontal Arrangement choice (TCP)
		Also Fold/UnFold choice
		Improve Menus
  05/26/89	Switch Horizontal & Vertical terminology (TCP)
  05/31/89	Add NodeBorder & Connector style to Menu (TCP)
		Provide initial set-up for new org-chart
		Ensure initial set-up only when necessary
  06/01/89	Split Menu into two cards (TCP)
  05/02/89	Automatically use Add command for new chart (TCP)
  06/06/89	Prevent infinite-loop on multiple Rename clicks (TCP)
		Also other bogus user missteps.
		Use WantUpdate for RIF & LIF
		(Curious need for printf in R.I.F. ...)
  06/07/89	Automatically expose Description area if Root has one (TCP)
		Make Palette taller
		Always expose Description with 50% of window
		Ensure ReadOnly-ness (when detectable)
  06/09/89	Temporarily cope with no horizontal explosion (TCP)
  06/23/89	Support Horizontal Explosion (TCP)
  07/12/89	Ensure update of Description appears only after resizing (TCP)
  07/25/89	Remove arg from im_ForceUpdate (TCP)
  07/31/89	Change invocation to suite: ChangeItemCaption to ChangeItemAttribute (TCP)
  08/23/89	Correct setting of ExposeControls/Description flags (TCP)
		Remove Create method
  08/24/89	Upgrade to Tree & Suite V1.0 Interfaces (TCP)
  08/31/89	Change OfData to OfDatum (TCP)

END-SPECIFICATION  ************************************************************/

#include <andrewos.h>
ATK_IMPL("orgv.H")
#include <ctype.h>
#include <errno.h>
#include <sys/stat.h>
#include <orgv.H>
#include <graphic.H>
#include <observable.H>
#include <view.H>
#include <message.H>
#include <fontdesc.H>
#include <im.H>
#include <lpair.H>
#include <text.H>
#include <textview.H>
#include <filetype.H>
#include <attribs.h>
#include <menulist.H>
#include <keymap.H>
#include <keystate.H>
#include <bind.H>
#include <rect.h>
#include <apts.H>
#include <aptv.H>
#include <org.H>
#include <tree.H>
#include <treev.H>
#include <suite.H>

static  class menulist		 *class_menulist;
static  class keymap		 *class_keymap;

#define  menu_default		  (1<<0)
#define  menu_buttons		  (1<<3)
#define  menu_exploded		  (1<<4)
#define  menu_imploded		  (1<<5)
#define  menu_description_hidden  (1<<6)
#define  menu_description_exposed (1<<7)
#define  menu_palette_hidden	  (1<<8)
#define  menu_palette_exposed	  (1<<9)
#define  menu_horizontal	  (1<<10)
#define  menu_vertical		  (1<<11)
#define  menu_folded		  (1<<12)
#define  menu_unfolded		  (1<<13)
#define  menu_debug		  (1<<14)


static const char		  ExplodePhrase[]   = "Explode",
				  ImplodePhrase[]   = "Implode",
				  VerticalPhrase[]  = "Vertical",
				  HorizontalPhrase[] = "Horizontal",
				  FoldPhrase[]	    = "Fold",
				  UnFoldPhrase[]    = "UnFold",
				  NodeBorderPhrase[] = "NodeBorder",
				  NodeConnectorPhrase[] = "NodeConnector",
				  RenamePhrase[]    = "Rename",
				  AddPhrase[]	    = "Add",
				  DeletePhrase[]    = "Delete",
				  DescriptionPhrase[] = "Description";

#define  Org			  (self->data_object)
#define  Anchor			  (self->anchor)
#define  Menu			  (self->menu)
#define  Keystate		  (self->keystate)
#define  Bounds			  (&self->bounds)
#define  Left			  (self->bounds.left)
#define  Top			  (self->bounds.top)
#define  Width			  (self->bounds.width)
#define  Height			  (self->bounds.height)
#define  Center			  (Left + Width/2)
#define  Middle			  (Top + Height/2)
#define  Bottom			  (Top + Height)
#define  PaletteHeight		  (85)
#define  PaletteTop		  (Top + Height - PaletteHeight)

#define  HitHandler		  (self->hit_handler)

#define  Tree			  (Org->tree_data_object)
#define  TreeView		  (self->tree_view_object)
#define  PairView		  (self->pair_view)
#define  DescriptionExposed	  (self->description_exposed)
#define  Description		  (self->description_text)
#define  DescriptionView	  (self->description_textview)
#define  DescriptionViewScroll	  (self->description_textview_scroll)
#define  PreviousNode		  (self->previous_node)
#define  Suite			  (self->suite_object)

#define  FirstTime		  (self->first_time)
#define  InputFocus		  (self->input_focus)
#define  PaletteExposed		  (self->controls_exposed)
#define  Exploded		  (self->exploded)
#define  ExplodedNode		  (self->exploded_node)
#define  IgnoreLoseInputFocus	  (self->ignore_loseinputfocus)
#define  IgnoreFullUpdate	  (self->ignore_fullupdate)

#define  Arrangement		  (self->arrangement)
#define  HorizontalArrangement	  (self->arrangement==treev_Horizontal)
#define  VerticalArrangement	  (self->arrangement==treev_Vertical)
#define  NodeBorderStyle	  (self->node_border_style)
#define  NodeConnectorStyle	  (self->node_connector_style)
#define  BackgroundShade	  (self->background_shade)
#define  Fold			  (self->fold)
#define  LastModified		  (self->last_modified)
#define  DescriptionLastModified  (self->description_last_modified)
#define  InitialNodeCount	  (self->initial_node_count)


ATKdefineRegistry(orgv, aptv, orgv::InitializeClass);
static long Control_Button_Hit( class orgv		   *self, class suite		   *suite, struct suite_item	   *item, long			    type, enum view_MouseAction    action, long			    x , long			    y , long			    clicks );
static void Add_Command( class orgv  *self );
static void Delete_Command( class orgv  *self );
static void Rename_Command( class orgv  *self );
static void Plode_Command( class orgv  *self );
static void Fold_Command( class orgv  *self );
static void Node_Border_Command( class orgv  *self );
static void Node_Connector_Command( class orgv  *self );
static void DEBUG_Command( class orgv	      *self );
static void Palette_Command( class orgv  *self );
static void Description_Command( class orgv  *self );
static void Arrangement_Command( class orgv  *self );
static void Alter_Control_Button( class orgv  *self, long  datum, const char  *new_c );
static void Passivate( class orgv  *self );
static void Activate( class orgv  *self );
static void FullUpdate_Tree( class orgv  *self );
static long Tree_Hit( class orgv		  *self, class treev	          *tree_view, struct tree_node	  *node, long			   type, enum view_MouseAction   action, long			   x , long			   y , long			   clicks );
static void Prepare_Description( class orgv  *self, struct tree_node  *node );


static treev_Specification specification[] = {
  treev_NodeFontName( "andysans10" ),
  treev_NodeBorderStyle( treev_Rectangle ),
  treev_NodeConnectorStyle( treev_DogLeg | treev_Fold ),
  treev_NodeFiligree( treev_DropShadow ),
  treev_NodeOrder( treev_ColumnMajor ),
  treev_HitHandler( Tree_Hit ),
  treev_Scroll( treev_Left | treev_Bottom ),
  treev_BackgroundShade( 25 ),
  treev_Cursor( 'z' ),
  treev_CursorFontName( "aptcsr20" ),
  0
};

#define  add_code	      1
#define  delete_code	      2
#define  rename_code	      3
#define  x_code		      4
#define  print_code	      5
#define  save_code	      6
#define  y_code		      7
#define  plode_code	      8
#define  description_code     9
#define  arrangement_code    10
#define  fold_code	     11
#define  node_border_code    12
#define  node_connector_code 13

boolean Orgv_Debug = 0;
#define debug Orgv_Debug


static suite_Specification add_button[] = {
  suite_ItemCaption(AddPhrase),
  suite_ItemDatum(add_code),  0
};

static suite_Specification delete_button[] = {
  suite_ItemCaption(DeletePhrase),
  suite_ItemDatum(delete_code),  0
};

static suite_Specification rename_button[] = {
  suite_ItemCaption(RenamePhrase),
  suite_ItemDatum(rename_code),  0
};

static suite_Specification plode_button[] = {
  suite_ItemCaption(ExplodePhrase),
  suite_ItemDatum(plode_code),  0
};

static suite_Specification description_button[] = {
  suite_ItemCaption(DescriptionPhrase),
  suite_ItemDatum(description_code),  0
};

static suite_Specification arrangement_button[] = {
  suite_ItemCaption(HorizontalPhrase),
  suite_ItemDatum(arrangement_code),  0
};

static suite_Specification fold_button[] = {
  suite_ItemCaption(UnFoldPhrase),
  suite_ItemDatum(fold_code),  0
};

static suite_Specification node_border_button[] = {
  suite_ItemCaption(NodeBorderPhrase),
  suite_ItemDatum(node_border_code),  0
};

static suite_Specification node_connector_button[] = {
  suite_ItemCaption(NodeConnectorPhrase),
  suite_ItemDatum(node_connector_code),  0
};



static suite_Specification control_buttons[] = {
  suite_Item( add_button ),
  suite_Item( delete_button ),
  suite_Item( rename_button ),
  suite_Item( plode_button ),
  suite_Item( description_button ),
  suite_Item( arrangement_button ),
  suite_Item( fold_button ),
  suite_Item( node_border_button ),
  suite_Item( node_connector_button ),
  suite_ItemCaptionFontName( "andysans10b" ),
  suite_HitHandler( Control_Button_Hit ),
  suite_Arrangement( suite_Matrix | suite_Fixed ),
  suite_Rows( 3 ), suite_Columns( 3 ),
  0
};


static struct bind_Description view_menu[] = {
  { "orgv-Add", "",	    0,	"Add Node~20",	 0, menu_default,
    (proctable_fptr) Add_Command,	"Add Node" },
  { "orgv-Delete", "",   0,	"Delete Node~21",0, menu_default,
    (proctable_fptr) Delete_Command,	"Delete Node" },
  { "orgv-Rename", "",   0,	"Rename Node~22",0, menu_default,
    (proctable_fptr) Rename_Command,	"Rename Node" },
  { "orgv-Plode", "",    0,	"Explode Nodes~30",0, menu_imploded,
    (proctable_fptr) Plode_Command,	"Plode Tree" },
  { "orgv-Plode", "",    0,	"Implode Nodes~30",0, menu_exploded,
    (proctable_fptr) Plode_Command,	"Plode Tree" },
  { "orgv-Description", "", 0,	"Expose Description~31",0, menu_description_hidden,
    (proctable_fptr) Description_Command,"Description" },
  { "orgv-Description", "", 0,	"Hide Description~31",0, menu_description_exposed,
    (proctable_fptr) Description_Command,"Description" },
  { "orgv-Palette",  "", 0,	"Expose Palette~40",	0, menu_palette_hidden,
    (proctable_fptr) Palette_Command,	"Palette-toggle" },
  { "orgv-Palette",  "", 0,	"Hide Palette~40",	0, menu_palette_exposed,
    (proctable_fptr) Palette_Command,	"Palette-toggle" },
  { "orgv-DEBUG",  "",   0,	"DEBUG~98",	0, menu_debug,
    (proctable_fptr) DEBUG_Command,	"Debug-toggle" },
  { "org-Arrangement", "", 0,	"Styling~10,Vertical Layout~10",0, menu_horizontal,
    (proctable_fptr) Arrangement_Command,"Arrangement" },
  { "org-Arrangement", "", 0,	"Styling~10,Horizontal Layout~10",0, menu_vertical,
    (proctable_fptr) Arrangement_Command,"Arrangement" },
  { "org-Fold", "", 0,		"Styling~10,Fold Nodes~73",11, menu_unfolded,
    (proctable_fptr) Fold_Command,	"Fold" },
  { "org-Fold", "", 0,		"Styling~10,UnFold Nodes~73",11, menu_folded,
    (proctable_fptr) Fold_Command,	"Fold" },
  { "orgv-Border", "",    0,	"Styling~10,Node Border~20",	0, menu_default,
    (proctable_fptr) Node_Border_Command,"Change Node Border" },
  { "orgv-Connector", "", 0,	"Styling~10,Node Connector~21",	0, menu_default,
    (proctable_fptr) Node_Connector_Command,"Change Node Connector" },
  NULL
};


boolean
orgv::InitializeClass( )
  {
  IN(orgv_InitializeClass );
  DEBUGst(RCSID,rcsid);
  class_menulist = new menulist;
  class_keymap = new keymap;
  bind::BindList( view_menu, class_keymap, class_menulist, &orgv_ATKregistry_  );
  OUT(orgv_InitializeClass );
  return(TRUE);
}

orgv::orgv( )
    {
	ATKinit;
  class orgv *self=this;
  boolean status = true;

  IN(orgv_InitializeObject);
  DEBUGst(RCSID,rcsid);
  (this)->SetOptions(  aptv_SuppressControl |
			 aptv_SuppressBorder |
			 aptv_SuppressEnclosures );
  InitialNodeCount = 0;
  Org = NULL;
  HitHandler = NULL;
  PreviousNode = NULL;
  InputFocus = PaletteExposed = DescriptionExposed = Exploded =
      IgnoreLoseInputFocus  = IgnoreFullUpdate = false;
  LastModified = DescriptionLastModified = 0;
  Arrangement = treev_Vertical;
  NodeConnectorStyle = treev_DogLeg;
  NodeBorderStyle = treev_Rectangle;
  FirstTime = Fold = true;
  Menu = (class_menulist)->DuplicateML(  this );
  (Menu)->SetView(  this );
  Keystate = keystate::Create( this, class_keymap );
  DEBUG(Create TreeView);
  if ( (TreeView = treev::Create( specification, this )) == NULL ) {
    printf( "OrgV: Unable to Create Tree View Object\n" );
    status = false;
  }
  if ( status == true ) { DEBUG(Created TreeView);
    (TreeView)->SetDebug(  debug );
    if ( (Suite = suite::Create( control_buttons, (long) this )) == NULL ) {
      printf( "OrgV: Unable to Create Suite Object\n" );
      status = false;
    }
  }
  if ( status == true ) { DEBUG(Created Suite);
    Description = new text;
    Description->InsertCharacters(0, "Add a root node", 15);
    Description->SetReadOnly(TRUE);
    DescriptionView = new textview;
    (DescriptionView)->SetDataObject(  Description );
    DescriptionViewScroll = (DescriptionView )->GetApplicationLayer( );
    (Description)->AddObserver(  this );
  }
  if ( status == true ) { DEBUG(Created Description);
    PairView = new lpair;
    (PairView)->VSplit(  TreeView, DescriptionViewScroll, 0, 1 );
  }
  OUT(orgv_InitializeObject);
  THROWONFAILURE((status));
}

orgv::~orgv( )
    {
	ATKinit;
  class orgv *self=this;
  IN(orgv_FinalizeObject );
  if ( Description ) {
      (PairView)->Destroy();
      (DescriptionView)->DeleteApplicationLayer(DescriptionViewScroll);
      (DescriptionView)->Destroy();
      (Description)->Destroy();
  }
  if ( TreeView )   (TreeView )->Destroy();
  if ( Suite )	    (Suite )->Destroy();
  if ( Menu )	    delete Menu ;
  if ( Keystate )   delete Keystate ;
  OUT(orgv_FinalizeObject );
}

void
orgv::SetDataObject( class dataobject  *ddata )
    {
    class orgv *self=this;
    class org *data=(class org *)ddata;
  IN(orgv_SetDataObject);
  (this)->aptv::SetDataObject(  data );
  Org = data;
  (Org)->AddObserver(  this );
  (TreeView)->SetDataObject(  Tree );
  InitialNodeCount = (Tree)->NodeCount(  (Tree )->RootNode( ) );
  OUT(orgv_SetDataObject);
}

void
orgv::ReceiveInputFocus( )
  {
  class orgv *self=this;
  IN(orgv_ReceiveInputFocus);
  InputFocus = true;
  if ( Keystate ) {
    Keystate->next = NULL;
    (this)->PostKeyState(  Keystate );
  }
  if ( Menu ) {
    (Menu)->SetMask(  (Menu )->GetMask( ) | menu_default |
	(((debug) ? menu_debug : 0) |
	 ((Exploded) ? menu_exploded : menu_imploded) |
	 ((DescriptionExposed) ? menu_description_exposed : menu_description_hidden) |
	 ((Fold) ? menu_folded : menu_unfolded) |
	 ((PaletteExposed) ? menu_palette_exposed : menu_palette_hidden) |
	 ((HorizontalArrangement) ? menu_horizontal : menu_vertical) ) );
    (this)->PostMenus(  Menu );
  }
  (this)->WantUpdate(  this );
  OUT(orgv_ReceiveInputFocus);
}

void
orgv::LoseInputFocus( )
  {
  class orgv *self=this;
  IN(orgv_LoseInputFocus);
  InputFocus = false; 
  if ( ! IgnoreLoseInputFocus ) {
    (this)->WantUpdate(  this );
  }
  OUT(orgv_LoseInputFocus);
}

void
orgv::SetDebug( boolean  state )
    {
  class orgv *self=this;
  IN(orgv_SetDebug);
  debug = state;
  if ( Org )		(Org)->SetDebug(  debug );
  if ( Org  &&  Tree )	(Tree)->SetDebug(  debug );
  if ( TreeView )	(TreeView)->SetDebug(  debug );
  OUT(orgv_SetDebug);
}

void
orgv::SetHitHandler( org_hitfptr handler, class view  *anchor )
      {
  class orgv *self=this;
  IN(orgv_SetHitHandler);
  HitHandler = handler;
  if ( anchor )
    Anchor = anchor;
  OUT(orgv_SetHitHandler);
}

void
orgv::FullUpdate( enum view_UpdateType  type, long  left , long  top , long  width , long  height )
      {
  class orgv *self=this;
  long controls = PaletteExposed * PaletteHeight;

  IN(orgv_FullUpdate);
  DEBUGdt(Type,type);
  if ( ! IgnoreFullUpdate  &&  (type == view_FullRedraw  ||  type == view_LastPartialRedraw) ) {
    (this)->aptv::FullUpdate(  type, left, top, width, height );
    Left = (this )->BodyLeft( );
    Top = (this )->BodyTop( );
    Width = (this )->BodyWidth( );
    Height = (this )->BodyHeight( );
    (this)->SetTransferMode(  graphic_COPY );
    if ( FirstTime ) { DEBUG(FirstTime);
      FirstTime = false;
      if ( (Tree )->RootNode( ) ) { DEBUG(RootNode);
	if ( (Tree)->NodeDatum(  (Tree )->RootNode( ) ) )
          DescriptionExposed = true;
      }
      if ( (Tree)->NodeCount(  (Tree )->RootNode( )) == 0 ) {
	DescriptionExposed = PaletteExposed = true;
	controls = PaletteExposed * PaletteHeight;
	(Menu)->SetMask(  ((Menu )->GetMask( ) &
			~menu_palette_hidden) | menu_palette_exposed );
      }
      if ( DescriptionExposed ) {
	  (Menu)->SetMask(  ((Menu )->GetMask( ) & 			 ~menu_description_hidden) | menu_description_exposed );
	  (PairView)->VSplit(  TreeView, DescriptionViewScroll, 40, 1 );
      }
      else {
	  (PairView)->VSplit(  TreeView, DescriptionViewScroll, 0, 1 );
	  (Menu)->SetMask(  ((Menu )->GetMask( ) & ~menu_description_exposed) | menu_description_hidden );
      }
      if ( InputFocus ) 
	  (this)->PostMenus(  Menu );
    }
    Prepare_Description( this, (TreeView )->CurrentNode( ) );
    (PairView)->LinkTree( this);
    (PairView)->InsertViewSize(  this, Left, Top, Width, (Height - controls) );
    (PairView)->FullUpdate(  type, 0,0, Width, (Height - controls) );
    if ( PaletteExposed ) { DEBUG(Palette Exposed);
      (Suite)->LinkTree(  this);
      (Suite)->InsertViewSize(  this, Left, PaletteTop, Width, PaletteHeight );
      (Suite)->FullUpdate(  type, 0,0, Width, PaletteHeight );
    }
  }
  OUT(orgv_FullUpdate);
}

class view *
orgv::Hit( enum view_MouseAction  action, long  x , long  y , long  clicks )
      {
  class orgv *self=this;
  class view *hit = (class view *) this;

  IN(orgv_Hit );
  (this)->Announce(  "" );
  if ( !InputFocus  &&  action == view_LeftDown ) { DEBUG(Grab IF);
    (this)->WantInputFocus(  this );
  }
  if ( InputFocus ) {
    IgnoreLoseInputFocus = true;
    if ( PaletteExposed  &&  y > PaletteTop ) { DEBUG(Palette ::Hit);
      hit = (class view *) (Suite)->Hit(  action,
	    (Suite)->EnclosedXToLocalX(  x ),
	    (Suite)->EnclosedYToLocalY(  y ), clicks );
    }
    else { DEBUG(Pair ::Hit);
      hit = (class view *) (PairView)->Hit(  action,
	    (PairView)->EnclosedXToLocalX(  x ),
	    (PairView)->EnclosedYToLocalY(  y ), clicks );
    }
    IgnoreLoseInputFocus = false;
  }
  else
      hit = (class view *) (PairView)->Hit(  action,
	    (PairView)->EnclosedXToLocalX(  x ),
	    (PairView)->EnclosedYToLocalY(  y ), clicks );
  OUT(orgv_Hit);
  return(hit);
}

view_DSattributes
orgv::DesiredSize( long		      given_width , long		      given_height, enum view_DSpass   pass, long		     *desired_width , long		     *desired_height )
        {
  class orgv *self=this;
  view_DSattributes result = view_WidthFlexible | view_HeightFlexible;
  IN(orgv_DesiredSize);
  if ( TreeView )
    result = (TreeView)->DesiredSize(  given_width, given_height, pass, desired_width, desired_height );
  OUT(orgv_DesiredSize);
  return(result);
}

static long
Control_Button_Hit( class orgv		   *self, class suite		   *suite, struct suite_item	   *item, long			    type, enum view_MouseAction    action, long			    x , long			    y , long			    clicks )
            {
  char msg[512];

  IN(Control_Button_Hit);
  DEBUGdt(Action,action);
  if ( type == suite_ItemObject  &&  action == view_LeftUp ) {
    switch ( (suite)->ItemAttribute(  item, suite_itemdatum ) ) {
      case  add_code:		Add_Command( self );		break;
      case  delete_code:	Delete_Command( self );		break;
      case  rename_code:	Rename_Command( self );		break;
      case  plode_code:		Plode_Command( self );		break;
      case  description_code:	Description_Command( self );	break;
      case  arrangement_code:	Arrangement_Command( self );    break;
      case  fold_code:		Fold_Command( self );		break;
      case  node_border_code:	Node_Border_Command( self );	break;
      case  node_connector_code:Node_Connector_Command( self );	break;
      default:
	sprintf( msg, "Unknown control-code (%ld)",
		    (suite)->ItemAttribute(  item, suite_itemdatum ) );
	(self)->Announce(  msg );
    } 
    (suite)->NormalizeItem(  item );
    if ( !InputFocus )
	(self)->WantInputFocus(  self );
  }
  OUT(Control_Button_Hit);
  return(0);
}

static void
Add_Command( class orgv  *self )
  {
  struct tree_node *node;
  char *reply;

  IN(Add_Command);
  while ( true ) {
      (self)->Query(  "Enter Node Name: ", "", &reply );
      if ( reply == NULL  ||  *reply == 0 )
	  break;
      if ( node = (Tree)->CreateChildNode(  "?", 0, (TreeView )->CurrentNode( ) ) ) {
	  (Tree)->SetNotificationCode(  tree_NodeCreated );
	  (Tree)->SetNotificationNode(  node );
	  (self)->Announce(  "" );
	  (Tree)->SetNodeName(  node, apts::StripString( reply ) );
	  (Tree)->NotifyObservers(  0 );
	  if(node == Tree->RootNode())
	      Prepare_Description( self, node );
	  (Org )->SetModified( );
	  if ( PaletteExposed )
	      Activate( self );
      }
      else {
	  (self)->Announce(  "Unable to Create Node." );
	  break;
      }
  }
  OUT(Add_Command);
}

static void
Delete_Command( class orgv  *self )
  {
  struct tree_node *current_node = (TreeView )->CurrentNode( );
  IN(Delete_Command);
  if ( current_node ) {
      (DescriptionView)->SetDataObject(  Description );
      (Tree)->SetNotificationCode(  tree_NodeDestroyed );
      (Tree)->SetNotificationNode(  current_node );
      (Tree)->NotifyObservers(  0 );
      (Tree)->DestroyNode(  current_node );
      (Org )->SetModified( );
      if ( PaletteExposed  &&  (Tree)->NodeCount(  (Tree )->RootNode( ) ) == 0 ) {
	  Passivate( self );
	  im::ForceUpdate();
      }
  }
  else
      (self)->Announce(  "Nothing to Delete." );
  OUT(Delete_Command);
}

static void
Rename_Command( class orgv  *self )
  {
  struct tree_node *current_node = (TreeView )->CurrentNode( );
  char *reply;

  IN(Rename_Command);
  if ( current_node ) {
      (self)->Query(  "Enter New Node Name: ",
		  (Tree)->NodeName(  current_node ), &reply );
      (self)->Announce(  "" );
      if ( reply  &&  *reply ) {
	  (Tree)->SetNodeName(  current_node, apts::StripString( reply ) );
	  (Tree)->SetNotificationCode(  tree_NodeNameChanged );
	  (Tree)->SetNotificationNode(  current_node );
	  (Tree)->NotifyObservers(  0 );
	  (Org )->SetModified( );
      }
  }
  else
      (self)->Announce(  "Nothing to Rename." );
  OUT(Rename_Command);
}

void
orgv::Print( FILE		      *file, const char		      *processor, const char		      *format, boolean	       level )
          {
  class orgv *self=this;
  IN(orgv_Print);
  (TreeView)->Print(  file, processor, format, level );
  OUT(orgv_Print);
}

static void
Plode_Command( class orgv  *self )
  {
  IN(Plode_Command);
  if ( (TreeView )->CurrentNode( ) ) {
    if ( Exploded = !Exploded ) {
      (TreeView)->ExplodeNode(  ExplodedNode = (TreeView )->CurrentNode( ) );
      Alter_Control_Button( self, plode_code, ImplodePhrase );
      (Menu)->SetMask(  ((Menu )->GetMask( ) & ~menu_imploded) | menu_exploded );
    }
    else {
      (TreeView)->ImplodeNode(  ExplodedNode );
      Alter_Control_Button( self, plode_code, ExplodePhrase );
      ExplodedNode = NULL;
      (Menu)->SetMask(  ((Menu )->GetMask( ) & ~menu_exploded) | menu_imploded );
    }
    (self)->PostMenus(  Menu );
  }
  else
      (self)->Announce(  "Nothing to Explode." );
  OUT(Plode_Command);
}

static void
Fold_Command( class orgv  *self )
  {
  IN(Fold_Command);
  if ( Fold = !Fold ) {
    (TreeView)->SetTreeAttribute(  treev_NodeConnectorStyle( treev_Fold | NodeConnectorStyle ) );
    Alter_Control_Button( self, fold_code, UnFoldPhrase );
    (Menu)->SetMask(  ((Menu )->GetMask( ) & ~menu_unfolded) | menu_folded );
  }
  else {
    (TreeView)->SetTreeAttribute(  treev_NodeConnectorStyle( treev_NoFold | NodeConnectorStyle ) );
    Alter_Control_Button( self, fold_code, FoldPhrase );
    (Menu)->SetMask(  ((Menu )->GetMask( ) & ~menu_folded) | menu_unfolded );
  }
  FullUpdate_Tree( self );
  OUT(Fold_Command);
}

static void
Node_Border_Command( class orgv  *self )
  {
  static const char * const choices[] = {"Cancel", "Rectangle", "Round", "Oval", "Circle", 0};
  long response = 0, style, state = 0;

  IN(Node_Border_Command);
  IgnoreLoseInputFocus = IgnoreFullUpdate = true;
  if (NodeBorderStyle==treev_Rectangle)	    state = 1; 
  if (NodeBorderStyle==treev_RoundAngle)    state = 2;
  if (NodeBorderStyle==treev_Oval)	    state = 3;
  if (NodeBorderStyle==treev_Circle)	    state = 4;
  message::MultipleChoiceQuestion(
	    self, 0, "Choose Style:", state, &response, choices, NULL );
  DEBUGdt(Response,response);
  switch ( response ) {
      case 0:	  break;
      case 1:	  style = treev_Rectangle;	break;
      case 2:	  style = treev_RoundAngle;	break;
      case 3:	  style = treev_Oval;		break;
      case 4:	  style = treev_Circle;		break;
      default:    response = 0;		 break;
  }
  if ( response  &&  style != NodeBorderStyle )
    (TreeView)->SetTreeAttribute( 
	treev_NodeBorderStyle( (NodeBorderStyle = style) ) );
  IgnoreLoseInputFocus = IgnoreFullUpdate = false;
  (self)->FullUpdate(  view_FullRedraw, 0, 0, Width-3, Height-3 );
  OUT(Node_Border_Command);
}

static void
Node_Connector_Command( class orgv  *self )
  {
  static const char * const choices[] = {"Cancel", "Dog Leg", "Direct", 0};
  long response = 0, style, state = 0;

  IN(Node_Connector_Command);
  IgnoreLoseInputFocus = IgnoreFullUpdate = true;
  state = (NodeConnectorStyle == treev_DogLeg) ? 1 : 2;
  message::MultipleChoiceQuestion(
	    self, 0, "Choose Style:", state, &response, choices, NULL );
  DEBUGdt(Response,response);
  switch ( response ) {
      case 0:	  break;
      case 1:	  style = treev_DogLeg;  break;
      case 2:	  style = treev_Direct;  break;
      default:    response = 0;		 break;
  }
  if ( response  &&  style != NodeConnectorStyle ) {
    DEBUGxt(style,((Fold) ? treev_Fold : treev_NoFold) | (NodeConnectorStyle = style));
    (TreeView)->SetTreeAttribute( 
	treev_NodeConnectorStyle( ((Fold) ? treev_Fold : treev_NoFold) |
	    (NodeConnectorStyle = style) ) );
  }
  IgnoreLoseInputFocus = IgnoreFullUpdate = false;
  (self)->FullUpdate(  view_FullRedraw, 0, 0, Width-3, Height-3 );
  OUT(Node_Connector_Command);
}

static void
DEBUG_Command( class orgv	      *self )
  {
  IN(DEBUG_Command);
  (self)->SetDebug(  !debug );
  (Org)->SetDebug(  debug );
  OUT(DEBUG_Command);
}

static void
Palette_Command( class orgv  *self )
  {
  IN(Palette_Command);
  if ( PaletteExposed = !PaletteExposed )
    (Menu)->SetMask(  ((Menu )->GetMask( ) & ~menu_palette_hidden) | menu_palette_exposed );
  else
      (Menu)->SetMask(  ((Menu )->GetMask( ) & ~menu_palette_exposed) | menu_palette_hidden );
  (self)->FullUpdate(  view_FullRedraw, 0, 0, Width, Height );
  (self)->PostMenus(  Menu );
  OUT(Palette_Command);
}

static void
Description_Command( class orgv  *self )
  {
  IN(Description_Command);
  if ( DescriptionExposed = !DescriptionExposed ) {
      (PairView)->VSplit(  TreeView, DescriptionViewScroll, 40, 1 );
      (Menu)->SetMask(  ((Menu )->GetMask( ) &
			~menu_description_hidden) | menu_description_exposed );
  }
  else {
      (PairView)->VSplit(  TreeView, DescriptionViewScroll, 0, 1 );
      (Menu)->SetMask(  ((Menu )->GetMask( ) & ~menu_description_exposed) | menu_description_hidden );
  }
  (PairView)->FullUpdate(  view_FullRedraw, 0, 0, Width, Height - (PaletteExposed * PaletteHeight));
  (self)->SetTransferMode(  graphic_COPY );
  (self)->PostMenus(  Menu );
  OUT(Description_Command);
}


static void
Arrangement_Command( class orgv  *self )
  {
  IN(Arrangement_Command);
  if ( HorizontalArrangement ) {
      Arrangement = treev_Vertical;
      (TreeView)->SetTreeAttribute(  treev_Arrangement( treev_Vertical ) );
      (TreeView)->SetTreeAttribute(  treev_Cursor( 'z' ) );
      Alter_Control_Button( self, arrangement_code, HorizontalPhrase );
      (Menu)->SetMask(  ((Menu )->GetMask( ) & ~menu_horizontal) | menu_vertical );
  }
  else {
      Arrangement = treev_Horizontal;
      (TreeView)->SetTreeAttribute(  treev_Arrangement( treev_Horizontal ) );
      (TreeView)->SetTreeAttribute(  treev_Cursor( 'b' ) );
      Alter_Control_Button( self, arrangement_code, VerticalPhrase );
      (Menu)->SetMask(  ((Menu )->GetMask( ) & ~menu_vertical) | menu_horizontal );
  }
  FullUpdate_Tree( self );
  OUT(Arrangement_Command);
}

static
void Alter_Control_Button( class orgv  *self, long  datum, const char  *new_c )
      {
  if ( PaletteExposed )
      (Suite)->ChangeItemAttribute(  (Suite)->ItemOfDatum(  datum ),
				suite_ItemCaption(new_c) );
  else
      (Suite)->SetItemAttribute(  (Suite)->ItemOfDatum(  datum ),
	    suite_ItemCaption( new_c ) );
}

static
void Passivate( class orgv  *self )
    {
  if ( PaletteExposed ) {
    (Suite)->PassivateItem(  (Suite)->ItemOfDatum(  plode_code   ) );
    (Suite)->PassivateItem(  (Suite)->ItemOfDatum(  rename_code  ) );
    (Suite)->PassivateItem(  (Suite)->ItemOfDatum(  delete_code  ) );
    (Suite)->PassivateItem(  (Suite)->ItemOfDatum(  print_code   ) );
    (Suite)->PassivateItem(  (Suite)->ItemOfDatum(  save_code    ) );
  }
}

static
void Activate( class orgv  *self )
  {
    if ( PaletteExposed ) {
	(Suite)->ActivateItem(  (Suite)->ItemOfDatum(  plode_code   ) );
	(Suite)->ActivateItem(  (Suite)->ItemOfDatum(  rename_code  ) );
	(Suite)->ActivateItem(  (Suite)->ItemOfDatum(  delete_code  ) );
	(Suite)->ActivateItem(  (Suite)->ItemOfDatum(  print_code   ) );
	(Suite)->ActivateItem(  (Suite)->ItemOfDatum(  save_code    ) );
    }
}

static
void FullUpdate_Tree( class orgv  *self )
  {
  struct rectangle bounds;

  ((PairView)->GetNth(  0 ))->GetLogicalBounds(  &bounds );
  ((PairView)->GetNth(  0 ))->FullUpdate(  view_FullRedraw, bounds.left, bounds.top, bounds.width, bounds.height );
  (self)->Announce(  "" );
  (self)->PostMenus(  Menu );
}

static long
Tree_Hit( class orgv		  *self, class treev	          *tree_view, struct tree_node	  *node, long			   type, enum view_MouseAction   action, long			   x , long			   y , long			   clicks )
            {
  IN(Tree_Hit);
  DEBUGdt(Type,type);
  DEBUGdt(Action,action);
  if ( type == treev_NodeObject ) { DEBUG(Node ::Hit);
    if ( action == view_LeftDown  &&  node != PreviousNode ) {
	PreviousNode = node;
	(TreeView)->HighlightNode(  node );
	if ( ! (TreeView)->NodeExploded(  node ) ) {
	    if ( Exploded )
		Plode_Command( self );
	    (TreeView)->ExposeNodeChildren(  node );
	}
	Prepare_Description( self, node );
    }
    if ( HitHandler )
	(HitHandler)((long) Anchor, self, node, action, x, y, clicks );
  }
  OUT(Tree_Hit);
  return(0);
}

static
void Prepare_Description( class orgv  *self, struct tree_node  *node )
    {
  FILE *file;
  class text *textp;

  IN(Prepare_Description);
  if ( DescriptionExposed ) {
    (self )->UseWaitCursor( );
    if (( textp = (class text *) (Tree)->NodeDatum( node)) == NULL ) {
	(Tree)->SetNodeDatum(  node, (long) (textp = new text));
	if(!Tree->RootNode()) { /* some sort of pseudo-node in empty tree */
	    textp->InsertCharacters(0, "Add a root node", 15);
	    textp->SetReadOnly(TRUE);
	}
    }
    if(DescriptionView->GetDataObject() != textp)
      (DescriptionView)->SetDataObject( textp);
    DescriptionLastModified = (textp )->GetModified( );
    (self )->UseNormalCursor( );
  }
  OUT(Prepare_Description);
}

void
orgv::ObservedChanged( class observable	      *changed, long			       change )
        {
  IN(orgv_ObservedChanged);
/*=== needed ? */
  OUT(orgv_ObservedChanged);
  }

void
orgv::LinkTree(class view  *parent)
        {
  class orgv *self=this;
    (this)->aptv::LinkTree( parent);
    if(parent && (this)->GetIM()) {
	(PairView)->LinkTree( (class view *) this);
	if ( PaletteExposed )
	    (Suite)->LinkTree(  this );
    }
}
