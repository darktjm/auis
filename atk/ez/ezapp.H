ATK_INTER
#ifndef _ezapp_H_
#define _ezapp_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/config/COPYRITE'     *
\* ********************************************************************** */

/*
	$Disclaimer: 
// Permission to use, copy, modify, and distribute this software and its 
// documentation for any purpose and without fee is hereby granted, provided 
// that the above copyright notice appear in all copies and that both that 
// copyright notice and this permission notice appear in supporting 
// documentation, and that the name of IBM not be used in advertising or 
// publicity pertaining to distribution of the software without specific, 
// written prior permission. 
//                         
// THE COPYRIGHT HOLDERS DISCLAIM ALL WARRANTIES WITH REGARD 
// TO THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF 
// MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL ANY COPYRIGHT 
// HOLDER BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL 
// DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, 
// DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE 
// OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION 
// WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
// 
//  $
*/


 


struct ezapp_fileList {
    struct ezapp_fileList *next;
    char *filename;
    char *ObjectName;
    char *newWindowProc;
    boolean readOnly;
    int	initLine;	/* new */
};

#include <application.H>

class ezapp: public  application {
public:
	virtual ATKregistryEntry *ATKregistry();

    
	 boolean ParseArgs(int argc, char **argv)  ;
         void ReadInitFile();
	 boolean Start()  ;
         int Run()  ;
    
        ezapp();
	
	static boolean InitializeClass();

    
        struct ezapp_fileList *files,**fileLink;
        boolean initFile;
        boolean haveBufferInWindow;
        boolean stdinput;
        char *stdinputObject;
	boolean first_im;
        class im *imp;
        class frame *framep;
        class buffer *bufferp;
        class buffer *errorBuffer;
	char *defaultObject;
	char *ezTitle;
};

#endif /* _ezapp_H_ */
