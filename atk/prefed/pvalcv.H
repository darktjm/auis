ATK_INTER
#ifndef _pvalcv_H_
#define _pvalcv_H_
/* Copyright 1992 by the Andrew Toolkit Consortium and Carnegie Mellon University. All rights Reserved. */

/* $Header: /afs/cs.cmu.edu/project/atk-src-C++/atk/prefed/RCS/pvalcv.H,v 1.5 1994/11/30 20:42:06 rr2b Stab74 $ */
/* $ACIS:eza.ch 1.4$ */
/* $Source: /afs/cs.cmu.edu/project/atk-src-C++/atk/prefed/RCS/pvalcv.H,v $ */

#include <pvalsbv.H>
class pvalcv : public  pvalsbv {
public:
	virtual ATKregistryEntry *ATKregistry();


    
	pvalcv();
    
	virtual ~pvalcv();

     void UpdateSButton();
};


#endif /* _pvalcv_H_ */
