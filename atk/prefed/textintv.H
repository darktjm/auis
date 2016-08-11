ATK_INTER
#ifndef _textintv_H_
#define _textintv_H_
/* Copyright 1992 by the Andrew Toolkit Consortium and Carnegie Mellon University. All rights Reserved. */

/* $Header: /afs/cs.cmu.edu/project/atk-src-C++/atk/prefed/RCS/textintv.H,v 1.5 1994/11/30 20:42:06 rr2b Stab74 $ */
/* $ACIS:eza.ch 1.4$ */
/* $Source: /afs/cs.cmu.edu/project/atk-src-C++/atk/prefed/RCS/textintv.H,v $ */

#include <wrapv.H>
class textintv : public  wrapv {
public:
	virtual ATKregistryEntry *ATKregistry();


    
	textintv();
    
	virtual ~textintv();

    inline class textview *GetTextView() { return ((class textview *)((class wrapv *)this)->tv); }
    inline class text *GetText() { return ((class text *)((class wrapv *)this)->t); }

    virtual void SetDotPosition(long pos);

};

#endif /* _textintv_H_ */
