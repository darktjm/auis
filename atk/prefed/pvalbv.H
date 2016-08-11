ATK_INTER
#ifndef _pvalbv_H_
#define _pvalbv_H_
/* Copyright 1992 by the Andrew Toolkit Consortium and Carnegie Mellon University. All rights Reserved. */

/* $Header: /afs/cs.cmu.edu/project/atk-src-C++/atk/prefed/RCS/pvalbv.H,v 1.5 1994/11/30 20:42:06 rr2b Stab74 $ */
/* $ACIS:eza.ch 1.4$ */
/* $Source: /afs/cs.cmu.edu/project/atk-src-C++/atk/prefed/RCS/pvalbv.H,v $ */

#include <pvalsbv.H>
class pvalbv : public  pvalsbv {
public:
	virtual ATKregistryEntry *ATKregistry();


    
	pvalbv();
    
	virtual ~pvalbv();

     class view * Hit (enum view_MouseAction action, long x, long y, long numberOfClicks)  ;
     void UpdateSButton();

    int activated;
};


#endif /* _pvalbv_H_ */
