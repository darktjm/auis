ATK_INTER
#ifndef _figorrec_H_
#define _figorrec_H_
/* figorrec.ch - fig element object: rounded rectangle */
/*
	Copyright Carnegie Mellon University 1992 - All rights reserved
*/

#include <figorect.H>

class figorrec : public  figorect {
public:
	virtual ATKregistryEntry *ATKregistry();


    
      static class figorrec * Create(long left, long top, long width, long height)  ;
      
	figorrec();

    
       void PrintObject(class figview *v, FILE *file, const char *prefix, boolean newstyle);
       const char * ToolName(class figtoolview *v, long rock)  ;
       void Draw(class figview *v); 
       void Sketch(class figview *v); 
};

#endif /* _figorrec_H_ */
