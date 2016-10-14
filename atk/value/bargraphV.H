#ifndef _bargraphV_H_
#define _bargraphV_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1989 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

/** \addtogroup libvalue
 * @{ */
#include <sliderV.H>
class bargraphV : public  sliderV {
public:
	TRACED_CLASS_NODEST(bargraphV);


   void Drawslider(boolean fullupdate,struct rectangle *rr);
   void DrawFromScratch(long x,long y,long width,long height);

    
	bargraphV();

	long lasttop;
};
/** @} */
#endif /* _bargraphV_H_ */
