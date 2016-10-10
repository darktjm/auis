#ifndef _flexview_H_
#define _flexview_H_
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
#include <lpair.H>
class flexview: public lpair {
public:
	virtual ATKregistryEntry *ATKregistry();

    
      
	static boolean InitializeClass();

      
	flexview();
      
	virtual ~flexview();
    
       void FullUpdate(enum view::UpdateType type, long left, long top,
		  long width, long right);
       void Update();
       class view * Hit (enum view::MouseAction action, long x, long y,
	    long numberOfClicks)  ;
       void PostKeyState(class keystate *keystate);
       void PostMenus(class menulist *ml);
       view::DSattributes DesiredSize(long width, long height,
		   enum view::DSpass pass,
		   long *dWidth, long *dheight)
	 ;
    
      class view *leftview, *rightview;
      class dataobject *oldleftdata, *oldrightdata;
      class keystate *KeystateWithViews, *KeystateWithNoViews;
      class menulist *MenulistWithViews, *MenulistWithNoViews;
};

#endif /* _flexview_H_ */
