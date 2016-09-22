/* x11r4.c */
typedef struct gwindow *Gwindow;
typedef int COORD;

extern NO_DLL_EXPORT Pointer X11init(int gdevhandle);
extern NO_DLL_EXPORT void X11start(Gwindow w);
extern NO_DLL_EXPORT void X11flush(Gwindow w);
extern NO_DLL_EXPORT void X11stop(Gwindow w);
extern NO_DLL_EXPORT void X11rect(Gwindow w, COORD x, COORD y, COORD x2, COORD y2);
extern NO_DLL_EXPORT void X11text(Gwindow w, COORD x, COORD y, char *text);
extern NO_DLL_EXPORT void X11point(Gwindow w, COORD x, COORD y);
extern NO_DLL_EXPORT void X11line(Gwindow w, COORD x1, COORD y1, COORD x2, COORD y2);
extern NO_DLL_EXPORT void X11setdim(Gwindow w, COORD width, COORD height);
extern NO_DLL_EXPORT void X11getdim(Gwindow w, COORD *wp, COORD *hp);
extern NO_DLL_EXPORT void X11menuitem(Gwindow w, char *itemname, int retval, int addflag);
extern NO_DLL_EXPORT void X11mouseinterest(Gwindow w, int event);
