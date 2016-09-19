ATK_INTER
#ifndef _scroller_H_
#define _scroller_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

#include <point.h>

#define scroller_VERSION 1

/* The different locations scrollerbars can appear in. For more than one scrollerbar, or the values of the single underscore constants together. */
#define scroller_SIDES 4
#define scroller__LEFT 0
#define scroller__RIGHT 1
#define scroller__TOP 2
#define scroller__BOTTOM 3
#define scroller_LEFT (1<<scroller__LEFT)
#define scroller_RIGHT (1<<scroller__RIGHT)
#define scroller_TOP (1<<scroller__TOP)
#define scroller_BOTTOM (1<<scroller__BOTTOM)

/* The different types of scrollerbars supported. Currently only vertical and horizontal */
#define scroller_TYPES 2
#define scroller_VERT 0
#define scroller_HORIZ 1
#define scroller_BOTH sroller_TYPES

#include <scrolling.H>


#define scroller_REPEATING (-1)
#define scroller_THUMBING (-2)
#define scroller_MAYBETHUMBING (-3)
#define scroller_NOTHING (0)

/* The endzone codes */
#define scroller_TOPENDZONE 1
#define scroller_BOTTOMENDZONE 2
#define scroller_MOTIFTOPENDZONE (-1)
#define scroller_MOTIFBOTTOMENDZONE (-2)

/* constants for the motif behavior */
#define scroller_NOWHERE 0
#define scroller_UP 1
#define scroller_DOWN 2


struct scrollerbar {
    int endzones;		/* TRUE iff the endzones are displayed. */
};

struct scrollerstate {
    int location;
    struct scrollerbar bar[scroller_TYPES];
};

#include <view.H>

enum scroller_Regions {
    scroller_Elevator,
    scroller_Dot,
    scroller_LeftEnd,
    scroller_RightEnd,
    scroller_UpperEnd,
    scroller_BottomEnd,
    NUM_SCROLLERREGIONS
};

class scroller : public  view {
public:
	virtual ATKregistryEntry *ATKregistry();
  
     void FullUpdate(enum view_UpdateType type, long left, long top, long width, long right);
     void Update();
     class view * Hit(enum view_MouseAction action, long x, long y, long numberOfClicks)  ;
     void WantUpdate(class view *requestor);
     void LinkTree(class view *parent);
     void UnlinkNotification(class view *unlinkedTree);
     boolean RecSearch(struct SearchPattern *pat, boolean toplevel);
     boolean RecSrchResume(struct SearchPattern *pat);
     boolean RecSrchReplace(class dataobject *text, long pos, long len);
     void RecSrchExpose(const struct rectangle &logical, struct rectangle &hit);
     void PrintPSDoc(FILE *outfile, long pagew, long pageh);
     void *GetPSPrintInterface(const char *printtype);
     void DesiredPrintSize(long width, long height, enum view_DSpass pass, long *desiredwidth, long *desiredheight);
     void PrintPSRect(FILE *outfile, long logwidth, long logheight, struct rectangle *visrect);

  
    /* A ``location'' is a bitmask of scroller_LEFT, etc. describing the places for the scrollerbars to appear. The location returned by GetLocation is the desired location, while GetCurrentLocation returns the real condititions based on the size of the region were in. */
    virtual void SetLocation(int location);
    virtual int GetLocation()  ;
    virtual int GetCurrentLocation()  ;

    /* The parameters consist of:
         endzone: The minimum number of pixles the scrollerbar must have to display the endzones.
         bar: The minimum number of pixles to try to put a scrollerbar in.
         without: The minimum size for the elevator given that the scrollerbar is currently without endzones.
         with: The minimum size of the elevator when the scrollerbar is displayed with endzones. */
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

    /* The ``child'' is the view directly under the scrollerbars, while the ``scrolleree'' is the view being scrollered. Normally, both of these are the same, and set with the SetView method. SetChild and SetScrolleree can be used to change them individually. */
    virtual void SetView(class view *view);
    virtual void SetChild(class view *child);
    virtual void SetScrolleree(class view *scrolleree);
    virtual class view * GetChild()  ;
    virtual class view * GetScrolleree()  ;
    
    virtual void SetElevatorWidth(long newWidth);
    virtual long GetElevatorWidth()  ;
    virtual void SetWindowPadding(long newPaddig);
    virtual long GetWindowPadding()  ;
    virtual void SetViewPadding(long newPadding);
    virtual long GetViewPadding()  ;
    
  
    
	static boolean InitializeClass();


    /* Allocates a new set of scrollerbars, and sets the view and location */
    static class scroller * Create(class view *view, int location)  ;
    static class scroller * CreateScroller(class ScrollInterface *s, class view *view, int location, char *scrollerClass=NULL);
    static class scroller * CreateScroller(class view *view, int location, char *scrollerClass=NULL);
    static class scroller *CreateCustomScroller(class view *view, int location, char *scrollerClass=NULL);
    
	scroller();
    
	virtual ~scroller();

	inline void SetInterface(class ScrollInterface *s) {
	    si=s;
	}
	inline class ScrollInterface *Interface() { return si; }

	void SetRegion(enum scroller_Regions sr, class ScrollRegion *r);
	

	inline class ScrollRegion *Region(enum scroller_Regions sr) {
	    return regions[sr];
	}
	
    class view *child;
    class view *scrolleree;
    struct scrollerstate current, desired;/* The state currently displayed and the state to display on the next update */

    class updatelist *updatelist;	/* The list of all decendents that want updates. */

/* Excuse the structure packing. Efficiency never hurts... */
    short endzone_threshold;		/* if height < this val, endzones disapear. */
    short bar_threshold;		/* if height < this val, bar disapears. */
    short min_elevator[2];		/* min size for elevator w/o and w/ endzones. */

    short endzoneLength;		/* length of the endzone */
    short endbarSpace;			/* space between endzeone and rest of scroller bar */
    short buttonSpace;			/* space between button and bar */
    short barWidth;			/* Width of the scroller bar */
    short dotWidth;			/* Width of Dot indicator */
    short elevatorWidth;		/* Width of the elevator */
    short windowPadding;		/* Padding between window border and scrollerbar or view */
    short viewPadding;			/* Padding between view  and scrollerbars */
    char pending_update;		/* TRUE iff we have an update that is pending */
    char ideal_location;		/* The ideal location(s) for the bars */
    char force_full_update;             /* TRUE if we must do a full update. */
    short side;
    int hitcoord;
    long seenLength;		/* Used for thumbing to get handle end of the scroller bar */

    class cursor *cursor;
    class event *scrollerEvent;
    struct sbutton_prefs *prefs, *boxprefs, *matteprefs, *buttonprefs, *elevatorprefs, *dotprefs;
    struct rectangle childrect;
    struct rectangle fullinterior;
    struct rectangle interiors[scroller_SIDES];
    struct rectangle topbutton[scroller_SIDES];
    struct rectangle botbutton[scroller_SIDES];
    boolean lastbuttonstate;
    long lastwidth, lastheight;
    enum view_MouseAction lastaction;
    double barbackground[3];
    double mattebackground[3];
    int mousestate; /* one of scroller_TOPENDZONE, scroller_BOTTOMENDZONE, scroller_REPEATING, scroller_THUMBING, scroller_MAYBETHUMBING, scroller_NOTHING */
    long startScrollerTime;
    long minContScrollerTime;
    long maxContScrollerTime;
    long endzonereptime;
    boolean adjustScroller;
    boolean thumbScroller;
    int dir; /* one of scroller_NOWHERE, scroller_UP or scroller_DOWN */
    boolean emulation;
    boolean drawborder;
    boolean prefsready;
    class cursor *barcursor[scroller_TYPES];
    class ScrollInterface *si;
    class ScrollRegion *regions[NUM_SCROLLERREGIONS];
    boolean custom;
};
  

#endif /* _scroller_H_ */
