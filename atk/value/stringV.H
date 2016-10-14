#ifndef _stringV_H_
#define _stringV_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1989 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

/** \addtogroup libvalue
 * @{ */
#include <fontdesc.H>
#include <rect.h>

#include <valueview.H>
class stringV : public  valueview {
public:
	TRACED_CLASS_NODEST(stringV);

 
   void DrawFromScratch(long x,long y,long width,long height);
   void DrawHighlight();
   void DrawDehighlight();
   void LookupParameters();
   void DrawNewValue();
 
  stringV();
	
	static boolean InitializeClass();

 
   inline boolean GetUseAlt(){ return (this->UseAlt); }
   inline void SetUseAlt(boolean val) { (this->UseAlt = val); }
 
  const char * label;
  const char * fontname;
  short fontsize;
  class fontdesc * normalfont, * boldfont, * activefont;
  struct rectangle labelrec;
  long x,y,width,height;
  boolean UseAlt;
  char *plabel,*foreground,*background;
};

/* parameters to spst
   label		string		Defaults to NULL
   bodyfont		string		Defaults to "andytype"
   bodyfont-size	long		Defaults to 10
*/
/** @} */
#endif /* _stringV_H_ */
