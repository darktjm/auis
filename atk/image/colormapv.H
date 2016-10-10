#ifndef _colormapv_H_
#define _colormapv_H_


#include <view.H>

class colormapv  : public  view {
public:
	virtual ATKregistryEntry *ATKregistry();




  
	static boolean InitializeClass();

  
	colormapv();
  
	virtual ~colormapv();



   void FullUpdate(enum view::UpdateType type, long left, long top, long width, long height);
   void Update();
   void PostMenus( class menulist *menulist );
   class view * Hit( enum view::MouseAction action, long x, long y, long numberOfClicks)  ;



  class suite *map, *controlPanel;
  class lpair *top;
  class color *currentColor;
};



#endif /* _colormapv_H_ */
