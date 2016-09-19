#ifndef _gofigapp_H_
#define _gofigapp_H_
/* ********************************************************************** *\
 *     Copyright Carnegie Mellon University, 1992 - All Rights Reserved   *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

/* version number  */
#define MAJORVERSION 1
#define MINORVERSION 6


#include <application.H>
class gofigapp : public  application {
public:
	virtual ATKregistryEntry *ATKregistry();

	 boolean ParseArgs(int argc, const char **argv)  ;
	 boolean Start()  ;
	
	static boolean InitializeClass();
	gofigapp();
	virtual ~gofigapp();

	const char *inputfile;
	class gofig *dobj;
};

#endif /* _gofigapp_H_ */
