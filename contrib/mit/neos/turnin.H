#ifndef _turnin_H_
#define _turnin_H_
/*
	$Disclaimer: 
*Permission to use, copy, modify, and distribute this software and its 
*documentation for any purpose and without fee is hereby granted, provided 
*that the above copyright notice appear in all copies and that both that 
*copyright notice and this permission notice appear in supporting 
*documentation, and that the name of IBM not be used in advertising or 
*publicity pertaining to distribution of the software without specific, 
*written prior permission. 
*                        
*THE COPYRIGHT HOLDERS DISCLAIM ALL WARRANTIES WITH REGARD 
*TO THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF 
*MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL ANY COPYRIGHT 
*HOLDER BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL 
*DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, 
*DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE 
*OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION 
*WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
*
* $
*/


 

/* ************************************************************
 *  Copyright (C) 1989, 1990, 1991
 *  by the Massachusetts Institute of Technology
 *  For full copyright information see:'mit-copyright.h'     *
 *************************************************************/

#include <mit-copyright.h>

#include <view.H>
class turnin : public  view {
public:
	virtual ATKregistryEntry *ATKregistry();


   class view * Hit(enum view_MouseAction action, long x, long y, long clicks)  ;
   void PostKeyState(class keystate *ks);
   void PostMenus(class menulist *ml);
/*  LinkTree(struct view *v); */
   void FullUpdate(enum view_UpdateType type, long t, long l, long h, long w);
   void ReceiveInputFocus();

  
	static boolean InitializeClass();

  
	turnin();
  
	virtual ~turnin();

  virtual void SetTitle(const char *string);
  virtual void GoForIt();

  inline void SetParent(class eos *dad) { ((this)->daddy = dad); }
  inline MISSING_RETURN_TYPE SetCourse(char *name) { return (strcpy((this)->course, name)); }

  class label *title;
  class labelview *titleV;
  class lpair *topscreen;
  class lpair *userarea;
  class lpair *questions;
  class lpair *buttons;
  class lpair *whole;
  class value *onoff;
  class onoffV *onoffv;
  class pushbutton *okb;
  class pushbutton *cancelb;
  class newbuttonview *okv;
  class newbuttonview *cancelv;
  class strinput *name;
  class strinput *number;
  class eos *daddy;
  class keystate *kstate;
  class menulist *menus;
  char *course;
  boolean turninfromfile;
  boolean IDoNotExist;
  boolean ResourcesPosted;
};

#endif /* _turnin_H_ */
