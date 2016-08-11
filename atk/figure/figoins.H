ATK_INTER
#ifndef _figoins_H_
#define _figoins_H_
/* figoins.ch - fig element object: inset */
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

  $Header: /afs/cs.cmu.edu/project/atk-src-C++/atk/figure/RCS/figoins.H,v 3.5 1994/11/30 20:42:06 rr2b Stab74 $
*/

#include <figorect.H>

class figoins : public  figorect {
public:
	virtual ATKregistryEntry *ATKregistry();


    
      static class figoins * Create(long left, long top, long width, long height, char *dataobjectname)  ;
      
	figoins();

    
       class figobj * Instantiate(class figtoolview *v, long rock)  ;  
       void PrintObject(class figview *v, FILE *file, char *prefix, boolean newstyle);
       void WriteBody(FILE *fp);
       long ReadBody(FILE *file, boolean recompute)  ;
       char * ToolName(class figtoolview *v, long rock)  ;
       enum figobj_Status Build( class figview *v, enum view_MouseAction action, long x, long y, long clicks)  ; 
       void Draw(class figview *v); 
       void RecomputeBounds();

    
      inline class dataobject  * GetDataObject()  { return ((this)->dobj); }

    
      class dataobject *dobj;
      boolean moribund;
      boolean paste;
};

#endif /* _figoins_H_ */
