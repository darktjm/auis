#ifndef _pvaltvc_H_
#define _pvaltvc_H_
/* Copyright 1992 by the Andrew Toolkit Consortium and Carnegie Mellon University. All rights Reserved. */

/** \addtogroup libprefs
 * @{ */
#include <pvaltv.H>
class pvaltvc : public  pvaltv {
public:
	TRACED_CLASS(pvaltvc);


    
	static boolean InitializeClass();

    
	pvaltvc();
    

    virtual void Select(int ind);

     void ObservedChanged(class observable *changed, long value);
     void SetDataObject(class dataobject *d);
     void UpdateText(long val);
     void UpdateValue();
     class keystate * Keys()  ;
     class view * Hit(enum view::MouseAction act, long x, long y, long numclicks)  ;


    class keystate *ks;
    class menulist *menulistp;
};
/** @} */
#endif /* _pvaltvc_H_ */
