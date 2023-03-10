#ifndef _oscroll_H_
#define _oscroll_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

/** \addtogroup libsupviews
 * @{ */
#include <point.h>

/* the arrows in the arrow array */
#define ARROW_UP	0
#define ARROW_LEFT	1

struct arrow {
    struct point pa[6];
};


#include <scroll.H>

class oscroll : public  scroll {
public:
	TRACED_CLASS(oscroll);

  
     void FullUpdate(enum view::UpdateType type, long left, long top, long width, long right);
     void Update();
     class view * Hit(enum view::MouseAction action, long x, long y, long numberOfClicks)  ;
     void WantUpdate(class view *requestor);
     void LinkTree(class view *parent);
     void UnlinkNotification(class view *unlinkedTree);

    /* A ``location'' is a bitmask of scroll_LEFT, etc. describing the places for the scrollbars to appear. The location returned by GetLocation is the desired location, while GetCurrentLocation returns the real condititions based on the size of the region were in. */
     void SetLocation(int location);
     int GetLocation()  ;
     int GetCurrentLocation()  ;

    /* The parameters consist of:
         endzone: The minimum number of pixels the scrollbar must have to display the endzones.
         bar: The minimum number of pixles to try to put a scrollbar in.
         without: The minimum size for the elevator given that the scrollbar is currently without endzones.
         with: The minimum size of the elevator when the scrollbar is displayed with endzones. */
     void SetParameters(long endzone, long bar, int without, int with);
     void GetParameters(long *endzone, long *bar, int *without, int *with);

     void SetWidth(long newWidth);
     long GetWidth()  ;
     void SetDotWidth(long newWidth);
     long GetDotWidth()  ;
     void SetEndZoneLength(long newHeight);
     long GetEndZoneLength()  ;
     void SetEndToBarSpace(long space);
     long GetEndToBarSpace()  ;

    /* The ``child'' is the view directly under the scrollbars, while the ``scrollee'' is the view being scrolled. Normally, both of these are the same, and set with the SetView method. SetChild and SetScrollee can be used to change them individually. */
     void SetView(class view *view);
     void SetChild(class view *child);
     void SetScrollee(class view *scrollee);
     class view * GetChild()  ;
     class view * GetScrollee()  ;

  
    
	static boolean InitializeClass();


    /* Allocates a new set of scrollbars, and sets the view and location */
    static class oscroll * Create(class view *view, int location)  ;
    
	oscroll();
    

  
    class view *child;
    class view *scrollee;
    struct scrollstate current, desired;/* The state currently displayed and the state to display on the next update */

    const struct scrollfns *fns[scroll_TYPES];/* Pointers to the interface structures. */

    class updatelist *updatelistp;	/* The list of all decendents that want updates. */

    long left, top, width, height;	/* The coords of the child inset rel to me. Not in the state structure because they are calculated from things in the state structure. */

/* Excuse the structure packing. Efficiency never hurts... */
    short endzone_threshold;		/* if height < this val, endzones disapear. */
    short bar_threshold;		/* if height < this val, bar disapears. */
    short min_elevator[2];		/* min size for elevator w/o and w/ endzones. */

    short endzoneLength;		/* length of the endzone */
    short endbarSpace;			/* space between endzeone and rest of scroll bar */
    short barWidth;			/* Width of the scroll bar */
    short dotWidth;			/* Width of Dot indicator */

    char pending_update;		/* TRUE iff we have an update that is pending */
    char ideal_location;		/* The ideal location(s) for the bars */
    char thumbing;
    char button;
    char force_full_update;             /* TRUE if we must do a full update. */
    char force_get_interface;           /* TRUE if the scrollee has changed since the last time we got the scroll functions. */
    short side;
    int hitcoord;
    int hitposn;
    long seenLength;		/* Used for thumbing to get handle end of the scroll bar */

    class cursor *curse[scroll_SIDES];
    
    class graphic *cornerFill;
    class graphic *endzoneFill;
    class graphic *barFill;
    class graphic *elevatorFill;
    class graphic *whiteFill;
    class event *scrollEvent;
    int direction;		/* used for the endzone arrows */
    /* array[2] of array[6] of points for a left-pointing and an up-pointing
       arrow.   w.r.t. a (0,0) origin at the lower left of a bouding box.
       Basically, offsets to be used when drawing the arrows */
    struct arrow arrows[2];
};

/* Fake a get view routine. */
#define oscroll_GetView scroll_GetScrollee

/* package of routines for doing 3-D boxes and stuff like that */

#define TD_DEPTH 	2	/* width of 3d border */
#define TD_GAP		7	/* space from edge of view to 3d border */
#define TD_BORDER	TD_DEPTH/* distance from edge of view's border to */
				/* edge of view */
/* 8, 12, 16 */

#define TD_FGPATVAL	8	/* recommended foreground pattern */
#define TD_BACKPATVAL	4	/* recommended nominal background pattern */
#define TD_BGPATVAL	0	/* recommended background pattern */

#define TD_FGPAT(self)	((self)->GrayPattern(TD_FGPATVAL,16))
#define TD_BGPAT(self)	((self)->GrayPattern(TD_BGPATVAL,16))
#define TD_BACKPAT(self) ((self)->GrayPattern(TD_BACKPATVAL,16))
/** @} */
#endif /* _oscroll_H_ */
