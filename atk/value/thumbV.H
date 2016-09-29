#ifndef _thumbV_H_
#define _thumbV_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1989 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */
/** \addtogroup libvalue
 * @{ */
#include <fontdesc.H>
#include <rect.h>

#include <valueview.H>
class thumbV : public  valueview {
public:
	virtual ATKregistryEntry *ATKregistry();

 
   void DrawFromScratch(long x,long y,long width,long height);
   void DrawHighlight();
   void DrawDehighlight();
   void DrawNewValue();
   class valueview  * DoHit(enum view_MouseAction type, long x,long y,long hits)
     ;
   void LookupParameters();

  virtual const char * GetValueString()  ;

  inline long GetTmpVal() { return (this->tmpval); }

  thumbV();
	
	static boolean InitializeClass();


  char * label;
  const char * fontname;
  short fontsize;
  char buf[32];
  class fontdesc * normalfont, * boldfont, * activefont, *valuefont;
  long maxval, minval , rv;
  long increment, tmpval,lasty;
  struct rectangle wheelrec,labelrec,valrec,inwheelrec;
  long x,y,width,height,granular,gran;
  char *foreground,*background;
  boolean immediate, immediatedefault;
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
#endif /* _thumbV_H_ */
