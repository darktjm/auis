#ifndef _figoplin_H_
#define _figoplin_H_
/** \addtogroup libfigure
 * @{ */
/* figoplin.H - fig element object: polyline */
/*
	Copyright Carnegie Mellon University 1992 - All rights reserved
*/

#include <point.h>

#include <figobj.H>

class figoplin : public  figobj {
    public:
      virtual ATKregistryEntry *ATKregistry();

      static class figoplin * Create(struct point *pointlist, long numpoints, boolean closed)  ;

      figoplin();

      virtual ~figoplin();

      static boolean InitializeClass();

      virtual void CopyData(class figoplin *src);
      virtual void SetClosed(boolean newval);

      void PrintObject(class figview *v, FILE *file, const char *prefix, boolean newstyle);
      const char * ToolName(class figtoolview *v, long rock)  ;
      void ToolModify(class figtoolview *v, long rock, boolean firsttime); 
      class figobj * Instantiate(class figtoolview *v, long rock)  ;  
      void WriteBody(FILE *fp);
      long ReadBody(FILE *file, boolean recompute)  ;
      void Draw(class figview *v); 
      void Sketch(class figview *v); 
      void Select(class figview *v); 
      enum figobj_Status Build(class figview *v, enum view::MouseAction action, long x, long y, long clicks)  ; 
      enum figobj_HitVal HitMe(long x, long y, long delta, long *ptref)  ;
      boolean Reshape(enum view::MouseAction action, class figview *v, long x, long y, boolean handle, long ptref)  ;
      boolean AddParts(enum view::MouseAction action, class figview *v, long x, long y, boolean handle, long ptref)  ;
      boolean DeleteParts(enum view::MouseAction action, class figview *v, long x, long y, boolean handle, long ptref)  ;
      void MoveHandle(long x, long y, long ptref);
      void Reposition(long xd, long yd);
      void InheritVAttributes(class figattr *attr, unsigned long mask);
      unsigned long UpdateVAttributes(class figattr *attr, unsigned long mask)  ;
      void RecomputeBounds();
      enum figobj_HandleType GetHandleType(long num)  ;
      long * GetCanonicalHandles()  ;


      inline struct point  * &Pts()  { return ((this)->pts); }
      inline int   &NumPts()  { return ((this)->numpts); }
      inline boolean Closed()  { return ((this)->closed); }


      struct point *pts, *orpts;
      struct point orhandles[4];
      int numpts;
      int pts_size;
      short buildstate;
      boolean stickysketch;
      boolean shortmode;
      boolean closed;
      int regular, dummysides;
      boolean flipv, fliph;
      long rockx, rocky, lastx, lasty, cenx, ceny;
      long rock;
};
/** @} */
#endif /* _figoplin_H_ */
