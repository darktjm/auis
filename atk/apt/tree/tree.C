/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

/**  SPECIFICATION -- External Facility Suite  *********************************

TITLE	The Tree Data-Class

MODULE	tree.c

VERSION	1.0

NOTICE	IBM Internal Use Only

AUTHOR	TC Peters
	Information Technology Center, Carnegie-Mellon University 

DESCRIPTION
	This is the suite of Methods that support the Tree Data-Class.

    NB: The comment-symbol "===" indicates areas which are:
	    1 - Questionable
	    OR
	    2 - Arbitrary
	    OR
	    3 - Temporary Hacks
    Such curiosities need be resolved prior to Project Completion...


HISTORY
  02/14/88	Created (TCP)
  05/19/89	Add NodeAncestry method (TCP)
  06/05/89	Add NodeAncestor method (TCP)
  08/24/89	Upgrade to Version 1.0 (TCP)
  08/31/89	Change OfData to OfDatum (TCP)

END-SPECIFICATION  ************************************************************/



#include <andrewos.h>
ATK_IMPL("tree.H")
#include <tree.H>
#include <ctype.h>

static char tree_debug = 0;
#define  RootNode		((self)->root_node)
#define  ParentNode(node)	((node)->parent)
#define  ChildNode(node)	((node)->child)
#define  LeftNode(node)		((node)->left)
#define  RightNode(node)	((node)->right)
#define  NodeName(node)	    	((node)->name)
#define  NodeCaption(node)	((node)->caption)
#define  NodeTitle(node)	((node)->title)
#define  NodeDatum(node)	((node)->datum)
#define  NodeMode(node)	    	((node)->mode)
#define  NodeModified(node)	((node)->modified)
#define  NODELEVEL(node)	((self)->NodeLevel(node))
#define  NEXTNODE(node)		((self)->NextNode(node))
#define  UnHookedNodes		(self->unhooked_nodes)
#define  TREEMODIFIED		(self->tree_modified)

#define  Anchor			(self->anchor)
#define  TraversalOrder		(self->traversal_order)


ATKdefineRegistryNoInit(tree, apt);
static tree_type_node Build_Node( class tree	      *self, const char	    	      *name, long		       datum );
static char * Ancestry( class tree	      *self, tree_type_node      node, const char		      *separator , char		      *string );


class tree *
tree::Create( const tree_Specification	         *specification, class dataobject     *anchor )
        {
  class tree		*self;

  IN(tree_Create);
  self = new tree;
  Anchor = anchor;
  while ( specification  &&  specification->attribute )
    {
    (self)->SetTreeAttribute(  specification->attribute, specification->value );
    specification++;
    }
  OUT(tree_Create);
  return self;
  }

tree::tree( )
{
    class tree *self=this;
  IN(tree_InitializeObject);
  DEBUGst(RCSID,rcsid);
  RootNode = UnHookedNodes = NULL;
  TREEMODIFIED = false;
  this->FreeNodeDatum = NULL;
  OUT(tree_InitializeObject);
  THROWONFAILURE( TRUE);
  }

tree::~tree( )
      {
    class tree *self=this;
  IN(tree_FinalizeObject );
  (this)->DestroyNode(  RootNode );
  if ( UnHookedNodes )
    {
/*===*/
    }
/*===*/
  OUT(tree_FinalizeObject );
  }

long
tree::SetTreeAttribute( long		       attribute , long		       value )
      {
    class tree *self=this;
  long		      status = ok;

  IN(tree_SetTreeAttribute);
  switch ( attribute )
    {
    case  tree_order:	DEBUG(order);
      TraversalOrder = value;
    break;

    default:/*===*/
	printf( "Tree: ERROR -- Unrecognized Attribute '%ld' (Ignored)\n", attribute );
    }
  OUT(tree_SetTreeAttribute);
  return  status;
  }

long
tree::TreeAttribute( long		       attribute )
      {
    class tree *self=this;
  long		      value = 0;

  IN(tree_TreeAttribute);
  switch ( attribute )
    {
    case  tree_order:	DEBUG(order);
      value = TraversalOrder;
    break;

    default:/*===*/
	printf( "Tree: ERROR -- Unrecognized Attribute '%ld'\n", attribute );
    }
  OUT(tree_TreeAttribute);
  return  value;
  }

static
tree_type_node
Build_Node( class tree	      *self, const char	    	      *name, long		       datum )
        {
  struct tree_node  *node;

  IN(Build_Node);
  DEBUGst(Name,name);
  if ( ( node = (struct tree_node *) calloc( 1, sizeof(struct tree_node) ) ) )
    {
    (self)->SetNodeDatum(  node, datum );
    (self)->SetNodeName(  node, name );
    }
  OUT(Build_Node);
  return  node;
  }

tree_type_node
tree::CreateRootNode( const char	    	      *name, long		       datum )
        {
    class tree *self=this;
  tree_type_node     node = NULL;

  IN(tree_CreateRootNode);
  if ( RootNode == NULL  &&  (node = Build_Node( this, name, datum )) )
    {
    RootNode = node;
    }
  OUT(tree_CreateRootNode);
  return  node;
  }

tree_type_node
tree::CreateParentNode( const char	    	      *name, long		       datum, tree_type_node      child )
{
  tree_type_node     node = NULL;

  IN(tree_CreateParentNode);
/*===*/
  OUT(tree_CreateParentNode);
  return  node;
  }

tree_type_node
tree::CreateChildNode( const char	    	      *name, long		       datum, tree_type_node      parent )
{
  tree_type_node     node = NULL,  prior = NULL;

  IN(tree_CreateChildNode);
  if ( parent  &&  (node = Build_Node( this, name, datum )) )
    {
    ParentNode(node) = parent;
    if ( ( prior = ChildNode(parent) ) )
      {
      while ( RightNode(prior) )
	prior = RightNode(prior);
      RightNode(prior) = node;
      LeftNode(node) = prior;
      }
      else  ChildNode(parent) = node;
    }
    else
    if ( parent == NULL )
      node = (this)->CreateRootNode(   name, datum );
  OUT(tree_CreateChildNode);
  return  node;
  }

tree_type_node
tree::CreateRightNode( const char	    	      *name, long		       datum, tree_type_node      left )
{
  tree_type_node     node = NULL;

  IN(tree_CreateRightNode);
  if ( left  &&  (node = Build_Node( this, name, datum )) )
    {
    ParentNode(node) = ParentNode(left);
    RightNode(node) = RightNode(left);
    RightNode(left) = node;
    LeftNode(node) = left;
    if ( RightNode(node) )
      LeftNode(RightNode(node)) = node;
    }
  OUT(tree_CreateRightNode);
  return  node;
  }

tree_type_node
tree::CreateLeftNode( const char	    	      *name, long		       datum, tree_type_node      right )
{
  tree_type_node     node = NULL;

  IN(tree_CreateLeftNode);
  if ( right  &&  (node = Build_Node( this, name, datum )) )
    {
    ParentNode(node) = ParentNode(right);
    LeftNode(node) = LeftNode(right);
    LeftNode(right) = node;
    RightNode(node) = right;
    if ( LeftNode(node) )
      RightNode(LeftNode(node)) = node;
    }
  OUT(tree_CreateLef_Node);
  return  node;
  }

void tree::DestroyDatum( void (*FreeNodeDatum)(tree_type_node node, void *user), void *user )
{
    this->FreeNodeDatum = FreeNodeDatum;
    this->fnd_user = user;
}

void
tree::DestroyNode( tree_type_node      node )
{
    class tree *self=this;
  tree_type_node     child, peer;

  IN(tree_DestroyNode);
  if ( node  ||  (node = RootNode) )
    { DEBUGst(Name,NodeName(node));
    if ( ( child = ChildNode(node) ) )
      while (child )
	{
        if ( ChildNode(child) )
          (this)->DestroyNode(  ChildNode(child) );
	peer = RightNode(child);
        (this)->DestroyNode( child );
	child = peer;
	}
    if ( LeftNode(node) )
      { DEBUG(Adjust Left);
      RightNode(LeftNode(node)) = RightNode(node);
      }
      else
      { DEBUG(Adjust Parent);
      if ( ParentNode(node) )
        ChildNode(ParentNode(node)) = RightNode(node);
      }
    if ( RightNode(node) )
      { DEBUG(Adjust Right);
      LeftNode(RightNode(node)) = LeftNode(node);
      }
    if (this->FreeNodeDatum) this->FreeNodeDatum(node, fnd_user);
    if ( NodeName(node) )	free( NodeName(node) );
    if ( NodeCaption(node) )	free( NodeCaption(node) );
    if ( NodeTitle(node) )	free( NodeTitle(node) );
    free( node );
    if ( node == RootNode )  RootNode = NULL;
    }
  OUT(tree_DestroyNode);
  }

void
tree::DestroyNodeChildren( tree_type_node      node )
{
    class tree *self=this;
  tree_type_node     child, peer;

  IN(tree_DestroyNodeChildren);
  if ( node  ||  (node = RootNode) )
    { DEBUGst(Name,NodeName(node));
    if ( ( child = ChildNode(node) ) )
      while (child )
	{
        if ( ChildNode(child) )
          (this)->DestroyNode(  ChildNode(child) );
	peer = RightNode(child);
        (this)->DestroyNode( child );
	child = peer;
	}
    }
  OUT(tree_DestroyNodeChildren);
  }

tree_type_node
tree::HookNode( tree_type_node      node , tree_type_node      parent , tree_type_node      left , tree_type_node      right )
{
    class tree *self=this;
  IN(tree_HookNode);
  if ( node  ||  (node = RootNode) )
    {
/*===*/    
    }
  OUT(tree_HookNode);
  return  node;
  }

tree_type_node
tree::UnHookNode( tree_type_node      node )
{
    class tree *self=this;
  IN(tree_UnhookNode);
  if ( node  ||  (node = RootNode) )
    {
/*===*/    
    }
  OUT(tree_UnhookNode);
  return  node;
  }

tree_type_node
tree::MoveNode( tree_type_node      node , tree_type_node      parent , tree_type_node      left , tree_type_node      right )
{
    class tree *self=this;
  IN(tree_MoveNode);
  if ( node  ||  (node = RootNode) )
    {
/*===*/    
    }
  OUT(tree_MoveNode);
  return  node;
  }

tree_type_node
tree::DuplicateNode( tree_type_node      node , tree_type_node      parent , tree_type_node      left , tree_type_node      right )
{
    class tree *self=this;
  IN(tree_DuplicateNode);
  if ( node  ||  (node = RootNode) )
    {
/*===*/    
    }
  OUT(tree_DuplicateNode);
  return  node;
  }

tree_type_node
tree::NodeOfName( const char		      *name, tree_type_node      node )
{
    class tree *self=this;
  tree_type_node     candidate = NULL;
  long		      level;

  IN(tree_NodeOfName);
  if ( node  ||  (node = RootNode) )
    {
    level = NODELEVEL(node);
    while ( node  &&  NODELEVEL(node) >= level )
      {
      if ( strcmp( NodeName(node), name) == 0 )
	{
	candidate = node;
	break;
	}
      node = NEXTNODE(node);
      }
    }
  OUT(tree_NodeOfName);
  return  candidate;
  }

tree_type_node
tree::NodeOfDatum( long		       datum, tree_type_node      node )
{
    class tree *self=this;
  tree_type_node     candidate = NULL;
  long		      level;

  IN(tree_NodeOfDatum);
  if ( node  ||  (node = RootNode) )
    {
    level = NODELEVEL(node);
    while ( node  &&  NODELEVEL(node) >= level )
      {
      if ( NodeDatum(node) == datum )
	{
	candidate = node;
	break;
	}
      node = NEXTNODE(node);
      }
    }
  OUT(tree_NodeOfDatum);
  return  candidate;
  }

tree_type_node *
tree::NodesOfName( const char		      *name, tree_type_node      node )
{
    class tree *self=this;
  tree_type_node    *candidates = NULL;
  long		      level, count = 0;

  IN(tree_NodesOfName);
  if ( node  ||  (node = RootNode) )
    {
    level = NODELEVEL(node);
    while ( node  &&  NODELEVEL(node) >= level )
      {
      if ( strcmp( NodeName(node), name) == 0 )
	{
	if ( candidates )
	  candidates = (tree_type_node *)
	    realloc( candidates, (++count +1)  * sizeof(tree_type_node) );
	  else
	  candidates = (tree_type_node *)
	    malloc( (++count + 1) * sizeof(tree_type_node) );
	candidates[count-1] = node;
	candidates[count] = NULL;
	}
      node = NEXTNODE(node);
      }
    }
  OUT(tree_NodesOfName);
  return  candidates;
  }

tree_type_node *
tree::NodesOfDatum( long		       datum, tree_type_node      node )
{
    class tree *self=this;
  tree_type_node    *candidates = NULL;
  long		      level, count = 0;

  IN(tree_NodesOfDatum);
  if ( node  ||  (node = RootNode) )
    {
    level = NODELEVEL(node);
    while ( node  &&  NODELEVEL(node) >= level )
      {
      if ( NodeDatum(node) == datum )
	{
	if ( candidates )
	  candidates = (tree_type_node *)
	    realloc( candidates, (++count +1)  * sizeof(tree_type_node) );
	  else
	  candidates = (tree_type_node *)
	    malloc( (++count + 1) * sizeof(tree_type_node) );
	candidates[count-1] = node;
	candidates[count] = NULL;
	}
      node = NEXTNODE(node);
      }
    }
  OUT(tree_NodesOfDatum);
  return  candidates;
  }

long
tree::TreeWidth( tree_type_node      node)
{
  long		      width = 0;

  IN(tree_TreeWidth);
/*===*/
  OUT(tree_TreeWidth);
  return  width;
  }

long
tree::TreeHeight( tree_type_node      node)
{
    class tree *self=this;
  long		      height = 0, level, l;

  IN(tree_TreeHeight);
  if ( node  ||  (node = RootNode) )
    {
    level = NODELEVEL(node);
    while ( node  &&  (l = NODELEVEL(node)) >= level )
      {
      level = l;
      if ( level > height )
	height = level;
      node = NEXTNODE(node);
      }
    }
  OUT(tree_TreeHeight);
  return  height;
  }

long
tree::NodeLevel(tree_type_node      node )
{
    class tree *self=this;
  long		      level = 1;
  tree_type_node     parent = node;

  IN(tree_NodeLevel);
  if ( node  ||  (node = parent = RootNode) )
    while ( ( parent = ParentNode(parent) ) )
      level++;
  OUT(tree_NodeLevel);
  return  level;
  }

long
tree::NodePosition( tree_type_node      node )
{
    class tree *self=this;
  long		      position = 0;
  tree_type_node     peer = node;

  IN(tree_NodePosition);
  if ( node  ||  (node = peer = RootNode) )
    {
    position = 1;
    while ( ( peer = LeftNode(peer) ) )
      position++;
    }
  DEBUGdt(Position,position);
  OUT(tree_NodePosition);
  return  position;
  }

char *
tree::NodeIndex( tree_type_node      node )
{
    class tree *self=this;
  tree_type_node     parent = node;
  static char		      string[1024];
  char			      temp[1024];
  long		      length;

  IN(tree_NodeIndex);
  *string = 0;
  if ( node  ||  (node = parent = RootNode) )
    {
    while ( parent  &&  (parent != RootNode) )
      {
      if ( strlen( string ) > (sizeof(string) - 20) )
	{
	strcat( string, "*TOO LONG*." );
	break;
	}
      strcpy( temp, string );
      sprintf( string, "%ld.%s", (this)->NodePosition(  parent ), temp );
      parent = ParentNode(parent);
      }
    if ( ( length = strlen( string ) ) )
      string[length-1] = 0;
      else
      strcpy( string, "0" );
    }
  DEBUGst(Index,string);
  OUT(tree_NodeIndex);
  return  string;
  }

static char *
Ancestry( class tree	      *self, tree_type_node      node, const char		      *separator , char		      *string )
        {
  IN(Ancestry);
  if ( ParentNode(node) )
    string = Ancestry( self, ParentNode(node), separator, string );
  string = (char *) realloc( string, strlen( string ) +
		strlen( NodeName(node) ) + strlen( separator ) + 2 );
  if ( *string )
    strcat( string, separator );
  strcat( string, NodeName(node) );
  IN(Ancestry);
  return  string;
  }

char *
tree::NodeAncestry( tree_type_node      node, const char		      *separator )
{
  char		     *ancestry = NULL;

  IN(tree_NodeAncestry);
  if ( node  &&  ParentNode(node)  &&
      (ancestry = (char *) calloc( 1, 2 )) )
    ancestry = Ancestry( this, ParentNode(node), separator, ancestry );
  DEBUGst(Ancestry,ancestry);
  OUT(tree_NodeAncestry);
  return  ancestry;
  }

boolean
tree::NodeAncestor( tree_type_node      candidate , tree_type_node      node )
{
  boolean	      ancestor = false;

  IN(tree_NodeAncestor);
  if ( candidate  &&  node )
    while ( ( node = ParentNode(node) ) )
      if ( candidate == node )
	{
	ancestor = true;
	break;
	}
  DEBUGdt(ancestor,ancestor);
  OUT(tree_NodeAncestor);
  return  ancestor;
  }

long
tree::Apply( tree_type_node      node, tree_applyfptr proc, char		      *anchor, char		      *datum )
{
    class tree *self=this;
  long		      level, result = 0;

  IN(tree_Apply);
  if ( node  ||  (node = RootNode) )
    {
    level = NODELEVEL(node);
    while ( result == 0  &&  node  &&  NODELEVEL(node) >= level )
      {
      result = (proc)( anchor, this, node, datum );
      node = NEXTNODE(node);
      }
    }
  OUT(tree_Apply);
  return  result;
  }

tree_type_node
tree::NextNode(tree_type_node      node )
{
    class tree *self=this;
  tree_type_node     next = NULL, parent;

  IN(tree_NextNode);
  if ( node )
    {
    if ( ! (next = node->child) )
      if ( ! (next = node->right) )
	if ( ( parent = node->parent ) )
	  while ( parent )
	    if ( ( next = parent->right ) )
	      break;
	    else
	    parent = parent->parent;
    }
    else  next = RootNode;
  OUT(tree_NextNode);
  return  next;
  }

tree_type_node
tree::PriorNode( tree_type_node      node )
{
  tree_type_node     prior = NULL;

  IN(tree_PriorNode);
/*===*/
  OUT(tree_PriorNode);
  return  prior;
  }

#if 0 /* unused (probably ought to be class methods, eh? */
static void
tree__SetNodeModified( class tree 	      *self, tree_type_node      node, char		       state )
        {
  IN(tree_SetNodeModified);
  if ( node  ||  (node = RootNode) )
    NodeModified(node) = TREEMODIFIED = state;
  OUT(tree_SetNodeModified);
  }
static boolean
tree__NodeModified( class tree	      *self, tree_type_node      node )
      {
  IN(tree_NodeModified);
  OUT(tree_NodeModified);
  return  (node) ? NodeModified(node) : false;
  }
#endif
void
tree::SetTreeModified( boolean		       state )
{
    class tree *self=this;
  IN(tree_SetTreeModified);
  TREEMODIFIED = state;
  OUT(tree_SetTreeModified);
  }

boolean
tree::TreeModified( )
{
    class tree *self=this;
  IN(tree_TreeModified);
  OUT(tree_TreeModified);
  return  TREEMODIFIED;
  }

boolean
tree::SetNodeName( tree_type_node        node, const char		        *name )
{
  long		        status = ok;

  IN(tree_SetNodeName);
  if ( node )
    {
    if ( NodeName(node) )   free( NodeName(node) );
    NodeName(node) = NULL;
    if ( name  &&  *name )
      {
      if ( ! (NodeName(node) = strdup(name)) )
	status = failure;
      }
    }
  OUT(tree_SetNodeName);
  return  status;
  }

boolean
tree::SetNodeCaption( tree_type_node          node, const char			  *caption )
{
  IN(tree_SetNodeCaption);
  if ( node )
    {
    if ( NodeCaption(node) )   free( NodeCaption(node) );
    NodeCaption(node) = NULL;
    if ( caption  &&  *caption )
      NodeCaption(node) = strdup( caption );
    }
  OUT(tree_SetNodeCaption);
  return  TRUE;
  }

boolean
tree::SetNodeTitle( tree_type_node         node, const char			 *title )
{
  IN(tree_SetNodeTitle);
  if ( node )
    {
    if ( NodeTitle(node) )   free( NodeTitle(node) );
    NodeTitle(node) = NULL;
    if ( title  &&  *title )
      NodeTitle(node) = strdup( title );
    }
  OUT(tree_SetNodeTitle);
  return  TRUE;
  }

boolean
tree::SetNodeDatum( tree_type_node        node, long		         datum )
{
  IN(tree_SetNodeDatum);
  if ( node )
    NodeDatum(node) = datum;
  OUT(tree_SetNodeDatum);
  return  TRUE;
  }

long
tree::NodeCount( tree_type_node      node )
{
    class tree *self=this;
  long		      count = 0, level;

  IN(tree_NodeCount);
  if ( node  ||  (node = RootNode) )
    {
    level = NODELEVEL(node);
    while ( node  &&  NODELEVEL(node) >= level )
      {
      count++;
      node = NEXTNODE(node);
      }
    }
  OUT(tree_NodeCount);
  return  count;
  }

long
tree::PeerNodeCount( tree_type_node      node )
{
  long		      count = 0;
  tree_type_node     peer;

  IN(tree_PeerNodeCount);
  if ( node  &&  ParentNode(node) )
    {
    peer = ChildNode(ParentNode(node));
    while ( ( peer = RightNode(peer) ) )
      count++;
    }
  OUT(tree_PeerNodeCount);
  return  count;
  }


long
tree::ChildNodeCount( tree_type_node      node )
{
  long		      count = 0;

  IN(tree_ChildNodeCount);
  if ( node  &&  ChildNode(node) )
    count = 1 + (this)->PeerNodeCount(  ChildNode(node) );
  OUT(tree_ChildNodeCount);
  return  count;
  }

long
tree::LeafNodeCount( tree_type_node      node )
{
    class tree *self=this;
  long		      count = 0, level;

  IN(tree_LeafNodeCount);
  if ( node  ||  (node = RootNode) )
    {
    level = NODELEVEL(node);
    while ( node  &&  NODELEVEL(node) >= level )
      {
      if ( ChildNode(node) == NULL )
        count++;
      node = NEXTNODE(node);
      }
    }
  OUT(tree_LeafNodeCount);
  return  count;
  }

void
tree::SetDebug( boolean		        state )
{
  IN(tree_SetDebug);
  tree_debug = state;
  OUT(tree_SetDebug);
  }
