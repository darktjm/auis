#ifndef _sbuttonv_H_
#define _sbuttonv_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1989, 1991 - All Rights Reserved     *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

/** \addtogroup libsupviews
 * @{ */
/*
  Trigger:  "buttonpushed"
     called when the user lets up on the pushbutton over the pushbutton.

*/

#include <view.H>

#define sbuttonv_BORDEROUT FALSE
#define sbuttonv_BORDERIN TRUE

/* for saving a views state from before to after a sbuttonv drawing operation */
struct sbuttonv_view_info {
    class fontdesc *font;
    double fgr, fgg, fgb;
    double bgr, bgg, bgb;
    short transfermode;
};

/* info relevant to each individual button */
struct sbuttonv_info {
    struct rectangle rect;
    long drawflag;
};

#define sbuttonv_LEFTBUTTON 1
#define sbuttonv_RIGHTBUTTON 2

enum sbuttonv_conv {
    sbuttonv_Interior,
    sbuttonv_Enclosing
};

#include <sbutton.H>

class sbuttonv: public  view {
public:
	TRACED_CLASS(sbuttonv);

  
    
	static boolean InitializeClass();

    
	sbuttonv();
    
    static void SaveViewState(class view *self, struct sbuttonv_view_info *vi);
    static void RestoreViewState(class view *self, struct sbuttonv_view_info *vi);

    static void DrawBorder(class view *v, long x, long y, long w, long h, struct sbutton_prefs *prefs, boolean lit, boolean draw, struct rectangle *interior, boolean fill=TRUE);
    static void EraseBorder(class view *v, long x, long y, long w, long h, struct sbutton_prefs *prefs);
    static void EraseRectBorder(class view  *self, struct rectangle  *enclosing, struct sbutton_prefs  *prefs);
/*
  just like DrawBorder except it uses a rectangle for the enclosing box */
    static void DrawRectBorder(class view *v, struct rectangle *enclosing, struct sbutton_prefs *prefs, boolean lit, boolean draw, struct rectangle *interior, boolean fill=TRUE);
    
    static void SizeForBorder(class view *v, enum sbuttonv_conv dir, int style, boolean lit, long w, long h, long *rw, long *rh);
    static void SizeForBorder(class view *v, enum sbuttonv_conv dir, struct sbutton_prefs *prefs, boolean lit, long w, long h, long *rw, long *rh);
    static void InteriorBGColor(class view *v, struct sbutton_prefs *prefs, boolean lit, double *result);
    static void DrawButton(class view *v, struct sbutton_info *b, struct rectangle *r);
    static void SafeDrawButton(class view *v, struct sbutton_info *b, struct rectangle *r);
    static void HighlightButton(class view *v, struct sbutton_info *b, struct rectangle *r);
    static void UnHighlightButton(class view *v, struct sbutton_info *b, struct rectangle *r);
    static class sbuttonv * CreateFilledSButtonv(const char *defview, struct sbutton_prefs *prefs, const struct sbutton_list *blist)  ;
    
    static void DrawLabel(class view *v, const char *label, long x, long y, struct sbutton_prefs *b, boolean lit, long flags, boolean sensitive=TRUE);
    
/*  special case of drawlabel which draws the label in the middle of the rectangle passed. */
    static void DrawButtonLabel(class view *v, const char *label, struct rectangle *interior, struct sbutton_prefs *prefs,  boolean lit, boolean sensitive=TRUE);

  
     void ObservedChanged (class observable *changed, long value);
     view::DSattributes DesiredSize(long width, long height, enum view::DSpass pass, long * desired_width, long * desired_height)  ;
     void FullUpdate(enum view::UpdateType type, long left, long top, long width, long height);
     void Update();
     class view * Hit (enum view::MouseAction action, long x, long y, long numberOfClicks)  ;
     void LinkTree(class view *parent);
     void WantUpdate(class view *requestor);
     void GetOrigin(long width, long height, long *originX, long *originY);
     void SetDataObject(class dataobject *d);
     void Print(FILE *file, const char *processor, const char *finalFormat, boolean topLevel);

    
  
    virtual int WhichButton(long x, long y)  ;
    virtual unsigned char SetActiveMouseButtons(unsigned char active, unsigned char deactive)  ;
    virtual boolean Touch(int ind, enum view::MouseAction action)  ;
    
  
    inline int GetVBorder() { return ((this)->vborder); }
    inline int GetHBorder() { return ((this)->hborder); }
    inline int GetVSpacing() { return ((this)->vspacing); }
    inline int GetHSpacing() { return ((this)->hspacing); }
    inline class sbutton * ButtonData() { return ((class sbutton *)((class view *)this)->GetDataObject()); }
    inline int LastButton() { return (((this)->lastbutton<((class sbutton *)((class view *)this)->GetDataObject())->count && (this)->lastbutton>=0)?(this)->lastbutton:0); }
    inline boolean Litp(int ind) { return (!((ind)<0 || (ind)>=((class sbutton *)((class sbuttonv *)this)->GetDataObject())->count || !((class sbutton *)((class sbuttonv *)this)->GetDataObject())->buttons[ind].lit)); }
    
    inline boolean GetDoTriggers() { return ((this)->dotriggers); }
    
  
    int maxheight, maxwidth, lastbutton, lasthighlight;
    int needredraw;
    int vborder, hborder;
    int vspacing, hspacing;
    boolean awaitingUpdate;
    int specialwidth;
    boolean dotriggers;
    unsigned char activebuttons;
    struct sbuttonv_info *info;
    int bcount;
    boolean forceupdate;
    long lwidth, lheight;
    long drawcount;
    boolean drawmatte;
};
/** @} */
#endif /* _sbuttonv_H_ */
