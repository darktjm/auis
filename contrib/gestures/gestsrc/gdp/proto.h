/* draw.c */
extern int DrawDebug;
extern void CacheStats(void);
extern void CacheStats(void);
extern void DrawSync(void);
extern void DrawClear(void);
extern int DistanceToPoint(int x1, int y1, int x2, int y2);
/* gesture.c */
extern void GESTUREinit(int ac, char **av);
extern void GESTUREcharacter(int c);
extern int GESTUREgetchar(void);
extern void GESTUREgetXYT(int *xp, int *yp, int *tp);
extern int Ggetchar(void);
extern void GgetXYT(int *xp, int *yp, int *tp);
/* gpoint.c */
extern void Sreset(void);
extern void Serase(void);
extern void Gpoint(int x, int y);
extern void Gline(int x1, int y1, int x2, int y2);
/* wm.c */
extern void WmClear(void);
extern void WmInit(char *program);
extern void WmFlush(void);
