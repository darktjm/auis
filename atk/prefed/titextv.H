ATK_INTER
#ifndef _titextv_H_
#define _titextv_H_
/* Copyright 1992 by the Andrew Toolkit Consortium and Carnegie Mellon University. All rights Reserved. */

/** \addtogroup libprefs
 * @{ */
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
/** @} */
#endif /* _titextv_H_ */
