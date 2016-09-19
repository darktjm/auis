#ifndef _chomp_H_
#define _chomp_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */
#include <dataobject.H>
class chomp: public  dataobject {
public:
	virtual ATKregistryEntry *ATKregistry();

};

#endif /* _chomp_H_ */
