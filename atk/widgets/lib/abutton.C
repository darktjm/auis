/* Copyright 1996 Carnegie Mellon University All rights reserved. */
#include <andrewos.h>
#include <abutton.H>
#include <abuttonv.H>
#include <ashadow.H>

static boolean Init() {
    AButtonv::Init();
    return TRUE;
}

ATKdefineRegistry(AButton,AWidget,Init);

void AButton::CommonInit() {
    SLOT(text);
    SLOT(font);
    SLOTINIT(marginHeight,2);
    SLOTINIT(marginWidth,2);
    SLOTINIT(marginLeft,0);
    SLOTINIT(marginRight,0);
    SLOTINIT(marginTop,0);
    SLOTINIT(marginBottom,0);
    SLOTINIT(set,FALSE);
    SLOTINIT(activateCallback,NULLACT);
    SLOTINIT(labelActivateCallback,NULLACT);
    SLOTINIT(indicatorOn,FALSE);
    SLOTINIT(spacing,4);
    SLOTINIT(indicatorType,"N");
    SLOTINIT(showAsDefault,FALSE);
    SLOTINIT(fillOnArm, TRUE);
    SLOTINIT(labelData,NULLATK);
    SLOTINIT(labelViewClass,NULLSTR);
    SLOTINIT(fontInheritanceFlags,~0);
    SLOTINIT(borderType, AShadow_Highlight|AShadow_Default|AShadow_Plain);
}

AButton::AButton() {
    ATKinit;
    CommonInit();
}

AButton::AButton(const char *txt) {
    ATKinit;
    text=(char *)txt;
    CommonInit();
}

AButton::~AButton() {
   
}
