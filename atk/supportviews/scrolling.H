ATK_INTER
/* Copyright 1994 Carnegie Mellon University All rights reserved.
  $Disclaimer: 
 * Permission to use, copy, modify, and distribute this software and its 
 * documentation for any purpose and without fee is hereby granted, provided 
 * that the above copyright notice appear in all copies and that both that 
 * copyright notice and this permission notice appear in supporting 
 * documentation, and that the name of IBM not be used in advertising or 
 * publicity pertaining to distribution of the software without specific, 
 * written prior permission. 
 *                         
 * THE COPYRIGHT HOLDERS DISCLAIM ALL WARRANTIES WITH REGARD 
 * TO THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF 
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL ANY COPYRIGHT 
 * HOLDER BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL 
 * DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, 
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE 
 * OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION 
 * WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 * 
 *  $
 */
#ifndef _scrolling_H_
#define _scrolling_H_

#include <andrewos.h>
#include <point.h>
#include <rect.h>
#include <view.H>

enum scroll_Direction {
    scroll_Up,
    scroll_Down,
    scroll_Right,
    scroll_Left,
    scroll_None
};

/* The endzone codes */
#define scroll_TOPENDZONE 1
#define scroll_BOTTOMENDZONE 2
#define scroll_MOTIFTOPENDZONE (-1)
#define scroll_MOTIFBOTTOMENDZONE (-2)

typedef void (*scroll_getinfofptr)(class view *scrollee, struct range *total, struct range *seen, struct range *dot);
typedef void (*scroll_setframefptr)(class view *scrollee, long posn, long coord, long height);
typedef void (*scroll_endzonefptr)(class view *scrollee, int typeEnd, enum view_MouseAction act);
typedef long (*scroll_whatfptr)(class view *scrollee, long coord, long height);

struct scrollfns {
    scroll_getinfofptr GetInfo;
    scroll_setframefptr SetFrame;
    scroll_endzonefptr EndZone;
    scroll_whatfptr WhatIsAt;
};

class scroll;
class ScrollInterface {
  public:
    ScrollInterface();
    
    virtual void Absolute(long totalx, long x, long totaly, long y)=0;
 /* Instructs the client to draw its data so that the given upper left corner(x/totalx*client_width,y/totaly*client_height) in the clients coordinate system is in the upper left of the view. */

    virtual void ScreenDelta(long dx, long dy)=0;
   /* Instructs the client to draw its data so that the data is shifted by dx,dy pixels from its current position. */

    virtual void Shift(scroll_Direction dir)=0;
    /* Instructs the client to shift its data by one unit in the indicated direction.  (The choice of unit is up to the client, text uses lines.) */

    virtual void Extreme(scroll_Direction dir)=0;
    /* Instructs the client to display the data at the indicated edge of its data. */

    virtual void UpdateRegions(class scroll &scrollerview)=0;
    /* Update the ScrollRegions associated with this interface and the given scrollerview. */
    
    virtual ~ScrollInterface();
};

struct range {
    long beg, end;		/* The begining and the end of the range. */
     };
     
class ScrollInterfaceClassic : public ScrollInterface {
  public:
    ScrollInterfaceClassic(class view *v);
    ScrollInterfaceClassic();
    virtual void Absolute(long totalx, long x, long totaly, long y);
    virtual void ScreenDelta(long dx, long dy);
    virtual void Shift(scroll_Direction dir);
    virtual void Extreme(scroll_Direction dir);
    virtual void UpdateRegions(class scroll &scrollerview);
    
    inline void SetScrollee(class view *v) {
	vw=v; 
	if(vw) { 
	    fns[scroll_VERT]=(const struct scrollfns *)vw->GetInterface("scroll,vertical");
	    fns[scroll_HORIZ]=(const struct scrollfns *)vw->GetInterface("scroll,horizontal");
	}
    }
    inline class view *GetScrollee() { return vw; }
    
    const struct scrollfns *fns[scroll_TYPES];
    class view *vw;
};

enum scroll_RegionType {
	scroll_Range,
	scroll_Endzone,
	scroll_Rectangle
}; /* Each ScrollRegion can have its extent computed in different ways based on its usage.  The scroll_Range and scroll_Endzone types are implemented in ScrollRegion::ComputeRect.  RegionType scroll_Rectangle should be used by subclasses if they have their own means of computing where the region should appear.
*/

typedef scroll_Direction scroll_Zone;

union scroll_RegionData {
	scroll_Zone zone;
	struct scroll_range {
		long xrange, xstart, xend;
		long yrange, ystart,yend;
	} range;
}; /* The data used by ComputeRect to implement the scroll_Range and scroll_EndZone RegionTypes. */

struct sbutton_prefs;

class ScrollRegion {
  public:
    ScrollRegion(scroll_RegionType rt, boolean elevator=TRUE, int pri=100, scroll_Zone z=scroll_None, boolean highlight=FALSE);
    /* The constructor for a region of type z, optionally with an endzone and highlight status specified. */
    
    virtual void ComputeRect(class scroll &scroll, int stype, const struct rectangle &bar, struct rectangle &area);
		/* Computes the rectangle within the scroll area where the region should appear.  The rectangle should reflect both the x and y axis locations.  If the scroller supports only x or y it will override the non-applicable data.
*/	
    virtual void Draw(class scroll &scroll, int stype, view_UpdateType type, const struct rectangle &damaged);
		/*  Essentially equivalent to FullUpdate for views. stype indicates whether this is the horizontal or vertical version.  (If there aren't serparate horizontal and vertical elevators drawn, same stype should be used all the type either scroll_VERT or scroll_HORIZ. */

    virtual void Update(class scroll &scroll, int stype);
		/* Called when the highlight status has changed and the region should be updated. */

    virtual void SetHighlight(boolean highlight);
		/*  Sets the new highlight status. */

    virtual class view *GetView();
		/* If this region includes a child view returns a pointer to it.  Otherwise returns NULL. */

    void SetRanges(long xrange, long xstart, long xend, long yrange, long ystart, long yend);
		/* If The RegionType is scroll_Range this sets the  displayed range, otherwise it is a no-op. */

    void SetElevator(boolean elevator);
		/*  Sets whether or not this region is an elevator. */

    inline void GetLastRect(struct rectangle &lr, int stype) { lr = lastrect[stype]; }

    inline void SetMinWidth(long mwidth) { minwidth=mwidth; }
    inline void SetMinHeight(long mheight) { minheight=mheight; }
    inline long GetMinWidth() { return minwidth; }
    inline long GetMinHeight() { return minheight; }
    
    void SetBaseSButtonPrefs(struct sbutton_prefs *sbp);
    /* Sets the SButton preferences inherited from the scroll. */
    
    enum scroll_RegionType type;
		/* See above. */

    boolean changed;
		/* set to true if the rect has been changed. */

    boolean isElevator;
		/* One region can be marked as the elevator.  The elevator is special in that it may be dragged to move the currently visible area. */

    boolean newHighlight;
		/* The new highlight status, set by SetHighlight.  Used primarily by the Endzone RegionType. */

    boolean currentHighlight;
		/* The currently displayed highlighting. */

    union scroll_RegionData data;
		/* The data used by ComputeRect for the Endzone and Range RegionTypes. */

    struct rectangle rect[scroll_TYPES];
		/* The rectangle within the scroll area where the region should appear.  This should be updated by ComputeRect, which will be called whenever the region may have changed. */

    struct rectangle lastrect[scroll_TYPES];
    /* The last rectangle within the scroll area where the region appeared.  This must be updated by the Draw method. */

    int priority;
    /* Regions with higher priority numbers will be drawn over regions with lower numbers. */

    struct sbutton_prefs *baseprefs;
    /* The base preferences inherited from whatever is displaying this region. */

    long minwidth, minheight;
    /* the minimum width and height of this region, for use only with Elevators and Dots. */
    
    virtual ~ScrollRegion();
};

class ScrollRegionElevator : public ScrollRegion {
    public:
    ScrollRegionElevator();
    ~ScrollRegionElevator();
};

class ScrollRegionDot : public ScrollRegion {
  public:
    ScrollRegionDot();
    ~ScrollRegionDot();
};


class ScrollRegionEndzone : public ScrollRegion {
  public:
    ScrollRegionEndzone();
    ~ScrollRegionEndzone();
    void Draw(class scroll &scroll, int stype, view_UpdateType type, const struct rectangle &damaged);
    void Update(class scroll &scroll, int stype);
};

#endif

