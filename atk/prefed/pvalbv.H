#ifndef _pvalbv_H_
#define _pvalbv_H_
/* Copyright 1992 by the Andrew Toolkit Consortium and Carnegie Mellon University. All rights Reserved. */

/** \addtogroup libprefs
 * @{ */
#include <pvalsbv.H>
class pvalbv : public  pvalsbv {
public:
	TRACED_CLASS(pvalbv);


    
	pvalbv();
    

     class view * Hit (enum view::MouseAction action, long x, long y, long numberOfClicks)  ;
     void UpdateSButton();

    int activated;
};
/** @} */
#endif /* _pvalbv_H_ */
