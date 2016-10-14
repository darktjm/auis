#ifndef _weblinkview_H_
#define _weblinkview_H_
/** \addtogroup libhtmltext
 * @{ */
#include <linkview.H>
class weblinkview: public  linkview{
public:
	TRACED_CLASS(weblinkview);

 
  
	static boolean InitializeClass();

  
	weblinkview();
  
 	
   void PostMenus(class menulist *ml);
 
   void LinkFile(char *dest);
 
  class menulist *ml;
};
/** @} */
#endif /* _weblinkview_H_ */
