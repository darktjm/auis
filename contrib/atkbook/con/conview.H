#ifndef _conview_H_
#define _conview_H_
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
#include <view.H>
class conview: public view {
public:
	TRACED_CLASS(conview);

    
      
	conview();
      
    
       view::DSattributes DesiredSize(long width, long height,
	enum view::DSpass pass, long *dWidth,
	long *dheight)  ;
       void FullUpdate(enum view::UpdateType type, long left,
	long top, long width, long right);
       void Update();
       class view * Hit (enum view::MouseAction action, long x,
	long y, long numberOfClicks)  ;
    
      virtual void DrawMyself(struct rectangle *r, boolean IsFullUpdate);
      virtual void SetLastString(char *str);
    
      inline char  * GetLastString() { return (this->laststring); }
    
      char *laststring;
};

#endif /* _conview_H_ */
