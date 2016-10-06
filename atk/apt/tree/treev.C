/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

/**  SPECIFICATION -- External Facility Suite  *********************************

TITLE	The Tree View-object

MODULE	treev.c

VERSION	1.0

NOTICE	IBM Internal Use Only

AUTHOR	TC Peters
	Information Technology Center, Carnegie-Mellon University 

DESCRIPTION
	This is the suite of Methods that support the Tree View-object.

    NB: The comment-symbol "===" indicates areas which are:
	    1 - Questionable
	    OR
	    2 - Arbitrary
	    OR
	    3 - Temporary Hacks
    Such curiosities need be resolved prior to Project Completion...


HISTORY
  02/14/88	Created (TCP)
  05/05/89	Use SetPrintStream to pass print-setttings to pseudo-view (TCP)
		Remove ForceUpdate from FullUpdate
  05/11/89	Fix overwriting of fold-box bottom line (TCP)
		Reduce Sizing of Node borders
		Change placement of captions from ..BASELINE to ..BOTTOM
		Change Check_Dimensions to use Update instead of FullUpdate
  05/24/89	Support Vertical arrangement (TCP)
  05/26/89	Switch Horizontal & Vertical terminology (TCP)
  05/30/89	Prevent dump in FinalizeObject/Destroy_Shadows (TCP)
		Fix Explode to check for exposed children -- VAX detected bug
  05/31/89	Fix changing of NodeBorder to Circle to use Set_Dimensions (TCP)
		Improve Scrolling
		Improve Background filling on Redisplay.
  06/05/89	Implement Pale Footprints (TCP)
  06/06/89	Fix Horizontal Node-connector printing (TCP)
  06/06/89	Set CurrentNode default to Root-node (TCP)
  06/23/89	Support Horizontal Explosion (TCP)
  06/28/89	Fix Folded flag setting (TCP)
		Ensure node-connectors drawn even when nodes out of sight
  06/29/89	Fix Null CurrentNodeShadow within ImplodeNode (TCP)
		Use only 1-pixel highlight border
  07/14/89	Changed treev_Hit() to call the top-level hit-handler, iff
                one exists, even if no node is hit (GW Keim)
  07/19/89	Changed the #define ViewLinked from (self->header.view.parent) to
		 (ScrollView ? ScrollView->header.view.parent : self->header.view.parent).
		 This bug caused treev graphics ops to be done even when the treev was not
		 in the viewTree; (GW Keim)
  07/25/89	Remove arg from im_ForceUpdate (TCP)
  07/26/89	Use graphic_COPY instead of graphic_OR for Pale: X oddities (TCP)
  08/24/89	Pass object-type arg to Hit-handlers (TCP)
   10/24/89	Changed the Mark_Child_Exposure call in Expose_Node_Children to 
	              be recursive. (GW Keim)
	              Changed the definition of the macro ViewLinked to check for an imPtr.

END-SPECIFICATION  ************************************************************/



#include  <andrewos.h>
ATK_IMPL("treev.H")
#include  <graphic.H>
#include  <observable.H>
#include  <view.H>
#include  <fontdesc.H>
#include  <im.H>
#include  <cursor.H>
#include  <rect.h>
#include  <scroll.H>
#include  <apts.H>
#include  <aptv.H>
#include  <tree.H>
#include  <treev.H>
#include <atom.H>

static int treev_debug = 0;

struct treev_instance
  {
  class tree			 *data_object, *shadow_tree_object;
  treev_hitfptr hit_handler;
  class view			 *anchor;
  class scroll			 *scroll_view;
  class treev			 *scrolled_treev;
  struct node_shadow		 *root_node_shadow, *current_node_shadow, *prior_node_shadow;
  struct tree_node		 *current_node, *prior_node;
  struct rectangle		  bounds;
  treev_updatefptr pending_update;
  short				  cell_height, cell_width,
				  node_width, node_height,
				  column_width, row_height,
				  max_name_width, max_name_height,
				  desired_width, desired_height,
  				  greatest_row, greatest_column;
  long				  max_peer_count, max_level_count,
				  vertical_offset, horizontal_offset,
				  pending_vertical_offset,
				  pending_horizontal_offset,
				  m_width, half_m_width, m_height;
  unsigned char			  scroll, node_border_style, node_highlight_style,
				  node_connector_style, fold, node_filigree,
				  arrangement, background_shade, node_footprint_style,
				  node_order;
  class graphic		 *background_pattern;
  boolean			  graphics_initialized;
  char				 *node_font_name, *title_font_name, *tree_cursor_font_name;
  class fontdesc		 *node_font, *title_font, *tree_cursor_font;
  unsigned char			  tree_cursor_byte;
  class cursor			 *tree_cursor;
  class graphic		 *black_tile, *dotted_tile;
  };

#define  Tree			      (self->instance->data_object)
#define  ROOTNODE		      ((Tree)->RootNode())
#define  NEXTNODE(node)		      ((Tree)->NextNode(node))
#define  CURRENTNODE		      (self->instance->current_node)
#define  PriorNode		      (self->instance->prior_node)
#define  TitleFontName		      (self->instance->title_font_name)
#define  TitleFont		      (self->instance->title_font)
#define  NodeFontName		      (self->instance->node_font_name)
#define  NodeFont		      (self->instance->node_font)
#define  MaxNameWidth		      (self->instance->max_name_width)
#define  MaxNameHeight		      (self->instance->max_name_height)
#define  MaxPeerCount		      (self->instance->max_peer_count)
#define  MaxLevelCount		      (self->instance->max_level_count)
#define  DesiredWidth		      (self->instance->desired_width)
#define  DesiredHeight		      (self->instance->desired_height)
#define  GraphicsInitialized	      (self->instance->graphics_initialized)




#define  ShadowTree		      ((self)->instance->shadow_tree_object)
#define  ShadowRootNode		      ((ShadowTree)->RootNode())
#define  NextShadowNode(node)	      ((ShadowTree)->NextNode(node))
#define  ShadowNodeDatum(node)	      ((node_shadow_type)node->datum)

#define  CurrentNodeShadow	      ((self)->instance->current_node_shadow)
#define  PriorNodeShadow	      ((self)->instance->prior_node_shadow)
#define  ShadowedNode(shadow)	      ((shadow)->node)
#define  ShadowExposed(shadow)	      ((shadow)->exposed)
#define  ShadowHidden(shadow)	      ((shadow)->hidden)
#define  ShadowExploded(shadow)	      ((shadow)->exploded)
#define  ShadowFolded(shadow)	      ((shadow)->folded)
#define  ShadowChildrenExposed(shadow) ((shadow)->children_exposed)
#define  ShadowChildrenFolded(shadow)  ((shadow)->children_folded)
#define  ShadowVisible(shadow)	      ((shadow)->visible)
#define  ShadowVisage(shadow)	      ((shadow)->visage)
#define  Plain			      (0)
#define  ShadowPlain(shadow)	      (ShadowVisage(shadow) == Plain)
#define  Highlighted		      (1)
#define  ShadowHighlighted(shadow)    (ShadowVisage(shadow) == Highlighted)
#define  Footprinted		      (2)
#define  ShadowFootprinted(shadow)    (ShadowVisage(shadow) == Footprinted)
#define  ShadowSubWidth(shadow)	      ((shadow)->sub_width)
#define  ShadowSubHeight(shadow)      ((shadow)->sub_height)
#define  ShadowBounds(shadow)	      (&(shadow)->bounds)
#define  ShadowLeft(shadow)	      ((shadow)->bounds.left)
#define  SL			      (ShadowLeft(shadow))
#define  ShadowTop(shadow)	      ((shadow)->bounds.top)
#define  ST			      (ShadowTop(shadow))
#define  ShadowWidth(shadow)	      ((shadow)->bounds.width)
#define  SW			      (ShadowWidth(shadow))
#define  ShadowHeight(shadow)	      ((shadow)->bounds.height)
#define  SH			      (ShadowHeight(shadow))
#define  ShadowRight(shadow)	      ((shadow)->bounds.left+(shadow)->bounds.width)
#define  SR			      (ShadowRight(shadow))
#define  ShadowCenter(shadow)	      ((shadow)->bounds.left+(shadow)->bounds.width/2-DropShadow*2)
#define  SC			      (ShadowCenter(shadow))
#define  ShadowBottom(shadow)	      ((shadow)->bounds.top+(shadow)->bounds.height)
#define  SB			      (ShadowBottom(shadow))
#define  ShadowMiddle(shadow)	      ((shadow)->bounds.top+(shadow)->bounds.height/2)
#define  SM			      (ShadowMiddle(shadow))
#define  ShadowChildrenRowCount(shadow) ((shadow)->sub_row_count)
#define  ShadowChildrenColumnCount(shadow) ((shadow)->sub_column_count)

#define  PARENTNODE(node)	      ((node)->parent)
#define  ChildNode(node)	      ((node)->child)
#define  LeftNode(node)		      ((node)->left)
#define  RightNode(node)	      ((node)->right)
#define  NodeName(node)	    	      ((node)->name)
#define  NodeCaption(node)	      ((node)->caption)
#define  NodeCaptionName(node)	      (((node)->caption) ? (node)->caption : NodeName(node) )
#define  NodeTitle(node)	      ((node)->title)
#define  NodeDatum(node)	      ((node)->datum)
#define  NodeMode(node)	    	      ((node)->mode)
#define  NODELEVEL(node)	      ((Tree)->NodeLevel(node))
#define  NodeModified(node)	      ((node)->modified)

#define  NodeShadow(node)	      (Node_Shadow(self,node))
#define  NodeShadowNode(node)	      (Node_Shadow_Node(self,node))
#define  NodeHitHandler(node)	      (NodeShadow(node)->hit_handler)
#define  NodeViewObject(node)	      (NodeShadow(node)->view_object)
#define  NodeDataObject(node)	      (NodeShadow(node)->data_object)
#define  NODEEXPOSED(node)	      (NodeShadow(node)->exposed)
#define  NodePlain(node)	      (NodeShadow(node)->visage==Plain)
#define  NODEHIGHLIGHTED(node)	      (NodeShadow(node)->visage==Highlighted)
#define  NodeFootPrinted(node)	      (NodeShadow(node)->visage==FootPrinted)
#define  NODECHILDRENEXPOSED(node)    (NodeShadow(node)->children_exposed)
#define  NODEEXPLODED(node)	      (NodeShadow(node)->exploded)
#define  NodeVisible(node)	      (NodeShadow(node)->visible)

#define  Anchor			      (self->instance->anchor)
#define  HitHandler		      (self->instance->hit_handler)
#define  Bounds			     (&self->instance->bounds)
#define  Left			      (self->instance->bounds.left)
#define  Top			      (self->instance->bounds.top)
#define  Width			      (self->instance->bounds.width)
#define  Right			      (Left+Width)
#define  Center			      (Left+Width/2)
#define  Middle			      (Top+Height/2)
#define  Height			      (self->instance->bounds.height)
#define  Bottom			      (Top+Height)

#define  CellWidth		      (self->instance->cell_width)
#define  CellHeight		      (self->instance->cell_height)
#define  ColumnWidth		      (self->instance->column_width)
#define  RowHeight		      (self->instance->row_height)
#define  MWidth			      (self->instance->m_width)
#define  MHeight		      (self->instance->m_height)
#define  HalfMWidth		      (self->instance->half_m_width)
#define  GreatestRow		      (self->instance->greatest_row)
#define  GreatestColumn		      (self->instance->greatest_column)
#define  VerticalOffset		      (self->instance->vertical_offset)
#define  HorizontalOffset	      (self->instance->horizontal_offset)
#define  PendingVerticalOffset	      (self->instance->pending_vertical_offset)
#define  PendingHorizontalOffset      (self->instance->pending_horizontal_offset)

#define  NodeWidth	    	      (self->instance->node_width)
#define  NodeHeight	    	      (self->instance->node_height)
#define  NodeBorderStyle	      (self->instance->node_border_style)
#define  NodeHighlightStyle	      (self->instance->node_highlight_style)
#define  NodeFootprintStyle	      (self->instance->node_footprint_style)
#define  NodeConnectorStyle	      (self->instance->node_connector_style)
#define  Arrangement		      (self->instance->arrangement)
#define  HorizontalArrangement	      (self->instance->arrangement == treev_Horizontal)
#define  VerticalArrangement	      (self->instance->arrangement == treev_Vertical)
#define  NodeOrder		      (self->instance->node_order)
#define  NodeOrderRowMajor	      (self->instance->node_order == treev_RowMajor)
#define  NodeOrderColumnMajor	      (self->instance->node_order == treev_ColumnMajor)
#define  NodeFiligree		      (self->instance->node_filigree)
#define  DropShadow		      (NodeFiligree & treev_DropShadow)

#define  ScrollView		      (self->instance->scroll_view)
#define  ScrolledView		      (self->instance->scrolled_treev)
#define  ViewLinked		      ((self)->IsAncestor((self)->GetIM()))
#define  PendingUpdate		      (self->instance->pending_update)

#define  TreeCursor		      (self->instance->tree_cursor)
#define  TreeCursorByte		      (self->instance->tree_cursor_byte)
#define  TreeCursorFont		      (self->instance->tree_cursor_font)
#define  TreeCursorFontName	      (self->instance->tree_cursor_font_name)

#define  Scroll			      (self->instance->scroll)
#define  Fold			      (self->instance->fold)
#define  BackgroundShade	      (self->instance->background_shade)
#define  BackgroundPattern	      (self->instance->background_pattern)
#define  BackgroundWhite	      (self->instance->background_shade == 1)
#define  BackgroundNonWhite	      (self->instance->background_shade != 1)

#define  BlackTile		      (self->instance->black_tile)
#define  DottedTile		      (self->instance->dotted_tile)

// Why does this forget PendingUpdate?  Weird.
#define  UpdateScrollbars	      {/* PendingUpdate=NULL; */\
				       if(ViewLinked) (self)->WantUpdate(self);}

typedef  struct node_shadow	     *node_shadow_type;
struct  node_shadow
  {
  struct tree_node		     *node;
  struct rectangle		      bounds;
  treev_hitfptr hit_handler;
  class view			     *view_object;
  class observable		     *data_object;
  class view			   *(*observer)();
  short				      sub_width, sub_height;
  unsigned char			      sub_row_count, sub_column_count, visage;
  unsigned			      visible	:1,
				      exposed	:1,
				      hidden	:1,
				      children_exposed	:1,
				      folded	:1,
				      children_folded	:1,
				      exploded	:1;
  };

#define  Balanced		    (view_BETWEENTOPANDBOTTOM | view_BETWEENLEFTANDRIGHT)
#define  Halo			    true
#define  NoHalo			    false

#define  max(a,b)		    (((a)>(b)) ? (a):(b))
#define  min(a,b)		    (((a)<(b)) ? (a):(b))
#define  abs(x)			    (((x)>0) ? (x):-(x))


















































static const char *treev_fg=NULL, *treev_bg=NULL;
static long treev_fgr, treev_fgg, treev_fgb;
static long treev_bgr, treev_bgg, treev_bgb;


ATKdefineRegistry(treev, aptv, NULL);
static void treev_FlipColors(class treev  *self);
static void treev_RestoreColors(class treev  *self);
static void SetTreeAttribute( class treev	      *self, long		       attribute , long		       value );
static long Name_Sizing( class treev	      *self, class tree	      *tree, tree_type_node      node, char * datum );
static void Initialize_Graphics( class treev	      *self );
static void First_Time( class treev	      *self );
static void Redisplay( class treev	      *self );
static void Set_Dimensions( class treev	      *self );
static void Check_Dimensions( class treev	      *self, tree_type_node      node );
static void Erase_Node_Children( class treev	      *self, struct tree_node   *shadow_node );
static void Redisplay_Node_Children( class treev	      *self, struct tree_node   *shadow_node );
static struct tree_node * Which_Node_Hit( class treev	       *self, long		        x , long		        y );
static void Fill_Area( class treev	      *self, long		       mode , class graphic     *tile, long		       shape, long		       left , long		       top , long		       width , long		       height );
static void Fill_Shadow( class treev	       *self, struct node_shadow  *shadow, long		        mode, class graphic      *tile );
static void Clear_Shadow( class treev		 *self, struct node_shadow	 *shadow );
static void Normalize_Other_Nodes( class treev	         *self, struct node_shadow    *node_shadow );
static void Normalize_Node_Shadow( class treev	       *self, struct node_shadow  *shadow );
static void Highlight_Node_Shadow( class treev	       *self, struct node_shadow  *shadow );
static void Footprint_Node_Shadow( class treev		  *self, struct node_shadow	  *shadow );
static void Hide_Node_Children( class treev	      *self, struct tree_node   *shadow_node );
static void Expose_Node_Children( class treev	      *self, struct tree_node   *shadow_node );
static void Printer( class treev	      *self );
static void Print_Tree( class treev	      *self, struct tree_node   *shadow_node );
static void Print_Node_Border( class treev	       *self, struct node_shadow  *shadow );
static void Print_Node_Connector( class treev	       *self, struct node_shadow  *shadow , struct node_shadow  *parent_shadow );
static void y_getinfo( class treev	      *self, struct range	      *total , struct range	      *seen , struct range	      *dot );
static long y_whatisat( class treev	      *self, long		       pos , long		       outof );
static void y_setframe( class treev	      *self, int		       place, long		       pos , long		       outof );
static void y_endzone( class treev	      *self, int		       zone , int		       action );
static void x_getinfo( class treev	      *self, struct range	      *total , struct range	      *seen , struct range	      *dot );
static long x_whatisat( class treev	      *self, long		       pos , long		       outof );
static void x_setframe( class treev	      *self, long		       place , long		       pos , long		       outof );
static void x_endzone( class treev	      *self, long		       zone , long		       action );
static void Arrange_Tree( class treev	      *self, struct tree_node   *shadow_node, long		       left , long		       top , long		       width , long		       height );
static void Arrange_Horizontal_Tree( class treev	      *self, struct tree_node   *shadow_node, long		       left , long		       top , long		       width , long		       height );
static void Arrange_Vertical_Tree( class treev	      *self, struct tree_node   *shadow_node, long		       left , long		       top , long		       width , long		       height );
static void Arrange_Exploded_Tree( class treev	      *self, struct tree_node   *shadow_node, long		       left , long		       top , long		       width , long		       height );
static long Compute_Horizontal_Exploded_SubTree_Heights( class treev	      *self, struct tree_node   *shadow_node );
static void Arrange_Horizontal_Exploded_Tree( class treev	      *self, struct tree_node   *shadow_node, long		       left , long		       top , long		       width , long		       height );
static void Arrange_Horizontal_Exploded_SubTree( class treev	      *self, struct tree_node   *shadow_node, long		       left , long		       top , long		       width , long		       height );
static long Compute_Vertical_Exploded_SubTree_Widths( class treev	      *self, struct tree_node   *shadow_node );
static void Arrange_Vertical_Exploded_Tree( class treev	      *self, struct tree_node   *shadow_node, long		       left , long		       top , long		       width , long		       height );
static void Arrange_Vertical_Exploded_SubTree( class treev	      *self, struct tree_node   *shadow_node, long		       left , long		       top , long		       width , long		       height );
static void Mark_Child_Exposure( class treev	      *self, struct tree_node   *shadow_node, boolean	       state , boolean	       recursive );
static void Draw_Tree( class treev	      *self, struct tree_node   *shadow_node );
static void Draw_Background( class treev	      *self );
static void Fill_Background( class treev	      *self );
static void Draw_Node_Caption( class treev	       *self, struct node_shadow  *shadow, boolean	        halo );
static void Draw_Node_Title( class treev	       *self, struct node_shadow  *shadow );
static void Draw_Node_Border( class treev	       *self, struct node_shadow  *shadow );
static void Draw_Node_Connector( class treev	       *self, struct node_shadow  *shadow , struct node_shadow  *parent_shadow );
static struct node_shadow * Node_Shadow( class treev	      *self, struct tree_node   *node );
static tree_type_node Node_Shadow_Node( class treev	      *self, struct tree_node   *node );
static struct node_shadow * Create_Shadow( class treev	       *self );
static long Generate_Shadows( class treev	       *self, struct tree_node    *node );
static long Generate_Children_Shadows( class treev	      *self, struct tree_node   *node );
static void Destroy_Shadows( class treev	      *self, struct tree_node   *node );
static void Destroy_Children_Shadows( class treev	      *self, struct tree_node   *node );
static void Dump_Trees( class treev	      *self );

static void treev_FlipColors(class treev  *self)
{
    const class atom *a;
    (self)->GetForegroundColor( &treev_fg, &treev_fgr, &treev_fgg, &treev_fgb);
    (self)->GetBackgroundColor( &treev_bg, &treev_bgr, &treev_bgg, &treev_bgb);
    if(treev_fg) {
	a=atom::Intern(treev_fg);
	if(a) treev_fg=(a)->Name();
	else treev_fg=NULL;
    }
    if(treev_bg) {
	a=atom::Intern(treev_bg);
	if(a) treev_bg=(a)->Name();
	else treev_bg=NULL;
    }
    (self)->SetForegroundColor( treev_bg, treev_bgr, treev_bgg, treev_bgb);
    (self)->SetBackgroundColor( treev_fg, treev_fgr, treev_fgg, treev_fgb);
}

static void treev_RestoreColors(class treev  *self)
{
    (self)->SetForegroundColor( treev_fg, treev_fgr, treev_fgg, treev_fgb);
    (self)->SetBackgroundColor( treev_bg, treev_bgr, treev_bgg, treev_bgb);
}


class treev *
treev::Create( const treev_Specification		  *specification, class view		  *anchor )
        {
  class treev		 *self;
  long			  mode = 0;
  const treev_Specification		 *spec = specification;
  static boolean		  bypass_scroll = false;

  IN(treev_Create);
  self = new treev;
  Anchor = anchor;
  while ( specification  &&  specification->attribute )
    {
    ::SetTreeAttribute( self, specification->attribute, specification->value );
    specification++;
    }
  if ( Scroll  &&  !bypass_scroll )
    { DEBUG(Scroll SetUp);
    if ( Scroll & treev_Left )	 mode |= scroll_LEFT;
    if ( Scroll & treev_Bottom ) mode |= scroll_BOTTOM;
    bypass_scroll = true;
    ScrolledView = treev::Create( spec, anchor );
    bypass_scroll = false;
    ScrollView = scroll::Create( ScrolledView, mode );
    (ScrollView)->SetView(  ScrolledView );
    }
  OUT(treev_Create);
  return  self;
  }

treev::treev( )
{
    class treev *self=this;
  long		       status = true;

  IN(treev_InitializeObject);
  DEBUGst(RCSID,rcsid);
  this->instance = (struct treev_instance *) calloc( 1, sizeof(struct treev_instance ) );
  (this)->SetDimensions(  150, 200 );
  (this)->SetOptions(  aptv_SuppressControl | aptv_SuppressBorder );
  DesiredWidth = DesiredHeight = 200;
  NodeFootprintStyle = treev_Pale;
  NodeOrder = treev_ColumnMajor;
  Arrangement = treev_Vertical;
  Fold = true;
BackgroundShade = 1; /*=== force to white because some clients neglect to clear */
  if ( (ShadowTree = new tree) == NULL )
    {
    printf( "TreeView: ShadowTree not Created\n");
    status = false;
    }
  OUT(treev_InitializeObject);
  THROWONFAILURE(  status);
  }

treev::~treev( )
{
    class treev *self=this;
  IN(treev_FinalizeObject );
  if ( this->instance )
    {
    if ( ScrolledView ) {
      (ShadowTree )->Destroy();
      (ScrollView )->Destroy();
      ScrollView = NULL;
      (ScrolledView )->Destroy();
    }
    else{
      Destroy_Shadows( this, ROOTNODE );
      (ShadowTree )->Destroy();
    }
    if ( NodeFontName )		free( NodeFontName );
    if ( TitleFontName )	free( TitleFontName );
    if ( TreeCursorFontName )	free( TreeCursorFontName );
    free( this->instance );
    }
  OUT(treev_FinalizeObject );
  }

void
treev::SetDataObject( class dataobject	       *data )
{
    class treev *self=this;
    class tree *data_object=(class tree *)data;
  IN(treev_SetDataObject);
  Tree = data_object;
  if ( ScrolledView )
    (ScrolledView)->SetDataObject(  data_object );
    else
    (this)->aptv::SetDataObject(  data_object );
  OUT(treev_SetDataObject);
  }

long
treev::SetTreeAttribute( long		       attribute , long		       value )
{
  long		      status = ok;

  IN(treev_SetTreeAttribute);
  ::SetTreeAttribute( this, attribute, value );
  OUT(treev_SetTreeAttribute);
  return  status;
  }

static
void SetTreeAttribute( class treev	      *self, long		       attribute , long		       value )
      {

  IN(SetTreeAttribute);
  if ( ScrolledView )
    SetTreeAttribute( ScrolledView, attribute, value );
  else
  switch ( attribute )
    {
    case  treev_nodeborderstyle:
      NodeBorderStyle = value; Set_Dimensions( self );  break;
    case  treev_nodehighlightstyle:
      NodeHighlightStyle = value;			break;
    case  treev_nodefootprintstyle:
      NodeFootprintStyle = value;			break;
    case  treev_nodeconnectorstyle:
      if ( value & treev_DogLeg )
        NodeConnectorStyle = treev_DogLeg;
        else  NodeConnectorStyle = treev_Direct;
      if ( value & treev_Fold )    Fold = true;
      if ( value & treev_NoFold )  Fold = false;
      break;
    case  treev_nodefiligree:
      NodeFiligree = value;				break;
    case  treev_scroll:
      Scroll = value;					break;
    case  treev_nodefontname:
      apts::CaptureString( (char *)value, &NodeFontName );	break;
    case  treev_hithandler:
      HitHandler = (treev_hitfptr) value;		break;
    case  treev_arrangement:
      Arrangement = value; Set_Dimensions( self );	break;
    case  treev_nodeorder:
      NodeOrder = value;				break;
    case  treev_backgroundshade:
      BackgroundShade = value;				break;
    case  treev_cursor:
      TreeCursorByte = value;
      if ( GraphicsInitialized  &&  TreeCursor )
	{
	if ( TreeCursorFont )
	  (TreeCursor)->SetGlyph(  TreeCursorFont, TreeCursorByte );
	  else
	  (TreeCursor)->SetStandard(  TreeCursorByte );
	}
      break;
    case  treev_cursorfontname:
      apts::CaptureString((char *) value, &TreeCursorFontName ); break;
    default:/*===*/
	printf( "treev: Unrecognized Attribute (%ld) (Ignored)\n", attribute );
    }
  OUT(SetTreeAttribute);
  }

long
treev::TreeAttribute( long		       attribute )
{
    class treev *self=this;
  long		      value = 0;

  IN(treev_TreeAttribute);
  if ( ScrolledView )
    value = (ScrolledView)->TreeAttribute(  attribute );
  else
  switch ( attribute )
    {
    case  treev_nodeborderstyle:
      value = (long) NodeBorderStyle;      break;
    case  treev_nodehighlightstyle:
      value = (long) NodeHighlightStyle;    break;
    case  treev_nodeconnectorstyle:
      value = (long) NodeConnectorStyle;    break;
    case  treev_nodefiligree:
      value = (long) NodeFiligree;	    break;
    case  treev_scroll:
      value = (long) Scroll;		    break;
    case  treev_nodefontname:
      value = (long) NodeFontName;	    break;
    case  treev_hithandler:
      value = (long) HitHandler;	    break;
    case  treev_arrangement:
      value = (long) Arrangement;	    break;
    case  treev_nodeorder:
      value = NodeOrder;		    break;
    case  treev_backgroundshade:
      value = (long) BackgroundShade;	    break;
    case  treev_cursor:
      value = (long) TreeCursorByte;	    break;
    case  treev_cursorfontname:
      value = (long) TreeCursorFontName;    break;
    default:/*===*/
      printf( "TreeView: Unrecognized Attribute (%ld)\n", attribute );
    }
  OUT(treev_TreeAttribute);
  return  value;
  }

void
treev::SetDebug( boolean		        state )
{
    class treev *self=this;
  IN(treev_SetDebug);
  treev_debug = state;
  if ( ScrolledView )
    (ScrolledView)->SetDebug(  state );
  OUT(treev_SetDebug);
  }

tree_type_node
treev::CurrentNode( )
{
    class treev *self=this;
  tree_type_node      node;

  if ( (node = (this->instance->scrolled_treev) ?
	    this->instance->scrolled_treev->instance->current_node :
	    this->instance->current_node) == NULL )
    node = ROOTNODE;
  return  node;
  }

static
long Name_Sizing( class treev	      *self, class tree	      *tree, tree_type_node      node, char * datum )
        {
  long			      width, height;
  long		      peers, level;

  IN(Name_Sizing);
  (NodeFont)->StringSize( 
	(self )->GetDrawable( ), NodeCaptionName(node), &width, &height );
  if ( width > MaxNameWidth )
    MaxNameWidth = width;
  if ( (peers = (Tree)->PeerNodeCount(  node )) > MaxPeerCount )
    MaxPeerCount = peers + 1;
  if ( (level = NODELEVEL(  node )) > MaxLevelCount )
    MaxLevelCount = level;
  OUT(Name_Sizing);
  return 0;
  }

static
void Initialize_Graphics( class treev	      *self )
    {
  char			      font_family[256];
  long			      font_type, font_size;

  IN(Initialize_Graphics);
  if ( !GraphicsInitialized  &&  Tree )
    {
    GraphicsInitialized = true;
    BlackTile = (self )->BlackPattern( );
    DottedTile = (self)->GrayPattern(  50, 100 );
    if ( TreeCursorByte )
      {
      TreeCursor = cursor::Create( self );
      if ( TreeCursorFontName )
	{
	fontdesc::ExplodeFontName( TreeCursorFontName,
				font_family, sizeof(font_family), &font_type, &font_size );
	TreeCursorFont = fontdesc::Create( font_family, font_type, font_size );
	(TreeCursor)->SetGlyph(  TreeCursorFont, TreeCursorByte );
	}
	else (TreeCursor)->SetStandard(  TreeCursorByte );
      }
    fontdesc::ExplodeFontName( (NodeFontName) ? NodeFontName : "andysans10b",
				font_family, sizeof(font_family), &font_type, &font_size );
    NodeFont = fontdesc::Create( font_family, font_type, font_size );
    (Tree)->Apply(  ROOTNODE, (tree_applyfptr)Name_Sizing, (char *)self, NULL );
    MaxNameHeight =
	(NodeFont)->FontSummary(  (self )->GetDrawable( ) )->maxHeight;
    (NodeFont)->StringSize(  (self )->GetDrawable( ), "M", &MWidth, &MHeight );
    MWidth = MWidth - (MWidth % 2);
    MWidth -= MWidth % 2;
    HalfMWidth = MWidth/2;
    Set_Dimensions( self );
    if ( (MaxPeerCount * ColumnWidth) > DesiredWidth )
      DesiredWidth = MaxPeerCount * ColumnWidth;
    DEBUGdt(MaxPeerCount,MaxPeerCount);  DEBUGdt(DesiredWidth,DesiredWidth);
    if ( (MaxLevelCount * RowHeight) > DesiredHeight )
      DesiredHeight = MaxLevelCount * RowHeight;
    DEBUGdt(MaxLevelCount,MaxLevelCount);  DEBUGdt(DesiredHeight,DesiredHeight);
    }
  OUT(Initialize_Graphics);
  }

static
void First_Time( class treev	      *self )
    {
  IN(First_Time);
  if ( ROOTNODE )
    {
    Initialize_Graphics( self );
    Generate_Shadows( self, ROOTNODE );
    ShadowExposed(ShadowNodeDatum(ShadowRootNode)) = true;
    ShadowVisage(ShadowNodeDatum(ShadowRootNode)) = Highlighted;
    CURRENTNODE = ROOTNODE;
    CurrentNodeShadow = ShadowNodeDatum(ShadowRootNode);
    (self)->ExposeNode(  ROOTNODE );
    }
  OUT(First_Time);
  }

static void
Redisplay( class treev	      *self )
    {
  PendingUpdate = NULL;
  VerticalOffset = PendingVerticalOffset;
  HorizontalOffset = PendingHorizontalOffset;
  (self)->SetClippingRect(  Bounds );
  GreatestRow = GreatestColumn = 0;
  Arrange_Tree( self, ShadowRootNode, Left, Top, Width, Height );
  Fill_Background( self );
  Draw_Tree( self, ShadowRootNode );
  }

static
void Set_Dimensions( class treev	      *self )
    {
  IN(Set_Dimensions);
  PendingHorizontalOffset = 0;
  PendingVerticalOffset = 0;		
  if ( NodeWidth )
    CellWidth = NodeWidth;
    else
    CellWidth = MaxNameWidth + MWidth;
  CellWidth -= CellWidth % 2;
  ColumnWidth = CellWidth + HalfMWidth;
  if ( ! VerticalArrangement )
    ColumnWidth += HalfMWidth;
  if ( NodeHeight )
    CellHeight = NodeHeight;
    else
    CellHeight = MaxNameHeight + MWidth;
  CellHeight -= CellHeight % 2;
  if ( NodeBorderStyle == treev_Circle )
      CellHeight = CellWidth;
  RowHeight = CellHeight + MWidth;
  if ( ! VerticalArrangement )
    RowHeight -= HalfMWidth;
  OUT(Set_Dimensions);
  }

static
void Check_Dimensions( class treev	      *self, tree_type_node      node )
      {
  long			      width, height;
  struct tree_node  *peer;

  IN(Check_Dimensions);
  (NodeFont)->StringSize( 
	(self )->GetDrawable( ), NodeCaptionName(node), &width, &height );
  if ( width > MaxNameWidth )
    {
    MaxNameWidth = width;
    Set_Dimensions( self );
    PendingUpdate = (treev_updatefptr)Redisplay;
    (self)->WantUpdate(  self );
    }
  if ( ( peer = ChildNode(node) ) )
    {
    while( peer )
      {
      Check_Dimensions( self, peer );
      peer = RightNode(peer);
      }
    }
  OUT(Check_Dimensions);
  }

view_DSattributes
treev::DesiredSize( long			    given_width , long			    given_height,
		      enum view_DSpass	    pass, long			   *desired_width , long			   *desired_height )
{
    class treev *self=this;
  view_DSattributes  result = view_WidthFlexible |
					       view_HeightFlexible;

  IN(treev_DesiredSize);
  if ( ScrolledView )
    result = (ScrolledView)->DesiredSize(  given_width, given_height,
		      pass, desired_width, desired_height );
    else
    {
    DEBUGdt(Given Width,given_width);  DEBUGdt(Given Height,given_height);
    DEBUGdt(Pass, pass);
    *desired_width  = ((DesiredWidth > given_width) ? DesiredWidth : given_width);
    *desired_height = DesiredHeight;
    DEBUGdt(Desired Width,*desired_width);  DEBUGdt(Desired Height,*desired_height);
    DEBUGxt(Result-code,result);
    }
  OUT(treev_DesiredSize);
  return result;
  }

void 
treev::FullUpdate( enum view_UpdateType    type, long			   left , long			   top , long			   width , long			   height )
{
    class treev *self=this;
  IN(treev_FullUpdate);
  if ( Tree  &&  (type == view_FullRedraw || type == view_LastPartialRedraw) )
    {
    if ( ScrollView )
      { DEBUG(Use Scrolled View);
      (ScrollView)->InsertViewSize(  this, left, top, width, height );
      (ScrollView)->FullUpdate(  type, left, top, width, height );
      DEBUG(Back from Pseudo Update);
      }
    else
    { DEBUG(Use Real View);
    PendingUpdate = NULL;
    VerticalOffset = PendingVerticalOffset;
    HorizontalOffset = PendingHorizontalOffset;
    (this)->aptv::FullUpdate(  type, left, top, width, height );
    if ( ! (this)->BypassUpdate() )
      { DEBUG(Not Bypassed);
      GreatestRow = GreatestColumn = 0;
      (this)->GetLogicalBounds( Bounds);
      (this)->SetClippingRect(  Bounds );
      BackgroundPattern = (this )->WhitePattern( );
      Fill_Background( this );
      if ( ShadowRootNode  &&  ShadowExposed(ShadowNodeDatum(ShadowRootNode)) )
        {
        Arrange_Tree( this, ShadowRootNode, Left, Top, Width, Height );
        Draw_Tree( this, ShadowRootNode );
        }
        else
        { DEBUG(First Time);
	First_Time( this );
	}
      if ( TreeCursor )
        (this)->PostCursor(  Bounds, TreeCursor );
      }
    }
    }
  OUT(treev_FullUpdate);
  }

static
void Erase_Node_Children( class treev	      *self, struct tree_node   *shadow_node )
      {
  struct tree_node  *shadow_peer;

  IN(Erase_Node_Children);
  shadow_peer = ChildNode(shadow_node);
  while ( shadow_peer )
    {
    if ( ShadowChildrenExposed(ShadowNodeDatum(shadow_peer)) )
      Erase_Node_Children( self, shadow_peer );
    shadow_peer = RightNode(shadow_peer);
    }
  GreatestRow -= ShadowChildrenRowCount(ShadowNodeDatum(shadow_node));
  if ( GreatestRow <= 0 )    GreatestRow = 1;
  GreatestColumn -= ShadowChildrenColumnCount(ShadowNodeDatum(shadow_node));
  if ( GreatestColumn <= 0 )  GreatestColumn = 1;
  OUT(Erase_Node_Children);
  }

static
void Redisplay_Node_Children( class treev	      *self, struct tree_node   *shadow_node )
      {
  IN(Redisplay_Node_Children);
    if(shadow_node) {
      Draw_Background( self );
      if( ChildNode(shadow_node) ) {
          VerticalOffset = PendingVerticalOffset;
          HorizontalOffset = PendingHorizontalOffset;
	  if( VerticalArrangement )
	      Arrange_Vertical_Tree( self, ChildNode(shadow_node),
				    Left, Top + GreatestRow * RowHeight, Width, Height );
	  else
	      Arrange_Horizontal_Tree( self, ChildNode(shadow_node),
				      Left + GreatestColumn * ColumnWidth, Top, Width, Height );
	  Draw_Tree( self, ChildNode(shadow_node) );
      }
  }
  else {
      CURRENTNODE = NULL;
      GreatestColumn = GreatestRow = 0;
      Draw_Background(self);
  }
  OUT(Redisplay_Node_Children);
  }

void
treev::ObservedChanged( class observable  *changed, long		       change )
{
    class treev *self=this;
  tree_type_node     parent, tmp_parent, node;
  node_shadow_type   shadow;

  IN(treev_ObservedChanged);
  if ( ScrolledView )
    (ScrolledView)->ObservedChanged(  changed, change );
  else
  {
  DEBUGdt(Notification Code,(Tree)->NotificationCode());
  if ( ( node = (Tree)->NotificationNode() ) )
    {
    boolean inExplosion = FALSE;

    DEBUGst(Notification Node,NodeName(node));
    tmp_parent = parent = PARENTNODE(node);
    while(tmp_parent)
	if(NodeExploded(tmp_parent)) {
	    inExplosion = TRUE;
	    break;
	}
	else tmp_parent = PARENTNODE(tmp_parent);
    switch ( (Tree )->NotificationCode( ) )
      {
      case  tree_NodeCreated:	    DEBUG(NodeCreated);
      case  tree_NodeHooked:	    DEBUG(NodeHooked);
	Generate_Shadows( this, node );
	if ( parent ) {
	    if(inExplosion) {
		ShadowExposed(NodeShadow(node)) = true;
		(this)->FullUpdate(view_FullRedraw,0,0,Width,Height);
	    }
	    else {
		Check_Dimensions( this, node );
		(this)->ExposeNodeChildren(  parent );
	    }
	  }
	  else
	  {
	  Initialize_Graphics( this );
          CURRENTNODE = node;
          CurrentNodeShadow = NodeShadow(node);
          ShadowExposed(CurrentNodeShadow) = true;
	  Check_Dimensions( this, node );
	  (this)->ExposeNode(  node );
	  (this)->HighlightNode(  node );
	  (this)->ExposeNodeChildren(  node );
	  }
        break;
      case  tree_NodeDestroyed:	    DEBUG(NodeDestroyed);
      case  tree_NodeUnhooked:	    DEBUG(NodeUnhooked);
	  if(parent) {
	      if((this)->NodeHighlighted(node)) {
	        (this)->NormalizeNode(node);
		if((CURRENTNODE = parent)) {
   	          CurrentNodeShadow = NodeShadow(CURRENTNODE);
		  (this)->HighlightNode(parent);
		}
	      }
	      if(!inExplosion)
		  Erase_Node_Children(this,NodeShadowNode(parent));
	  }
	  if(inExplosion)
	      (this)->FullUpdate(view_FullRedraw,0,0,Width,Height);
	  else {
	      Redisplay_Node_Children(this,parent ? NodeShadowNode(parent) : NULL);
	  }
	  if(parent && !LeftNode(node) && !RightNode(node)) {
	    ShadowChildrenExposed(ShadowNodeDatum(NodeShadowNode(parent))) = false;
	    ShadowChildrenRowCount(ShadowNodeDatum(NodeShadowNode(parent))) = 0;
	  }
	  Destroy_Shadows(this,node);
	  if(parent) CurrentNodeShadow=NodeShadow(parent);
	  else CurrentNodeShadow=NULL;
	  break;
      case  tree_NodeChildrenCreated:    DEBUG(NodeChildrenCreated);
	Check_Dimensions( this, node );
	Generate_Children_Shadows( this, node );
	(this)->ExposeNodeChildren(  node );
	break;
      case  tree_NodeChildrenDestroyed:	DEBUG(NodeChildrenDestroyed);
	(this)->HideNodeChildren(  node );
	Destroy_Children_Shadows( this, node );
	break;
      case  tree_NodeNameChanged:	DEBUG(NodeNameChanged);
      case  tree_NodeCaptionChanged:	DEBUG(NodeCaptionChanged);
	Check_Dimensions( this, node );
	shadow = NodeShadow(node);
	Clear_Shadow( this, shadow );
	(this)->SetTransferMode(  graphic_COPY );
	Draw_Node_Caption( this, shadow, NoHalo );
	if ( ShadowHighlighted(shadow) )
	  Highlight_Node_Shadow( this, shadow );
	else
	if ( ShadowFootprinted(shadow) )
          Footprint_Node_Shadow( this, shadow );
	break;
      case  tree_NodeTitleChanged:	DEBUG(NodeTitleChanged);
	Check_Dimensions( this, node );
    	shadow = NodeShadow(node);
	Clear_Shadow( this, shadow );
	(this)->SetTransferMode(  graphic_COPY );
	Draw_Node_Title( this, shadow );
	if ( ShadowHighlighted(shadow) )
	  Highlight_Node_Shadow( this, shadow );
	else
	if ( ShadowFootprinted(shadow) )
          Footprint_Node_Shadow( this, shadow );
	break;
      default:
/*===*/ printf( "TreeView: Unknown Notification-type (%d)\n",
		    (Tree )->NotificationCode( ));
      }
    UpdateScrollbars;
    }
    else
/*===*/ printf( "TreeView: Null Notification-node\n" );
  }
  OUT(treev_ObservedChanged);
  }

void 
treev::Update( )
{
    class treev *self=this;
  IN(treev_Update);
  if ( ViewLinked && PendingUpdate )
    {
    /* probably don't want a loop on this.. */
    treev_updatefptr pu = PendingUpdate;
    PendingUpdate = NULL;
    pu( this );
    }
  OUT(treev_Update);
  }

void
treev::SetHitHandler( treev_hitfptr handler, class view	    *anchor )
{
    class treev *self=this;
  IN(treev_SetHitHandler);
  HitHandler = handler;
  Anchor = anchor;
  OUT(treev_SetHitHandler);
  }

static struct tree_node *
Which_Node_Hit( class treev	       *self, long		        x , long		        y )
      {
  struct tree_node   *node = NULL, *shadow_node;
  node_shadow_type    shadow;

  IN(Which_Node_Hit);
  shadow_node = ShadowRootNode;
  while ( shadow_node )
    {
    shadow = ShadowNodeDatum(shadow_node);
    if ( ShadowExposed(shadow)  &&
	 x >= SL   &&  y >= ST  &&  x <= SR  &&  y <= SB )
      {
      CURRENTNODE = node = ShadowedNode(CurrentNodeShadow = shadow);
      break;
      }
    shadow_node = NextShadowNode(shadow_node);
    }
  OUT(Which_Node_Hit);
  return  node;
  }

class view *
treev::Hit( enum view_MouseAction    action, long			    x , long			    y , long			    clicks )
{
    class treev *self=this;
  struct tree_node	  *node = NULL;
  class view		  *hit;

  IN(treev_Hit );
  if ( ScrollView )
    hit = (ScrollView)->Hit(  action, x, y, clicks );
  else
  if ( (hit = (this)->aptv::Hit(  action, x, y, clicks )) == NULL )
    {
    hit = (class view *) this;
    if ( ( node = Which_Node_Hit( this, x, y ) ) )
    { DEBUGst(Node Name,NodeName(node));
    if ( NodeHitHandler(node) )
      { DEBUG(Specific Node HitHandler Found);
      (NodeHitHandler(node))
		( Anchor, this, node, treev_NodeObject, action, x, y, clicks );
      }
      else
      { DEBUG(No Specific Node Hit Handler Found);
      if ( NodeViewObject(node) )
	{ DEBUG(Viewer Node Hit Handler Found);
	Normalize_Other_Nodes( this, NodeShadow(node) );
	hit = (NodeViewObject(node))->Hit(  action,
			(NodeViewObject(node))->EnclosedXToLocalX(  x ),
			(NodeViewObject(node))->EnclosedYToLocalY(  y ), clicks );
	}
	else
        if ( HitHandler )
	  { DEBUG(General Node Hit Handler Found);
	  (HitHandler)( Anchor, this, node, treev_NodeObject, action, x, y, clicks );
	  }
	  else
	  { DEBUG(No Node Hit Handler Found);
	  if ( node != PriorNode )
	    {
	    PriorNode = node;
	    (this)->HighlightNode(  node );
	    (this)->ExposeNodeChildren(  node );
	    }
	  }
      }
    }
    else { DEBUG(No Node Hit);
    if ( HitHandler )
      { DEBUG(General Node Hit Handler Found);
      (HitHandler)( Anchor, this, node, 0, action, x, y, clicks );
      }
    }
    }
  OUT(treev_Hit );
  return  hit;
  }

static
void Fill_Area( class treev	      *self, long		       mode , class graphic     *tile, long		       shape, long		       left , long		       top , long		       width , long		       height )
          {
  long		      current_mode;

  IN(Fill_Area);
  if ( (current_mode = (self )->GetTransferMode( )) != mode )
    (self)->SetTransferMode(  mode );
  switch ( shape )
    {
    case  treev_Rectangle:
      (self)->FillRectSize(  left, top, width, height, tile );
      break;
    case  treev_Oval:
    case  treev_Circle:
      (self)->FillOvalSize(  left, top, width, height, tile );
      break;
    case  treev_RoundAngle:
      (self)->FillRRectSize(  left, top, width, height, 10, 10, tile );
      break;
    case  treev_Folder:
      (self)->FillRectSize(  left, top+5, width+1, height-4, tile );
      (self)->FillTrapezoid(  left+4, top, width/3-5,
				       left, top+4, width/3+5, tile );
      break;
    }
  //im::ForceUpdate();  This is evil.  It can cause an asyncronous treev_flipcolors!
  if ( current_mode != mode )
    (self)->SetTransferMode(  current_mode );
  OUT(Fill_Area);
  }

static
void Fill_Shadow( class treev	       *self, struct node_shadow  *shadow, long		        mode, class graphic      *tile )
          {
  long		       offset = DropShadow * 2;
  
  IN(Fill_Shadow);
  Fill_Area( self, mode, tile, NodeBorderStyle,
	SL+2, ST+2, (SW-3) - offset, (SH-3) - offset );
  OUT(Fill_Shadow);
  }

static
void Clear_Shadow( class treev		 *self, struct node_shadow	 *shadow )
      {
  long			 offset = DropShadow * 2;
  
  IN(Clear_Shadow);
  treev_FlipColors(self);
  Fill_Area( self, graphic_COPY, NULL, NodeBorderStyle,
	    SL+1, ST+1, (SW-1) - offset, (SH-1) - offset );
  treev_RestoreColors(self);
  OUT(Clear_Shadow);
  }

void
treev::HighlightNode( struct tree_node    *node )
{
    class treev *self=this;
  struct node_shadow *shadow;

  IN(treev_HighlightNode);
  if ( ScrolledView )
    (ScrolledView)->HighlightNode(  node );
  else
  if ( node  &&  !ShadowHighlighted(shadow = NodeShadow(node)) )
    {
    if ( ShadowFootprinted(shadow) )
      Normalize_Node_Shadow( this, shadow );
    Highlight_Node_Shadow( this, shadow );
    Normalize_Other_Nodes( this, shadow );
    }
  OUT(treev_HighlightNode);
  }

boolean
treev::NodeHighlighted( struct tree_node   *node )
{
    class treev *self=this;
  if ( ScrolledView )
    return  (ScrolledView)->NodeHighlighted(  node );
    else  if ( node )
      return  NODEHIGHLIGHTED(node);
      else  return false;
  }

static
void Normalize_Other_Nodes( class treev	         *self, struct node_shadow    *node_shadow )
      {
  struct tree_node	*shadow_node, *given_node;
  struct node_shadow	*shadow;

  IN(Normalize_Other_Nodes);
  given_node = ShadowedNode(node_shadow);
  shadow_node = ShadowRootNode;
  while ( shadow_node )
    { DEBUGst(Node Name,NodeName(ShadowedNode(ShadowNodeDatum(shadow_node))));
    shadow = ShadowNodeDatum(shadow_node);
    if ( shadow != node_shadow  &&  ! ShadowPlain(shadow)  &&
	  ! (Tree)->NodeAncestor(  ShadowedNode(shadow), given_node ) )
      Normalize_Node_Shadow( self, shadow );
    shadow_node = NextShadowNode(shadow_node);
    }
  shadow_node = NodeShadowNode(ShadowedNode(node_shadow));
  while ( shadow_node )
    { DEBUGst(Node Name,NodeName(ShadowedNode(ShadowNodeDatum(shadow_node))));
    if ( PARENTNODE(shadow_node) )
      { DEBUGst(Parent Node Name,
	NodeName(PARENTNODE(ShadowedNode(ShadowNodeDatum(shadow_node)))));
      shadow = ShadowNodeDatum(PARENTNODE(shadow_node));
      if ( ! ShadowFootprinted(shadow) )
	{
	if ( ! ShadowPlain(shadow) )
          Normalize_Node_Shadow( self, shadow );
        Footprint_Node_Shadow( self, shadow );
	}
      }
    shadow_node = PARENTNODE(shadow_node);
    }
  OUT(Normalize_Other_Nodes);
  }

void
treev::NormalizeNode( struct tree_node	  *node )
{
    class treev *self=this;
  struct node_shadow	 *shadow;

  IN(treev_NormalizeNode);
  if ( ScrolledView )
    (ScrolledView)->NormalizeNode(  node );
  else
  if ( node  &&  ! ShadowPlain(shadow = NodeShadow(node)) )
    Normalize_Node_Shadow( this, shadow );
  OUT(treev_NormalizeNode);
  }

static
void Normalize_Node_Shadow( class treev	       *self, struct node_shadow  *shadow )
      {
  char       style = '\0';

  IN(Normalize_Node_Shadow);
  if ( ShadowExposed(shadow) )
    {
    if ( ShadowHighlighted(shadow) )
      style = NodeHighlightStyle;
    if ( ShadowFootprinted(shadow) )
      style = NodeFootprintStyle;
    DEBUGdt(Style,style);
    switch ( style )
      {
      case  treev_Invert:
        Fill_Shadow( self, shadow, graphic_INVERT, NULL );
	break;
      case  treev_Pale:
	Clear_Shadow( self, shadow );
	Draw_Node_Title( self, shadow );
	Draw_Node_Caption( self, shadow, NoHalo );
	break;
      case  treev_Border:

	break;
      case  treev_Bold:

	break;
      case  treev_Italic:

	break;
      default: ; /* (Neither Highlighted nor Footprinted) */
      }
    }
  ShadowVisage(shadow) = Plain;
  OUT(Normalize_Node_Shadow);
  }

static
void Highlight_Node_Shadow( class treev	       *self, struct node_shadow  *shadow )
      {
  IN(Highlight_Node_Shadow);
  if ( ShadowExposed(shadow) )
    { DEBUGdt(NodeHighlightStyle,NodeHighlightStyle);
    switch ( NodeHighlightStyle )
      {
      case  treev_Invert:
        Fill_Shadow( self, shadow, graphic_INVERT, NULL );
	break;
      case  treev_Pale:
	Fill_Shadow( self, shadow, graphic_COPY, DottedTile );
	Draw_Node_Title( self, shadow );
	Draw_Node_Caption( self, shadow, Halo );
	break;
      case  treev_Border:

	break;
      case  treev_Bold:

	break;
      case  treev_Italic:

	break;
      }
    }
  ShadowVisage(shadow) = Highlighted;
  OUT(Highlight_Node_Shadow);
  }

static
void Footprint_Node_Shadow( class treev		  *self, struct node_shadow	  *shadow )
      {
  IN(Footprint_Node_Shadow);
  if ( ShadowExposed(shadow) )
    { DEBUGdt(NodeFootprintStyle,NodeFootprintStyle);
    switch ( NodeFootprintStyle )
      {
      case  treev_Invert:
        Fill_Shadow( self, shadow, graphic_INVERT, NULL );
	break;
      case  treev_Pale:
	Fill_Shadow( self, shadow, graphic_COPY, DottedTile );
	Draw_Node_Title( self, shadow );
	Draw_Node_Caption( self, shadow, Halo );
	break;
      case  treev_Border:

	break;
      case  treev_Bold:

	break;
      case  treev_Italic:

	break;
      }
    }
  ShadowVisage(shadow) = Footprinted;
  OUT(Footprint_Node_Shadow);
  }

void
treev::HighlightNodeCaption( struct tree_node   *node )
{
    class treev *self=this;
  IN(treev_HighlightNodeCaption);
  if ( node  &&  ! NodeViewObject(node) )
    {
/*===*/
    }
  OUT(treev_HighlightNodeCaption);
  }

boolean
treev::NodeCaptionHighlighted( struct tree_node   *node )
{
    class treev *self=this;
  if ( node  &&  ! NodeViewObject(node) )
    {
/*===*/
    }
return false;
  }

void
treev::NormalizeNodeCaption( struct tree_node   *node )
{
    class treev *self=this;
  IN(treev_NormalizeNodeCaption);
  if ( node  &&  ! NodeViewObject(node) )
    {
/*===*/
    }
  OUT(treev_NormalizeNodeCaption);
  }

void
treev::HideNodeChildren( struct tree_node   *node )
{
    class treev *self=this;
  struct tree_node  *shadow_node;

  IN(treev_HideNodeChildren);
  if ( ScrolledView )
    (ScrolledView)->HideNodeChildren(  node );
  else
  if ( node  &&  ChildNode(node)  &&
       (shadow_node = NodeShadowNode(node))  &&
	  ShadowChildrenExposed(ShadowNodeDatum(shadow_node)) )
    {
    Hide_Node_Children( this, shadow_node );
    Draw_Background( this );
    UpdateScrollbars;
    }
  OUT(treev_HideNodeChildren);
  }

static
void Hide_Node_Children( class treev	      *self, struct tree_node   *shadow_node )
      {
  struct tree_node  *shadow_peer;

  IN(Hide_Node_Children);
  shadow_peer = ChildNode(shadow_node);
  while ( shadow_peer )
    {
    if ( ShadowChildrenExposed(ShadowNodeDatum(shadow_peer)) )
      Hide_Node_Children( self, shadow_peer );
    shadow_peer = RightNode(shadow_peer);
    }
  GreatestRow -= ShadowChildrenRowCount(ShadowNodeDatum(shadow_node));
  if ( GreatestRow <= 0 )    GreatestRow = 1;
  GreatestColumn -= ShadowChildrenColumnCount(ShadowNodeDatum(shadow_node));
  if ( GreatestColumn <= 0 )  GreatestColumn = 1;
  Mark_Child_Exposure( self, shadow_node, false, false );
  OUT(Hide_Node_Children);
  }

void
treev::ExposeNodeChildren( struct tree_node   *node )
{
    class treev *self=this;
  IN(treev_ExposeNodeChildren);
  if ( ScrolledView )
    (ScrolledView)->ExposeNodeChildren(  node );
  else
  if ( node )
    {
    Expose_Node_Children( this, NodeShadowNode(node) );
    UpdateScrollbars;
    }
  OUT(treev_ExposeNodeChildren);
  }

static
void Expose_Node_Children( class treev	      *self, struct tree_node   *shadow_node )
      {
  struct tree_node  *shadow_peer;

  IN(Expose_Node_Children);
  if ( PARENTNODE(shadow_node) )
    {
    shadow_peer = ChildNode(PARENTNODE(shadow_node));
    while ( shadow_peer )
      {
      if ( ShadowChildrenExposed(ShadowNodeDatum(shadow_peer)) )
        Hide_Node_Children( self, shadow_peer );
      shadow_peer = RightNode(shadow_peer);
      }
    }
    else  Hide_Node_Children( self, ShadowRootNode );
  Draw_Background( self );
  if ( ChildNode(shadow_node) )
    {
    Mark_Child_Exposure( self, shadow_node, true, false );
    VerticalOffset = PendingVerticalOffset;
    HorizontalOffset = PendingHorizontalOffset;
    if ( VerticalArrangement )
      Arrange_Vertical_Tree( self, ChildNode(shadow_node),
	    Left, Top + GreatestRow * RowHeight, Width, Height );
      else
      Arrange_Horizontal_Tree( self, ChildNode(shadow_node),
	    Left + GreatestColumn * ColumnWidth, Top, Width, Height );
    Draw_Tree( self, ChildNode(shadow_node) );
    }
  OUT(Expose_Node_Children);
  }

boolean
treev::NodeChildrenExposed( struct tree_node   *node )
{
    class treev *self=this;
  if ( ScrolledView )
    return  (ScrolledView)->NodeChildrenExposed(  node );
    else  if ( node )
      return  NODECHILDRENEXPOSED(node);
      else  return false;
  }

void
treev::HideNode( struct tree_node   *node )
{
    class treev *self=this;
  struct tree_node  *shadow_node;
  struct tree_node  *parent;

  IN(treev_HideNode);
  if ( ScrolledView )
    (ScrolledView)->HideNode(  node );
  else
    {
    if ( node )
      {
      parent = PARENTNODE(node);
      if ( (this)->NodeHighlighted(  node ) )
	{
	(this)->NormalizeNode(  node );
	if ( ( CURRENTNODE = parent ) )
	  {
	  CurrentNodeShadow = NodeShadow(CURRENTNODE);
	  (this)->HighlightNode(  parent );
	  }
	}
      if ( parent )
	{
        if ( ( shadow_node = NodeShadowNode(parent) ) )
	  {
	  Hide_Node_Children( this, shadow_node );
	  Expose_Node_Children( this, shadow_node );
	  }
	}
	else
          if ( ( shadow_node = NodeShadowNode(node) ) )
	  {
	  Hide_Node_Children( this, shadow_node );
	  GreatestRow = GreatestColumn = 0;
	  Mark_Child_Exposure( this, shadow_node, false, false );
	  }
      ShadowHidden(NodeShadow(node)) = true;
      Draw_Background( this );
      }
    }
  OUT(treev_HideNode);
  }

void
treev::ExposeNode( struct tree_node   *node )
{
    class treev *self=this;
  struct tree_node  *parent;
  struct tree_node  *shadow_node;

  IN(treev_ExposeNode);
  if ( ScrolledView )
    (ScrolledView)->ExposeNode(  node );
  else
    {
    if ( node )
      {
      ShadowHidden(NodeShadow(node)) = false;
      if ( ( parent = PARENTNODE(node) ) )
        {
/*===
need to expose any parents not now exposed, down to node itself
might need to have parent hide its children, then expose them.
*/
        }
        else
        {
        shadow_node = NodeShadowNode(node);
        Mark_Child_Exposure( this, shadow_node, true, false );
        VerticalOffset = PendingVerticalOffset;
        HorizontalOffset = PendingHorizontalOffset;
	if ( VerticalArrangement )
          Arrange_Vertical_Tree( this, shadow_node,
	    Left, Top + GreatestRow * RowHeight, Width, Height );
	  else
          Arrange_Horizontal_Tree( this, shadow_node,
	    Left + GreatestColumn * ColumnWidth, Top, Width, Height );
        Draw_Tree( this, shadow_node );
        }
      }
    }
  OUT(treev_ExposeNode);
  }

boolean
treev::NodeExposed( struct tree_node   *node )
{
    class treev *self=this;
  if ( ScrolledView )
    return  (ScrolledView)->NodeExposed(  node );
    else  if ( node )
      return  NODEEXPOSED(node);
      else  return false;
  }

void
treev::ExplodeNode( struct tree_node   *node )
{
    class treev *self=this;
  struct tree_node  *shadow_node;

  IN(treev_ExplodeNode);
  if ( ScrolledView )
    (ScrolledView)->ExplodeNode(  node );
  else  if ( node )
    {
    (this)->HideNodeChildren(  node );
    shadow_node = NodeShadowNode(node);
    VerticalOffset = PendingVerticalOffset;
    HorizontalOffset = PendingHorizontalOffset;
    Mark_Child_Exposure( this, shadow_node, true, true );
    Arrange_Exploded_Tree( this, shadow_node, Left + GreatestColumn * ColumnWidth,
			    Top + GreatestRow * RowHeight, Width, Height  );
    Draw_Tree( this, shadow_node );
    }
  OUT(treev_ExplodeNode);
  }

void
treev::ImplodeNode( struct tree_node   *node )
{
    class treev *self=this;
  IN(treev_ImplodeNode);
  if ( ScrolledView )
    (ScrolledView)->ImplodeNode(  node );
  else  if ( node )
    {
    (this)->ExposeNodeChildren(  node );
    CURRENTNODE = node;
    CurrentNodeShadow = NodeShadow(node);
    }
  OUT(treev_ImplodeNode);
  }

boolean
treev::NodeExploded( struct tree_node   *node )
{
    class treev *self=this;
  if ( ScrolledView )
    return  (ScrolledView)->NodeExploded(  node );
    else  if ( node )
      return  NODEEXPLODED(node);
      else  return false;
  }

static
void Printer( class treev	      *self )
    {
  IN(Printer);
  (self)->SetPrintOrigin(  Left, Top );
  (self)->SetPrintFont(  NodeFontName );
  Print_Tree( self, ShadowRootNode );
  OUT(Printer);
  }

static
void Print_Tree( class treev	      *self, struct tree_node   *shadow_node )
      {
  struct node_shadow *shadow, *parent_shadow = NULL;
  struct tree_node   *shadow_parent, *shadow_peer;
  long		       L, T, R, B;
  boolean	       folded = false;

  IN(Print_Tree);
  if(shadow_node) {
      L = T = 99999; R = B = 0;
      shadow_peer = shadow_node;
      if ( ( shadow_parent = PARENTNODE(shadow_node) ) )
	  parent_shadow = ShadowNodeDatum(shadow_parent);
      while ( shadow_peer )
      {
	  shadow = ShadowNodeDatum(shadow_peer);
	  DEBUGst(Node Name,NodeName(ShadowedNode(shadow)));
	  if ( ShadowExposed(shadow)  &&  ShadowTop(shadow) < Bottom )
	  { DEBUG(Exposed);
	  Print_Node_Border( self, shadow );
	  (self)->PrintString(  SC, SM /*===*/-1,
			    NodeCaptionName(ShadowedNode(shadow)), 0 );
	  if ( ( folded = ShadowFolded(shadow) ) )
	  {
	      if ( SL < L )  L = SL - HalfMWidth;
	      if ( ST < T )  T = ST - HalfMWidth;
	      if ( SR > R )  R = SR + HalfMWidth;
	      if ( SB > B )  B = SB + HalfMWidth;
	  }
	  else
	      if ( parent_shadow )
		  Print_Node_Connector( self, shadow, parent_shadow );
	  if ( ShadowChildrenExposed(shadow) )
	      Print_Tree( self, ChildNode(shadow_peer) );
	  }
	  shadow_peer = RightNode(shadow_peer);
      }
      if ( folded )
      {
	  (self)->PrintBox(  L+1, T+1, R-L-2, B-T-2, 0 );
	  (self)->PrintBox(  L+2, T+2, R-L-4, B-T-4, 0 );
	  if ( parent_shadow  &&  !ShadowFolded(shadow) )
	  {
	      (self)->PrintLine(  SC, T, SC, SB+1 );
	  }
      }
  }
  OUT(Print_Tree);
  }

static
void Print_Node_Border( class treev	       *self, struct node_shadow  *shadow )
      {
  IN(Print_Node_Border);
  switch ( NodeBorderStyle )
    {
    case  treev_Rectangle:
      if ( DropShadow )
        { DEBUG(DropShadow);
        (self)->PrintBox(  SL, ST, SW-1, SH-1, 0 );
/*===suppress until Leaf attribute intriduced
	if ( ChildNode(ShadowedNode(shadow)) )
===*/
	  {
          (self)->PrintLine(  SR-1, ST+2, SR-1, SB-1 );
          (self)->PrintLine(  SR-1, SB-1, SL+2, SB-1 );
          (self)->PrintLine(  SR, ST+2, SR, SB );
          (self)->PrintLine(  SR, SB, SL+2, SB );
	  }
        }
        else
        (self)->PrintBox(  SL, ST, SW, SH, 0 );
      break;
    case  treev_Circle:
      (self)->PrintCircle(  SC, SM, SH/2 );
      break;
    case  treev_Oval:
/*===*/
      break;
    case  treev_RoundAngle:
      (self)->PrintRoundBox(  SL, ST, SW, SH, 0 );
      break;
    case  treev_Folder:
/*===*/
      break;
    }
  OUT(Print_Node_Border);
  }

static
void Print_Node_Connector( class treev	       *self, struct node_shadow  *shadow , struct node_shadow  *parent_shadow )
      {
  IN(Print_Node_Connector);
  if ( !ShadowFolded(shadow)  &&  !ShadowFolded(parent_shadow) )
    { DEBUG(Connect to Parent);
    if ( VerticalArrangement )
      (self)->PrintMoveTo(  SC, ST );
      else
      (self)->PrintMoveTo(  SL, SM );
    switch ( NodeConnectorStyle )
      {
      case  treev_Direct:
	if ( VerticalArrangement )
	  (self)->PrintLineTo(  ShadowCenter(parent_shadow),
				   ShadowBottom(parent_shadow) + 1 );
	  else
	  (self)->PrintLineTo(  ShadowRight(parent_shadow),
				   ShadowMiddle(parent_shadow) );
	break;
      case  treev_DogLeg:
	if ( VerticalArrangement )
	  {
          (self)->PrintLineTo(  SC, ST - HalfMWidth );
          (self)->PrintLine(  SC, ST - HalfMWidth, ShadowCenter(parent_shadow),
				 ShadowBottom(parent_shadow) + HalfMWidth );
          (self)->PrintLine(  ShadowCenter(parent_shadow),
				 ShadowBottom(parent_shadow) + HalfMWidth,
				 ShadowCenter(parent_shadow),
				 ShadowBottom(parent_shadow) + 1 );
	  }
	  else
	  {
          (self)->PrintLineTo(  SL - HalfMWidth, SM );
          (self)->PrintLine(  SL - HalfMWidth, SM,
				ShadowRight(parent_shadow) + HalfMWidth,
				ShadowMiddle(parent_shadow) );
          (self)->PrintLine(  ShadowRight(parent_shadow) + HalfMWidth,
				ShadowMiddle(parent_shadow),
				ShadowRight(parent_shadow),
				ShadowMiddle(parent_shadow) );
	  }
        break;
      }
    }
  OUT(Print_Node_Connector);
  }

void
treev::Print( FILE		      *file, const char		      *processor, const char		      *format, boolean	       level )
{
    class treev *self=this;
  static struct aptv_print_stream   *print_stream;

  IN(treev_Print);
  if ( ScrollView )
    {
    print_stream = (this )->PrintStream( );
    (ScrolledView)->Print(  file, processor, format, level );
    }
  else
  {
  if ( print_stream )
    (this)->SetPrintStream(  print_stream );
  if ( ! GraphicsInitialized )
      return; /* can't print from ezprint yet */
  (this)->PrintObject(  file, processor, format, level, (aptv_printfptr)Printer );
  }
  OUT(treev_Print);
  }

class view *
treev::GetApplicationLayer( )
{
    class treev *self=this;
  class scroll     *view;
  long		      mode = 0;

  IN(treev_GetApplicationLayer);
  if ( Scroll & treev_Left )	mode |= scroll_LEFT;
  if ( Scroll & treev_Bottom )	mode |= scroll_BOTTOM;
  view = scroll::Create( this, mode );
  (view)->SetView(  this );
  OUT(treev_GetApplicationLayer);
  return  (class view *) view;
  }


 
				       
				        
 
static const struct scrollfns		      vertical_scroll_interface =
		{(scroll_getinfofptr) y_getinfo, (scroll_setframefptr)y_setframe, (scroll_endzonefptr)y_endzone, (scroll_whatfptr)y_whatisat };
static const struct scrollfns		      horizontal_scroll_interface =
		{ (scroll_getinfofptr)x_getinfo, (scroll_setframefptr)x_setframe, (scroll_endzonefptr)x_endzone, (scroll_whatfptr)x_whatisat };

const void *
treev::GetInterface( const char		      *interface_name )
{
    class treev *self=this;
  const struct scrollfns  *interface = NULL;

  IN(treev_GetInterface);
  DEBUGst(Interface Name,interface_name);
  if ( (Scroll & treev_Left)  &&
	apts::CompareStrings( interface_name, "scroll,vertical" ) == 0 )
    interface = &vertical_scroll_interface;
    else
    if ( (Scroll & treev_Bottom)  &&
	  apts::CompareStrings( interface_name, "scroll,horizontal" ) == 0 )
      interface = &horizontal_scroll_interface;
  OUT(treev_GetInterface);
  return  interface;
  }

static void
y_getinfo( class treev	      *self, struct range	      *total , struct range	      *seen , struct range	      *dot )
      {
  long		      extent = GreatestRow * RowHeight;
/* debug=1; */
  IN(y_getinfo);
  total->beg = 0;
  total->end = extent;
  DEBUGdt(VerticalOffset,VerticalOffset);
  seen->beg = -VerticalOffset;
  seen->end = min(-VerticalOffset + Height, extent);
  dot->beg = dot->end = 0;
  DEBUGdt(Total-beg, total->beg);  DEBUGdt(Total-end, total->end);
  DEBUGdt(Seen-beg, seen->beg);  DEBUGdt(Seen-end, seen->end);
  OUT(y_getinfo);
/* debug=0; */
  }

static long
y_whatisat( class treev	      *self, long		       pos , long		       outof )
      {
  long		      value, coord;
/* debug=1; */
  IN(y_whatisat);
  DEBUGlt(Pos,pos);  DEBUGlt(Outof, outof);

    coord = pos * (self)->GetLogicalHeight();
    coord /= outof;

  value = -VerticalOffset + coord;
  DEBUGlt(Value,value);
  OUT(y_whatisat);
/* debug=0; */
  return  value;
  }

static void
y_setframe( class treev	      *self, int		       place, long		       pos , long		       outof )
        {
/* debug=1; */
  IN(y_setframe);
  DEBUGdt(Place, place);  DEBUGlt(Pos, pos);  DEBUGlt(Outof, outof);
  PendingVerticalOffset = -(place - pos);
  if ( -PendingVerticalOffset >= GreatestRow * RowHeight )
    PendingVerticalOffset = -((GreatestRow-1) * RowHeight);
  DEBUGdt(PendingVerticalOffset,PendingVerticalOffset);
  PendingUpdate = (treev_updatefptr)Redisplay;
  (self)->WantUpdate(  self );
  OUT(y_setframe);
/* debug=0; */
  }

static void
y_endzone( class treev	      *self, int		       zone , int		       action )
      {
  long		      nrows, proposed_offset;
/* debug=1; */
  IN(y_endzone);
  DEBUGdt(Zone,zone);  DEBUGdt(Action,action);
  if ( zone == scroll_TOPENDZONE )
    { DEBUG(Top);
    proposed_offset = 0;
    }
    else
    { DEBUG(Bottom);
    nrows = Height / RowHeight;
    proposed_offset = (nrows - 2)*RowHeight - (GreatestRow * RowHeight);
    }
  DEBUGdt(proposed_offset,proposed_offset);
  if (proposed_offset != PendingVerticalOffset) 
    {
    PendingVerticalOffset = proposed_offset; 
    PendingUpdate = (treev_updatefptr)Redisplay;
    (self)->WantUpdate(  self );
    }
  OUT(y_endzone);
/* debug=0; */
  }

/* HorizontalOffset will be positive to center image */

static void
x_getinfo( class treev	      *self, struct range	      *total , struct range	      *seen , struct range	      *dot )
      {
  long		      extent = GreatestColumn * ColumnWidth;
/*debug=1;*/
  IN(x_getinfo);
  total->beg = 0;
  total->end = extent;
  DEBUGdt(HorizontalOffset,HorizontalOffset);
  seen->beg = max(0, -HorizontalOffset);
  seen->end = min(seen->beg + Width, extent);
  dot->beg = dot->end = 0;
  DEBUGdt(Total-beg, total->beg);  DEBUGdt(Total-end, total->end);
  DEBUGdt(Seen-beg, seen->beg);  DEBUGdt(Seen-end, seen->end);
  OUT(x_getinfo);
/*debug=0;*/
  }

static long
x_whatisat( class treev	      *self, long		       pos , long		       outof )
      {
  long		      value, coord;
/*debug=1;*/
  IN(x_whatisat);
  DEBUGlt(Pos,pos);  DEBUGlt(Outof, outof);

    coord = pos * (self)->GetLogicalWidth();
    coord /= outof;

  value = -HorizontalOffset + coord;
  DEBUGlt(Value,value);
  OUT(x_whatisat);
/*debug=0;*/
  return value;
  }

static void
x_setframe( class treev	      *self, long		       place , long		       pos , long		       outof )
      {
  long		      extent = GreatestColumn * ColumnWidth;
/*debug=1;*/
  IN(x_setframe);
  DEBUGdt(Place,place);  DEBUGlt(Pos,pos);  DEBUGlt(Outof,outof );
  PendingHorizontalOffset = -(place - pos);
  DEBUGdt(PendingHorizontalOffset,PendingHorizontalOffset);
  if ( -PendingHorizontalOffset >= extent )
    PendingHorizontalOffset = -((GreatestColumn-1) * ColumnWidth);
  DEBUGdt(PendingHorizontalOffset,PendingHorizontalOffset);
  PendingUpdate = (treev_updatefptr)Redisplay;
  (self)->WantUpdate(  self );
  OUT(x_setframe);
/*debug=0;*/
  }

static void
x_endzone( class treev	      *self, long		       zone , long		       action )
      {
  long		      proposed_offset, ncols;
/*debug=1;*/
  IN(x_endzone);
  DEBUGdt(Zone,zone);  DEBUGdt(Action,action);
  if ( zone == scroll_TOPENDZONE )
    { DEBUG(Left);
    proposed_offset = 0;
    }
    else
    { DEBUG(Right);
    ncols = Width / ColumnWidth;
    proposed_offset = (ncols-2)*ColumnWidth - (GreatestColumn * ColumnWidth);
    }
  DEBUGdt(proposed_offset,proposed_offset);
  if (proposed_offset != PendingHorizontalOffset) 
    {
    PendingHorizontalOffset = proposed_offset; 
    PendingUpdate = (treev_updatefptr)Redisplay;
    (self)->WantUpdate(  self );
    }
  OUT(x_endzone);
/*debug=0;*/
  }

static
void Arrange_Tree( class treev	      *self, struct tree_node   *shadow_node, long		       left , long		       top , long		       width , long		       height )
        {
  IN(Arrange_Tree);
  if ( VerticalArrangement )
    Arrange_Vertical_Tree( self, shadow_node, left, top, width, height );
    else
    Arrange_Horizontal_Tree( self, shadow_node, left, top, width, height );
  OUT(Arrange_Tree);
  }

static
void Arrange_Horizontal_Tree( class treev	      *self, struct tree_node   *shadow_node, long		       left , long		       top , long		       width , long		       height )
        {
  boolean	      folded = false;
  long		      L = left, T = top, top_offset = 0,
			      peers = 0, column = 0, columns = 1,
			      row = 0, rows, indent;
  struct node_shadow *shadow;
  struct tree_node   *shadow_parent, *shadow_peer;

  IN(Arrange_Horizontal_Tree);
  peers = 1 + (ShadowTree)->PeerNodeCount(  shadow_node );
  rows = (height / RowHeight) ? (height / RowHeight) : 1;
  if ( Fold )
    columns = peers / rows + ((peers % rows) ? 1 : 0);
  DEBUGdt(Peers,peers);  DEBUGdt(Columns,columns);  DEBUGdt(Rows,rows);
  GreatestColumn += columns;
  DEBUGdt(GreatestColumn,GreatestColumn);
  if ( peers > rows  &&  Fold )
    { DEBUG(Fold);
    folded = true;
    if ( NodeOrderColumnMajor )
      {
      if ( (indent = ((peers / columns) + ((peers % columns) ? 1 : 0))) <= rows )
        T += (height - (indent * RowHeight)) / 2;
      }
      else
      T += (top_offset = (height - (rows * RowHeight)) / 2);
    }
    else
    T += (height - (peers * RowHeight)) / 2;
  if ( ! folded )    rows = peers;
  GreatestRow = max( GreatestRow, rows );
  shadow_peer = shadow_node;
  while ( shadow_peer )
    {
    shadow = ShadowNodeDatum(shadow_peer);
    DEBUGst(Node Name,NodeName(ShadowedNode(shadow)));
    if ( ShadowExposed(shadow)  &&  !ShadowHidden(shadow) )
      { DEBUG(Exposed);
      ShadowFolded(shadow) = folded;
      if ( ShadowExploded(shadow) )
	Arrange_Horizontal_Exploded_Tree( self, shadow_peer,
	    left + (columns * ColumnWidth), top, width, height );
        else  if ( ChildNode(shadow_peer)  &&  ShadowChildrenExposed(shadow) )
	Arrange_Horizontal_Tree( self, ChildNode(shadow_peer),
	    left + (columns * ColumnWidth), top, width, height );
      if ( NodeOrderRowMajor )
	{
        if ( row++ ) {
	  if ( row > rows  &&  folded )
            {
            row = 1;
            T = top + top_offset;
	    L += ColumnWidth;
            }
            else  T += RowHeight;
	}
	}
	else
	{
	if ( ! folded  &&  row++ )
	  T += RowHeight;
	  else
	  if ( column++ ) {
	    if ( column > columns )
	      {
	      column = 1;
	      T += RowHeight;
	      L  = left;
	      }
	      else  L += ColumnWidth;
	  }
	}
      SL = L + HalfMWidth + HorizontalOffset;
      ST = T + HalfMWidth + VerticalOffset;
      SW = CellWidth;  SH = CellHeight;
      }
    shadow_peer = RightNode(shadow_peer);
    }
  DEBUG(Peers Done);
  if ( (shadow_parent = PARENTNODE(shadow_node))  &&
       (shadow = ShadowNodeDatum(shadow_parent)) )
    {
    ShadowChildrenFolded(shadow) = folded;
    ShadowChildrenColumnCount(shadow) = columns;
    DEBUGdt(ColumnCount,ShadowChildrenColumnCount(shadow));
    }
  OUT(Arrange_Horizontal_Tree);
  }

static
void Arrange_Vertical_Tree( class treev	      *self, struct tree_node   *shadow_node, long		       left , long		       top , long		       width , long		       height )
        {
  boolean	      folded = false;
  long		      L = left, T = top, left_offset = 0,
			      peers = 0, column = 0, columns,
			      row = 0, rows = 1, indent;
  struct node_shadow *shadow;
  struct tree_node   *shadow_parent, *shadow_peer;

  IN(Arrange_Vertical_Tree);
  peers = 1 + (ShadowTree)->PeerNodeCount(  shadow_node );
  columns = (width / ColumnWidth) ? (width / ColumnWidth) : 1;
  if ( Fold )
    rows = peers / columns + ((peers % columns) ? 1 : 0);
  DEBUGdt(Peers,peers);  DEBUGdt(Columns,columns);  DEBUGdt(Rows,rows);
  GreatestRow += rows;
  DEBUGdt(GreatestRow,GreatestRow);
  if ( peers > columns  &&  Fold )
    { DEBUG(Fold);
    folded = true;
    if ( NodeOrderColumnMajor )
      {
      if ( (indent = ((peers / rows) + ((peers % rows) ? 1 : 0))) <= columns )
        L += (width - (indent * ColumnWidth)) / 2;
      }
      else
      L += (left_offset = (width - (columns * ColumnWidth)) / 2);
    }
    else
    L += (width - (peers * ColumnWidth)) / 2;
  if ( ! folded )    columns = peers;
  GreatestColumn = max( GreatestColumn, columns );
  shadow_peer = shadow_node;
  while ( shadow_peer )
    {
    shadow = ShadowNodeDatum(shadow_peer);
    DEBUGst(Node Name,NodeName(ShadowedNode(shadow)));
    if ( ShadowExposed(shadow)  &&  !ShadowHidden(shadow) )
      { DEBUG(Exposed);
      ShadowFolded(shadow) = folded;
      if ( ShadowExploded(shadow) )
	Arrange_Vertical_Exploded_Tree( self, shadow_peer,
	    left, top + (rows * RowHeight), width, height );
        else  if ( ChildNode(shadow_peer)  &&  ShadowChildrenExposed(shadow) )
        Arrange_Vertical_Tree( self, ChildNode(shadow_peer),
	    left, top + (rows * RowHeight), width, height );
      if ( NodeOrderRowMajor )
	{
        if ( column++ ) {
	  if ( column > columns  &&  folded )
            {
            column = 1;
            L = left + left_offset;
	    T += RowHeight;
            }
            else  L += ColumnWidth;
	}
	}
	else
	{
	if ( ! folded  &&  column++ )
	  L += ColumnWidth;
	  else
	  if ( row++ ) {
	    if ( row > rows )
	      {
	      row = 1;
	      T = top;
	      L += ColumnWidth;
	      }
	      else  T += RowHeight;
	  }
	}
      SL = L + HalfMWidth + HorizontalOffset;
      ST = T + HalfMWidth + VerticalOffset;
      SW = CellWidth;  SH = CellHeight;
      }
    shadow_peer = RightNode(shadow_peer);
    }
  DEBUG(Peers Done);
  if ( (shadow_parent = PARENTNODE(shadow_node))  &&
       (shadow = ShadowNodeDatum(shadow_parent)) )
    {
    ShadowChildrenFolded(shadow) = folded;
    ShadowChildrenRowCount(shadow) = rows;
    DEBUGdt(RowCount,ShadowChildrenRowCount(shadow));
    }
  OUT(Arrange_Vertical_Tree);
  }

static
void Arrange_Exploded_Tree( class treev	      *self, struct tree_node   *shadow_node, long		       left , long		       top , long		       width , long		       height )
        {
  if ( VerticalArrangement )
    Arrange_Vertical_Exploded_Tree( self, shadow_node, left, top, width, height );
    else
    Arrange_Horizontal_Exploded_Tree( self, shadow_node, left, top, width, height );
  }

static long
Compute_Horizontal_Exploded_SubTree_Heights( class treev	      *self, struct tree_node   *shadow_node )
      {
  struct tree_node  *shadow_peer;
  struct node_shadow *shadow;
  long		      height = 0;

  IN(Compute_Horizontal_Exploded_SubTree_Heights);
  if ( ( shadow_peer = ChildNode(shadow_node) ) )
    while ( shadow_peer )
      {
      height += Compute_Horizontal_Exploded_SubTree_Heights( self, shadow_peer );
      shadow_peer = RightNode(shadow_peer);
      }
    else height = CellHeight + HalfMWidth;
  DEBUGdt(height,height);
  shadow = ShadowNodeDatum(shadow_node);
  ShadowSubHeight(shadow) = height;
  OUT(Compute_Horizontal_Exploded_SubTree_Heights);
  return  height;
  }

static
void Arrange_Horizontal_Exploded_Tree( class treev	      *self, struct tree_node   *shadow_node, long		       left , long		       top , long		       width , long		       height )
        {
  long		      sub_height;

  IN(Arrange_Horizontal_Exploded_Tree);
  ShadowExploded(ShadowNodeDatum(shadow_node)) = true;
  sub_height = Compute_Horizontal_Exploded_SubTree_Heights( self, shadow_node );
  DEBUGdt(sub_height,sub_height);
  GreatestRow = sub_height/RowHeight;
  Arrange_Horizontal_Exploded_SubTree( self, shadow_node, left,
			Middle - ShadowSubHeight(ShadowNodeDatum(shadow_node))/2,
			width, ShadowSubHeight(ShadowNodeDatum(shadow_node)) );
  OUT(Arrange_Horizontal_Exploded_Tree);
  }

static
void Arrange_Horizontal_Exploded_SubTree( class treev	      *self, struct tree_node   *shadow_node, long		       left , long		       top , long		       width , long		       height )
        {
  struct node_shadow *shadow;
  struct tree_node  *shadow_peer;
  long		      T = top, H;

  IN(Arrange_Horizontal_Exploded_SubTree);
  if ( ( shadow_peer = ChildNode(shadow_node) ) )
    {
    ShadowChildrenColumnCount(ShadowNodeDatum(shadow_node)) = 1;
    GreatestColumn++;
    while ( shadow_peer )
      {
      shadow = ShadowNodeDatum(shadow_peer);
      ShadowExploded(shadow) = true;
      H = ShadowSubHeight(shadow);
      SL = left + HalfMWidth + HorizontalOffset;
      ST = ((T + H/2) - CellHeight/2) + HalfMWidth + VerticalOffset;
      SW = CellWidth;  SH = CellHeight;
      Arrange_Horizontal_Exploded_SubTree( self, shadow_peer, left + ColumnWidth,
					     T, width, H );
      T += H;
      shadow_peer = RightNode(shadow_peer);
      }
    }
  OUT(Arrange_Horizontal_Exploded_SubTree);
  }

static long
Compute_Vertical_Exploded_SubTree_Widths( class treev	      *self, struct tree_node   *shadow_node )
      {
  struct tree_node  *shadow_peer;
  struct node_shadow *shadow;
  long		      width = 0;

  IN(Compute_Vertical_Exploded_SubTree_Widths);
  if ( ( shadow_peer = ChildNode(shadow_node) ) )
    while ( shadow_peer )
      {
      width += Compute_Vertical_Exploded_SubTree_Widths( self, shadow_peer );
      shadow_peer = RightNode(shadow_peer);
      }
    else width = CellWidth + HalfMWidth;
  DEBUGdt(width,width);
  shadow = ShadowNodeDatum(shadow_node);
  ShadowSubWidth(shadow) = width;
  OUT(Compute_Vertical_Exploded_SubTree_Widths);
  return  width;
  }

static
void Arrange_Vertical_Exploded_Tree( class treev	      *self, struct tree_node   *shadow_node, long		       left , long		       top , long		       width , long		       height )
        {
  long		      sub_width;

  IN(Arrange_Vertical_Exploded_Tree);
  ShadowExploded(ShadowNodeDatum(shadow_node)) = true;
  sub_width = Compute_Vertical_Exploded_SubTree_Widths( self, shadow_node );
  DEBUGdt(Sub-width,sub_width);
  GreatestColumn = sub_width/ColumnWidth;
  Arrange_Vertical_Exploded_SubTree( self, shadow_node,
			Center - ShadowSubWidth(ShadowNodeDatum(shadow_node))/2,
			top, ShadowSubWidth(ShadowNodeDatum(shadow_node)), height );
  OUT(Arrange_Vertical_Exploded_Tree);
  }

static
void Arrange_Vertical_Exploded_SubTree( class treev	      *self, struct tree_node   *shadow_node, long		       left , long		       top , long		       width , long		       height )
        {
  struct node_shadow *shadow;
  struct tree_node  *shadow_peer;
  long		      L = left, W;

  IN(Arrange_Vertical_Exploded_SubTree);
  if ( ( shadow_peer = ChildNode(shadow_node) ) )
    {
    ShadowChildrenRowCount(ShadowNodeDatum(shadow_node)) = 1;
    GreatestRow++;
    while ( shadow_peer )
      {
      shadow = ShadowNodeDatum(shadow_peer);
      ShadowExploded(shadow) = true;
      W = ShadowSubWidth(shadow);
      SL = ((L + W/2) - CellWidth/2) + HalfMWidth + HorizontalOffset;
      ST = top + HalfMWidth + VerticalOffset;
      SW = CellWidth;  SH = CellHeight;
      Arrange_Vertical_Exploded_SubTree( self, shadow_peer, L, top + RowHeight, W, height );
      L += W;
      shadow_peer = RightNode(shadow_peer);
      }
    }
  OUT(Arrange_Vertical_Exploded_SubTree);
  }

static
void Mark_Child_Exposure( class treev	      *self, struct tree_node   *shadow_node, boolean	       state , boolean	       recursive )
        {
  struct tree_node  *shadow_peer = ChildNode(shadow_node);
  struct node_shadow *shadow = ShadowNodeDatum(shadow_node);

  IN(Mark_Child_Exposure);
  ShadowChildrenExposed(shadow) = state;
  if ( state == false )
    {
    ShadowChildrenRowCount(shadow) = 0;
    ShadowExploded(shadow) = false;
    }
  while ( shadow_peer )
    {
    shadow = ShadowNodeDatum(shadow_peer);
    ShadowExposed(shadow) = state;
    if ( state == false )
      ShadowExploded(ShadowNodeDatum(shadow_node)) = false;
    if ( recursive  ||  state == false )
      Mark_Child_Exposure( self, shadow_peer, state, recursive );
    shadow_peer = RightNode(shadow_peer);
    }
  OUT(Mark_Child_Exposure);
  }

static
void Draw_Tree( class treev	      *self, struct tree_node   *shadow_node )
      {
  struct node_shadow *shadow, *parent_shadow = NULL;
  struct tree_node   *shadow_parent, *shadow_peer;
  long		       L, T, R, B;
  boolean	       folded = false;

  IN(Draw_Tree);
  if( ViewLinked )
  {
  DEBUGst(Given Node Name,NodeName(ShadowedNode(ShadowNodeDatum(shadow_node))));
  L = T = 99999; R = B = 0;
  if ( (self )->GetFont( ) != NodeFont )
    (self)->SetFont(  NodeFont );
  shadow_peer = shadow_node;
  if ( ( shadow_parent = PARENTNODE(shadow_node) ) )
    parent_shadow = ShadowNodeDatum(shadow_parent);
  while ( shadow_peer )
    {
    shadow = ShadowNodeDatum(shadow_peer);
    folded = ShadowFolded(shadow);
    DEBUGst(Node Name,NodeName(ShadowedNode(shadow)));
    ShadowVisible(shadow) = false;
    if ( ShadowExposed(shadow) )
      { DEBUG(Exposed);
      if ( SB > (Top - RowHeight)    &&  ST < (Bottom + RowHeight)  &&
	   SR > (Left - ColumnWidth) &&  SL < (Right + ColumnWidth) )
	{
	ShadowVisible(shadow) = true;
        Draw_Node_Border( self, shadow );
	Clear_Shadow( self, shadow );
	Draw_Node_Title( self, shadow );
	Draw_Node_Caption( self, shadow, NoHalo );
	if ( ShadowHighlighted(shadow) )
	  Highlight_Node_Shadow( self, shadow );
	else
	  if ( ShadowFootprinted(shadow) )
            Footprint_Node_Shadow( self, shadow );
	}
      if ( ! folded  &&  parent_shadow )
	Draw_Node_Connector( self, shadow, parent_shadow );
      if ( ChildNode(shadow_peer)  &&  ShadowChildrenExposed(shadow) )
	{ DEBUG(Expose Children);
        Draw_Tree( self, ChildNode(shadow_peer) );
	}
      }
    if ( folded )
      {
      if ( SL < L )  L = SL - HalfMWidth;
      if ( ST < T )  T = ST - HalfMWidth;
      if ( SR > R )  R = SR + HalfMWidth;
      if ( SB > B )  B = SB + HalfMWidth;
      }
    shadow_peer = RightNode(shadow_peer);
    }
  if ( folded )
    {
    (self)->DrawRectSize(  L+1, T+1, R-L-2, B-T-2 );
    (self)->DrawRectSize(  L+2, T+2, R-L-4, B-T-4 );
    if ( parent_shadow  &&  !ShadowFolded(shadow) )
      {
      (self)->MoveTo(  SC, T );
      (self)->DrawLineTo(  SC, SB+1 );
      }
    }
  }
  OUT(Draw_Tree);
  }

static
void Draw_Background( class treev	      *self )
    {
  long		      left, top, width, height;

  IN(Draw_Background);
  if ( ViewLinked && BackgroundShade && CurrentNodeShadow)
    {
    DEBUGxt(CN,CURRENTNODE);  DEBUGxt(CNS,CurrentNodeShadow);
    if ( VerticalArrangement )
      {
      left = Left;
      top = Top + VerticalOffset + (GreatestRow * RowHeight) +
	     (DropShadow * 2);
      if ( ! ShadowFolded(CurrentNodeShadow) )
        top -= HalfMWidth;
      top--;
      }
      else
      {
      left = Left + HorizontalOffset + (GreatestColumn * ColumnWidth) +
	    (DropShadow * 2);
      if ( ! ShadowFolded(CurrentNodeShadow) )
	left -= HalfMWidth;
      top = Top;
      }
    width = Right - left + 1;
    height = Bottom - top + 1;
    if ( BackgroundNonWhite )
      (self)->SetTransferMode(  graphic_COPY );
    else {
	treev_FlipColors(self);
    }
    DEBUGdt(l,left);DEBUGdt(t,top);DEBUGdt(w,width);DEBUGdt(h,height);
    (self)->FillRectSize(  left, top, width, height, BackgroundPattern );
    if(!BackgroundNonWhite) treev_RestoreColors(self);
    (self)->SetTransferMode(  graphic_COPY );
    }
  OUT(Draw_Background);
  }

static
void Fill_Background( class treev	      *self )
    {
  if ( BackgroundShade )
    {
    if ( BackgroundNonWhite )
      {
      BackgroundPattern = (self)->GrayPattern(  BackgroundShade, 100 );
      (self)->SetTransferMode(  graphic_COPY );
      (self)->FillRect(  Bounds, BackgroundPattern );
      }
      else
      {
      treev_FlipColors(self);
      (self)->FillRect(  Bounds, BackgroundPattern );
      treev_RestoreColors(self);
      }
    (self)->SetTransferMode(  graphic_COPY );
    }
  }

static
void Draw_Node_Caption( class treev	       *self, struct node_shadow  *shadow, boolean	        halo )
        {
  short	       i, j;

  IN(Draw_Node_Caption);
  if ( NodeCaptionName(ShadowedNode(shadow)) )
    { DEBUG(Captioned);
    if ( halo )
    {
	treev_FlipColors(self);
      for ( i = -1; i < 2; i++ )
      for ( j = -1; j < 2; j++ )
	{
        (self)->MoveTo(  SC + i, SM+1 + j );
        (self)->DrawString(  NodeCaptionName(ShadowedNode(shadow)), Balanced );
      }
      treev_RestoreColors(self);
    }
    (self)->MoveTo(  SC, SM+1 );
    (self)->DrawString(  NodeCaptionName(ShadowedNode(shadow)), Balanced );
    }
  OUT(Draw_Node_Caption);
  }

static
void Draw_Node_Title( class treev	       *self, struct node_shadow  *shadow )
      {
  IN(Draw_Node_Title);
  if ( NodeTitle(ShadowedNode(shadow)) )
    { DEBUG(Titled);
/*===*/  (self)->MoveTo(  SC, SM+1 );
    (self)->DrawString(  NodeTitle(ShadowedNode(shadow)), Balanced );
    }
  OUT(Draw_Node_Title);
  }

static
void Draw_Node_Border( class treev	       *self, struct node_shadow  *shadow )
      {
  IN(Draw_Node_Border);
  switch ( NodeBorderStyle )
    {
    case  treev_Rectangle:
      if ( DropShadow )
        { DEBUG(DropShadow);
        (self)->DrawRectSize(  SL, ST, SW-2, SH-2 );
/*===suppress until Leaf attribute intriduced
	if ( ChildNode(ShadowedNode(shadow)) )
===*/
	  {
          (self)->MoveTo(  SR-1, ST+2 );
          (self)->DrawLineTo(  SR-1, SB-1 );
          (self)->DrawLineTo(  SL+2, SB-1 );
          (self)->MoveTo(  SR, ST+2 );
          (self)->DrawLineTo(  SR, SB );
          (self)->DrawLineTo(  SL+2, SB );
	  }
        }
        else
        (self)->DrawRect(  ShadowBounds(shadow) );
      break;
    case  treev_Oval:
    case  treev_Circle:
      (self)->DrawOval(  ShadowBounds(shadow) );
      break;
    case  treev_RoundAngle:
      (self)->DrawRRectSize(  SL, ST, SW-2, SH-2, 10, 10 );
      break;
    case  treev_Folder:
      (self)->MoveTo(  SL + 5, ST );
      (self)->DrawLineTo(  SL + SW/3, ST );
      (self)->DrawLineTo(  SL + SW/3+4, ST+5 );
      (self)->DrawLineTo(  SR-1, ST+5 );
      (self)->DrawLineTo(  SR-1, SB-1 );
      (self)->DrawLineTo(  SL, SB-1 );
      (self)->DrawLineTo(  SL, ST+5 );
      (self)->DrawLineTo( SL+5, ST );
      break;
    }
  OUT(Draw_Node_Border);
  }

static
void Draw_Node_Connector( class treev	       *self, struct node_shadow  *shadow , struct node_shadow  *parent_shadow )
      {
  IN(Draw_Node_Connector);
  if ( !ShadowFolded(shadow)  &&  !ShadowFolded(parent_shadow) )
    { DEBUG(Connect to Parent);
    if ( VerticalArrangement )
      (self)->MoveTo(  SC, ST );
      else
      (self)->MoveTo(  SL, SM );
    switch ( NodeConnectorStyle )
      {
      case  treev_Direct:
	if ( VerticalArrangement )
	  (self)->DrawLineTo(  ShadowCenter(parent_shadow),
				  ShadowBottom(parent_shadow) + 1 );
	  else
	  (self)->DrawLineTo(  ShadowRight(parent_shadow),
				  ShadowMiddle(parent_shadow) );
	break;
      case  treev_DogLeg:
	if ( VerticalArrangement )
	  {
          (self)->DrawLineTo(  SC, ST - HalfMWidth );
          (self)->DrawLineTo(  ShadowCenter(parent_shadow),
				  ShadowBottom(parent_shadow) + HalfMWidth );
          (self)->DrawLineTo(  ShadowCenter(parent_shadow),
				  ShadowBottom(parent_shadow) + 1 );
	  }
	  else
	  {
          (self)->DrawLineTo(  SL - HalfMWidth, SM );
          (self)->DrawLineTo(  ShadowRight(parent_shadow) + HalfMWidth,
				  ShadowMiddle(parent_shadow) );
          (self)->DrawLineTo(  ShadowRight(parent_shadow),
				  ShadowMiddle(parent_shadow) );
	  }
        break;
      }
    }
  OUT(Draw_Node_Connector);
  }

static struct node_shadow *
Node_Shadow( class treev	      *self, struct tree_node   *node )   /*=== might want to optimize !!! ===*/
      {
  struct tree_node  *candidate;
  struct node_shadow *shadow = NULL, *shadow_candidate;

  IN(Node_Shadow);
  if ( node )
    { DEBUGst(Node Name,(Tree)->NodeName(  node ) );
    candidate = ShadowRootNode;
    while ( candidate )
      {
      shadow_candidate = ShadowNodeDatum(candidate);
      DEBUGst(Candidate Name,(Tree)->NodeName(  ShadowedNode(shadow_candidate) ) );
      if ( ShadowedNode(shadow_candidate) == node )
	{
	shadow = shadow_candidate;
	break;
	}
      candidate = NextShadowNode(candidate);
      }
    }
  OUT(Node_Shadow);
  return  shadow;
  }

static tree_type_node
Node_Shadow_Node( class treev	      *self, struct tree_node   *node )   /*=== might want to optimize !!! ===*/
      {
  struct tree_node  *candidate, *shadow_node = NULL;
  struct node_shadow *shadow = NULL;

  IN(Node_Shadow_Node);
  if ( node )
    { DEBUGst(Node Name,(Tree)->NodeName(  node ) );
    candidate = ShadowRootNode;
    while ( candidate )
      {
      shadow = ShadowNodeDatum(candidate);
      DEBUGst(Candidate Name,NodeName(ShadowedNode(shadow)));
      if ( ShadowedNode(shadow) == node )
	{
	shadow_node = candidate;
	break;
	}
      candidate = NextShadowNode(candidate);
      }
    }
  OUT(Node_Shadow_Node);
  return  shadow_node;
  }

static struct node_shadow *
Create_Shadow( class treev	       *self )
    {
  struct node_shadow *shadow;

  if ( (shadow = (struct node_shadow *)
	    calloc( 1, sizeof(struct node_shadow) )) == NULL )
    { DEBUG(ERROR -- No Space);
/*===*/ printf( "TreeView:  No ShadowTree Node Space.\n" );
    Destroy_Shadows( self, ROOTNODE );
    }
  return  shadow;
  }

static
long Generate_Shadows( class treev	       *self, struct tree_node    *node )
      {
  long		       status = ok;
  struct node_shadow *shadow = NULL;
  struct tree_node   *shadow_node = NULL;

  IN(Generate_Shadows);
  if ( node )
    {
    if ( ( shadow = Create_Shadow( self ) ) )
      {
      ShadowedNode(shadow) = node;
      if ( ShadowRootNode )
        {
        if ( LeftNode(node) )
	  shadow_node = (ShadowTree)->CreateRightNode(  NULL,
			(long)shadow, NodeShadowNode(LeftNode(node)) );
	  else
	  if ( RightNode(node) )
	    shadow_node = (ShadowTree)->CreateLeftNode(  NULL,
			    (long)shadow, NodeShadowNode(RightNode(node)) );
	    else
	    if ( PARENTNODE(node) )
	      shadow_node = (ShadowTree)->CreateChildNode(  NULL,
			    (long)shadow, NodeShadowNode(PARENTNODE(node)) );
	      else
	      { DEBUG(Orphan Node);
	      status = failure;
	      }
        }
        else  shadow_node = (ShadowTree)->CreateRootNode(  NULL, (long)shadow );
      }
      else
      { DEBUG(ERROR -- No Space);
      status = failure;
      }
    if ( treev_debug  &&  shadow_node )
      (ShadowTree)->SetNodeName(  shadow_node, NodeName(node) );
    if ( shadow_node  &&  ChildNode(node) )
      status = Generate_Children_Shadows( self, node );
    Dump_Trees( self );
    }
  OUT(Generate_Shadows);
  return  status;
  }

static
long Generate_Children_Shadows( class treev	      *self, struct tree_node   *node )
      {
  long		      status = ok, i, current_level, node_level, start_level;
  struct node_shadow *shadow = NULL;
  struct tree_node   *shadow_node = NULL, *shadow_node_parent = NULL;

  IN(Generate_Children_Shadows);
  current_level = start_level = NODELEVEL(node);  DEBUGdt(Start Level,start_level);
  shadow_node = NodeShadowNode(node);
  while ( (node = NEXTNODE(node))  &&  (node_level = NODELEVEL(node)) > start_level )
    { DEBUGst(Node Name,NodeName(node));  DEBUGdt(Level,node_level);
    if ( ( shadow = Create_Shadow( self ) ) )
      {
      ShadowedNode(shadow) = node;
      if ( node_level == current_level )
	{ DEBUG(Same Level);
	shadow_node = (ShadowTree)->CreateRightNode(  NULL, (long)shadow, shadow_node );
	}
	else
	  if ( node_level > current_level )
	    { DEBUG(Deeper Level);
	    shadow_node_parent = shadow_node;
	    shadow_node = (ShadowTree)->CreateChildNode(  NULL,
				(long)shadow, shadow_node_parent );
	    }
	else
	  { DEBUG(Higher Level);
	  for ( i = current_level - node_level; i; i-- )
	    shadow_node_parent = shadow_node = (ShadowTree)->ParentNode(  shadow_node );
	  shadow_node = (ShadowTree)->CreateRightNode(  NULL, (long)shadow, shadow_node );
	  }
      if ( treev_debug )
	(ShadowTree)->SetNodeName(  shadow_node, NodeName(node) );
      current_level = node_level;
      }
      else
      {
      status = failure;
      break;
      }
    }
  OUT(Generate_Children_Shadows);
  return  status;
  }

static
void Destroy_Shadows( class treev	      *self, struct tree_node   *node )
      {
  struct tree_node  *shadow_node;

  IN(Destroy_Shadows);
  if ( node  &&  (shadow_node = NodeShadowNode(node)) )
    { DEBUGst(Prime Node Name,NodeName(node));
    if ( ChildNode( node ) )
      Destroy_Children_Shadows( self, node );
    if ( ShadowNodeDatum(shadow_node) )
      free( ShadowNodeDatum(shadow_node) );
    (ShadowTree)->DestroyNode(  shadow_node );
    if ( (ShadowTree )->RootNode( ) == NULL )
      MaxNameWidth = MaxPeerCount = MaxLevelCount = 0;
    }
  Dump_Trees( self );
  OUT(Destroy_Shadows);
  }

static
void Destroy_Children_Shadows( class treev	      *self, struct tree_node   *node )
      {
  struct tree_node  *shadow_node, *next;
  long		      level;

  IN(Destroy_Children_Shadows);
  if ( node )
    { DEBUGst(Prime Node Name,NodeName(node));
    shadow_node = NodeShadowNode(node);
    level = (ShadowTree)->NodeLevel(  shadow_node );
    next = NodeShadowNode(ChildNode(node));
    while ( next )
    {DEBUGst(Node Name,NodeName(ShadowedNode(ShadowNodeDatum(next))));
      if ( ShadowNodeDatum(next) )
	free( ShadowNodeDatum(next) );
      if ( (next = NextShadowNode(next))  &&
	    (ShadowTree)->NodeLevel(  next ) <= level )
	break;
      }
    (ShadowTree)->DestroyNodeChildren(  shadow_node );
    }
  Dump_Trees( self );
  OUT(Destroy_Children_Shadows);
  }

static
void Dump_Trees( class treev	      *self )
    {
  struct tree_node  *node;

  IN(Dump_Trees);
  if ( !treev_debug ) return;
  (Tree)->SetDebug(  0 );
  printf("\nNodes");
  node = ROOTNODE;
  while ( node )
    {
    printf( "\n%*s%s",(int)(2*(Tree)->NodeLevel(node)),"",
	    NodeCaptionName(node));
    node = NEXTNODE(node);
    }
  printf("\nShadows");
  node = ShadowRootNode;
  while ( node )
    {
    printf( "\n%*s%s",(int)(2*(ShadowTree)->NodeLevel(node)),"",
	    NodeCaptionName(ShadowedNode(ShadowNodeDatum(node))));
    node = NextShadowNode(node);
    }
  printf("\n");
  (Tree)->SetDebug(  1 );
  OUT(Dump_Trees);
  }

void
treev::LinkTree(class view  *parent)
{
    class treev *self=this;
    (this)->aptv::LinkTree(parent);
    if(ScrollView && parent && (this)->GetIM()) {
	(ScrollView)->LinkTree(parent);
	(ScrollView)->SetView(ScrolledView);
    }
}

void
treev::UnlinkTree()
{
    class treev *self=this;
    (this)->aptv::UnlinkTree();
    if(ScrollView) {
	(ScrollView)->UnlinkTree();
    }
}
