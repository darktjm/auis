#ifndef _ALIST_H_
#define _ALIST_H_
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

#include <atext.H>
#include <mflex.H>

class AListv;
class AListvTextAct;
class style;
class AList;
class text;
class environment;

DEFINE_AACTION_FUNC_CLASS(AListSlotAct,ASlot);

struct AListSelectedItem {
    long ind;
    long pos, len;
};

DEFINE_MFLEX_CLASS(AListSelectedItems,AListSelectedItem,1);

class AListSelectionFormula : public AFormula {
    AList *al;
    AListSelectedItems items;
    static boolean ScanSelection(long rock, text *self, long pos, environment *curenv);
  public:
    inline AListSelectionFormula(AList *a) {
        al=a;
    }
    virtual void Evaluate();
    inline long Count() {
        Validate();
        return items.GetN();
    }
    inline AListSelectedItem *Items() {
        size_t dummy;
        Validate();
        return items.GetBuf(0, items.GetN(), &dummy);
    }

};

class AListStateFormula : public AFormula {
    AList *al;
  public:
    inline AListStateFormula(AList *l) {
        al=l;
    }
    virtual void Evaluate();
};

class AList :public AText {
    friend class AListv;
    friend class AListvTextAct;
    friend class AListSelectionFormula;
    friend class AListStateFormula;

    static void Count(ASlot *slot, const avalueflex &aux, const avalueflex &in, avalueflex &out);
    static void Rows(ASlot *slot, const avalueflex &aux, const avalueflex &in, avalueflex &out);
    static void SelCount(ASlot *slot, const avalueflex &aux, const avalueflex &in, avalueflex &out);
    static void Items(ASlot *slot, const avalueflex &aux, const avalueflex &in, avalueflex &out);
    
    ASlotFunction listHitCallback;
    ASlotInt selectedItemCount;
    ASlotPtr selectedItems;
    ASlotString selectionPolicy;
    ASlotFunction defaultActionCallback;
    ASlotInt topItemPosition;
    ASlotInt visibleItemCount;
    ASlotInt itemCount;

    AListSlotAct rowsAct;
    AListSlotAct selCountAct;
    AListSlotAct itemsAct;
    AListSlotAct countAct;
    AListSelectionFormula selectionFormula;
    AListStateFormula stateFormula;
    
    style *selected;
    style *notselected;
  public:
    ATKregistryEntry *ATKregistry();
    AList();
    virtual ~AList();
    void Insert(long ind, const char *str);
    void Remove(long ind);
    void Select(long ind);
    void DeSelect(long ind);
    void Toggle(long ind);
    boolean Selected(long ind);
    const char *Item(long ind);
    
    inline void SetListHitCallback(aaction *act) {
        listHitCallback=act;
    }
    inline aaction *ListHitCallback() {
        return listHitCallback;
    }
    inline long SelectedItemCount() {
        return selectedItemCount;
    }
    inline void SetSelectionPolicy(const char *p) {
        selectionPolicy=p;
    }
    inline const char *SelectionPolicy() {
        return selectionPolicy;
    }
    inline void SetDefaultActionCallback(aaction *act) {
        defaultActionCallback=act;
    }
    inline aaction *DefaultActionCallback() {
        return defaultActionCallback;
    }
    inline void SetTopItemPosition(long pos) {
        topItemPosition=pos;
    }
    inline long TopItemPosition() {
        return topItemPosition;
    }
    inline void SetVisibleItemCount(long count) {
        visibleItemCount=count;
    }
    inline long VisibleItemCount() {
        return visibleItemCount;
    }
    inline long ItemCount() {
        stateFormula.Validate();
        return itemCount;
    }
    inline const AListSelectedItem *SelectedItems() {
        stateFormula.Validate();
        return (AListSelectedItem *)(void *)selectedItems;
    }
};

#endif /* _ALIST_H_ */
