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

#include <andrewos.h>		/* sys/types.h */

#ifndef NORCSID
#define NORCSID
static UNUSED const char rcsid[]="$Header: /afs/cs.cmu.edu/project/atk-src-C++/overhead/util/lib/RCS/profile.c,v 2.21 1995/11/07 20:17:10 robr Stab74 $";
#endif


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
    char *pl=getenv(filename);
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

    if(pl)
	pl = strdup(pl);
    if(pl==NULL || *pl=='\0') {
	/* Only check InitFilePath for Global profiles */
	if (!strcmp(filename, "GLOBALPROFILES")) {
	    /* Look for GLOBALPRFNAME in InitFilePath (if it exists), else use default */	
	    const char *initpath = GetConfiguration("InitFilePath");
	    if (initpath != NULL && *initpath!='\0') {
		pl = malloc(MAXPATHLEN);
		if(pl) findfileinpath((char *)pl, initpath, GLOBALPRFNAME);
		if (pl == NULL || *pl == '\0')
		    pl=strdup(defaultname);
	    }
	    else pl=strdup(defaultname);
	}
	else pl=strdup(defaultname);
    }

    do{
	const char *name;
	int namelen;

	sep=(char *)index(pl,sepchar);

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
	    firstProfileFileName = (char *) malloc(strlen(tmpFileName) + 1);
	    strcpy(firstProfileFileName, tmpFileName);
	}

	if ((cl = (struct configurelist *) ReadConfigureFile(tmpFileName)) != NULL)  {
	    if (savefname) {
		if (profileFileName != NULL)  {
		    free(profileFileName);
		}
		profileFileName = (char *) malloc(strlen(tmpFileName) + 1);
		strcpy(profileFileName, tmpFileName);
	    }
	    return cl;
	}

    } while(sep!=NULL);
    free(pl);

    return NULL;

}

const char *GetProfileFileName()
{
    if (! inited)  {
	profileHead = openprofile("PROFILES", DEFAULTPROFILES, 1);
	GloprofileHead = openprofile("GLOBALPROFILES", GLOBALPROFILE, 0);
	inited = 1;
    }

    return profileFileName;
}

const char *GetFirstProfileFileName()
{
    if (! inited)  {
	profileHead = openprofile("PROFILES", DEFAULTPROFILES, 1);
	GloprofileHead = openprofile("GLOBALPROFILES", GLOBALPROFILE, 0);
	inited = 1;
    }

    return firstProfileFileName;
}

void refreshprofile() {  /* Force rereading */
    if (profileHead != NULL)  {
	FreeConfigureList(profileHead);
	profileHead = NULL;
    }
    if (profileFileName != NULL)  {
	free(profileFileName);
	profileFileName = NULL;
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
    if (! inited)  {
	profileHead = openprofile("PROFILES", DEFAULTPROFILES, 1);
	GloprofileHead = openprofile("GLOBALPROFILES", GLOBALPROFILE, 0);
	inited = 1;
    }
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
    static struct keys {
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
    register struct keys   *p;
    if (var && (val = getprofile (var))) {
	for (p = keys; p -> name; p++)
	    if (FOLDEDEQ(p->name, val))
		return p -> value;
    }
    return DefaultValue;
}

int getprofileint (const char *var, int DefaultValue)
{
    register const char  *val;
    register    int n = 0;
    register    int neg = 0;

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

    if (! inited)  {
	profileHead = openprofile("PROFILES", DEFAULTPROFILES, 1);
	GloprofileHead = openprofile("GLOBALPROFILES", GLOBALPROFILE, 0);
	inited = 1;
    }

    return (var != NULL && ( (GetConfig(StringProfileHead, var, usedefault) != NULL) || 
			     (GetConfig(profileHead, var, usedefault) != NULL) || 
			     (GetConfig(GloprofileHead, var, usedefault) != NULL)));
}
