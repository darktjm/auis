#ifndef _enode_H_
#define _enode_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */
/** \addtogroup libchamp
 * @{ */
#include "champ.h"

#include <dataobject.H>
/** Data object associated with event editor buttons.
 *  This is used along with \ref enodeview to display and
 *  manage the event editor buttons for \ref chimpview.  While this
 *  can be used as an independent inset, it is only intended for use with
 *  \ref chimp.
 */
class enode: public dataobject {
public:
	TRACED_CLASS_NODEST(enode);
	enode();

    
      virtual void SetEvent(struct eventnode *event);
      virtual void SetChimp(class chimp *chimp);
    
      inline struct eventnode * GetEvent() { return (this->event); }
    
      struct eventnode *event;
      class chimp *mychimp;
};
/** @} */
#endif /* _enode_H_ */
