#ifndef _scrollandpanner_H_
#define _scrollandpanner_H_
/** \addtogroup libsupviews
 * @{ */
/* scpanner.ch - scrollbar / panner box view */

/*
	Copyright Carnegie Mellon University 1992 - All rights reserved
*/


#include <scroll.H>

class scrollandpanner  : public  scroll {
public:
	TRACED_CLASS(scrollandpanner);


    
      static class scrollandpanner * Create(class view *view, int location)  ;
      
	scrollandpanner();
      


    
       void SetChild(class view *child);
       void SetScrollee(class view *scrollee);
       class view * GetChild()  ;

    
      class panner *pannerp;

};
/** @} */
#endif /* _scrollandpanner_H_ */
