extern void init_hlptextview(class hlptextview  *hv);
extern void help_aux_ExitProc(class help  *self);
extern void help_aux_Print(class help  *self);
extern void help_aux_NewHelp(class help  *self, long  type		/* help_ON 			if Help On... */);
extern void help_aux_AddBookmark(class help  *self);
extern void help_aux_AddSearchDir(class help  *self);
extern boolean help_class_inited;
#undef ATKinit
#define ATKinit if(!help_class_inited) InitializeClass()
extern void  AddHistoryItem (class help  *self, int  marcp			/* is this a bookmark? */, int  flash			/* should we expose the history panel? */);
extern int help_GetHelpOn(class help  *self, const char  *aname	/* what topic */, long  isnew	/* is this a new topic? */, int  ahistory	/* show in history log under what name?
		   help_HIST_NOADD - none at all,
		   help_HIST_NAME - aname,
		   help_HIST_TAIL - tail of the found filename
		   */, const char  *errmsg	/* error to print if failure. "Error" if this is NULL */);
extern void  SetupMenus(struct cache  *c);
extern void NextHelp(class help  *self);
extern void  HistoryHelp(class help  *self		/* callback rock */, struct history_entry  *ent	/* panelEntry rock */, class panel  *apanel		/* appropriate panel */);
extern void  OverviewHelp(class help  *self, char  *name		/* which topic to request - panelEntry rock */, class panel  *apanel);
extern void FreePanelListData();
extern long SetupPanel(boolean  readpairs, const char  *fname, class panel  *panel		/* the panel to add entries to */, const char  * const *def);
extern class view *SetupLpairs(class help  *self);
extern void  ToggleProgramListSize(class help * self, long  rock);
char * LowerCase(char  *astring);
