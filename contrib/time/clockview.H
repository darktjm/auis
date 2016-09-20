#ifndef _clockview_H_
#define _clockview_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1991 - All Rights Reserved           *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */
/** \addtogroup libclock
 * @{ */
#include <stdio.h>
#include <clock.H>
#include <menulist.H>
#include <fontdesc.H>
#include <cursor.H>

#include <view.H>
class clockview: public  view {
public:
	virtual ATKregistryEntry *ATKregistry();

 
  
	static boolean InitializeClass();
	static boolean atk_inited;

  
	clockview();
  
	virtual ~clockview();
 	
   void FullUpdate(enum view_UpdateType type, long left, long top, long width, long height);
   void Update();
   class view * Hit (enum view_MouseAction action, long x, long y, long numberOfClicks)  ;
   void Print(FILE *file, const char *processor, const char *finalFormat, boolean topLevel);
   void PostMenus(class menulist *ml);
 
  struct clock_time lastclockface;
  int need_full_update;
  class cursor *cursor;
  class fontdesc *myfontdesc;
  class menulist *ml;
  long last_options_timestamp;
};

/** @} */
#endif /* _clockview_H_ */
