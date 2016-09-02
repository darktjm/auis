#ifndef _AOPTIONV_H_
#define _AOPTIONV_H_
ATK_INTER
/* Copyright 1996 Carnegie Mellon University All rights reserved.
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

#include <abuttonv.H>
#include <aaction.H>
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
    virtual ATKregistryEntry *ATKregistry();
    DECLARE_AACTION_METHOD(ButtonHitMethod);
    
    AOptionMenuv();
    virtual ~AOptionMenuv();
    void ObservedChanged(observable *changed, long change);
    void SetDataObject(class dataobject *dobj);
};

WIDGET_FUNC_AND_METHOD_CLASS(AOptionMenu,AOptionMenuv);
ATK_CLASS(AOptionCardv);
WIDGET_VIEW_METHOD_CLASS(AButton,AOptionCardv);

inline AOptionSourceFormula::~AOptionSourceFormula() {
    if(observed) observed->RemoveObserver(av);
}
#endif
