#ifndef _clogview_H_
#define _clogview_H_
/* **************************************************** *\
Copyright 1989 Nathaniel S. Borenstein
Permission to use, copy, modify, and distribute this software and its
documentation for any purpose and without fee is hereby granted,
provided that the above copyright notice appear in all copies and
that both that copyright notice and this permission notice appear in
supporting documentation, and that the name of 
Nathaniel S. Borenstein not be used in
advertising or publicity pertaining to distribution of the software
without specific, written prior permission. 

Nathaniel S. Borenstein DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING ALL
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
Nathaniel S. Borenstein BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY
DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER
IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING
OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
\* ***************************************************** */
/* If we had multiple inheritance, this would be a subclass of conview AND textview. */

#include <conview.H>
class clogview: public conview {
public:
	virtual ATKregistryEntry *ATKregistry();

    
      
	clogview();
      
	virtual ~clogview();
    
       void FullUpdate(enum view::UpdateType type, long left,
		  long top, long width, long right);
       void Update();
       class view * Hit (enum	view::MouseAction action, long x, long y,
	    long numberOfClicks)  ;
       void LinkTree(class view *parent);
    
      class scroll *s;
      class text *t;
      class textview *tv;
};

#endif /* _clogview_H_ */
