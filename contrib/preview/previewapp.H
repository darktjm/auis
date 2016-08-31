ATK_INTER
#ifndef _previewapp_H_
#define _previewapp_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/config/COPYRITE'     *
\* ********************************************************************** */
#include <application.H>
class previewapp : public  application {
public:
	virtual ATKregistryEntry *ATKregistry();

    
      
	previewapp();
    
	 boolean ParseArgs(int argc,const char **argv)  ;
         boolean Start()  ;
	 int Run()  ;
};

#endif /* _previewapp_H_ */
