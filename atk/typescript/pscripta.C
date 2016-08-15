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

#include <andrewos.h>

#ifndef NORCSID
#define NORCSID
static UNUSED const char rcsid[]="$Header: /afs/cs.cmu.edu/project/atk-src-C++/atk/typescript/RCS/pscripta.C,v 1.6 1994/11/30 20:42:06 rr2b Stab74 $";
#endif


 


ATK_IMPL("pscripta.H")
#include <pscripta.H>
#include <application.H>
#include <typescript.H>
#include <style.H>
#include <frame.H>
#include <im.H>
#include <fontdesc.H>

static FILE *outfile = NULL;


ATKdefineRegistry(pipescriptapp, application, NULL);
#ifndef NORCSID
#endif


boolean pipescriptapp::ParseArgs(int  argc,const char  **argv)
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

    while(*++argv!=NULL && **argv=='-')
	switch((*argv)[1]){
	    case 't':
		GETARGSTR(this->titleLine);
		break;
            case 'f':
		GETARGSTR(this->font);
		break;
	    case 'o':
		outfile = stdout;
		break;
	    default:
		fprintf(stderr,"%s: unrecognized switch: %s\n", (this)->GetName(), *argv);
		return FALSE;
	    }

    if(*argv!=NULL){
	fprintf(stderr,"%s: unrecognized argument: %s\n", (this)->GetName(), *argv);
	return FALSE;
	}

 return TRUE;
}

boolean pipescriptapp::Start()
{
    class view *vs;
    class im *im;

    if(!(this)->application::Start())
	return FALSE;

    this->ss =typescript::CreatePipescript(stdin, outfile, TRUE);

    if (this->font != NULL) {
        char family[256];
        long size, stylel;
        class style *defaultStyle = (this->ss)->GetDefaultStyle();

        if (defaultStyle == NULL)
            defaultStyle = new style;
        if (fontdesc::ExplodeFontName(this->font, family, sizeof(family), &stylel, &size)) {
            (defaultStyle)->SetFontFamily( family);
            (defaultStyle)->SetFontSize( style_ConstantFontSize, size);
            (defaultStyle)->ClearNewFontFaces();
            (defaultStyle)->AddNewFontFace( stylel);
            (this->ss)->SetDefaultStyle( defaultStyle);
        }
    }

    vs = (this->ss)->GetApplicationLayer();
    if((this->framep = new frame) == NULL) {
	fprintf(stderr,"pipescript: Could not allocate enough memory; exiting.\n");
	return(FALSE);
    }
    if((im = im::Create(NULL)) == NULL) {
	fprintf(stderr,"pipescript: Could not create new window; exiting.\n");
	return(FALSE);
    }
    if (this->titleLine != NULL)
	(this->framep)->SetTitle( this->titleLine);
    (im)->SetView( this->framep);
    (this->framep)->SetView(vs);
    (this->ss)->WantInputFocus(this->ss);

    return TRUE;
}

pipescriptapp::pipescriptapp()
{
    (this)->SetName( "pipescript");
    this->filemenu = 1;
    this->titleLine = NULL;
    this->font = NULL;
    this->ShellMenu = 0;
    this->df = NULL;
    this->ss = NULL;
    this->Menu = NULL;
    this->framep = NULL;
    this->argv=NULL;
    (this)->SetMajorVersion( 7);
    (this)->SetMinorVersion( 2);

    THROWONFAILURE( TRUE);
}
