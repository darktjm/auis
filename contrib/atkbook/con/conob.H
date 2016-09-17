#ifndef _conob_H_
#define _conob_H_
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
#include <dataobject.H>
class conob: public  dataobject  {
public:
	virtual ATKregistryEntry *ATKregistry();

    
      
	conob();
      
	virtual ~conob();
    
       long Read(FILE *fp, long id)  ;
       long Write(FILE *fp, long writeID, int level)  ;
       const char * ViewName()  ;
    
      virtual void SetNumval(long num);
      virtual void SetStrval(const char *str);
      /* SetStrval doesn't make a new copy of the string */
      virtual void SetDisplayTemplate(const char *displaytemplate);
      /* SetDisplayTemplate makes a new copy of the string */
      virtual void SetDisplayMin(int min);
      virtual void SetDisplayMax(int max);
      virtual void SetBoxed(boolean Boxed);
      virtual void GetStringToDisplay(char *Buf, int len, boolean IsClick);
      virtual void HandleDataLine(char *line);
      virtual void WriteState(FILE *fp);
    
      inline long   GetNumval() { return (this->numval); }
      inline const char  * GetStrval() { return (this->strval); }
      inline const char  * GetDisplayTemplate() { return (this->DisplayTemplate); }
      inline int   GetDisplayMin() { return (this->displaymin); }
      inline int   GetDisplayMax() { return (this->displaymax); }
      inline boolean  GetBoxed() { return (this->Boxed); }
    
      long numval;
      const char *strval;
      int displaymin, displaymax;
      /* (The min & max for things like EKG graphs) */
      boolean Boxed; /* Draw a box or not? */
      char *DisplayTemplate;
      /* Tells what to say for indicators & clicks */
};

#endif /* _conob_H_ */
