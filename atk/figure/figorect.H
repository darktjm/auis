ATK_INTER
#ifndef _figorect_H_
#define _figorect_H_
/* figorect.ch - fig element object: rectangle */
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
*/
#include <figobj.H>

class figorect : public  figobj {
public:
	virtual ATKregistryEntry *ATKregistry();


    
      static class figorect * Create(long left, long top, long width, long height)  ;
      
	figorect();

    
       void PrintObject(class figview *v, FILE *file, const char *prefix, boolean newstyle);
       void WriteBody(FILE *fp);
       long ReadBody(FILE *file, boolean recompute)  ;
       const char * ToolName(class figtoolview *v, long rock)  ;
       void ToolModify(class figtoolview *v, long rock, boolean firsttime);
       class figobj * Instantiate(class figtoolview *v, long rock)  ;  
       void Draw(class figview *v); 
       void Sketch(class figview *v); 
       enum figobj_HitVal HitMe(long x, long y, long delta, long *ptref)  ;
       enum figobj_Status Build(class figview *v, enum view_MouseAction action,  long x, long y, long clicks)  ; 
       boolean Reshape(enum view_MouseAction action, class figview *v, long x, long y, boolean handle, long ptref)  ;
       void MoveHandle(long x, long y, long ptref);
       void Reposition(long xd, long yd);
       void InheritVAttributes(class figattr *attr, unsigned long mask);
       unsigned long UpdateVAttributes(class figattr *attr, unsigned long mask)  ;
       void RecomputeBounds();
       enum figobj_HandleType GetHandleType(long ptref)  ;
       long * GetCanonicalHandles()  ;
      
    
      inline long   &PosW()  { return ((this)->w); }
      inline long   &PosH()  { return ((this)->h); }

    
      long w, h;  /* may be negative */
      boolean dummysymmetry; 
};

#endif /* _figorect_H_ */
