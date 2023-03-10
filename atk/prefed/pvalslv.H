#ifndef _pvalslv_H_
#define _pvalslv_H_
/* Copyright 1992 by the Andrew Toolkit Consortium and Carnegie Mellon University All rights Reserved. */

/** \addtogroup libprefs
 * @{ */
#include <ssliderv.H>
class pvalslv : public  ssliderv {
public:
	TRACED_CLASS(pvalslv);


    
	pvalslv();
    

     long WhatIsAt(long num, long denom)  ;
     void Endzone(int end, enum view::MouseAction act);
     void SetFrame(long pos, long num, long denom);
     void GetInfo(struct range *total, struct range *seen, struct range *dot);



};
/** @} */
#endif /* _pvalslv_H_ */
