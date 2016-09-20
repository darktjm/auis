#ifndef _timeodayview_H_
#define _timeodayview_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1991 - All Rights Reserved           *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

#include <stdio.h>
#include <menulist.H>
#include <cursor.H>
/** \addtogroup libtimeoday
 * @{ */
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
   void Print(FILE *file, const char *processor, const char *finalFormat, boolean topLevel);
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

/** @} */
#endif /* _timeodayview_H_ */
