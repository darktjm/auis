ATK_INTER
#ifndef _checkv_H_
#define _checkv_H_


/*
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
*/
#include <fontdesc.H>
#include <rect.h>

#include <valueview.H>
class checkv : public  valueview
{
public:
	virtual ATKregistryEntry *ATKregistry();

 
   void DrawFromScratch(long x,long y,long width,long height);
   void DrawHighlight();
   void DrawDehighlight();
   void DrawNewValue();
   class valueview * DoHit(enum view_MouseAction type, long x,long y,long hits)
     ;
   void LookupParameters();

  inline long GetTmpVal() { return (this->tmpval); }

  checkv();
	
	static boolean InitializeClass();


  char *label;
  char *fontname;
  class fontdesc *normalfont;
  class graphic *BlackPattern;
  short fontsize;
  long  tmpval;
  short checktype;
  long x,y,width,height;
  char *foreground,*background;
};

#endif /* _checkv_H_ */
