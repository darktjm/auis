#ifndef _calcv_H_
#define _calcv_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */
/** \addtogroup libcalc
 * @{ */

/*  $Log: calcv.H,v $
 * Revision 1.1  1993/05/29  15:33:56  rr2b
 * Initial revision
 *
*Revision 1.2  1992/12/14  21:44:00  rr2b
*fixed disclaimerization
*.
*
* Revision 1.1  1992/10/05  20:07:19  susan
* Initial revision
*
Revision 1.4  1991/09/12  19:25:33  bobg
Update copyright notice

Revision 1.3  1989/10/13  18:12:05  gk5g
Fix for Bug#64.  Calc captions not appearing on Sun3.  The problem was that the placement value for each caption was being stored in a char.  The possible range of values for the placement (from graphic.ch) is 0 - 0200.  The stored placement value was being clipped and then compared against a literal constant that exceeded that constant.  All captions were being drawn at x = 0 and were not visible because the ClipRect was being set.

Revision 1.2  89/08/04  17:09:35  tom
Accept keyboard inputs;
Suppress Shrink & Help Icons.

Revision 1.1  89/05/10  20:58:01  tom
Initial revision


*/

/***  SPECIFICATION -- External Facility Suite  *********************************

TITLE	The Calcv Data-object

MODULE	calcv.ch

VERSION	1.0

AUTHOR	TC Peters
	Information Technology Center, Carnegie-Mellon University 

DESCRIPTION
	This is the suite of Methods that suport the Calcv Data-object.

HISTORY
  02/23/88	Created (TCP)

END-SPECIFICATION  ************************************************************/

#define  calcv_VERSION		    1

#include <aptv.H>
struct calcv_states
  {
  unsigned int		   inputfocus		    : 1;
  unsigned int		   pending_display	    : 1;
  unsigned int		   pending_outline	    : 1;
  unsigned int		   point_present	    : 1;
  };

struct calcv_area_states
  {
  unsigned int		   highlighted		    : 1;
  };

class calcv;
typedef void (*calcv_hitfptr)(class calcv *self, long area);
struct calcv_area
  {
  const char			     *string;
  class fontdesc		     *font;
  struct rectangle		      bounds;
  char				      shape;
  int				      mode;
  calcv_hitfptr			      hit_handler;
  const struct calcv_setup	     *spec;
  struct calcv_area_states	      states;
  };


/** View for a simple calculator.
 *  This is a 4-function calculator with hard-coded key bindings.  The
 *  display assumes the size of text in pixels while specifying them in
 *  points, so there is a lot of overwriting at more than 75dpi.
 *  The value displayed is the only thing remembered; use this with a
 *  \ref calc data object to save it.  It claims the ability to print,
 *  but that ability is limited: it only supports troff printing, and it
 *  refuses to print from \b ezprint.  Otherwise, the printout looks
 *  just like the view, except the fonts don't overlap the boundaries.
 */
class calcv : public  aptv
  {
public:
	TRACED_CLASS(calcv);



   void SetDataObject( class dataobject *calc );
   void FullUpdate( enum view::UpdateType type, long left, long top, long width, long height );
   void Update();
   void ObservedChanged( class observable *changed, long value );
   class view * Hit( enum view::MouseAction action, long x, long y, long n )	 ;
   void Print( FILE *file, const char *processor, const char *finalFormat, boolean topLevel );
   void ReceiveInputFocus();
   void LoseInputFocus();





  
	static boolean InitializeClass();
	static boolean atk_inited;

  
	calcv();
  



  struct calcv_states	  states;
  class keystate	 *keystate;
  long			  area_count;
  struct calcv_area	  areas[20];
  struct rectangle	  bounds;
  char			  expression[50];
  char			  prior_expression[50];
  char			  operand_1[25], operand_2[25];
  char			  pending_op;
  };
/** @} */
#endif /* _calcv_H_ */
