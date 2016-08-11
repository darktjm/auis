#ifndef _writestamp_H_
#define _writestamp_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1991 - All Rights Reserved           *
 *        For full copyright information see:'andrew/config/COPYRITE'     *
\* ********************************************************************** */
/* #include <time.h> */
#include <fontdesc.H>
#include <event.H>

#include <timeoday.H>
class writestamp: public  timeoday {
public:
	virtual ATKregistryEntry *ATKregistry();

    
      
	static boolean InitializeClass();
	static boolean atk_inited;

      
	writestamp();
      
	virtual ~writestamp();
    
       void WriteDataPart(FILE *fp);
       long ReadDataPart(FILE *fp)  ;
       void UpdateTime();
};


#endif /* _writestamp_H_ */
