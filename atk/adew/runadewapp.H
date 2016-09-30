#ifndef _runadewapp_H_
#define _runadewapp_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

#include <andrewos.h>

/** \addtogroup libarbiter
 * @{ */
struct runadewapp_fileList {
    struct runadewapp_fileList *next;
    const char *filename;
    boolean newWindow;
    boolean readOnly;
};

#include <application.H>
class runadewapp: public  application {
public:
	virtual ATKregistryEntry *ATKregistry();

    
	 boolean ParseArgs(int argc, const char **argv)  ;
	 boolean Start()  ;
    
        runadewapp();
	
	static boolean InitializeClass();

        static const char ** GetArguments(int *argc)  ;
    
        struct runadewapp_fileList *files,**fileLink;
        boolean initFile;
        boolean haveBufferInWindow;
        class im *im;
        class frame *framep;
        class buffer *buffer;
	const char *title, *cls,*func;
	const char *objectname;
	};
/** @} */
#endif /* _runadewapp_H_ */
