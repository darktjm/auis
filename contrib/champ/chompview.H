#ifndef _chompview_H_
#define _chompview_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */
/** \addtogroup libchamp
 * @{ */
#include <view.H>
/** View two months' worth of events.
 *  Two one-month calandars are displayed, along with a text area with
 *  nearby events.  Clicking in the month displays selects the day around
 *  which events are displayed.  Arrows in the month displays allow selection
 *  of different months.  This "views" the dummy \ref chomp object.
 */
class chompview: public  view {
public:
	TRACED_CLASS(chompview);

    
      
	chompview();
      
       void FullUpdate(enum view::UpdateType type, long left, long top, long width, long right);
       view::DSattributes DesiredSize(long width, long height, enum view::DSpass pass, long *dWidth, long *dheight)  ;
       void Print(FILE *file, const char *processor, const char *finalFormat, boolean topLevel);
       class view * Hit (enum view::MouseAction action, long x, long y, long numberOfClicks)  ;
       void LinkTree(class view *parent);
    
      class lpair *toplpair, *lp1;
      class textview *tv;
      class month *m1, *m2;
      class monthview *mv1, *mv2;
      class scroll *s;
};
/** @} */
#endif /* _chompview_H_ */
