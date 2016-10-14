#ifndef _help_H_
#define _help_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

/** \addtogroup libhelp Object help
 *  A dynamically loadable ATK class file.  Contains help, hlptextview,
 *  helpdb, helpapp.
 * @{ */
/*---------------------------------------------------------------------------*/
/*	MODULE: help.H							     */
/*		Subclass of view, includes all major lookup and display      */
/*		routines.						     */
/*---------------------------------------------------------------------------*/

#define help_VERSION		2

/*
 * this is done since macromethods don't work for help_ calls on OTHER
 * help object from within the help.c code
 */
#define help_GetInfo(hv) ((hv)->info)

#include <view.H>
class help : public  view {
public:
	TRACED_CLASS(help);

    
 
     void PostMenus(class menulist *menuList);
     void PostKeyState(class keystate *keyState);
     void FullUpdate(enum view::UpdateType type, long left, long top, long width, long right);
     void WantUpdate(class view *req);
     void Update();
     void LinkTree(class view *parent);
     class view * Hit (enum view::MouseAction action, long x, long y, long numberOfClicks)  ;
    
 
    
 
    static int HelpappGetHelpOn(const char *name, long number, int addToHistory, char *errmsg)  ;
    static void GetHelpOnTerminal(const char *akey, int list, int print);
    
	help();
    
	static boolean InitializeClass();

    
    static void SetIndex(const char *indexName);
    static void SetAliasesFile(const char *aliasName);
    static void AddSearchDir(const char *dirName);
    static class view * GetInstance()  ;

 
    class keystate *state;	/* our key and menu bindings */
    char expandedList;		/* is the program list expanded? */
    char showPanels;		/* are we to show the panels or not? */
    char showHistory;		/* are we showing the history now */
    char showList;
    char showOverview;

    struct cache *info;		/* menulist, keystate, view, dataobj... */

    class panel *overviewPanel;/* overview panel and its label */
    class lpair *overviewLpair;
    class label *overviewLab;
    class labelview *overviewLabV;
    class scroll *overviewScroll; /* and its scrollbar */

    class panel *listPanel;	/* program list panel and its label */
    class lpair *listLpair;
    class label *listLab;
    class labelview *listLabV;
    class scroll *listScroll;	/* ditto */

    class panel *historyPanel;	/* history  panel and its label */
    class lpair *historyLpair;
    class label *historyLab;
    class labelview *historyLabV;
    class scroll *historyScroll;	/* ditto */

    class lpair *mainLpair;	/* lpair connecting the textview and the panels */
    class lpair *panelLpair1;	/* lpair connecting the panels */
    class lpair *panelLpair2;	/* another lpair connecting the panels, not always used */

    class panel *tmpanel;	/* for filtered programList */
    class panel *oldpanel;	/* to placehold original programPanel */

    class scroll *app;		/* This view's application layer */

    boolean showing;
};
/** @} */
#endif /* _help_H_ */
