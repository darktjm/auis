/* Copyright 1996 Carnegie Mellon University All rights reserved. */

#ifndef _ATextV_H_
#define _ATextV_H_
ATK_INTER
/** \addtogroup libAButton
 * @{ */
#include <awidgetview.H>
#include <ashadow.H>
#include <fontdesc.H>
#include <aaction.H>

#include <aformula.H>
#include <aslot.H>

class ATextv;
#include <atext.H>


class ATextvStateFormula : public AFormula {
    ATextv *atv;
  public:
    inline ATextvStateFormula(ATextv *t) {
        SetMode(AFORMULAIMMEDIATE);
        atv=t;
    }
    virtual void Evaluate();
};
    
class ATextSizeFormula : public AFormulaViewAct {
    long dw, dh, ox, oy;
  public:
    virtual ATKregistryEntry *ATKregistry();
    
    inline ATextSizeFormula(ATextv *v=NULL, aaction *a=NULL, aaction *w=NULL) : AFormulaViewAct((view *)v, a, w) {
	dw=dh=42;
        ox=oy=0;
    }
    virtual inline ~ATextSizeFormula() {
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

class AText;
class textview;
class keystate;
class ATextv : public AWidgetView {
    friend class AText;
    friend class ATextvStateFormula;
    observable *lastobs;
    long swidth, sheight;
    enum view_DSpass  pass;
    struct rectangle interior;
    AFormulaViewAct lformula;
    AFormulaViewAct bformula;
    AFormulaViewAct scaleFormula;
    AFormulaViewAct keyStateFormula;
    ADependerKey forcelink;
    keystate *keyState;
    boolean postKeyState;
    view *mouseFocus;
    
    void TextDimensions(long &w, long &h, long lrm, long tbm);
    inline void ForceLink() {
        dataViewFormula.Invalidate(forcelink);
    }
  protected:
    AFormulaViewAct dataViewFormula;
    ATextvStateFormula stateFormula;
    ATextSizeFormula size;
    textview *tv;
    view *child;
  public:
    DECLARE_AACTION_METHOD(TextMethod);
    DECLARE_AACTION_METHOD(BorderMethod);
    DECLARE_AACTION_METHOD(SizeMethod);
    DECLARE_AACTION_METHOD(WantUpdateMethod);
    DECLARE_AACTION_METHOD(WantNewSizeMethod);
    DECLARE_AACTION_METHOD(ScaleMethod);
    DECLARE_AACTION_METHOD(DataViewMethod);
    DECLARE_AACTION_METHOD(HitMethod);
    DECLARE_AACTION_METHOD(ActivateMethod);
    DECLARE_AACTION_METHOD(KeyStateMethod);
    DECLARE_AACTION_METHOD(WantKeyStateMethod);
    
    ATKregistryEntry *ATKregistry();
    ATextv();
    ~ATextv();
    static void Init();

    void FullUpdate(enum view_UpdateType type, 
		    long left, long top, long width, long height);
    void Update();
    void SetDataObject(class dataobject *d);
    view_DSattributes DesiredSize(long width, long height, 
				  enum view_DSpass pass, 
				  long * desired_width, long * desired_height);
    void ObservedChanged (class observable *changed, 
    long value);
    void WantUpdate(class view *requestor);
    void GetOrigin(long width, long height, 
                   long * originX, long * originY);
    void InitChildren();
    void LinkTree(class view *parent);
    boolean RecSearch(struct SearchPattern *pat, boolean toplevel);
    boolean RecSrchResume(struct SearchPattern *pat);
    boolean RecSrchReplace(class dataobject *text, long pos, long len);
    void RecSrchExpose(const struct rectangle &logical, 
		       struct rectangle &hit);
    void *GetPSPrintInterface(const char *printtype);
    void PrintPSRect(FILE *outfile, long logwidth, long logheight, struct rectangle *visrect);     
    void DesiredPrintSize(long width, long height, enum view_DSpass pass, long *desiredwidth, long *desiredheight);
    void GetPrintOrigin(long w, long h, long *xoff, long *yoff);
    view *GetApplicationLayer();
    void ReceiveInputFocus();
    void LoseInputFocus();
    void PostKeyState(keystate *ks);
};

WIDGET_FUNC_AND_METHOD_CLASS(AText,ATextv);

#define START_ATEXT_MOUSE_FUNC(name) START_DATA_MOUSE_FUNC(name,AText,ATextv);
#define END_ATEXT_MOUSE_FUNC() END_MOUSE_FUNC()

#define START_ATEXT_MOUSE_METHOD(name) START_DATA_MOUSE_METHOD(name,AText,ATextv)
#define END_ATEXT_MOUSE_METHOD() END_MOUSE_METHOD()

#define START_ATEXTV_MOUSE_METHOD(name) START_VIEW_MOUSE_METHOD(name,AText,ATextv);
#define END_ATEXTV_MOUSE_METHOD() END_MOUSE_METHOD()

#define START_ATEXT_CALLBACK_FUNC(name) START_DATA_CALLBACK_FUNC(name,AText,ATextv);
#define END_ATEXT_CALLBACK_FUNC() END_CALLBACK_FUNC()

#define START_ATEXT_CALLBACK_METHOD(name) START_DATA_CALLBACK_METHOD(name,AText,ATextv)
#define END_ATEXT_CALLBACK_METHOD() END_CALLBACK_METHOD()

#define START_ATEXTV_CALLBACK_METHOD(name) START_VIEW_CALLBACK_METHOD(name,AText,ATextv)
#define END_ATEXTV_CALLBACK_METHOD() END_CALLBACK_METHOD()
/** @} */
#endif /* _AtextV_H_ */
