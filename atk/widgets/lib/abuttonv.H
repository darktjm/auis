/* Copyright 1996 Carnegie Mellon University All rights reserved. */

#ifndef _AButtonV_H_
#define _AButtonV_H_
/** \addtogroup libAButton
 * @{ */
#include <awidgetview.H>
#include <ashadow.H>
#include <fontdesc.H>
#include <aaction.H>

#include <aformula.H>
#include <aslot.H>

class AButtonv;
#include <abutton.H>


class AButtonSizeFormula : public AFormulaViewAct {
    long dw, dh, ox, oy;
  public:
    virtual ATKregistryEntry *ATKregistry();
    
    inline AButtonSizeFormula(AButtonv *v=NULL, aaction *a=NULL, aaction *w=NULL) : AFormulaViewAct((view *)v, a, w) {
	dw=dh=-1;
        ox=oy=-1;
    }
    virtual inline ~AButtonSizeFormula() {
    }
    inline void Desired(long *desired_width, long *desired_height) {
	*desired_width=dw;
	*desired_height=dh;
    }
    inline void SetDesired(long d_w, long d_h) {
	dw=d_w;
	dh=d_h;
    }
    inline void Origin(long *o_x, long *o_y) {
        *o_x=ox;
        *o_y=oy;
    }
    inline void SetOrigin(long o_x, long o_y) {
        ox=o_x;
        oy=o_y;
    }
};

class AButton;
class AButtonv : public AWidgetView {
    friend AButton;
    boolean highlighted;
    boolean selected;
    long swidth, sheight;
    enum view::DSpass  pass;
    boolean rsvalid;
    long rsstart;
    long rslen;
    void CheckIndicator(boolean force=FALSE);
    void DoSelectionHighlight(char *textl, class fontdesc *my_fontdesc, long tx, const struct rectangle *r, struct rectangle *hit, boolean draw);
    AFormulaViewAct lformula;
    AFormulaViewAct bformula;
    AFormulaViewAct iformula;
    AFormulaViewAct layout;
    struct rectangle erect;
    view *label;
    void CheckLabelView(boolean link=TRUE);
    ASlotBool armed;
    AShadow *indicator;
    void DrawLabelText(class fontdesc  *font, const struct rectangle *interior, const char  *text, int  len, long  flags);
    struct rectangle interior;
    fontdesc *lastfont;
    AFormulaViewAct scaleFormula;
  protected:
    virtual void LabelDimensions(long &w, long &h, long *b=NULL);
    virtual fontdesc *FindFont();
    AButtonSizeFormula size;
  public:
    DECLARE_AACTION_METHOD(LabelMethod);
    DECLARE_AACTION_METHOD(BorderMethod);
    DECLARE_AACTION_METHOD(IndicatorMethod);
    DECLARE_AACTION_METHOD(LayoutMethod);
    DECLARE_AACTION_METHOD(Arm);
    DECLARE_AACTION_METHOD(SizeMethod);
    DECLARE_AACTION_METHOD(WantUpdateMethod);
    DECLARE_AACTION_METHOD(ScaleMethod);
    
    TRACED_CLASS(AButtonv);
    AButtonv();
    static void Init();

    inline void SetArmed(boolean a) {armed=a;}
    inline boolean Armed() {return armed;}
    inline void SetSelected(boolean a) {selected=a;}
    inline boolean Selected() {return selected;}
    inline void SetHighlighted(boolean h) {highlighted=h;}
    inline boolean Highlighted() {return highlighted;}
    inline view *Label(void) {
        CheckLabelView(FALSE);
        return label;
    }
    void FullUpdate(enum view::UpdateType type, 
		    long left, long top, long width, long height);
    void Update();
    void SetDataObject(class dataobject *d);
    view::DSattributes DesiredSize(long width, long height, 
				  enum view::DSpass pass, 
				  long * desired_width, long * desired_height);
    void GetOrigin(long width, long height, 
		   long * originX, long * originY);
    void LinkTree(class view *parent);
    boolean RecSearch(class search *pat, boolean toplevel);
    boolean RecSrchResume(class search *pat);
    boolean RecSrchReplace(class dataobject *text, long pos, long len);
    void RecSrchExpose(const struct rectangle &logical, 
		       struct rectangle &hit);
    void *GetPSPrintInterface(const char *printtype);
    void PrintPSRect(FILE *outfile, long logwidth, long logheight, struct rectangle *visrect);     
    void DesiredPrintSize(long width, long height, enum view::DSpass pass, long *desiredwidth, long *desiredheight);
    void GetPrintOrigin(long w, long h, long *xoff, long *yoff);
};

WIDGET_FUNC_AND_METHOD_CLASS(AButton,AButtonv);

#define START_ABUTTON_MOUSE_FUNC(name) START_DATA_MOUSE_FUNC(name,AButton,AButtonv);
#define END_ABUTTON_MOUSE_FUNC() END_MOUSE_FUNC()

#define START_ABUTTON_MOUSE_METHOD(name) START_DATA_MOUSE_METHOD(name,AButton,AButtonv)
#define END_ABUTTON_MOUSE_METHOD() END_MOUSE_METHOD()

#define START_ABUTTONV_MOUSE_METHOD(name) START_VIEW_MOUSE_METHOD(name,AButton,AButtonv);
#define END_ABUTTONV_MOUSE_METHOD() END_MOUSE_METHOD()

#define START_ABUTTON_CALLBACK_FUNC(name) START_DATA_CALLBACK_FUNC(name,AButton,AButtonv);
#define END_ABUTTON_CALLBACK_FUNC() END_CALLBACK_FUNC()

#define START_ABUTTON_CALLBACK_METHOD(name) START_DATA_CALLBACK_METHOD(name,AButton,AButtonv)
#define END_ABUTTON_CALLBACK_METHOD() END_CALLBACK_METHOD()

#define START_ABUTTONV_CALLBACK_METHOD(name) START_VIEW_CALLBACK_METHOD(name,AButton,AButtonv)
#define END_ABUTTONV_CALLBACK_METHOD() END_CALLBACK_METHOD()
/** @} */
#endif /* _AButtonV_H_ */
