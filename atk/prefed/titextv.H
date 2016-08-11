ATK_INTER
#ifndef _titextv_H_
#define _titextv_H_
/* Copyright 1992 by the Andrew Toolkit Consortium and Carnegie Mellon University. All rights Reserved. */

/* $Header: /afs/cs.cmu.edu/project/atk-src-C++/atk/prefed/RCS/titextv.H,v 1.5 1994/11/30 20:42:06 rr2b Stab74 $ */

#include <textview.H>
class titextv : public  textview  {
public:
	virtual ATKregistryEntry *ATKregistry();


    
	titextv();
    
	virtual ~titextv();

     void Update();

    inline void SetTextIntv(class textintv *v) { (this->intv=(v)); }
    inline class textintv  * GetTextIntv() { return (this->intv); }

   class textintv *intv;
   long lastpos;
};

#endif /* _titextv_H_ */
