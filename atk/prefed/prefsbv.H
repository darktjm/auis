ATK_INTER
#ifndef _prefsbv_H_
#define _prefsbv_H_
/* Copyright 1992 by the Andrew Toolkit Consortium and Carnegie Mellon University All rights Reserved. */

/* $Header: /afs/cs.cmu.edu/project/atk-src-C++/atk/prefed/RCS/prefsbv.H,v 1.5 1994/11/30 20:42:06 rr2b Stab74 $ */
/* $ACIS:eza.ch 1.4$ */
/* $Source: /afs/cs.cmu.edu/project/atk-src-C++/atk/prefed/RCS/prefsbv.H,v $ */

#include <sbuttonv.H>
class prefsbv: public  sbuttonv {
public:
	virtual ATKregistryEntry *ATKregistry();

  
    
	prefsbv();
    
	virtual ~prefsbv();
  
     boolean Touch(int ind, enum view_MouseAction act)  ;
 };


#endif /* _prefsbv_H_ */
