#ifndef _phelpv_H_
#define _phelpv_H_
/* Copyright 1992 by Carnegie Mellon University All rights Reserved. */

/** \addtogroup libprefs
 * @{ */
#include <textview.H>
class phelpv : public  textview  {
public:
	virtual ATKregistryEntry *ATKregistry();


    
	static boolean InitializeClass();

    
	phelpv();
    
	virtual ~phelpv();

     void SetDotPosition(long pos);
     class view * Hit(enum view::MouseAction action, long x, long y, long clicks)  ;



    inline void SetPrefs(class prefs *p) { ((this)->prefs=(p)); }
    inline class prefs  * GetPrefs() { return ((this)->prefs); }

    class prefs *prefs;
};
/** @} */
#endif /* _phelpv_H_ */
