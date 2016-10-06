/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

#include <andrewos.h>
#include <util.h>
ATK_IMPL("application.H")


#include <im.H>
#include <init.H>
#include <environ.H>
#include <graphic.H>
#include <application.H>
#include <path.H>

#ifdef DOS_STYLE_PATHNAMES
#define DIR_SEPARATOR '\\'
#define PATH_DELIMITER ';'
#else
#define DIR_SEPARATOR '/'
#define PATH_DELIMITER ':'
#endif

/* The "startup_app" is the first application created in the
 * program (usually the *only* application).
 */
static application *startup_app = NULL;


ATKdefineRegistry(application, ATK, NULL);


static void errorProc(long rock, char  *str)
{
    fprintf(stderr, "%s", str);
    fflush(stderr);
}

application::application()
{
    this->fork=TRUE; /* do it by default */
    this->name=NULL;
    this->forceload=FALSE;
    this->readInitFile=TRUE;
    this->errorProc=::errorProc;
    this->errorRock=0;
    this->fgcolor=this->bgcolor=this->geometry=NULL;
    this->majorversion = -42;
    this->minorversion = -42;
    this->printversioninfo = TRUE;
    /* tjm - this is probably wrong, since it happens before the
     * app has a chance to set up the config */
    this->iconic=environ::GetProfileSwitch("StartIconic", FALSE);
    init_argv = NULL;
    if (startup_app == NULL)
	startup_app = this;
    THROWONFAILURE( TRUE);
}

application::~application()
{
    if(this == startup_app)
	startup_app = NULL;
}

application *application::GetStartupApplication()
{
    return startup_app;
}
const char * const *application::AppSessionCheckpoint()
{
    return (startup_app) ? startup_app->SessionCheckpoint() : NULL;
}
const char *application::AppGetName()
{
    return (startup_app) ? startup_app->GetName() : NULL;
}
void application::AppSetName(const char *s)
{
    if (startup_app) startup_app->SetName(s);
}
const char * const *application::AppGetInitialArgv()
{
    return (startup_app) ? startup_app->GetInitialArgv() : NULL;
}

/* SessionCheckpoint is called when the window system wants the application
 * to checkpoint itself.  The application should override this and
 * return a NULL terminated argv that is the command line to reinvoke the app.
 *
 * NOTE:  The caller is responsible for making a copy of the value
 *        returned here.
 */
const char * const *application::SessionCheckpoint()
{
    return init_argv;
}

/* These functions make it possible for everyone in the program
 * to find and manipulate the startup application.
 */

void application::PrintVersionNumber()
{
    if (!environ::GetProfileSwitch("PrintVersionNumber", TRUE))
	return;
    switch((this)->GetMajorVersion()){
	case -42:
	    fprintf(stderr,
		    "Starting %s (Version Unknown, ATK %s); please wait...\n",
		    (this)->GetName(), application::GetATKVersion());
	    break;
	default:
	    fprintf(stderr,
		    "Starting %s (Version %ld.%ld, ATK %s); please wait...\n",
		    (this)->GetName(),
		    (this)->GetMajorVersion(),
		    (this)->GetMinorVersion(),
		    application::GetATKVersion());
    }
    fflush(stderr);
}

/* Remember the initial command string.
 * The argument list is copied.
 */
void application::SaveInitArgv(int argc, const char **argv)
{
    int i;

    init_argv = (char **)malloc(sizeof(char *) * (argc+1));
    if (init_argv == NULL) return;	 /* oops. */
    for (i = 0; i < argc; i++) {
	if (argv[i] == NULL)
	    break;	/* The caller lied about argc! */
	init_argv[i] = strdup(argv[i]);
    }
    init_argv[i] = NULL;
}

/*
 * These 4 routines are the heart of the application interface
 */

boolean application::ParseArgs(int  argc,const char  **argv)
{
    const char *host;
    const char *pref;

    SaveInitArgv(argc, argv);
    if(this->name==NULL){
	const char *t=strrchr(*argv,'/');
	if(t==NULL)
	    this->name= *argv;
	else
	    this->name=t+1;
    }

#define GETARGSTR(var, pos)\
{\
    if((*argv)[pos]!='\0')\
        var= ((*argv)[pos]=='=' ? &(*argv)[pos+1] : &(*argv)[pos]);\
    else if(argv[1]==NULL){\
	fprintf(stderr,"%s: %s switch requires an argument.\n",(this)->GetName(),*argv);\
        return FALSE;\
    }else{\
        argc--;\
        application::DeleteArgs(argv,1);\
    	var= *argv;\
    }\
}

    im::SetDefaultIconic(this->iconic);
    while(*++argv!=NULL)
	if(**argv=='-'){
	    switch((*argv)[1]){
		case 'a':
		    if(strncmp(*argv+2, "ppname", 6) == 0) {
			GETARGSTR(this->name, 8);
			im::SetProgramName(this->name);
		    }
		    else
			continue;
		    break;
		case 'b':
		    if((*argv)[2]!='g')
			continue;

		    GETARGSTR(this->bgcolor, 3);
		    break;
		case 'd':
		    if ((*argv)[2] == '\0')  {
			this->fork=FALSE;
		    }
		    else if (strncmp(&((*argv)[2]), "isplay", 6) == 0)  {
			GETARGSTR(host, 8);
			im::SetDefaultServerHost(host);
		    }
		    else
			continue;
  		    break;
		case 'f':
		    if((*argv)[2]=='g'){
			GETARGSTR(this->fgcolor, 3);
		    }else if((*argv)[2]=='l')
			this->forceload=TRUE;
  		    else
			continue;
  		    break;
		case 'g':
		    if (strncmp(&((*argv)[2]), "eometry", 7) == 0)  {
			GETARGSTR(this->geometry, 9);
			im::SetGeometrySpec(this->geometry);
		    }
		    else
			continue;
		    break;
		case 'h':
		    if (strncmp(&((*argv)[2]), "ost", 3) == 0)  {
			GETARGSTR(host, 5);
			im::SetDefaultServerHost(host);
		    }
		    else
			continue;
		    break;
		case 'i':
		    if(strncmp(*argv+1, "iconic", 6)==0) {
			if((*argv)[7]=='o') {
			    im::SetDefaultIconic(FALSE);
			} else {
			    im::SetDefaultIconic(TRUE);
			}
		    }
		    else
			continue;
		    break;
		case 'n':
		    if((*argv)[2]=='i')
			this->readInitFile=FALSE;
  		    else
			continue;
		    break;
		case 'p':
		    if (strncmp(&((*argv)[2]), "ref", 3) == 0)  {
			GETARGSTR(pref, 5);
			environ::AddStringProfile(pref);
		    }
		    else
			continue;
		    break;
  		default:
		    /* skip over the DeleteArgs */
		    continue;
  	    }

	    application::DeleteArgs(argv--,1);
	}
    return TRUE;
}


void 
application::ReadInitFile()
{
    char buffer[256];
    const char *andrewDir, *name=this->name;
    const char *sitename;
    char *bufferp = buffer;
    class init *initp;
    boolean HadGlobalNameInit = FALSE;
    boolean siteinit = FALSE,sitegloinit = FALSE;
    const char *home;
    const char *initfilepath, *localLib;
    char initfilename[128];

    if (!this->readInitFile) return;

    initp = new init;
    im::SetGlobalInit(initp);	/* tell im about it.    -wjh
			The init can then be modified by called procs.
			(retract below if no inits found)  */

    home = environ::Get("HOME");
    andrewDir = environ::AndrewDir(NULL);

    if((sitename = environ::GetConfiguration("SiteConfigName")) != NULL){
	if ((initfilepath = environ::GetConfiguration("InitFilePath")) != NULL) {
	    sprintf(initfilename, "%s.atkinit", sitename);
	    path::FindFileInPath(bufferp, initfilepath, initfilename);
	}
	else
	    sprintf(buffer, "%s/lib/%s.atkinit", andrewDir,sitename);
	sitegloinit = (initp)->Load( buffer, this->errorProc, (long) this->errorRock,this->forceload) >= 0;
	if(name != NULL){
	    if (initfilepath != NULL) {
		sprintf(initfilename, "%s.%sinit", sitename, name);
		path::FindFileInPath(bufferp, initfilepath, initfilename);
	    }
	    else
		sprintf(buffer, "%s/lib/%s.%sinit", andrewDir,sitename, name);
	    siteinit = (initp)->Load( buffer,  this->errorProc,
				 (long) this->errorRock, this->forceload) >= 0;
	}
    }

    /* try for .NAMEinit and quit if succeed */
    if (home != NULL  &&  name != NULL)  {
	sprintf(buffer, "%s/.%sinit", home, name);
	if (((initp)->Load( buffer, this->errorProc, 
		       (long) this->errorRock, this->forceload)) >= 0) 
	    return;
    }

    /* try for LocalLib/global.NAMEinit */
    if(name != NULL && (localLib = environ::GetConfiguration("LocalLib")) != NULL){
	char lib[1000];
	char *thisStr, *nextStr;
	thisStr = lib;
	strcpy(lib, localLib);
	while (thisStr != NULL) {
	    nextStr = strchr(thisStr, PATH_DELIMITER);
	    if (nextStr != NULL) *nextStr++ = '\0';
	    sprintf(buffer, "%s/global.%sinit", thisStr, name);
	    if ((initp)->Load( buffer, this->errorProc, (long) this->errorRock, this->forceload) >= 0) {
		HadGlobalNameInit = TRUE;
		localLib = strdup(thisStr);
		break;
	    }
	    thisStr = nextStr;
	}
    }

    /* try for andrew/lib/global.NAMEinit and continue even if succeed */
    if (! HadGlobalNameInit && (name != NULL)) {
	sprintf(buffer, "%s/lib/global.%sinit", andrewDir, name);
	HadGlobalNameInit = ((initp)->Load( buffer,	this->errorProc,(long) this->errorRock, this->forceload)) >= 0;
    }

    /* try for ~/.atkinit or ~/.be2init  either alone or extending
	global.NAMEinit quit if succeed */
    if (home != NULL) {
	sprintf(buffer, "%s/.atkinit", home);
	if (((initp)->Load( buffer, this->errorProc, 
		       (long) this->errorRock, this->forceload)) >= 0) 
	    return;
	sprintf(buffer, "%s/.be2init", home);
	if (((initp)->Load( buffer, this->errorProc, 
		       (long) this->errorRock, this->forceload)) >= 0 ) 
	    return;
    }

    /* as a last resort, try for global.atkinit, but only if there was no global.NAMEinit */
    if (! HadGlobalNameInit) {
	if (localLib) {
	    sprintf(buffer, "%s/global.atkinit", localLib);
	    if (((initp)->Load( buffer, this->errorProc, (long) this->errorRock,this->forceload)) >= 0)
		return;
	}
	else {
	    sprintf(buffer, "%s/lib/global.atkinit", andrewDir);
	    if (((initp)->Load( buffer, this->errorProc, (long) this->errorRock,this->forceload)) >= 0)
		return;
	}
    }
 
    /* If we failed to find initialization, discard the data structure we might have used */
    if(!HadGlobalNameInit && !siteinit && !sitegloinit){
	im::SetGlobalInit(NULL);
	delete initp;
    }
}


boolean application::Start()
{
    const char *t;

    if(this->fgcolor==NULL)  {
	t = environ::GetProfile("ForeGroundColor");
	if (t != NULL)
	    this->fgcolor = strdup(t);
    }
    if(this->bgcolor==NULL)  {
	t = environ::GetProfile("BackGroundColor");
	if (t != NULL)
	    this->bgcolor = strdup(t);
    }

    graphic::SetDefaultColors(this->fgcolor,this->bgcolor);

    /* get a geometry spec if specified */
    if (this->geometry == NULL){
	t = environ::GetProfile("Geometry");
	if (t != NULL){
	    im::SetGeometrySpec(t);
	}
    }


    return TRUE;
}

void application::Stop()
{
}

/* this assumes interaction; maybe should be in a subclass, like
 * interactiveapp or something
 */
int application::Run()
{
    if(!(this)->Fork())
	return -1;

    im::KeyboardProcessor();

    return 0;
}

boolean application::Fork()
{
#ifdef HAVE_FORK
    if(this->fork){
	this->fork=FALSE; /* just in case */
	switch(::fork()){
	    case 0:
#if SY_AIX221 || defined(M_UNIX)
		/* Change process group so we don't get interrupts */
		NEWPGRP();
#endif
		return TRUE;
	    case -1:
		perror(this->name);
		return FALSE;
	    default:
		exit(0);
	}
    }
#endif /* !SY_OS2 */
    return TRUE;
}


/*
 * Some utility routines
 */

void application::DeleteArgs(const char  **argv,int  num)
{
    int i;

    for(i=0;i<num && argv[i]!=NULL; i++)
	;

    while(*(argv+i)!=NULL){
	*argv= *(argv+i);
	argv++;
    }

    *argv=NULL;
}

char *application::GetATKVersion()
{
    FILE *fp;
    const char *andrewDir;
    char fname[1200], *s;
    static char VerNo[40] = "ATK-No-Version-Number";
    static boolean HasChecked = FALSE;

    if (!HasChecked) {
	andrewDir = environ::AndrewDir(NULL);
	sprintf(fname, "%s/lib/atkvers.num", andrewDir);
	fp = fopen(fname, "r");
	if (!fp) return(VerNo);
	if (fgets(fname, sizeof(fname), fp) == NULL) {
	    fclose(fp);
	    return(VerNo);
	}
	fclose(fp);
	s = strchr(fname, '\n');
	if (s) *s = '\0';
	strncpy(VerNo, fname, sizeof(VerNo) - 1);
	HasChecked = TRUE;
    }
    return(VerNo);
}
