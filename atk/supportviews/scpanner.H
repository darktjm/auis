ATK_INTER
#ifndef _scrollandpanner_H_
#define _scrollandpanner_H_
/* scpanner.ch - scrollbar / panner box view */

/*
	Copyright Carnegie Mellon University 1992 - All rights reserved
*/


#include <scroll.H>

class scrollandpanner  : public  scroll {
public:
	virtual ATKregistryEntry *ATKregistry();


    
      static class scrollandpanner * Create(class view *view, int location)  ;
      
	scrollandpanner();
      
	virtual ~scrollandpanner();


    
       void SetChild(class view *child);
       void SetScrollee(class view *scrollee);
       class view * GetChild()  ;

    
      class panner *pannerp;

};

#endif /* _scrollandpanner_H_ */
