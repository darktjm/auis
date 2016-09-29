#ifndef _rofftextapp_H_
#define _rofftextapp_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

/** \addtogroup librofftext
 * @{ */
/* rofftext application */

#include <application.H>
class rofftextapp : public  application {
public:
	virtual ATKregistryEntry *ATKregistry();

    
	 boolean ParseArgs(int argc,const char **argv)  ;
         boolean Start()  ;
         int Run()  ;
    
        
	rofftextapp();
    
        char *macrofile;
        boolean RoffType;
        const char *outputfile;
        const char *inputfile;
        const char **inputfiles;
        const char **argv;
        int argc;
        boolean HelpMode;
        boolean BeCompletelyBogus;
        boolean PrintWarnings;
};
/** @} */
#endif /* _rofftextapp_H_ */
