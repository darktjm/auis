/*LIBS: -lconsole -lerrors -lutil -lcont -lvint -lr -llwp -lm
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/config/COPYRITE'     *
\* ********************************************************************** */

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

/* 
 * console application class - to run dynamically loaded console with
 * shared libraries.
 */

#include <andrewos.h>
ATK_IMPL("vopconapp.H")
#include <application.H>
#include <view.H>
#include <im.H>
#include <consoleClass.H>
#include <logview.H>
#include <initglbs.h>
#include <vopconapp.H>
#include <convers.h>
#include <proctable.H>



ATKdefineRegistry(vopconapp, application, NULL);

vopconapp::vopconapp( )
{
    class vopconapp *self=this;
    self->consoleName = NULL;
    (self)->SetMajorVersion(self, MAJORVERSION);
    (self)->SetMinorVersion(self, MINORVERSION);
    return TRUE;
}

vopconapp::~vopconapp( )
{
    class vopconapp *self=this;
    if(self->consoleName != NULL){
	free(self->consoleName);
    }
}

/*
 * These 3 routines are the heart of the vopconapp interface
 */
int ForceErrorMonitoring = FALSE;

boolean vopconapp::ParseArgs(int  argc, const char  **argv)
{
    struct vopconapp  *self=this;
    if(!super_ParseArgs(self,argc,argv))
	return FALSE;

#define GETARGSTR(var)\
{\
    if((*argv)[2]!='\0')\
        var= ((*argv)[2]=='=' ? &(*argv)[3] : &(*argv)[2]);\
    else if(argv[1]==NULL){\
	fprintf(stderr,"%s: %s switch requires an argument.\n",(self)->GetName(self),*argv);\
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
		    fprintf(stderr,"%s: unrecognized switch: %s\n", (self)->GetName(self), *argv);
		    return FALSE;
	    }
	else {
	    if(self->consoleName!=NULL){
		fprintf(stderr,"%s: only one console at a time\n", (self)->GetName(self));
		return FALSE;
	    }
	    self->consoleName = (char *)malloc(strlen(*argv)+1);
	    strcpy(self->consoleName, *argv);
	}

    return TRUE;
}


boolean vopconapp::Start( )
{
    struct vopconapp *self=this;
    class consoleClass *con;
    class im *im;

    if(!super_Start(self))
	return FALSE;

    PostParseArgs(self->consoleName);
    con = new consoleClass;
    OneTimeInit(con);
    SetupFromConsoleFile(con,TRUE);
    if (MaxWidth < MinWidth) MaxWidth = MinWidth;
    if (MaxHeight < MinHeight) MaxHeight = MinHeight;
    im::SetPreferedDimensions(0, 0, MaxWidth, MaxHeight);
    if ((im = im::Create(NULL)) == NULL) {
        fprintf(stderr,"Could not create new window.\nexiting.\n");
        return FALSE;
    }
    (im)->SetView( con);
    (con)->WantInputFocus(con);
    return(TRUE);
}
