/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

/* driver for generic applications
 */

#include <andrewos.h>
#include <stdio.h>


#include <im.H>
#include <application.H>
#ifdef RCH_ENV
#include <ATKdebug.H>
#endif

#include <util.h>

#include <sys/time.h>
#include <sys/resource.h>

#ifndef RUNAPP
#define RUNAPP "runapp"
#endif /* RUNAPP */

#define APPSUFFIX "app"
#define DEFAULTAPP "ezapp"
#define SHORTAPPSUFFIX "a"

static void usage();
static char *leaf(char  *path);
static void stripSuffix(char  *buf,const char  *suffix);

static void usage()
{
    fprintf(stderr,"usage:\t%s [-npdD] {-l classname} appclass args...\n",RUNAPP);
    exit(1);
}

static char *leaf(char  *path)
{
    char *p;
#ifdef DOS_STYLE_PATHNAMES
    p = strrchr(path, '.');
    if (p) *p = '\0';	/* remove file extension */
    p = strrchr(path, '\\');
    if (p) return p+1;
#endif
    p=strrchr(path,'/');
    if(p==NULL)
	return path;
    else
	return p+1;
}

/* strip off the suffix, if any */
static void stripSuffix(char  *buf,const char  *suffix)
{
    char *end=buf+strlen(buf)-strlen(suffix);
    if(strcmp(suffix,end)==0)
	*end='\0';
}

int main(int  argc,char  **argv)
{
#ifdef NEED_ATKINIFINI
    ATK_DoIniFini();
#endif
    class application *app;
    char appclass[200];
    boolean NoSub=FALSE,addedSuffix;
    boolean staticLoad=TRUE;
    int exitCode = 1;

#if sys_sun3_41 || sys_sun4_41
    int fd;

    /*
     * XXX - force "/dev/zero" to be open as a file descriptor one
     * greater than the first available one, as a workaround for a
     * 4.1 bug (also present in 4.1.1) in the run-time loader.
     * (Fixed in System V Release 4, allegedly.)
     */
    fd = open("/dev/zero", O_RDWR);
    dup(fd);		/* one greater */
    close(fd);
#endif

#if defined(UNLIMIT_ENV) || defined(CMUCS)
    {
    struct rlimit rl;
    getrlimit (RLIMIT_STACK, &rl);
    rl.rlim_cur = rl.rlim_max;
    setrlimit (RLIMIT_STACK, &rl);
    getrlimit (RLIMIT_DATA, &rl);
    rl.rlim_cur = rl.rlim_max;
    setrlimit (RLIMIT_DATA, &rl);
    }
#endif

    argv[0] = leaf(argv[0]); /* Canonicalize the name of the application so we don't have to worry about it anywhere else... */

#ifdef DOS_STYE_PATHNAMES
    /* Downcase argv[0]. */
    for (char *p = argv[0]; *p; p++)
	*p = tolower(*p); 
#endif

    if(strcmp(argv[0],"whichdo")==0) {
	ATK::DynamicLoadTrace()++;
	argv++;
	if(*argv==NULL || **argv=='\0') {
	    
	    fprintf(stderr,
		    "whichdo: a classname argument is required.\n");
	    exit(1);
	}
	else {
	    doStaticLoads();
	    if(ATK::IsLoaded(*argv)) {
		printf("%s is statically loaded.\n", *argv);
	    }
	    if(ATK::DynamicLoad(*argv)==NULL) {
		fprintf(stderr,
			"whichdo: Couldn't load the class %s.\n",
			*argv);
		exit(1);
	    }
	}
	exit(0);
    }
	
    if(strcmp(argv[0],RUNAPP)==0){
	boolean prependDir=TRUE;
	if(*++argv==NULL)
	    usage();

	while(**argv=='-'){
	    switch(*++*argv){
		case 'n':
		    NoSub=TRUE;
		    break;
		case 'p':
		    prependDir=FALSE;
		    break;
		case 'd':
                    /* the hp800 doload supports different
                     * levels of debugging
		     */
		    ATK::DynamicLoadTrace()++;
		    break;
		case 'l':
		    if(staticLoad) {
		        doStaticLoads();
			staticLoad = FALSE;
		    }
		    if(*++*argv=='\0')
			argv++;
		    if(*argv==NULL || **argv=='\0')
			fprintf(stderr,
				"%s: The -l switch requires a classname as an argument.\n",
				RUNAPP);
		    else if(ATK::LoadClass(*argv)==NULL)
			fprintf(stderr,
				"%s: Couldn't load the class %s.\n",
				RUNAPP,
				*argv);
		    *argv+=strlen(*argv)-1;
		    break;
		case 'D':
		    staticLoad=FALSE;
		    break;
		default:
		    usage();
	    }

	    if(*++*argv!='\0')
		fprintf(stderr,
			"%s: switches cannot be concatenated in a single argument.\n",
			RUNAPP);

	    argv++;
	    argc--;
	}

	/* search the directory we got the app from for other do's? */
	if(prependDir){
	    char *dirEnd=rindex(*argv,'/');
	    if(dirEnd!=NULL){
		int tempChar = dirEnd[1];

		dirEnd[1]='\0'; /* temporarily. Use [1] to handle application in root correctly. */
		/* ATK_PrependClassPath(*argv); */
		dirEnd[1]=tempChar; /* restore it */
	    }
	}

	strcpy(appclass,leaf(*argv));
	argc--;

	addedSuffix=FALSE;

	stripSuffix(*argv,APPSUFFIX);
    }else{
	strcpy(appclass,leaf(*argv));
	strcat(appclass,APPSUFFIX);
	addedSuffix=TRUE;
    }

    if(staticLoad)
	doStaticLoads();

    ATK::LoadClass(appclass);
    if(!ATK::IsTypeByName(appclass,"application")) {
	if(NoSub){
		fprintf(stderr,"%s: There is no known application called %s.\n",RUNAPP,appclass);
	 	exit(1);
	} else {
	    if(addedSuffix)
		stripSuffix(appclass,APPSUFFIX);

	    ATK::LoadClass(appclass);
	    if(!ATK::IsTypeByName(appclass,"dataobject")){
		if(addedSuffix) {
		    strcpy(appclass,leaf(*argv));
		    strcat(appclass,SHORTAPPSUFFIX);
		    ATK::LoadClass(appclass);
		}
		if(!ATK::IsTypeByName(appclass,"application")) {
		    strcpy(appclass,leaf(*argv));
		    fprintf(stderr,"%s: There is no known application or datatype called %s.\n",RUNAPP,appclass);
		    exit(1);
		}
	    } else
		strcpy(appclass,DEFAULTAPP);
	}
    }

    app=(class application *)ATK::NewObject(appclass);
    if(app==NULL){
	fprintf(stderr,"%s: Error creating the %s object.\n",RUNAPP,appclass);
	exit(1);
    }
    if((app)->GetName()==NULL){
	(app)->SetName(leaf(*argv));	/* just make sure */
	im::SetProgramName(leaf(*argv));
    }
    else
	im::SetProgramName((app)->GetName());
 
      /* From now on, it's assumed that applications will print their own error messages */

    if((app)->ParseArgs(argc,(const char **)argv)){
	if((app)->GetPrintVersionFlag() == TRUE){
	    (app)->PrintVersionNumber();
	}
	(app)->ReadInitFile();
	if((app)->Start()){
	    exitCode=(app)->Run();
	    (app)->Stop();
	}
    } else {
	if((app)->GetPrintVersionFlag() == TRUE){
	    (app)->PrintVersionNumber();
	}
    }

    exit(exitCode);
}
