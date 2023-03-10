#ifndef _switcher_H_
#define _switcher_H_
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

struct switchee {
    class dataobject *d;
    char *viewname;
    char *label;
    struct switchee *next; 
};

#include <dataobject.H>
class switcher : public  dataobject  {
public:
	TRACED_CLASS(switcher);

    
      
	switcher();
      
    
       long Read (FILE *file, long id)  ;
       long Write (FILE *file, long writeid, int level)  ;
       const char * ViewName()  ;
    
      virtual boolean AddObject(class dataobject *d, const char *label,
		 const char *viewname)  ;
      virtual boolean DeleteObject(class dataobject *d)  ;
      virtual boolean SetNowPlaying(class dataobject *d)  ;
    
      struct switchee *FirstSwitchee, *NowPlaying;
};

#endif /* _switcher_H_ */
