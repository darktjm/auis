#ifndef _ATEXT_H_
#define _ATEXT_H_
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
#include <awidget.H>
#include <fontdesc.H>
#include <aaction.H>
#include <aformula.H>
DEFINE_AACTION_FUNC_CLASS(ATextSlotAct,ASlot);
#define AText_SingleLine 1
#define AText_MultiLine 2
class text;
class ATextv;
class AText;
class ATextStateFormula : public AFormula {
    AText *txt;
  public:
    inline ATextStateFormula(AText *t) {
        SetMode(AFORMULAIMMEDIATE);
        txt=t;
    }
    virtual void Evaluate();
};
    
class AText : public AWidget {
    void CommonInit();
    friend class ATextv;
  protected:
    ASlotFont font;
    ASlotInt marginHeight;
    ASlotInt marginWidth;
    ASlotFunction activateCallback;
    ASlotFunction focusCallback;
    ASlotFunction losingFocusCallback;
    ASlotFunction valueChangedCallback;
// DEFERRED:    ASlotBool autoShowCursorPosition;
    ASlotBool editable;
// DEFERRED:    ASlotBool resizeHeight;
// DEFERRED:    ASlotBool resizeWidth;
    ASlotBool wordWrap;
    ASlotBool scrollHorizontal; 
    ASlotBool scrollVertical;
    ASlotBool scrollLeftSide;
    ASlotBool scrollTopSide;
    ASlotBool scrolled;
    ASlotInt editMode;
    ASlotInt topCharacter;
    ASlotInt cursorPosition;
    ASlotInt rows;
    ASlotInt columns;
    ASlotDataObj source;
    ASlotString value;
    ASlotString textViewClass;

    ATextStateFormula stateFormula;
    ATextSlotAct sourceAct;
    ATextSlotAct valueAct;
    ATextSlotAct valueAssignAct;
    boolean observeTextChanges;
    boolean doingTextValue;
  public:    
    ATKregistryEntry *ATKregistry();
    AText();
    AText(const char *t);
    virtual ~AText();
    void ObservedChanged(observable *changed, long change);
    inline void SetTextViewClass(const char *c) {
        textViewClass=c;
    }
    inline const char *TextViewClass() const {
        return (const char *)textViewClass;
    }
#if 0
    inline void SetAutoShowCursorPosition(boolean show) {
	autoShowCursorPosition=show;
    }
    inline boolean AutoShowCursorPosition() const {
	return autoShowCursorPosition;
    }
#endif
    inline void SetCursorPosition(long pos) {
        cursorPosition=pos;
    }
    inline long CursorPosition() {
	return cursorPosition;
    }
    inline void SetEditable(boolean e) {
        editable=e;
    }
    inline boolean Editable() const {
	return editable;
    }
    inline void SetEditMode(long mode) {
        editMode=mode;
    }
    inline long EditMode() const {
	return editMode;
    }
    inline void SetFont(fontdesc *f) {
	font=f;
    }
    inline fontdesc *Font() const {
	return font;
    }
    inline void SetMarginHeight(long height) {
	marginHeight=height;
    }
    inline long MarginHeight() const {
	return marginHeight;
    }
    
    inline void SetMarginWidth(long Width) {
	marginWidth=Width;
    }
    inline long MarginWidth() const {
	return marginWidth;
    }
    
    inline void SetActivateCallback(proctable_Entry *pe) {
	activateCallback=pe;
    }
    inline void SetActivateCallback(aaction *act) {
	activateCallback=act;
    }
    inline aaction *ActivateCallback() const {
	return activateCallback;
    }
    inline void SetFocusCallback(proctable_Entry *pe) {
        focusCallback=pe;
    }
    inline void SetFocusCallback(aaction *focus) {
	focusCallback=focus;
    }
    inline aaction *FocusCallback() const {
	return focusCallback;
    }
    inline void SetLosingFocusCallback(proctable_Entry *pe) {
	losingFocusCallback=pe;
    }
    inline void SetLosingFocusCallback(aaction *act) {
	losingFocusCallback=act;
    }
    inline aaction *LosingFocusCallback() const {
	return losingFocusCallback;
    }
#if 0
    inline void SetMaxLength(long len) {
	maxLength=len;
    }
    inline long MaxLength() const {
	return maxLength;
    }
#endif
    
    inline void SetSource(text *t) {
	source=(ATK *)t;
    }
    inline text *Source() const {
	return (text *)source;
    }
    inline void SetTopCharacter(long pos) {
        topCharacter=pos;
    }
    inline long TopCharacter() const {
	return topCharacter;
    }
    inline void SetValue(const char *str) {
        value=str;
    }
    inline const char *Value() const {
        return value;
    }
    inline void SetValueChangedCallback(proctable_Entry *pe) {
	valueChangedCallback=pe;
    }
    inline void SetValueChangedCallback(aaction *act) {
	valueChangedCallback=act;
    }
    inline aaction *ValueChangedCallback() const {
	return valueChangedCallback;
    }
#if 0
    inline void SetPendingDelete(boolean e) {
        pendingDelete=e;
    }
    inline boolean PendingDelete() const {
	return pendingDelete;
    }
    inline void SetResizeHeight(boolean e) {
        resizeHeight=e;
    }
    inline boolean ResizeHeight() const {
	return resizeHeight;
    }
    inline void SetResizeWidth(boolean e) {
        resizeWidth=e;
    }
    inline boolean ResizeWidth() const {
	return resizeWidth;
    }
#endif
    inline void SetWordWrap(boolean e) {
        wordWrap=e;
    }
    inline boolean WordWrap() const {
	return wordWrap;
    }
    inline void SetScrollHorizontal(boolean e) {
        scrollHorizontal=e;
    }
    inline boolean ScrollHorizontal() const {
	return scrollHorizontal;
    }
    inline void SetScrollLeftSide(boolean e) {
        scrollLeftSide=e;
    }
    inline boolean ScrollLeftSide() const {
	return scrollLeftSide;
    }
    inline void SetScrollTopSide(boolean e) {
        scrollTopSide=e;
    }
    inline boolean ScrollTopSide() const {
	return scrollTopSide;
    }
    inline void SetScrollVertical(boolean e) {
        scrollVertical=e;
    }
    inline boolean ScrollVertical() const {
	return scrollVertical;
    }
    inline void SetRows(long pos) {
        rows=pos;
    }
    inline long Rows() const {
	return rows;
    }
    inline void SetColumns(long pos) {
        columns=pos;
    }
    inline long Columns() const {
	return columns;
    }
    inline void SetScrolled(boolean scroll) {
        scrolled=scroll;
    }
    inline boolean Scrolled() {
        return scrolled;
    }
    // the following are internal only, public for implementation reasons
    inline void SetObserveTextChanges(boolean observe) {
        observeTextChanges=observe;
    }
    inline void SetDoingTextValue(boolean doing) {
        doingTextValue=doing;
    }
    inline boolean DoingTextValue() {
        return doingTextValue;
    }
};

#endif

