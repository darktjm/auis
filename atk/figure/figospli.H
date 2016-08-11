ATK_INTER
#ifndef _figospli_H_
#define _figospli_H_
/* figospli.ch - fig element object: spline */
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

  $Header: /afs/cs.cmu.edu/project/atk-src-C++/atk/figure/RCS/figospli.H,v 3.4 1994/11/30 20:42:06 rr2b Stab74 $
*/

struct figospli_cubit {
    double xa, xb, xc, xd;
    double ya, yb, yc, yd;
};

#include <figoplin.H>

class figospli : public  figoplin {
public:
	virtual ATKregistryEntry *ATKregistry();


    
      	static boolean InitializeClass();
static class figospli * Create(struct point *pointlist, long numpoints, boolean closed)  ;
      
	figospli();
      
	virtual ~figospli();

    
       void PrintObject(class figview *v, FILE *file, char *prefix, boolean newstyle);
       char * ToolName(class figtoolview *v, long rock)  ;
       void Draw(class figview *v); 
       void RecomputeBounds();
       enum figobj_HitVal HitMe(long x, long y, long delta, long *ptref)  ;

    
      struct figospli_cubit *cubit;
      int cubit_size;
      struct point *tmppts;
      int tmppts_size;
};

#endif /* _figospli_H_ */
