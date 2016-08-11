#ifndef _nbutterv_H_
#define _nbutterv_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/config/COPYRITE'     *
\* ********************************************************************** */
/* $Header: /afs/cs.cmu.edu/project/atk-src-C++/atkams/messages/lib/RCS/nbutterv.H,v 1.1 1993/06/01 00:45:29 rr2b Stab74 $ */
/* $ACIS: $ */
/* $Source: /afs/cs.cmu.edu/project/atk-src-C++/atkams/messages/lib/RCS/nbutterv.H,v $ */

#include <sbuttonv.H>
class nbutterv : public  sbuttonv {
public:
	virtual ATKregistryEntry *ATKregistry();


     boolean Touch(int ind, enum view_MouseAction action)  ;
};


#endif /* _nbutterv_H_ */
