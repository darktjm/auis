#ifndef _pipescriptapp_H_
#define _pipescriptapp_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

/** \addtogroup libtypescript
 * @{ */
#include <stdio.h>
#include <application.H>
class pipescriptapp : public application {
public:
	virtual ATKregistryEntry *ATKregistry();
	pipescriptapp();


     boolean ParseArgs(int argc, const char **argv)  ;
     boolean Start()  ;

    FILE *df;
    class typescript *ss;
    class frame *framep;
    int ShellMenu;
    int filemenu;
    char *Menu;
    const char *titleLine ;
    const char *font ;
    char **argv;
};
/** @} */
#endif /* _pipescriptapp_H_ */
