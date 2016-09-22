extern NO_DLL_EXPORT boolean textview_PrevCharIsNewline(class text *t, long pos);
extern NO_DLL_EXPORT int drawtxtv_tabscharspaces;
extern NO_DLL_EXPORT int textview_SizeOfLongestLineOnScreen(textview *self);
extern NO_DLL_EXPORT int textview_SizeOfLongestLineOnScreen(textview *self);	/* should be a method */
extern NO_DLL_EXPORT long lcInsertEnvironment;
extern NO_DLL_EXPORT long lcNewLine;
extern NO_DLL_EXPORT long textview_ExposeRegion(class textview *tv, long pos1, long rlen, class view *inset, const struct rectangle &area, struct rectangle &hit, long &off, long extra);
extern NO_DLL_EXPORT long textview_LineStart(class textview *tv, long curx, long cury, long xs, long ys, long pos, long *lend=NULL, long *lheight=NULL);
extern NO_DLL_EXPORT void textview_ComputeViewArea(class textview *tv, const struct rectangle &area, long &curx, long &cury, long &xs, long &ys);
extern NO_DLL_EXPORT void textview_DestroyPrintingLayout(class textview  *txtv);
extern NO_DLL_EXPORT void textview_DestroyPrintingLayoutPlan(class textview  *txtv);
extern NO_DLL_EXPORT void textview_InitializePS(void);
