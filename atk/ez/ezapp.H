ATK_INTER
#ifndef _ezapp_H_
#define _ezapp_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

/** \addtogroup libez libez.so
 *  A statically loadable ATK class file.  Contains ezapp.
 * @{ */
struct ezapp_fileList {
    struct ezapp_fileList *next;
    const char *filename;
    const char *ObjectName;
    char *newWindowProc;
    boolean readOnly;
    int	initLine;	/* new */
};

#include <application.H>

class ezapp: public  application {
public:
	virtual ATKregistryEntry *ATKregistry();

    
	 boolean ParseArgs(int argc, const char **argv)  ;
         void ReadInitFile();
	 boolean Start()  ;
         int Run()  ;
    
        ezapp();
	
	static boolean InitializeClass();

    
        struct ezapp_fileList *files,**fileLink;
        boolean initFile;
        boolean haveBufferInWindow;
        boolean stdinput;
        const char *stdinputObject;
	boolean first_im;
        class im *imp;
        class frame *framep;
        class buffer *bufferp;
        class buffer *errorBuffer;
	const char *defaultObject;
	const char *ezTitle;
};
/** @} */
#endif /* _ezapp_H_ */
