ATK_INTER
#ifndef _treev_H_
#define _treev_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/config/COPYRITE'     *
\* ********************************************************************** */

/*
	$Disclaimer: 
// Permission to use, copy, modify, and distribute this software and its 
// documentation for any purpose and without fee is hereby granted, provided 
// that the above copyright notice appear in all copies and that both that 
// copyright notice and this permission notice appear in supporting 
// documentation, and that the name of IBM not be used in advertising or 
// publicity pertaining to distribution of the software without specific, 
// written prior permission. 
//                         
// THE COPYRIGHT HOLDERS DISCLAIM ALL WARRANTIES WITH REGARD 
// TO THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF 
// MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL ANY COPYRIGHT 
// HOLDER BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL 
// DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, 
// DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE 
// OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION 
// WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
// 
//  $
*/

/**  SPECIFICATION -- External Facility Suite  *********************************

TITLE	The Tree View-Class

MODULE	treev.ch

VERSION	1.0

AUTHOR	TC Peters
	Information Technology Center, Carnegie-Mellon University 

DESCRIPTION
	This is the suite of Methods that support the Tree View-Class.

    NB: The comment-symbol "===" indicates areas which are:
	    1 - Questionable
	    OR
	    2 - Arbitrary
	    OR
	    3 - Temporary Hacks
    Such curiosities need be resolved prior to Project Completion...


HISTORY
  02/14/88	Created (TCP)
  08/24/89	Upgrade to Version 1.0 (TCP)
		Pass object-type arg to Hit-handlers (TCP)

END-SPECIFICATION  ************************************************************/



#define  treev_VERSION	      1

#include <aptv.H>
 /***  Object Types  ***/
#define  treev_NodeObject		    (1)
#define  treev_TitleObject		    (2)

/***  Node Ordering  ***/
#define  treev_ColumnMajor		    (0)
#define  treev_RowMajor			    (1<<0)

/***  Node Border Styles  ***/
#define  treev_Rectangle		    (0)
#define  treev_Circle			    (1<<0)
#define  treev_Oval			    (1<<1)
#define  treev_RoundAngle		    (1<<2)
#define  treev_Roundangle		    (1<<2)
#define	 treev_Folder			    (1<<3)

/***  Tree Scrolling  ***/
#define  treev_NoVertical		    (1<<0)
#define  treev_NoHorizontal		    (1<<1)

/***  Node Filigree  ***/
#define  treev_NoDropShadow		    (0)
#define  treev_DropShadow		    (1<<0)

/***  Node Connector Styles  ***/
#define  treev_Direct			    (0)
#define  treev_DogLeg			    (1<<0)
#define  treev_Fold			    (1<<1)
#define  treev_NoFold			    (1<<2)

/****  Placements  ***/
#define  treev_Center			    (0)
#define  treev_Left			    (1<<0)
#define  treev_Right			    (1<<1)
#define  treev_Top			    (1<<2)
#define  treev_Bottom			    (1<<3)

/***  Highlight & FootPrint Styles ***/
#define  treev_Invert			    (0)
#define  treev_Border			    (1<<0)
#define  treev_Bold			    (1<<1)
#define  treev_Italic			    (1<<2)
#define  treev_Pale			    (1<<3)

/***  Arrangements ***/
#define  treev_Horizontal		    (0)
#define  treev_Vertical			    (1<<0)

/***  Attribute Specifications  ***/
#define  treev_NodeHighlightStyle(x)	    treev_nodehighlightstyle,	(long) x
#define  treev_NodeFootPrintStyle(x)	    treev_nodefootprintstyle,	(long) x
#define  treev_NodeBorderStyle(x)	    treev_nodeborderstyle,	(long) x
#define  treev_Scroll(x)		    treev_scroll,		(long) x
#define  treev_NodeConnectorStyle(x)	    treev_nodeconnectorstyle,	(long) x
#define  treev_NodeFontName(x)		    treev_nodefontname,		(long) x
#define  treev_NodeFiligree(x)		    treev_nodefiligree,	    	(long) x
#define  treev_HitHandler(x)		    treev_hithandler,		(long) x
#define  treev_Arrangement(x)		    treev_arrangement,		(long) x
#define  treev_BackgroundShade(x)	    treev_backgroundshade,	(long) x
#define  treev_NodeWidth(x)		    treev_nodewidth,		(long) x
#define  treev_NodeHeight(x)		    treev_nodeheight,		(long) x
#define  treev_Cursor(x)		    treev_cursor,		(long) x
#define  treev_CursorFontName(x)	    treev_cursorfontname,	(long) x
#define  treev_NodeOrder(x)		    treev_nodeorder,		(long) x

/***  Tree View Attribute Codes  ***/

#define  treev_nodeborderstyle		    1
#define  treev_scroll			    2
#define  treev_nodehighlightstyle	    3
#define  treev_nodeconnectorstyle	    4
#define  treev_nodefontname		    5
#define  treev_titlefontname		    6
#define  treev_nodefiligree		    7
#define  treev_hithandler		    8
#define  treev_arrangement		    9
#define  treev_backgroundshade		   10
#define  treev_nodewidth		   11
#define  treev_nodeheight		   12
#define  treev_nodefootprintstyle	   13
#define  treev_cursor			   14
#define  treev_cursorfontname		   15
#define  treev_nodeorder		   16


typedef struct treev_specification  treev_Specification;
struct  treev_specification
  {
  char	attribute;
  long	value;
  };


class treev;

typedef class view *(*treev_hitfptr)(class view *anchor, class treev *self, struct tree_node *node, int t, view_MouseAction act, long x, long y, long clicks);
typedef void (*treev_updatefptr)(class treev *self);
class treev : public  aptv
  {
public:
	virtual ATKregistryEntry *ATKregistry();



   void SetDataObject( class dataobject *data );
   view_DSattributes DesiredSize( long width, long height, enum view_DSpass pass, long *dWidth, long *dheight )
								 ;
   void FullUpdate( enum view_UpdateType type, long left, long top, long width, long height );
   void Update();
   void ObservedChanged( class observable *changed, long value );
   class view * Hit( enum view_MouseAction action, long x, long y, long n)	 ;
   void Print( FILE *file, char *processor, char *finalFormat, boolean topLevel );
   class view * GetApplicationLayer()						 ;
   char * GetInterface( char *interface_name )				 ;

   void LinkTree( class view *parent );
   void UnlinkTree();



  /**  Methods Dealing with Tree as a Whole  **/

  virtual long SetTreeAttribute( long attribute_code, long value )			 ;
  virtual long TreeAttribute( long attribute_code )				 ;
  virtual struct tree_node * CurrentNode()							 ;
  virtual void SetHitHandler( treev_hitfptr handler, class view *anchor );
  virtual void SetDebug( boolean state );

  /**  Methods Dealing with Individual Nodes  **/

  virtual void ExposeNode( struct tree_node *node );
  virtual boolean NodeExposed( struct tree_node *node )				 ;
  virtual void HideNode( struct tree_node *node );
  virtual void ExplodeNode( struct tree_node *node );
  virtual boolean NodeExploded( struct tree_node *node )			 ;
  virtual void ImplodeNode( struct tree_node *node );
  virtual void ExposeNodeChildren( struct tree_node *node );
  virtual boolean NodeChildrenExposed( struct tree_node *node )			 ;
  virtual void HideNodeChildren( struct tree_node *node );
  virtual void HighlightNode( struct tree_node *node );
  virtual boolean NodeHighlighted( struct tree_node *node )			 ;
  virtual void NormalizeNode( struct tree_node *node );
  virtual void HighlightNodeCaption( struct tree_node *node );
  virtual boolean NodeCaptionHighlighted( struct tree_node *node )		 ;
  virtual void NormalizeNodeCaption( struct tree_node *node );





  static class treev * Create(treev_Specification *treev_Spec, class view *anchor )		 ;
  
	treev();
  
	virtual ~treev();



  struct treev_instance		     *instance;
  };

#endif /* _treev_H_ */