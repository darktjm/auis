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

/*	Modified 1/19/90 CCH cch@mtgzx.att.com
 *	Fixed so that help files that have upper case charactrers can be found.
 *	The name being searched for is lowercased in help.c, but when it is
 *	looking for a man file, it does not lowercase the filenames.
 */

/*---------------------------------------------------------------------------*/
/*	MODULE: helpdb.c						     */
/*		Package of routines to do the index-related lookups and	     */
/*		matching algorithms.					     */
/*---------------------------------------------------------------------------*/

#include <andrewos.h> /* sys/types.h sys/file.h */
ATK_IMPL("helpdb.H")

#include <andyenv.h>

#include <cursor.H>
#include <environ.H>
#include <message.H>
#include <im.H>

#include <stdio.h>
#include <errno.h>
#include <ctype.h>
#include <sys/errno.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "config.h"
#include "helpsys.h"
#include <path.H>
#include <helpdb.H>

#include <index.h>




















/*---------------------------------------------------------------------------*/
/*			CONDITIONAL DEBUGGING				     */
/*---------------------------------------------------------------------------*/

#ifdef DEBUGGING
/*
 * debugging statements can be included by compiling add modules with
 * -DDEBUGGING.  Debugging output is toggled by existance/nonexistance
 * of the environment variable HELPDEBUG.
 */
int HELPDBDEBUG = 0;
#undef DEBUG
#define DEBUG(arg) if (HELPDBDEBUG != 0) { printf arg; fflush(stdout); }
#else
#define DEBUG(arg)
#endif /* DEBUGGING */


/*---------------------------------------------------------------------------*/
/*				GLOBALS					     */
/*---------------------------------------------------------------------------*/

static struct helpAlias *allAliases = (struct helpAlias *)NULL;
static struct Index *openIndex = (struct Index *)NULL;
static int indexErrno = 0;	/* errno for index package */

static struct helpDir *firstHelpDirs = NULL;

static class cursor *waitCursor; /* the watch cursor */

/* list of "good" file extensions */
static const char * const file_ext_array[] = FILE_EXTS;

/* the directory to store "missing" files in */
static const char missing_dir[] = MISSINGDIR;

static const char err_server[] = "Sorry; a file server is down.";
static const char err_index2[] = "Sorry; index cannot be found";



ATKdefineRegistry(helpdb, ATK, helpdb::InitializeClass);

static int  mysystem(const char  *acmd);
static int safeatoi(char  *astring);
static void EnumAllSplot(struct Index  *aindex, struct indexComponent  *ac, struct helpdb_EnumAllSplot  *rock);
static void ParseBaseName(const char  *aname, char  *abase);
static void ComputeMetric(struct helpFile  *ah);
static int Match(const char  *akey, const char  *afile, int  amatchName);
static void NotifyError(const char  *aname);
static char *LowerCase(char  *astring);
static struct helpFile *AddFilesFromDir(const char  *dname, const char  *aname, struct helpFile  *tmplist);
static struct helpFile *SetupHelpAux(const char  *aname, int  strip			/* whether to strip changes files */);


boolean helpdb::InitializeClass()
{
    char pathName[MAXPATHLEN];
    const char *tmp;

#ifdef DEBUGGING
    if ((char *)getenv("HELPDBDEBUG") != (char *) NULL)
	HELPDBDEBUG = 1;
#endif /* DEBUGGING */

    DEBUG(("db: IN init class\n"));

    waitCursor = cursor::Create(0);
    (waitCursor)->SetStandard( Cursor_Wait);

    tmp = environ::GetConfiguration(SETUP_ALIASDIR);
    if (tmp == NULL) {
	tmp = environ::AndrewDir(DEFAULT_ALIASDIR);
    }
    sprintf(pathName, "%s%s", tmp, ALIASFILE);
    helpdb::ReadAliasesFile(pathName);
    
    if (openIndex == (struct Index *)NULL) { /* if we haven't SetIndex */
	tmp = environ::GetConfiguration(SETUP_INDEXDIR);
	if (tmp == NULL) 
	    tmp = environ::AndrewDir(DEFAULT_INDEXDIR);
	helpdb::SetIndex(tmp);
    }
    return TRUE;
}
    

/*
 * Opens a given index file
 */
int helpdb::SetIndex(const char  *aindex)
{
	ATKinit;

    if (openIndex)
	index_Close(openIndex);
    if (access(aindex, 4) < 0) {
        indexErrno = errno;
        openIndex = (struct Index *) 0;
	fprintf(stderr, "help: cannot open index directory '%s'\n", aindex);
    } else {
        indexErrno = 0;
        openIndex = index_Open(aindex);
    }
    return indexErrno;
}


/*
 * checks the status of the help index and prints an error dialog
 */
int helpdb::CheckIndex(class view  *v)
{
	ATKinit;

    char err_buf[HELP_MAX_ERR_LENGTH * 2];

    if (openIndex == (struct Index *) 0) {
        /* index errno has the reason */
        if (indexErrno == ETIMEDOUT) {
	    sprintf(err_buf, "%s %s", err_server, err_index2);
	    if (v)
		ERRORBOX(v, err_buf);
        } else
	    if (v)
		ERRORBOX(v, err_index2);
	DEBUG(("db: check index fail\n"));
        return 0;
    }
    DEBUG(("db: check index OK\n"));
    return 1;
}


/*
 * just like system(3) only closes fds 3..., and doesn't wait
 */
static int 
mysystem(const char  *acmd)
{
    long pid;
    if(strchr(acmd, '`')) {
	fprintf(stderr, "help: command execution failed due to illegal character '`' in command.\n");
	return -1;
    }
    pid = osi_vfork();
    if (pid < 0) return -1;
    else if (pid == 0) {
        /* child, next close window mgr's fd, so that parent window can be killed */
        for(pid = 3; pid < FDTABLESIZE(); pid++) close(pid);
        execl("/bin/sh", "sh", "-c", acmd, (char *)NULL);
        _exit(127);
	/*NOTREACHED*/
    }
    else return 0;      /* parent, success */
}


/*
 * atoi that only converts numbers, a little safer...
 */
static int safeatoi(char  *astring)
{
    long value;
    char tc;
    
    value = 0;
    while ((tc = *astring++) != '\0') {
        if (tc >= '0' && tc <= '9') {
            value *= 10;
            value += tc - '0';
        }
    }
    return value;
}


/*
 * returns alias matching a string.  simple.
 */
const char *helpdb::MapAlias(const char  *alias)
{
	ATKinit;

    struct helpAlias *ta;
    
    for(ta=allAliases; ta; ta=ta->next) {
        if (strcmp(ta->alias, alias) == 0) return ta->original;
    }
    return NULL;
}

struct helpDir *helpdb::GetHelpDirs()
{
	ATKinit;

    return firstHelpDirs;
}

struct helpdb_EnumAllSplot {
    helpdb_efptr proc;
    char *ptr;
};

static void EnumAllSplot(struct Index  *aindex, struct indexComponent  *ac, struct helpdb_EnumAllSplot  *rock)
{
    if (ac && ac->name && (*(ac->name) != '\0')) {
	(*(rock->proc))(ac->name, ac->data, rock->ptr);
    }
}

/* call proc for each help alias and help index entry. proc should have the definition
void proc(char *name, char *original, rock) 
name is the help topic keyword; original is the filename (for index entries) or the real name of the alias (for aliases.) */
void helpdb::EnumerateAll(helpdb_efptr proc, char  *ptr)
{
	ATKinit;

    struct helpdb_EnumAllSplot heas;
    struct helpAlias *ta;

    heas.proc = proc;
    heas.ptr = ptr;
    for (ta=allAliases; ta; ta=ta->next) {
	(*proc)(ta->alias, ta->original, ptr);
    }
    index_Enumerate(openIndex, (index_efptr)EnumAllSplot, (char *)&heas);
}

/* call proc for each help index entry. proc should have the definition
void proc(struct Index *aindex, struct indexComponent *ac, rock) */
void helpdb::Enumerate(index_efptr proc, char  *ptr)
{
	ATKinit;

    index_Enumerate(openIndex, proc, ptr);
}


void helpdb::AddSearchDir(const char  *dirName)
{
	ATKinit;

    struct helpDir *thd, *lhd;
    const char *lastchar, *firstchar;
    int dirlen;

    if(dirName && (*dirName != (char)0)) {
	lastchar = dirName + strlen(dirName) - 1;
	firstchar = dirName;

	if(*lastchar ==	'/')	 /* remove trailing '/' */
	    lastchar--;
	while(firstchar && (*firstchar != (char)0) && isspace(*firstchar))	/* skip whitespace */
	    firstchar++;
	dirName = firstchar;
	dirlen = (int)(lastchar - firstchar);

	for(thd = firstHelpDirs, lhd = NULL; thd; lhd = thd, thd = thd->next)
	    if(!strncmp(dirName, thd->dirName, dirlen)) return;

	thd = (struct helpDir*) malloc(sizeof(struct helpDir));
	thd->next = NULL;	    /* Add new one on end of linked list */
	if (lhd) lhd->next = thd;
	else firstHelpDirs = thd;
	thd->dirName = (char *)malloc(dirlen + 1);
	memcpy(thd->dirName, dirName, dirlen);
	thd->dirName[dirlen] = 0;
    }
}


void helpdb::PrintSearchDirs()
{
	ATKinit;

    struct helpDir *thd;

    for (thd = firstHelpDirs; thd; thd=thd->next) {
	printf(thd->dirName);
    }
}


/*
 * Construct a list of aliases to be checked in case the index call
 * misses.
 */
void helpdb::ReadAliasesFile(const char  *aname)
{
	ATKinit;

    char original[HNSIZE+1];
    char alias[HNSIZE+1];
    FILE *tf;
    long code;
    struct helpAlias *ta;
    char *tp;
    int tc;
    static char done = 0;

    DEBUG(("db: reading alias file: %s\n", aname));

    if (done)
	return;

    /* read new file */
    tf = fopen(aname, "r");
    if (!tf) {
	fprintf(stderr, "help: cannot open aliases file '%s'\n", aname);
	return;
    } else {
        while(1) {
            /* read one line.  
	      states:
	      0 = reading alias
	      1 = skipping spaces
	      2 = copying original
	      3 =done with line
	      4 = done with line, but don't enter entry
	      5 = hit eof
            */
            code = 0;
            tp = alias;
            while (1) {
                tc = getc(tf);
                if (code == 0) {
                    if (tc == ' ' || tc == '\t') {
                        code = 1;
                        *tp++ = '\0';
		    } else 
			if (tc == '#' || tc == '!') { /*stay back compatible*/
			    /* comment line */
			    while (1) {
				tc = getc(tf);
				if (tc < 0 || tc == '\n')
				    break;
			    }
			    code = 4; /* pretend state is normal for end of line */
			    break;	/* process next line */
			} else
			    *tp++ = tc;
		} else 
		    if (code == 1) {
			if (tc != ' ' && tc != '\t') {
			    code = 2;
			    tp = original;
			    *tp++ = tc;
			}
		    } else 
			if (code == 2) {
			    if (tc == '\n' || tc == '!' || tc == '\t') {
				code = 3;
				*tp++ = '\0';
				break;
			    }
			    else 
				*tp++ = tc;
			}
		if (tc == '\n') {
		    code = 4;   /* premature newline */
		    break;
		}
		if (tc < 0) {
		    code = 5;   /* end of file, possibly premature */
		    break;
		}
	    }
	    if (code == 5) 
		break;
	    else 
		if (code != 3)
		    continue;
	    ta = (struct helpAlias *) malloc(sizeof (struct helpAlias));
	    ta->next = allAliases;
	    allAliases = ta;
	    ta->original = (char *) malloc(strlen(original)+1);
	    strcpy(ta->original, original);
	    ta->alias = (char *) malloc(strlen(alias)+1);
	    strcpy(ta->alias, alias);
	}
	fclose(tf);
    }
}


/*
 * returns a string sans extension, if any
 */
static void ParseBaseName(const char  *aname, char  *abase)
{
    const char *tp;
    
    tp = strrchr(aname, '.');
    if (tp) {
        strncpy(abase, aname, tp-aname);
        abase[tp-aname] = '\0';
    } else
        strcpy(abase, aname);
}


/*
 * comput metric based on file type and extension.
 * The higher the metric, the later the file will be shown
 */
static void ComputeMetric(struct helpFile  *ah)
{
    char *extension;
    char *tf;
    const char * const *defptr;
    int metric;

    metric = 0;
    tf = ah->fileName;
    extension = strrchr(tf, '.');
    ah->extPtr = extension;
    if (extension) {
        ah->extension = safeatoi(extension+1);
    } else {
        ah->extension = 0;
    }
    /* /usr/man is relatively unimportant */
    if (strncmp(tf, MANDIR, sizeof(MANDIR)) == 0)
	metric += 10;
    /* fortran is even worse! */
    if (extension && tf[strlen(tf)-1] == 'f')
	metric += 20;
    /* now reward for good file extensions */
    if (extension) {
	defptr = file_ext_array;
	while (defptr && *defptr != (char *) NULL)
	    if (strcmp(extension, *defptr++) == 0)
		metric -= 5;
    }
    metric += ah->extension;	/* all else fails, show foo.1 before foo.2 */
    ah->metric = metric;
}

/*
 * Complex file matching mechanism
 */
static int Match(const char  *akey, const char  *afile, int  amatchName)
{
    const char *keyExt, *fileExt;
    const char *tp;
    char *mtp;
    long tc;
    long keyValue, fileValue;
    char keyBase[64], fileBase[64];

    keyExt = strrchr(akey, '.');
    if (keyExt == akey) keyExt = NULL; /* if . is first, not extension */
    fileExt = strrchr(afile, '.');
    if (fileExt == afile) fileExt = NULL;     /* ditto */
    keyValue = fileValue = 0;

    /* compute extension values */
    if (keyExt) {
        tp = keyExt+1;      /* skip the '.' */
        while ((tc = *tp++) != '\0') {
            if (tc >= '0' && tc <= '9') {
                keyValue *= 10;
                keyValue += tc - '0';
            }
        }
    }
    if (fileExt) {
        tp = fileExt+1;     /* skip the '.' */
        while ((tc = *tp++) != '\0') {
            if (tc >= '0' && tc <= '9') {
                fileValue *= 10;
                fileValue += tc - '0';
            }
        }
    }
    
    /* compute basenames */
    strcpy(keyBase, akey);
    mtp = strrchr(keyBase,'.');
    if ((mtp != NULL) && mtp != keyBase)
	*mtp = '\0';
    
    if ((tp = strrchr(afile, '/')) != NULL)
        /* real path name */
        strcpy(fileBase, tp+1);
    else
	strcpy(fileBase, afile);
    /* now prune a '.' from the end */
    mtp = strrchr(fileBase, '.');
    if (mtp && mtp != fileBase) *mtp = '\0';
    
    /* finally ready to compare! */
    if(amatchName && strcmp(fileBase, keyBase)) return 0;
        /* no match if basenames differ */
    if (keyExt == NULL) return 1; /* no key ext spec'd, then win */
    if (fileExt == NULL) return 0; /* no file ext, but key ext, lose */
    /* here if both key and file have extensions, matching basenames */
    if (keyValue) {     /* looking for a number */
        if (keyValue == fileValue) return 1;
        else return 0;
    }
    /* otherwise must match exact extension */
    if (strcmp(keyExt, fileExt) == 0) return 1;
    else return 0;
}


/*
 * Sets up the c->all and c->cur fields of the passed in cache
 * to be a list of helpFiles for the passed in topic (aname).
 * If successful, returns 1, and c is modified.  If not, 0 is
 * returned and the cache isn't touched.  If the topic is a
 * command-running alias, run the command, and return 2
 */
int helpdb::SetupHelp(struct cache  *c, const char  *aname, int  strip			/* whether to strip changes files */)
{
	ATKinit;

    struct helpFile *tf, *nf;
    struct helpFile *al = NULL;
    struct helpFile *rl = NULL;

    const char *alias;

    /* check for an alias */
    alias = helpdb::MapAlias(aname);
    DEBUG(("db: mapalias on: %s\n",aname));
    if (alias) {
	DEBUG(("db: alias: %s\n", alias));
	if (alias[0] == '#') {
	    char msg[256];
	    sprintf(msg, "Running command: %.230s", &alias[1]);
	    im::SetProcessCursor(waitCursor);
	    message::DisplayString(c->view, 0, msg);
	    im::ForceUpdate();
	    mysystem(&alias[1]);
	    im::SetProcessCursor((class cursor *) NULL);
	    return 2;
	} else {
	    al = SetupHelpAux(alias, strip); /* alias list */
	}
    }

    rl = SetupHelpAux(aname, strip); /* ""real" list */

    if (rl || al) {		/* found something */

	/* free last set of help items */
	for(tf=c->all;tf;tf=nf) {
	    nf=tf->next;
	    free(tf->fileName);
	    free(tf);
	}

	/* make it official */
	strncpy(c->name, (alias) ? alias : aname,HNSIZE);

	/* if no alias list, return real */
	if (!al) {
	    c->cur = c->all = rl;
	    return 1;
	}

	/* if no real list, return alias */
	if (!rl) {
	    c->cur = c->all = al;
	    return 1;
	}

	/* if both, add real list after alias and return alias */
	for (tf = al; tf; tf=nf) {
	    nf = tf->next;
	    if (!nf) {
		tf->next = rl;
		c->cur = c->all = al;
		return 1;
	    }
	}
	
    } else {			/* found nothing */
	DEBUG(("db: found nothing\n"));
	NotifyError(aname);
	return 0;
    }
    return 0;
}


/*
 * Notify Error - stores a file with a useful filename when help cannot
 * find help on a topic.  Filename is Missing.name.number in MISSINGDIR
 * when help doesn't find an index hit.  Increments 'number' each subsequent miss.
 */
static void NotifyError(const char  *aname)
{
    /* tname is the full path to the "Miss" file, without the number
       	  appended
       dname is the Miss file directory
       bname is the base name of the "Miss" file, ie Missing.subject,
           without the appended number
       we compare the portion of the file name /path/path/Missing.name.n
           from the last '/' to the last '.' to bname for matches.
    */
    char tname[MAXPATHLEN], dname[MAXPATHLEN], *bname, *lastdot;
    const char *helpDir;
    int fd, sofar, found = 0;
    DIR *dd;
    DIRENT_TYPE *ent;
    
    helpDir = environ::GetConfiguration(SETUP_MISSINGDIR);
    if (helpDir == NULL)
	helpDir = environ::AndrewDir(DEFAULT_MISSINGDIR);
    sprintf(tname, "%s%s/Missing.%s", helpDir, missing_dir, aname);
    DEBUG(("db: missing name: %s\n",tname));
    bname = strrchr(tname,'/');
    if (bname)
	bname = bname+1;
    else
	bname = tname;
    sprintf(dname, "%s%s", helpDir, missing_dir);

    if ((dd = opendir(dname)) == (DIR *)NULL)
        return;
    while (((ent = readdir(dd)) != (DIRENT_TYPE *)NULL) && !found) {
	/* 'remove' numerical extension */
	if ((lastdot = strrchr(ent->d_name, '.')) != NULL)
	    *lastdot = '\0';
	if (!strcmp(ent->d_name, bname)) { /* found it */
	    /* put extension back */
	    if (lastdot) *lastdot = '.';
	    sofar = atoi(strrchr(ent->d_name, '.')+1);
	    sprintf(tname, "%s%s/Missing.%s.%d",
		    helpDir, missing_dir, aname, sofar);
	    unlink(tname);	/* delete the old entry */
	    sprintf(tname, "%s%s/Missing.%s.%d",
		    helpDir, missing_dir, aname, ++sofar);
	    found = 1;
	}
    }
    if (!found)			/* make the first entry */
	strcat(tname, ".1");
    fd = open(tname, O_RDWR | O_CREAT | O_TRUNC, 0666);
    closedir(dd);
    close(fd);
}

static char *LowerCase(char  *astring)
{
    char *tp = astring;

    while (tp && *tp != '\0')
	if (isupper(*tp)) {
	    *tp = tolower(*tp);
	    tp++;
	} else
	    tp++;
    return astring;
}

/*
 * Given a directory path "dname", adds all files in that directory
 * that match topic "aname" to the list "tmplist".
 */
static struct helpFile *AddFilesFromDir(const char  *dname, const char  *aname, struct helpFile  *tmplist)
{
    struct helpFile *tf, *nf, **ef;
    DIR *tempdir;
    DIRENT_TYPE *tde;
    char tfn[MAXPATHLEN];
    char *tfp;
    
    tempdir = opendir(dname);
    if (tempdir == (DIR *)NULL) {/* don't use unopened directory */
	return tmplist;
    }
    
    strcpy(tfn, dname); /* make a base for filenames dir/dir/ */
    tfp = tfn + strlen(tfn);
    *tfp++ = '/';	/* tfp points just after the last '/' */
    
    while((tde=readdir(tempdir)) != NULL) {
	struct stat buf;
	char lfname[MAXPATHLEN];
	strcpy(lfname, tde->d_name);
	LowerCase(lfname);
	if (!Match(aname, lfname, 1)) continue;
	/* don't add directories */
	strcpy(tfp, tde->d_name); /* finish the filename */
	if ((stat(tfn, &buf) != 0) || (buf.st_mode & S_IFDIR)) {
	    DEBUG(("db: dir or non-stat: '%s'\n",tfn));
	    continue;
	}
	
	tf = (struct helpFile *) malloc(sizeof(struct helpFile));
	/* allocate enough room for dir, file, slash between them
	   and a null at the end */
	tf->fileName =
	    (char *) malloc(strlen(dname) + 1 + DIRENT_NAMELEN(tde) + 1);
	strcpy(tf->fileName, dname);
	strcat(tf->fileName, "/");
	strcat(tf->fileName, tde->d_name);
	/* now, thread on in the appropriate spot */
	ComputeMetric(tf);
	tf->metric -= 30;	/* give precedence to searched files */

	ef = &tmplist;
        for(nf = *ef; nf; nf=nf->next) {
            if (nf->metric > tf->metric) break;
            ef = &nf->next;
        }
        tf->next = *ef;
        *ef = tf;
    }
    closedir(tempdir);
    return tmplist;
}
    


/*
 * Returns a linked list of help files for topic 'aname'. Looks in both the
 * index and search directories for files.  If no files found, returns NULL.
 * If strip is non-zero, strips files with extensions CHANGE_EXT and TUTORIAL_EXT
 * from the returned list
 */
static struct helpFile *SetupHelpAux(const char  *aname, int  strip			/* whether to strip changes files */)
{
    long i;
    struct helpFile *t, *p, *n, **ef;
    struct helpFile *tmplist;
    struct recordSet *ts;
    struct helpDir *thd;
    char baseName[HNSIZE];
    char pathName[MAXPATHLEN];
    char unfolded[MAXPATHLEN];
    const char *usethis=NULL;
    long code;
    
    ParseBaseName(aname, baseName);
    
    if (!openIndex)
	return NULL;
    
    tmplist = (struct helpFile *)NULL;
    
    ts = index_GetAnySet(openIndex, baseName);
    for(i=0;i<ts->count;i++) {
        code = index_GetData(openIndex, &ts->data[i], pathName,
			     sizeof(pathName));
        if (code)
	    continue;             /* mysterious error */
	
	usethis=path::UnfoldFileName(pathName, unfolded, NULL);
	
        if (!Match(aname, usethis, 0))
	    continue;
        t = (struct helpFile *) malloc (sizeof(struct helpFile));
        t->fileName = (char *) malloc(strlen(usethis) + 1);
        strcpy(t->fileName, usethis);

        /* now, thread on the list in order based on actual file name */
        ComputeMetric(t);

        ef = &tmplist;
        for(n = *ef; n; n=n->next) {
            if (n->metric > t->metric) break;
            ef = &n->next;
        }
        t->next = *ef;
        *ef = t;
    }

    /* now add the auxiliary help units */

    for(thd = firstHelpDirs; thd; thd = thd->next) {
	char *tmp;
	const char *subdir = MANSUBS;
	char dir[MAXPATHLEN];

	tmp = strrchr(thd->dirName, '/');
	DEBUG(("thd->dirName: %s\n", thd->dirName));
	if (tmp)
	    tmp = tmp+1;
	else
	    tmp = thd->dirName;
	if (!strcmp(tmp, "man")) {
	    strcpy(dir, thd->dirName);
	    strcat(dir, "/man");
	    tmp = dir + strlen(dir);
	    *(tmp + 1) = '\0';
	    while (*subdir) {
		*tmp = *subdir++;
		tmplist = AddFilesFromDir(dir, aname, tmplist);
	    }
	} else
	    tmplist = AddFilesFromDir(thd->dirName, aname, tmplist);
    }

#ifdef DEBUGGING
    DEBUG(("db: For topic %s:\n", aname));
    ef = &tmplist;
    for (n = *ef; n; n=n->next)
	DEBUG(("db: \tName: %s\tMetric: %d\n",n->fileName,n->metric));
#endif /* DEBUGGGING */

    recordset_Free(ts);
    if (tmplist == (struct helpFile *)NULL) {
	DEBUG(("db: setup returning null\n"));
	return NULL;
    } else {
	/* now change global since we have a non-empty list */
	if (!strip) {
	    DEBUG(("db: setup return nostrip\n"));
	    return tmplist;
	}

	/* now, on request of the ASA documentation group,
	   remove any file that is a "changes" file. */
	p = NULL;
	t = tmplist;
	while (t) {
	    if (t->extPtr && (!strncmp(t->extPtr,CHANGE_EXT,sizeof(CHANGE_EXT)) ||
		!strncmp(t->extPtr,TUTORIAL_EXT,sizeof(TUTORIAL_EXT)))) {
		if (p == NULL) { /* removing head */
		    tmplist = t->next;
		    free(t->fileName);
		    free(t);
		    t = tmplist;
		} else {
		    p->next = t->next;
		    free(t->fileName);
		    free(t);
		    t = p->next;
		}
	    } else {
		p = t;
		t = t->next;
	    }
	}

	DEBUG(("db: setup return strip\n"));
	return tmplist;
    }
}

