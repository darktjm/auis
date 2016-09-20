ATK_INTER
#ifndef _panner_H_
#define _panner_H_
/* panner.ch - panner box view */

/*
	Copyright Carnegie Mellon University 1992 - All rights reserved
*/

/** \addtogroup libsupviews
 * @{ */

#include <rect.h>

#define scroll_DRAGGING (-4)
#define panner_NumberOfCursors (4)

#include <scroll.H>

class panner : public  scroll {
public:
	virtual ATKregistryEntry *ATKregistry();


    
      
	static boolean InitializeClass();

      static class panner * Create(class view *view)  ;
      
	panner();
      
	virtual ~panner();

    
       void SetLocation(int location);
       void FullUpdate(enum view_UpdateType type, long left, long top, long width, long right);
       void Update();
       class view * Hit(enum view_MouseAction action, long x, long y, long numberOfClicks)  ;
       void PostCursor(struct rectangle *rec, class cursor *cursor);

    
      boolean MotifStyle;
      int PannerShade;
      struct rectangle panrect;
      boolean visible;
      struct rectangle oldpanrect;
      boolean oldvisible;
      boolean panrectchanged;
      class region *pannerreg, *childclip;
      long idealwidth, idealheight;
      struct rectangle seenrect, dotrect, gseenrect;
      long rockx, rocky, isatx, isaty;
      class cursor *cursors[panner_NumberOfCursors];
};
/** @} */
#endif /* _panner_H_ */
