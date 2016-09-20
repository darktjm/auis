#ifndef _AOPTION_H_
#define _AOPTION_H_
/* Copyright 1996 Carnegie Mellon University All rights reserved. */
/** \addtogroup libAButton
 * @{ */
#include <abutton.H>
#include <alist.H>
#include <text.H>
#include <aaction.H>

DEFINE_AACTION_FUNC_CLASS(AOptionSlotAct,ASlot);

class AOptionMenu : public  AButton {
    ASlotDataObj source;
#ifdef AOPTION_GIVES_ALIST_INTERFACE
    ASlotInt selectedItemCount;
    ASlotPtr selectedItems;
    AListSelectedItem item;
#endif
    ASlotInt selected;
    AOptionSlotAct buttonTextAct;
  public:
    virtual ATKregistryEntry *ATKregistry();

    void NotifyObservers(long change);
    
    static void ButtonText(ASlot *slot, const avalueflex &aux, const avalueflex &in, avalueflex &out);
    
    AOptionMenu();
    
    void Insert(long ind, const char *str);
    void Remove(long ind);
    const char *Item(long ind);

    inline long ItemCount() {
        class text *src=Source();
        return (src && src->GetLength()>0)?src->GetLineForPos(src->GetLength()-1):0;
    }
#ifdef AOPTION_GIVES_ALIST_INTERFACE
    inline long SelectedItem() {
        return item.ind;
    }
    inline AListSelectedItem *SelectedItems() {
        return &item;
    }
#endif
    inline void SetSource(class text *src) {
        source=src;
    }
    inline class text *Source() {
        return source;
    }
    inline void SetSelected(long ind) {
        selected=ind;
#ifdef AOPTION_GIVES_ALIST_INTERFACE
        text *src=Source();
        if(src) {
            item.ind=ind;
            long lpos=src->GetPosForLine(ind);
            long epos=src->GetEndOfLine(lpos);
            item.pos=lpos;
            item.len=epos-lpos;
        }
#endif
    }
    inline long Selected() {
        return selected;
    }
};
/** @} */
#endif
