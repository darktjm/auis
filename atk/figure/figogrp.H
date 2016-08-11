ATK_INTER
#ifndef _figogrp_H_
#define _figogrp_H_
/* figogrp.ch - fig element object: group */
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

  $Header: /afs/cs.cmu.edu/project/atk-src-C++/atk/figure/RCS/figogrp.H,v 3.3 1994/11/30 20:42:06 rr2b Stab74 $
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
       char * ToolName(class figtoolview *v, long rock)  ;
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
