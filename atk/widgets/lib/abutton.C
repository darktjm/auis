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
