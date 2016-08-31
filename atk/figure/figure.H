ATK_INTER
#ifndef _figure_H_
#define _figure_H_
/* figure.ch - drawing data object */
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

#include <rect.h>

#include <dataobject.H>

#define figure_NULLREF (-1)

/* status values for NotifyObservers */
#define figure_DATACHANGED 1

struct figure_oref {
    class figobj *o;
    long counter;
    long next;
};

/* return values for figobj->HitMe(). Used by FindRefByPos(). */
enum figobj_HitVal {
    figobj_Miss,	/* miss (ptref is undefined) */
    figobj_HitInside,	/* inside body (ptref is undefined) */
    figobj_HitBody,	/* on body part (indicated by a ptref) */
    figobj_HitHandle 	/* on handle (indicated by a ptref) */
};

typedef boolean (*figure_eofptr)(class figobj *o, long ref, class figure *self, long rock);

class figure : public  dataobject  {
public:
	virtual ATKregistryEntry *ATKregistry();


    
      
	static boolean InitializeClass();

      
	figure();
      
	virtual ~figure();
    
       void SetAttributes(struct attributes *attributes);
       long Write(FILE *fp, long writeid, int level)  ;
       long Read(FILE *file, long id)  ;
       const char * ViewName()  ;

    
      virtual long InsertObject(class figobj *o, long parent, long depth)  ;
      virtual boolean DeleteObject(class figobj *o)  ;
      virtual void LinkObjectByRef(long ref, long parent, long depth);
      virtual void UnlinkObjectByRef(long ref);
      virtual long AlwaysInsertObject(class figobj *o, long parent, long depth)  ;
      virtual boolean AlwaysDeleteObject(class figobj *o)  ;
      virtual void AlwaysLinkObjectByRef(long ref, long parent, long depth);
      virtual void AlwaysUnlinkObjectByRef(long ref);
      virtual class figobj * EnumerateObjects(struct rectangle *area, boolean allowoverlap, figure_eofptr func, long rock)  ;
      virtual class figobj * EnumerateObjectGroup(long groupref, struct rectangle *area, boolean allowoverlap, figure_eofptr func, long rock)  ;
      virtual class figobj * EnumerateObjectTree(long groupref, struct rectangle *area, boolean allowoverlap, figure_eofptr func, long rock)  ;
      virtual long FindRefByObject(class figobj *o)  ;
      virtual class figobj * FindObjectByRef(long ref)  ;
      virtual long FindRefByName(char *name)  ;
      virtual long FindRefByPos(long gref, boolean recursive, enum figobj_HitVal howhit, long delta, long x, long y, long *ptref)  ;
      virtual long FindDepthByRef(long ref)  ;
      virtual void WritePartial(FILE *fp, long writeid, int level, long *list, long listsize, struct point *origin);
      virtual long ReadPartial(FILE *file, long id, long focus, struct point *origin)  ;
      virtual struct rectangle * GetOverallBounds()  ;

    
      inline long   RootObjRef()  { return ((this)->root); }
      inline class figobj *FindObjectByName(char *nm) { return this->FindObjectByRef(this->FindRefByName(nm)); }
      inline long   GetObjectCounter()  { return ((this)->ocounter); }
      inline void SetChildBoundMod()  { ((this)->bboxdirty = TRUE); }
      inline boolean   GetReadOnly()  { return ((this)->ReadOnly); }
      inline void SetReadOnly(boolean val)  { ((this)->ReadOnly = (val)); }
      inline long   GetOriginX()  { return ((this)->originx); }
      inline long   GetOriginY()  { return ((this)->originy); }
      inline void SetOrigin(long x, long y) { ((this)->originx = (x), (this)->originy = (y)); }
      inline double   GetPrintScaleX() { return ((this)->printscalex); }
      inline double   GetPrintScaleY() { return ((this)->printscaley); }
      inline void SetPrintScale(double x, double y)  { ((this)->printscalex = (x), (this)->printscaley = (y)); }
      inline boolean   GetPrintLandscape() { return ((this)->PrintLandscape); }
      inline void SetPrintLandscape(boolean val)  { ((this)->PrintLandscape = (val)); }

    
      struct figure_oref *objs;
      long objs_size;
      long root;
      boolean ReadOnly;

      long ocounter;
      boolean bboxdirty;
      struct rectangle bbox;
      long originx, originy;

      double printscalex, printscaley;
      boolean PrintLandscape;
};


#endif /* _figure_H_ */
