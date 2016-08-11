/* figogrp.ch - fig element object: group */
/*
	Copyright Carnegie Mellon University 1992 - All rights reserved
	$Disclaimer: 
 * Permission to use, copy, modify, and distribute this software and its 
 * documentation for any purpose and without fee is hereby granted, provided 
 * that the above copyright notice appear in all copies and that both that 
 * copyright notice and this permission notice appear in supporting 
 * documentation, and that the name of IBM not be used in advertising or 
 * publicity pertaining to distribution of the software without specific, 
 * written prior permission. 
 *                         
 * THE COPYRIGHT HOLDERS DISCLAIM ALL WARRANTIES WITH REGARD 
 * TO THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF 
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL ANY COPYRIGHT 
 * HOLDER BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL 
 * DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, 
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE 
 * OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION 
 * WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 * 
 *  $

  $Header: /afs/cs.cmu.edu/project/atk-src-C++/overhead/c++conv/chfiles/RCS/figogrp.ch,v 1.1 1993/10/13 15:52:18 rr2b Stab74 $
*/

#include <rect.h>

#define figogrp_Range (2048)

class figogrp : figobj {

    classprocedures:
      InitializeObject(struct figogrp *self) returns boolean;
      FinalizeObject(struct figogrp *self);
      InitializeClass() returns boolean;
      
    overrides:
      WriteBody(FILE *fp);
      ReadBody(FILE *file, boolean recompute) returns long;
      ToolName(struct figtoolview *v, long rock) returns char *;
      Draw(struct figview *v); 
      Sketch(struct figview *v);
      Select(struct figview *v); 
      Build(enum view_MouseAction action, struct figview *v, long x, long y, long clicks) returns enum figobj_Status;
      Reshape(enum view_MouseAction action, struct figview *v, long x, long y, boolean handle, long ptref) returns boolean;
      MoveHandle(x, y, ptref);
      GetHandleType(long num) returns enum figobj_HandleType;
      GetCanonicalHandles() returns long *;
      StabilizeAttachments(boolean keepproport);

      Reposition(long xd, long yd);
      RecomputeBounds();
      GetBounds(struct figview *vv) returns struct rectangle *;
      GetSelectedBounds(struct figview *vv) returns struct rectangle *;
      InheritVAttributes(struct figattr *attr, unsigned long mask);
      UpdateVAttributes(struct figattr *attr, unsigned long mask) returns unsigned long;
      SetParent(long parentref, struct *fig ancestor);

    methods:
      SetConstraintsActive(boolean val);
      Reconfigure();
      
    macromethods:
      GetRoot()       ((self)->root)
      GetRootPtr()  (&((self)->root))
      SetChildBoundMod()  ((self)->bboxdirty = TRUE)

    data:
      long root;
      boolean bboxdirty;
      boolean doconstraints;
      struct rectangle handlebox;
};
