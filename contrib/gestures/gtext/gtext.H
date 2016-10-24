#ifndef _gtext_H_
#define _gtext_H_
/** \addtogroup libgtext Object gtext
 *  A dynamically loadable ATK class file.  Contains gtext, gtextview.
 * @{ */
#include <text.H>
/** A version of \ref text that recognizes gestures.
 *  By virtue of having \ref gtextv as its default view, this \ref text
 *  clone adds gesture recognition to the standard ATK \ref text editor.
 */
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
