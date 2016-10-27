#ifndef _monthview_H_
#define _monthview_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

/** \addtogroup libchamp
 * @{ */

#include <view.H>
/** A view of a month of events.
 *  This presents a display of the days of the month, with days
 *  containing events bolded.  See \ref champ for details on
 *  how events work.  Days may be clicked to see events on that day.
 *  Clicking on the header gives an event summary for the month.  Arrows may
 *  be used to offset the month.  The \ref month data object it is meant
 *  to view stores the offset from the current month due to arrow presses.
 */
class monthview: public view {
public:
	TRACED_CLASS(monthview);

    
      
	static boolean InitializeClass();
	static boolean atk_inited;

      
	monthview();
      
    
       void FullUpdate(enum view::UpdateType type, long left, long top, long width, long right);
       void Update();
       class view * Hit (enum view::MouseAction action, long x, long y, long numberOfClicks)  ;
       view::DSattributes DesiredSize(long width, long height, enum view::DSpass pass, long *dWidth, long *dheight)  ;
       void LinkTree(class view *parent);
    
      virtual void SetTextview(class textview *tv);
      virtual void GrabTextview(boolean ForceRedraw);
      virtual void ResetMonth(boolean ForceRedraw);
    
      class textview *tv;
      int mymonth, myyear;
      int skippedatstart;
      int EventCt[31];
      int textpositions[31];
      struct tm FullTimes[31];
      class text *t;
      const char *AnnounceArray[11];
      int AnnounceArraySize;
      int pendingtop;
      int firstpendingtop;
};
/** @} */
#endif /* _monthview_H_ */
