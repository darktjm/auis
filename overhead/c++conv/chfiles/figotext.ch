/* figotext.ch - fig element object: text */
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
*/

#include <rect.h>

class figotext : figobj {

    classprocedures:
      Create(char *chars, long x, long y) returns struct figotext *;
      InitializeClass() returns boolean; 
      InitializeObject(struct figotext *self) returns boolean;
      FinalizeObject(struct figotext *self);

    overrides:
      PrintObject(struct figview *v, FILE *file, char *prefix);
      ToolName(struct figtoolview *v, long rock) returns char *;
      Draw(struct figview *v); 
      Sketch(struct figview *v); 
      Build(enum view_MouseAction action, struct figview *v, long x, long y, long clicks) returns enum figobj_Status; 
      HitMe(long x, long y, long delta, long *ptref) returns enum figobj_HitVal;
      MoveHandle(x, y, ptref);
      Reshape(enum view_MouseAction action, struct figview *v, long x, long y, boolean handle, long ptref) returns boolean;
      RecomputeBounds();
      GetBounds(struct figview *vv) returns struct rectangle *;
      InheritVAttributes(struct figattr *attr, unsigned long mask);
      UpdateVAttributes(struct figattr *attr, unsigned long mask) returns unsigned long;
      WriteBody(FILE *fp);
      ReadBody(FILE *file, boolean recompute) returns long;
      GetHandleType(int num) returns enum figobj_HandleType;
      GetCanonicalHandles() returns long *;

    methods:

    macromethods:
      Text()  ((self)->text)

    data:
      char *text;
      int text_size;
      boolean textdirty;
      long textw, texth, yoffset;
      struct rectangle handlerect;
      int dotpos;
      long excessx, excessy;
      struct fontdesc *fdesc;
      struct figview *basis;
      struct keystate *Keystate;
      short buildstate;
      struct figview *buildview;
};
