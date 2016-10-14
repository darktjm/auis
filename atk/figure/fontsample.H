#ifndef _fontsample_H_
#define _fontsample_H_
/** \addtogroup libfigure
 * @{ */
/* fontsamp.H - font sample view */
/*
	Copyright Carnegie Mellon University 1992 - All rights reserved
*/

#include <view.H>

class fontsample  : public  view {
public:
	TRACED_CLASS(fontsample);


    
      
	static boolean InitializeClass();

      
	fontsample();
      

    
       void FullUpdate(enum view::UpdateType type, long left, long top, long width, long height);
       void Update();
       class view * Hit(enum view::MouseAction action, long x, long y, long n)	 ;
       void ObservedChanged(class observable *fontsel, long status);

    
      virtual void SetString(char *val);
      virtual class fontdesc * GetFontDesc()  ;

    
      inline char  * GetString()  { return ((this)->teststring); }

    
      char *teststring;
      boolean dirty;
      class fontdesc *fdesc;
};
/** @} */
#endif /* _fontsample_H_ */
