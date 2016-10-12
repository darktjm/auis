/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

#include <andrewos.h>
#include <im.H>
#include <consoleClass.H>
#include <view.H>
#include <fontdesc.H>
#include <cursor.H>
#include <graphic.H>
#include <rect.h>
#include <math.h>
#include <mktrig.h>
#include <console.h>
#include <sys/param.h>




extern int Pposx, Pposy;
extern char Pstring1[256], Pstring2[256], Pstring3[256], Pstring4[MAXPATHLEN];


void DrawDebug(class consoleClass  *self,int  Op, struct display  *disp);
void RingAlarm(class consoleClass  *self, int  Op , int  indexed);
int DrawGauge(class consoleClass  *self, int  Op, struct display  *disp);
void SetStandardCursor(class consoleClass  *self, short  cursor);
void SignalTrouble(class consoleClass  *self, int  Op, struct display  *disp);
void DrawTitle(class consoleClass  *self,int  Op, struct display  *disp);
void draw_corners(class consoleClass  *self, int  x1,int  y1,int  x2,int  y2,int  inc);
void DrawTics1(class consoleClass  *self, int  xc,int  yc,int  x1,int  y1,int  x2,int  y2);
void DrawTics0(class consoleClass  *self, int  xc,int  yc,int  x1,int  y1,int  x2,int  y2);
void SetupTics(class consoleClass  *self, int  xc , int  yc , int  r1 , int  r2 , int  r3 , int  r4 , boolean  IsRound, int  MaxValue);
void draw_dial(class consoleClass  *self, int  xc , int  yc , int  r1 , boolean  IsRound, int  MaxValue);


void DrawDebug(class consoleClass  *self,int  Op, struct display  *disp)
{
    mydbg(("entering: DrawDebug\n"));
    if (Op == NEWVAL) {
        mydbg(("NEWVAL %d on display %s with text %s\n", disp->WhatToDisplay->Value, disp->label, disp->WhatToDisplay->RawText));
    }
    if (Op == REDRAW) {
        mydbg(("REDRAW on display %s\n", disp->label));
    }
}

void RingAlarm(class consoleClass  *self, int  Op , int  indexed)
{
    char *BufPtr;

    mydbg(("entering: RingAlarm\n"));
    if (Op == NEWVAL) {
        PauseEnqueuedEvents = TRUE;
        if (RingingAlarm == FALSE){
            RingingAlarm = TRUE;
	    BufPtr = Numbers[ALARM].RawText;
            while(*BufPtr != ':') BufPtr++;/* Alarm: */
            BufPtr++;
            while(*BufPtr != ':') BufPtr++;/* 12:00 */
            BufPtr++;
            while(*BufPtr != ':') BufPtr++;/* AM: */
	    BufPtr++;
	    InitPstrings();
	    sprintf(Pstring1, "%s", Numbers[CLOCKALL].RawText);
	    sprintf(Pstring3, "%s", BufPtr);
	    PromptToWindow(self);
	}
	InvertWindow(self);
        PauseEnqueuedEvents = FALSE;
    }
}

int DrawGauge(class consoleClass  *self, int  Op, struct display  *disp)
{
    int xo, yo, xc, yc, xoff, yoff, diam,
        DialPosition,
	yb;
    struct rectangle clpRect;

    mydbg(("entering: DrawGauge\n"));
    if (!PauseEnqueuedEvents && !RingingAlarm){
	rectangle_SetRectSize(&clpRect, disp->Xmin, disp->Ymin, disp->Width, disp->FullHeight);
	(self)->SetClippingRect( &clpRect);
	
        xc = (disp->Xmax - disp->Xmin) / 2;
        yc = (disp->Ymax - disp->Ymin) / 2;
        diam = (xc > (yc * 2))? ((yc * 4) - 3): ((xc * 2) - 3);
	if (diam < 1) return(0); 
	xc = xc + disp->Xmin;
	yc = yc + disp->Ymin;
	yb = disp->Ymax;
	yoff = SineMult[160] * diam / 2 / 10000;
	xoff = CosineMult[160] * diam / 2 / 10000;
	xo = xc - diam / 2;
	yo = yb - diam / 2;

        DialPosition = 150 - (120 * disp->WhatToDisplay->Value) / disp->ValueMax;
        if (DialPosition < 30)
            DialPosition = 30;

	if (Op == REDRAW) {
	    (self)->SetTransferMode( graphic::BLACK);
	    (self)->DrawArcSize(xo, yo, diam, diam, 290, 140);
	    (self)->MoveTo( xc, yb);
	    (self)->DrawLineTo( xc - xoff, yb - yoff);
            (self)->MoveTo( xc, yb);
            (self)->DrawLineTo( xc + xoff, yb - yoff);
        }
        if (Op == NEWVAL) {
            (self)->SetTransferMode( graphic::WHITE);
            (self)->MoveTo( xc, yb);
            (self)->DrawLineTo( xc + CosineMult[disp->displayparam1] * diam / 2  / 11000, yb - SineMult[disp->displayparam1] * diam / 2 / 11000);
            (self)->SetTransferMode( graphic::BLACK);
            if (disp->displayparam1 > 90 || disp->displayparam1 < 10) {
                (self)->MoveTo( xc, yb);
                (self)->DrawLineTo( xc - xoff, yb - yoff);
                (self)->MoveTo( xc, yb);
                (self)->DrawLineTo( xc + xoff, yb - yoff);
            }
        }
        if (Op == REDRAW || Op == NEWVAL) {
            (self)->MoveTo( xc, yb);
            (self)->DrawLineTo( xc + CosineMult[DialPosition] * diam / 2  / 11000, yb - SineMult[DialPosition] * diam / 2 / 11000);
            disp->displayparam1 = DialPosition;
        }
    }
    return 0;
}

void SetStandardCursor(class consoleClass  *self, short  cursor)
{
    static class cursor *cp;

    mydbg(("entering: SetStandardCursor\n"));
    if (cp == NULL) cp  = cursor::Create(self);
    (cp)->SetStandard( cursor);
    (((class view *)self)->GetIM())->SetWindowCursor( cp);
}

void SignalTrouble(class consoleClass  *self, int  Op, struct display  *disp)
{
    mydbg(("entering: SignalTrouble\n"));
    if (Op == REDRAW || Op == NEWVAL) {
        disp->Trouble = TRUE;
    }
}


void DrawTitle(class consoleClass  *self,int  Op, struct display  *disp)
{
    char    TextDum[50];

    mydbg(("entering: DrawTitle\n"));
    maketext(self, TextDum, disp, 0);
    (((class view *) self)->GetIM())->SetTitle( TextDum);
}

void draw_corners(class consoleClass  *self, int  x1,int  y1,int  x2,int  y2,int  inc)
{
    mydbg(("entering: draw_corners\n"));
    if (!PauseEnqueuedEvents && !RingingAlarm){
        (self)->MoveTo( x1, y1);
        (self)->DrawLineTo( x1 + inc, y1 + inc);
        (self)->MoveTo( x1, y2);
        (self)->DrawLineTo( x1 + inc, y2 - inc);
        (self)->MoveTo( x2, y2);
        (self)->DrawLineTo( x2 - inc, y2 - inc);
        (self)->MoveTo( x2, y1);
        (self)->DrawLineTo( x2 - inc, y1 + inc);
    }
}



void DrawTics1(class consoleClass  *self, int  xc,int  yc,int  x1,int  y1,int  x2,int  y2)
{
    mydbg(("entering: DrawTics1\n"));
    if (!PauseEnqueuedEvents && !RingingAlarm){
        (self)->MoveTo((xc + x1),(yc + y1));
        (self)->DrawLineTo((xc + x2),(yc + y2));
        (self)->MoveTo((xc + x1),(yc - y1));
        (self)->DrawLineTo((xc + x2),(yc - y2));
        (self)->MoveTo((xc - x1),(yc - y1));
        (self)->DrawLineTo((xc - x2),(yc - y2));
        (self)->MoveTo((xc - x1),(yc + y1));
        (self)->DrawLineTo((xc - x2),(yc + y2));
    }
}


void DrawTics0(class consoleClass  *self, int  xc,int  yc,int  x1,int  y1,int  x2,int  y2)
{
    mydbg(("entering: DrawTics0\n"));
    if (!PauseEnqueuedEvents && !RingingAlarm){
        (self)->MoveTo((xc + x1),(yc));
        (self)->DrawLineTo((xc + x2),(yc));
        (self)->MoveTo((xc),(yc - y1));
        (self)->DrawLineTo((xc),(yc - y2));
        (self)->MoveTo((xc),(yc + y1));
        (self)->DrawLineTo((xc),(yc + y2));
        (self)->MoveTo((xc - x1),(yc));
        (self)->DrawLineTo((xc - x2),(yc));
    }
}


void SetupTics(class consoleClass  *self, int  xc , int  yc , int  r1 , int  r2 , int  r3 , int  r4 , boolean  IsRound, int  MaxValue)
{
    int     x1, y1, x2, y2, Limit, FewTics, theta;
    double    Theta, LotsOfTics, inc;

    mydbg(("entering: SetupTics\n"));
    if (!PauseEnqueuedEvents && !RingingAlarm){
	if ((MaxValue % 4) != 0){
	    arrgh(("Console: Don't know how to divide dial for MaxValue: %d -- I'll guess.\n", MaxValue));
	}
	if (MaxValue < 61){
	    FewTics = 30;
	}
	else{
	    FewTics = 18;
	}
	Limit = (int) ((2 * MaxValue) / 3);
	LotsOfTics = 360.0 / MaxValue;
	inc = (r1 < Limit) ? (double)FewTics : LotsOfTics;
	for (Theta = 0.0; Theta <= 45.0; Theta += inc) {
	    theta = (int) Theta;
	    if ((theta % FewTics) == 0) {
		if (theta == 0) {
		    x1 = y1 = r4;
		    x2 = y2 = r2;
		}
		else {
		    if (IsRound){
			x1 = (int)(r4 * CosineMult[theta] / 10000);
			x2 = (int)(r2 * CosineMult[theta] / 10000);
			y1 = (int)(r4 * SineMult[theta] / 10000);
			y2 = (int)(r2 * SineMult[theta] / 10000);
		    }
		    else{
			x1 = r4;
			x2 = r2;
			y1 = (int)(((double) SineMult[theta] / (double) CosineMult[theta]) * x1);
			y2 = (int)(((double) SineMult[theta] / (double) CosineMult[theta]) * x2);
		    }
		}
	    }
	    else {
		if (IsRound){
		    x1 = (int)(r3 * CosineMult[theta] / 10000);
		    x2 = (int)(r2 * CosineMult[theta] / 10000);
		    y1 = (int)(r3 * SineMult[theta] / 10000);
		    y2 = (int)(r2 * SineMult[theta] / 10000);
		}
		else{
		    x1 = r3;
		    x2 = r2;
		    y1 = (int)(((double) SineMult[theta] / (double) CosineMult[theta]) * x1);
		    y2 = (int)(((double) SineMult[theta] / (double) CosineMult[theta]) * x2);
		}
	    }
	    if (theta == 0) {
		DrawTics0(self, xc, yc, x1, y1, x2, y2);
	    }
	    else {
		DrawTics1(self, xc, yc, x1, y1, x2, y2);
		DrawTics1(self, xc, yc, y1, x1, y2, x2);
	    }
	}
    }
}




void draw_dial(class consoleClass  *self, int  xc , int  yc , int  r1 , boolean  IsRound, int  MaxValue)
{
    int     r2, r3, r4, corner;

    mydbg(("entering: draw_dial\n"));
    if (!PauseEnqueuedEvents && !RingingAlarm){
        r2 = (int)(r1 * 0.91);
        r3 = (int)(r2 * 0.95);
	r4 = (int)(r2 * 0.85);
	if (r1 < (int)(2 * MaxValue / 3)){
	    r2 = r1+2;
	}
	if (IsRound){
	    int d = r1 * 2;
	    (self)->DrawOvalSize( xc - r1, yc - r1, d, d);
	    d = r2 * 2;
	    (self)->DrawOvalSize( xc - r2, yc - r2, d, d);
	}
	else{
	    (self)->DrawRectSize( xc - r1, yc - r1, r1 * 2, r1 * 2);
	    (self)->DrawRectSize( xc - r2, yc - r2, r2 * 2, r2 * 2);
	    corner = (r1 - r2);
	    draw_corners(self, xc - r1, yc - r1, xc + r1, yc + r1, corner);
	}
	SetupTics(self, xc, yc, r1, r2, r3, r4, IsRound, MaxValue);
    }
}


