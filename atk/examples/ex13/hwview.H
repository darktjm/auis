#ifndef _helloworldview_H_
#define _helloworldview_H_
/* 
	Copyright IBM Corporation 1988,1991 - All Rights Reserved
	Copyright Carnegie Mellon University 1996
	For full copyright information see: <src or dest>/doc/COPYRITE
*/

/* Mouse drag, key bindings, menus, scroll bar, data object, messages, prompting for input, GetApplicationLayer */

#include <view.H>
class helloworldview: public  view {
public:
  TRACED_CLASS_NODEST(helloworldview);


  void FullUpdate(enum view::UpdateType type, long left, long top, long width, long height);
  void Update();
  class view * Hit (enum view::MouseAction action, long x, long y, long numberOfClicks)  ;
  void ReceiveInputFocus();
  void LoseInputFocus();
  const void * GetInterface(const char *type);
  class view * GetApplicationLayer();
  void DeleteApplicationLayer(class view *v);

  helloworldview();

  static boolean InitializeClass();

  class keystate *keystate;
  class menulist *menulistDup;
  boolean HaveDownTransition;
  boolean haveInputFocus;
  long hitX,hitY;
  long x,y;
  boolean blackOnWhite;
  long frameX, frameY;
  long newFrameX, newFrameY;
  int vrWidth,vrHeight;
  boolean didMove;
};

#endif /* _helloworldview_H_ */
