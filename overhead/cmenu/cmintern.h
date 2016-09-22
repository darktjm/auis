/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

/* Useful definitions... */

#ifndef NULL
#define NULL    0
#endif
#ifndef TRUE
#define FALSE   0
#define TRUE    1
#endif

#define max(a, b) (((a) >= (b)) ? (a) : (b))
#define min(a, b) (((a) <= (b)) ? (a) : (b))

#include <cmenu.h>

/* Global menu data. One of these structures is allocated for each display.
 * That structure is shared by all cmenu structures on that display.
 */
struct cmenudata  {
    struct cmenudata *next;
    Display *dpy;
    char *def_env;
    Window menuWindow;
    XFontStruct *titleFont;
    long titleFontHeight;
    long titleFontAscent;
    long titleFontDescent;
    long xTitleOffset;
    long yTitleOffset;
    XFontStruct *selectionFont;
    long selectionFontHeight;
    long selectionFontAscent;
    long xSelectionOffset;
    long overlapWidth;
    long bottomMargin;
    long xShift;
    long yShift;
    GC whiteGC;
    GC titleBlackGC;
    GC blackGC;
    GC invertGC;
    GC grayGC;
    GC topshadowGC;
    GC bottomshadowGC;
    GC saveUnderGC;
    int clickInterval;
    int highlightUsingGray;
    int motifMenus;
    Pixmap wormIcon;
    int wormWidth;
    int wormHeight;
    int useSaveUnder;
    int overlapPct;
    char *foregroundColor;
    char *backgroundColor;
    char *topshadowColor;
    char *bottomshadowColor;
    short keyspace;
    unsigned long foregroundPixel;
    unsigned long grayforegroundPixel;
    unsigned long keysPixel;
    char *keysColor;
    XFontStruct *keysFont;
};

#define HORIZONTALMARGIN 10

/* A single "item" of a menu pane. This is a line of text that you can select
 * with the mouse.
 */
struct selection {
    struct selection *next; /* All selections on a pane are on a doubly linked list. */
    const char *label;            /* Text of item. */
    short labelWidth;        /* Dimensions of above label. Used for calculating pane size efficiently. */
    short labelLength;       /* This one can probably be eliminated... */
    long data;              /* Data to return if this item is selected. Should be void *. */
    short priority;         /* For sorting. */
    short groupPriority;    /* For finding out where spaces go. */
    const char *keys;		    /* the keys equivalent to the menu selection */
    short	keysWidth;	    /* the width of the key string. */
    char active;            /* TRUE if this item is selectable. Inactive items appear grayed out. */
};

/* A single pane or card of a menu stack. */
struct pane {
    struct pane *next;          /* All panes on a cmenu are on a doubly linked list. */
    const char *label;                /* Card title. */
    int labelWidth;            /* Dimensions of above title. Used for efficient calculation of stack size. */
    int maxSelectionWidth;      /* Maximum of selection widths from cmSelect data structures for this pane. */
    struct selection *selections;    /* Linked list of selections on this pane. */
    int numberOfSelections;     /* Number of selections on s_list. */
    short priority;             /* This card's priority in the stack. */
    short maxKeysWidth;		/* Maximum of keys widths. */
    char active;                /* TRUE if items on card a selectable. Probably can nuke this... */
};

/* Actual menu data structure. Corresponds to a complete stack of cards. */
struct cmenu  {
    struct cmenudata *gMenuData;    /* Pointer to shared global data. */
    struct pane *panes;             /* Linked list of cards. */
    int numberOfPanes;              /* Number of panes in p_list. */
    int wormPane;                   /* Pane number of item last selected. -1 if none valid. */
    int wormSelection;              /* Selection number of item last selected. -1 if none valid. */
    cmenu_FreeFunction freeFunction;        /* Function to call on selection data when selection is freed. */
};

extern NO_DLL_EXPORT struct pane *PaneNumToPtr(struct cmenu *menu, int paneNum);
extern NO_DLL_EXPORT int PanePtrToNum(struct cmenu *menu, struct pane *panePtr);
extern NO_DLL_EXPORT struct selection *SelectionNumToPtr(struct cmenu *menu, struct pane *panePtr, int selectionNum);
extern NO_DLL_EXPORT int SelectionPtrToNum(struct cmenu *menu, struct pane *panePtr, struct selection *selectionPtr);

/* Wired in constants. */

/* Maximum time for a double click in milliseconds. */
#define cmenu_DefaultClickTime      250
#define cmenu_DefaultTitleFont      "andysans12b"
#define cmenu_DefaultSelectionFont  "andysans12b"
#define	cmenu_DefaultKeysFont	    "andysans12bi"
