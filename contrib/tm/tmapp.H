#ifndef _tmapp_H_
#define _tmapp_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

#include <application.H>
class tmapp : public  application {
public:
	virtual ATKregistryEntry *ATKregistry();

    
	 boolean ParseArgs(int argc,const char **argv)  ;
	 boolean Start()  ;
    
        
	tmapp();
    
	const char **args;
        const char *title;
	const char *menufile;
	boolean fileMenus;
	int rows,cols;
};

#endif /* _tmapp_H_ */
