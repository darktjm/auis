#ifndef _circlepi_H_
#define _circlepi_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1991 - All Rights Reserved           *
 *        For full copyright information see:'andrew/config/COPYRITE'     *
\* ********************************************************************** */

#include <dataobject.H>
class circlepi: public  dataobject {
public:
	virtual ATKregistryEntry *ATKregistry();

  
    
	static boolean InitializeClass();
	static boolean atk_inited;

    
	circlepi();
    
	virtual ~circlepi();
  
    virtual void SetDepth(int limit);
  
    inline int   GetDepth() { return (this->depth_limit); }
  
    int depth_limit;
};


#endif /* _circlepi_H_ */
