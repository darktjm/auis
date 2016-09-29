#ifndef _pvalfv_H_
#define _pvalfv_H_
/* Copyright 1992 by the Andrew Toolkit Consortium and Carnegie Mellon University. All rights Reserved. */

/** \addtogroup libprefs
 * @{ */
#include <pvalsbv.H>
class pvalfv : public  pvalsbv {
public:
	virtual ATKregistryEntry *ATKregistry();


    
	pvalfv();
    
	virtual ~pvalfv();

     void UpdateSButton();
};
/** @} */
#endif /* _pvalfv_H_ */
