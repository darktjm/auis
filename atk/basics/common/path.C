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
ATK_IMPL("path.H")

#include <ctype.h>

#include <environ.H>
#include <im.H>

#include <errno.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <pwd.h>
#include <util.h>

#if SY_OS2
#include <os2.h>
#endif
#include <filetype.H>

#include <path.H>

struct homestruct {
    char fullPath[MAXPATHLEN];
    char shortPath[MAXPATHLEN];
    long fullLength;
    long shortLength;
    struct homestruct *next;
};

static struct homestruct *homes = NULL;

#define INITIALCHUNK 30 /* ::::files */

	
ATKdefineRegistry(path, observable, NULL);
static char * strappend(char  *dest , char  *src);
static void FreeList(char  **list);
static void FreeFilesAndDirs(class path  *self);
static void SetPath(class path  *self, char  *filepath);
#if 1
static void  FoldName (char  *p		);
#else
static void FoldName (char  *p);
#endif
static long SetNewHome(char  *shortPathName, const char  *name, const char  *cell, const char  *dir, long  dirlen);
static void HandleCellTwiddle(const char  *fromString, char  *toString);
static void HandleRelativeFileName(const char  *fromString, char  *toString, const char  *basefile);
int CompareFileNames(char  **a, char  **b);


static char *
strappend(char  *dest , char  *src)
	{
	int len = strlen(src);
	memmove(dest, src, len); /* allow overlapping src/dest */
	return dest + len;
}


static void FreeList(char  **list)
    {
    long i;

    if (list != NULL) {
        for (i=0; list[i] != NULL; i++) {
            free(list[i]);
        }
        free(list);
    }
} /* path__FreeList */

static void FreeFilesAndDirs(class path  *self)
    {
    FreeList(self->files);
    FreeList(self->dirs);
    self->files = NULL;
    self->dirs = NULL;
} /* FreeFilesAndDirs */

static void SetPath(class path  *self, char  *filepath)
        {
    if (self->filepath != NULL) {
        free(self->filepath);
    }

    if (self->truncatedPath != NULL) {
        free(self->truncatedPath);
        self->truncatedPath = NULL;
    }

    FreeFilesAndDirs(self);
 
    if (filepath != NULL) {
        /* SHOULD CACHE THESE THINGS */
        self->filepath = strdup(filepath);
    }
    else {
        self->filepath = NULL;
    }

    self->haveScanned = FALSE;
    self->knowIsDir = FALSE;

} /* SetPath */

void path::InputTruncatedPathCache(FILE  *fp)
        {
    char lens[MAXPATHLEN];
    char fpath[MAXPATHLEN];
    char spath[MAXPATHLEN];
    long flen;
    long slen;
    struct homestruct *aHome;
    struct homestruct *newHome;
    struct homestruct *endHome = NULL;
    struct homestruct *inputHomes = NULL;
    boolean gotHome;

    fgets(lens, MAXPATHLEN, fp);
    while (strncmp(lens, "no", 2) != 0) {
        sscanf(lens, "fullpathlen: %ld shortpathlen: %ld\n", &flen, &slen);
        fgets(fpath, MAXPATHLEN, fp);
        fgets(spath, MAXPATHLEN, fp);
        fpath[flen] = '\0';
        spath[slen] = '\0';
        gotHome = FALSE;
        for (aHome = homes; aHome != NULL; aHome = aHome->next) {
            if (strcmp(aHome->fullPath, fpath) == 0) {
                gotHome = TRUE;
                break;
            }
        }
        if (!gotHome) {
            newHome = (struct homestruct *) malloc(sizeof(struct homestruct));
            strcpy(newHome->fullPath, fpath);
            strcpy(newHome->shortPath, spath);
            newHome->fullLength = flen;
            newHome->shortLength = slen;

            if (inputHomes == NULL) {
                endHome = newHome;
            }
            newHome->next = inputHomes;
            inputHomes = newHome;
        }
        fgets(lens, MAXPATHLEN, fp);
    }

    if (endHome != NULL) {
        endHome->next = homes;
    }
    homes = inputHomes;

} /* path__InputTruncatedPathCache */

void path::OutputTruncatedPathCache(FILE  *fp)
        {
    struct homestruct *aHome;

    for (aHome = homes; aHome != NULL; aHome = aHome->next) {
        fprintf(fp, "fullpathlen: %ld shortpathlen: %ld\n", aHome->fullLength, aHome->shortLength);
        fprintf(fp, "%s\n%s\n", aHome->fullPath, aHome->shortPath);
    }
    fprintf(fp, "no more home directories\n");
} /* path__OutputTruncatedPathCache */

path::path()
        {
    this->filepath = NULL;
    this->truncatedPath = NULL;
    this->files = NULL;
    this->dirs = NULL;
    this->haveScanned = FALSE;
    this->knowIsDir = FALSE;
    this->mayBeWrong = TRUE;
    this->numFiles = -1;
    this->numDirs = -1;
    this->changetime = 0;

    THROWONFAILURE( TRUE);
} /* path__InitializeObject */

class path *path::Create(char  *filepath)
        {
    class path *self = new path;

    SetPath(self, filepath);

    return self;
} /* path__Create */


#if 1

/* FoldName -- remove all "." and ".." components from a file name 
     modifies the buffer passed to it 

RESTRICTION:
When there are symlinks, .. is meaningful and should get into 
an entirely different directory.  This does not happen here.

In general, names passed to this routine begin with slash.
However, the routine works equally well if they do not.


The pathname is broken into segments by /'s.
There is a state machine, where the state depends on the previous segment.
For each cell, there is an action and a new state.  Segments may be
	.   ..   empty   x (i.e., any name)

The states are:
	Start	-  There is no prior seg.
	ISlash	-  Prior seg is / at start of path
	IX	-  Prior seg is /x at start of path 
	NewC	-  Prior seg is /../ at start of path  (for Newcastle)
	IDot	-  Prior seg is . at strat of path
	IDots	-  Prior seg is .. at start of path
	X	-  Prior seg is a name
In states X and IX, there is a name preserved which will usually be later
generated to the output.  We refer to this saved name as y.

In the following diagram, the action is a triple: text generated to the
path, text preserved as y, and new state.  In general, slashes
are generated at the beginning of generation for a segment, if needed.

	|	Current segment  (is followed by a slash)
State	|	.		..		x		empty
___________________________________________________________________________
Start	|	, , IDot	, , IDots	, x, IX		, , ISlash
ISlash	|	, , ISlash	, , NewC	, x, X		, , ISlash
IX	|	, y, IX		, , IDot	y, x, X		, y, IX
NewC	|	, , NewC	, , NewC	/.., x, X	, , NewC
IDot	|	, , IDot	, , IDots	, x, IX		, , IDot
IDots	|	, , IDots	../, , IDots	.., x, X	, , IDots
X	|	, y, X		(1), Check	/y, x, X	, y, X


(1) the .. and y cancel each other.  We restore the state according to
how we could have transitioned to the X state.  These rules could have gotten to
state X:

ISlash - x:	, x, X		dest == path
IX - x:		y, x, X		no slash, just an identifier in dest
NewC - x:	/.., x, X	/.. in dest
IDots - x:	.., x, X	no slash, just .. in dest
X - .:		, y, X		ignore this case
X - x:		/y, x, X	/y in dest
X - empty:	, y, X		ignore this case, too


When the current segment is at the end and has no following slash,
the machine is finished.  The only action is to generate text to output.

	|	Current segment  (at the end with no following slash)
State	|	.	..	x	empty
___________________________________________________________________________
Start	|	.	..	x	.
ISlash	|	/	/..	/x	/
IX	|	y	.	y/x	y/
NewC	|	/../	/../	/../x	/../
IDot	|	.	..	x	./
IDots	|	..	../..	../x	../
X	|	/y	/	/y/x	/y/

*/

/* States */
enum state {
	Start,	/* There is no prior seg.  */
	ISlash,	/* Prior seg is / at start of path  */
	IX,	/* Prior seg is /x at start of path   */
	NewC,	/* Prior seg is /../ at start of path  (for Newcastle)  */
	IDot,	/* Prior seg is . at strat of path  */
	IDots,	/* Prior seg is .. at start of path  */
	X,	/* Prior seg is a name  */
	Check	/* used for Backup case */
};

/*      Actions  */
enum segaction {
	/*  , ,    */	Naught,
	/*  , x    */	SaveX,
	/*  , y    */	SaveY,
	/*  y, x   */	GenYSvX,
	/*  /.., x */	NewCSvX,
	/*  ../,   */	Dots,
	/*  .., x  */	DotsSvX,
	/*  (1)    */	Backup,
	/*  /y, x  */	NextX
};

enum segtype {Dot, DotDot, ID, Empty};

#define NSTATES 7
#define NTYPES 4

static const struct {enum segaction Action; enum state NewState;}
	SegTrans[NSTATES][NTYPES]
= {
/* Start */  {{Naught, IDot},  {Naught, IDots},{SaveX, IX}, {Naught, ISlash}},
/* ISlash */ {{Naught, ISlash},{Naught, NewC}, {SaveX, X},  {Naught, ISlash}},
/* IX */     {{SaveY, IX},     {Naught, IDot}, {GenYSvX, X}, {SaveY, IX}},
/* NewC */   {{Naught, NewC},  {Naught, NewC}, {NewCSvX, X}, {Naught, NewC}},
/* IDot */   {{Naught, IDot},  {Naught, IDots},{SaveX, IX}, {Naught, IDot}},
/* IDots */  {{Naught, IDots}, {Dots, IDots},  {DotsSvX, X}, {Naught, IDots}},
/* X */	     {{SaveY, X},      {Backup, Check},{NextX, X},   {SaveY, X}}
};

static const char * const LastSeg[NSTATES][NTYPES]
	= {
/* Start */	{".",     "..",     "x",     "."},
/* ISlash */	{"/",     "/..",    "/x",    "/"},
/* IX */	{"y",     ".",      "y/x",   "y/"},
/* NewC */	{"/../",  "/../",   "/../x", "/../"},
/* IDot */	{".",     "..",     "x",     "./"},
/* IDots */	{"..",    "../..",  "../x",  "../"},
/* X */	  	{"/y",    "/",       "/y/x",  "/y/"}
};


	static void 
FoldName (char  *p		/* path to fold */)
	{
	enum state CurrState, NextState;
	char SavedY [MAXPATHLEN];

	char *dest;	/* where to generate output */
	char *src;	/* start of current segment */
	char *sx;	/* segment scanner 
				segment extends from src to sx-1 */
	const char *lx;	/* last segment string */
	char *slx;
	enum segtype CurrType;

	*SavedY = '\0';
	CurrState = Start;
	src = p;
	dest = p;

	while (TRUE) {
		/* get end of segment which starts at src */
		sx = strchr(src, '/');
		if (sx == NULL) sx = src + strlen(src);
	
		/* determine segment type: . .. x empty */
		if (sx - src <= 0)  CurrType = Empty;
		else if (*src != '.') CurrType = ID;
		else if (sx - src == 1)  CurrType = Dot;
		else if (sx - src == 2 && *(src+1) == '.')
				CurrType = DotDot;
		else CurrType = ID;
		if (*sx != '/') 
			/* we are at the end */
			break;
	
		/* process state transition */
		NextState = SegTrans[(long)CurrState][(long)CurrType].
				NewState;
		switch ((long)SegTrans[(long)CurrState][(long)CurrType].
				Action) {
		case Naught:		/*  , ,    */
		case SaveY:		/*  , y    */
			break;
		case SaveX:		/*  , x    */
	savex:
			strncpy(SavedY, src, sx-src);
			SavedY[sx-src] = '\0';
			break;
		case GenYSvX:		/*  y, x   */
			dest = strappend(dest, SavedY);
			goto savex;
		case Dots:		/*  ../,   */
			*dest++ = '.';
			*dest++ = '.';
			*dest++ = '/';
			break;
		case NewCSvX:		/*  /.., x */
			*dest++ = '/';
			/* FALL THRU */
		case DotsSvX:		/*  .., x  */
			*dest++ = '.';
			*dest++ = '.';
			goto savex;
		case NextX:		/*  /y, x  */
			*dest++ = '/';
			dest = strappend(dest, SavedY);
			goto savex;
		case Backup:		/*  (1)    */
			if (dest == p) {
				/* nothing generated so far */
				NextState = ISlash;
				break;
			}
			*dest = '\0';
			slx = strrchr(p, '/');	/* find last slash */
			if (slx == NULL) {
				/* no slash generated yet */
				strcpy(SavedY, p);
				NextState = (strcmp(SavedY, "..") == 0)
					? IDots  :  IX;
				dest = p;
				break;
			}
			strcpy(SavedY, slx+1);
			dest = slx;
			if (strcmp(SavedY, "..") != 0)
				NextState = X;
			else if (*p == '/')
				NextState = NewC;
			else {
				/* dest begins with one or more "../" */
				NextState = IDots;
				dest++;		/* keep the slash */
			}
			break;
		}
		CurrState = NextState;

		/* set start of next segment */
		src = sx + 1;	/* just past the slash */
	}
	/* process final segment via LastSeg table 
	   for each char:
		x : copy src to dest
		y : copy SavedY to dest
		otherwise : copy char to dest
	*/
	lx = LastSeg[(long)CurrState][(long)CurrType];
	for ( ; *lx; lx++)
		if (*lx == 'x')
			dest = strappend(dest, src);
		else if (*lx == 'y')
			dest = strappend(dest, SavedY);
		else *dest++ = *lx;
	*dest = '\0';

}

#else

/*
	Rules:
	    buffer is	=> use this string
		/	=>  /
		.	=> .
		..	=> ..
		empty	=> .
		x	=> x
	    buffer starts    use this start
		/..	=> /..   (was => / , but must preserve /../
					for newcastle connection  -wjh)
		x/..	=> .
		./..	=> ..
		..	=> ..
		../..	=> ../..   (any number of ../)
		x/y	=> x/y
	    buffer continues    replace with
		/./	=> /
		//	=> /
		/x/../	=> /
    Does not know about ~ 
    Does not remove trailing /. or /..
    Does not merge leading /../../ into /../
    Converts /x/../../ into ../, but should be /../

*/

static void FoldName (char  *p			/* path to fold */)
    {
    char   *pStart,		/* points to first char of a component */
	   *pEnd;		/* points to first char following
				   component */
    char *dest;
    int     len;

    if (p == NULL)
	return;
    dest = p;
    pEnd = (*p == '/' ? p + 1 : p);
    for (;;) {
	pStart = pEnd;
	pEnd = strchr (pStart, '/');
	if (pEnd == NULL)
	    pEnd = pStart + strlen (pStart);
	len = pEnd - pStart;
	if (len == 0) { /* ignore empty components, but preserve trailing /'s. */
            if (*pEnd == '\0')
                *dest++ = '/';
        }
	else if (len == 1 && *pStart == '.' && pStart[1] == '/')
	    {}   /* ignore single dots */
	else if (len == 2 && pStart[0] == '.' && pStart[1] == '.' && pStart[2] == '/') {
	    switch (dest - p) {
#if 0
/* this old version did not preserve /../  */
	    case 0: /*  /.. => /   and   .. => .. */
		    if (*p != '/')  
			*dest++ = '.', *dest++ = '.';
		    break;
#else
	    case 0: /*  /.. => /..   and   .. => .. */
		    if (*p == '/')  *dest++ = '/';
		    *dest++ = '.', *dest++ = '.';
		    break;
#endif
	    case 1: /*  ./.. => ..   and   x/.. => . */
		    if (*p == '.') 
			*dest++ = '.';
		    else *p = '.';
		    break;
	    case 2: /*  ../.. => ../..   and   x/.. => .  */
		    if (strncmp(p, "..", 2)==0)
			*dest++ = '/', *dest++ = '.', *dest++ = '.';
		    else *p = '.', dest = p+1;
		    break;
	    default: /*  y/x/..=>y   x/..=>.  /x/..=>/   ../../..=>../../..  */
		    if (strncmp(dest-3, "/..", 3)==0)
			*dest++ = '/', *dest++ = '.', *dest++ = '.';
		    else {  /* this is the principal case for .. */
			while (dest > p && *--dest != '/') {}
			if (dest == p && *p != '/')
			    *dest++ = '.';
		    }
		    break;
	    }
	}
	else {
	    if (dest>p || *p == '/') 
		*dest++ = '/';
	    strncpy (dest, pStart, len);
	    dest += len;
	}
	if (*pEnd++ == 0)
	    break;
    }
    if (dest==p)   /* inital path was  /  .  or empty */
	*dest++ = (*p == '/' ? '/' : '.');
    *dest++ = 0;
}
#endif

static long SetNewHome(char  *shortPathName, const char  *name, const char  *cell, const char  *dir, long  dirlen)
{
    struct homestruct *newHome;
    long addedLen = 1;

    newHome = (struct homestruct *) malloc(sizeof(struct homestruct));
    newHome->next = homes;
    homes = newHome;
    strcpy(newHome->fullPath, dir);
    newHome->fullLength = dirlen;

    strcpy(shortPathName, "~");
    if (name != NULL) {
	strcat(shortPathName, name);
	addedLen += strlen(name);
    }
    strcpy(newHome->shortPath, shortPathName);
    newHome->shortLength = strlen(shortPathName);

    return addedLen;
}

/*
 * Truncates a path so that the end is visible.
 * (Initial code stolen from frame.c).
 *
 * If result is NULL the returned value must be freed.
 * 
 * Also, call FreeTruncatedPaths() to free up the cached entries.
 */

char *path::TruncatePath(char  *frompath, char  *result, long  limit, boolean  tryHome)
                    {
    char shorter[MAXPATHLEN];
    char foldedpath[MAXPATHLEN];
    char *pathp;
    int len, maxLen;
    static long lastUID = -1;
    static char passwdName[100];
    static char passwdDir[MAXPATHLEN];
    static int passwdDirLen;
    static boolean gotBaseInfo = FALSE;
    static boolean CheckOwnerHome = FALSE;
    
    if (frompath == NULL) {
        return NULL;
    }

    if (! gotBaseInfo) {
	const char *homeDir = environ::GetHome(NULL);
	char shortName[10];

	if (homeDir != NULL) {
	    SetNewHome(shortName, NULL, NULL, homeDir, strlen(homeDir));
	}
	CheckOwnerHome = environ::GetProfileSwitch("CheckOwnerHome", TRUE);
	gotBaseInfo = TRUE;
    }

    strcpy(foldedpath, frompath);
    FoldName(foldedpath);
    pathp = foldedpath;

    maxLen = limit;
    shorter[0] = '\0';

    if (tryHome && pathp[0] == '/' && pathp[1] != '\0') {
        char tmpPath[MAXPATHLEN];
        struct passwd *passwd;
        struct stat buf;
        char *slash;
        struct homestruct *aHome;
        boolean hitCache = FALSE;


        for (aHome = homes; aHome != NULL; aHome = aHome->next) {
            if (strncmp(aHome->fullPath, pathp, aHome->fullLength) == 0 && (pathp[aHome->fullLength] == '/' || pathp[aHome->fullLength] == '\0')) {
                strcpy(shorter, aHome->shortPath);
                maxLen -= aHome->shortLength;
                pathp += aHome->fullLength;
                hitCache = TRUE;
                break;
            }
        }

        if (!hitCache) {

	    strcpy(tmpPath, pathp);
            len = strlen(tmpPath);
            slash = &tmpPath[len];
            while (len > 1 && tmpPath[len-1] == '/') {
                slash--;
                len--;
            }

	    if (len > 0) {
		/* Have more than the root directory */
		*slash = '\0';
		if (CheckOwnerHome && stat(tmpPath, &buf) == 0) {
		    if (buf.st_uid != lastUID) {
			passwd = getpwuid(buf.st_uid);
			if (passwd != NULL) {
			    strcpy(passwdDir, passwd->pw_dir);
			    strcpy(passwdName, passwd->pw_name);
			    passwdDirLen = strlen(passwdDir);
			}
			else {
			    passwdName[0] = '\0';
			}
			lastUID = buf.st_uid;
		    }

		    if (passwdName[0]!= '\0' && strncmp(tmpPath, passwdDir, passwdDirLen) == 0 && (tmpPath[passwdDirLen] == '/' || tmpPath[passwdDirLen] == '\0')) {
			maxLen -= SetNewHome(shorter, passwdName, NULL, passwdDir, passwdDirLen);
			pathp += passwdDirLen;
		    }
		}
            }
        }
    }

    /* put the result together */

    len = strlen(pathp);
    if (len > maxLen) {
        char *partialName;

	if (result == NULL) {
	    result = (char *) malloc(limit + 1);
	    if (result == NULL) return NULL;
	}

        maxLen -= sizeof("---") - 1;
        partialName = strchr(pathp + (len - maxLen), '/');
        if (partialName == NULL) {
            partialName = pathp + (len - maxLen);
        }
        else {
            ++partialName; /* Skip slash... */
        }
        strcpy(result, "---");
        strcat(result, partialName);
    }
    else {
	if (result == NULL) {
	    result = (char *) malloc(strlen(shorter) + len + 1);
	    if (result == NULL) return NULL;
	}
        strcpy(result, shorter);
        strcat(result, pathp);
    }

    return result;
} /* path__TruncatePath */

void path::FreeTruncatedPaths()
    {
    struct homestruct *home;
    struct homestruct *nexthome;

    home = homes;
    while (home != NULL) {
        nexthome = home->next;
        free(home);
        home = nexthome;
    }
    homes = NULL;
} /* path__FreeTruncatedPaths */

boolean path::ModifyToParentDirectory(char  *p, boolean  isDirectory)
            {
    long len = strlen(p);

    if (isDirectory) {
	while (len > 1 && p[len-1] == '/') {
	    len--;
	}
    }

    while (len > 0 && p[len-1] != '/') {
	len--;
    }

    p[len] = '\0';

    return (len > 0);
} /* path__ModifyToParentDirectory */

static void HandleCellTwiddle(const char  *fromString, char  *toString)
{
    const char *home=NULL;
    struct passwd *passwd;
    long p;

    if (fromString[1] == '\0' || fromString[1] == '/') {
	p = 1;
	home = environ::Get("HOME");
	if (home == NULL) {
	    /* Current user */
	    passwd = getpwuid(getuid());
	    if (passwd != NULL) {
		home = passwd->pw_dir;
	    }
	    if (home == NULL) {
		p = 0;
	    }
	}
    }
    else {
	long pos;
	char name[MAXPATHLEN];
	char cellName[MAXPATHLEN];
	char *cn = cellName;

	for (pos = 1; fromString[pos] != '\0' && fromString[pos] != '/'; pos++) {
	}

	strncpy(name, &fromString[1], pos - 1);
	name[pos -1] = '\0';
	if (cn == cellName) {
	    passwd = getpwnam(name);
	    p = pos;
	}
	else {
	}

	if (passwd != NULL) {
	    home = passwd->pw_dir;
	}
    }
       if (home != NULL) {
 	strcpy(toString, home);
 	strcat(toString, &fromString[p]);
     }
     else {
 	strcpy(toString, fromString);
     }
}

static void HandleRelativeFileName(const char  *fromString, char  *toString, const char  *basefile)
{
    char *slash;
    /* Use allDone instead of confusing combination of #if/#endif and if/else's */
    boolean allDone = FALSE;

    if (basefile == NULL) basefile = "";

/* Special case for OS/2: If fromString starts with a drive letter followed by a colon, we only make it to this function if the colon is followed by a relative path; we assume this relative path is relative to the cwd for the drive. Squeeze the cwd in between the colon and the relative path. */
 
#if SY_OS2
    if (fromString[1] == ':') {
	/* get the cwd for this drive */
	ULONG drivenum;
	ULONG dirpathlen = CCHMAXPATH;
	UCHAR dirpath[dirpathlen];

	/* Convert drive letter into drivenum */
	drivenum = *fromString - (*fromString/32)*32;
	if (DosQueryCurrentDir(drivenum, dirpath, &dirpathlen))
	    /* ERROR! Just pass back fs */
	    strcpy(toString, fromString);
	else {
	    strncpy(toString, fromString, 2);
	    toString[2] = '\0';
	    sprintf(toString,"%s\\%s%s%s", toString, dirpath, ((dirpath[0] == '\0') ? "" : "\\"), &fromString[2]);
	}
	allDone = TRUE;
    }
#endif

    if (!allDone) {
	if (*basefile != '/' && im::GetDirectory(toString) != NULL) {
#ifdef DOS_STYLE_PATHNAMES
	/* If the file name starts with '\' or '/', don't include the cwd in the full path name; just include the current drive */
	/* Assume cwd includes current drive, e.g. c:\home */
	    if (*fromString == '/' || *fromString == '\\')
		toString[2] = '\0';
	    else
		strcat (toString, "/");
#else
	    strcat (toString, "/");
#endif
    }
    else
	toString[0] = '\0';    /* ??? discard error message if getwd==NULL */

    strcat (toString, basefile);
/* MBY This part doesn't make sense for OS/2 */
#ifdef DOS_STYLE_PATHNAMES
#else
	slash = strrchr(toString, '/');
	if (slash==NULL) {
	    slash = &toString[strlen(toString)];
	    *slash = '/';
	}
	*++slash = '\0';
#endif
	strcat(toString, fromString);
    }
}
/*
    Returns either fromString or toString depending on
    if the file name needs to be unfolded.
*/

const char *path::UnfoldFileName(const char  *fromString, char  *toString, const char  *basefile)
{
    const char *fs = fromString;
    char tempstr[2*MAXPATHLEN+1];

    while (isspace(*fs))
        fs++;

    environ::ExpandEnvVars(tempstr, fs, 2*MAXPATHLEN+1);
    fs = tempstr;

#ifndef DOS_STYLE_PATHNAMES
    if (*fs == '/') strcpy(toString, fs);
#else
    boolean uncName=FALSE;
    char *slash;
	/* Change all backslashes to forward slashes
	 * just to make it pretty */
    while ((slash = strchr(fs, '\\')) != NULL)
	    *slash = '/';
    /* file names of the following forms are "fully qualified":
     *      X:/...... (where X is any character)
     *      //whatever (this is the UNC format */
    if ((fs[1] == ':' && fs[2] == '/') || (fs[0] == '/' && fs[1] == '/')) {
	if (fs[0] == '/') uncName = TRUE;
	strcpy(toString, fs);
    }
#endif /*!DOS_STYLE_PATHNAMES*/
    else if (*fs == '~') {
	HandleCellTwiddle(fs, toString);
    }
    else {
	HandleRelativeFileName(fs, toString, basefile);
    }

    FoldName(toString);

#ifdef DOS_STYLE_PATHNAMES
    /* FoldName can't handle "driveletter:\\." (note period)
     * It returns "driveletter:" which isn't a valid dir.
     * Add a slash, if this is what's returned.
     */
    if (strlen(toString) == 2 && toString[strlen(toString)-1] == ':')
	strcat(toString, "/");
    /* If this was a unc name, put the add'l / back on the front.
     *  (FoldName zapped it) */
    if (uncName) {
	strcpy(tempstr, toString);
	sprintf(toString, "/%s", tempstr);
    }
#endif

    return toString;
} /* path__UnfoldFileName */

void path::ReleaseFiles(char  **files)
        {
} /* path__ReleaseFiles */

void path::ReleaseDirs(char  **dirs)
        {
} /* path__ReleaseDirs */

path::~path()
        {
    if (this->filepath != NULL) {
        free(this->filepath);
    }
    if (this->truncatedPath != NULL) {
        free(this->truncatedPath);
    }

    FreeFilesAndDirs(this);
    (this)->NotifyObservers( observable_OBJECTDESTROYED);

    return;
} /* path__FinalizeObject */

int CompareFileNames(char  **a, char  **b)
        {
    /* this puts .files before all others */

    if (**a == '.') {
        return (**b == '.') ? strcmp(*a, *b) : -1;
    }
    else {
        return (**b == '.') ? 1 : strcmp(*a, *b);
    }
} /* CompareFileNames */

boolean path::Scan(boolean  statEverything)
        {
    char dirbuf[MAXPATHLEN];
    char fullName[MAXPATHLEN];
    char *filePart;
    struct stat statBuf;
    DIR *thisDir;
    DIRENT_TYPE *dirEntry;
    long filesalloced = 0;
    long dirsalloced = 0;
    long nextfile = 0;
    long nextdir = 0;
    long len;
    boolean noProblems = FALSE;

    if (this->filepath == NULL || (stat(this->filepath, &statBuf) != 0)) {
        FreeFilesAndDirs(this);
    }
    else if ((statBuf.st_ctime > this->changetime) || (this->mayBeWrong && statEverything)) {
        time_t changetime;

        /* We're going to (re)scan the path */
        FreeFilesAndDirs(this);
        changetime = statBuf.st_ctime;
        /*
          Since it's not impossible for isDir to be wrong,
              it's safer to recompute it.
          */
        this->isDir = (statBuf.st_mode & S_IFMT) == S_IFDIR;
        this->knowIsDir = TRUE;
        if (!this->isDir) {
            this->mayBeWrong = FALSE;
            noProblems = TRUE;
        }
        else {
            this->mayBeWrong = !statEverything;
            strcpy(dirbuf, this->filepath);

            if ((thisDir = opendir(dirbuf)) != NULL) {
                noProblems = TRUE;
                this->haveScanned = TRUE;

                strcpy(fullName, dirbuf);
                len = strlen(fullName);
                if (len == 0 || fullName[len - 1] != '/') {
                    fullName[len] = '/';
                    len++;
                }


                filePart = &fullName[len];
                while ((dirEntry = readdir(thisDir)) != NULL) {
                    boolean isdirectory = FALSE;
                    char *name = dirEntry->d_name;

                    {
                        strcpy(filePart, name);
                        stat(fullName, &statBuf);
                        if ((statBuf.st_mode & S_IFMT) == S_IFDIR) {
                            isdirectory = TRUE;
                        }
                    }
                    if (isdirectory) {
                        if (strcmp(name, ".") != 0 && strcmp(name, "..") != 0) {
                            if (this->dirs == NULL) {
                                this->dirs = (char **) malloc(INITIALCHUNK * sizeof(char *));
                                dirsalloced = INITIALCHUNK;
                                nextdir = 0;
                            }
                            else if (nextdir >= dirsalloced) {
                                dirsalloced *= 2;
                                this->dirs = (char **) realloc(this->dirs, dirsalloced * sizeof(char *));
                            }
                            this->dirs[nextdir] = strdup(name);
                            nextdir++;
                        }
                    }
                    else {
                        if (this->files == NULL) {
                            this->files = (char **) malloc(INITIALCHUNK * sizeof(char *));
                            filesalloced = INITIALCHUNK;
                            nextfile = 0;
                        }
                        else if (nextfile >= filesalloced) {
                            filesalloced *= 2;
                            this->files = (char **) realloc(this->files, filesalloced * sizeof(char *));
                        }
                        this->files[nextfile] = strdup(name);
                        nextfile++;
                    }
                }

                closedir(thisDir);
                this->numDirs = nextdir;
                this->numFiles = nextfile;

                /* null terminate both lists and sort them */
                if (this->dirs != NULL) {
                    if (nextdir >= dirsalloced) {
                        dirsalloced++;
                        this->dirs = (char **) realloc(this->dirs, dirsalloced * sizeof(char *));
                    }
                    this->dirs[nextdir] = 0;
                    qsort(this->dirs, nextdir, sizeof(char *), QSORTFUNC(CompareFileNames));
                }

                if (this->files != NULL) {
                    if (nextfile >= filesalloced) {
                        filesalloced++;
                        this->files = (char **) realloc(this->files, filesalloced * sizeof(char *));
                    }
                    this->files[nextfile] = 0;
                    qsort(this->files, nextfile, sizeof(char *), QSORTFUNC(CompareFileNames));
                }
            }
        }
        (this)->NotifyObservers( observable_OBJECTCHANGED);

        if (noProblems) {
            this->changetime = changetime;
        }
    }
    else {
        noProblems = TRUE;
    }

    return noProblems;
} /* path__Scan */

void path::Input(FILE  *fp)
{
    char p[MAXPATHLEN];
    long len;
    boolean knowIsDir;
    boolean isDir;

    fgets(p, MAXPATHLEN, fp);
    if (strncmp(p, "no", 2) != 0) {
        knowIsDir = TRUE;
        sscanf(p, "is directory: %d\n", &isDir);
    }
    else {
        knowIsDir = FALSE;
    }

    fgets(p, MAXPATHLEN, fp);
    if (strncmp(p, "no", 2) != 0) {
        len = strlen(p);
        if (p[len-1] == '\n') {
            p[len-1] = '\0';
        }
        SetPath(this, &p[strlen("path: ")]);
    }

    /* SetPath alters knowIsDir */
    this->knowIsDir = knowIsDir;
    this->isDir = isDir;

    fgets(p, MAXPATHLEN, fp);
    if (strncmp(p, "no", 2) != 0) {
        long tlen;

        len = strlen(p);
        if (p[len-1] == '\n') {
            p[len-1] = '\0';
        }
        tlen = strlen("truncated path: ");
        this->truncatedPath = (char *) malloc(len + 1 - tlen);
        strcpy(this->truncatedPath, &p[tlen]);
    }
    else {
        this->truncatedPath = NULL;
    }

} /* path__Input */

void path::Output(FILE  *fp)
{
    if (this->knowIsDir) {
        fprintf(fp, "is directory: %d\n", this->isDir);
    }
    else {
        fprintf(fp, "no directory information\n");
    }

    if (this->filepath != NULL) {
        fprintf(fp, "path: %s\n", this->filepath);
    }
    else {
        fprintf(fp, "no path\n");
    }

    if (this->truncatedPath != NULL) {
        fprintf(fp, "truncated path: %s\n", this->truncatedPath);
    }
    else {
        fprintf(fp, "no truncated path\n");
    }

} /* path__Output */

boolean path::IsDirectory()
{
    if (!this->knowIsDir) {
        struct stat statBuf;

        this->knowIsDir = TRUE;
        this->isDir = FALSE;
        if (stat(this->filepath, &statBuf) == 0 &&
             (statBuf.st_mode & S_IFMT) == S_IFDIR) {
            this->isDir = TRUE;
        }
    }
    return this->isDir;
} /* path__IsDirectory */

char **path::GetFiles()
{
    if (!this->haveScanned) {
        (this)->Scan( FALSE);
    }
    return this->files;
} /* path__GetFiles */

char **path::GetDirs()
{
    if (!this->haveScanned) {
        (this)->Scan( FALSE);
    }
    return this->dirs;
} /* path__GetDirs */

long path::GetNumFiles()
{
    if (!this->haveScanned) {
        (this)->Scan( FALSE);
    }
    return this->numFiles;
} /* path__GetNumFiles */

long path::GetNumDirs()
{
    if (!this->haveScanned) {
        (this)->Scan( FALSE);
    }
    return this->numDirs;
} /* path__GetNumDirs */

char *path::GetTruncatedPath()
{
    if (this->truncatedPath == NULL && this->filepath != NULL) {
        this->truncatedPath = path::TruncatePath(this->filepath, NULL, MAXPATHLEN, TRUE);
    }
    return this->truncatedPath;
} /* path__GetTruncatedPath */

void path::FindFileInPath(char *retbuff, const char *path, const char *fname)
{
    findfileinpath(retbuff, path, fname);
}

/* RelativizePathname
	makes name relative if prefixes of name 
		and basepath are equal
	maximum of 'maxup' ../s
	returns true if shortened name by making it relative
	if basepath does not end in 'slash', 
		it is assumed to have a trailing file name, 
		which is ignored
*/
	boolean 
path::RelativizePathname(char *name, const char *basepath, int maxup) {
	if ( ! name || ! basepath) return FALSE;

	// scan across common parts of names 
	char *nx = name;
	const char *bx = basepath;
	while (*nx && *bx && *nx == *bx) nx++, bx++;
	nx--, bx--;
	while (nx > name && *nx != '/') 
		nx--, bx--;
	// nx and bx point to last common slash
	nx++, bx++;

	// now count remaining directories in basepath
	int ndirs = 0;
	for (; *bx; bx++) if (*bx == '/') ndirs++;
	if (ndirs > maxup || nx - name < 3*ndirs) 
		return FALSE;

	// construct relative name from ../s and tail of name
	char *nbx = name; 
	while (ndirs --) 
		*nbx++ = '.', *nbx++ = '.', *nbx++ = '/';
	while ((*nbx++ = *nx++)) {}
	return TRUE;
}
