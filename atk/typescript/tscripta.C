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
static char rcsid[]="$Header: /afs/cs.cmu.edu/project/atk-src-C++/atk/typescript/RCS/tscripta.C,v 1.7 1995/11/07 20:17:10 robr Stab74 $";
#endif


 


#include <andrewos.h>
ATK_IMPL("tscripta.H")
#include <tscripta.H>
#include <application.H>
#include <typescript.H>
#include <style.H>
#include <frame.H>
#include <im.H>
#include <event.H>
#include <environ.H>
#include <fontdesc.H>
#include <framemessage.H>

ATKdefineRegistry(typescriptapp, application, NULL);

boolean typescriptapp::ParseArgs(int argc, char **argv)
{
    char *cwd = NULL;

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

    while(*++argv!=NULL && **argv=='-'){
	switch((*argv)[1]){
	    case 'c':
		GETARGSTR(cwd);
		break;
            case 's':
                this->ShellMenu=TRUE;
                break;
	    case 't':
		GETARGSTR(this->titleLine);
		break;
	    case 'F':
		this->filemenu=TRUE;
		break;
	    case 'm':
		this->ShellMenu=TRUE;
		GETARGSTR(this->Menu);
		break;
	    case 'f':
		GETARGSTR(this->font);
		break;
	    case 'p':
		/* obsolete */
		break;
	    default:
		fprintf(stderr,"%s: unrecognized switch: %s\n", (this)->GetName(), *argv);
		return FALSE;
	}
    }

    this->argv=argv;
    if(*argv==NULL)
	this->ShellMenu=TRUE;

    if (cwd)
	if (im::ChangeDirectory(cwd) == -1) {
	    fprintf(stderr, "%s: chdir to %s failed (warning)\n", GetName(), cwd);
	}

    return TRUE;
}

typescriptapp::typescriptapp()
{
    (this)->SetName( "typescript");
    this->argv=NULL;
    this->filemenu = TRUE;		/* default on--makes -F useless */
    this->titleLine = NULL;
    this->font = NULL;
    this->ShellMenu = FALSE;
    this->ss = NULL;
    this->Menu = NULL;
    (this)->SetMajorVersion( 7);
    (this)->SetMinorVersion( 2);
    THROWONFAILURE( TRUE);
}

typescriptapp::~typescriptapp()
{
}

boolean typescriptapp::Start()
{
    class view *vs;
    class im *im;
    char nbuf[256];
    FILE *df = NULL;
    char *home;

    if(!(this)->application::Start())
	return FALSE;

    if (this->Menu == NULL || (df = fopen(this->Menu, "r")) == NULL)  {
	/*
	 Uses the default shmenu if either the user has not specified a menu file
	     or the menu file specified could not be read.
	     */

	if (this->Menu != NULL)
	    perror(this->Menu);
	if (this->ShellMenu) {
	    home = environ::Get("HOME");
	    if (home != NULL)  {
		sprintf (nbuf, "%s/.shmenu", home);
		df = fopen (nbuf, "r");
	    }
	    if (df == NULL)  {
		char *fileName;

		fileName = environ::AndrewDir("/lib/shmenu");
		df = fopen (fileName, "r");
	    }
	}
    }
    this->ss = typescript::Create(this->argv,df,this->filemenu);
    if (ss == NULL)
	return FALSE;
    if (df != NULL)
	fclose(df);

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
	fprintf(stderr,"typescript: Could not allocate enough memory; exiting.\n");
	return(FALSE);
    }
    if((im = im::Create(NULL)) == NULL) {
	fprintf(stderr,"typescript: Could not create new window; exiting.\n");
	return(FALSE);
    }
    (this->framep)->SetView(vs);
    (im)->SetView( this->framep);
    (this->ss)->SetFrame( this->framep);
    if (this->titleLine != NULL) {
	(this->framep)->SetTitle( this->titleLine);
	(this->ss)->SetTitle( this->titleLine);
    }
    (this->ss)->WantInputFocus(this->ss);
    (im)->PostDefaultHandler( "message",
                           this->framep->messageLine);

    return TRUE;
}


void typescriptapp::Stop()
{
    if (this->ss)
	(this->ss)->Destroy();
    (this)->application::Stop();
}

/*
 * SessionCheckpoint is called by the im when a session manager
 * wants to "checkpoint" the state of typescript such that it
 * can be restarted.  This implementation takes the original
 * argument list and munges it to include a -c <dir> to restore
 * the current working directory.
 */
char **typescriptapp::SessionCheckpoint()
{
    char **argv;
    int i, new_argc;
    static char *new_argv[50];
    static char cwd[4096];

    argv = GetInitialArgv();
    if (argv) {
	new_argc = 0;
	/* Copy arguments, skipping any -c dir args. */
	for (i = 0; argv[i] != NULL && i < 47; i++) {
	    if (argv[i][0] == '-' && argv[i][1] == 'c') {
		/* Found a -c argument. */
		if (argv[i][2] == '\0')
		    i++;	/* skip next arg, too */
	    } else {
		/* Copy unknown argument. */
		new_argv[new_argc++] = argv[i];
	    }
	}
	new_argv[new_argc++] = "-c";
	im::GetDirectory(cwd);
	new_argv[new_argc++] = cwd;
	new_argv[new_argc] = NULL;
	return new_argv;
    }
    return NULL;
  }