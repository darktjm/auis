#ifndef _pianoV_H_
#define _pianoV_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1989 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

/** \addtogroup libvalue
 * @{ */
#include <fontdesc.H>
#include <rect.h>
#define pianov_NUMLABELS 13
#include <valueview.H>
class pianoV : public  valueview
{
public:
	TRACED_CLASS_NODEST(pianoV);

 
   void DrawFromScratch(long x,long y,long width,long height);
   void DrawHighlight();
   void DrawDehighlight();
   void DrawNewValue();
   class valueview  * DoHit(enum view::MouseAction type, long x,long y,long hits)
     ;
   void LookupParameters();

  inline long GetTmpVal() { return (this->tmpval); }

  pianoV();
	
	static boolean InitializeClass();


  char *label[pianov_NUMLABELS];
  int lsize[pianov_NUMLABELS];
  const char *fontname;
  short fontsize;
  class fontdesc * normalfont, * boldfont, * activefont, *valuefont;
  long  tmpval, lastval;
  long x,y,width,height;
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
#endif /* _pianoV_H_ */
