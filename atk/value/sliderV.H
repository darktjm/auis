ATK_INTER
#ifndef _sliderV_H_
#define _sliderV_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1989 - All Rights Reserved      *
 *        For full copyright information see:'andrew/config/COPYRITE'     *
\* ********************************************************************** */

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
class sliderV : public  valueview
{
public:
	virtual ATKregistryEntry *ATKregistry();

 
   void DrawFromScratch(long x,long y,long width,long height);
   void DrawHighlight();
   void DrawDehighlight();
   void DrawNewValue();
   class valueview  * DoHit(enum view_MouseAction type, long x,long y,long hits)
     ;
   void LookupParameters();

  virtual char * GetValueString()  ;
  virtual void Drawslider(boolean fullupdate,struct rectangle *rr);

  inline long GetTmpVal() { return (this->tmpval); }

  
	static boolean InitializeClass();

  
	sliderV();
  
	virtual ~sliderV(); 

  char * label;
  char * fontname;
  char buf[32];
  short fontsize;
  class fontdesc * normalfont, * boldfont, * activefont, *valuefont;
  long maxval, minval , rv;
  long increment, tmpval,lasty;
  struct rectangle wheelrec,labelrec,valrec,inwheelrec,lastdrawn;
  long x,y,width,height,granular,gran;
  struct sbutton_prefs *prefs;
  boolean doclip;
  long sliderwidth;
  boolean immediate, immediatedefault,readonly,readonlydefault;
  int mono;
};

/* parameters to spst
   label		string		Defaults to NULL
   bodyfont		string		Defaults to "andytype"
   bodyfont-size	long		Defaults to 10
   max-value		long		defaults to 100
  min-value		long		dafaults to 0
  increment		float		defaults to 1.0
*/

#endif /* _sliderV_H_ */
