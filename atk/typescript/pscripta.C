/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

#include <andrewos.h>
ATK_IMPL("pscripta.H")
#include <pscripta.H>
#include <application.H>
#include <typescript.H>
#include <style.H>
#include <frame.H>
#include <im.H>
#include <fontdesc.H>

static FILE *outfile = NULL;


ATKdefineRegistryNoInit(pipescriptapp, application);

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
