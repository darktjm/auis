#ifndef _tree_H_
#define _tree_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

/** \addtogroup libtree Object tree
 *  A dynamically loadable ATK class file.  Contains tree, treev.
 * @{ */
/***  SPECIFICATION -- External Facility Suite  *********************************

TITLE	The Tree Data-Class

MODULE	tree.ch

VERSION	1.0

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



#define  tree_VERSION	      1

#include <apt.H>

 /***  Node Object Structure  ***/
typedef  struct tree_node	     *tree_type_node;
struct tree_node
  {
  struct tree_node		     *parent, *child, *left, *right;
  char				     *name, *caption, *title;
  long				      datum;
  long				      mode;
  boolean			      modified;
  };

/***  Ordering  ***/
#define  tree_PreOrder				    (0)
#define  tree_PostOrder				    (1<<0)

/***  Attribute Specifications  ***/
#define  tree_Order(x)	tree_order,		    (long) x

/***  Notification Codes  ***/
#define  tree_NodeCreated			 1
#define  tree_NodeDestroyed			 2
#define  tree_NodeUnhooked			 3
#define  tree_NodeHooked			 4
#define  tree_NodeNameChanged			 5
#define  tree_NodeCaptionChanged		 6
#define  tree_NodeTitleChanged			 7
#define  tree_NodeChildrenDestroyed		 8
#define  tree_NodeChildrenCreated		 9
#define  tree_NodeMoved				10
#define  tree_NodeDuplicated			11

/***  Tree Attribute Codes  ***/
#define  tree_order				 1


typedef struct tree_specification  tree_Specification;
struct  tree_specification
  {
  char	attribute;
  long	value;
  };

#include <view.H>

class tree;
typedef long (*tree_applyfptr)(char *anchor, class tree *self, tree_type_node node, char *datum );
typedef void (*tree_hitfptr) (char *anchor, class tree *self, tree_type_node node, int otype, view::MouseAction action, long x, long y, long numberOfClicks);
class tree : public  apt
  {
public:
	virtual ATKregistryEntry *ATKregistry();





  /***  Methods Dealing with Tree as a Whole  **/

  virtual long SetTreeAttribute( long attribute_code, long attribute_value );
  virtual long TreeAttribute( long attribute_code )				 ;
  virtual long TreeWidth( tree_type_node      node)							 ;
  virtual long TreeHeight( tree_type_node      node)							 ;
  virtual void SetTreeModified( boolean state );
  virtual boolean TreeModified()						 ;
  virtual void SetDebug( boolean state );

  /***  Methods Dealing with Individual Nodes  **/

  virtual struct tree_node * CreateRootNode( const char *name, long datum )			 ;
  virtual struct tree_node * CreateParentNode( const char *name, long datum, struct tree_node *child ) 
								 ;
  virtual struct tree_node * CreateChildNode( const char *name, long datum, struct tree_node *parent ) 
								 ;
  virtual struct tree_node * CreateRightNode( const char *name, long datum, struct tree_node *left ) 
								 ;
  virtual struct tree_node * CreateLeftNode( const char *name, long datum, struct tree_node *right ) 
								 ;
  virtual void DestroyDatum( void (*FreeNodeDatum)(tree_type_node node, void *user), void *user );
  virtual void DestroyNode( struct tree_node *node );
  virtual void DestroyNodeChildren( struct tree_node *node );
  virtual struct tree_node * NextNode( struct tree_node *node )				 ;
  virtual struct tree_node * PriorNode( struct tree_node *node )				 ;
  virtual struct tree_node * NodeOfName( const char *name, struct tree_node *node )		 ;
  virtual struct tree_node ** NodesOfName( const char *name, struct tree_node *node )		 ;
  virtual struct tree_node * NodeOfDatum( long datum, struct tree_node *node )		 ;
  virtual struct tree_node ** NodesOfDatum( long datum, struct tree_node *node )		 ;
  virtual struct tree_node * HookNode( struct tree_node *node, struct tree_node *parent, 
		struct tree_node *left, struct tree_node *right ) 
								 ;
  virtual struct tree_node * UnHookNode( struct tree_node *node )				 ;
  virtual struct tree_node * MoveNode( struct tree_node *node, struct tree_node *parent, 
		struct tree_node *left, struct tree_node *right )
								 ;
  virtual struct tree_node * DuplicateNode( struct tree_node *node, struct tree_node *parent, 
		struct tree_node *left, struct tree_node *right )
								 ;
  virtual long Apply( struct tree_node *node, tree_applyfptr proc, char *anchor, char *datum )
								 ;
  virtual boolean SetNodeName( struct tree_node *node, const char *name )		 ;
  virtual boolean SetNodeCaption( struct tree_node *node, const char *caption )	 ;
  virtual boolean SetNodeTitle( struct tree_node *node, const char *title )		 ;
  virtual boolean SetNodeDatum( struct tree_node *node, long datum )		 ;
  virtual long NodeCount( struct tree_node *node )				 ;
  virtual long PeerNodeCount( struct tree_node *node )			 ;
  virtual long ChildNodeCount( struct tree_node *node )			 ;
  virtual long LeafNodeCount( struct tree_node *node )			 ;
  virtual long NodeLevel( struct tree_node *node )				 ;
  virtual long NodePosition( struct tree_node *node )			 ;
  virtual char * NodeIndex( struct tree_node *node )				 ;
  virtual char * NodeAncestry( struct tree_node *node, const char *separator )	 ;
  virtual boolean NodeAncestor( struct tree_node *candidate, struct tree_node *node )  ;



  inline struct tree_node * RootNode()	    		      { return ((this)->root_node); }
  inline const char * NodeName( struct tree_node *node)		      { return ((node) ? node->name : NULL); }
  inline const char * NodeCaption( struct tree_node * node)		      { return ((node) ? node->caption : NULL); }
  inline const char * NodeTitle( struct tree_node * node)		      { return ((node) ? node->title : NULL); }
  inline long NodeDatum( struct tree_node * node)		      { return ((node) ? node->datum : 0); }
  inline struct tree_node * ParentNode( struct tree_node * node)		      { return ((node) ? node->parent : NULL); }
  inline struct tree_node * ChildNode( struct tree_node * node)		      { return ((node) ? node->child : NULL); }
  inline struct tree_node * LeftNode( struct tree_node * node)		      { return ((node) ? node->left : NULL); }
  inline struct tree_node * RightNode( struct tree_node * node)		      { return ((node) ? node->right : NULL); }

  inline void SetNotificationCode(char code)	      { ((this)->notification_code = code); }
  inline char NotificationCode()	    	      { return ((this)->notification_code); }
  inline void SetNotificationNode(struct tree_node *node)	      { ((this)->notification_node = node); }
  inline struct tree_node * NotificationNode()		      { return ((this)->notification_node); }



  static class tree * Create(const tree_Specification	         *specification, class dataobject     *anchor );
  
	tree();
  
	virtual ~tree();



  struct tree_node	  	 *root_node,
			  	 *unhooked_nodes;
  class dataobject		 *anchor;
  char				  traversal_order, tree_modified;
  char				  notification_code;
  struct tree_node		 *notification_node;
  void (*FreeNodeDatum)(tree_type_node node, void *user);
  void *fnd_user;
  };
/** @} */
#endif /* _tree_H_ */
