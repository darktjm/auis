#ifndef _AOPTIONV_H_
#define _AOPTIONV_H_
/* Copyright 1996 Carnegie Mellon University All rights reserved. */
/** \addtogroup libAButton
 * @{ */
#include <abuttonv.H>
#include <aaction.H>
#include <aoption.H>
class popupwin;

class AOptionMenuv;

class AOptionSourceFormula : public AFormula {
    AOptionMenuv *av;
    observable *observed;
  public:
    virtual ATKregistryEntry *ATKregistry();
    inline AOptionSourceFormula(AOptionMenuv *v=NULL) {
        av=v;
        observed=NULL;
    }
    inline ~AOptionSourceFormula();
    inline observable *Observed() {
        return observed;
    }
    inline void ClearObserved() {
        observed=NULL;
    }
    virtual void Evaluate();
};

class AOptionCardv : public AButtonv {
    AOptionMenuv *mv;
  public:
    virtual ATKregistryEntry *ATKregistry();
    DECLARE_AACTION_METHOD(CardHitMethod);
    
    AOptionCardv(AOptionMenuv *v=NULL);
    virtual ~AOptionCardv();
    inline void SetOptionMenuv(AOptionMenuv *v) {
        mv=v;
    }
};

class textview;
class AOptionMenuv : public AButtonv {
    popupwin *win;
    AOptionCardv *cv;
    AButton *b;
    AOptionSourceFormula source;
  protected:
    virtual void LabelDimensions(long &w, long &h, long *b=NULL);
  public:
    static boolean Init();
    TRACED_CLASS(AOptionMenuv);
    DECLARE_AACTION_METHOD(ButtonHitMethod);
    
    AOptionMenuv();
    void ObservedChanged(observable *changed, long change);
    void SetDataObject(class dataobject *dobj);
};

WIDGET_FUNC_AND_METHOD_CLASS(AOptionMenu,AOptionMenuv);
ATK_CLASS(AOptionCardv);
WIDGET_VIEW_METHOD_CLASS(AButton,AOptionCardv);

inline AOptionSourceFormula::~AOptionSourceFormula() {
    if(observed) observed->RemoveObserver(av);
}
/** @} */
#endif
