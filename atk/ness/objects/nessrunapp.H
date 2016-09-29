
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1991 - All Rights Reserved      *
	Copyright Carnegie Mellon University 1992, 1993 - All Rights Reserved
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

/** \addtogroup libness
 * @{ */
#ifndef _nessrunapp_H_
#define _nessrunapp_H_

#include <application.H>
class nessrunapp : public  application {
public:
	virtual ATKregistryEntry *ATKregistry();

	boolean ParseArgs(int argc, const char **argv)  ;
	boolean Start()  ;
	int Run()  ;
	
	nessrunapp();
	virtual ~nessrunapp();

	const char *inputfile;
	class ness *theNess;
	boolean dump;
};
/** @} */
#endif /* _nessrunapp_H_ */
