ATK_INTER
#ifndef _suitex1app_H_
#define _suitex1app_H_
/* Copyright 1993 Carnegie Mellon University All rights reserved. */
/** \addtogroup libsuite
 * @{ */
#include <application.H>
class suitex1app : public  application  {
public:
	virtual ATKregistryEntry *ATKregistry();

    
       boolean Start()  ;
};
/** @} */
#endif /* _suitex1app_H_ */
