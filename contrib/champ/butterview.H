#ifndef _butterview_H_
#define _butterview_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */
/** \addtogroup libchamp
 * @{ */
#include <view.H>
class butterview: public  view {
public:
	TRACED_CLASS_NODEST(butterview);

    
       void FullUpdate(enum view::UpdateType type, long left, long top, long width, long height);
       void Update();
       class view * Hit (enum view::MouseAction action, long x, long y, long numberOfClicks)  ;
};
/** @} */
#endif /* _butterview_H_ */
