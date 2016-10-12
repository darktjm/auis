#ifndef _POPUPWIN_H_
#define _POPUPWIN_H_
/* Copyright 1995 Carnegie Mellon University All rights reserved. */
/** \addtogroup libsupviews
 * @{ */
#include <view.H>
#include <keystate.H>

class popupwin : public view {
  public:
    virtual ATKregistryEntry *ATKregistry();
    popupwin();
    ~popupwin();
    view *Hit(view::MouseAction act, long x, long y, long numclicks);
    
    static popupwin *Create(im *original, view *popup);
    virtual void SetView(view *popup);
    virtual void Post(view *origin, long mousex, long mousey, view::MouseAction act=view::LeftDown, long viewx=0, long viewy=0, long w=0, long h=0);
    virtual void Retract();


    void LinkTree(class view *parent);
    void FullUpdate(enum view::UpdateType type, long left, long top, long width, long right);
    void Update();
    view::DSattributes DesiredSize(long width, long height, enum view::DSpass pass, long *dWidth, long *dheight)  ;
    void GetOrigin(long width, long height, long *originX, long *originY);
    void WantInputFocus(class view *requestor);
    void InitChildren();
    void PostMenus(class menulist *ml);
    void PostKeyState(class keystate *ks);
    void WantUpdate(class view *requestor);

    keystate *lks;
    view *v;
};
/** @} */
#endif
