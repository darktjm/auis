#ifndef _sliderV_H_
#define _sliderV_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1989 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

/** \addtogroup libvalue
 * @{ */
#include <fontdesc.H>
#include <rect.h>

#include <valueview.H>
class sliderV : public  valueview
{
public:
	TRACED_CLASS(sliderV);

 
   void DrawFromScratch(long x,long y,long width,long height);
   void DrawHighlight();
   void DrawDehighlight();
   void DrawNewValue();
   class valueview  * DoHit(enum view::MouseAction type, long x,long y,long hits)
     ;
   view::DSattributes DesiredSize(long width, long height, enum view::DSpass pass, long *dWidth, long *dheight)  ;
   void LookupParameters();

  virtual const char * GetValueString()  ;
  virtual void Drawslider(boolean fullupdate,struct rectangle *rr);

  inline long GetTmpVal() { return (this->tmpval); }

  
	static boolean InitializeClass();

  
	sliderV();
  

  char * label;
  const char * fontname;
  char buf[32];
  short fontsize;
  class fontdesc * normalfont, * boldfont, * activefont, *valuefont;
  long maxval, minval , rv;
  long increment, tmpval,lasty;
  struct rectangle wheelrec,labelrec,valrec,inwheelrec,lastdrawn;
  long x,y,width,height,granular,gran;
  struct sbutton_prefs *prefs;
  boolean doclip;
  long sliderwidth;
  boolean immediate, immediatedefault,readonly,readonlydefault;
  int mono;
};

/* parameters to spst
   label		string		Defaults to NULL
   bodyfont		string		Defaults to "andytype"
   bodyfont-size	long		Defaults to 10
   max-value		long		defaults to 100
  min-value		long		dafaults to 0
  increment		float		defaults to 1.0
*/
/** @} */
#endif /* _sliderV_H_ */
