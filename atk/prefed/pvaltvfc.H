ATK_INTER
#ifndef _pvaltvfc_H_
#define _pvaltvfc_H_
/* Copyright 1992 by the Andrew Toolkit Consortium and Carnegie Mellon University. All rights Reserved. */
/* $Header: /afs/cs.cmu.edu/project/atk-src-C++/atk/prefed/RCS/pvaltvfc.H,v 1.5 1994/11/30 20:42:06 rr2b Stab74 $ */
/* $ACIS:eza.ch 1.4$ */
/* $Source: /afs/cs.cmu.edu/project/atk-src-C++/atk/prefed/RCS/pvaltvfc.H,v $ */

#include <pvaltvc.H>
class pvaltvfc : public  pvaltvc {
public:
	virtual ATKregistryEntry *ATKregistry();


    
	static boolean InitializeClass();

    
	pvaltvfc();
    
	virtual ~pvaltvfc();


     void Select(int ind);


};

#endif /* _pvaltvfc_H_ */
