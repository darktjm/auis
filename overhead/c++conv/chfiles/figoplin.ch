/* figoplin.ch - fig element object: polyline */
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

#include <point.h>

class figoplin : figobj {

    classprocedures:
      Create(struct point *pointlist, long numpoints, boolean closed) returns struct figoplin *;
      InitializeObject(struct figoplin *self) returns boolean;
      FinalizeObject(struct figoplin *self);
      InitializeClass() returns boolean;

    methods:
      CopyData(struct figoplin *src);

    overrides:
      PrintObject(struct figview *v, FILE *file, char *prefix);
      ToolName(struct figtoolview *v, long rock) returns char *;
      ToolModify(struct figtoolview *v, long rock); 
      Instantiate(struct figtoolview *v, long rock) returns struct figobj *;  
      WriteBody(FILE *fp);
      ReadBody(FILE *file, boolean recompute) returns long;
      Draw(struct figview *v); 
      Sketch(struct figview *v); 
      Select(struct figview *v); 
      Build(enum view_MouseAction action, struct figview *v, long x, long y, long clicks) returns enum figobj_Status; 
      HitMe(long x, long y, long delta, long *ptref) returns enum figobj_HitVal;
      Reshape(enum view_MouseAction action, struct figview *v, long x, long y, boolean handle, long ptref) returns boolean;
      AddParts(enum view_MouseAction action, struct figview *v, long x, long y, boolean handle, long ptref) returns boolean;
      DeleteParts(enum view_MouseAction action, struct figview *v, long x, long y, boolean handle, long ptref) returns boolean;
      MoveHandle(x, y, ptref);
      Reposition(long xd, long yd);
      InheritVAttributes(struct figattr *attr, unsigned long mask);
      UpdateVAttributes(struct figattr *attr, unsigned long mask) returns unsigned long;
      RecomputeBounds();
      GetHandleType(int num) returns enum figobj_HandleType;
      GetCanonicalHandles() returns long *;

    macromethods:
      Pts()  ((self)->pts)
      NumPts()  ((self)->numpts)
      Closed()  ((self)->closed)

    data:
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
