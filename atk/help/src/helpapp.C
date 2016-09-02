/* ********************************************************************** *
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/config/COPYRITE'     *
* ********************************************************************** */

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

/*---------------------------------------------------------------------------*/
/*									     */
/*		          	ATK Help Program			     */
/*									     */
/*	History:							     */
/*		original be2 version: Mike Kazar, c. 1985		     */
/*									     */
/*		complete ATK rewrite: Marc Pawliger, 2/89		     */
/*									     */
/*	See README for programmer details				     */
/*									     */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/*	MODULE: helpa.c							     */
/*		Subclass of application, front-end for help.  Parses switches*/
/*		and creates a new help object.  Sets up a resident service   */
/*		on a socket to listen for new help connections.	             */
/*---------------------------------------------------------------------------*/

#define label gezornenplatz
#include <andrewos.h>
ATK_IMPL("helpapp.H")
#undef label

#include <view.H>
#include <scroll.H>
#include <application.H>
#include <environ.H>
#include <frame.H>
#include "config.h"
#include <help.H>
#include <im.H>
#include <message.H>
#include <panel.H>

#include <stdio.h>


#include <sys/param.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>

#include <helpsys.h>
#include <helpapp.H>

/*---------------------------------------------------------------------------*/
/*			CONDITIONAL DEBUGGING				     */
/*---------------------------------------------------------------------------*/

#ifdef DEBUGGING
/*
 * debugging statements can be included by compiling add modules with
 * -DDEBUGGING.  Debugging output is toggled by existance/nonexistance
 * of the environment variable HELPAPPDEBUG.
 */
int HELPAPPDEBUG = 0;
#undef DEBUG
#define DEBUG(arg)  if (HELPAPPDEBUG != 0) { printf arg; fflush(stdout); }
#else
#define DEBUG(arg)
#endif

/*---------------------------------------------------------------------------*/
/*				GLOBALS					     */
/*---------------------------------------------------------------------------*/
#if 0
/* for setting up the help service */
static struct servent *ts;
static struct hostent *th;
static struct sockaddr_in myaddr;
static int helpSocket = -1;
#endif

const char *help_helpKey="";	/* the topic */
static int moreMode=FALSE;	/* use the termcap-based interface? */
static int listMode=FALSE;	/* just list files? */
static int print=FALSE;		/* in termcap-based mode, prompt for printing? */
int help_newWindow=FALSE;	/* force a new help window? */
static int noDefault=FALSE;	/* use a default file or not? */
const char *help_indexName=NULL;	/* alternative index file? */
const char *help_aliasName=NULL;	/* additional index file? */
/* list of addition search directories */
struct helpDir *helpsearchDirs=(struct helpDir *)NULL;
static const char error[] = "Sorry; no help available on '%s'.";

class help *helpobj;		/* global help object for ncproc use */

/*
 * usage statement
 */

ATKdefineRegistry(helpapp, application, NULL);

static void  show_usage(class helpapp  *self);
static void AddPath(const char  *astr);
extern "C" int help_unique();
extern "C" void helpapp_ncproc();
extern int helpSocket;

static void 
show_usage(class helpapp  *self)
{
#ifdef DEBUGGING
    fprintf(stderr,
	"Usage: %s [-dmnhlo] [-i index dir] [-a alias file] [-s dir] [topic]\n",
	    (self)->GetName());
    fprintf(stderr,"\t-d: do not fork (debug mode)\n");
#else /* !DEBUGGING */
    fprintf(stderr,
	"Usage: %s [-mnhlo] [-i index dir] [-a alias file] [-s dir] [topic]\n",
	    (self)->GetName());
#endif /* DEBUGGING */
    fprintf(stderr,
"	-h: show this usage statement\n"
"	-m: 'more' mode (for use with terminals)\n"
"	-o: in 'more' mode, prompt for printing each file\n"
"	-n: use a new help window, don't re-use old window\n"
"	-l: just list the help files available for this topic\n"
"	-e: don't show default help file\n"
"	-i directory: specify different index directory\n"
"	-a alias file: specify different help.aliases file\n"
"	-s directory: search this directory, too\n"
"	topic: the subject on which you want help.\n");
}
    

/*
 * add a path to those to be searched
 */
static void
AddPath(const char  *astr)
{
    char tname[MAXPATHLEN];
    const char *np;

    DEBUG(("ha: IN addpath\n"));
    while(1) {
	if (strlen(astr) == 0) break;
	np = strchr(astr, ':');
	if (np) {
	    strncpy(tname, astr, np-astr);
	    tname[np-astr] = 0;
	    astr = np+1;
	    help::AddSearchDir(tname);
	}
	else {
	    /* no colon, try the whole name */
	    help::AddSearchDir(astr);
	    break;
	}
    }
    DEBUG(("ha: OUT addpath\n"));
}

/*
 * contact other help servers (if any) before doing any unecessary processing.
 *
 * If another server is found, we pass in -i -a and -s switches and finally the
 * new requested help topic.  The protocal is simple: a packet length, then a
 * character representing the command (i, a, s and h), and then the text of the
 * thing to be added.
 */

extern int helpPort;
#if 0
static void 
unique_help(class helpapp  *self)
{
    int i;
    char *wmHost = NULL, *dpyHost = NULL, displayHost[MAXHOSTNAMELEN], *colon;
    int runningLocally = 0;
    static FILE *tfile;

    /* lookup the help port, default to the 'right' thing */
    if ((ts = getservbyname(IPPORT_HELPNAME, "tcp")) != (struct servent *)NULL)
	self->helpPort = ts->s_port;
    else
	self->helpPort = htons(HELPSOCK);
#ifdef WM_ENV
    wmHost = (char *) environ::Get("WMHOST");
    if (wmHost == NULL) {
	wmHost = (char *)malloc(MAXHOSTNAMELEN * sizeof(char));
	gethostname(wmHost, MAXHOSTNAMELEN);
	DEBUG(("ha: wmhost: %s\n",wmHost));
	runningLocally = 1;
    }
#endif /* WM_ENV */
#ifdef X11_ENV
    wmHost = (char *)malloc(MAXHOSTNAMELEN * sizeof(char));
    gethostname(wmHost, MAXHOSTNAMELEN);
    DEBUG(("ha: wmhost: %s\n",wmHost));
    if(dpyHost = (char *) environ::Get("DISPLAY")) {
	strcpy(displayHost, dpyHost);
	if (*displayHost && (colon = strchr(displayHost, ':')) != NULL) {
	    *colon = (char)0;
	    if(colon == displayHost || /* colon but no hostname */
	       strcmp(displayHost, wmHost) == 0 ||
	       strcmp(displayHost, "unix") == 0)
		runningLocally = 1;
	    else {
		/* here we might want to look for an existing help server that is servicing $DISPLAY */
		/* this case occurs when you have help running on another host and have your DISPLAY set back to where you are sitting; how do we determine if there is already a help proc for DISPLAY? */
		/* without doing anything special here, it'll bring up a new help window */
	    }
	    runningLocally = 1;
	    *colon = ':';
	}
    }
#endif /* X11_ENV */
    if (!help_newWindow && runningLocally) {
	/* see if we can find a help server already */
	th = gethostbyname(wmHost);
	if (th != NULL) {
	    memmove(&myaddr.sin_addr.s_addr, th->h_addr, sizeof(long));
	    myaddr.sin_family = AF_INET;
	    myaddr.sin_port = self->helpPort;
	    helpSocket = socket(AF_INET, SOCK_STREAM, 0);
	    i = connect(helpSocket, (struct sockaddr *)&myaddr, sizeof(myaddr));
	    if (i >= 0) {
		struct helpDir *thd, *nhd;

		for (thd = helpsearchDirs; thd; thd = nhd) {
		    helpapp_send_pack('s', thd->dirName, helpSocket);
		    nhd = thd->next;
		    free(thd->dirName);
		    free(thd);
		}
		if (help_indexName != NULL)
		    helpapp_send_pack('i', help_indexName, helpSocket);
		if (help_aliasName != NULL)
		    helpapp_send_pack('a', help_aliasName, helpSocket);
		helpapp_send_pack('h', help_helpKey, helpSocket);
		printf("Sent request to existing help window.\n");
		close(helpSocket);
		exit(0);
	    }
	} else
	    printf("No 'localhost' found in host table; creating new window.\n");
    }

    /*
     *setup the help server, but not if we're running on someone else's machine
     */
    if (runningLocally) {
	helpSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (helpSocket >= 0) {
	    int on;
	    
	    on = 1;
	    setsockopt(helpSocket, SOL_SOCKET, SO_REUSEADDR, (char *)&on, sizeof(on) );
	    myaddr.sin_family = AF_INET;
	    myaddr.sin_port = self->helpPort;
	    myaddr.sin_addr.s_addr = 0;		/* me, me! */
	    i = bind(helpSocket, (struct sockaddr *)&myaddr, sizeof(myaddr));
	    if (i >= 0) {
		i = listen(helpSocket, 4);
		if (i == 0) {
		    /*
		     * we've got the socket correctly setup, now listen to it
		     */
		    tfile = fdopen(helpSocket, "r");
		    if (tfile) {
			im::AddFileHandler(tfile, (im_filefptr)ncproc, 0, 0);
		    }
		}
	    }
	}
    }
}
#endif



extern char *ncproc_AddSearchDir;
extern char *ncproc_SetIndex;
extern char *ncproc_SetAliasesFile;
extern char *ncproc_GetHelpOn;

static void ncproc()
{
    char errbuf[MAXPATHLEN+sizeof(error)];
    helpapp_ncproc();
    
    if(ncproc_AddSearchDir) {
	help::AddSearchDir(ncproc_AddSearchDir);
	free(ncproc_AddSearchDir);
    }
    if(ncproc_SetIndex) {
	help::SetIndex(ncproc_SetIndex);
	free(ncproc_SetIndex);
    }
    if(ncproc_SetAliasesFile) {
	help::SetAliasesFile(ncproc_SetAliasesFile);
	free(ncproc_SetAliasesFile);
    }
    if(ncproc_GetHelpOn) {
	((help::GetInstance())->GetIM())->ExposeWindow();
	sprintf(errbuf, error, ncproc_GetHelpOn);
	help::HelpappGetHelpOn(ncproc_GetHelpOn, help_NEW, help_HIST_NAME, errbuf);
	free(ncproc_GetHelpOn);	
    }
}

static void unique_help(class helpapp *self)
{
    static FILE *tfile;
    int helpSocket=help_unique();
    if(helpSocket) {
      /* we've got the socket correctly setup, now listen to it*/
	tfile = fdopen(helpSocket, "r");
	if (tfile) {
	    im::AddFileHandler(tfile, (im_filefptr)ncproc, 0, 0);
	}
    }
}

/*
 * parse command line arguments
 */
boolean 
helpapp::ParseArgs(int  argc, const char  **argv)
{
    const char *helpPath;

    DEBUG(("ha: IN parse args\n"));

    if(!(this)->application::ParseArgs(argc,argv))
	return FALSE;

#define GETARGSTR(var)\
{\
    if((*argv)[2]!='\0')\
        var= ((*argv)[2]=='=' ? &(*argv)[3] : &(*argv)[2]);\
    else if(argv[1]==NULL){\
	fprintf(stderr,"%s: %s switch requires an argument.\n",\
		(this)->GetName(),*argv);\
        return FALSE;\
    }else\
    	var= *++argv;\
}

    if((helpPath = environ::Get("HELPPATH")) != NULL) {
	AddPath(helpPath);
    }

    while(*++argv!=NULL)
	if(**argv=='-')
	    switch((*argv)[1]){
		case 'i':
		    if (help_indexName != NULL) {
			fprintf(stderr,"%s: %s: only one index allowed\n",
				(this)->GetName(),*argv);
			exit(-1);
		    }
		    GETARGSTR(help_indexName);
		    break;
		case 'e':
		    noDefault=TRUE;
		    break;
		case 'a':
		    GETARGSTR(help_aliasName);
		    break;
		case 'l':
		    listMode=TRUE;
		case 'm':
		    moreMode=TRUE;
		    break;
		case 's':
		    GETARGSTR(helpPath);
		    if(helpPath && *helpPath != '\0')
			AddPath(helpPath);
		    break;
		case 'n':
		    help_newWindow=TRUE;
		    break;
		case 'o':
		    print=TRUE;
		    break;
		case 'h':
		case 'x':
		    show_usage(this);
		    exit(0);
		    break;
		    /*NOTREACHED*/
		default:
		    fprintf(stderr,"%s: unrecognized switch: %s\n",
			    (this)->GetName(), *argv);
		    show_usage(this);
		    return FALSE;
	    } else {
		help_helpKey = *argv;
		DEBUG(("ha: key: %s\n",help_helpKey));
	}

    return TRUE;
}

boolean 
helpapp::Start()
{
    const char *tp;
    struct helpDir *thd, *nhd;

    DEBUG(("ha: IN start\n"));
    if(!(this)->application::Start())
	return FALSE;

    /* try to determine if we're not under a window manager */
    if (((tp = environ::Get("WMHOST")) == NULL || *tp == '\0') &&
	((tp = environ::Get("DISPLAY")) == NULL || *tp == '\0') &&
	((tp = environ::Get("TERM")) == NULL || strcmp(tp, "wm") != 0))  {
	moreMode = TRUE;
	this->helpobj = new help;
    }

    if (!moreMode) {
        unique_help(this);
    }

    /* allow us to 'see' the frame proctable, so we can use bind frame procs */
    ATK::LoadClass("framecmds");
    
    if (!moreMode) {

	this->imp = im::Create(NULL);		/* default window */
	if (!this->imp) {
	    fprintf(stderr,"%s: failed to create new window; exiting.\n",
		    (this)->GetName());
	    exit(1);
	}

	this->helpobj = new help;
	
	this->framep = new frame;

	if (!this->helpobj || !this->framep) {
	    fprintf(stderr,"%s: Could not initialize help properly; exiting.\n",
		    (this)->GetName());
	    exit(1);
	}
	/* frame for frame_SetView must have associated im */
	this->helpobj->app = (class scroll*) (this->helpobj)->GetApplicationLayer();
	(this->framep)->SetView( this->helpobj->app);
	(this->imp)->SetView( this->framep);

	/* add in a message handler */
	(this->framep)->PostDefaultHandler( "message",
				 (this->framep)->WantHandler( "message"));
    }

    for (thd = helpsearchDirs; thd; thd = nhd) {
	help::AddSearchDir(thd->dirName);
	nhd = thd->next;
	free(thd->dirName);
	free(thd);
    }
    DEBUG(("key: '%s' nodef: %d\n", help_helpKey, noDefault));
    if ((!help_helpKey || !(*help_helpKey)) && !noDefault) {
	if(moreMode) {
	    help_helpKey=NONWMDEFAULTFILE;
	} else help_helpKey=WMDEFAULTFILE;
	DEBUG(("ha: nodef: %s\n",help_helpKey));
    }
    if ((tp = environ::GetProfile("SearchPath")) != NULL)
	/* add all the elements of this path to the help list */
	AddPath(tp);

    /* specify alternative index to use for debugging */
    if (help_indexName)
	help::SetIndex(help_indexName);

    /* specify additional help.aliases file */
    if (help_aliasName) {
	DEBUG(("ha: setting alias: %s\n",help_aliasName));
	help::SetAliasesFile(help_aliasName);
    }

    if (moreMode) {
	help::GetHelpOnTerminal(help_helpKey, listMode, print);
	exit(0);
    }

    DEBUG(("ha: OUT start\n"));
    return TRUE;
}

int 
helpapp::Run()
{
    char tbuffer[200];
    int code;

    DEBUG(("ha: IN run\n"));

    DEBUG(("ha: window made\n"));
    
    sprintf(tbuffer, error, help_helpKey);
    code = help::HelpappGetHelpOn(help_helpKey, help_NEW, help_HIST_NAME, tbuffer);
    if (code == 0) {
	/* couldn't get help on that item, and it wasn't because of a server,
	   so show the default file, which we 'know' exists */
	sprintf(tbuffer, error, WMDEFAULTFILE);
	help::HelpappGetHelpOn(WMDEFAULTFILE, help_NEW, help_HIST_NOADD, tbuffer);
    }
    (this)->application::Run();
    DEBUG(("ha: OUT run\n"));
    return(0);
}

helpapp::helpapp()
{
    (this)->SetMajorVersion( MAJOR_VERSION);
    (this)->SetMinorVersion( MINOR_VERSION);
#ifdef DEBUGGING
    if ((char *)environ::Get("HELPAPPDEBUG") != (char *) NULL)
	HELPAPPDEBUG = 1;
#endif /* DEBUGGING */
    this->helpPort = 0;
    THROWONFAILURE( TRUE);
}
