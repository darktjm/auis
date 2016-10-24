#ifndef _circlepiview_H_
#define _circlepiview_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1991 - All Rights Reserved           *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */
/** \addtogroup libcirclepi
 * @{ */
#include <view.H>
/** A demonstration of ATK graphics.
 *  This view visibly demonstrates an algorithm for the estimation of pi.
 *  It uses \ref circlepi to store its depth preference, which can be set
 *  by key or menu.  Otherwise, all work is done in the view.
 */
class circlepiview: public  view {
public:
	TRACED_CLASS(circlepiview);

  
    
	static boolean InitializeClass();
	static boolean atk_inited;

    
	circlepiview();
    
  	
     void ObservedChanged (class observable *changed, long value);
     void LinkTree(class view *parent);
     void FullUpdate(enum view::UpdateType type, long left, long top, long width, long height);
     void Update();
     class view * Hit (enum view::MouseAction action, long x, long y, long numberOfClicks)  ;
     void PostMenus(class menulist *ml);
  
    int need_full_update;
    class cursor *cursor;
    int color;
    class menulist *ml;
};

/** @} */
#endif /* _circlepiview_H_ */
