ATK_INTER
#ifndef _describer_H_
#define _describer_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */
#include <view.H>

#include <ATK.H>

class describer : public ATK  {
public:
	virtual ATKregistryEntry *ATKregistry();


    virtual  view_DescriberErrs Describe(class view * viewToDescribe, char * format, FILE * file, long rock)  ;
};

#endif /* _describer_H_ */
