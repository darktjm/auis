#ifndef _figoflow_H_
#define _figoflow_H_
/** \addtogroup libfigure
 * @{ */
/* figoflow.H - fig element object: text flow unit */
/*
	Copyright Carnegie Mellon University 1992 - All rights reserved
*/

#include <figorect.H>

class figoflow : public  figorect {
  public:
    TRACED_CLASS_NODEST(figoflow);
    static boolean InitializeClass();

    figoflow();
    static class figoflow * Create(long left, long top, long width, long height);

    void ObservedChanged(class observable  *obs, long  status);
    void PrintObject(class figview *v, FILE *file, const char *prefix, boolean newstyle);
    const char * ToolName(class figtoolview *v, long rock)  ;
    enum figobj_HitVal HitMe(long x, long y, long delta, long *ptref)  ;
    void Draw(class figview *v); 
    void Sketch(class figview *v);
    void SetParent(long  pref, class figure  *ancfig);
    long Read(FILE  *fp, long  id);

    long figmodified;
    long order;
};
/** @} */
#endif /* _figoflow_H_ */
