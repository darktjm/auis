ATK_INTER
#ifndef _wrapv_H_
#define _wrapv_H_
/* Copyright 1992 by the Andrew Toolkit Consortium and Carnegie Mellon University. All rights Reserved. */

/*
	$Disclaimer: 
// Permission to use, copy, modify, and distribute this software and its 
// documentation for any purpose and without fee is hereby granted, provided 
// that the above copyright notice appear in all copies and that both that 
// copyright notice and this permission notice appear in supporting 
// documentation, and that the name of IBM not be used in advertising or 
// publicity pertaining to distribution of the software without specific, 
// written prior permission. 
//                         
// THE COPYRIGHT HOLDERS DISCLAIM ALL WARRANTIES WITH REGARD 
// TO THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF 
// MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL ANY COPYRIGHT 
// HOLDER BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL 
// DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, 
// DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE 
// OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION 
// WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
// 
//  $
*/



#include <view.H>
class wrapv : public  view {
public:
	virtual ATKregistryEntry *ATKregistry();


    
	wrapv();
    
	virtual ~wrapv();

    virtual class keystate * Keys()  ;
    virtual class menulist * Menus()  ;

    inline class view  * GetInterfaceView() { return (this->intv); }
    inline void SetInterfaceView(class view *v) { (this->intv=(class view *)(v)); }
    inline class view  *GetView() { return (this->tv); }
    inline class dataobject *GetData() { return (this->t); }
    inline void SetView(class view *v) { (this->tv=(class view *)(v)); }
    inline void SetData(class dataobject *d) { (this->t=(class dataobject *)d); }

     void LinkTree(class view *parent);
     void FullUpdate(enum view_UpdateType type, long left, long top, long width, long right);
     void Update();
     class view * Hit (enum view_MouseAction action, long x, long y, long numberOfClicks)  ;
     view_DSattributes DesiredSize(long width, long height, enum view_DSpass pass, long *dWidth, long *dheight)  ;
     void GetOrigin(long width, long height, long *originX, long *originY);
     void WantInputFocus(class view *requestor);
     void InitChildren();
     void PostMenus(class menulist *ml);
     void PostKeyState(class keystate *ks);
     void WantUpdate(class view *requestor);

    


   class dataobject *t;
   class view *tv;
   class view *intv;
};

#endif /* _wrapv_H_ */
