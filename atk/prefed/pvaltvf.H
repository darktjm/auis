ATK_INTER
#ifndef _pvaltvf_H_
#define _pvaltvf_H_
/* Copyright 1992 by the Andrew Toolkit Consortium and Carnegie Mellon University. All rights Reserved. */

/* $Header: /afs/cs.cmu.edu/project/atk-src-C++/atk/prefed/RCS/pvaltvf.H,v 1.6 1994/11/30 20:42:06 rr2b Stab74 $ */
/* $ACIS:eza.ch 1.4$ */
/* $Source: /afs/cs.cmu.edu/project/atk-src-C++/atk/prefed/RCS/pvaltvf.H,v $ */

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
