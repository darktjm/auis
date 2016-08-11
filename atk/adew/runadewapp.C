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
static char rcsid[]="$Header: /afs/cs.cmu.edu/project/atk-src-C++/atk/adew/RCS/runadewapp.C,v 1.5 1994/12/06 21:54:28 rr2b Stab74 $";
#endif


 

#include <andrewos.h> /* sys/types.h sys/file.h */
ATK_IMPL("runadewapp.H")

#include <andyenv.h>


#include <im.H>
#include <frame.H>
#include <dataobject.H>
#include <buffer.H>
#include <message.H>
#include <environ.H>
#include <application.H>
#include <proctable.H>

#include <runadewapp.H>


static char **Gargv;
static int Gargc;
extern int errno;


ATKdefineRegistry(runadewapp, application, runadewapp::InitializeClass);
#ifndef NORCSID
#endif
static void StartupError( char  *string);
static void addFile(class runadewapp  *self,char  *name,boolean  newWin,boolean  ro);
static char *getarg(char  **argv,int  *argc);


runadewapp::runadewapp()
{
	ATKinit;

    this->initFile=TRUE;
    this->files=NULL;
    this->fileLink= &this->files;
    this->haveBufferInWindow = FALSE;
    this->im = NULL;
    this->framep = NULL;
    this->buffer = NULL;
    (this)->SetMajorVersion( 7);
    (this)->SetMinorVersion( 0);
    this->cls = NULL;
    this->func = "go";
    this->title = NULL;
    this->objectname = NULL;
    THROWONFAILURE( TRUE);
}


static void StartupError( char  *string)
    {
	fprintf(stderr,"%s\n",string);
	fflush(stderr);
}


static void addFile(class runadewapp  *self,char  *name,boolean  newWin,boolean  ro)
{
    struct runadewapp_fileList *fileEntry=
      (struct runadewapp_fileList *) malloc(sizeof(struct runadewapp_fileList));

    fileEntry->filename=name;
    fileEntry->newWindow=newWin;
    fileEntry->readOnly=ro;
    fileEntry->next=NULL;
    *self->fileLink=fileEntry;
    self->fileLink=(&(fileEntry->next));
}
static char *getarg(char  **argv,int  *argc)
{
    int cnt;
    char *opt;
    if(argv[0][2] == '\0'){
	opt = argv[1];
	cnt = 2;
    }
    else {
	opt = (*argv) + 2;
	cnt = 1;
    }
    application::DeleteArgs(argv,cnt);
    *argc -= cnt;
    return opt;
}
boolean runadewapp::ParseArgs(int  argc,char  **argv)
{
    int maxInitWindows=environ::GetProfileInt("MaxInitWindows", 2);
    boolean useNewWindow = FALSE;
    boolean pendingReadOnly = TRUE;
    Gargc = argc;
    Gargv = argv;

    if(!(this)->application::ParseArgs(argc,argv))
	return FALSE;


    if (maxInitWindows < 2)
	maxInitWindows = 1;
    argv++;
    while(*argv!=NULL){
	if(**argv=='-')
	    switch((*argv)[1]){
		case 'C': 
		    this->cls = getarg(argv,&Gargc);
		    break;
		case 'F':
		    this->func =getarg(argv,&Gargc);
		    break;
		case 'T': 
		    this->title =getarg(argv,&Gargc);
		    break;
		case 'S':
		    addFile(this,
			    getarg(argv,&Gargc),
			    (useNewWindow || maxInitWindows-->0),
			    pendingReadOnly);
		    break;		
		case 'I':
		    this->objectname = getarg(argv,&Gargc);
		    break;
		default:
		    argv++;
	    }
	else{
	    argv++;
	}
    }
    return TRUE;
}



boolean runadewapp::Start()
{
    struct runadewapp_fileList *fileEntry, *next;
    char iname[256];
    struct proctable_Entry *pr;
    proctable_fptr proc;
    if(!(this)->application::Start())
	return FALSE;
    if(this->objectname){
	if((this->buffer = buffer::Create("", NULL, this->objectname, NULL)) == NULL){
	    fprintf(stderr,"adew: Could not create a buffer with %s; exiting.\n",this->objectname);
	    exit(-1);
	}

	if((this->framep = new frame) == NULL) {
	    fprintf(stderr,"adew: Could not allocate enough memory; exiting.\n");
	    exit(-1);
	}
	if((this->im = im::Create(NULL)) == NULL) {
	    fprintf(stderr,"adew: Could not create new window; exiting.\n");
	    exit(-1);
	}
	(this->im)->SetView( this->framep);
	(this->framep)->PostDefaultHandler( "message", (this->framep)->WantHandler( "message"));
	(this->framep)->SetBuffer( this->buffer, TRUE);
	(this->buffer)->SetScratch(TRUE);
	(this->framep)->SetCommandEnable( FALSE);
	if(this->title)
	    (this->framep)->SetTitle(this->title);
	this->haveBufferInWindow = TRUE;
    }
    else for (fileEntry = this->files; fileEntry != NULL; fileEntry = next) {
        this->buffer = buffer::GetBufferOnFile(fileEntry->filename, fileEntry->readOnly ? buffer_ReadOnly : 0);

	if (this->buffer != NULL && fileEntry->newWindow) {
	    if((this->framep = new frame) == NULL) {
		fprintf(stderr,"adew: Could not allocate enough memory; exiting.\n");
		exit(-1);
	    }
	    if((this->im = im::Create(NULL)) == NULL) {
		fprintf(stderr,"adew: Could not create new window; exiting.\n");
		exit(-1);
	    }
	    (this->im)->SetView( this->framep);
	    (this->framep)->PostDefaultHandler( "message", (this->framep)->WantHandler( "message"));
	    (this->framep)->SetBuffer( this->buffer, TRUE);
	    (this->buffer)->SetScratch(TRUE);
	    (this->framep)->SetCommandEnable( FALSE);
	    if(this->title)
		(this->framep)->SetTitle(this->title);
	    im::ForceUpdate();
	    if(this->cls){
		strcpy(iname,this->cls);
		if(ATK::LoadClass(iname) != NULL){
		    strcat(iname,"-");
		    strcat(iname,this->func);
		    if((pr = proctable::Lookup(iname)) != NULL && proctable::Defined(pr) ){
			proc = proctable::GetFunction(pr) ;
			(*proc)((this->framep)->GetView(),0);
		    }
		}
	    }
	    this->haveBufferInWindow = TRUE;
	}
        else {
            char errorMessage[200];
            sprintf(errorMessage, "File %s does not exist and could not be created.", fileEntry->filename);
            StartupError(errorMessage);
        }
        next = fileEntry->next;
        free(fileEntry);
    }

    if(this->haveBufferInWindow == FALSE){
	return FALSE;
    }
    return TRUE;
}


boolean runadewapp::InitializeClass()
{
    Gargv = NULL;
    Gargc = 0;
    return TRUE;
}
char **runadewapp::GetArguments(int  *argc)
{
	ATKinit;

    if(argc != NULL) *argc = Gargc;
    return Gargv;
}
