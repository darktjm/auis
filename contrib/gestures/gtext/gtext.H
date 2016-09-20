#ifndef _gtext_H_
#define _gtext_H_
/** \addtogroup libgtext libgtext.so
 *  A dynamically loadable ATK class file.  Contains gtext, gtextview.
 * @{ */
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
/** @} */
#endif /* _gtext_H_ */
