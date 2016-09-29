#ifndef _figospli_H_
#define _figospli_H_
/** \addtogroup libfigure
 * @{ */
/* figospli.H - fig element object: spline */
/*
	Copyright Carnegie Mellon University 1992 - All rights reserved
*/

struct figospli_cubit {
    double xa, xb, xc, xd;
    double ya, yb, yc, yd;
};

#include <figoplin.H>

class figospli : public  figoplin {
public:
	virtual ATKregistryEntry *ATKregistry();


    
      	static boolean InitializeClass();
static class figospli * Create(struct point *pointlist, long numpoints, boolean closed)  ;
      
	figospli();
      
	virtual ~figospli();

    
       void PrintObject(class figview *v, FILE *file, const char *prefix, boolean newstyle);
       const char * ToolName(class figtoolview *v, long rock)  ;
       void Draw(class figview *v); 
       void RecomputeBounds();
       enum figobj_HitVal HitMe(long x, long y, long delta, long *ptref)  ;

    
      struct figospli_cubit *cubit;
      int cubit_size;
      struct point *tmppts;
      int tmppts_size;
};
/** @} */
#endif /* _figospli_H_ */
