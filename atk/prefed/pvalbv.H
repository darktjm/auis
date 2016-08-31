ATK_INTER
#ifndef _pvalbv_H_
#define _pvalbv_H_
/* Copyright 1992 by the Andrew Toolkit Consortium and Carnegie Mellon University. All rights Reserved. */

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
