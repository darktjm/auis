#ifndef _tm19_H_
#define _tm19_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

/*
 * subclass of termulator implementing h19 escape sequences
 */

#include <termulator.H>
class tm19: public  termulator {
public:
	virtual ATKregistryEntry *ATKregistry();

    
        
	tm19();
    
         char * GetTermcap()  ;
         char * GetTerm()  ;
    
        int savedX,savedY;
        int foldCount;
};

#endif /* _tm19_H_ */
