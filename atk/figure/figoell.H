ATK_INTER
#ifndef _figoell_H_
#define _figoell_H_
/* figoell.ch - fig element object: ellipse */
/*
	Copyright Carnegie Mellon University 1992 - All rights reserved
*/

#include <figorect.H>

class figoell : public  figorect {
public:
	virtual ATKregistryEntry *ATKregistry();


    
      static class figoell * Create(long left, long top, long width, long height)  ;
      
	figoell();

    
       void PrintObject(class figview *v, FILE *file, const char *prefix, boolean newstyle);
       const char * ToolName(class figtoolview *v, long rock)  ;
       enum figobj_HitVal HitMe(long x, long y, long delta, long *ptref)  ;
       void Draw(class figview *v); 
       void Sketch(class figview *v); 
};

#endif /* _figoell_H_ */
