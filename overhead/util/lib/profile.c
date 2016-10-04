/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

#include <andrewos.h>		/* sys/types.h */
#include <fdplumb.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/param.h>	/* For MAXPATHLEN */
#include <pwd.h>
#include <ctype.h>
#include <util.h>

#ifdef DOS_STYLE_PATHNAMES
#define DEFAULTPROFILES "~/preferences;~/.preferences;~/.Xdefaults" 
#else
#define DEFAULTPROFILES "~/preferences:~/.preferences:~/.Xdefaults" 
#endif
#define GLOBALPRFNAME "global.prf"
#define GLOBALPROFILE AndrewDir("/lib/"GLOBALPRFNAME)

static struct configurelist *profileHead = NULL;
static struct configurelist *StringProfileHead = NULL;
static struct configurelist *GloprofileHead = NULL;

static  int inited = 0;  /* Used to be static local to openprofile -- nsb */
static char *profileFileName = NULL;
static char *firstProfileFileName = NULL;

/* open a list of profile files, with an environment variable that can override
 * the default list.  "savefname" flag added 12/13/91 by cn0h
 */
static struct configurelist *
openprofile(const char *filename, const char *defaultname, int savefname)
{
    char *pl0, *pl=getenv(filename);
    const char *home=(char *) gethome(NULL);
    char *sep;
#ifdef DOS_STYLE_PATHNAMES
    char  sepchar = ';';
#else
    char sepchar = ':';
#endif
    int homelen=(home==NULL ? 0 : strlen(home));
    struct configurelist *cl;
    char tmpFileName[MAXPATHLEN];

    if(pl && *pl)
	pl = strdup(pl);
    else
	pl = NULL;
    if(pl==NULL) {
	/* Only check InitFilePath for Global profiles */
	if (!strcmp(filename, "GLOBALPROFILES")) {
	    /* Look for GLOBALPRFNAME in InitFilePath (if it exists), else use default */	
	    const char *initpath = GetConfiguration("InitFilePath");
	    if (initpath != NULL && *initpath!='\0') {
		pl = malloc(MAXPATHLEN);
		if(pl) findfileinpath(pl, initpath, GLOBALPRFNAME);
		if (*pl == '\0') {
		    free(pl);
		    pl = NULL;
		}
	    }
	}
    }
    if(pl==NULL)
	pl=strdup(defaultname);
    pl0 = pl;

    do{
	const char *name;
	int namelen;

	sep=strchr(pl,sepchar);

	name = pl;

	if(sep!=NULL){
	    namelen = sep - pl;
	    pl=sep+1;
	}
	else
	    namelen=strlen(name);

	if(*name=='~' && (name++,namelen--,homelen>0)){
	    strcpy(tmpFileName,home);
	    strncat(tmpFileName,name, namelen);
	    tmpFileName[namelen + homelen] = '\0';
	}
	else {
	    strncpy(tmpFileName,name, namelen);
	    tmpFileName[namelen] = '\0';
	}

	if (savefname && firstProfileFileName == NULL)  {
	    firstProfileFileName = strdup(tmpFileName);
	}

	if ((cl = (struct configurelist *) ReadConfigureFile(tmpFileName)) != NULL)  {
	    if (savefname) {
		if (profileFileName != NULL)  {
		    free(profileFileName);
		}
		profileFileName = strdup(tmpFileName);
	    }
	    free(pl0);
	    return cl;
	}

    } while(sep!=NULL);
    free(pl0);

    return NULL;

}


static void init_profile(void)
{
    if (! inited)  {
	profileHead = openprofile("PROFILES", DEFAULTPROFILES, 1);
	GloprofileHead = openprofile("GLOBALPROFILES", GLOBALPROFILE, 0);
	inited = 1;
    }
}

const char *GetProfileFileName(void)
{
    init_profile();
    return profileFileName;
}

const char *GetFirstProfileFileName(void)
{
    init_profile();
    return firstProfileFileName;
}

void refreshprofile(void) {  /* Force rereading */
    if (profileHead != NULL)  {
	FreeConfigureList(profileHead);
	profileHead = NULL;
    }
    if (profileFileName != NULL)  {
	free(profileFileName);
	profileFileName = NULL;
    }
    if (firstProfileFileName != NULL) {
	free(firstProfileFileName);
	firstProfileFileName = NULL;
    }
    inited = 0;
}

/* Add a string to the front of the stringprofile list.
 */
void addstringprofile(char *s)
{
    struct configurelist *cl;

    cl = (struct configurelist *)ReadStringConfig(s);
    if (cl) {
	cl->next = StringProfileHead;
	StringProfileHead = cl;
    }
}

const char *getprofile (const char *var)
{
    const char *retval;

    init_profile();
#ifdef GLOBALPREFERENCE
/* check for exact match in string profile */
    if((retval = (char *) GetConfig(StringProfileHead, var, 0)) != NULL)
	return retval;
/* check for exact match in users profile */
    if((retval = (char *) GetConfig(profileHead, var, 0)) != NULL)
	return retval;
/* check for exact match in global profile */
    if((retval = (char *) GetConfig(GloprofileHead, var, 0)) != NULL)
	return retval;
#endif
/* check for unexact match in string profile */
    if((retval = (char *) GetConfig(StringProfileHead, var, 1)) != NULL)
	return retval;
/* check for unexact match in users profile */
    if((retval = (char *) GetConfig(profileHead, var, 1)) != NULL)
	return retval;
/* check for unexact match in global profile */
    return (char *) GetConfig(GloprofileHead, var, 1) ;
}

int getprofileswitch (const char *var, int DefaultValue)
{
    const char   *val;
    static const struct keys {
	const char   *name;
	int     value;
    }                   keys[] = {
	                    { "true", 1},
	                    {"false", 0},
	                    {"on", 1},
	                    {"off", 0},
	                    {"yes", 1},
	                    {"no", 0},
	                    {"1", 1},
	                    {"0", 0},
	                    {0, 0}
    };
    const struct keys   *p;
    if (var && (val = getprofile (var))) {
	for (p = keys; p -> name; p++)
	    if (FOLDEDEQ(p->name, val))
		return p -> value;
    }
    return DefaultValue;
}

int getprofileint (const char *var, int DefaultValue)
{
    const char  *val;
       int n = 0;
       int neg = 0;

    if (var == 0 || (val = getprofile(var)) == 0)  {
	return DefaultValue;
    }
    while (*val) {
	if (*val == '-')
	    neg = ~neg;
	else {
	    if (*val != ' ' && *val != '\t') {
		if ('0' <= *val && *val <= '9')
		    n = n * 10 + *val - '0';
		else  {
		    return DefaultValue;
		}
	    }
	}
	val++;
    }
    return neg ? -n : n;
}

int profileentryexists(const char *var, int usedefault)
{

    init_profile();

    return (var != NULL && ( (GetConfig(StringProfileHead, var, usedefault) != NULL) || 
			     (GetConfig(profileHead, var, usedefault) != NULL) || 
			     (GetConfig(GloprofileHead, var, usedefault) != NULL)));
}
