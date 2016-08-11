ATK_INTER
#ifndef _pvalfv_H_
#define _pvalfv_H_
/* Copyright 1992 by the Andrew Toolkit Consortium and Carnegie Mellon University. All rights Reserved. */

/* $Header: /afs/cs.cmu.edu/project/atk-src-C++/atk/prefed/RCS/pvalfv.H,v 1.5 1994/11/30 20:42:06 rr2b Stab74 $ */
/* $ACIS:eza.ch 1.4$ */
/* $Source: /afs/cs.cmu.edu/project/atk-src-C++/atk/prefed/RCS/pvalfv.H,v $ */

#include <pvalsbv.H>
class pvalfv : public  pvalsbv {
public:
	virtual ATKregistryEntry *ATKregistry();


    
	pvalfv();
    
	virtual ~pvalfv();

     void UpdateSButton();
};


#endif /* _pvalfv_H_ */
