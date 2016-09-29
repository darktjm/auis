#ifndef _consoleapp_H_
#define _consoleapp_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

/** \addtogroup libconsoleapp libconsoleapp.so
 *  A dynamically loadable ATK class file.  Contains consoleapp,
 *  consoleClass, logview.
 * @{ */
#include <application.H>
class consoleapp : public  application {
public:
	virtual ATKregistryEntry *ATKregistry();

    
	 boolean ParseArgs(int argc,const char **argv)  ;
	 boolean Start()  ;
    
	
	static boolean InitializeClass();

	consoleapp();
	virtual ~consoleapp();
    
        char *consoleName;
};
/** @} */
#endif /* _consoleapp_H_ */
