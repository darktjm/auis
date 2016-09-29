#ifndef _prefsv_H_
#define _prefsv_H_
/* Copyright 1992 by the Andrew Toolkit Consortium and Carnegie Mellon University All rights Reserved. */

/** \addtogroup libprefs
 * @{ */
#include <textview.H>
class prefsv : public  textview  {
public:
	virtual ATKregistryEntry *ATKregistry();


    
	static boolean InitializeClass();

    
	prefsv();
    
	virtual ~prefsv();


     void PostMenus(class menulist *ml);
     void PostKeyState(class keystate *ks);



    class keystate *ks;
    class menulist *menulistp;
};
/** @} */
#endif /* _prefsv_H_ */
