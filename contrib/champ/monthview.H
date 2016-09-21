#ifndef _monthview_H_
#define _monthview_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

/** \addtogroup libchamp
 * @{ */

#include <view.H>
class monthview: public view {
public:
	virtual ATKregistryEntry *ATKregistry();

    
      
	static boolean InitializeClass();
	static boolean atk_inited;

      
	monthview();
      
	virtual ~monthview();
    
       void FullUpdate(enum view_UpdateType type, long left, long top, long width, long right);
       void Update();
       class view * Hit (enum view_MouseAction action, long x, long y, long numberOfClicks)  ;
       view_DSattributes DesiredSize(long width, long height, enum view_DSpass pass, long *dWidth, long *dheight)  ;
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
