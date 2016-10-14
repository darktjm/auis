#ifndef _prefsbv_H_
#define _prefsbv_H_
/* Copyright 1992 by the Andrew Toolkit Consortium and Carnegie Mellon University All rights Reserved. */

/** \addtogroup libprefs
 * @{ */
#include <sbuttonv.H>
class prefsbv: public  sbuttonv {
public:
	TRACED_CLASS(prefsbv);

  
    
	prefsbv();
    
  
     boolean Touch(int ind, enum view::MouseAction act)  ;
 };
/** @} */
#endif /* _prefsbv_H_ */
