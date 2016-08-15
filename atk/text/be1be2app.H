ATK_INTER
#ifndef _be1be2app_H_
#define _be1be2app_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/config/COPYRITE'     *
\* ********************************************************************** */
/*
 * BE1 to BE2 conversion utility
 */

#include <application.H>

class be1be2app: public  application {
public:
	virtual ATKregistryEntry *ATKregistry();


     boolean ParseArgs(int argc, const char **argv)  ;
     boolean Run()  ;
};

#endif /* _be1be2app_H_ */
