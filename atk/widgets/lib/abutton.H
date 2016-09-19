#ifndef _ABUTTON_H_
#define _ABUTTON_H_
/* Copyright 1996 Carnegie Mellon University All rights reserved. */
#include <awidget.H>
#include <fontdesc.H>
#include <aaction.H>

#define ABUTTON_INHERIT_FontFamily (1<<15)
#define ABUTTON_INHERIT_FontSize (1<<14)
class AButtonv;
class AButton : public AWidget {
    void CommonInit();
    friend class AButtonv;
  protected:
    ASlotString text;
    ASlotFont font;
    ASlotInt marginHeight;
    ASlotInt marginWidth;
    ASlotInt marginLeft, marginRight;
    ASlotInt marginTop, marginBottom;
    ASlotBool set;
    ASlotFunction activateCallback;
    ASlotFunction labelActivateCallback;
    ASlotBool indicatorOn;
    ASlotInt spacing;
    ASlotString indicatorType;
    ASlotInt showAsDefault;
    ASlotBool fillOnArm;
    ASlotDataObj labelData;
    ASlotString labelViewClass;
    ASlotInt fontInheritanceFlags;
    ASlotInt borderType;
  public:
    
    ATKregistryEntry *ATKregistry();
    AButton();
    AButton(const char *t);
    virtual ~AButton();
    inline void SetLabelData(dataobject *dobj) {
	labelData=dobj;
    }
    inline dataobject *LabelData() {
	return labelData;
    }
    inline void SetLabelViewClass(const char *str) {
	labelViewClass=str;
    }
    inline const char *LabelViewClass() {
	return labelViewClass;
    }
    inline void SetSet(boolean s) {
	set=s;
    }
    inline boolean Set() {
	return set;
    }
    inline void SetText(const char *t) {
	text=t;
    }
    inline const char *Text() const {
	return text;
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
    inline long MarginHeight() {
	return marginHeight;
    }
    
    inline void SetMarginWidth(long Width) {
	marginWidth=Width;
    }
    inline long MarginWidth() {
	return marginWidth;
    }
    
    inline void SetMarginLeft(long Left) {
	marginLeft=Left;
    }
    inline long MarginLeft() {
	return marginLeft;
    }
    
    inline void SetMarginRight(long Right) {
	marginRight=Right;
    }
    inline long MarginRight() {
	return marginRight;
    }
    
    inline void SetMarginTop(long Top) {
	marginTop=Top;
    }
    inline long MarginTop() {
	return marginTop;
    }
    inline void SetMarginBottom(long Bottom) {
	marginBottom=Bottom;
    }
    inline long MarginBottom() {
	return marginBottom;
    }
    inline void SetActivateCallback(proctable_Entry *pe) {
	activateCallback=pe;
    }
    inline void SetActivateCallback(aaction *act) {
	activateCallback=act;
    }
    inline void SetLabelActivateCallback(proctable_Entry *pe) {
	labelActivateCallback=pe;
    }
    inline void SetLabelActivateCallback(aaction *act) {
	labelActivateCallback=act;
    }
    inline aaction *ActivateCallback() {
	return activateCallback;
    }
    inline aaction *LabelActivateCallback() {
	return labelActivateCallback;
    }
    inline void SetIndicatorOn(boolean io) {
	indicatorOn=io;
    }
    inline boolean IndicatorOn() {
	return indicatorOn;
    }
    inline void SetShowAsDefault(long io) {
	showAsDefault=io;
    }
    inline long ShowAsDefault() {
	return showAsDefault;
    }
    inline void SetSpacing(long io) {
	spacing=io;
    }
    inline long Spacing() {
	return spacing;
    }
    inline void SetIndicatorType(const char *t) {
	indicatorType=t;
    }
    inline const char *IndicatorType() {
	return indicatorType;
    }
    inline void SetFillOnArm(boolean fill) {
	fillOnArm=fill;
    }
    inline boolean FillOnArm() {
	return fillOnArm;
    }
    inline void SetFontInheritanceFlags(long flags) {
        fontInheritanceFlags=flags;
    }
    inline long FontInheritanceFlags() {
        return fontInheritanceFlags;
    }
    inline void SetBorderType(long border) {
        borderType=border;
    }
    inline long BorderType() {
        return borderType;
    }
};
#endif
