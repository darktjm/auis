ATK_INTER
#ifndef _pvalslv_H_
#define _pvalslv_H_
/* Copyright 1992 by the Andrew Toolkit Consortium and Carnegie Mellon University All rights Reserved. */

/* $Header: /afs/cs.cmu.edu/project/atk-src-C++/atk/prefed/RCS/pvalslv.H,v 1.5 1994/11/30 20:42:06 rr2b Stab74 $ */
/* $ACIS:eza.ch 1.4$ */
/* $Source: /afs/cs.cmu.edu/project/atk-src-C++/atk/prefed/RCS/pvalslv.H,v $ */

#include <ssliderv.H>
class pvalslv : public  ssliderv {
public:
	virtual ATKregistryEntry *ATKregistry();


    
	pvalslv();
    
	virtual ~pvalslv();

     long WhatIsAt(long num, long denom)  ;
     void Endzone(int end, enum view_MouseAction act);
     void SetFrame(long pos, long num, long denom);
     void GetInfo(struct range *total, struct range *seen, struct range *dot);



};

#endif /* _pvalslv_H_ */
