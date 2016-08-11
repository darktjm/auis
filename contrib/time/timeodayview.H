#ifndef _timeodayview_H_
#define _timeodayview_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1991 - All Rights Reserved           *
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

#include <stdio.h>
#include <menulist.H>
#include <cursor.H>

#include <view.H>
class timeodayview: public  view {
public:
	virtual ATKregistryEntry *ATKregistry();

 
  
	static boolean InitializeClass();
	static boolean atk_inited;

  
	timeodayview();
  
	virtual ~timeodayview();
 	
   void FullUpdate(enum view_UpdateType type, long left, long top, long width, long height);
   void Update();
   class view * Hit (enum view_MouseAction action, long x, long y, long numberOfClicks)  ;
   void Print(FILE *file, char *processor, char *finalFormat, boolean topLevel);
   view_DSattributes DesiredSize(long width, long height, enum view_DSpass pass, long * desired_width, long * desired_height)  ;
   void GetOrigin(long width, long height, long * originX, long * originY);
   void PostMenus(class menulist *ml);
 
  inline class menulist  * GetMenuList() { return (this->ml); }
  inline void SetMenuList(class menulist *ml) { (this->ml = (ml)); }
 
  int need_full_update;
  class cursor *cursor;
  class menulist *ml;
  long last_width, last_height;
};


#endif /* _timeodayview_H_ */
