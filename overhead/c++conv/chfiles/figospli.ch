/* figospli.ch - fig element object: spline */
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

  $Header: /afs/cs.cmu.edu/project/atk-src-C++/overhead/c++conv/chfiles/RCS/figospli.ch,v 1.1 1993/10/13 15:52:18 rr2b Stab74 $
*/

struct figospli_cubit {
    double xa, xb, xc, xd;
    double ya, yb, yc, yd;
};

class figospli : figoplin {

    classprocedures:
      Create(struct point *pointlist, long numpoints, boolean closed) returns struct figospli *;
      InitializeObject(struct figospli *self) returns boolean;
      FinalizeObject(struct figospli *self);

    overrides:
      PrintObject(struct figview *v, FILE *file, char *prefix);
      ToolName(struct figtoolview *v, long rock) returns char *;
      Draw(struct figview *v); 
      RecomputeBounds();
      HitMe(long x, long y, long delta, long *ptref) returns enum figobj_HitVal;

    data:
      struct figospli_cubit *cubit;
      int cubit_size;
      struct point *tmppts;
      int tmppts_size;
};
