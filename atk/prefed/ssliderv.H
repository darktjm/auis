#ifndef _ssliderv_H_
#define _ssliderv_H_
/* Copyright 1992 by the Andrew Toolkit Consortium and Carnegie Mellon University All rights Reserved. */

/** \addtogroup libprefs
 * @{ */
#include <scroll.H>
class ssliderv : public  scroll {
public:
	TRACED_CLASS(ssliderv);


    
	ssliderv();
    

    virtual long WhatIsAt(long num, long denom)  ;
    virtual void Endzone(int end, enum view::MouseAction act);
    virtual void SetFrame(long pos, long num, long denom);
    virtual void GetInfo(struct range *total, struct range *seen, struct range *dot);

     view::DSattributes DesiredSize(long width, long height, enum view::DSpass pass, long *dWidth, long *dheight)  ;
     void GetOrigin(long width, long height, long *originX, long *originY);

     const void * GetInterface(const char *name)  ;
     void FullUpdate(enum view::UpdateType type, long left, long top, long width, long right);



};
/** @} */
#endif /* _ssliderv_H_ */
