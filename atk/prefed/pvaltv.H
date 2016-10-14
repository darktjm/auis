#ifndef _pvaltv_H_
#define _pvaltv_H_
/* Copyright 1992 by the Andrew Toolkit Consortium and Carnegie Mellon University. All rights Reserved. */

/** \addtogroup libprefs
 * @{ */
#include <textintv.H>
class pvaltv : public  textintv {
public:
	TRACED_CLASS(pvaltv);


    
	static boolean InitializeClass();

    
	pvaltv();
    

    virtual void UpdateText(long val);
    virtual void UpdateValue();
    virtual int Locate(long pos)  ;

     void ObservedChanged(class observable *changed, long value);
     class keystate * Keys()  ;
     class menulist * Menus()  ;
     void SetDataObject(class dataobject *d);


    class keystate *ks;
    class menulist *menulistp;
};
/** @} */
#endif /* _pvaltv_H_ */
