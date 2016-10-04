#ifndef _toezapp_H_
#define _toezapp_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1991 - All Rights Reserved           *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

/** \addtogroup libtoezapp Object toezapp
 *  A dynamically loadable ATK class file.  Contains toezapp.
 * @{ */
/* toandrew application */
#include <stdio.h>
#include <application.H>
class toezapp : public  application {
public:
	virtual ATKregistryEntry *ATKregistry();



	 boolean ParseArgs(int argc, const char **argv)  ;
	 boolean Start()  ;
	 int Run()  ;


	
	toezapp();
	
	virtual ~toezapp();



	const char *inputfile;
	const char *outputfile;
	const char *inputtype;
	FILE *inf;
	FILE *outf;

	/*  vars for rofftext */
	char *macrofile;		/* /usr/lib/tmac/tmac.%s */
	boolean HelpMode;	/* ensures \n atop page */
	boolean BeCompletelyBogus;  /* only used in toandya */
};
/** @} */
#endif /* _toezapp_H_ */
