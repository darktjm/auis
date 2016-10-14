#ifndef _textintv_H_
#define _textintv_H_
/* Copyright 1992 by the Andrew Toolkit Consortium and Carnegie Mellon University. All rights Reserved. */

/** \addtogroup libprefs
 * @{ */
#include <wrapv.H>
class textintv : public  wrapv {
public:
	TRACED_CLASS(textintv);


    
	textintv();
    

    inline class textview *GetTextView() { return ((class textview *)((class wrapv *)this)->tv); }
    inline class text *GetText() { return ((class text *)((class wrapv *)this)->t); }

    virtual void SetDotPosition(long pos);

};
/** @} */
#endif /* _textintv_H_ */
