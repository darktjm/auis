#ifndef _gtext_H_
#define _gtext_H_
/** \addtogroup libgtext Object gtext
 *  A dynamically loadable ATK class file.  Contains gtext, gtextview.
 * @{ */
#include <text.H>
class gtext : public  text {
public:
	TRACED_CLASS(gtext);

    
      
	static boolean InitializeClass();
	static boolean atk_inited;

      
	gtext();
      
    
       const char * ViewName()  ;
};
/** @} */
#endif /* _gtext_H_ */
