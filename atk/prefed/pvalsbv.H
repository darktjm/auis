ATK_INTER
#ifndef _pvalsbv_H_
#define _pvalsbv_H_
/* Copyright 1992 by the Andrew Toolkit Consortium and Carnegie Mellon University. All rights Reserved. */

/* $Header: /afs/cs.cmu.edu/project/atk-src-C++/atk/prefed/RCS/pvalsbv.H,v 1.5 1994/11/30 20:42:06 rr2b Stab74 $ */
/* $ACIS:eza.ch 1.4$ */
/* $Source: /afs/cs.cmu.edu/project/atk-src-C++/atk/prefed/RCS/pvalsbv.H,v $ */

#include <wrapv.H>
class pvalsbv : public  wrapv {
public:
	virtual ATKregistryEntry *ATKregistry();


    
	pvalsbv();
    
	virtual ~pvalsbv();

     void ObservedChanged(class observable *changed, long val);
     void SetDataObject(class dataobject *prefval);

    inline class sbuttonv *GetSButtonView() { return ((class sbuttonv *)((class wrapv *)this)->tv); }
    inline class sbutton *GetSButton() { return ((class sbutton *)((class wrapv *)this)->t); }

    virtual void UpdateSButton();
};


#endif /* _pvalsbv_H_ */