#ifndef _checkv_H_
#define _checkv_H_

#include <fontdesc.H>
#include <rect.h>

/** \addtogroup libvalue
 * @{ */
#include <valueview.H>
class checkv : public  valueview
{
public:
	TRACED_CLASS_NODEST(checkv);

 
   void DrawFromScratch(long x,long y,long width,long height);
   void DrawHighlight();
   void DrawDehighlight();
   void DrawNewValue();
   class valueview * DoHit(enum view::MouseAction type, long x,long y,long hits)
     ;
   void LookupParameters();

  inline long GetTmpVal() { return (this->tmpval); }

  checkv();
	
	static boolean InitializeClass();


  const char *label;
  const char *fontname;
  class fontdesc *normalfont;
  class graphic *BlackPattern;
  short fontsize;
  long  tmpval;
  short checktype;
  long x,y,width,height;
  char *foreground,*background;
};
/** @} */
#endif /* _checkv_H_ */
