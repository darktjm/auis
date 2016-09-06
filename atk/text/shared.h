extern boolean textview_PrevCharIsNewline(class text *t, long pos);
extern int drawtxtv_tabscharspaces;
extern int textview_SizeOfLongestLineOnScreen(textview *self);
extern int textview_SizeOfLongestLineOnScreen(textview *self);	/* should be a method */
extern long lcInsertEnvironment;
extern long lcNewLine;
extern long textview_ExposeRegion(class textview *tv, long pos1, long rlen, class view *inset, const struct rectangle &area, struct rectangle &hit, long &off, long extra);
extern long textview_LineStart(class textview *tv, long curx, long cury, long xs, long ys, long pos, long *lend=NULL, long *lheight=NULL);
extern void textview_ComputeViewArea(class textview *tv, const struct rectangle &area, long &curx, long &cury, long &xs, long &ys);
extern void textview_DestroyPrintingLayout(class textview  *txtv);
extern void textview_DestroyPrintingLayoutPlan(class textview  *txtv);
extern void textview_InitializePS(void);
