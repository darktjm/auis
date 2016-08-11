#ifndef _papers_H_
#define _papers_H_
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
#include "eos_structs.h"

#define DELETED 1

enum papers_DisplayType { papers_SIDE, papers_SIDESUBMIT, papers_ALTSIDE, papers_PLAIN, papers_SIDESUBDISP };
enum papers_Types { papersHandouts, papersGrading, papersExchange, papersNotKnown };
enum papers_Toggles { papers_OLD, papers_NEW };

#include <view.H>
class papers : public  view {
public:
	virtual ATKregistryEntry *ATKregistry();


  
	static boolean InitializeClass();

  
	papers();
  
	virtual ~papers();

   void LinkTree(class view *parent);
   void FullUpdate(enum view_UpdateType type, long left, long top, long width, long height);
   void Update();
   class view * Hit(enum view_MouseAction action, long x, long y, long clicks)  ;
   void PostMenus(class menulist *menus);
   void ReceiveInputFocus();

  virtual int ListHandouts()  ;
  virtual int ListGrade()  ;
  virtual int ListExchange()  ;
  virtual void SetTitle(char *string);
  virtual void SetDisplay(boolean buttons, enum papers_Types windowtype);
  virtual void GradingListType(enum papers_Toggles type);
  virtual void SetDefault();

  inline void SetParent(class eos *parent) { ((this)->daddy = parent); }
  inline char *SetCourse(char *name) { return (strcpy((this)->course, name)); }
  inline void SetGrading() { ((this)->menuflags |= MENUS_instructor); }

  class lpair *main, *screen;
  class lpair *submitscreen, *display;
  class lpair *buttons;
  class lpair *altbuttons;
  class lpair *xtrabuttons;
  class newbuttonview *toggle;
  class cursor *maincursor;
  class text *textobj;
  class scroll *scroll;
  class textview *textv;
  class label *title;
  class labelview *titleV;
  class menulist *menus;
  class style *style;
  class eos *daddy;
  int menuflags;
  long downdot;
  int markcount;
  char *course;
  enum papers_DisplayType wantbuttons;
  Paperlist_res *list;
  struct paperPositions *Positions;
  enum papers_Types thiswindow;
  enum papers_Toggles toggled;
  boolean IDoNotExist;
  class lpair *subdispbuttons;
  int assignment;
  char *student;
};

#endif /* _papers_H_ */
