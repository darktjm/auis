ATK_INTER
#ifndef _figorrec_H_
#define _figorrec_H_
/* figorrec.ch - fig element object: rounded rectangle */
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

  $Header: /afs/cs.cmu.edu/project/atk-src-C++/atk/figure/RCS/figorrec.H,v 3.4 1994/11/30 20:42:06 rr2b Stab74 $
*/

#include <figorect.H>

class figorrec : public  figorect {
public:
	virtual ATKregistryEntry *ATKregistry();


    
      static class figorrec * Create(long left, long top, long width, long height)  ;
      
	figorrec();

    
       void PrintObject(class figview *v, FILE *file, char *prefix, boolean newstyle);
       char * ToolName(class figtoolview *v, long rock)  ;
       void Draw(class figview *v); 
       void Sketch(class figview *v); 
};

#endif /* _figorrec_H_ */
