

/*LIBS: -lconsole -lerrors -lutil -lm
*/

/*
	$Disclaimer: 
 * Permission to use, copy, modify, and distribute this software and its 
 * documentation for any purpose and without fee is hereby granted, provided 
 * that the above copyright notice appear in all copies and that both that 
 * copyright notice and this permission notice appear in supporting 
 * documentation, and that the name of IBM not be used in advertising or 
 * publicity pertaining to distribution of the software without specific, 
 * written prior permission. 
 *                         
 * THE COPYRIGHT HOLDERS DISCLAIM ALL WARRANTIES WITH REGARD 
 * TO THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF 
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL ANY COPYRIGHT 
 * HOLDER BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL 
 * DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, 
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE 
 * OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION 
 * WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 * 
 *  $
*/

/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/config/COPYRITE'     *
\* ********************************************************************** */

 

/* 
 * console application class - to run dynamically loaded console with
 * shared libraries.
 */

#include <andrewos.h>
ATK_IMPL("consoleapp.H")
#include <application.H>

#include <view.H>
#include <im.H>
#include <consoleClass.H>
#include <logview.H>
#include <consoleapp.H>
#include <convers.h>
#include <proctable.H>
#include <environ.H>
#include <console.h>
/*
 * This callback is called when the window
 * manager requests to destroy the im.
 */

ATKdefineRegistry(consoleapp, application, consoleapp::InitializeClass);
static void delete_console_win(class im  *im, long  rock);


static void delete_console_win(class im  *im, long  rock)
{
     im::KeyboardExit();
}

consoleapp::consoleapp()
{
	ATKinit;

    this->consoleName = NULL;
    /* NOTE: the following defines for the version info come from ../lib/convers.h */
    (this)->SetMajorVersion( MAJORVERSION); 
    (this)->SetMinorVersion( MINORVERSION);
    THROWONFAILURE( TRUE);
}

consoleapp::~consoleapp()
{
	ATKinit;

    if(this->consoleName != NULL){
	free(this->consoleName);
    }
}

/*
 * These 3 routines are the heart of the consoleapp interface
 */
int ForceErrorMonitoring = FALSE;
int InhibitErrorMonitoring = FALSE;

boolean consoleapp::ParseArgs(int  argc,const char  **argv)
{
    if(!(this)->application::ParseArgs(argc,argv))
	return FALSE;

#define GETARGSTR(var)\
{\
    if((*argv)[2]!='\0')\
        var= ((*argv)[2]=='=' ? &(*argv)[3] : &(*argv)[2]);\
    else if(argv[1]==NULL){\
	fprintf(stderr,"%s: %s switch requires an argument.\n",(this)->GetName(),*argv);\
        return FALSE;\
    }else\
    	var= *++argv;\
}

    while(*++argv!=NULL)
	if(**argv=='-')
	    switch((*argv)[1]){
		case 'c':
		    ForceErrorMonitoring = TRUE;
		    break;
 		case 'C':
 		    InhibitErrorMonitoring = TRUE;
 		    break;
		case 'D':
		    MYDEBUGGING = TRUE;
		    mydbg(("Entering my debugging mode\n"));
		    break;
		case 'q':
		    HighestPriority = 0;
		    break;
		case 'v':
		    HighestPriority = 6;
		    break;
		case 'V':
		    HighestPriority = 9;
		    break;
		default:
		    fprintf(stderr,"%s: unrecognized switch: %s\n", (this)->GetName(), *argv);
		    return FALSE;
	    }
	else {
	    if(this->consoleName!=NULL){
		fprintf(stderr,"%s: only one console at a time\n", (this)->GetName());
		return FALSE;
	    }
	    this->consoleName = strdup(*argv);
	}

    return TRUE;
}



boolean consoleapp::Start()
{
    class consoleClass *con;
    class im *im;

    if(!(this)->application::Start())
	return FALSE;
    PostParseArgs(this->consoleName);
    con = new consoleClass;
    OneTimeInit(con);
    SetupFromConsoleFile(con,TRUE);
    InitializeGetStats(con);
    if (MaxWidth < MinWidth) MaxWidth = MinWidth;
    if (MaxHeight < MinHeight) MaxHeight = MinHeight;
    im::SetPreferedDimensions(0, 0, MaxWidth, MaxHeight);
    if ((im = im::Create(NULL)) == NULL) {
        fprintf(stderr,"Could not create new window.\nexiting.\n");
        return FALSE;
    }
    (im)->SetDeleteWindowCallback( delete_console_win, 0);
    (im)->SetView( con);
    (con)->WantInputFocus(con);
    return(TRUE);
}



boolean consoleapp::InitializeClass()
{
    const char *s;
    if ((s = environ::GetConfiguration("InhibitErrorMonitoring")) != NULL) 
	InhibitErrorMonitoring = TRUE;
    if ((s = environ::GetConfiguration("ForceErrorMonitoring")) != NULL) 
	ForceErrorMonitoring = TRUE;
    return(TRUE);
}
