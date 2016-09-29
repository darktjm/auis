#ifndef _textview_H_
#define _textview_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

/** \addtogroup libtext
 * @{ */
#include <mark.H>
#include <menulist.H>
#include <keystate.H>
#include <environment.H>
#include <style.H>
#include <stylesheet.H>
#include <viewref.H>
#include <view.H>
#include <rectlist.H>

#define textview_VERSION 1

class textview;

enum textview_MovementUnits  {
    textview_MoveByPixels,
    textview_MoveByLines,
    textview_MoveByPseudoLines
};

enum textview_ScrollDirection  {
    textview_NoScroll,
    textview_ScrollForward,
    textview_ScrollBackward,
    textview_MultipleScroll
};

enum textview_LineRedrawType {
    textview_FullLineRedraw,
    textview_GetPosition,
    textview_GetCoordinate,
    textview_GetHeight,
    textview_MoveView,
    textview_PartialLineRedraw
};

enum textview_ExtendDirection {
    textview_ExtendLeft,
    textview_ExtendRight
};

struct linedesc  {
    short y;				/* last position drawn */
    short height;			/* height used during last draw */
    short textheight;                   /* textheight used during last draw */
    short xMax;				/* farthest printed */
    short nChars;			/* length of the line in chars */
    boolean containsView;		/* true if this line contains a view */
    class mark *data;			/* the range of the text storing the data */
    int	  nPixels;			/* Approx. length of the entire line (even if clipped) */
    boolean childwantsupdate; /* a child in this line has requested an update */
    spotcolor *spots;
};

struct InsertStack {
    struct InsertStack *next;
    char *name;
};

#include <bpv.H>
#include <fnotev.H>
#include <headrtv.H>
#include <texttagv.H>
#include <textrefv.H>

/* structures used by PS printing routines of printtype "text" */

#define textview_coltag_NewPage   (1) /* this will be the first column on a new physical page (this tag is assumed for the first column in the layout.) */
#define textview_coltag_FootNotes (2) /* footnotes can go in this column */
#define textview_coltag_PageBreak (4) /* a "page-break" will go up to the beginning of a column with this tag (and there had better be one. If not, Bad Things(tm) will occur.) */

struct textps_layout_block {
    long x1, x2;
    long height;
};
/* measurements are in PS units */
struct textps_layout_column {
    short coltags; /* OR of textview_coltag_* values */
    long x, y; /* position of top left corner of this column on the page */
    long w, h; /* width and height of column. w must be the union of the part widths, and h the sum of the part heights. */
    int numparts; /* may be zero */
    struct textps_layout_block *parts;
    class view *background; /* view to draw in background with PrintPSRect */
};
struct textps_layout_plan {
    long planid; /* id number of this plan. two textps_layout_plan structures must be identical if (but not only if) their planids are equal. */
    long numcols;
    struct textps_layout_column *cols;
    long repeat; /* if 0, the page layout ends with the last column given. if positive N, extend it by repeating the last N columns from the given list. */

    textps_layout_plan();
    ~textps_layout_plan();
};

enum textview_insettype {
    textview_StitchText, /* lay out a given text object in-line */
    textview_StitchString, /* lay out a given char *string in-line */
    textview_Layout, /* supply a new page (or several) of layout plan */
    textview_NewPage, /* new page */
    textview_Footnote, /* add footnote */
    textview_Header, /* header/footer change */
    textview_TextTag, /* text tag */
    textview_TextRef /* reference to text tag */
};
typedef char *(*textview_getstring_fptr)(void *rock, class style **);
struct textview_getstring {
    textview_getstring_fptr func; /* function returning ptr to NULL-terminated string, and also an optional style to surround it with. The string that's returned should be freshly malloc()ed, because the caller will free() it. */
    void *rock;
};
typedef struct textps_layout_plan *(*textview_getlayout_fptr)(void *rock, long pagewidth, long pageheight);
struct textview_getlayout {
    textview_getlayout_fptr func; /* function returning ptr to a layout plan, given the page size in PS units. The plan will be deleted by the caller. */
    void *rock;
};
/* this is the structure returned by GetPSPrintInterface("text") */
struct textview_insetdata {
    enum textview_insettype type;
    union {
	class textview *StitchText; /* ptr to textview */
	textview_getstring StitchString; 
	textview_getlayout Layout;
	/* for other types, the view will be queried for details */
	class bpv *NewPage;
	class fnotev *Footnote;
	class headrtv *Header;
	class texttagv *TextTag; 
	class textrefv *TextRef;
    } u;
};

/* Line number callback types. */
typedef void (*textview_drawCB)(textview *tv, mark *line, struct rectangle *draw_rect, void *rock);
typedef void (*textview_hitCB)(textview *tv, mark *line, enum view_MouseAction action, long x, long y, long numberOfClicks, void *rock);

#define textview_UNKNOWNVIEW 1
#define textview_REMOVEVIEW 0L

/* Menu mask bits. */
#define textview_NoMenus -1         /* Used as a flag indicating no menus are posted. Minor hack. */
#define textview_SelectionMenus 1   /* Menus to be posted when there is a selection region. */
#define textview_NoSelectionMenus 2 /* Menus to be posted only when there is no
                                     * selection. Handles the bogus case of the
                                     * paste item which should show up all the
                                     * time.
                                     */
#define textview_NotReadOnlyMenus 4 /* Menus to be posted for writable textviews. */

/* Key (as in third argument to menulist_ChainML) under which style menus are
 * chained to normal menulist.
 */
#define textview_StyleMenusKey 1
#include <scroll.H>
#include <scrolling.H>

class textviewInterface : public ScrollInterface {
  public:
    textviewInterface(class textview *tva) : tv(tva) {
    }	
    ~textviewInterface() {
    }
    void Absolute(long totalx, long x, long totaly, long y);
    void ScreenDelta(long dx, long dy);
    void Shift(scroll_Direction dir);
    void Extreme(scroll_Direction dir);
    void UpdateRegions(class scroll &scrollview);
    class textview *tv;
};

class aaction;

class textview : public  view {
    public:
      virtual ATKregistryEntry *ATKregistry();
      textview();
      virtual ~textview();
      static boolean InitializeClass();

      static long GetUniquePlanId();  /* supplies IDs for PS printing layouts. */  

      void WantUpdate(class view *requestor);
      void SetDataObject(class dataobject *dataObject);
      void FullUpdate(enum view_UpdateType type, long left, long top, long width, long height);
      void Update();
      class view * Hit (enum view_MouseAction action, long x, long y, long numberOfClicks)  ;
      void ReceiveInputFocus();
      void LoseInputFocus();
      class view * GetApplicationLayer()  ;
      void DeleteApplicationLayer(class view *scrollbar);
      const void * GetInterface(const char *interfaceName)  ;
      void Print(FILE *file, const char *processor, const char *finalFormat, boolean topLevel);
      void WantNewSize(class view *requestor);
      void ObservedChanged (class observable *changed, long value);
      void LinkTree(class view *parent);
      view_DSattributes DesiredSize(long width, long height, enum view_DSpass pass, long *dWidth, long *dheight)  ;
      void GetOrigin(long width, long height, long *originX, long *originY);
      void InitChildren();
      boolean CanView(const char *TypeName)  ;
      long WriteSelection(FILE *out)  ;
      void LoseSelectionOwnership();
      void ExposeChild(class view *v);
      boolean RecSearch(struct SearchPattern *pat, boolean toplevel);
      boolean RecSrchResume(struct SearchPattern *pat);
      boolean RecSrchReplace(class dataobject *text, long pos, long len);
      void RecSrchExpose(const struct rectangle &logical, struct rectangle &hit);
      void PrintPSDoc(FILE *outfile, long pagew, long pageh);
      void *GetPSPrintInterface(const char *printtype);
      void PrintPSRect(FILE *outfile, long logwidth, long logheight, struct rectangle *visrect);

      struct view_printoptlist *PrintOptions();
      long GetPrintOption(class atom *popt);

      virtual void SetDotPosition(long newPosition);
      virtual void SetDotLength(long newLength);
      virtual long GetDotPosition()  ;
      virtual long GetDotLength()  ;
      virtual long GetTopPosition()  ;
      virtual void SetTopPosition(long newTopPosition);
      virtual void SetBorder(long xBorder, long yBorder);
      virtual void SetEmbeddedBorder(long xBorder, long yBorder);
      virtual long CollapseDot()  ;
      virtual void GetClickPosition(long position, long numberOfClicks, enum view_MouseAction action, long startLeft, long startRight, long *leftPos, long *rightPos);
      virtual boolean Visible(long pos)  ;
      virtual long Locate(long x, long y, class view **view)  ;
      virtual void GetTextSize(long *width, long *height);
      virtual long MoveBack(long pos, long units, enum textview_MovementUnits type, long *distMoved, long *linesAdded)  ;
      virtual long MoveForward(long pos, long units, enum textview_MovementUnits type, long *distMoved, long *linesAdded)  ;
      virtual void FrameDot(long pos);
      virtual long FindLineNumber(long pos)  ;
      virtual long LineRedraw(enum textview_LineRedrawType type, class mark *currentLine, long x, long y, long xSize, long ySize, long search, boolean *cont,long *textheight, struct formattinginfo *info, spotcolor **spots)  ;
      virtual long LineRedraw(enum textview_LineRedrawType type, class mark *currentLine, long x, long y, long xSize, long ySize, long search, boolean *cont,long *textheight, struct formattinginfo *info)  ;
      virtual void LookCmd(int look);
      virtual void SetDefaultStyle(class style *styelptr);
      virtual class style * GetDefaultStyle()  ;
      virtual void ViewMove(struct linedesc *currentLine,long movement);
      virtual class environment * GetStyleInformation(struct text_statevector *sv, long pos, long *length)  ;
      virtual class environment * GetEnclosedStyleInformation(long pos, long *length)  ;
      virtual void ReleaseStyleInformation(class environment *env);

      virtual void PrepareDeletion(long pos, long len);
      virtual void FinishDeletion();
      virtual void DeleteCharacters(long pos, long len);

      virtual void PrepareInsertion(boolean insertingNewLine);
      virtual void FinishInsertion();

      virtual class environment * GetEnclosingEnvironment(long pos)  ;
      virtual class environment * GetInsertEnvironment(long pos)  ;

      virtual void ClearInsertStack();
      virtual void PopInsertStack();
      virtual void AddStyleToInsertStack(char *styleName);

      virtual void PlainInsertEnvironment();
      virtual void UpInsertEnvironment();
      virtual void DownInsertEnvironment();
      virtual void LeftInsertEnvironment();
      virtual void RightInsertEnvironment();

      virtual void DoCopyRegion(long pos, long len, boolean append, boolean copyAsText);

      virtual void ToggleVIMode();	/* toggle input/command modes for vi editor FAS */
      virtual void ToggleEditor();	/* toggle between editors (vi/emacs) FAS */

      virtual void SetTopOffTop(long newTopPosition, long pixelsOffTop);

      void WantExposure(class view *requestor, struct rectangle *childrect);

      class ScrollInterface *GetScrollInterface();
      
      virtual void LineNumberDisplay(boolean on_or_off);
      inline void SetLineNumberDrawCallback(textview_drawCB func, void *rock)
                     { para_draw_callback = func; para_draw_rock = rock; };
      inline void SetLineNumberHitCallback(textview_hitCB func, void *rock)
                     { para_hit_callback = func; para_hit_rock = rock; };

      inline int GetEditor() { return (this->editor); }
      inline int GetVIMode() { return (this->viMode); }
      inline long EncodePosition(long pos)  { return ((pos) << 7); }
      inline long DecodePosition(long pos)  { return ((pos) >> 7); }

      inline boolean GetAlwaysDrawDot() { return alwaysDrawDot; }
      inline void SetAlwaysDrawDot(boolean val) { alwaysDrawDot = val; }
      inline void SetDesiredRowsAndColumns(unsigned char r, unsigned char c) {
          rows=r;
          columns=c;
      }
      inline void GetDesiredRowsAndColumns(unsigned char *r, unsigned char *c) {
          *r=rows;
          *c=columns;
      }
      short aLines;				/* allocated lines */
      short nLines;				/* number of lines currently shown */
      struct linedesc *lines;			/* the view image */
      short bx;				/* the border size - when using application layer*/
      short by;
      short ebx;				/* the border size when embedded */
      short eby;
      boolean hasApplicationLayer;
      boolean hasInputFocus;			
      boolean zapRest;				/* true if hit file's eof in redisplay */
      boolean force;				/* should we force a full redraw? */
      class mark *dot;
      class mark *top;
      class mark *frameDot;
      class mark *recsearchpos;		/* position of last RecSearch */
      boolean recsearchvalid;		/* was last RecSearch successful? */
      class view *recsearchchild;		/* child in which last RecSearch found something */
      boolean alwaysDrawDot;			/* TRUE if dot shown regardless of input focus */
      class keystate *keystate;
      class menulist *styleMenus, *menus;	/* style menus are only present when dot is non-zero length. */
      short clearFrom;				/* y last cleared from */
      short csxPos;				/* old cursor x position */
      short csyPos;				/* old cursor y posn (-1 -> never shown) */
      short cshPos;				/* old cursor line height */
      short csbPos;				/* old cursor linebelow */
      short cexPos;				/* old cursor x position */
      short ceyPos;				/* old cursor y posn (-1 -> never shown) */
      short cehPos;				/* old cursor line height */
      short cebPos;				/* old cursor linebelow */
      enum textview_ScrollDirection scroll;
      int scrollLine;				/* line to use for scrolling. */
      int scrollDist;				/* distance to scroll */
      boolean needUpdate;
      int movePosition;
      long lastStyleMenuVersion;			/* Version number of stylesheet from which menus were last posted. */
      class mark *atMarker;
      class style *defaultStyle;			/* style from preferences */
      class viewref *currentViewreference; /* Used for creating new views on existing dataobjects */
      long displayLength;
      class environment *displayEnvironment;
      long displayEnvironmentPosition;
      long tabWidth;
      boolean exposeStyles;
      boolean showColorStyles;

      char *cur_bgcolor;			/* Current text background color. */
      char *cur_fgcolor;			/* Current text foreground color. */

      long predrawnY,predrawnX; 
      class mark	*predrawn;	/* text preceding the dot that may avoid redrawing */
      class mark	*prepredrawn;	/* text preceeding predrawn that may avoid redrawing,
       used if the predrawn mark is modified */

      int editor;					/* 0->emacs, 1->vi */
      int viMode;					/* 0->command, 1->input */
      class keystate *emacsKeystate;
      class keystate *viInputModeKeystate;
      class keystate *viCommandModeKeystate;

      class environment *insertEnvironment;
      class mark *insertEnvMark;
      struct InsertStack *insertStack;

      /* The next two params are to get tabs looking correct. Good values are
       * Andy12 = Adobe14 -> Mul = 14, Div = 12     -njw.
       */
      int ScreenScaleMul;
      int ScreenScaleDiv;

      /* These three are used to keep track of insets that are partially
       * scrolled off the top of the view. -dba.
       */
      int pixelsShownOffTop;
      int pixelsReadyToBeOffTop;
      int pixelsComingOffTop;
      /* This parameter determines if we draw a line through format
       * notes when we display them on the screen. */
      int LineThruFormatNotes;

      /* Auto scroll at the end of a page will scroll by this amount (default 67%) */
      long scroll_percent;

      int hz_offset;		/* Offset in pixels for horizontal scrolling */

      /* stuff for PS printing */
      struct textps_layout *cachedlayout;
      struct textps_layout_plan *cachedlayoutplan;
      long cachedplanwid, cachedplanhgt; /* if nonzero, the cachedlayoutplan is a rectangle with these dimensions. */

      /* This is used to keep track of which direction the user is highlighting
       * text...mainly for textview-forward/backward-extend procs. */
      enum textview_ExtendDirection extendDirection;

      /* Line (paragraph) number display info */
      boolean show_para_display;		/* Defaults to FALSE */
      boolean three_d_display;			/* Show 3D (TRUE) or plain. */
      int para_width_default;			/* Width (pixels) of line display */
      int para_width;				/* Current width (pixels) of line display */
      struct fontdesc *para_font;		/* Font for line numbers */
      long para_last_top_pos;
      long para_last_top_linenum;
      textview_hitCB para_hit_callback;
      void *para_hit_rock;
      textview_drawCB para_draw_callback;
      void *para_draw_rock;
      const char *para_bgcolor;
      const char *para_fgcolor;
      unsigned char rows, columns;
      // hit callback is called as:
      // (*hitcallback)(text, textview, action, pos, view, x, y)
      // if it returns no value or zero no other processing
      // of the hit will be done.
      aaction *hitcallback;
      inline void SetHitCallback(aaction *hit) {
          hitcallback=hit;
      }
      inline aaction *HitCallback() {
          return hitcallback;
      }
      // If TRUE only '*'s will ever be displayed.  But editing will
      // work normally, and the appropriate characters will appear
      // in the actual text.
      boolean password;
      inline void SetPassword(boolean pass) {
          password=pass;
      }
      inline boolean Password() {
          return password;
      }
      /* For experimentation (normally unused and NULL). */
      void *extension;
};

#define EMACS	0
#define VI	1

/* VI mode values  FAS */
#define	COMMAND	0
#define INPUT	1
/** @} */
#endif /* _textview_H_ */
