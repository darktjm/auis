#ifndef _POPUPWIN_H_
#define _POPUPWIN_H_
ATK_INTER
/* Copyright 1995 Carnegie Mellon University All rights reserved.
  $Disclaimer: 
 * Permission to use, copy, modify, and distribute this software and its 
 * documentation for any purpose and without fee is hereby granted, provided 
 * that the above copyright notice appear in all copies and that both that 
 * copyright notice and this permission notice appear in supporting 
 * documentation, and that the name of IBM not be used in advertising or 
 * publicity pertaining to distribution of the software without specific, 
 * written prior permission. 
 *                         
 * THE COPYRIGHT HOLDERS DISCLAIM ALL WARRANTIES WITH REGARD 
 * TO THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF 
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL ANY COPYRIGHT 
 * HOLDER BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL 
 * DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, 
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE 
 * OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION 
 * WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 * 
 *  $
 */
#include <view.H>
#include <keystate.H>

class popupwin : public view {
  public:
    virtual ATKregistryEntry *ATKregistry();
    popupwin();
    ~popupwin();
    view *Hit(view_MouseAction act, long x, long y, long numclicks);
    
    static popupwin *Create(im *original, view *popup);
    virtual void SetView(view *popup);
    virtual void Post(view *origin, long mousex, long mousey, view_MouseAction act=view_LeftDown, long viewx=0, long viewy=0, long w=0, long h=0);
    virtual void Retract();


    void LinkTree(class view *parent);
    void FullUpdate(enum view_UpdateType type, long left, long top, long width, long right);
    void Update();
    view_DSattributes DesiredSize(long width, long height, enum view_DSpass pass, long *dWidth, long *dheight)  ;
    void GetOrigin(long width, long height, long *originX, long *originY);
    void WantInputFocus(class view *requestor);
    void InitChildren();
    void PostMenus(class menulist *ml);
    void PostKeyState(class keystate *ks);
    void WantUpdate(class view *requestor);

    keystate *lks;
    view *v;
};

#endif
