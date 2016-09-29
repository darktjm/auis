#ifndef _launchapp_H_
#define _launchapp_H_
/* Copyright 1992 Carnegie Mellon University All rights reserved. */
/** \addtogroup liblaunchapp liblaunchapp.so
 *  A dynamically loadable ATK class file.  Contains launchapp.
 * @{ */
#include <application.H>
class launchapp : public  application  {
public:
	virtual ATKregistryEntry *ATKregistry();

  
    
	launchapp();
    
	virtual ~launchapp();
  
     boolean Start()  ;
     int Run()  ;
  
    char *TourFile;
    class pushbutton **buttons;
    class text **buttontext;
    class frame *framep;
    int numbuttons;
};
/** @} */
#endif /* _launchapp_H_ */
