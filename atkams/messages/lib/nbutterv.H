#ifndef _nbutterv_H_
#define _nbutterv_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/config/COPYRITE'     *
\* ********************************************************************** */

#include <sbuttonv.H>
class nbutterv : public  sbuttonv {
public:
	virtual ATKregistryEntry *ATKregistry();


     boolean Touch(int ind, enum view_MouseAction action)  ;
};


#endif /* _nbutterv_H_ */
