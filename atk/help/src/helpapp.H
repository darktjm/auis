#ifndef _helpapp_H_
#define _helpapp_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

/*---------------------------------------------------------------------------*/
/*	MODULE: helpa.ch						     */
/*		Subclass of application, front-end for help.  Parses switches*/
/*		and creates a new help object.  Sets up a resident service   */
/*		on a socket to listen for new help connections.	             */
/*---------------------------------------------------------------------------*/

/** \addtogroup libhelp
 * @{ */
#include <application.H>
class helpapp : public  application {
public:
	virtual ATKregistryEntry *ATKregistry();

    
	 boolean ParseArgs(int argc,const char **argv)  ;
         boolean Start()  ;
	 int Run()  ;
    
	
	helpapp();
    
      short helpPort;
      class im *imp;
      class frame *framep;
      class help *helpobj;
};
/** @} */
#endif /* _helpapp_H_ */
