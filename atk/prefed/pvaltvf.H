#ifndef _pvaltvf_H_
#define _pvaltvf_H_
/* Copyright 1992 by the Andrew Toolkit Consortium and Carnegie Mellon University. All rights Reserved. */

/** \addtogroup libprefs
 * @{ */
#include <pvaltvl.H>
class pvaltvf : public  pvaltvl {
public:
	TRACED_CLASS(pvaltvf);


    
	static boolean InitializeClass();

    
	pvaltvf();
    


     class keystate * Keys()  ;


    class keystate *ks;
    class menulist *menulistp;
};
/** @} */
#endif /* _pvaltvf_H_ */
