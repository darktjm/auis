#ifndef _weblinkview_H_
#define _weblinkview_H_
ATK_INTER

#include <linkview.H>
class weblinkview: public  linkview{
public:
	virtual ATKregistryEntry *ATKregistry();

 
  
	static boolean InitializeClass();

  
	weblinkview();
  
	virtual ~weblinkview();
 	
   void PostMenus(class menulist *ml);
 
   void LinkFile(char *dest);
 
  class menulist *ml;
};


#endif /* _weblinkview_H_ */
