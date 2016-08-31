ATK_INTER
#ifndef _pvalcv_H_
#define _pvalcv_H_
/* Copyright 1992 by the Andrew Toolkit Consortium and Carnegie Mellon University. All rights Reserved. */

#include <pvalsbv.H>
class pvalcv : public  pvalsbv {
public:
	virtual ATKregistryEntry *ATKregistry();


    
	pvalcv();
    
	virtual ~pvalcv();

     void UpdateSButton();
};


#endif /* _pvalcv_H_ */
