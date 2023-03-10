#ifndef _ezprintapp_H_
#define _ezprintapp_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */
/** \addtogroup libezprintapp Object ezprintapp
 *  A dynamically loadable ATK class file.  Contains ezprintapp.
 * @{ */
#include <application.H>
class ezprintapp : public  application {
public:
	virtual ATKregistryEntry *ATKregistry();

  static boolean InitializeClass();
      
	ezprintapp();
    
	 boolean ParseArgs(int argc,const char **argv)  ;
	 int Run()  ;
         void ReadInitFile();
};
/** @} */
#endif /* _ezprintapp_H_ */
