#ifndef _gtextv_H_
#define _gtextv_H_
/******************************************************************************
 *
 * gtextv - Gesture Text View
 * Medical Informatics 
 * Washington University, St. Louis
 * July 29, 1991
 *
 * Scott Hassan
 * Steve Cousins
 * Mark Frisse
 *
 *****************************************************************************/
/** \addtogroup libgtext
 * @{ */
/*****************************************************************************
 * 
 * gtextv.c -- The Gesture Text View Module
 *
 *****************************************************************************/

#include <textview.H>
/** A \ref textview with gesture recognition.
 *  Gestures are invoked by holding down the mouse button.  Lines are drawn
 *  as the mouse moves, which are removed on release of the mouse.  The
 *  recognized gestures invoke standard \ref textview operations.  It is
 *  not possible to change the set of recognized gestures or their
 *  actions, and the documentation for the currently recognized gestures
 *  leaves much to be desired (no arrows on those lines, for example).
 *  Since it only works when this view is being used, and changing the
 *  view is cumbersome at best, this serves as little more than a demo.
 *  It would have been better implemented as an interception of all mouse
 *  hits, similar to how Ness event handlers work.
 */
class gtextv : public  textview  {
public:
	TRACED_CLASS(gtextv);


 
   class view * Hit(enum view::MouseAction action, long x, long y, long numberOfClicks)
     ;
 
 
  
	static boolean InitializeClass();
	static boolean atk_inited;

  
	gtextv();
  

 
  int *xp, *yp;          /* Mouse Buffering Arrays */
  int index;             /* The current length of the buffers */
  int limit;             /* The current limit of the buffers */
  long parstart, parend; /* The starting and ending positions for */
                         /* the selected region. */
}; 


/** @} */
#endif /* _gtextv_H_ */
