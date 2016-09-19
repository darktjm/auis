ATK_INTER
#ifndef _bushv_H_
#define _bushv_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'     *
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

/***  SPECIFICATION -- External Facility Suite  *********************************

TITLE	The Bush View-object

MODULE	bushv.ch

VERSION	0.0

AUTHOR	TC Peters & GW Keim
	Information Technology Center, Carnegie-Mellon University 

DESCRIPTION
	This is the suite of Methods that support the Bush View-object.

    NB: The comment-symbol "===" indicates areas which are:
	    1 - Questionable
	    OR
	    2 - Arbitrary
	    OR
	    3 - Temporary Hacks
    Such curiosities need be resolved prior to Project Completion...


HISTORY
  08/21/85	Created (TCP)
  01/15/89	Convert to ATK (GW Keim)

END-SPECIFICATION  ************************************************************/


#define MAXEDITORS		    25

#include <aptv.H>
class bushv : public  aptv {
public:
	virtual ATKregistryEntry *ATKregistry();

  
    
	static boolean InitializeClass();

    
	bushv();
    
	virtual ~bushv();
    static class bushv * Create( char object )  ;
  
     void FullUpdate( enum view_UpdateType Type, long left, long top, long width, long height )  ;
     class view * Hit( enum view_MouseAction action, long x, long y, long numberOfClicks )  ;
     void PostMenus( class menulist *menulist )  ;
     void PostKeyState( class keystate *kstate )  ;
     void SetDataObject( class dataobject *bush )  ;
     class view * GetApplicationLayer()  ;
     void ReceiveInputFocus()  ;
     void LinkTree( class view *parent );

  
    class bush			*bush;
    tree_type_node		 current_node, 
				 initial_node,
	                         move_node;
    struct Dir_Entry		*current_entry;
    int				 editor_index;
    int				 num_editor_choices;
    char			 editor_program[1025];
    char			*editor_choices[MAXEDITORS];
    class suite		*control_view;
    class suite		*entries_view;
    class treev		*dir_tree_view;
    class view		        *entry_view, *entry_view_application_layer;
    class dataobject		*entry;
    long			 entry_object_modified;
    long			 entry_object_last_checkpoint;
    FILE			*entry_filep;
    int				 num_prev_selected;
    class lpair		*lp;
    int			         object;
    int				 sortmode;
    class keystate		*keyStatep;
    class keymap		*kmap;
    class cursor		*cursorp;
    class menulist		*menulistp;
    boolean			 detail;
    boolean			 top_level_inset;
    int				 debug;
};



#endif /* _bushv_H_ */
