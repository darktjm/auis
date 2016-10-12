#ifndef _helptextview_H_
#define _helptextview_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

/** \addtogroup libframe
 * @{ */
#include <textview.H>

class helptextview : public  textview {
public:
	virtual ATKregistryEntry *ATKregistry();

    
       void WantInputFocus( class view *requestor);
       void GetClickPosition(long position, long numberOfClicks, enum view::MouseAction action, long startLeft, long startRight, long *leftPos, long *rightPos);
       class view * Hit (enum view::MouseAction action, long x, long y, long numberOfClicks)  ;
       void FullUpdate(enum view::UpdateType type, long left, long top, long width, long height);
    
      
	virtual ~helptextview();
      
	helptextview();    
    
      class cursor *myCursor;

};
/** @} */
#endif /* _helptextview_H_ */
