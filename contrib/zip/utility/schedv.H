#ifndef _schedv_H_
#define _schedv_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

/** \addtogroup libsched
 * @{ */
/***  SPECIFICATION -- External Facility Suite  *********************************

TITLE	The Sched View-object Specification

MODULE	schedv.c

NOTICE	IBM Internal Use Only

DESCRIPTION
	This defines the suite of Methods that support the LightTable View-object.

HISTORY
  01/20/89	Created (TCP)

END-SPECIFICATION  ************************************************************/

#define  schedv_VERSION		    1

#include <zipfig.h>

#include <view.H>
class schedv : public  view {
public:
  virtual ATKregistryEntry *ATKregistry();
   void FullUpdate( enum view::UpdateType type, long left, long top, long width, long height );
   class view *Hit( enum view::MouseAction action, long x, long y, long n );
   void SetDataObject( class dataobject *data );
   void ReceiveInputFocus();
   void LoseInputFocus();
   virtual void Set_Debug( boolean debug );
   static boolean InitializeClass();
   static boolean atk_inited;
   schedv();
   virtual ~schedv();
private:
   class sched			    *data;
   class zipview		    *zipviewp;
   struct zip_pane		    *chart_pane;
   class suite			    *control_buttons;
   struct rectangle		     block, chart_block;
   class menulist		    *menu;
   class keystate		    *keystate;
   class cursor			    *cursor;
   char				     line_width, line_style, figure_type;
   boolean			     tracking, modified, input_focus, pending_question, pending_duplicate;
   struct zip_stream		    *stream;
   struct zip_image		    *image;
   struct zip_figure		    *figure, *current_slot_figure,  *current_text_figure, *previous_slot_figure, *previous_text_figure;
   long				     point;
   long				     initial_x_pixel, initial_y_pixel;
   unsigned char		     wm_type;
   friend class schedv_private; /* ick */
};
/** @} */
#endif /* _schedv_H_ */
