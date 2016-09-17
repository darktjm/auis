#ifndef _schedv_H_
#define _schedv_H_
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


/**  SPECIFICATION -- External Facility Suite  *********************************

TITLE	The Sched View-object Specification

MODULE	schedv.c

NOTICE	IBM Internal Use Only

DESCRIPTION
	This defines the suite of Methods that support the LightTable View-object.

HISTORY
  01/20/89	Created (TCP)

END-SPECIFICATION  ************************************************************/

#define  schedv_VERSION		    1


#include <view.H>
class schedv : public  view {
public:
  virtual ATKregistryEntry *ATKregistry();
   void FullUpdate( enum view_UpdateType type, long left, long top, long width, long height );
   class view *Hit( enum view_MouseAction action, long x, long y, long n );
   void SetDataObject( class dataobject *data );
   void ReceiveInputFocus();
   void LoseInputFocus();
   virtual void Set_Debug( boolean debug );
   static boolean InitializeClass();
   static boolean atk_inited;
   schedv();
   virtual ~schedv();

   class sched			    *data;
   class zipview		    *zipviewp;
   struct zip_pane		    *chart_pane;
   class suite			    *control_buttons;
   struct rectangle		     block, chart_block;
   class menulist		    *menu;
   class cursor			    *cursor;
   char				     line_width, line_style, figure_type;
   boolean			     tracking, modified, input_focus, pending_question, pending_duplicate;
   struct zip_stream		    *stream;
   struct zip_image		    *image;
   struct zip_figure		    *figure, *current_slot_figure,  *current_text_figure, *previous_slot_figure, *previous_text_figure;
   long				     point;
   long				     initial_x_pixel, initial_y_pixel;
   unsigned char		     wm_type;
};

#endif /* _schedv_H_ */
