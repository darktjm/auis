#ifndef _enode_H_
#define _enode_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/config/COPYRITE'     *
\* ********************************************************************** */
#include "champ.h"

#include <dataobject.H>
class enode: public dataobject {
public:
	virtual ATKregistryEntry *ATKregistry();
	enode();

    
      virtual void SetEvent(struct eventnode *event);
      virtual void SetChimp(class chimp *chimp);
    
      inline struct eventnode * GetEvent() { return (this->event); }
    
      struct eventnode *event;
      class chimp *mychimp;
};

#endif /* _enode_H_ */
