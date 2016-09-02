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
/*	MODULE: help.c							     */
/*		Subclass of view, includes all major lookup and display      */
/*		routines.						     */
/*---------------------------------------------------------------------------*/



#define label gezornenplatz
/* sys/types.h in AIX PS2 defines "struct label", causing a type name clash.
   Avoid this by temporarily redefining "label" to be something else in the preprocessor. */
#include <andrewos.h> /* sys/types.h sys/file.h */
ATK_IMPL("help.H")
#undef label

#include "config.h"
#include <help.H>
#include <bind.H>
#include <proctable.H>
#include <dataobject.H>
#include <environ.H>
#include <filetype.H>
#include <im.H>
#include <label.H>
#include <labelview.H>
#include <lpair.H>
#include <menulist.H>
#include <keymap.H>
#include <message.H>
#include <panel.H>
#include <rofftext.H>
#include <scroll.H>
#include <text.H>
#include <textview.H>
#include <attribs.h>
#include <view.H>

#include <errno.h>
#include <ctype.h>
#if POSIX_ENV || defined(M_UNIX)
#include <dirent.h>
#else
#define dirent direct
#endif
#include <sys/errno.h>
#include <sys/param.h>

#include <index.h>
#include <regexp.h> /* use the reg expression routines in overhead */

#include <helpsys.h>
#include <help.h>
#include <helpdb.H>

#include <helpaux.h>

/*---------------------------------------------------------------------------*/
/*				GLOBALS					     */
/*---------------------------------------------------------------------------*/

/* the new menu and key states */
class keymap *Help_Map;
class menulist *Help_Menus;

char *help_tutorialDirs[MAX_TUTORIAL_DIRS];
char help_changesDir[MAXPATHLEN];

#define USE_PRINTOPTS /* used to be in contrib, but now we should always use it. */

/* a list of instances of help */
struct self_help *help_ego = (struct self_help *)NULL;

class cursor *help_waitCursor; /* the watch cursor */

char **help_panelList = NULL; /* used for enumerating across the help index */
int help_panelIndex = 0, help_panelListSize = help_MAXPANEL;

/* hooks to textview and frame procs */
proctable_fptr help_textSearch = NULL;
proctable_fptr help_textRevSearch = NULL;
proctable_fptr help_textSearchAgain = NULL;
proctable_fptr help_textCopyRegion = NULL;
proctable_fptr help_textPageDown = NULL;
proctable_fptr help_textPageUp = NULL;
proctable_fptr help_frameSetPrinter = NULL;
#ifdef USE_PRINTOPTS
proctable_fptr help_poptPostWindow = NULL;
#endif

static int packedString[] = {037, 036, 0, 0};
static int compressedString[] = {037, 0235, 0220};
static int gzippedString[] = {037, 0213, 010, 010};
struct filterinfo {
    const char *cmd;
    int len;
    int *magic;
    boolean possible;
};
struct filterinfo filters[] = { /* possible field modified below */
    {"zcat", 3, compressedString, 0},
    {"pcat", 4, packedString, 0},
    {"gunzip", 4, gzippedString, 0},
    {0, 0, 0, 0}
};
const char colcmd[] = "col -b | tr '\t' ' '";

/*---------------------------------------------------------------------------*/
/*			CONDITIONAL DEBUGGING				     */
/*---------------------------------------------------------------------------*/

#ifdef DEBUGGING
/*
 * debugging statements can be included by compiling add modules with
 * -DDEBUGGING.  Debugging output is toggled by existance/nonexistance
 * of the environment variable HELPDEBUG.
 */
int HELPDEBUG = 0;
#undef DEBUG
#define DEBUG(arg) if (HELPDEBUG != 0) { printf arg; fflush(stdout); }
#else
#undef DEBUG
#define DEBUG(arg)
#endif /* DEBUGGING */


/*---------------------------------------------------------------------------*/
/*				UTILITY FUNCTIONS			     */
/*---------------------------------------------------------------------------*/

/*
 * copy protoname into aresult, prepending /usr/andy or whatever,
 * as appropriate
 */

ATKdefineRegistry(help, view, help::InitializeClass);
static char * AndyCopy(char  *aproto , char  *aresult);
static char * CopyString(char  *as);
static int  ScanLine(FILE  *afile, char  *ae1 , char  *ae2);
static int safesystem(char  *acmd);
static int  mysystem(char  *acmd);
char * LowerCase(char  *astring);
static char * MapParens(char  *s);
static char * sindex(char  *big , char  *small);
void  AddHistoryItem (class help  *self, int  marcp			/* is this a bookmark? */, int  flash			/* should we expose the history panel? */);
static int  ShowFile(class help  *self, const char  *afilename	/* the file */, int  amoreFlag			/* put "(more)" in the titlebar?
				   TRUE - yes, FALSE - no,
				   help_USE_OLD_TITLE - use last setting */, int  hist			/* add this previous file to the history?
				   help_HIST_NOADD - none at all,
				   help_HIST_NAME - aname,
				   help_HIST_TAIL - tail of the found filename
				   */);
static const char * FindEntryInDirs(const char	 * const dirs[], const char	 *entry  , const char	 *extension);
int  help_GetHelpOn(class help  *self, const char  *aname	/* what topic */, long  isnew	/* is this a new topic? */, int  ahistory	/* show in history log under what name?
		   help_HIST_NOADD - none at all,
		   help_HIST_NAME - aname,
		   help_HIST_TAIL - tail of the found filename
		   */, const char  *errmsg	/* error to print if failure. "Error" if this is NULL */);
void  SetupMenus(struct cache  *c);
static void  SearchOverviews(class help * self);
static void  SearchPrograms(class help * self);
static void  TextviewProc(class help * self, long  rock);
static void  Quit(class help  *self);
static void  SendComments(class help  *self);
static void  ShowTutorial(class help  *self);
static void  ShowChanges(class help  *self);
void NextHelp(class help  *self);
void  HistoryHelp(class help  *self		/* callback rock */, struct history_entry  *ent	/* panelEntry rock */, class panel  *apanel		/* appropriate panel */);
void  OverviewHelp(class help  *self, char  *name		/* which topic to request - panelEntry rock */, class panel  *apanel);
static boolean EnsurePanelListSize();
void FreePanelListData();
long SetupPanel(boolean  readpairs, const char  *fname, class panel  *panel		/* the panel to add entries to */, const char  * const *def);
class view *SetupLpairs(class help  *self);
static void  TogglePanels(class help  *self, long  rock);
static void  ToggleOverviews(class help  *self, long  rock);
static void  TogglePrograms(class help  *self, long  rock);
static void  ToggleHistory(class help  *self, long  rock);
static void  ExpanderAux(char  *dname);
static int  panelCompare(char  **s1 , char  **s2);
static void  SortAndMakePanel(class panel  *p);
static char * AddToPanelList(char  *s);
static void  Expander(struct Index  *aindex, struct indexComponent  *ac, class help  *self);
void  ToggleProgramListSize(class help * self, long  rock);
static void  RestorePanel(class help  *self);
static void  FilterPanel(class help  *self, long  rock);
static void  nono(class help  *self);


static char *
AndyCopy(char  *aproto , char  *aresult)
{
    const char *tp;

    tp = environ::AndrewDir(aproto);
    strcpy(aresult, tp);
    return aresult;
}

/*
 * allocate new string
 */
static char *
CopyString(char  *as)
{
    char *tp;
    tp = (char*) malloc(strlen(as)+1);
    if (!tp)
	return NULL;
    strcpy(tp, as);
    return tp;
}

/*
 * parse comma terminated field followed by new-line terminated field
 */
static int 
ScanLine(FILE  *afile, char  *ae1 , char  *ae2)
{
    int state;		/* 0->reading ae1, 1->reading ae2 */
    int tc;		/* char we're reading */
    char *tp;		/* points to string */

    state = 0;
    tp = ae1;
    while(1) {
        tc = getc(afile);
        if (tc < 0 || tc == '\n') {
            /* done */
            if (state == 1) {
                *tp++ = 0;
                return 2;
            } else return 0;
        }
        if (state == 0 && tc == ',') {
            state = 1;
            *tp++ = 0;
            tp = ae2;
            continue;
        }
        *tp++ = tc;
    }
}

/* just like system, but fail if the command to be executed has a '`' in it. */
static int safesystem(char  *acmd)
{
    if(strchr(acmd, '`')) {
	fprintf(stderr, "help: command execution failed due to illegal character '`' in command.\n");
	return -1;
    }
    return system(acmd);
}

/*
 * just like system(3) only closes fds 3..., and doesn't wait
 */
static int 
mysystem(char  *acmd)
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
        execl("/bin/sh", "sh", "-c", acmd, NULL);
        _exit(127);
	/*NOTREACHED*/
    }
    else return 0;      /* parent, success */
}


/*
 * lowercases's a string.
 */
char *
LowerCase(char  *astring)
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
 * maps string(n) to string.n in place
 */
static char *
MapParens(char  *s)
{
    char *lpp, *rpp;

    if (((lpp = strrchr(s, '(')) != NULL) &&  /* has a ( */
	((rpp = strrchr(s, ')')) != NULL) &&  /* and a ) */
	(*(rpp+1) == '\0')) {			     /* ) is the last char */
	*lpp = '.';
	*rpp = '\0';
	return s;
    }
    return s;
}


/*
 * stolen from libcs.  Returns the index of string small in big, 0 otherwise
 */
static char *
sindex(char  *big , char  *small) 
{
    char *bp, *bp1, *sp;
    char c = *small++;

    if (c==0) return(0);
    for (bp=big;  *bp;  bp++)
	if (*bp == c) {
	    for (sp=small,bp1=bp+1;   *sp && *sp == *bp1++;  sp++)
		;
	    if (*sp==0) return(bp);
	}
    return 0;
}

/*
 * add an item to the history buffer
 */
void 
AddHistoryItem (class help  *self, int  marcp			/* is this a bookmark? */, int  flash			/* should we expose the history panel? */)
{
    struct history_entry *ent;
    struct cache *c = self->info;

    DEBUG(("addhist\n"));
    if (c->histent == NULL || c->histent[0] == '\0')
	return;

    ent = (struct history_entry *) malloc (sizeof(struct history_entry));
    if (ent == NULL)
	return;

    ent->pos = ((class textview *)c->view)->GetDotPosition(),
    ent->top = ((class textview *)c->view)->GetTopPosition(),
    ent->sellen = ((class textview *)c->view)->GetDotLength(),
    ent->fname = (char *) malloc (strlen(c->histent) + 1);
    strcpy(ent->fname, c->histent);

    if (marcp == help_HE_HISTORY) { /* normal history add */
	
	/* now add the item */
	c->lastHist = (c->hist)->Add( c->histent, (char *)ent, TRUE);
    } else {			/* a bookmark */
	char tfname[HNSIZE + 11];

	sprintf(tfname, "%s @ %ld", c->histent, ent->top);
	/* now add the item */
	c->lastHist = (c->hist)->Add( tfname, (char *)ent, TRUE);
    }

    if (flash == help_SHOW_HIST)
	ToggleHistory(self, help_SHOW_HIST);

    DEBUG(("OUT addhist\n"));
}


/*---------------------------------------------------------------------------*/
/*				MAIN FUNCTIONS				     */
/*---------------------------------------------------------------------------*/

/*
 * Help's file displayer - given a filename, attempts to find out if
 * it's a roff file (1st char '.' or '#' or '\'') and read it in using rofftext.
 * Otherwise, assumes it's text or ATK format and lets text_read take care
 * of the dirty work of dealing with that.
 *
 * Can display any file that uses a textview or a subclass of textview.
 * Creates a new view and dataobject for the file and reads the file into the
 * new object.
 *
 * Also takes care of adding elements to the history panel for the last-shown
 * help file.
 */
static int 
ShowFile(class help  *self, const char  *afilename	/* the file */, int  amoreFlag			/* put "(more)" in the titlebar?
				   TRUE - yes, FALSE - no,
				   help_USE_OLD_TITLE - use last setting */, int  hist			/* add this previous file to the history?
				   help_HIST_NOADD - none at all,
				   help_HIST_NAME - aname,
				   help_HIST_TAIL - tail of the found filename
				   */)
{
    FILE *fd;
    long objectID;
    static const char *manfiles[] = MACFILES; /* entries may be modifed below */
    const char **manptr = manfiles;
    char tbuffer[MAXPATHLEN];
    char isTroffFormat = 0;
    int tc;
    static int lastmore;
    char outfile[MAXPATHLEN];
    boolean inCat;
    int pos;
    int useFilter;
    boolean checkFilters;
    boolean hlp_view = FALSE;
    int ind;
    boolean reOpen;
    const char *p, *nextp;
    const char *realfilename = afilename;


    struct attributes attrs, *attr;

    const char *viewName, *objName;
    char *tmp;
    const char *t;

    struct cache *c = self->info; /* view and dataobject to replace with new file object */

    class view *newview;
    class dataobject *newdata;
    class view *oldview = c->view;
    class dataobject *olddata = c->data;

    if (message::Asking(help_ego->this_c->info->view)) {
	DEBUG(("retracting\n"));
	message::CancelQuestion(help_ego->this_c->info->view);
    }

    /* Check if file exists */
    if (!(fd = fopen(realfilename, "r"))) {
	char buf[HELP_MAX_ERR_LENGTH *2];
	
	if (access(realfilename, 4) < 0) {
	    if (errno == ETIMEDOUT)
		sprintf(buf, "%s %s", err_server, err_read);
	    else
		sprintf(buf, err_missing, afilename);
	    ERRORBOX(c->view, buf);
	} else ERRORBOX(c->view, err_read);
	return 0;
    }
    /* is it troff ? */
    tc = getc(fd);

    useFilter = -1;
    checkFilters = FALSE;
    reOpen = FALSE;
    for (ind = 0; filters[ind].cmd != 0; ind++) {
	if ((filters[ind].possible = (tc == *filters[ind].magic))) {
	    checkFilters = TRUE;
	    if (filters[ind].len == 1) {
		useFilter = ind;
	    }
	}
    }

    if (checkFilters) {
	/* possible filter to be applied */
	pos = 1;
	reOpen = TRUE;
	while (checkFilters && useFilter == -1) {
	    tc = getc(fd);
	    checkFilters = FALSE;
	    for (ind = 0; filters[ind].cmd != 0; ind++) {
		if (filters[ind].possible) {
		    if ((filters[ind].possible = (tc ==	filters[ind].magic[pos])))  {
			checkFilters = TRUE;
			if (filters[ind].len == pos + 1) {
			    useFilter = ind;
			}
		    }
		}
	    }
	    pos++;
	}
    }

    inCat = FALSE;
    for (p = strchr(afilename, '/'); p != NULL; p = nextp) {
	p++;
	nextp = strchr(p,'/');
	if (strncmp(p, "man", 3) == 0 && nextp) {
	    inCat = FALSE;
	}
	else if (strncmp(p, "cat", 3) == 0 && nextp) {
	    inCat = TRUE;
	}
    }

    if (useFilter >= 0 || inCat) {
	static int count;
	char cmd[MAXPATHLEN + 50];

	fclose(fd);
	reOpen = TRUE;
	sprintf(outfile, "/tmp/hfil.%d.%d", getpid(), count++);

	if (useFilter >= 0 && inCat) {
	    sprintf(cmd, "%s < %s | %s > %s", filters[useFilter].cmd, afilename, colcmd, outfile);
	}
	else if (useFilter >= 0) {
	    sprintf(cmd, "%s < %s > %s", filters[useFilter].cmd, afilename, outfile);
	}
	else {
	    sprintf(cmd, "cat %s | %s > %s", afilename, colcmd, outfile);
	}
	safesystem(cmd);
	realfilename = outfile;
    }
    else if (reOpen) {
	fclose(fd);
    }


    if (reOpen) {
	if (! (fd = fopen(realfilename, "r"))) {
	    char buf[HELP_MAX_ERR_LENGTH *2];

	    if (access(realfilename, 4) < 0) {
		if (errno == ETIMEDOUT)
		    sprintf(buf, "%s %s", err_server, err_read);
		else
		    sprintf(buf, err_missing, realfilename);
		ERRORBOX(c->view, buf);
	    } else ERRORBOX(c->view, err_read);
	    return 0;
	}
	if (realfilename == outfile) {
	    unlink(realfilename);
	}
	tc = getc(fd);
    }

    ungetc(tc, fd);
    isTroffFormat = (tc == '.' || tc == '#' || tc == '\'');

    /* what kind of object does this file want ? (don't touch attributes) */
    objName = filetype::Lookup(NULL, realfilename, NULL, NULL);
    if ((newdata = (class dataobject *) ATK::NewObject(objName)) ==
	(class dataobject *)NULL) {
	sprintf(tbuffer, err_dobj, objName);
	ERRORBOX(c->view, tbuffer);
	return 0;
    }

    /* what kind of view do we need ? */
    viewName = (newdata)->ViewName();
    if (strcmp(viewName, "textview") == 0) {
	hlp_view = TRUE;
	viewName = "hlptextview";   /* Provides hypertext functions */
    }
    if ((newview = (class view *)ATK::NewObject(viewName)) ==
	(class view *)NULL) {
	sprintf(tbuffer, err_view, viewName);
	ERRORBOX(c->view, tbuffer);
	(newdata)->Destroy();
	return 0;
    }
    if (hlp_view)
	init_hlptextview((class hlptextview *)newview);  /* Put it in helpaux because this file already almost blows cpp. */

    /* is it a subclass of textview? */
    if (!ATK::IsTypeByName((newview)->GetTypeName(), "textview")) {
	/* not text, punt */
	sprintf(tbuffer, err_nontext, viewName);
	ERRORBOX(c->view, tbuffer);
	(newview)->Destroy();
	(newdata)->Destroy();
	return 0;
    }

    /* hook 'em up */
    (newview)->SetDataObject( newdata);

    /* since now committed, take care of the history for previous item, if we
       haven't already done so, and we should */

    if (c->histat != help_HIST_NOADD) {

	DEBUG(("hist... getting hist on %d\n", (int)c->view));
	
	/* save position of last view's top, dot and dotlen */
	
	AddHistoryItem(self, help_HE_HISTORY, help_IGNORE_TOGGLE);
	
	/* only add this entry once */
	c->histat = help_HIST_NOADD;
    }

    attrs.next = (struct attributes *)NULL;
    attrs.key = "readonly";
    attrs.value.integer = 0;
    ((class text *)newdata)->SetAttributes( &attrs);
    fseek(fd, 0 ,0);	/* somebody moved the pointer */
    attr = NULL;
    filetype::Lookup(fd, realfilename, &objectID, &attr);
    if (attr != NULL)
	((class text *)newdata)->SetAttributes( attr);
    if (!isTroffFormat) {
	if (((class text *)newdata)->Read( fd, objectID) != dataobject_NOREADERROR) {
	    ERRORBOX(c->view, err_read);
	    (newview)->Destroy();
	    (newdata)->Destroy();
	    return 0;
	}
    } else {			/* it's troff */
	const char *manroot = "/usr/man", *slash;
	char mroot[strlen(afilename) + 1];

	/* check if the manfiles are absolute or relative paths.
	   If 1st char is '/', path is absolute, OW, we andycopy it
	   into a temp buffer, since it will be relative to ANDREWDIR */

	while (*manptr) {
	    if (**manptr != '/') {
		tmp = (char *)malloc(strlen(*manptr)+2);
		strcpy(tmp, "/");
		strcat(tmp, *manptr);
		*manptr = AndyCopy(tmp, (char *)malloc(MAXPATHLEN));
		free(tmp);
	    }
	    manptr++;
	}
	message::DisplayString(c->view, 0, msg_converting);
	im::ForceUpdate();

	/* ======== START of change to root of man pages ========== */
	slash = strchr(afilename, '/');
	while(slash && (slash+1)) {
	    if(!strncmp(slash+1,"man",3) && *(slash+4) == '/') {
		/* found substring "man" somewhere in afilename; chdir there */
		strcpy(mroot, afilename);
		mroot[slash - afilename + 4] = 0;
		manroot = mroot;
		break;
	    }
	    else 
		slash = strchr(slash + 1, '/');
	}
	if(!slash) {	/* troff file not located in standard man path */
	    /* Chdir to directory containing afilename */
	    strcpy(mroot, afilename);
	    manroot = mroot;
	    slash = strrchr(afilename,'/');
	    if(slash) mroot[slash - afilename] = 0;
	}
	chdir(manroot);
	/* ======== END of change to root of man pages ========== */

	if (rofftext::ReadRoffIntoText((class text *)newdata, fd, 0, manfiles) != dataobject_NOREADERROR) {
	    ERRORBOX(c->view, err_read);
	    (newview)->Destroy();
	    (newdata)->Destroy();
	    return 0;
	}
	message::DisplayString(c->view, 0, "");
    }
    ((class textview *)newview)->SetTopPosition( 0);
    ((class textview *)newview)->SetDotPosition( 0);
    ((class textview *)newview)->SetDotLength( 0);
    attrs.value.integer = 1;
    ((class text *)newdata)->SetAttributes( &attrs);

    /* now swap the old with the new */
    DEBUG(("swapping..."));
    (c->view)->UnlinkTree();
    c->view = newview;
    c->data = newdata;

    /* now set up the history state */

    c->histat = hist;

    switch (hist) {
      case help_HIST_NAME:
	strcpy(c->histent, c->name);
	break;
      case help_HIST_NOADD:	/* do this so that bookmarks will work correctly */
      case help_HIST_TAIL:
	t = strrchr(afilename,'/');
	strcpy(c->histent, (tmp != NULL) ? t+1 : afilename);
    }
    DEBUG(("setting histent to: %s\n", c->histent));

    /* now hook the new view up to the scrollbar */
    (c->scroll)->SetView( c->view); /* does a linktree */
    ((class textview *) c->view)->SetEmbeddedBorder( 20, 5);
    (c->data)->NotifyObservers( (long)0);

    /* get rid of the old view and dataobject*/
    DEBUG(("destroying... %d ",(int)oldview));
    (oldview)->Destroy();
    (olddata)->Destroy();

    strcpy(tbuffer, afilename);

    DEBUG(("titling\n"));

    if ((amoreFlag == TRUE) || ((amoreFlag == help_USE_OLD_TITLE) && lastmore == TRUE))
	strcat(tbuffer, " (more)");
    if (lastmore != help_USE_OLD_TITLE)
	lastmore = amoreFlag;
    (((class view *)c->view)->GetIM())->SetTitle( tbuffer);
    
    (self)->view::WantInputFocus( c->view);
    
    fclose(fd);

    DEBUG(("showfile returning\n"));
    return 1;
    /* phew! */
}

/*
FindEntryInDirs( dirs, entry, extension ) 
char *dirs[];	A list of directories to be searched for a given entry.
char *entry;	The entry to be searched for in dirs sans extension.
char *extension;    The extension that will be concatenated to entry for searching purposes.

   Purpose: This routine is used to find a specified file within an ordered list of possible directories that are specified via that AndrewSetup file (see config.h for possible AndrewSetup entries).  The dirs[] are a NULL-terminated list of pointers to character strings.  Entry is a pointer to a character string that is a file-name w/o the extension.  Extension is the extension that will be placed at the end of entry.  The first match that is found is returned in a static memory area. DON'T FREE THE VALUE THAT IS RETURNED... COPY IT TO SOME WORKING AREA OR USE IT BEFORE MAKING ANOTHER CALL TO THIS ROUTINE.

   Used in: HistoryHelp(), ShowTutorial(), GetHelpOn(). NOTE: this routine is currently only used to find tutorial files because it is useful to be able to place tutorials in various places ( /usr/local/help, /usr/andrew/help, ...).  In the future other files (like .help files) may have various homes and this routine should be used to access them.
*/

static const char *
FindEntryInDirs(const char	 * const dirs[], const char	 *entry  , const char	 *extension)
{
  static char	fullPath[MAXPATHLEN];
  char		*returnPath = NULL;
  int		i = 0;
  
  *fullPath = '\0';
  if(entry && dirs && *dirs) {
      for(i = 0; dirs[i] && *dirs[i] ; i++) {
	  sprintf(fullPath,"%s/%s%s",dirs[i],entry,extension);
	  if(access(fullPath, 0) == 0) break;
	  else *fullPath = '\0';
      }
  }
  if(*fullPath != '\0')
      returnPath = fullPath;
  return(returnPath);
}

/*
 * Main function for getting help on a string
 *
 * Returns:
 *	-1: if a server was down while trying to get help on this topic
 *	 0: if no help found for this topic
 *	 1: if successful

    Pseudo-code:
{
    if (index not open)
	return error;
    lowercase(topic);
    map topic(n) -> topic.n;
    if (first time for this topic)
	code = setuphelp for this topic;
    if (code != error)
	setupmenus if this topic has tutorial or changes files;
    if (empty file list)
	out of files, return error;
    else if (code != error) {
	show current file;
	check if we should erase the "show more files" menu item;
    }
    if (code == error)
	show error message;
    else
	advance current file pointer to next file;
    return code;
}
*/
int 
help_GetHelpOn(class help  *self, const char  *aname	/* what topic */, long  isnew	/* is this a new topic? */, int  ahistory	/* show in history log under what name?
		   help_HIST_NOADD - none at all,
		   help_HIST_NAME - aname,
		   help_HIST_TAIL - tail of the found filename
		   */, const char  *errmsg	/* error to print if failure. "Error" if this is NULL */)
{
    struct cache *c = self->info;
    char tbuffer[MAXPATHLEN];
    char helpBuffer[HNSIZE + 200];
    int code = 1;

    DEBUG(("%s get help: setup(%s)\n",(isnew) ? "new" : "old", aname));

    if (!helpdb::CheckIndex(c->view))
	return -1;

    /* tjm - hopefully no callers expect these side effects */
    char *an = strdup(aname);
    LowerCase(an);
    MapParens(an);

     if (isnew) {		/* if the first time through, setup c->all */
	 code = helpdb::SetupHelp(c, an, TRUE);

#ifdef DEBUGGING
	if (code != 0) {
	    DEBUG(("set found something\n"));
	} else {
	    DEBUG(("code to 0, tmplist == NULL\n"));
	}
#endif /*DEBUGGING*/

	if (code != 0) {
	    /* initially turn off changes and tutorial menus */
	    c->flags &= ~(MENU_SwitchChangesMenu | MENU_SwitchTutorialMenu | MENU_SwitchMoreMenu);
	
	    /* should we display "show tutorial" or "show changes" menus? */
	    if(FindEntryInDirs(help_tutorialDirs, c->name, TUTORIAL_EXT)) {
		if (!(c->flags & MENU_SwitchTutorialMenu))
		    c->flags |= MENU_SwitchTutorialMenu;
	    }
	    sprintf(tbuffer, "%s/%s%s", help_changesDir, c->name, CHANGE_EXT);
	    if (access(tbuffer, 0) == 0 && !(c->flags & MENU_SwitchChangesMenu))
		c->flags |= MENU_SwitchChangesMenu;
	}
    }
    /* tjm - hopefully noone is holding on to a pointer to this */
    free(an);

    im::SetProcessCursor(help_waitCursor);

    if (c->cur && (code == 1)) {
	DEBUG(("showfiling\n"));
	code = ShowFile(self, c->cur->fileName,
			(c->cur->next != (struct helpFile *)NULL),
			ahistory);
	DEBUG(("helpName: %s\n",c->name));
    }

    /* see if we should display "show more" menu item */
    if (c->cur && c->cur->next)
	c->flags |= MENU_SwitchMoreMenu; /* turn it on */
    else
	c->flags &= ~MENU_SwitchMoreMenu; /* turn it off */

    /* now set up the menus */
    SetupMenus(c);

    im::SetProcessCursor((class cursor *)NULL);
    
    if (code == 0) {		/* error */
	DEBUG(("ERROR\n"));
        sprintf(helpBuffer, (errmsg == NULL) ? err_generic : errmsg);
	ERRORBOX(c->view, helpBuffer);
    } else if(code == 2) /* ran command-alias */
	code = 0;
    else /* no error, advance to next file on list */
	c->cur = c->cur->next;
    DEBUG(("Help on returning %d\n",code));
    return code;
}


/*---------------------------------------------------------------------------*/
/*			HELP OBJECT ULTILITY FUNCTIONS			     */
/*---------------------------------------------------------------------------*/

/*
 * setup the menumask based on internal flags
 */
void 
SetupMenus(struct cache  *c)
{
    DEBUG(("IN setupmenus.."));
    (c->menus)->SetMask( c->flags);
    (c->view)->PostMenus( c->menus);
    DEBUG(("OUT setupmenus\n"));
}


/*
 * Do a search in the overview panel
 */
static void 
SearchOverviews(class help * self)
{
    if (!self->showPanels)
	TogglePanels(self, help_ALWAYS_TOGGLE);
    if (!self->showOverview)
	ToggleOverviews(self, help_ALWAYS_TOGGLE);
    (*help_textSearch) ((class textview *)self->overviewPanel, 0);
}


/*
 * Do a search in the programs panel
 */
static void 
SearchPrograms(class help * self)
{
    if (!self->showPanels)
	TogglePanels(self, help_ALWAYS_TOGGLE);
    DEBUG(("panels\n"));
    if (!self->showList)
	TogglePrograms(self, help_ALWAYS_TOGGLE);
    DEBUG(("search\n"));
    (*help_textSearch) ((class textview *)self->listPanel, 0);
}
    

/*
 * because textview's and frame's procs are bound at runtime, we must call them
 * like this, rather than just binding in a call to proctable_GetProc(...)
 */

static void 
TextviewProc(class help * self, long  rock)
{
    switch(rock) {
      case help_SEARCH:
	(*help_textSearch) ((class textview *)self->info->view, 0);
	break;
      case help_RSEARCH:
	(*help_textRevSearch) ((class textview *)self->info->view, 0);
	break;
      case help_SEARCH_AGAIN:
	(*help_textSearchAgain) ((class textview *)self->info->view, 0);
	break;
      case help_COPY_REGION:
	(*help_textCopyRegion) ((class textview *)self->info->view, 0);
	break;
      case help_PAGE_DOWN:
	(*help_textPageDown) ((class textview *)self->info->view, 0);
	break;
      case help_PAGE_UP:
	(*help_textPageUp) ((class textview *)self->info->view, 0);
	break;
      case help_SET_PRINTER:
#ifdef USE_PRINTOPTS
	if (help_poptPostWindow)
	    (*help_poptPostWindow) (self->info->view, 0);
	else
	    ERRORBOX(self->info->view, err_noproc);
#else
	if (help_frameSetPrinter)
	    (*help_frameSetPrinter) (self, 0);
	else
	    ERRORBOX(self->info->view, err_noproc);
#endif
    }
}

/*
 * quit help
 */
static void 
Quit(class help  *self)
{
    im::SetProcessCursor(help_waitCursor);
    im::KeyboardExit();
}

/*
 * send gripes/kudos to the help maintainers
 */
static void 
SendComments(class help  *self)
{
    char cmd[MAXPATHLEN];
    const char *prof;
    
    message::DisplayString(self, 0, msg_comment);
    im::ForceUpdate();
    im::SetProcessCursor(help_waitCursor);
    prof = environ::GetConfiguration(SETUP_GRIPE_ADDRESS);
    if (prof == NULL)
	prof = DEFAULT_GRIPE_ADDRESS;
    sprintf(cmd, "%s %s", COMMENTCMD, prof);
    mysystem(cmd);
    im::SetProcessCursor((class cursor *) NULL);
}

/*
 * show a tutorial, if it exists
 */
static void 
ShowTutorial(class help  *self)
{
    static char tbuffer[MAXPATHLEN];
    const char *tmp = NULL;

    if(tmp = FindEntryInDirs(help_tutorialDirs,self->info->name,TUTORIAL_EXT)) {
	strcpy(tbuffer,tmp);
	self->info->flags &= ~MENU_SwitchTutorialMenu; /* turn off menu item */
	SetupMenus(self->info);
    }
    ShowFile(self, tbuffer, help_USE_OLD_TITLE, help_HIST_TAIL);
}

/*
 * show changes doc
 */
static void 
ShowChanges(class help  *self)
{
    static char tbuffer[MAXPATHLEN];
    
    sprintf(tbuffer, "%s/%s%s", help_changesDir, self->info->name, CHANGE_EXT);
    self->info->flags &= ~MENU_SwitchChangesMenu; /* turn off menu item */
    SetupMenus(self->info);
    ShowFile(self, tbuffer, help_USE_OLD_TITLE, help_HIST_TAIL);
}

/*
 * show next file in list
 */
void NextHelp(class help  *self)
{
    if (!(self->info->flags & MENU_SwitchMoreMenu)) {
	return;
    }
    if(help_GetHelpOn(self, self->info->name, help_OLD, help_HIST_TAIL, err_no_more)) {
	(self->historyPanel)->ClearSelection();
    }
}

/*
 * get help on a clicked-on history item
 */
void 
HistoryHelp(class help  *self		/* callback rock */, struct history_entry  *ent	/* panelEntry rock */, class panel  *apanel		/* appropriate panel */)
{
    char buf[HNSIZE + HELP_MAX_ERR_LENGTH];
    char fnbuf[MAXPATHLEN];
    char *tmp = NULL;
    const char *dir = NULL;
    int code;
    int file = FALSE;

    DEBUG(("histhelp: %s\n", ent->fname));
    im::SetProcessCursor(help_waitCursor);
    sprintf(buf, err_unexpected, ent->fname);

    /* if it's a change or tutorial file, showfile it, since most of the time,
       the CHANGEDIR and TUTORIALDIR aren't indexed */
    tmp = strrchr(ent->fname, '.');
    if (tmp) {
	if (!strcmp(tmp, CHANGE_EXT)) {
	    file = TRUE;
	    dir = help_changesDir;
	}
	if (!strcmp(tmp, TUTORIAL_EXT)) {
	    file = TRUE;
	    *tmp = '\0';
	    dir = FindEntryInDirs( help_tutorialDirs, ent->fname , TUTORIAL_EXT );
	}
    }

    if (file) {
	strcpy(fnbuf, dir );
	strcat(fnbuf, "/");
	strcat(fnbuf, ent->fname);
	DEBUG(("history file: %s\n",fnbuf));
	code = ShowFile(self, fnbuf, FALSE, help_HIST_NOADD);
    } else
	code = help_GetHelpOn(self, ent->fname, help_NEW, help_HIST_NOADD, buf);

    if (code == 0) {
	DEBUG(("history help = 0\n"));
	/* error, so restore the old selection */
	if (self->info->lastHist != NULL) {
	    (self->historyPanel)->MakeSelection( self->info->lastHist);
	}
    } else {
	(self->overviewPanel)->ClearSelection();
	(self->listPanel)->ClearSelection();
	((class textview *)self->info->view)->SetDotPosition( ent->pos);
	((class textview *)self->info->view)->SetDotLength( ent->sellen);
	((class textview *)self->info->view)->SetTopPosition( ent->top);
    }
    im::SetProcessCursor((class cursor *)NULL);
    self->info->lastHist = (self->historyPanel)->CurrentHighlight();
}

/*
 * show overview or a help file from the program list panel
 */
void 
OverviewHelp(class help  *self, char  *name		/* which topic to request - panelEntry rock */, class panel  *apanel)
{
    char buf[HNSIZE + HELP_MAX_ERR_LENGTH];
    
    sprintf(buf, err_sorry, name);
    if (help_GetHelpOn(self, name, help_NEW, help_HIST_NAME, buf) != 0) {
	(self->historyPanel)->ClearSelection();
	/* clear the selection in the other panel */
	if (apanel == self->overviewPanel) {
	    (self->listPanel)->ClearSelection();
	} else {		/* it was self->listPanel */
	    (self->overviewPanel)->ClearSelection();
	}
    }
}


/*---------------------------------------------------------------------------*/
/*			     PANEL FUNCTIONS				     */
/*---------------------------------------------------------------------------*/

static boolean
EnsurePanelListSize()
{
    if(help_panelList || (help_panelList = (char**)calloc(help_panelListSize,sizeof(char*))))
	if(help_panelIndex >= (help_panelListSize-1)) {
	    help_panelListSize *= 2;
	    if(help_panelList = (char**) realloc(help_panelList,sizeof(char*) * help_panelListSize)) {
		int i;

		for(i = help_panelIndex; i < help_panelListSize; i++)
		    help_panelList[i] = NULL;
	    }
	}
    return(help_panelList ? TRUE : FALSE);
}

void
FreePanelListData()
{
    if(help_panelList && (help_panelIndex > 0)) {
	int i;

	for(i = 0; i <= help_panelIndex; i++)
/* Don't free the panelList strings because these strings are passed in as the client data (tag) for the panel object items and panel_FreeAllTags() will release these.*/
	    if(help_panelList[i]) help_panelList[i] = NULL;
	free(help_panelList);
	help_panelList = NULL;
	help_panelListSize = help_MAXPANEL;
	help_panelIndex = 0;
    }
}

/*
 * setup panels and default entries.
	This function had an ifdef to map it between cmu and non-cmu.
	But the non-cm,u functionality is needed for overviews, so
	I converted it to a runtime choice.   -- wjh

	readpairs:
		fname is the name of a file containing pair lines:
			panel-entry,help-key
		def is NULL, but could be an array of keys
	non-readpairs:
		fname is the name of a  directory of help files
		def is a list of extensions of files to be used in the panel
 
	returns the number of entries added to the panel
 */
long
SetupPanel(boolean  readpairs, const char  *fname, class panel  *panel		/* the panel to add entries to */, const char  * const *def)
				{
	const char * const *defptr;
	FILE *tfile;
	char elt1[HNSIZE], elt2[HNSIZE];
	long code;
	char *tmp;
	DIR *helpdir;
	struct dirent *ent;
	char tf[MAXPATHLEN];
	char *tfp;
	long entriesadded = 0;

	if (readpairs) {
		tfile = fopen(fname, "r");		/* try opening file */
		if (tfile) {
			while (1) {
				code = ScanLine(tfile, elt1, elt2);
				if (code != 2) break;
				if (elt1[0] != '#') {
					(panel)->Add( elt1, CopyString(elt2), FALSE);
					entriesadded++;
				}
			}
			fclose(tfile);
		}
		else {			/* if that don't woik, use da backup */
			defptr = def;
			if (defptr) 
			    while (*defptr != (char *) NULL) {
				(panel)->Add( *defptr, strdup(*defptr), FALSE);
				defptr++;
				entriesadded++;
			    }
		}
	}
	else if ((helpdir = opendir(fname)) == (DIR *)NULL)
		ERRORBOX(panel, err_openhelpdir);
	else {
	    FreePanelListData();
	    if(!EnsurePanelListSize()) {
		fprintf(stderr,"Couldn't allocate enough memory!\n");
		return(0);
	    }

	    strcpy(tf, fname); /* make a base for filenames dir/dir/ */
	    tfp = tf + strlen(tf);
	    *tfp++ = '/';	/* tfp points just after the last '/' */

	    while((ent = readdir(helpdir)) != (struct dirent *)NULL) {
		strcpy(tfp, ent->d_name); /* finish the filename */
		defptr = def;
		tmp = strrchr(ent->d_name, '.');
		if (tmp != NULL) {
		    if (defptr)
			while (*defptr != (char *) NULL) {
			    if (strcmp(tmp, *defptr++) == 0) {
				*tmp = '\0'; /* chop off extension */
				/* found a good candidate filename */
				AddToPanelList(ent->d_name);
				*tmp = '.';   /* replace period */
				entriesadded++;
			    }
			}
		}
	    }
	    closedir(helpdir);

	    /* now make the list */
	    SortAndMakePanel(panel);
	}
	return entriesadded;
}


/*
 * Setup the lpairs for the side panel(s)
 */
class view *SetupLpairs(class help  *self)
{
    long which = 0;

    (self->panelLpair1)->UnlinkTree();
    (self->panelLpair2)->UnlinkTree();

    if (self->showOverview) which |= help_SHOW_OVER;
    if (self->showList) which |= help_SHOW_LIST;
    if (self->showHistory) which |= help_SHOW_HIST;

    DEBUG(("lpair which: %d\n",which));

    switch (which) {
        case 0:
	    return (class view *)NULL;

	case help_SHOW_OVER:
	    return (class view *)self->overviewLpair;

	case help_SHOW_LIST:
	    return (class view *)self->listLpair;

	case help_SHOW_HIST:
	    return (class view *)self->historyLpair;

	case help_SHOW_OVER | help_SHOW_LIST:
	    (self->panelLpair1)->SetNth( 0, self->overviewLpair);
	    (self->panelLpair1)->SetNth( 1, self->listLpair);
	    self->panelLpair1->needsfull = 3;
	    return (class view *)self->panelLpair1;

	case help_SHOW_OVER | help_SHOW_HIST:
	    (self->panelLpair1)->SetNth( 0, self->overviewLpair);
	    (self->panelLpair1)->SetNth( 1, self->historyLpair);
	    self->panelLpair1->needsfull = 3;
	    return (class view *)self->panelLpair1;

	case help_SHOW_LIST | help_SHOW_HIST:
	    (self->panelLpair1)->SetNth( 0, self->listLpair);
	    (self->panelLpair1)->SetNth( 1, self->historyLpair);
	    self->panelLpair1->needsfull = 3;
	    return (class view *)self->panelLpair1;

	case help_SHOW_OVER | help_SHOW_LIST | help_SHOW_HIST:
	    (self->panelLpair1)->SetNth( 0, self->overviewLpair);
	    (self->panelLpair1)->SetNth( 1, self->listLpair);
	    self->panelLpair1->needsfull = 3;

	    (self->panelLpair2)->SetNth( 0, self->panelLpair1);
	    (self->panelLpair2)->SetNth( 1, self->historyLpair);
	    self->panelLpair2->needsfull = 2;
	    return (class view *)self->panelLpair2;
    }
    return NULL;
}


/*
 * turn side panels on and off
 */
static void 
TogglePanels(class help  *self, long  rock)
{
    if ((self->showPanels && (rock == help_SHOW_PANEL)) ||
	(!self->showPanels && (rock == help_HIDE_PANEL)))
	return;
    /* could fall through here if rock is ALWAYS_TOGGLE */
    self->showPanels = 1 - self->showPanels;    	/* toggle it */
    self->info->flags ^= (MENU_TogglePanelShow | MENU_TogglePanelHide); /* toggle menus */

    if (self->showPanels) {
	(self->info->scroll)->UnlinkTree();
	if((self->showOverview + self->showList + self->showHistory) == 0) {
	    self->showOverview = self->showList = self->showHistory = 1;
	    self->info->flags ^= (MENU_ToggleOverHide | MENU_ToggleOverShow |
				  MENU_ToggleListHide | MENU_ToggleListShow |
				  MENU_ToggleHistHide | MENU_ToggleHistShow);
	    (self->mainLpair)->SetNth( 1, SetupLpairs(self));
	}	    
	(self->mainLpair)->LinkTree( self);
    } else {
	if(self->showOverview) {
	    self->showOverview = 0;
	    self->info->flags ^= (MENU_ToggleOverHide | MENU_ToggleOverShow);
	}
	if(self->showList) {
	    self->showList = 0;
	    self->info->flags ^= (MENU_ToggleListHide | MENU_ToggleListShow);
	}
	if(self->showHistory) {
	    self->showHistory = 0;
	    self->info->flags ^= (MENU_ToggleHistHide | MENU_ToggleHistShow);
	}
	(self->mainLpair)->UnlinkTree();
	(self->info->scroll)->LinkTree( self);
    }
    SetupMenus(self->info);
    /* here we want to update everything */
    if(self->info && self->info->view) (self->info->view)->WantInputFocus( self->info->view);
    (self)->Update();
}


/*
 * toggle overview panel on and off
 */
static void 
ToggleOverviews(class help  *self, long  rock)
{
    class view *v;
    boolean doUpdate = FALSE;

    if ((self->showOverview && (rock == help_SHOW_OVER)) ||
	(!self->showOverview && (rock == help_HIDE_OVER)))
	return;
     /* could fall through here if rock is ALWAYS_TOGGLE */

    self->showOverview = 1 - self->showOverview; /* toggle */
    v = SetupLpairs(self);
    self->info->flags ^= (MENU_ToggleOverHide | MENU_ToggleOverShow);

    if (v) {
	if (!self->showPanels) {
	    self->showPanels = 1;
	    self->info->flags ^= (MENU_TogglePanelShow | MENU_TogglePanelHide);
	    (self->info->scroll)->UnlinkTree();
	    (self->mainLpair)->LinkTree( self);
	    doUpdate = TRUE;
	}
	self->mainLpair->needsfull = 2;  /* -- hack to get redraw to work */
	(self->mainLpair)->SetNth( 1, v);
	SetupMenus(self->info);
	if(doUpdate) (self)->Update();
	(self)->view::WantInputFocus( self->info->view);
    } else if (self->showPanels)
	TogglePanels(self, help_ALWAYS_TOGGLE); /* turn them off */
}


/*
 * toggle program list panel on and off
 */
static void 
TogglePrograms(class help  *self, long  rock)
{
    class view *v;
    boolean doUpdate = FALSE;

    if ((self->showList && (rock == help_SHOW_LIST)) ||
	(!self->showList && (rock == help_HIDE_LIST)))
	return;
     /* could fall through here if rock is ALWAYS_TOGGLE */

    self->showList = 1 - self->showList; /* toggle */
    v = SetupLpairs(self);
    self->info->flags ^= (MENU_ToggleListHide | MENU_ToggleListShow);

    if (v) {
	if (!self->showPanels) {
	    self->showPanels = 1;
	    self->info->flags ^= (MENU_TogglePanelShow | MENU_TogglePanelHide);
	    (self->info->scroll)->UnlinkTree();
	    (self->mainLpair)->LinkTree( self);
	    doUpdate = TRUE;
	}
	self->mainLpair->needsfull = 2;  /* -- hack to get redraw to work */
	(self->mainLpair)->SetNth( 1, v);
	SetupMenus(self->info);
	if(doUpdate) (self)->Update();
	(self)->view::WantInputFocus( self->info->view);
    } else if (self->showPanels)
	TogglePanels(self, help_ALWAYS_TOGGLE); /* turn them off */
}



/*
 * toggle history panel on and off
 */
static void 
ToggleHistory(class help  *self, long  rock)
{
    class view *v;
    boolean doUpdate = FALSE;

    if ((self->showHistory && (rock == help_SHOW_HIST)) ||
	(!self->showHistory && (rock == help_HIDE_HIST)))
	return;
     /* could fall through here if rock is ALWAYS_TOGGLE */

    self->showHistory = 1 - self->showHistory; /* toggle */
    v = SetupLpairs(self);
    self->info->flags ^= (MENU_ToggleHistHide | MENU_ToggleHistShow);

    if (v) {
	if (!self->showPanels) {
	    self->showPanels = 1;
	    self->info->flags ^= (MENU_TogglePanelShow | MENU_TogglePanelHide);
	    (self->info->scroll)->UnlinkTree();
	    (self->mainLpair)->LinkTree( self);
	    doUpdate = TRUE;
	}
	self->mainLpair->needsfull = 2;  /* -- hack to get redraw to work */
	(self->mainLpair)->SetNth( 1, v);
	SetupMenus(self->info);
	if(doUpdate) (self)->Update();
	(self)->view::WantInputFocus( self->info->view);
    } 
    else if (self->showPanels)
	TogglePanels(self, help_ALWAYS_TOGGLE); /* turn them off */
}


/*
 * Used to add all files in a directory to the expanded program list
 */
static void 
ExpanderAux(char  *dname)
{
    DIR *tmpdir;
    struct dirent *tde;
    char tf[MAXPATHLEN], *tfp;
    
    tmpdir = opendir(dname);
    
    if (tmpdir == (DIR *)NULL)
	return; /* don't use unopened directory */
    strcpy(tf, dname); /* make a base for filenames dir/dir/ */
    tfp = tf + strlen(tf);
    *tfp++ = '/';	/* tfp points just after the last '/' */
    
    while((tde=readdir(tmpdir)) != NULL) {
	if (*tde->d_name != '.') { /* no dot files */
	    strcpy(tfp, tde->d_name); /* finish the filename */
	    AddToPanelList(tde->d_name);
	}
    }
    closedir(tmpdir);
}


/*
 * comparison function for qsort
 */
static int 
panelCompare(char  **s1 , char  **s2)
{
    return (strcmp(*s1, *s2));
}


/*
 * terminates and then sorts the panelList
 */
static void 
SortAndMakePanel(class panel  *p)
{
    int i;

    DEBUG(("sort and make..."));
    DEBUG(("sort..."));
    if(help_panelIndex > 1) 
	qsort(help_panelList, help_panelIndex, sizeof(char *), QSORTFUNC(panelCompare));
    DEBUG(("removing..."));
    if(help_panelIndex > 0){	
	static char lastItem[100] = "";

	(p)->FreeAllTags();
	(p)->RemoveAll();
	DEBUG(("adding..."));
	for (i = 0; i < help_panelIndex; i++) {
	    if(help_panelList[i] && strcmp(lastItem,help_panelList[i])) {
		(p)->Add( help_panelList[i], help_panelList[i], FALSE);
		strcpy(lastItem, help_panelList[i]);
	    }
	}
	*lastItem = '\0';
    }
    DEBUG(("done\n"));
}


/*
 * just adds a string to the global panelList
 */
static char *
AddToPanelList(char  *s)
{
    if(EnsurePanelListSize()) {
	if(!(help_panelList[help_panelIndex] = (char*)malloc(strlen(s) + 1))) {
	    fprintf(stderr,"Couldn't allocate enough memory!\n");
	    return(NULL);
	}
	strcpy(help_panelList[help_panelIndex], s);
	help_panelIndex++;
	return help_panelList[(help_panelIndex - 1)];
    }
    else {
	fprintf(stderr,"Couldn't allocate enough memory!\n");
	return(NULL);
    }
}    


/*
 * Index library callback helper for 'expand the program list'
 */
static void 
Expander(struct Index  *aindex, struct indexComponent  *ac, class help  *self)
{
    if(ac && ac->name && (*(ac->name) != '\0')) {
	AddToPanelList(ac->name);
    }
}


/*
 * toggle programs list size.  Like da name dun say.
 */
void 
ToggleProgramListSize(class help * self, long  rock)
{
    struct helpDir *thd;

    if ((self->expandedList && (rock == help_EXPAND)) ||
	(!self->expandedList && (rock == help_SHRINK)))
	return;
    /* could fall through here if rock is ALWAYS_TOGGLE */
    if (!self->showList) TogglePrograms(self, help_ALWAYS_TOGGLE);

    self->expandedList = 1 - self->expandedList; /* toggle */
    self->info->flags ^= (MENU_ToggleSizeExpand | MENU_ToggleSizeShrink); /* toggle menus */

    if (self->oldpanel)	/* using tmp, restore original */
	RestorePanel(self);

    if (!self->expandedList) { /* shrink it down */
	char pathName[MAXPATHLEN];
	const char *tmp;

	DEBUG(("shrinking "));
	
	self->expandedList = 0;

	(self->listPanel)->FreeAllTags();
	(self->listPanel)->RemoveAll(); /* this frees all the strings in panelLits */
	
	/* add only the small list of entries to listPanel */
	tmp = environ::GetConfiguration(SETUP_PANELSDIR);
	if (tmp == NULL)
	    tmp = environ::AndrewDir(DEFAULT_PANELSDIR);
	sprintf(pathName, "%s%s", tmp, PROGRAMFILE);

	if (0 == SetupPanel(TRUE, pathName, self->listPanel, NULL)) {
		/* we got nothing from the lib/help.programs file.  Try using extensions. */
		tmp = environ::GetConfiguration(SETUP_HELPDIR);
		if (tmp == NULL)
			tmp = environ::AndrewDir(DEFAULT_HELPDIR);
		strcpy(pathName, tmp);
	
		SetupPanel(FALSE, pathName, self->listPanel, program_ext_array);
	}

    } else {			/* pump it up */

	DEBUG(("expanding "));

	FreePanelListData();
	if(EnsurePanelListSize() == FALSE) {
	    fprintf(stderr,"Couldn't allocate enough memory!\n");
	    return;
	}

	if (!helpdb::CheckIndex(self))
	    return;
	message::DisplayString(self, 0, msg_expanding);
	im::ForceUpdate();
	im::SetProcessCursor(help_waitCursor);
	self->expandedList = 1;

	/* get everything in the index */
	helpdb::Enumerate((index_efptr)Expander, (char *)self);

	/* now add in the auxiliary help dirs */
	for(thd = (struct helpDir *)helpdb::GetHelpDirs(); thd; thd = thd->next) {
	    char *tmp;
	    const char *subdir = MANSUBS;
	    char dir[MAXPATHLEN];
	    
	    tmp = strrchr(thd->dirName, '/');
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
		    ExpanderAux(dir);
		}
	    } else
		ExpanderAux(dir);
	}

	/* now make the list */
	SortAndMakePanel(self->listPanel);

	im::SetProcessCursor((class cursor *) NULL);
    }
    (self->listLab)->SetText( (char *)((self->expandedList) ?
		  "Full Program List" : "Programs"));
    SetupMenus(self->info);
    message::DisplayString(self, 0, "Done.");
}


/*
 * restores the original program panel (unfiltered)
 */
static void 
RestorePanel(class help  *self)
{
    if (!self->oldpanel) {
	DEBUG(("already have old panel\n"));
	return;
    }

    self->info->flags ^= (MENU_ToggleRestoreShow | (MENU_ToggleFilterShow | MENU_ToggleReFilterShow));

    if (!self->showPanels)
	TogglePanels(self, help_ALWAYS_TOGGLE);

    (self->tmpanel)->FreeAllTags();
    (self->tmpanel)->RemoveAll();
    (self->listScroll)->SetView( self->oldpanel);
    self->oldpanel = (class panel *)NULL;

    (self->listLab)->SetText( (char *)((self->expandedList) ?
		  "Full Program List" : "Programs"));
    SetupMenus(self->info);
}


/*
 * replaces the program panel with a new panel that contains only those
 * entries from the old panel specified by the user
 */
static void 
FilterPanel(class help  *self, long  rock)
{
    int code;
    char buf[255];
    char lbuf[255];
    struct panel_Entry *pe, *list;
    regexp *pattern;    /* used to store compiled version of expression */

    /* if rock == sort && self->oldpanel, punt */
    /* if rock == resort && self->oldpanel, OK */
    if (self->oldpanel && (rock == help_FILTER_FILTER)) { /* it's already being used */
	DEBUG(("already using old panel\n"));
	return;
    }

    if (!self->showPanels)
	TogglePanels(self, help_ALWAYS_TOGGLE);

    code = message::AskForString(self, 0, msg_filter_prompt,
				0, buf, sizeof(buf)-1);
    if ((code < 0) || (buf[0] == '\0'))
	return;

    FreePanelListData();

    if (rock == help_FILTER_FILTER)
	list = self->listPanel->panelList;
    else
	list = self->tmpanel->panelList;
    
    pattern = reg_comp(buf); /* compile the pattern and save the results */
    if (pattern == NULL) {
	ERRORBOX(self->info->view, "Sorry; filter pattern was not understood");
	return;
    } else {
	for (pe = list; pe != NULL; pe = pe->next) {
	    if (reg_exec(pattern, pe->tag) != 0)
		AddToPanelList(pe->tag);
	}
	DEBUG(("done\n"));
	free(pattern);	/* return the space */
    }
    if(help_panelIndex == 0){
	char bbuf[512];
	sprintf(bbuf,"Panel does not contain \"%s\"",buf);
	ERRORBOX(self->info->view,bbuf);
	return;
    }

    if (rock == help_FILTER_FILTER) { /* only the first time */
	self->oldpanel = self->listPanel;

	self->info->flags ^= (MENU_ToggleRestoreShow | (MENU_ToggleFilterShow | MENU_ToggleReFilterShow));
	SetupMenus(self->info);
    }

    im::SetProcessCursor(help_waitCursor);
    im::ForceUpdate();

    sprintf(lbuf, "%s '%s'", "filtered by ", buf);
    (self->listLab)->SetText( lbuf);

    /* now make the list */
    SortAndMakePanel(self->tmpanel);
    
    (self->listScroll)->SetView( self->tmpanel);
    DEBUG(("set data\n"));

    im::SetProcessCursor((class cursor *)NULL);
}

/*
 * random key hit proc to chastise the user
 */
static void 
nono(class help  *self)
{
    message::DisplayString(self, 0, err_readonly);
}


static struct bind_Description helpBindings[] = {

/*
    {"proc-name", keybinding, keyrock,
    "menu string", menurock, menuflag, function-to-call,
    "documentation string", module-name}
*/

    /* Toggling menu items */

    /* if self->showPanels */
    {"help-toggle-panels", NULL, help_HIDE_PANEL,
    "Panels~10,Hide Panels~2", help_HIDE_PANEL, MENU_TogglePanelHide,(proctable_fptr) TogglePanels,
    NULL, NULL},

    {"help-toggle-panels", NULL, help_SHOW_PANEL,
    "Panels~10,Show Panels~2", help_SHOW_PANEL, MENU_TogglePanelShow,(proctable_fptr) TogglePanels,
    NULL, NULL},

    {"help-toggle-panels", HELP_KEY_TOGGLE_PANELS, help_ALWAYS_TOGGLE,
    NULL, help_ALWAYS_TOGGLE, MENU_Never,(proctable_fptr) TogglePanels,
    "toggle help panels", NULL},


    /* if self->showOverview */
    {"help-toggle-overviews", NULL, help_HIDE_OVER,
    "Panels~10,Hide Overviews~20", help_HIDE_OVER, MENU_ToggleOverHide,(proctable_fptr) ToggleOverviews,
    NULL, NULL},

    {"help-toggle-overviews", NULL, help_SHOW_OVER,
    "Panels~10,Show Overviews~20", help_SHOW_OVER, MENU_ToggleOverShow,(proctable_fptr) ToggleOverviews,
    NULL, NULL},

    {"help-toggle-overviews", HELP_KEY_TOGGLE_OVER, help_ALWAYS_TOGGLE,
    NULL, help_ALWAYS_TOGGLE, MENU_Never,(proctable_fptr) ToggleOverviews,
    "toggle overviews panel", NULL},


    /* if self->showList */
    {"help-toggle-programs", NULL, help_HIDE_LIST,
    "Panels~10,Hide Programs~22", help_HIDE_LIST, MENU_ToggleListHide,(proctable_fptr) TogglePrograms,
    NULL, NULL},

    {"help-toggle-programs", NULL, help_SHOW_LIST,
    "Panels~10,Show Programs~22", help_SHOW_LIST, MENU_ToggleListShow,(proctable_fptr) TogglePrograms,
    NULL, NULL},

    {"help-toggle-programs", HELP_KEY_TOGGLE_PROGRAMS, help_ALWAYS_TOGGLE,
    NULL, help_ALWAYS_TOGGLE, MENU_Never,(proctable_fptr) TogglePrograms,
    "toggle programs panel", NULL},


    /* if self->showHistory */
    {"help-toggle-history", NULL, help_HIDE_HIST,
    "Panels~10,Hide History~24", help_HIDE_HIST, MENU_ToggleHistHide,(proctable_fptr) ToggleHistory,
    NULL, NULL},

    {"help-toggle-history", NULL, help_SHOW_HIST,
    "Panels~10,Show History~24", help_SHOW_HIST, MENU_ToggleHistShow,(proctable_fptr) ToggleHistory,
    NULL, NULL},

    {"help-toggle-history", HELP_KEY_TOGGLE_HIST, help_ALWAYS_TOGGLE,
    NULL, help_ALWAYS_TOGGLE, MENU_Never,(proctable_fptr) ToggleHistory,
    "toggle history panel", NULL},


    /* if self->expandedList */
    {"help-toggle-size", NULL, help_SHRINK,
    "Panels~10,Shrink Programs List~10", help_SHRINK, MENU_ToggleSizeShrink,(proctable_fptr)
    ToggleProgramListSize, NULL, NULL},

    {"help-toggle-size", NULL, help_EXPAND,
    "Panels~10,Expand Programs List~10", help_EXPAND, MENU_ToggleSizeExpand,(proctable_fptr)
    ToggleProgramListSize, NULL, NULL},

    {"help-toggle-size", HELP_KEY_TOGGLE_PROGRAMS, help_ALWAYS_TOGGLE,
    NULL, help_ALWAYS_TOGGLE, MENU_Never,(proctable_fptr) ToggleProgramListSize,
    "toggle program list size", NULL},


    {"help-filter-programs", NULL, help_FILTER_FILTER,
    "Other~10,Filter Programs Panel~10", help_FILTER_FILTER, MENU_ToggleFilterShow,(proctable_fptr) FilterPanel,
    "filter-programs", NULL},

    {"help-refilter-programs", NULL, help_FILTER_REFILTER,
    "Other~10,ReFilter Programs Panel~12", help_FILTER_REFILTER, MENU_ToggleReFilterShow,(proctable_fptr) FilterPanel,
    "filter-programs", NULL},

    {"help-restore-programs", NULL, help_FILTER_RESTORE,
    "Other~10,Restore Programs Panel~10", help_FILTER_RESTORE, MENU_ToggleRestoreShow,(proctable_fptr) RestorePanel,
    "restore programs panel", NULL},


    {"help-show-more", HELP_KEY_SHOW_MORE, 0,
    "Show More Documentation~30", 0, MENU_SwitchMoreMenu,(proctable_fptr) NextHelp,
    "show more documentation", NULL},

    {"help-show-tutorial", HELP_KEY_SHOW_TUTORIAL, 0,
    "Show Tutorial~32", 0, MENU_SwitchTutorialMenu,(proctable_fptr) ShowTutorial,
    "show tutorial", NULL},

    {"help-changes", HELP_KEY_SHOW_CHANGES, 0,
    "Show Changes~34", 0, MENU_SwitchChangesMenu,(proctable_fptr) ShowChanges,
    "show changes on topic", NULL},
    
    {"help-copy-region", HELP_KEY_COPY_REGION, help_COPY_REGION,
    "Copy~2", help_COPY_REGION, MENU_SwitchCopy,(proctable_fptr) TextviewProc,
    "Copy region to kill-buffer", NULL},

    
    /* non-toggling menu items */


    {"help-delete-window", HELP_KEY_DELETE_WINDOW, 0,
    "Delete Window~80",0 ,MENU_SwitchDeleteMenu,(proctable_fptr) help_aux_ExitProc,
    "delete this window", NULL},

    {"help-add-search-dir", HELP_KEY_ADD_SEARCH_DIR, 0,
    "Other~10,Add Search Directory~20", 0, MENU_Always,(proctable_fptr) help_aux_AddSearchDir,
    "add a directory to the search path", NULL},

    {"help-add-bookmark", HELP_KEY_ADD_BOOKMARK, 0,
    "Other~10,Make Bookmark~30", 0, MENU_Always,(proctable_fptr) help_aux_AddBookmark,
    "add a history entry for this file", NULL},

    {"help-show-help-on", HELP_KEY_HELP_ON, help_ON,
    "Show Help on ...~14", help_ON, MENU_Always,(proctable_fptr) help_aux_NewHelp,
    "show help on a prompted keyword", NULL},

    {"help-show-help-on", HELP_KEY_HELP_ON_2, help_ON,
    NULL, help_ON, MENU_Always,(proctable_fptr) help_aux_NewHelp,
    NULL, NULL},

    {"help-new-show-help-on", HELP_KEY_WINDOW_SPLIT, help_ON | help_NEW_WIN,
    "Window~12,Show Help on...~14", help_ON | help_NEW_WIN, MENU_Always,(proctable_fptr) help_aux_NewHelp,
    "show help on a prompted keyword in a new window", NULL},

    {"help-show-help-on-selected", HELP_KEY_HELP_ON_SELECTED, help_SEL,
    "Show Help on Selected Word~16", help_SEL, MENU_Always,(proctable_fptr) help_aux_NewHelp,
    "show help on selected region", NULL},

    {"help-new-show-help-on-selected", NULL, help_SEL | help_NEW_WIN,
    "Window~12,Show Help on Selected Word~26", help_SEL | help_NEW_WIN, MENU_Always,(proctable_fptr) help_aux_NewHelp,
    "show help on selected region in a new window", NULL},

    {"help-comments", HELP_KEY_SEND_COMMENT, 0,
    "Send Comment On Help~31", 0, MENU_Always,(proctable_fptr) SendComments,
    "send comments on help", NULL},

#ifdef USE_PRINTOPTS
/* If we're building Contrib, use PrinterSetup instead of SetPrinter */
   {"help-printer-setup", HELP_KEY_SET_PRINTER, help_SET_PRINTER,
    "Print~6,Printer Setup~12", help_SET_PRINTER, MENU_Always, (proctable_fptr) TextviewProc,
    "Pop up dialogue box to set printer name and other parameters.", "printopts"},
#else /* USE_PRINTOPTS */
   {"help-set-printer", HELP_KEY_SET_PRINTER, help_SET_PRINTER,
     "Print~6,Set Printer~12", help_SET_PRINTER, MENU_Always,(proctable_fptr) TextviewProc,
    "set the printer", NULL},
#endif /* USE_PRINTOPTS */

    {"help-print", HELP_KEY_PRINT, 0,
    "Print~6,Print This File~22", 0, MENU_Always,(proctable_fptr) help_aux_Print,
    "print current help file", NULL},

    {"help-quit", HELP_KEY_QUIT, 0,
    "Quit~99", 0, MENU_Always,(proctable_fptr) Quit,
    "quit help", NULL},

    /* imported from textview... */

    {"help-search", HELP_KEY_SEARCH, help_SEARCH,
    "Search~4,Forward~12", help_SEARCH, MENU_Always,(proctable_fptr) TextviewProc,
    "forward search", NULL},

    {"help-reverse-search", HELP_KEY_RSEARCH, help_RSEARCH,
    "Search~4,Backward~14", help_RSEARCH, MENU_Always,(proctable_fptr) TextviewProc,
    "reverse search", NULL},
    
    {"help-search-again", HELP_KEY_SEARCH_AGAIN, help_SEARCH_AGAIN,
    "Search~4,Search Again~16", help_SEARCH_AGAIN, MENU_Always,(proctable_fptr) TextviewProc,
    "search again", NULL},

    {"help-search-overviews", HELP_KEY_SEARCH_OVERVIEWS, 0,
    "Search~4,Search Overviews~20", 0, MENU_Always,(proctable_fptr) SearchOverviews,
    "search overviews", NULL},

    {"help-search-programs", HELP_KEY_SEARCH_PROGRAMS, 0,
    "Search~4,Search Programs~22", 0, MENU_Always,(proctable_fptr) SearchPrograms,
    "search programs", NULL},
    
    {"help-next-screen", HELP_KEY_NEXT_SCREEN, help_PAGE_DOWN,
    NULL, help_PAGE_DOWN, 0,(proctable_fptr) TextviewProc,
    "advance to next screen", NULL},

    {"help-next-screen", HELP_KEY_NEXT_SCREEN2, help_PAGE_DOWN,
    NULL, help_PAGE_DOWN, 0,(proctable_fptr) TextviewProc,
    "advance to next screen", NULL},

    {"help-prev-screen", HELP_KEY_PREV_SCREEN, help_PAGE_UP,
    NULL, help_PAGE_UP, 0,(proctable_fptr) TextviewProc,
    "go back to previous screen", NULL},

    NULL};

/*
 * makes new instance of a help object.  Creates keylist and menus, binds procedures
 * to menus and keys, and adds a default filetype so that all files will have
 * at least the default template when they are displayed.
 */
    boolean help_class_inited=FALSE;
boolean 
help::InitializeClass()
{
    unsigned char c[2];
    struct proctable_Entry *pe;

    help_class_inited=TRUE;
#ifdef DEBUGGING
    if ((char *)getenv("HELPDEBUG") != (char *) NULL)
	HELPDEBUG = 1;
#endif /* DEBUGGING */
    
    DEBUG(("IN init class\n"));
    Help_Menus = new menulist;
    Help_Map = new keymap;
    DEBUG(("created Help_Menus, Help_Map\n"));

    if (!Help_Menus || !Help_Map)
	return FALSE;

    /* make all printing keys put a readonly message in the message line */
    c[1] = '\0';
    pe = proctable::DefineProc("help-readonly-key", (proctable_fptr)nono, &help_ATKregistry_ , 0, "fake readonliness");
    for (c[0] = (unsigned char)0; c[0] <= (unsigned char)127; c[0]++)
	if (isprint(c[0]) || c[0] == '\n' || c[0] == ' ')
	    (Help_Map)->BindToKey( (char *)c, pe, 0);
    DEBUG(("about to bind helpBindings\n"));
    bind::BindList(helpBindings, Help_Map, Help_Menus, &help_ATKregistry_ );

    DEBUG(("about to create catchall filetype\n"));
    /* what this is really doing is adding a catchall filetype and template */
    filetype::AddEntry("*", "text", "template=default");

    DEBUG(("about to load helpdb\n"));
    ATK::LoadClass("helpdb");
    DEBUG(("about to load framecmds\n"));
    ATK::LoadClass("framecmds");
#ifdef USE_PRINTOPTS
    DEBUG(("about to load printopts\n"));
    ATK::LoadClass("printopts");
#endif /* USE_PRINTOPTS */

    DEBUG(("OUT init class\n"));
    return TRUE;
}

