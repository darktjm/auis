#ifndef _nullapp_H_
#define _nullapp_H_
/* ********************************************************************** *\
 *     Copyright Carnegie Mellon University, 1992 - All Rights Reserved   *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

/* version number  */
#define MAJORVERSION 1
#define MINORVERSION 6


#include <application.H>
class nullapp : public  application {
public:
	virtual ATKregistryEntry *ATKregistry();

	 boolean ParseArgs(int argc, const char **argv)  ;
	 boolean Start()  ;
	
	static boolean InitializeClass();
	nullapp();
	virtual ~nullapp();

	const char *inputfile;
	class null *dobj;
};

#endif /* _nullapp_H_ */
