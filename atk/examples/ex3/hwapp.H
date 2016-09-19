ATK_INTER
#ifndef _helloworldapp_H_
#define _helloworldapp_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */
#include <application.H>
class helloworldapp : public  application{
public:
	virtual ATKregistryEntry *ATKregistry();

    
	 boolean Start()  ;
};

#endif /* _helloworldapp_H_ */
