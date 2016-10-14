#ifndef _circlepi_H_
#define _circlepi_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1991 - All Rights Reserved           *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */
/** \addtogroup libcirclepi Object circlepi
 *  A dynamically loadable ATK class file.  Contains circlepi circlepiview.
 * @{ */
#include <dataobject.H>
class circlepi: public  dataobject {
public:
	TRACED_CLASS(circlepi);

  
    
	static boolean InitializeClass();
	static boolean atk_inited;

    
	circlepi();
    
  
    virtual void SetDepth(int limit);
  
    inline int   GetDepth() { return (this->depth_limit); }
  
    int depth_limit;
};

/** @} */
#endif /* _circlepi_H_ */
