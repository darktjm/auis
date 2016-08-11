ATK_INTER
#ifndef _figoplin_H_
#define _figoplin_H_
/* figoplin.ch - fig element object: polyline */
/*
	Copyright Carnegie Mellon University 1992 - All rights reserved
	$Disclaimer: 
// Permission to use, copy, modify, and distribute this software and its 
// documentation for any purpose and without fee is hereby granted, provided 
// that the above copyright notice appear in all copies and that both that 
// copyright notice and this permission notice appear in supporting 
// documentation, and that the name of IBM not be used in advertising or 
// publicity pertaining to distribution of the software without specific, 
// written prior permission. 
//                         
// THE COPYRIGHT HOLDERS DISCLAIM ALL WARRANTIES WITH REGARD 
// TO THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF 
// MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL ANY COPYRIGHT 
// HOLDER BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL 
// DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, 
// DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE 
// OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION 
// WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
// 
//  $

  $Header: /afs/cs.cmu.edu/project/atk-src-C++/atk/figure/RCS/figoplin.H,v 3.5 1994/11/30 20:42:06 rr2b Stab74 $
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

      void PrintObject(class figview *v, FILE *file, char *prefix, boolean newstyle);
      char * ToolName(class figtoolview *v, long rock)  ;
      void ToolModify(class figtoolview *v, long rock, boolean firsttime); 
      class figobj * Instantiate(class figtoolview *v, long rock)  ;  
      void WriteBody(FILE *fp);
      long ReadBody(FILE *file, boolean recompute)  ;
      void Draw(class figview *v); 
      void Sketch(class figview *v); 
      void Select(class figview *v); 
      enum figobj_Status Build(class figview *v, enum view_MouseAction action, long x, long y, long clicks)  ; 
      enum figobj_HitVal HitMe(long x, long y, long delta, long *ptref)  ;
      boolean Reshape(enum view_MouseAction action, class figview *v, long x, long y, boolean handle, long ptref)  ;
      boolean AddParts(enum view_MouseAction action, class figview *v, long x, long y, boolean handle, long ptref)  ;
      boolean DeleteParts(enum view_MouseAction action, class figview *v, long x, long y, boolean handle, long ptref)  ;
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

#endif /* _figoplin_H_ */
