ATK_INTER
#ifndef _pvaltvf_H_
#define _pvaltvf_H_
/* Copyright 1992 by the Andrew Toolkit Consortium and Carnegie Mellon University. All rights Reserved. */

#include <pvaltvl.H>
class pvaltvf : public  pvaltvl {
public:
	virtual ATKregistryEntry *ATKregistry();


    
	static boolean InitializeClass();

    
	pvaltvf();
    
	virtual ~pvaltvf();


     class keystate * Keys()  ;


    class keystate *ks;
    class menulist *menulistp;
};

#endif /* _pvaltvf_H_ */
