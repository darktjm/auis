ATK_INTER
#ifndef _wrapv_H_
#define _wrapv_H_
/* Copyright 1992 by the Andrew Toolkit Consortium and Carnegie Mellon University. All rights Reserved. */

/** \addtogroup libprefs
 * @{ */
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
/** @} */
#endif /* _wrapv_H_ */
