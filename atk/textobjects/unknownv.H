#ifndef _unknownv_H_
#define _unknownv_H_
/* Copyright 1994 Carnegie Mellon University All rights reserved. */
/** \addtogroup libtextobjs
 * @{ */
#include <textview.H>
class unknownv : public  textview  {
public:
	TRACED_CLASS(unknownv);

    
       void PostMenus(class menulist *menus);
    
      
	static boolean InitializeClass();

    
	unknownv();
    
    
      class menulist *menus;
 };
/** @} */
#endif /* _unknownv_H_ */
