#ifndef _butt_H_
#define _butt_H_
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

typedef int (*butt_hit_fptr)(void * rock1, void * rock2, class butt * self, enum view_MouseAction action);

class butt: public  view {
public:
	virtual ATKregistryEntry *ATKregistry();

    
       void FullUpdate(enum view_UpdateType type, long left, long top,
		  long width, long height);
       void Update();
       class view * Hit (enum view_MouseAction action, long x, long y,
	    long numberOfClicks)  ;
    
      
	butt();
      
	virtual ~butt();
    
      virtual void SetText(char *txt);
      virtual void SetButtonFont(class fontdesc *f);
      virtual void SetHitFunction(butt_hit_fptr HitFunction);
      virtual void SetRocks(void *r1, void *r2);
    
      char *text;
      butt_hit_fptr HitFunction;
      void *rock1, *rock2;
      class fontdesc *myfontdesc;
      class cursor *mycursor;
};


#endif /* _butt_H_ */
