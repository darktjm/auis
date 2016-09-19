ATK_INTER
#ifndef _prefedapp_H_
#define _prefedapp_H_
/* Copyright 1992 Carnegie Mellon University All rights reserved. */


/* Dummy class so we can have the preferences editor called prefed without query replacing all the source files which reference prefs. */

#include <ezapp.H>
class prefedapp : public  ezapp {
public:
	virtual ATKregistryEntry *ATKregistry();


      
	prefedapp();
      
	virtual ~prefedapp();

	boolean ParseArgs(int argc, const char **argv)  ;
	boolean Start();
};

#endif /* _prefedapp_H_ */
