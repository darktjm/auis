#ifndef _HTMLIMAGE_H_
#define _HTMLIMAGE_H_
/** \addtogroup libhtmltext
 * @{ */
/* Copyright 1996 Carnegie Mellon University All rights reserved. */
#include <imagev.H>
class htmltextview;
class htmlimagev : public imagev {
    long x, y;
  public:
    TRACED_CLASS(htmlimagev);
    htmlimagev();
    
    view *Hit(enum view::MouseAction action,long x, long y, long numberOfClicks);
    inline void GetHitPos(long *xp, long *yp) {
	*xp=x;
	*yp=y;
    }
    virtual htmltextview *HTMLTextView();
    virtual void LinkTree(view *parent);
};
/** @} */
#endif
