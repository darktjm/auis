#ifndef _gtext_H_
#define _gtext_H_
#include <text.H>
class gtext : public  text {
public:
	virtual ATKregistryEntry *ATKregistry();

    
      
	static boolean InitializeClass();
	static boolean atk_inited;

      
	gtext();
      
	virtual ~gtext();
    
       const char * ViewName()  ;
};

#endif /* _gtext_H_ */
