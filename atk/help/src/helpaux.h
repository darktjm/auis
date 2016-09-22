extern NO_DLL_EXPORT void init_hlptextview(class hlptextview  *hv);
extern NO_DLL_EXPORT void help_aux_ExitProc(class help  *self);
extern NO_DLL_EXPORT void help_aux_Print(class help  *self);
extern NO_DLL_EXPORT void help_aux_NewHelp(class help  *self, long  type		/* help_ON 			if Help On... */);
extern NO_DLL_EXPORT void help_aux_AddBookmark(class help  *self);
extern NO_DLL_EXPORT void help_aux_AddSearchDir(class help  *self);
extern NO_DLL_EXPORT NO_DLL_EXPORT boolean help_class_inited;
#undef ATKinit
#define ATKinit if(!help_class_inited) InitializeClass()
extern NO_DLL_EXPORT void  AddHistoryItem (class help  *self, int  marcp			/* is this a bookmark? */, int  flash			/* should we expose the history panel? */);
extern NO_DLL_EXPORT int help_GetHelpOn(class help  *self, const char  *aname	/* what topic */, long  isnew	/* is this a new topic? */, int  ahistory	/* show in history log under what name?
		   help_HIST_NOADD - none at all,
		   help_HIST_NAME - aname,
		   help_HIST_TAIL - tail of the found filename
		   */, const char  *errmsg	/* error to print if failure. "Error" if this is NULL */);
extern NO_DLL_EXPORT void  SetupMenus(struct cache  *c);
extern NO_DLL_EXPORT void NextHelp(class help  *self);
extern NO_DLL_EXPORT void  HistoryHelp(class help  *self		/* callback rock */, struct history_entry  *ent	/* panelEntry rock */, class panel  *apanel		/* appropriate panel */);
extern NO_DLL_EXPORT void  OverviewHelp(class help  *self, char  *name		/* which topic to request - panelEntry rock */, class panel  *apanel);
extern NO_DLL_EXPORT void FreePanelListData();
extern NO_DLL_EXPORT long SetupPanel(boolean  readpairs, const char  *fname, class panel  *panel		/* the panel to add entries to */, const char  * const *def);
extern NO_DLL_EXPORT class view *SetupLpairs(class help  *self);
extern NO_DLL_EXPORT void  ToggleProgramListSize(class help * self, long  rock);
extern NO_DLL_EXPORT char * LowerCase(char  *astring);
