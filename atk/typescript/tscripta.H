#ifndef _typescriptapp_H_
#define _typescriptapp_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

/** \addtogroup libtypescript
 * @{ */
#include <application.H>
class typescriptapp : public  application {
public:
	virtual ATKregistryEntry *ATKregistry();


    
	typescriptapp();
    
	virtual ~typescriptapp();

     boolean ParseArgs(int argc, const char **argv)  ;
     boolean Start()  ;
     void Stop();
    const char * const *SessionCheckpoint();

    class typescript *ss;
    class frame *framep;
    int ShellMenu;
    int filemenu;
    const char *Menu;
    const char *titleLine ;
    const char *font ;
    const char **argv;
 };
/** @} */
#endif /* _typescriptapp_H_ */
