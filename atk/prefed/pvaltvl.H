#ifndef _pvaltvl_H_
#define _pvaltvl_H_
/* Copyright 1992 by the Andrew Toolkit Consortium and Carnegie Mellon University. All rights Reserved. */

/** \addtogroup libprefs
 * @{ */
#include <pvaltv.H>
class pvaltvl : public  pvaltv {
public:
	TRACED_CLASS(pvaltvl);


    
	static boolean InitializeClass();

    
	pvaltvl();
    


     void ObservedChanged(class observable *changed, long value);
     void SetDataObject(class dataobject *d);
     void UpdateText(long val);
     void UpdateValue();
     class keystate * Keys()  ;
     void SetDotPosition(long pos);


    class keystate *ks;
    class menulist *menulistp;
};
/** @} */
#endif /* _pvaltvl_H_ */
