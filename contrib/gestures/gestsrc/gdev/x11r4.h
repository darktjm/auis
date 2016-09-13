/* x11r4.c */
typedef struct gwindow *Gwindow;
typedef int COORD;

extern Pointer X11init(int gdevhandle);
extern void X11start(Gwindow w);
extern void X11flush(Gwindow w);
extern void X11stop(Gwindow w);
extern void X11rect(Gwindow w, COORD x, COORD y, COORD x2, COORD y2);
extern void X11text(Gwindow w, COORD x, COORD y, char *text);
extern void X11point(Gwindow w, COORD x, COORD y);
extern void X11line(Gwindow w, COORD x1, COORD y1, COORD x2, COORD y2);
extern void X11setdim(Gwindow w, COORD width, COORD height);
extern void X11getdim(Gwindow w, COORD *wp, COORD *hp);
extern void X11menuitem(Gwindow w, char *itemname, int retval, int addflag);
extern void X11mouseinterest(Gwindow w, int event);
