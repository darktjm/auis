/* figoins.ch - fig element object: inset */
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

  $Header: /afs/cs.cmu.edu/project/atk-src-C++/overhead/c++conv/chfiles/RCS/figoins.ch,v 1.1 1993/10/13 15:52:18 rr2b Stab74 $
*/

class figoins : figorect {

    classprocedures:
      Create(long left, long top, long width, long height, char *dataobjectname) returns struct figoins *;
      InitializeObject(struct figoins *self) returns boolean;

    overrides:
      PrintObject(struct figview *v, FILE *file, char *prefix);
      WriteBody(FILE *fp);
      ReadBody(FILE *file, boolean recompute) returns long;
      ToolName(struct figtoolview *v, long rock) returns char *;
      Build(enum view_MouseAction action, struct figview *v, long x, long y, long clicks) returns enum figobj_Status; 
      Draw(struct figview *v); 
      RecomputeBounds();

    macromethods:
      GetDataObject()  ((self)->dobj)

    data:
      struct dataobject *dobj;
      boolean moribund;
};
