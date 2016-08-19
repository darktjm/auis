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

#ifndef NORCSID
#define NORCSID
static char rcsid[]="$Header: /afs/cs.cmu.edu/project/atk-src-C++/contrib/preview/RCS/previewapp.C,v 1.7 1994/11/30 20:42:06 rr2b Stab74 $";
#endif


 

/*
*
*	Main module for BE2 preview.
*		A program for previewing dvitroff input
*
*/
#include <andrewos.h>
ATK_IMPL("previewapp.H")

#include <previewapp.H>
#include <application.H>
#include <preview.H>
#include <im.H>
#include <signal.h>

const char *DviBaseName=NULL;
const char *DviFileName=NULL;
boolean DoScaling = TRUE;
FILE *filein;
boolean DviFileComplete;
boolean debug;


ATKdefineRegistry(previewapp, application, NULL);

SIGNAL_RETURN_TYPE FinishUp(int sig);


previewapp::previewapp()
{
    (this)->SetMajorVersion( 7);
    (this)->SetMinorVersion( 0);
    THROWONFAILURE( TRUE);
}

boolean previewapp::ParseArgs(int  argc,const char  **argv)
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

    while(*++argv!=NULL){
	if(**argv=='-')
	    switch((*argv)[1]){
		case 'f':
		    DoScaling = FALSE;
		    break;
		case 'o':
		    GETARGSTR(DviBaseName);
		    break;
		default:
		    fprintf(stderr,"%s: unrecognized switch: %s\n", (this)->GetName(), *argv);
		    return FALSE;
	    }
	else{
	    DviFileName= *argv;
	    if(DviBaseName==NULL)
		DviBaseName= *argv;
	    DviFileComplete = TRUE;
	    if (debug)
		fprintf(stderr, "Dvi File is %s\n", DviFileName);
	}	    
    }

    return TRUE;
}

static class preview *pv;
SIGNAL_RETURN_TYPE FinishUp(int sig)
{
    delete pv;
    signal(SIGTERM, SIG_DFL);
}

boolean previewapp::Start()
{
    class view *v;
    class im *im;
#ifdef USEFRAME
    class frame *fr;
#endif /* USEFRAME */

    if(!(this)->application::Start())
	return FALSE;

    debug = FALSE;

    if(DviFileName==NULL)
	filein=stdin;
    else{
	filein=fopen(DviFileName,"r");
	if(filein==NULL){
	    fprintf(stderr, "Can't open %s\n", DviFileName);
	    return FALSE;
	}
    }

    /*     print_StaticEntry; */
    pv = preview::Create(filein, (DviFileName==NULL ? "" : DviFileName), (DviBaseName==NULL ? "" : DviBaseName), DviFileComplete, DoScaling);
    if(pv == NULL) exit(-1);
    /* pv->debug = debug; */
    v= (pv)->GetApplicationLayer();
    if((im = im::Create(NULL)) == NULL) {
	fprintf(stderr,"preview: Could not create new window; exiting.\n");
	return(FALSE);
    }
#ifdef USEFRAME
    if((fr = new frame) == NULL) {
	fprintf(stderr,"preview: Could not allocate enough memory; exiting.\n");
	return(FALSE);
    }
    (fr)->SetView( v);
#else /* USEFRAME */
    (im)->SetView(v);
#endif /* USEFRAME */
    (pv)->WantInputFocus(pv);
#ifndef POSIX_ENV
    {
    struct sigvec termVector;
    termVector.sv_handler = SIGVECHANDLERFUNC(FinishUp);
    termVector.sv_mask = (1 << (SIGURG - 1)); /* Block redraw signals while doing cleanup. */
    termVector.sv_onstack = 0;
    if(sigvec(SIGTERM, &termVector, NULL)!=0) {
	fprintf(stderr, "preview: sigvec call failed!\n");
    }
    }
#else
    {
	struct sigaction termVector;
	termVector.sa_handler = SIGACTIONHANDLERFUNC(FinishUp);
	sigemptyset(&termVector.sa_mask);
#ifdef WM_ENV
	sigaddset(&termVector.sa_mask, SIGURG);
#endif
	if(sigaction(SIGTERM, &termVector, NULL)!=0) {
	    fprintf(stderr, "preview: sigaction call failed!\n");
	}
    }
#endif
    return TRUE;
}

int previewapp::Run()
{
    if(!(this)->Fork())
	return -1;
    while(TRUE){
	im::KeyboardProcessor();
	if((pv)->ReadyToQuit()) break;
    }
    delete pv;
    return 0;
}
