ATK_INTER
#ifndef _stringV_H_
#define _stringV_H_
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
class stringV : public  valueview {
public:
	virtual ATKregistryEntry *ATKregistry();

 
   void DrawFromScratch(long x,long y,long width,long height);
   void DrawHighlight();
   void DrawDehighlight();
   void LookupParameters();
   void DrawNewValue();
 
  stringV();
	
	static boolean InitializeClass();

 
   inline boolean GetUseAlt(){ return (this->UseAlt); }
   inline void SetUseAlt(boolean val) { (this->UseAlt = val); }
 
  char * label;
  char * fontname;
  short fontsize;
  class fontdesc * normalfont, * boldfont, * activefont;
  struct rectangle labelrec;
  long x,y,width,height;
  boolean UseAlt;
  char *plabel,*foreground,*background;
};

/* parameters to spst
   label		string		Defaults to NULL
   bodyfont		string		Defaults to "andytype"
   bodyfont-size	long		Defaults to 10
*/

#endif /* _stringV_H_ */
