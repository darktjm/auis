ATK_INTER
#ifndef _figotext_H_
#define _figotext_H_
/** \addtogroup libfigure
 * @{ */
/* figotext.H - fig element object: text */
/*
	Copyright Carnegie Mellon University 1992 - All rights reserved
*/

#include <rect.h>

#include <figobj.H>

class figotext : public  figobj {
    public:
      virtual ATKregistryEntry *ATKregistry();

    
      static class figotext * Create(char *chars, long x, long y)  ;
      
      static boolean InitializeClass();


      figotext();

      virtual ~figotext();


      void PrintObject(class figview *v, FILE *file, const char *prefix, boolean newstyle);
      const char * ToolName(class figtoolview *v, long rock)  ;
      void Draw(class figview *v); 
      void Sketch(class figview *v); 
      enum figobj_Status Build(class figview *v, enum view_MouseAction action, long x, long y, long clicks)  ; 
      enum figobj_HitVal HitMe(long x, long y, long delta, long *ptref)  ;
      void MoveHandle(long x, long y, long ptref);
      boolean Reshape(enum view_MouseAction action, class figview *v, long x, long y, boolean handle, long ptref)  ;
      boolean AddParts(enum view_MouseAction action, class figview *v, long x, long y, boolean handle, long ptref)  ;
      void RecomputeBounds();
      struct rectangle * GetBounds(class figview *vv)  ;
      void InheritVAttributes(class figattr *attr, unsigned long mask);
      unsigned long UpdateVAttributes(class figattr *attr, unsigned long mask)  ;
      void WriteBody(FILE *fp);
      long ReadBody(FILE *file, boolean recompute)  ;
      enum figobj_HandleType GetHandleType(long num)  ;
      long * GetCanonicalHandles()  ;
      boolean ORecSearch(struct SearchPattern *pat);
      boolean ORecSrchResume(struct SearchPattern *pat);
      boolean ORecSrchReplace(class dataobject *text, long pos, long len);



      inline char  * Text()  { return ((this)->text); }


      char *text;
      boolean recsearchvalid;
      int recsearchsubstart, recsearchsublen;
      int text_size;
      boolean textdirty;
      long textw, texth, yoffset;
      struct rectangle handlerect;
      int dotpos;
      long excessx, excessy;
      class fontdesc *fdesc;
      class figview *basis;
      class keystate *Keystate;
      short buildstate;
      class figview *buildview;
};
/** @} */
#endif /* _figotext_H_ */
