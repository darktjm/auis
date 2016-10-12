#ifndef _scroll_H_
#define _scroll_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

/** \addtogroup libsupviews
 * @{ */
#include <point.h>

#define scroll_VERSION 1

/* The different locations scrollbars can appear in. For more than one scrollbar, or the values of the single underscore constants together. */
#define scroll_SIDES 4
#define scroll__LEFT 0
#define scroll__RIGHT 1
#define scroll__TOP 2
#define scroll__BOTTOM 3
#define scroll_LEFT (1<<scroll__LEFT)
#define scroll_RIGHT (1<<scroll__RIGHT)
#define scroll_TOP (1<<scroll__TOP)
#define scroll_BOTTOM (1<<scroll__BOTTOM)

#include <scrolling.H>


#define scroll_REPEATING (-1)
#define scroll_THUMBING (-2)
#define scroll_MAYBETHUMBING (-3)
#define scroll_NOTHING (0)

/* The endzone codes */
#define scroll_TOPENDZONE 1
#define scroll_BOTTOMENDZONE 2
#define scroll_MOTIFTOPENDZONE (-1)
#define scroll_MOTIFBOTTOMENDZONE (-2)

/* constants for the motif behavior */
#define scroll_NOWHERE 0
#define scroll_UP 1
#define scroll_DOWN 2


struct scrollbar {
    int endzones;		/* TRUE iff the endzones are displayed. */
    struct range total, seen, dot;
};

struct scrollstate {
    int location;
    struct scrollbar bar[scroll_TYPES];
};

#include <view.H>

enum scroll_Regions {
    scroll_Elevator,
    scroll_Dot,
    scroll_LeftEnd,
    scroll_RightEnd,
    scroll_UpperEnd,
    scroll_BottomEnd,
    NUM_SCROLLREGIONS
};

class scroll : public  view {
public:
	virtual ATKregistryEntry *ATKregistry();
  
     void FullUpdate(enum view::UpdateType type, long left, long top, long width, long right);
     void Update();
     class view * Hit(enum view::MouseAction action, long x, long y, long numberOfClicks)  ;
     void WantUpdate(class view *requestor);
     void LinkTree(class view *parent);
     void UnlinkNotification(class view *unlinkedTree);
     boolean RecSearch(class search *pat, boolean toplevel);
     boolean RecSrchResume(class search *pat);
     boolean RecSrchReplace(class dataobject *text, long pos, long len);
     void RecSrchExpose(const struct rectangle &logical, struct rectangle &hit);
     void PrintPSDoc(FILE *outfile, long pagew, long pageh);
     void *GetPSPrintInterface(const char *printtype);
     void DesiredPrintSize(long width, long height, enum view::DSpass pass, long *desiredwidth, long *desiredheight);
     void PrintPSRect(FILE *outfile, long logwidth, long logheight, struct rectangle *visrect);
    void GetPrintOrigin(long w, long h, long *xoff, long *yoff);
    view::DSattributes DesiredSize(long width, long height, enum view::DSpass pass, long * desired_width, long * desired_height);
    void GetOrigin(long width, long height, long * originX, long * originY);

  
    /* A ``location'' is a bitmask of scroll_LEFT, etc. describing the places for the scrollbars to appear. The location returned by GetLocation is the desired location, while GetCurrentLocation returns the real condititions based on the size of the region were in. */
    virtual void SetLocation(int location);
    virtual int GetLocation()  ;
    virtual int GetCurrentLocation()  ;

    /* The parameters consist of:
         endzone: The minimum number of pixles the scrollbar must have to display the endzones.
         bar: The minimum number of pixles to try to put a scrollbar in.
         without: The minimum size for the elevator given that the scrollbar is currently without endzones.
         with: The minimum size of the elevator when the scrollbar is displayed with endzones. */
    virtual void SetParameters(long endzone, long bar, int without, int with);
    virtual void GetParameters(long *endzone, long *bar, int *without, int *with);

    virtual void SetWidth(long newWidth);
    virtual long GetWidth()  ;
    virtual void SetDotWidth(long newWidth);
    virtual long GetDotWidth()  ;
    virtual void SetEndZoneLength(long newHeight);
    virtual long GetEndZoneLength()  ;
    virtual void SetEndToBarSpace(long space);
    virtual long GetEndToBarSpace()  ;

    /* The ``child'' is the view directly under the scrollbars, while the ``scrollee'' is the view being scrolled. Normally, both of these are the same, and set with the SetView method. SetChild and SetScrollee can be used to change them individually. */
    virtual void SetView(class view *view);
    virtual void SetChild(class view *child);
    virtual void SetScrollee(class view *scrollee);
    virtual class view * GetChild()  ;
    virtual class view * GetScrollee()  ;
    
    virtual void SetElevatorWidth(long newWidth);
    virtual long GetElevatorWidth()  ;
    virtual void SetWindowPadding(long newPaddig);
    virtual long GetWindowPadding()  ;
    virtual void SetViewPadding(long newPadding);
    virtual long GetViewPadding()  ;
    
  
    
	static boolean InitializeClass();


    /* Allocates a new set of scrollbars, and sets the view and location */
    static class scroll * Create(class view *view, int location)  ;
    static class scroll * CreateScroller(class ScrollInterface *s, class view *view, int location, const char *scrollClass=NULL);
    static class scroll * CreateScroller(class view *view, int location, const char *scrollClass=NULL);
    static class scroll *CreateCustomScroller(class view *view, int location, const char *scrollClass=NULL);
    
	scroll();
    
	virtual ~scroll();

	inline void SetInterface(class ScrollInterface *s) {
	    if(!customsi) delete si;
	    si=s;
	    customsi=FALSE;
	}
	inline class ScrollInterface *Interface() { return si; }

	void SetRegion(enum scroll_Regions sr, class ScrollRegion *r);
	

	inline class ScrollRegion *Region(enum scroll_Regions sr) {
	    return regions[sr];
	}

	inline void SetCustomInterface(class ScrollInterface *s) {
	    if(!customsi) delete si;
	    si=s;
	    customsi=TRUE;
	}
	void SetStandardRegions(int loc);
    class view *child;
    class view *scrollee;
    struct scrollstate current, desired;/* The state currently displayed and the state to display on the next update */

    class updatelist *updatelistp;	/* The list of all decendents that want updates. */

/* Excuse the structure packing. Efficiency never hurts... */
    short endzone_threshold;		/* if height < this val, endzones disapear. */
    short bar_threshold;		/* if height < this val, bar disapears. */
    short min_elevator[2];		/* min size for elevator w/o and w/ endzones. */

    short endzoneLength;		/* length of the endzone */
    short endbarSpace;			/* space between endzeone and rest of scroll bar */
    short buttonSpace;			/* space between button and bar */
    short barWidth;			/* Width of the scroll bar */
    short dotWidth;			/* Width of Dot indicator */
    short elevatorWidth;		/* Width of the elevator */
    short windowPadding;		/* Padding between window border and scrollbar or view */
    short viewPadding;			/* Padding between view  and scrollbars */
    char pending_update;		/* TRUE iff we have an update that is pending */
    char ideal_location;		/* The ideal location(s) for the bars */
    char force_full_update;             /* TRUE if we must do a full update. */
    short side;
    int hitcoord;
    long seenLength;		/* Used for thumbing to get handle end of the scroll bar */

    class cursor *cursor;
    class event *scrollEvent;
    struct sbutton_prefs *prefs, *boxprefs, *matteprefs, *buttonprefs, *elevatorprefs, *dotprefs;
    struct rectangle childrect;
    struct rectangle fullinterior;
    struct rectangle interiors[scroll_SIDES];
    struct rectangle topbutton[scroll_SIDES];
    struct rectangle botbutton[scroll_SIDES];
    boolean lastbuttonstate;
    long lastwidth, lastheight;
    enum view::MouseAction lastaction;
    double barbackground[3];
    double mattebackground[3];
    int mousestate; /* one of scroll_TOPENDZONE, scroll_BOTTOMENDZONE, scroll_REPEATING, scroll_THUMBING, scroll_MAYBETHUMBING, scroll_NOTHING */
    long startScrollTime;
    long minContScrollTime;
    long maxContScrollTime;
    long endzonereptime;
    boolean adjustScroll;
    boolean thumbScroll;
    int dir; /* one of scroll_NOWHERE, scroll_UP or scroll_DOWN */
    boolean emulation;
    boolean drawborder;
    boolean prefsready;
    class cursor *barcursor[scroll_TYPES];
    class ScrollInterface *si;
    class ScrollRegion *regions[NUM_SCROLLREGIONS];
    boolean custom;
    boolean force_get_interface;
    const struct scrollfns *fns[scroll_TYPES];
    boolean customsi;
};
/** @} */
#endif /* _scroll_H_ */
