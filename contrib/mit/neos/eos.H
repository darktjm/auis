#ifndef _eos_H_
#define _eos_H_
/* $Author: wjh $ */

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

struct readWindow {
    class buffer *data;
    class im *im;
    class frame *fr;
};

union windows {
    class papers *paperswin;
    class turnin *turninwin;
    class pickup *pickupwin;
};

struct windowlist {
    struct windowlist *next;
    union windows *window;
    char *course;
};

#include <view.H>
class eos : public  view {
public:
	virtual ATKregistryEntry *ATKregistry();


  
	eos();
  
	virtual ~eos();
  
	static boolean InitializeClass();


   void LinkTree(class view *parent);
   void FullUpdate(enum view_UpdateType type, long left, long top, long width, long height);
   void Update();
   class view * Hit(enum view_MouseAction action, long x, long y, long clicks)  ;
   void PostMenus(class menulist *menulist);
   void PostKeyState(class keystate *ks);
   void ReceiveInputFocus(); 

  virtual void SetBuffer(char *filename, long flags);
  virtual char * NameBuffer()  ;
  virtual void SetTitle(char *string);
  virtual void SetCourse(char *name);
  virtual void SetFontDisplay();
  virtual void SetFontDefault();

  inline char *SetProgram(char *name) { return (strcpy((this)->program, name)); }

  class lpair         *head, *screen;
  class buffer        *editregion;
  class frame         *frame;
  class label         *title;
  class labelview     *titleV;
  class lpair         *buttons;
  struct windowlist    *handouts;
  struct windowlist    *grades;
  struct windowlist    *pickups;
  struct windowlist    *exchanges;
  struct windowlist    *turnins;
  class help          *helpwindow;
  class menulist      *menus;
  class keystate      *keys;
  char *course;
  char *program;
  int dialogpri;
  int menuflags;
  boolean gradingflag;
  class atom *paperatom;   /* used by grade for naming the 'paper' property of a buffer's data object */
  struct readWindow *displaywindow;
};


#endif /* _eos_H_ */
