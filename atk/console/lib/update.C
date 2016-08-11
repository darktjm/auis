/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/config/COPYRITE'     *
\* ********************************************************************** */

/*
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

#ifndef NORCSID
#define NORCSID
static char rcsid[]="$Header: /afs/cs.cmu.edu/project/atk-src-C++/atk/console/lib/RCS/update.C,v 1.2 1993/05/19 17:04:37 rr2b Stab74 $";
#endif


 

#include <andrewos.h> /* sys/time.h sys/types.h */

#include <consoleClass.H>
#include <fontdesc.H>
#include <rect.h>
#include <graphic.H>
#include <view.H>
#include <scroll.H>
#include <console.h>
#include <errno.h>
#include <ctype.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <signal.h>

int DynamicXmin, DynamicXmax, DynamicYmin, DynamicYmax, XGrowthRange, YGrowthRange;




#ifndef NORCSID
#endif
int ScaleCoordinate(class consoleClass  *self, int  old, boolean  IsX);
void RedrawDisplays(class consoleClass  *self);


int ScaleCoordinate(class consoleClass  *self, int  old, boolean  IsX)
{
    int Gmin, Gmax, DesiredRange, Range, GrowthRange;

    mydbg(("entering: ScaleCoordinate\n"));
    if (IsX) {
        Gmin = DynamicXmin;
        Gmax = DynamicXmax;
        DesiredRange = MaxWidth;
        Range = (self)->GetLogicalWidth();
        GrowthRange = XGrowthRange;
    } else {
        Gmin = DynamicYmin;
        Gmax = DynamicYmax;
        DesiredRange = MaxHeight;
        Range = (self)->GetLogicalHeight();
        GrowthRange = YGrowthRange;
    }
    if (Range <= DesiredRange || Gmin < 0 || Gmin == 0 && Gmax >= ScaleFactor) {
        return(old * Range / ScaleFactor);
    }
    if (old < Gmin) {
        return(old * DesiredRange / ScaleFactor);
    }
    if (old > Gmax) {
        return(Range - (((ScaleFactor - old) * DesiredRange) / ScaleFactor));
    }
    return( ((Gmin * DesiredRange) / ScaleFactor) + (((old - Gmin) * GrowthRange) / (Gmax - Gmin)));
}



void RedrawDisplays(class consoleClass  *self)
{
    struct display *disp;
    char    ThisLabel[256];
    int	    l;
    long    j,
	    k;

    struct rectangle    clpRect,
        clpRect2;

    mydbg(("entering: RedrawDisplays\n"));
    (self)->WantUpdate(self);

    (self)->GetLogicalBounds( &clpRect);
    (self)->SetClippingRect( &clpRect);
    (self)->SetTransferMode( graphic_COPY);
    (self)->FillRect( &clpRect, (self)->WhitePattern());
    (self)->FlushGraphics();

    XGrowthRange = (self)->GetLogicalWidth() - (((ScaleFactor - DynamicXmax + DynamicXmin) * MaxWidth) / ScaleFactor);
    YGrowthRange = (self)->GetLogicalHeight() - (((ScaleFactor - DynamicYmax + DynamicYmin) * MaxHeight) / ScaleFactor);
    for (disp = VeryFirstDisplay; disp; disp = disp->NextOfAllDisplays) {
        if (disp->WhatToDisplay->IsDisplaying) {

	    if (disp->ScrollLogView != NULL){
		(disp->ScrollLogView)->UnlinkTree();
	    }

            disp->Xmin = ScaleCoordinate(self, disp->RelXmin, TRUE);
            disp->Xmax = ScaleCoordinate(self, disp->RelXmax, TRUE);
            disp->Ymin = ScaleCoordinate(self, disp->RelYmin, FALSE);
            disp->Ymax = ScaleCoordinate(self, disp->RelYmax, FALSE);
            disp->Width = disp->Xmax - disp->Xmin;
            disp->FullHeight = disp->Ymax - disp->Ymin;
            disp->XCenter = disp->Xmin + disp->Width / 2;
            disp->YCenter = disp->Ymin + disp->FullHeight / 2;
            if (disp->Boxed && (!disp->DependentUponVariables || IntrnlVars[disp->WhichVariable].Value == disp->AppearIfTrue)) {
		(self)->SetTransferMode( graphic_BLACK);
		(self)->DrawRectSize( disp->Xmin, disp->Ymin, disp->Width, disp->FullHeight);
                disp->Xmin += 3;
                disp->Ymin += 3;
                disp->Xmax -= 3;
                disp->Ymax -= 3;
		disp->Width -= 6;
		disp->FullHeight -= 6;
            }
            if (disp->IsLabelling
                && (!disp->DependentUponVariables ||
                    IntrnlVars[disp->WhichVariable].Value == disp->AppearIfTrue)) {
                        long    maxheight,
                            maxwidth;

                        maketext(self, ThisLabel, disp, 2);
                        if (disp->AdjustLabelFont) {
                            maxheight = disp->Ymax - disp->Ymin;
                            maxwidth = disp->Width;
                            if (disp->IsLabelling == TOP_LABEL || disp->IsLabelling == BOTTOM_LABEL) {
                                maxheight = maxheight / disp->AdjustLabelFont;
                                if (maxheight > disp->MaxLabelFontSize) {
                                    maxheight = disp->MaxLabelFontSize;
                                }
                            }
                            else {
                                maxwidth = maxwidth / disp->AdjustLabelFont;
                            }
                            l = FontCount;
                            do {
                                --l;
                            } while (AvailFontPts[l] > maxheight && l > 0);
                            if (l == 0) {
                                ++l;
                            }
                            do {
                                --l;
				if (FontsAvail[l] == 0) {
				    FontsAvail[l] = SetupFont(AvailFontNames[l]);
                                }
                                (self)->SetFont( FontsAvail[l]);
                                (FontsAvail[l])->StringSize( (self)->GetDrawable(), ThisLabel, &j, &k);
                                k = FontsAvail[l]->summary.maxHeight;
                            } while ((j > maxwidth || (k > maxheight)) && l > 0);
                            disp->Labelfont = FontsAvail[l];
                        }
                        else {
                            (self)->SetFont( disp->Labelfont);
                            (disp->Labelfont)->StringSize( (self)->GetDrawable(), ThisLabel, &j, &k);
                            k = disp->Labelfont->summary.maxHeight;
                            maxheight = k;
                            maxwidth = j;
                        }
                        if (k > maxheight) {
                            maxheight = k;
                        }
                        if (j > maxwidth) {
                            maxwidth = j;
                        }
                        switch (disp->IsLabelling) {
                            case BOTTOM_LABEL:
                                (self)->MoveTo( disp->XCenter, disp->Ymax - k / 2);
                                (self)->DrawString( ThisLabel, graphic_BETWEENTOPANDBASELINE | graphic_BETWEENLEFTANDRIGHT);
                                disp->Ymax -= (maxheight + 3);
                                if (disp->Boxed) {
                                    (self)->MoveTo( disp->Xmin - 3, disp->Ymax);
                                    (self)->DrawLineTo( disp->Xmax + 3, disp->Ymax);
                                    disp->Ymax -= 3;
                                }
                                break;
                            case TOP_LABEL:
                                (self)->MoveTo( disp->XCenter, disp->Ymin + k / 2);
                                (self)->DrawString( ThisLabel, graphic_BETWEENTOPANDBASELINE | graphic_BETWEENLEFTANDRIGHT);
                                disp->Ymin += maxheight + 3;
                                if (disp->Boxed) {
                                    (self)->MoveTo( disp->Xmin - 3, disp->Ymin);
                                    (self)->DrawLineTo( disp->Xmax + 3, disp->Ymin);
                                    disp->Ymin += 3;
                                }
                                break;
                            case LEFT_LABEL:
                                (self)->MoveTo( disp->Xmin + j / 2, disp->YCenter);
                                (self)->DrawString( ThisLabel, graphic_BETWEENTOPANDBASELINE | graphic_BETWEENLEFTANDRIGHT);
                                disp->Xmin += maxwidth + 3;
                                if (disp->Boxed) {
                                    (self)->MoveTo( disp->Xmin, disp->Ymin - 3);
                                    (self)->DrawLineTo( disp->Xmin, disp->Ymax + 3);
                                    disp->Xmin += 3;
                                }
                                break;
                            case RIGHT_LABEL:
                                (self)->MoveTo( disp->Xmax - j / 2, disp->YCenter);
                                (self)->DrawString( ThisLabel, graphic_BETWEENTOPANDBASELINE | graphic_BETWEENLEFTANDRIGHT);
                                disp->Xmax -= maxwidth + 3;
                                if (disp->Boxed) {
                                    (self)->MoveTo( disp->Xmax, disp->Ymax + 3);
                                    (self)->DrawLineTo( disp->Xmax, disp->Ymin - 3);
                                    disp->Xmax -= 3;
                                }
                                break;
                        }
                        disp->Width = disp->Xmax - disp->Xmin;
                        disp->FullHeight = disp->Ymax - disp->Ymin;
                        disp->XCenter = disp->Xmin + disp->Width / 2;
                        disp->YCenter = disp->Ymin + disp->FullHeight / 2;
                    }
            if (disp->WhatToDisplay->Value >= disp->Threshhold && disp->WhatToDisplay->Value <= disp->Ceiling && (!disp->DependentUponVariables || IntrnlVars[disp->WhichVariable].Value == disp->AppearIfTrue)) {
                disp->InRange = TRUE;
                if (disp->Clipped) {
                    rectangle_SetRectSize(&clpRect2, disp->Xmin, disp->Ymin, disp->Width, disp->FullHeight + 1);
                    (self)->SetClippingRect( &clpRect2);
                }
                disp->Inverted = FALSE;
                (*(disp->DrawFunction))(self, REDRAW, disp);
                if (disp->MayFlash && disp->FlashStyle) {
                    HighlightDisplay(self, disp);
                }
                (self)->SetClippingRect( &clpRect);
            }
            else {
                disp->InRange = FALSE;
            }
	}
    }
}

