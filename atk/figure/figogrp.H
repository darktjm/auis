ATK_INTER
#ifndef _figogrp_H_
#define _figogrp_H_
/* figogrp.ch - fig element object: group */
/*
	Copyright Carnegie Mellon University 1992 - All rights reserved
*/

#include <rect.h>

#define figogrp_Range (2048)

#include <figobj.H>

class figogrp : public  figobj {
public:
	virtual ATKregistryEntry *ATKregistry();


    
      
	figogrp();
      
	virtual ~figogrp();
      
	static boolean InitializeClass();

      
    
       void WriteBody(FILE *fp);
       long ReadBody(FILE *file, boolean recompute)  ;
       const char * ToolName(class figtoolview *v, long rock)  ;
       void Draw(class figview *v); 
       void Sketch(class figview *v);
       void Select(class figview *v); 
       enum figobj_Status Build( class figview *v, enum view_MouseAction action, long x, long y, long clicks)  ;
       boolean Reshape(enum view_MouseAction action, class figview *v, long x, long y, boolean handle, long ptref)  ;
       void MoveHandle(long x, long y, long ptref);
       enum figobj_HandleType GetHandleType(long num)  ;
       long * GetCanonicalHandles()  ;
       void StabilizeAttachments(boolean keepproport);

       void Reposition(long xd, long yd);
       void RecomputeBounds();
       struct rectangle * GetBounds(class figview *vv)  ;
       struct rectangle * GetSelectedBounds(class figview *vv)  ;
       void InheritVAttributes(class figattr *attr, unsigned long mask);
       unsigned long UpdateVAttributes(class figattr *attr, unsigned long mask)  ;
       void SetParent(long parentref,class figure *fig_ancestor);

    
      virtual void SetConstraintsActive(boolean val);
      virtual void Reconfigure();
      
    
      inline long   GetRoot()       { return ((this)->root); }
      inline long *GetRootPtr()  { return (&((this)->root)); }
      inline void SetChildBoundMod()  { ((this)->bboxdirty = TRUE); }

    
      long root;
      boolean bboxdirty;
      boolean doconstraints;
      struct rectangle handlebox;
};

#endif /* _figogrp_H_ */
