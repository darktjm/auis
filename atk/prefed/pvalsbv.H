#ifndef _pvalsbv_H_
#define _pvalsbv_H_
/* Copyright 1992 by the Andrew Toolkit Consortium and Carnegie Mellon University. All rights Reserved. */

/** \addtogroup libprefs
 * @{ */
#include <wrapv.H>
class pvalsbv : public  wrapv {
public:
	TRACED_CLASS(pvalsbv);


    
	pvalsbv();
    

     void ObservedChanged(class observable *changed, long val);
     void SetDataObject(class dataobject *prefval);

    inline class sbuttonv *GetSButtonView() { return ((class sbuttonv *)((class wrapv *)this)->tv); }
    inline class sbutton *GetSButton() { return ((class sbutton *)((class wrapv *)this)->t); }

    virtual void UpdateSButton();
};
/** @} */
#endif /* _pvalsbv_H_ */
