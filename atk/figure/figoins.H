ATK_INTER
#ifndef _figoins_H_
#define _figoins_H_
/** \addtogroup libfigure
 * @{ */
/* figoins.H - fig element object: inset */
/*
	Copyright Carnegie Mellon University 1992 - All rights reserved
*/

#include <figorect.H>

class figoins : public  figorect {
public:
	virtual ATKregistryEntry *ATKregistry();


    
      static class figoins * Create(long left, long top, long width, long height, char *dataobjectname)  ;
      
	figoins();

    
       class figobj * Instantiate(class figtoolview *v, long rock)  ;  
       void PrintObject(class figview *v, FILE *file, const char *prefix, boolean newstyle);
       void WriteBody(FILE *fp);
       long ReadBody(FILE *file, boolean recompute)  ;
       const char * ToolName(class figtoolview *v, long rock)  ;
       enum figobj_Status Build( class figview *v, enum view_MouseAction action, long x, long y, long clicks)  ; 
       void Draw(class figview *v); 
       void RecomputeBounds();

    
      inline class dataobject  * GetDataObject()  { return ((this)->dobj); }

    
      class dataobject *dobj;
      boolean moribund;
      boolean paste;
};
/** @} */
#endif /* _figoins_H_ */
