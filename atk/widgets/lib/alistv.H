#ifndef _ALISTV_H_
#define _ALISTV_H_
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
    void TextHit(view_MouseAction act, long pos);
    AListvTextviewStateFormula listTextviewStateFormula;
    
    long firstInd;
    boolean toggleMode;
    void SingleMode(view_MouseAction act, long ind);
    void BrowseMode(view_MouseAction act, long ind);
    void MultipleMode(view_MouseAction act, long ind);
    void ExtendedMode(view_MouseAction act, long ind);
  public:
    ATKregistryEntry *ATKregistry();
    AListv();
    static boolean Init();
    DECLARE_AACTION_METHOD(TextHitMethod);
    void InitChildren();
    void LinkTree(view *parent);
};

WIDGET_FUNC_AND_METHOD_CLASS(AList,AListv);

#endif /* _ALISTV_H_ */
