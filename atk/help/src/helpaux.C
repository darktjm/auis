/*********************************************************************** *\
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

#define label gezornenplatz
/* sys/types.h in AIX PS2 defines "struct label", causing a type name clash.
   Avoid this by temporarily redefining "label" to be something else in the preprocessor. */
#include <andrewos.h> /* sys/types.h sys/file.h */
#undef label

/* BSD/RT is overflowing the cpp define table. */
#define dontDefineRoutinesFor_keymap

#include <completion.H>
#include <cursor.H>
#include <dataobject.H>
#include <environ.H>
#include <im.H>
#include <frame.H>
#include <keymap.H>
#include <keystate.H>
#include <label.H>
#include <labelview.H>
#include <lpair.H>
#include <menulist.H>
#include <message.H>
#include <print.H>
#include <panel.H>
#include <proctable.H>
#include <rect.h>
#include <scroll.H>
#include <style.H>
#include <text.H>
#include <textview.H>
#include <view.H>

#include <ctype.h>
#include <sys/param.h>

#include <config.h>
#include <helpsys.h>
#include <help.h>
#include <helpdb.H>
#include <hlptextview.H>
#define AUXMODULE 1
#include <help.H>

#include <helpaux.h>

/* statics representing information cache */
extern char *help_tutorialDirs[MAX_TUTORIAL_DIRS];
extern char help_changesDir[MAXPATHLEN];

#define USE_PRINTOPTS /* used to be in contrib, but now we should always use it. */

/* a list of instances of help */
extern struct self_help *help_ego;

extern class keymap *Help_Map;
extern class menulist *Help_Menus;

extern class cursor *help_waitCursor; /* the watch cursor */

extern char **help_panelList; /* used for enumerating across the help index */

/* hooks to textview and frame procs */
extern proctable_fptr help_textSearch;
extern proctable_fptr help_textRevSearch;
extern proctable_fptr help_textSearchAgain;
extern proctable_fptr help_textCopyRegion;
extern proctable_fptr help_textPageDown;
extern proctable_fptr help_textPageUp;
extern proctable_fptr help_frameSetPrinter;
#ifdef USE_PRINTOPTS
extern proctable_fptr help_poptPostWindow;
#endif


#ifdef DEBUGGING
/*
 * debugging statements can be included by compiling add modules with
 * -DDEBUGGING.  Debugging output is toggled by existance/nonexistance
 * of the environment variable HELPDEBUG.
 */
extern int HELPDEBUG;
#undef DEBUG
#define DEBUG(arg) if (HELPDEBUG != 0) { printf arg; fflush(stdout); }
#else
#undef DEBUG
#define DEBUG(arg)
#endif /* DEBUGGING */

/*
 * We put this function in helpaux.c because help.c is already huge.
 * The preprocessor explodes on an RT when <hlptextv.ih> is included in
 * help.c.
 */

void init_hlptextview(class hlptextview  *hv);
static int safesystem(char  *acmd);
static class frame * getframe(class view  *vw);
static void  destroyWindow(class help  *self);
void help_aux_ExitProc(class help  *self);
void help_aux_Print(class help  *self);
static void ShowHelp(class help  *self, char  *topic, boolean  new_win);
static void CompletionSplot(char  *name , char  *original, struct helpdb_completesplot  *rock);
static enum message_CompletionCode HelpCompletionProc(char  *string, class help  *self, char  *buffer, int  buffersize);
static int lenstrcmp(char *s1 , char *s2);
static void HelpEnumProc(char  *name , char  *original, struct help_helpsplot  *rock);
static void HelpHelpProc(char  *partialKeyword, class help  *rock, message_workfptr HelpWork, char  *hrock);
void help_aux_NewHelp(class help  *self, long  type		/* help_ON 			if Help On... */);
void help_aux_AddBookmark(class help  *self);
void help_aux_AddSearchDir(class help  *self);


void init_hlptextview(class hlptextview  *hv)
{
    (hv)->SetHyperlinkCheck( TRUE);
}

/*---------------------------------------------------------------------------*/
/*			     HELP CLASSPROCEDURES			     */
/*---------------------------------------------------------------------------*/

/*
 * help__ method for adding a directory to the searchpath
 */
void help::AddSearchDir(const char  *dirName)
{
	ATKinit;

    if (access(dirName, 4) < 0) {
	fprintf(stderr, "help: cannot open search directory '%s'\n", dirName);
	return;
    }
    DEBUG(("adding dir: '%s'\n",dirName));
    helpdb::AddSearchDir(dirName);
}


/*
 * initializes a new help object.  Sets up the view hiearchy, sets up panels, etc
 */
help::help()
{
	ATKinit;

    char pathName[MAXPATHLEN];
    const char *tmp = NULL, *colon = NULL;
    struct proctable_Entry *pe;
    struct self_help *id;
    class view *v;
    int	 i = 0;


    DEBUG(("IN init obj\n"));
    help_waitCursor = cursor::Create(0);
    (help_waitCursor)->SetStandard( Cursor_Wait);
    im::SetProcessCursor(help_waitCursor);

    tmp = environ::GetConfiguration(SETUP_CHANGEDIR);
    if (tmp == NULL) {
	tmp = environ::LocalDir(DEFAULT_CHANGEDIR);
    }
    strcpy(help_changesDir, tmp);

    for(i = 0; i < MAX_TUTORIAL_DIRS; i++) 
	help_tutorialDirs[i] = NULL;
    i = 0;
    if(tmp = environ::GetConfiguration(SETUP_TUTORIALDIR)) {
	if((colon = strrchr(tmp,':')) == NULL) {
	    help_tutorialDirs[i] = (char*) malloc(strlen(tmp) + 1);
	    strcpy(help_tutorialDirs[i],tmp);
	}
	else {
	    while(colon && (colon != '\0')) {
		help_tutorialDirs[i] = (char*) malloc(strlen(tmp) + 1);
		memcpy(help_tutorialDirs[i],tmp,(int)(colon - tmp));
		help_tutorialDirs[i][(int)(colon - tmp)] = 0;
		tmp = colon + 1;
		colon = strrchr(tmp,':');
		i++;
	    }
	    help_tutorialDirs[i] = (char*) malloc(strlen(tmp) + 1);
	    strcpy(help_tutorialDirs[i],tmp);
	}
    }
    else {
	tmp = environ::AndrewDir(DEFAULT_TUTORIALDIR);
	help_tutorialDirs[i] = (char*) malloc(strlen(tmp) + 1);
	strcpy(help_tutorialDirs[i],tmp);
    }

    im::SetProcessCursor((class cursor *) NULL);

    /* general variables */
    this->expandedList = 0;
    this->showPanels = environ::GetProfileSwitch("ShowPanels", TRUE);
    this->showOverview = environ::GetProfileSwitch("ShowOverviews", TRUE);
    this->showList = environ::GetProfileSwitch("ShowPrograms", TRUE);
    this->showHistory = environ::GetProfileSwitch("ShowHistory", FALSE);

    this->state = keystate::Create(this, Help_Map);
    this->info = (struct cache *)malloc(sizeof(struct cache));
    if (!this->info || !this->state)
	THROWONFAILURE( FALSE);
    DEBUG(("info state "));
    this->info->scroll = NULL;
    this->info->menus = (Help_Menus)->DuplicateML( this);
    if (!this->info->menus)
	THROWONFAILURE( FALSE);
    DEBUG(("menus "));
    this->info->flags = 0;
    this->info->flags = MENU_ToggleSizeExpand; /* start out shrunk initially */
    this->info->flags |= (this->showPanels) ? MENU_TogglePanelHide : MENU_TogglePanelShow;
    this->info->flags |= (this->showOverview) ? MENU_ToggleOverHide : MENU_ToggleOverShow;
    this->info->flags |= (this->showList) ? MENU_ToggleListHide : MENU_ToggleListShow;
    this->info->flags |= (this->showHistory) ? MENU_ToggleHistHide : MENU_ToggleHistShow;
    this->info->flags |= MENU_ToggleFilterShow;
    this->info->all = this->info->cur = (struct helpFile *)NULL;
    this->info->histat = help_HIST_NOADD;
    this->info->histent[0] = '\0';
    this->info->lastHist = NULL;

    this->mainLpair = NULL;

    /* the help text object */
    this->info->data = (class dataobject *)new text;
    this->info->view = (class view *)new textview;
    if (!this->info->data || !this->info->view)
	THROWONFAILURE( FALSE);
    DEBUG(("data view "));

    (this->info->view)->SetDataObject( this->info->data);
    
    this->info->scroll = scroll::Create(this->info->view, scroll_LEFT);
    ((class textview *) this->info->view)->SetEmbeddedBorder( 20, 5);
    if (!this->info->scroll)
	THROWONFAILURE( FALSE);
    DEBUG(("scroll "));
    
    /* panels and their scrollbars */
    this->oldpanel = (class panel *)NULL;
    this->tmpanel = new panel;
    this->overviewPanel = new panel;
    this->listPanel = new panel;
    this->historyPanel = new panel;
    if (!this->tmpanel || !this->overviewPanel || !this->listPanel || !this->historyPanel)
	THROWONFAILURE( FALSE);
    DEBUG(("panels "));
    this->info->hist = this->historyPanel;

    this->overviewScroll = (class scroll *) (this->overviewPanel)->GetApplicationLayer();
    this->listScroll = (class scroll *) (this->listPanel)->GetApplicationLayer();
    this->historyScroll = (class scroll *) (this->historyPanel)->GetApplicationLayer();
    if (!this->overviewScroll || !this->listScroll)
	THROWONFAILURE( FALSE);


    /* labels */
    this->overviewLab = new label;
    this->overviewLabV = new labelview;
    this->listLab = new label;
    this->listLabV = new labelview;
    this->historyLab = new label;
    this->historyLabV = new labelview;
    if (!this->overviewLab || !this->overviewLabV || !this->historyLab || !this->historyLabV
	|| !this->listLab || !this->listLabV)
	THROWONFAILURE( FALSE);

    (this->overviewLabV)->SetDataObject( this->overviewLab);
    (this->overviewLab)->SetFlags( label_CENTERED | label_BOXED);
    (this->overviewLab)->SetText( "Overviews");

    (this->listLabV)->SetDataObject( this->listLab);
    (this->listLab)->SetFlags( label_CENTERED | label_BOXED);
    (this->listLab)->SetText( "Programs");
    
    (this->historyLabV)->SetDataObject( this->historyLab);
    (this->historyLab)->SetFlags( label_CENTERED | label_BOXED);
    (this->historyLab)->SetText( "Help History");


    /* lpairs */
    this->overviewLpair = lpair::Create(this->overviewLabV, this->overviewScroll, LABPCT);
    this->listLpair = lpair::Create(this->listLabV, this->listScroll, LABPCT);
    this->historyLpair = lpair::Create(this->historyLabV, this->historyScroll, LABPCT);
    this->panelLpair1 = lpair::Create(NULL, NULL, -PANELPCT);
    this->panelLpair2 = lpair::Create(NULL, NULL, -33);
    if (!this->overviewLpair || !this->listLpair || !this->historyLpair
	|| !this->panelLpair1 || !this->panelLpair2)
	THROWONFAILURE( FALSE);
    
    (this->overviewLpair)->VTFixed( this->overviewLabV, this->overviewScroll, LABPCT, 0);
    (this->listLpair)->VTFixed( this->listLabV, this->listScroll, LABPCT, 0);
    (this->historyLpair)->VTFixed( this->historyLabV, this->historyScroll, LABPCT, 0);

    /* if the user has all panels off, use the overview panel by default */
    v = SetupLpairs(this);
    this->mainLpair = lpair::Create(this->info->scroll,
				   v ? v : (class view *)this->overviewScroll, -MAINPCT);
    if (!this->mainLpair)
	THROWONFAILURE( FALSE);
    DEBUG(("main lpair "));
    
    (this->mainLpair)->SetLPState( lpair_NOCHANGE, lpair_VERTICAL, 1);

    DEBUG(("handlers "));

    /* default panel handlers */
    (this->overviewPanel)->SetHandler((panel_hfptr)OverviewHelp, (char *) this);
    (this->listPanel)->SetHandler((panel_hfptr) OverviewHelp, (char *) this);
    (this->tmpanel)->SetHandler((panel_hfptr) OverviewHelp, (char *) this);
    (this->historyPanel)->SetHandler((panel_hfptr) HistoryHelp, (char *) this);

    if (this->showPanels)
	(this->mainLpair)->LinkTree( this);
    else
	(this->info->scroll)->LinkTree( this);

    /* add minimum set of entries to listPanel */
    if((tmp = environ::GetConfiguration(SETUP_PANELSDIR)) == NULL) {
	tmp = environ::GetConfiguration(SETUP_LIBDIR);
	if (tmp == NULL)
	    tmp = environ::AndrewDir(DEFAULT_LIBDIR);
    }
    sprintf(pathName, "%s%s", tmp, PROGRAMFILE);
    
    if (0 == SetupPanel(TRUE, pathName, this->listPanel, NULL)) {
	/* got nothing from lib/help.programs;  use extensions */
	tmp = environ::GetConfiguration(SETUP_HELPDIR);
	if (tmp == NULL)
		tmp = environ::AndrewDir(DEFAULT_HELPDIR);
	strcpy(pathName, tmp);
	help::AddSearchDir(pathName);	/* ??? XXX */
   
	SetupPanel(FALSE, pathName, this->listPanel, program_ext_array);
    }

    DEBUG(("expanding "));
    if (environ::GetProfileSwitch("ExpandedList", FALSE))
	ToggleProgramListSize(this, help_EXPAND);

    /* get overviews entries */
    if((tmp = environ::GetConfiguration(SETUP_PANELSDIR)) == NULL) {
	tmp = environ::GetConfiguration(SETUP_LIBDIR);
	if (tmp == NULL)
	    tmp = environ::AndrewDir(DEFAULT_LIBDIR);
    }
    sprintf(pathName, "%s%s", tmp, OVERVIEWFILE);
    if (0 == SetupPanel(TRUE, pathName, this->overviewPanel, NULL)) {
	tmp = environ::GetConfiguration(SETUP_HELPDIR);
	if (tmp == NULL)
		tmp = environ::AndrewDir(DEFAULT_HELPDIR);
	strcpy(pathName, tmp);
	help::AddSearchDir(pathName);	/* ??? XXX */

	SetupPanel(FALSE, pathName, this->overviewPanel, overview_ext_array);
    }

    /* post our menus */
    SetupMenus(this->info);

    /* runtime-bound procedures */
    pe = proctable::Lookup("textview-search");
    if (pe) help_textSearch =  proctable::GetFunction(pe);
    else THROWONFAILURE(FALSE);
    pe = proctable::Lookup("textview-reverse-search");
    if (pe) help_textRevSearch =  proctable::GetFunction(pe);
    else THROWONFAILURE(FALSE);
    pe = proctable::Lookup("textview-search-again");
    if (pe) help_textSearchAgain =  proctable::GetFunction(pe);
    else THROWONFAILURE(FALSE);
    pe = proctable::Lookup("textview-copy-region");
    if (pe) help_textCopyRegion =  proctable::GetFunction(pe);
    else THROWONFAILURE(FALSE);
    pe = proctable::Lookup("textview-next-screen");
    if (pe) help_textPageDown =  proctable::GetFunction(pe);
    else THROWONFAILURE(FALSE);
    pe = proctable::Lookup("textview-prev-screen");
    if (pe) help_textPageUp =  proctable::GetFunction(pe);
    else THROWONFAILURE(FALSE);
    pe = proctable::Lookup("frame-set-printer");
    if (pe) help_frameSetPrinter =  proctable::GetFunction(pe);
    else THROWONFAILURE(FALSE);
#ifdef USE_PRINTOPTS
    pe = proctable::Lookup("printopts-post-window");
    if (pe) help_poptPostWindow = proctable::GetFunction(pe);
    else THROWONFAILURE(FALSE);
#endif

    /* now add ourself to the head of the instance list */
    DEBUG(("help_ego: %d\n",(int)help_ego));
    id = (struct self_help *)malloc(sizeof(struct self_help));
    if (!id)
	THROWONFAILURE( FALSE);
    id->this_c = this;
    id->next = help_ego;
    help_ego = id;
    DEBUG(("help_ego: %d\n",(int)help_ego));

    this->app = NULL;
    this->showing = FALSE;

    DEBUG(("OUT init obj\n"));
    THROWONFAILURE( TRUE);
}


/*
 * destroys the resources of the help object
 */
help::~help()
{
	ATKinit;

    DEBUG(("IN finalize\n"));
    this->mainLpair->obj[0] = NULL;
    this->mainLpair->obj[1] = NULL;
    (this->mainLpair)->Destroy();

    (this->info->view)->Destroy();
    (this->info->data)->Destroy();

    this->panelLpair1->obj[0] = NULL;
    this->panelLpair1->obj[1] = NULL;
    (this->panelLpair1)->Destroy();

    if(this->panelLpair2) {
	this->panelLpair2->obj[0] = NULL;
	this->panelLpair2->obj[1] = NULL;
	(this->panelLpair2)->Destroy();
    }

    if(this->overviewLpair) {
	this->overviewLpair->obj[0] = NULL;
	this->overviewLpair->obj[1] = NULL;
	(this->overviewLpair)->Destroy();
	(this->overviewScroll)->Destroy();
	(this->overviewPanel)->Destroy();
	(this->overviewLabV)->Destroy();
	(this->overviewLab)->Destroy();
    }

    if(this->listLpair) {
	this->listLpair->obj[0] = NULL;
	this->listLpair->obj[1] = NULL;
	(this->listLpair)->Destroy();
	(this->listScroll)->Destroy();
	(this->listPanel)->Destroy();
	(this->listLabV)->Destroy();
	(this->listLab)->Destroy();
    }

    if(this->historyLpair) {
	this->historyLpair->obj[0] = NULL;
	this->historyLpair->obj[1] = NULL;
	(this->historyLpair)->Destroy();
	(this->historyScroll)->Destroy();
	(this->historyPanel)->Destroy();
	(this->historyLabV)->Destroy();
	(this->historyLab)->Destroy();
    }

    /* the presence/absence of self->info indicates whether we are
       in the process of finalizing, so that in update we know we
       can't post menus */
    free(this->info);
    this->info = NULL;
    if(help_panelList) FreePanelListData();
    DEBUG(("info\n"));
    DEBUG(("OUT finalize\n"));
}

     
/*
 * help__ method interface for using an alias file
 */
void help::SetAliasesFile(const char  *alias)
{
	ATKinit;

    helpdb::ReadAliasesFile(alias);
}

/*
 * help__ method for adding a help index directory
 */
void help::SetIndex(const char  *aindex)
{
	ATKinit;

    DEBUG(("IN SetIndex: %s\n",aindex));
    helpdb::SetIndex(aindex);
}

/*
 * outside interface help__ method for GetHelpOn.  Used by helpapp
 *
 * Returns:
 *	-1: if a server was down while trying to get help on this topic
 *	 0: if no help found for this topic
 *	 1: if successful
 */
int help::HelpappGetHelpOn(const char  *aname	/* what topic */, long  isnew	/* is this a new topic? */, int  ahistory	/* show in history log? 1-show aname 2-show tail of filename */, char  *errmsg	/* error to print if failure. "Error" if this is NULL */)
{
	ATKinit;

    if (aname[0] == '\0') {
	/* so -e flag with no file shown works */
	return 1;
    }
    /* use the last instance added to the list */
    DEBUG(("using %d view %d asking %d\n", (int)help_ego->this_c,(int)help_ego->this_c->info->view,
	   message::Asking(help_ego->this_c->info->view)));
    if (message::Asking(help_ego->this_c->info->view)) {
	DEBUG(("retracting\n"));
	message::CancelQuestion(help_ego->this_c->info->view);
    }
    return help_GetHelpOn(help_ego->this_c, aname, isnew, ahistory, errmsg);
}


/*
 * return the first view in the list of instances, so helpapp can expose
 * a hidden window.
 */
class view *help::GetInstance()
{
	ATKinit;

    return (class view *)(help_ego->this_c);
}


/*---------------------------------------------------------------------------*/
/*				HELP METHODS				     */
/*---------------------------------------------------------------------------*/

/*
 * self explanatory
 */
void help::PostMenus(class menulist  *menuList)
{
    DEBUG(("post menus\n"));
    if (this->info != NULL)
	(this)->view::PostMenus( this->info->menus);
}

/*
 * override parents' keybindings
 */
void help::PostKeyState(class keystate  *keyState)
{
    DEBUG(("post keys\n"));
    (this->state)->AddBefore( keyState);
    (this)->view::PostKeyState( this->state);
}

/*
 * mouse action handler
 */
class view *help::Hit(enum view_MouseAction  action, long  x , long  y, long  numberOfClicks)
{
    class view *ret;
    char *topic;
    enum view_MouseAction hyp_action;

    if(this->showing) return((class view *) this);

    if (this->showPanels)
	ret = (this->mainLpair)->Hit( action, x, y, numberOfClicks);
    else
	ret = (this->info->scroll)->Hit( action, x, y, numberOfClicks);
    /* Check to see if we hit a "helptopic" style. */
    if (ret == this->info->view) {
	if (ATK::IsTypeByName((this->info->view)->GetTypeName(), "hlptextview")) {
	    topic = ((class hlptextview *)(this->info->view))->GetHyperTopic();
	    hyp_action = ((class hlptextview *)(this->info->view))->GetMouseAction();
	    if (topic) {
		static char mess[512];
		sprintf(mess, "Searching for help on \"%s\"...", topic);
		this->showing = TRUE;
		((class hlptextview *)(this->info->view))->SetDotLength( 0);
		message::DisplayString(this, 0, mess);
		im::ForceUpdate();
		ShowHelp(this, topic, hyp_action == view_RightDown);
		message::DisplayString(this, 0, "");
		return((class view *) this);    /* We have a new topic. */
	    }
	}
    }
    return ret;
}

/*
 * set up the view tree based on whether panels or scrollbar is the top level view
 */
void help::LinkTree(class view  *parent)
{
    DEBUG(("IN link tree\n"));
    DEBUG(("\tsuper..."));
    (this)->view::LinkTree( parent);

    if (this->showPanels) {
	if (this->mainLpair)
	    (this->mainLpair)->LinkTree( this);
    } else {
	if (this->info!=NULL && this->info->scroll)
	    (this->info->scroll)->LinkTree( this);
    }
    DEBUG(("OUT link tree\n"));
}

/*
 * refreshing!
 */
void help::FullUpdate(enum view_UpdateType  type, long  left , long  top , long  width , long  right)
{
    struct rectangle childRect;

    DEBUG(("IN fullupdate\n"));

    (this)->GetLogicalBounds( &childRect);

    if (this->showPanels) {	/* need the main lpair as the root */
	(this->mainLpair)->InsertView( this, &childRect);
	(this->mainLpair)->FullUpdate( type, left, top, width, right);
    } else {			/* we just have the textview to show */
	(this->info->scroll)->InsertView( this, &childRect);
	(this->info->scroll)->FullUpdate( type, left, top, width, right);
    }
    /* NEVER EVER mess with the input focus in a fullupdate routine (or at least if you do so, check to make sure you have the input focus to start with...) */
    /* super_WantInputFocus(self, self->info->view); */
    DEBUG(("OUT fullupdate\n"));
}

/*
 * erase and refresh the screen
 */
void help::Update()
{
    DEBUG(("IN update\n"));
    /* clear out the region, then do a full redraw */
    (this)->EraseVisualRect();
    (this)->FullUpdate( view_FullRedraw, (this)->GetLogicalTop(), (this)->GetLogicalLeft(),
        (this)->GetLogicalWidth(), (this)->GetLogicalHeight());
    DEBUG(("OUT update\n"));
}

/*
 * update event handler
 */
void help::WantUpdate(class view  *requestor)
{
    /* if the scrollbar took the hit, check if the user has selected something,
       and turn on some menus items if so */
    if (this->info && (requestor == (class view *)this->info->scroll)) {
	/* if the menumask changes, post menus */
	if ((this->info->menus)->SetMask(
			 (((class textview *)this->info->view)->GetDotLength() != 0) ?
			 (this->info->menus)->GetMask() | MENU_SwitchCopy :
			 (this->info->menus)->GetMask() & ~MENU_SwitchCopy))
	    (this)->PostMenus( this->info->menus);
    }
    (this)->view::WantUpdate( requestor);
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
 * classproc to handle getting help using a terminal-based interface
 */
void help::GetHelpOnTerminal(const char  *akey		/* topic string */,int  list		/* do help on topic, or just list files? */,int  print		/* prompt for printing each helpfile? */)
{
	ATKinit;

    FILE *tfile;
    char *alias, *tmp;
    const char *pager;
    const char *index;
    char pathName[MAXPATHLEN], cmdLine[MAXPATHLEN];
    char hbuffer[HNSIZE], tbuffer[HNSIZE];
    long digitMode, digitValue, firstChar;
    struct helpFile *tmpf;
    struct cache c;
    int tc;
    int first = 0;

    DEBUG(("key: %s\n",akey));
    if (!helpdb::CheckIndex(NULL)) { /* if we haven't SetIndex */
	index = environ::GetConfiguration(SETUP_INDEXDIR);
	if (index == NULL) 
	    index = environ::AndrewDir(DEFAULT_INDEXDIR);
	help::SetIndex(index);
    }

    if ((pager = environ::Get(PAGERVAR)) == (char *) NULL)
	pager = DEFAULTPAGER;

    if (!helpdb::CheckIndex(NULL)) {
        fprintf(stderr, err_index, pathName);
        return;
    }

    /* map helpKey */
    strncpy(tbuffer, akey, HNSIZE);
    tbuffer[HNSIZE-1] = 0;
    LowerCase(tbuffer);

    /* now see if it has an illegal, command running, alias */
    alias = (char *)helpdb::MapAlias(tbuffer);
    DEBUG(("map alias on: %s\n",tbuffer));
    if (alias) {
	DEBUG(("alias: %s\n", alias));
        if (alias[0] == '#') {
            fprintf(stderr,err_terminal);
	    putchar('\n');
            exit(1);
	    /*NOTREACHED*/
        }
    }

    alias = tbuffer;		/* src */
    tmp = hbuffer;		/* dst */
    digitMode = 0;
    digitValue = 0;
    firstChar = 1;
    /* copy alias (points to tbuffer) to hbuffer, suppressing spaces and
       computing digitValue */
    while((tc = *alias++) != '\0') {
        if (tc == ' ' || tc == '\n' || tc == '\t')
	    continue;
        if (tc == '(' || (!firstChar && tc == '.'))
	    digitMode = 1;
        else if (digitMode) {
            if (tc >= '0' && tc <= '9') {
                digitValue *= 10;
                digitValue += tc - '0';
            }
        } else {
            *tmp++ = tc;
	}
        firstChar = 1;
    }
    *tmp++ = 0;

    DEBUG(("tbuf: %s hbuf: %s\n",tbuffer,hbuffer));

    c.all = (struct helpFile *)NULL;

    if (helpdb::SetupHelp(&c, hbuffer, FALSE) == 0) {
        fprintf(stderr, err_sorry, akey);
	putchar('\n');
        exit(1);
	/*NOTREACHED*/
    }

    /* if non-0 return, c->all was set up for us */

    DEBUG(("setup done\n"));
    
    if (list) {
	printf("Using search directories: ");
	helpdb::PrintSearchDirs();
	printf("\n");
        printf("For topic %s the files are:\n",akey);
	for (tmpf = c.all; tmpf; tmpf = tmpf->next) {
	    printf("\t%s\n",tmpf->fileName);
	}
	exit(0);
	/*NOTREACHED*/
    }

    /* o.w. do real help on the topic */
    first = TRUE;
    for(tmpf = c.all; tmpf; tmpf = tmpf->next) {
	int in = 0;
	char *fn;
	int skipFlag = FALSE;

	tfile = fopen(tmpf->fileName, "r");
	if (!tfile) {
	    fprintf(stderr, err_file, tmpf->fileName);
	    continue;
	}

	if (!first) {
	    skipFlag = FALSE;
	    fn = strrchr(tmpf->fileName, '/');
	    printf(msg_term_prompt, fn ? fn+1 : tmpf->fileName);
	    in = getchar();
	    DEBUG(("'%c'\n", in));
	    if (in < 0) exit(1);
	    else if (in == 'q' || in == 'Q') exit(0);
	    else if (in == 'n' || in == 'N') skipFlag = TRUE;
	    if (in != '\n')
		while ((in = getchar()) > 0 && in != '\n')
		    ;
	}

	first = FALSE;
	
	if (!skipFlag) {
	    /* show the file */
	    tc = fgetc(tfile);
	    ungetc(tc,tfile);
	    fclose(tfile);
	    if ((tc == '.') || (tc == '#')) { /* troff file */
		sprintf(cmdLine, ROFFCMD, tmpf->fileName, pager);
	    } else {
		sprintf(cmdLine, ATKROFFCMD, tmpf->fileName, pager);
	    }
	    DEBUG(("cmd: %s\n",cmdLine));
	    safesystem(cmdLine);
	}

	if (print) {
	    /* see if they want to print it */
	    if (in < 0) exit(1);
	    fn = strrchr(tmpf->fileName, '/');
	    printf(msg_print_prompt, fn ? fn+1 : tmpf->fileName);
 	    in = getchar();
	    DEBUG(("'%c'\n", in));
	    if (in < 0) exit(1);
	    else if (in != 'n' && in != 'N') { /* print it */
		if ((tc == '.') || (tc == '#')) { /* troff file */
		    sprintf(cmdLine, ROFFPRINTCMD, tmpf->fileName);
		} else {
		    sprintf(cmdLine, ATKPRINTCMD, tmpf->fileName);
		}
		DEBUG(("cmd: %s\n",cmdLine));
		safesystem(cmdLine);
	    }
	    if (in != '\n')
		while ((in = getchar()) > 0 && in != '\n')
		    ;
	}
    }
}

static class frame *
getframe(class view  *vw)
{
    while(vw->parent) {
	vw = vw->parent;
	if(ATK::IsTypeByName((vw)->GetTypeName(),"frame"))
	    return((class frame*) vw);
    }
    return(NULL);
}

static void 
destroyWindow(class help  *self)
{
    class frame *fr;
    struct proctable_Entry *pr;
    proctable_fptr proc;

    if((pr = proctable::Lookup("frame-delete-window")) != NULL && 
	proctable::Defined(pr) &&
	(fr = getframe((class view*)self)) != NULL) {
	proc = proctable::GetFunction(pr) ;
	(self)->UnlinkTree();
	(self)->DeleteApplicationLayer(self->app);
	(*proc)(fr,0);
    }
}

/*
 * Allows help to just delete one window
 */
void help_aux_ExitProc(class help  *self)
{
    struct self_help *t, *p;

    DEBUG(("IN exit\n"));

    DEBUG(("help_ego: %d self: %d\n",(int)help_ego, (int)self));
    for (t=help_ego;t;t=t->next) {
	DEBUG(("t: %d this: %d next: %d\n", (int)t, (int)t->this_c, (int)t->next));
    }

    /* delete self from the instance list */
    p = NULL;
    t = help_ego;
    while (t) {
	if (t->this_c == self) {
	    if (p == NULL) {	/* deleting head */
		help_ego = t->next;
		free(t);
		t = help_ego;
	    } else {
		p->next = t->next;
		free(t);
		t = p->next;
	    }
	} else {
	    p = t;
	    t = t->next;
	}
    }
    DEBUG(("help_ego: %d self: %d\n",(int)help_ego, (int)self));
    for (t=help_ego;t;t=t->next) {
	DEBUG(("t: %d this: %d next: %d\n", (int)t, (int)t->this_c, (int)t->next));
    }

    /* take care of the cases with 0 and 1 instances left in the list */
    if (help_ego) {			/* something in the list */
	if (help_ego->next == (struct self_help *)NULL) { /* only one left */
	    /* remove "Delete this window" menu item */
	    DEBUG(("one left, Delete delete\n"));
	    help_ego->this_c->info->flags &= ~MENU_SwitchDeleteMenu;
	    SetupMenus(help_ego->this_c->info);
	}
    } else {			/* nobody left, all gone, we be outta here */
	im::KeyboardExit();
    }

    destroyWindow(self);

    DEBUG(("OUT exit\n"));
}

/*
 * print a help file
 */
void help_aux_Print(class help  *self)
{
    int pmode;
#ifdef PSPRINTING_ENV
    pmode = print_PRINTPOSTSCRIPT;
#else
    pmode = print_PRINTTROFF;
#endif
    message::DisplayString(self, 0, msg_print_queue);
    im::ForceUpdate();
    im::SetProcessCursor(help_waitCursor);
    print::ProcessView((class textview *)self->info->view, pmode, 0, "", "");
    message::DisplayString(self, 0, msg_queue_done);
    im::SetProcessCursor((class cursor *) NULL);
}

/*
 * show the new help
 */
static void
ShowHelp(class help  *self, char  *topic, boolean  new_win)
{ 
    class help *hv;
    class im *imp;
    class frame *framep;
    char buf[HNSIZE + HELP_MAX_ERR_LENGTH];
    struct self_help *tmp;

    hv = NULL;
    if (new_win) {
	imp = im::Create(NULL);
	framep = new frame;
	hv = (class help*) ATK::NewObject("help");
	if (!hv || !imp || !framep) {
	    ERRORBOX(self->info->view, err_no_new_view);
	    self->showing = FALSE;
	    return;
	}

	/* since we exist, help_ego must exist now, too */
	if (help_ego->next != (struct self_help *)NULL) { /* > 1 instances */

	    for (tmp = help_ego; tmp; tmp = tmp->next) {
		/* add "delete this window" menu item" */
		DEBUG(("Add delete\n"));
		tmp->this_c->info->flags |= MENU_SwitchDeleteMenu;
		SetupMenus(tmp->this_c->info);
	    }
	}
	
	/* frame for frame_SetView must have associated im */
	hv->app = (class scroll*) (hv)->GetApplicationLayer();
	(framep)->SetView( hv->app);
	(imp)->SetView( framep);

	/* add in a message handler */
	(framep)->PostDefaultHandler( "message",
				 (framep)->WantHandler( "message"));

    }
    
    sprintf(buf, err_sorry, topic);
    if (help_GetHelpOn(hv ? hv : self
		  , topic, help_NEW, help_HIST_NAME, buf) != 0) {
	(self->overviewPanel)->ClearSelection();
	(self->listPanel)->ClearSelection();
	(self->historyPanel)->ClearSelection();
    }

    im::ForceUpdate();
    self->showing = FALSE;
}

struct helpdb_completesplot {
    int sofar, len;
    char *origstr; 
    char *buffer;
    int buffersize;
    enum message_CompletionCode res;
};

static void CompletionSplot(char  *name , char  *original, struct helpdb_completesplot  *rock)
{
    int ix, jx;

    ix = lenstrcmp(name, rock->origstr);
    if (ix == (-1)) {
	/* exactly the same as the input */
	strncpy(rock->buffer, name, rock->buffersize);
	rock->buffer[rock->buffersize-1] = '\0';
	if (rock->res==message_Valid || rock->res==message_CompleteValid)
	    rock->res = message_CompleteValid;
	else
	    rock->res = message_Complete;
	rock->sofar = strlen(rock->buffer);
    }
    else if (ix == rock->len) {
	/* matched all of the input, but there's more */
	if (rock->sofar < rock->len)  {
	    /* first one found that does -- use all of it */
	    strncpy(rock->buffer, name, rock->buffersize);
	    rock->buffer[rock->buffersize-1] = '\0';
	    rock->sofar = strlen(rock->buffer);
	    rock->res = message_Complete;
	}
	else {
	    /* another -- use the greatest common prefix */
	    jx = lenstrcmp(name, rock->buffer);
	    if (jx==(-1)) {
		/* this is the same alias -- ignore it! */
	    }
	    else if (jx==rock->sofar)
		rock->res = message_CompleteValid;
	    else {
		rock->buffer[jx] = '\0';
		rock->sofar = jx;
		rock->res = message_Valid;
	    }
	}
    }
    else if (ix > rock->sofar) {
	/* matched part of input, but more than we have so far */
	strncpy(rock->buffer, name, ix);
	rock->buffer[ix] = '\0';
	rock->sofar = ix;
	rock->res = message_Invalid;
    }

}

static enum message_CompletionCode HelpCompletionProc(char  *string, class help  *self, char  *buffer, int  buffersize)
{
    struct helpAlias *ta;
    int ix, jx;
    char origstr[HNSIZE];
    struct helpdb_completesplot hcsplot;

    strncpy(origstr, string, HNSIZE);
    origstr[HNSIZE-1] = '\0';

    hcsplot.origstr = origstr;
    hcsplot.len = strlen(origstr);
    hcsplot.sofar = (-1);
    hcsplot.buffer = buffer;
    hcsplot.buffersize = buffersize;
    strcpy(hcsplot.buffer, "");
    hcsplot.res = message_Invalid;

    helpdb::EnumerateAll((helpdb_efptr)CompletionSplot, (char *)&hcsplot);

    message::DeleteCharacters(self, strlen(buffer), 999);

    return hcsplot.res;
}

/* like strcmp(), but it returns the index of the first character where s1 and s2 differ. If s1 and s2 are identical, returns -1. */
static int lenstrcmp(char *s1 , char *s2)
{
    int ix;
    unsigned char c1, c2;
    for (ix=0; TRUE; ix++, s1++, s2++) {
	if (!*s1 && !*s2) return (-1);
	c1 = (isupper(*s1)) ? tolower(*s1) : *s1;
	c2 = (isupper(*s2)) ? tolower(*s2) : *s2;
	    
	if (c1 != c2) return ix;
    }
}

struct help_helpsplot {
    message_workfptr HelpWork;
    char *hrock;
    char *keywd;
    int keywdlen;
    long nummatches;
};

static void HelpEnumProc(char  *name , char  *original, struct help_helpsplot  *rock)
{
    int ix = lenstrcmp(name, rock->keywd);
    char *tmp = NULL;
    if (ix==(-1) || ix==rock->keywdlen) {
	/* set tmp to the final component of the help filename (or the whole thing if it's just an alias entry */
	/*tmp = rindex(original, '/');
	if (!tmp)
	    tmp = original;
	else
	    tmp++;*/
	/* Actually, we leave tmp==NULL. It would be nice to have the help filenames in a second column in the buffer, but the tab stops are all wrong, so it looks terrible. */
	(*(rock->HelpWork))((long)rock->hrock, message_HelpListItem, name, tmp);
	rock->nummatches++;
    }
}

static void HelpHelpProc(char  *partialKeyword, class help  *rock, message_workfptr HelpWork, char  *hrock)
{
    struct help_helpsplot hhsplot;

    if (!HelpWork) return;
    hhsplot.HelpWork = HelpWork;
    hhsplot.hrock = hrock;
    hhsplot.keywd = partialKeyword;
    hhsplot.keywdlen = strlen(partialKeyword);
    hhsplot.nummatches = 0;
    if (hhsplot.keywdlen == 0) {
	/* if we don't return at this point, the help buffer will show a list of every help topic and alias available. Nice as that might seem, we can't allow it, because it's several thousand items and they're sorted using an O(N^2) algorithm -- takes months. */
	(*HelpWork)((long)hrock, message_HelpGenericItem, "Enter the beginning of a help topic name and hit \"?\" to list all topics beginning with that string.\n\n", NULL);
	return;
    }

    (*HelpWork)((long)hrock, message_HelpGenericItem, "Possible completions for \"", NULL);
    (*HelpWork)((long)hrock, message_HelpGenericItem, partialKeyword, NULL);
    (*HelpWork)((long)hrock, message_HelpGenericItem, "\":\n\n", NULL);
    helpdb::EnumerateAll((helpdb_efptr)HelpEnumProc, (char *)&hhsplot);

    if (hhsplot.nummatches == 0) {
	(*HelpWork)((long)hrock, message_HelpGenericItem, "There are no help topics installed that begin with that string.\n", NULL);
    }
}

/*
 * get help on a prompted-for topic or a selected word, bringing up a
 * new window if necessary
 */
void help_aux_NewHelp(class help  *self, long  type		/* help_ON 			if Help On... */)
			/* help_ON & help_NEW_WIN	if New Help On... */
			/* help_SEL			if Help On Selected */
			/* help_SEL & help_NEW_WIN	if New Help On Selected */
{
    int i, pos, len, code;
    char tc;
    char helpName[HNSIZE];

    if (type & help_SEL) {
	pos = ((class textview *)self->info->view)->GetDotPosition();
	len = ((class textview *)self->info->view)->GetDotLength();

	/*  only shows menu item if a selection has been made */
	/*  but, since can bind the procedure, must handle this */
	/*  case anyway. */
	if (len == 0) {
	    ERRORBOX(self->info->view, err_no_sel);
	    return;
	} else {
	    if (len >= HNSIZE) {
		ERRORBOX(self->info->view, err_sel_too_long);
		return;
	    }
	}
	i = 0;			/* position in text object */
	code = 0;               /* position in helpName */
	while(i<len) {
	    tc = ((class text *)self->info->data)->GetChar( i+pos);
	    i++;
	    if (tc == ' ' || tc == '\t' || tc == '\n') continue;
	    helpName[code++] = tc;
	}
	helpName[code++] = '\0';
    } else {			/* prompt for topic */
	code = message::AskForStringCompleted(self, 0, msg_ask_prompt, 0, helpName, HNSIZE, NULL, (message_completionfptr)HelpCompletionProc, (message_helpfptr)HelpHelpProc, (long)self, 0);
	if ((code < 0) || (helpName[0] == '\0')) return;
    }

    ShowHelp(self, helpName, (type & help_NEW_WIN) != 0);
}


/*
 * Add a history item for a given file with given dot, dotlen and top
 */
void help_aux_AddBookmark(class help  *self)
{
    AddHistoryItem(self, help_HE_BOOKMARK, help_SHOW_HIST);
}


/*
 * Adds a search directory to the searchpath
 */
void help_aux_AddSearchDir(class help  *self)
{
    char buf[MAXPATHLEN];
    char buf2[MAXPATHLEN+100];
    int code;

    sprintf(buf2, "%s", "Added directory: ");
    code = completion::GetFilename((class view *)self, msg_dir_prompt, NULL,
				  buf, sizeof(buf), TRUE, TRUE);
    if(code != -1) {
	help::AddSearchDir(buf);
	strcat(buf2, buf);
	message::DisplayString(self, 0, buf2);
    }
}

