#ifndef _GetRecV_H_
#define _GetRecV_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1989 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

/** \addtogroup libvalue
 * @{ */
#include <rect.h>

struct GetRecV_recpair {
    struct rectangle parent,child;
};
#include <valueview.H>
class GetRecV : public  valueview
{
public:
	TRACED_CLASS_NODEST(GetRecV);

 
   void DrawFromScratch(long x,long y,long width,long height);
   void DrawHighlight();
   void DrawDehighlight();
   void DrawNewValue();
   class valueview  * DoHit(enum view::MouseAction type, long x,long y,long hits)
     ;
   view::DSattributes DesiredSize(long width, long height, enum view::DSpass pass, long *dWidth, long *dheight)  ;
   void LookupParameters();


  inline struct GetRecV_recpair * GetTmpVal() { return (this->tmpval); }

  GetRecV();
	
	static boolean InitializeClass();


   long x,y,width,height;
   long lastx,lasty,firstx,firsty;
   struct GetRecV_recpair *tmpval;
};
/** @} */
#endif /* _GetRecV_H_ */
