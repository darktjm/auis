/* **********************************************************************o *\
 *         Copyright IBM Corporation 1990,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

#define mb_FullRedraw 0
#define mb_Update 1
#define mb_Exposed 2

struct prefs_s {
    Display *dpy;
    int newshadows;
    int ColorDisplay;
    int refcount;
    unsigned long topshadowPixel, cardtopshadowPixel;
    unsigned long bottomshadowPixel, cardbottomshadowPixel;
    unsigned long menutitlesPixel;
    unsigned long menubackgroundPixel;
    unsigned long carditemsPixel;
    unsigned long keysPixel;
    unsigned long cardbackgroundPixel;
    unsigned long grayitemPixel;
    unsigned long graytitlePixel;
    Pixmap topshadowPixmap, cardtopshadowPixmap;
    Pixmap bottomshadowPixmap, cardbottomshadowPixmap;
    Pixmap grayPixmap;
    Pixmap grayitemStipple;
    Pixmap graytitleStipple;
    XColor whiteColor, blackColor;
    XFontStruct *titlefont;
    XFontStruct *itemfont;
    XFontStruct *iconfont;
    XFontStruct *keysfont;
    int depth;
    int menubarheight;
    int vspacing, hspacing;
    int padding;
    int groupspacing;
    int itemhspace;
    unsigned long activatetime;
    unsigned int holdbutton;
    int grayPercentage, topshadowPercentage;
    struct prefs_s *next;
};

struct gcs {
    GC draw;
    GC select,selecterase;
    unsigned long topshadowPixel, bottomshadowPixel;
    Pixmap topshadowPixmap, bottomshadowPixmap;
    unsigned long grayPixel, whitePixel;
    Pixmap grayStipple;
};

typedef void (*menubar_exposefptr)(XExposeEvent *ee, long *exposedata);
typedef void (*menubar_freefptr)(void *ptr);
struct mbinit {
    Display *dpy;	    /* display the menu is on */
    int xfd;		    /* File descriptor for select (from dpy) */
    int	color;		    /* non-zero if this is a color display */
    Window client;	    /* window the menubar is for */
    Window menuw;	    /* window for the menu card */
    Window cascadew;	    /* window for first level of cascading */
    struct gcs titlegcs;    /* gcs used to draw the menubar */
    struct gcs cardgcs;	    /* gcs for the menu card */	    
    struct gcs cmcgcs;	    /* gcs for the first level of cascading */
    struct prefs_s *prefs;	    /* user configurable things */
    int x,y;
    unsigned int w,h;
    int	everdrawn;	    /* whether this menubar has ever been drawn */
    menubar_exposefptr HandleExpose;  /* function to be called with X expose events for the client window when
	a menu is up */
    long *ExposeData;	    /* Data to be passed to the HandleExpose function for the client */
    menubar_freefptr FreeItem;	    /* free a menu items data */
};

    
struct titem {
    const char *name;		/* name of this item */
    struct titem *next;	/* next item */
    const void *data;		/* data to be reported when this item is chosen */
    const char *keys;		/* the keybinding for this item */
    short y;
#define SUBMENUFLAG (1<<1)
#define ACTIVEFLAG (1<<0)
    char flags;		/* bit 0 is 1 if item is active
			   bit 1 is 1 if item is a submenu */
    char prio;		/* priority of this item */
};

struct tmenu {
    int x,y,w,h,mx,mw,ww,wh;
    const char *title;    /* title if this is a toplevel menu */
    struct titem *items;    /* linked list of the items */
    struct titem **lookup;
    struct tmenu *next;	/* next menu in the overflow list*/
    Window window;
    struct gcs gcs;
    short   iwidth;	    /* maximum width of any item name */
    short   kwidth;	    /* maximum width of any key binding */
    short   nitems;	    /* number of items in this menu */
    short   lastitem;
    short   titlelen;   /* length of the title */
    short   vert,horiz;
    unsigned short groupmask; /* groups which have items have their bit set... groups numbered 0-9 */
    char   groupcount; /* number of item groups used in this menu */
    char    prio;	    /* priority if this is a toplevel menu */
};

struct menubar {
    struct mbinit *mbi;
    const char *mainmenu,*moretitle;
    int	lastvm;  /* index of last menu title visible on the menubar (aside from more menu) */
    struct tmenu *overflow;	/* linked list of menus which don't fit */
    struct tmenu **menus;   /* array of all the toplevel menus */
    struct tmenu *lastmenu;	/* the last toplevel menu put up */
    struct tmenu *lasteventin;
    struct tmenu *moremenu;
    int	nmenus;		    /* number of top level menus */
    int	mmenus;		    /* number of pointers allocated for menus */
    int	lastmenuindex;	    /* index into menus of the top menu currently displayed */
    int resort;	    /* whether the menubar needs re-sorting */
    void (*MenuFunction)(struct menubar *mb, const void *data, const void *md); /* function to be called for a menu selection */
    char *MenuFunctionData; /* data for the MenuFunction */
    const void *data;	    /* more data */
    int morewidth;
    Bool refit;
};

typedef void (*menubar_menufptr)(struct menubar *mb, const void *data, const void *md);

typedef const char *(*GetDefaultsFunction)(Display *, const char *);
GetDefaultsFunction mb_SetGetDefault(GetDefaultsFunction  func);
void mb_SetKeys(struct menubar  *mb, const char *title, const char *item, char  *keys);
void mb_AddSelection(struct menubar  *mb,const char *title ,int  tprio,const char *item,int  iprio,int  submenu,const void *data);
void mb_SetItemStatus(struct menubar  *mb,const char *title,const char *item,int  status);
void mb_DeleteSelection(struct menubar  *mb,const char *title,const char *item);
void mb_RefitMenubar(struct menubar  *mb);
void mb_RedrawMenubar(struct menubar  *mb, int  clear);
void mb_HandleConfigure(struct mbinit  *mbi, struct menubar  *mb, long  width , long  height);
void mb_Activate(struct menubar  *mb, long  x, long  y);
void mb_KeyboardActivate(struct menubar  *mb);
struct prefs_s *mb_GetPrefsForDisplay(Display  *dpy, XColor  *fore , XColor  *back);
struct mbinit *mb_Init(Display  *dpy,XColor  *fore,XColor  *back, menubar_exposefptr expose, long  *exposedata, menubar_freefptr freeitem);
void mb_InitWindows(struct mbinit  *mbi,Window  client);
void mb_Finalize(struct mbinit  *mbi);
void mb_Destroy(struct menubar  *mb);
struct menubar *mb_Create(struct mbinit  *mbi, const char  *maintitle , const char  *moretitle, const void *data, menubar_menufptr func);
