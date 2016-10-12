#ifndef _ALISTV_H_
#define _ALISTV_H_
/* Copyright 1996 Carnegie Mellon University All rights reserved. */
/** \addtogroup libAButton
 * @{ */
#include <atextv.H>
#include <alist.H>

class style;
class AListv;

class AListvTextviewStateFormula : public AFormula {
    AListv *atv;
  public:
    inline AListvTextviewStateFormula(AListv *t) {
        SetMode(AFORMULAEAGER);
        atv=t;
    }
    virtual void Evaluate();
};
   
   
class AListvTextAct : public aaction {
    AListv *av;
  public:
    inline AListvTextAct(AListv *v) {
        av=v;
    }
    virtual void operator()(ATK *obj, const avalueflex &in, avalueflex &out);
};

class AListv :public ATextv {
    friend class AListvTextviewStateFormula;
    AListvTextAct hitAct;
    void TextHit(view::MouseAction act, long pos);
    AListvTextviewStateFormula listTextviewStateFormula;
    
    long firstInd;
    boolean toggleMode;
    void SingleMode(view::MouseAction act, long ind);
    void BrowseMode(view::MouseAction act, long ind);
    void MultipleMode(view::MouseAction act, long ind);
    void ExtendedMode(view::MouseAction act, long ind);
  public:
    ATKregistryEntry *ATKregistry();
    AListv();
    static boolean Init();
    DECLARE_AACTION_METHOD(TextHitMethod);
    void InitChildren();
    void LinkTree(view *parent);
};

WIDGET_FUNC_AND_METHOD_CLASS(AList,AListv);
/** @} */
#endif /* _ALISTV_H_ */
