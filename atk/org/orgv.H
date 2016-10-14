#ifndef _orgv_H_
#define _orgv_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

/** \addtogroup liborg
 * @{ */
/***  SPECIFICATION -- External Facility Suite  *********************************

TITLE	The Org View-Class

MODULE	org.ch

VERSION	1.0

AUTHOR	TC Peters
	Information Technology Center, Carnegie-Mellon University 

DESCRIPTION
	This is the suite of Methods that support the Org View-Class.

    NB: The comment-symbol "===" indicates areas which are:
	    1 - Questionable
	    OR
	    2 - Arbitrary
	    OR
	    3 - Temporary Hacks
    Such curiosities need be resolved prior to Project Completion...


HISTORY
  01/19/89	Created (TCP)
  05/24/89	Add ExposeDescription (TCP)
  05/25/89	Add Arrangement variable (TCP)
  06/06/89	Add override for Update method (TCP)
  06/07/89	Add First-time variable (TCP)
  08/23/89	Remove Create method (TCP)

END-SPECIFICATION  ************************************************************/


#define  orgv_VERSION	      1

#include <aptv.H>
class orgv;
typedef class view * (*org_hitfptr)( long anchor, class orgv *orgv, struct tree_node *node, enum view::MouseAction action, long x, long y, long clicks );
class orgv : public  aptv
  {
public:
	TRACED_CLASS(orgv);



   void SetDataObject( class dataobject *data );
   view::DSattributes DesiredSize( long width, long height, enum view::DSpass pass, long *dWidth, long *dheight )
								 ;
   void FullUpdate( enum view::UpdateType type, long left, long top, long width, long height );
   void ReceiveInputFocus();
   void LoseInputFocus();
   class view * Hit( enum view::MouseAction action, long x, long y, long n)	 ;
   void Print( FILE *file, const char *processor, const char *finalFormat, boolean topLevel );
   void ObservedChanged( class observable *changed, long value );
   void LinkTree( class view *parent );



  virtual void SetHitHandler( org_hitfptr handler, class view *anchor );
  virtual void SetDebug( boolean state );

  
	static boolean InitializeClass();

  
	orgv();
  



  class org			 *data_object;
  class lpair			 *pair_view;
  class treev			 *tree_view_object;
  class text			 *description_text;
  class textview		 *description_textview;
  class view			 *description_textview_scroll;
  struct tree_node		 *previous_node, *exploded_node;
  class suite			 *suite_object;
  class view			 *anchor;
  org_hitfptr		          hit_handler;
  struct rectangle		  bounds;
  class menulist		 *menu;
  class keystate		 *keystate;
  long				  last_modified, 
                                  initial_node_count,
				  description_last_modified;
  short				  arrangement, 
                                  node_border_style, 
                                  node_connector_style,
				  background_shade;
  boolean			  input_focus, 
                                  expose_controls, 
                                  controls_exposed,
				  fold, first_time,
				  expose_description, 
                                  description_exposed, 
                                  read_only,
				  exploded, 
                                  ignore_loseinputfocus, 
                                  ignore_fullupdate;
  };


/*
    $Log: orgv.H,v $
// Revision 1.6  1994/11/30  20:42:06  rr2b
// Start of Imakefile cleanup and pragma implementation/interface hack for g++
//
// Revision 1.5  1993/09/28  04:52:23  rr2b
// Fixed view::DSattributes to be an int not an enum.
//
 * Revision 1.4  1993/06/10  20:34:15  rr2b
 * Great atk_inited removal.
 *
 * Revision 1.3  1993/05/19  17:07:57  rr2b
 * Fixed argument to SetDataObject to be a class dataobject *.
 *
 * Revision 1.2  1993/05/18  16:49:53  rr2b
 * Converted.
 *
*Revision 1.14  1992/12/19  04:37:09  gk5g
*added LinkTree override
*.- there will be several checkins like this one.
*Many views that have children incorrectly link these children once, in InitializeObject.  These views must also provide a LinkTree because you can't assume that you are being linked into a valid viewtree (one with a path to the im), but it is assurred that once the tree is valid, the message will be distributed through the viewtree.
*.
*
*Revision 1.13  1992/12/14  23:20:33  rr2b
*add $Logs back after disclaimerization took them out
*
Revision 1.11  1992/11/11  22:39:12  gk5g
added call to lpair_LinkTree in FullUpdate... you must call LinkTree before InsertGraphic
.

Revision 1.10  1991/09/12  19:45:38  bobg
Update copyright notice

Revision 1.9  1990/03/02  18:45:04  gk5g
Removed the menu items (and suite items) Print, Save, and Quit.  These are not necessary (and, in fact, wrong) for org to have.  Frame (and buffer) handle those operations.  Removed the menu masks menu_application & menu_applicationlayer.  Removed the methods orgv_{Get,Delete}ApplicationLayer because they did nothing.  Removed the instance vars readonly, application, and org_app.  These are no longer necessary.

Revision 1.8  89/08/23  15:39:33  tom
Revise to Version 1.0

*/
/** @} */
#endif /* _orgv_H_ */
