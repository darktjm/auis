#ifndef _switview_H_
#define _switview_H_
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
struct switcheroo {
    struct switchee *switchee;
    class view *v;
    struct switcheroo *next;
};

#include <view.H>
class switview: public view {
public:
	virtual ATKregistryEntry *ATKregistry();

    
      
	static boolean InitializeClass();

      
	switview();
      
	virtual ~switview();
    
       void FullUpdate(enum view::UpdateType type, long left,
		  long top, long width, long height);
       void Update();
       class view * Hit (enum view::MouseAction action, long x, long y,
	    long numberOfClicks)  ;
       void PostKeyState(class keystate *keystate);
       void PostMenus(class menulist *ml);
       void LinkTree(class view *parent);
       void WantInputFocus(class view *v);
    
      class keystate *ks;
      class menulist *ml;
      struct switcheroo *FirstSwitcheroo, *NowPlaying;
};

#endif /* _switview_H_ */
