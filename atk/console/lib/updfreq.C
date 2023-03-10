/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

#include <andrewos.h> /* sys/time.h sys/types.h */
#include <consoleClass.H>
#include <fontdesc.H>
#include <event.H>
#include <graphic.H>
#include <rect.h>
#include <view.H>
#include <console.h>
#include <errno.h>
#include <ctype.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <signal.h>

#include <util.h>



void UpdateDisplay(class consoleClass  *self, struct display  *disp);
void NewValue(class consoleClass  *self, struct datum  *datum, int  val, char  *text, boolean  forceupdating);
void CheckFPA(class consoleClass  *self);
void CheckTrouble(class consoleClass  *self);
static void Invert(class consoleClass  *self,struct display  *disp);
void HighlightDisplay(class consoleClass  *self, struct display  *dp);


void UpdateDisplay(class consoleClass  *self, struct display  *disp)
{
    struct rectangle clpRect;

    mydbg(("entering: UpdateDisplay\n"));
    if (disp->Clipped) {
        /* adding the next if statement to try and get around 0 and/or negative dimensioned diplays */
        if ((disp->Width - disp->Xmin) > 0){
            rectangle_SetRectSize(&clpRect, disp->Xmin, disp->Ymin, disp->Width, disp->FullHeight + 1);
        }
	else{
	    (self)->GetLogicalBounds( &clpRect);
        }
        (self)->SetClippingRect( &clpRect);
    }
    if (disp->Inverted && disp->FlashStyle == 0) {
        Invert(self, disp);
    }
    if (disp->WhatToDisplay->Value < disp->Threshhold
        || disp->WhatToDisplay->Value > disp->Ceiling
        || (disp->DependentUponVariables
            && IntrnlVars[disp->WhichVariable].Value != disp->AppearIfTrue)) {
                disp->Trouble = FALSE;
                if (disp->Inverted) {
                    disp->Inverted = FALSE;
                    Invert(self, disp);
                }
                if (disp->InRange) {
                    disp->InRange = FALSE;
                    if (disp->WhiteOut) {
                        rectangle_SetRectSize(&clpRect, disp->Xmin, disp->Ymin, disp->Width, disp->FullHeight + 1);
                        ClearRectangle(self, &clpRect, graphic::COPY, (self)->WhitePattern());
                        (self)->SetTransferMode( graphic::BLACK);
                    }
                }
            }
    else {
        if (!disp->InRange) {
            disp->InRange = TRUE;
            rectangle_SetRectSize(&clpRect, disp->Xmin, disp->Ymin, disp->Width, disp->FullHeight + 1);
            ClearRectangle(self, &clpRect, graphic::COPY, (self)->WhitePattern());
            (self)->SetTransferMode( graphic::BLACK);
            disp->Inverted = FALSE;
            (*(disp->DrawFunction))(self, REDRAW, disp);
        }
        (*(disp->DrawFunction))(self, NEWVAL, disp);
    }

    if (disp->MayFlash
        && (!disp->DependentUponVariables
            || IntrnlVars[disp->WhichVariable].Value == disp->AppearIfTrue)) {
                if (disp->FlashStyle) {	/* Non-zero is highlight, not flash */
                    HighlightDisplay(self, disp);
                }
                else
                    if (disp->FlashMin <= disp->WhatToDisplay->Value
                        && disp->FlashMax >= disp->WhatToDisplay->Value
                        && disp->WhatToDisplay->Value != disp->LastClickValue) {
                            disp->Inverted = !disp->Inverted;
                            if (disp->Inverted) {
                                Invert(self, disp);
                            }
                        }
                    else {
                        disp->Inverted = FALSE;
                    }
    }
    (self)->GetLogicalBounds( &clpRect);
    (self)->SetClippingRect( &clpRect);
}




void NewValue(class consoleClass  *self, struct datum  *datum, int  val, char  *text, boolean  forceupdating)
{
    boolean DoUpdate = FALSE;
    int     i,
        diff,
        count;

    mydbg(("entering: NewValue\n"));
    if (forceupdating || val != datum->Value) {
        DoUpdate = TRUE;
    }
    /* Bump history list if appropriate */
    if (datum->Valuelist) {
        DoUpdate = TRUE;
        count = datum->ValueCtr += 1;
        if (count >= DATAMAX) {
            diff = DATAMAX - DATAMIN;
            for (i = 0; i < DATAMIN; ++i) {
                datum->Valuelist[i] = datum->Valuelist[i + diff];
            }
            count = datum->ValueCtr = DATAMIN;
        }
        datum->Valuelist[count] = datum->Value;
    }
    /* Now set the damn values */
    datum->Value = val;
    if (text) {
        strcpy(datum->RawText, text);
    }
    if (DoUpdate) {
	struct display *mydisp;

	for (mydisp = datum->FirstDisplay; mydisp != NULL; mydisp = mydisp->NextDisplay) {
	    if (mydisp->PreDrawFunction != NULL)  {
		(*(mydisp->PreDrawFunction))(self, NEWVAL, mydisp);
	    }
	}
        datum->NeedsUpdate = TRUE;
        (self)->WantUpdate( self);
    }
    else if (datum->AnAlwaysUpdate) {  /* This is redundant code but faster generally */
	struct display *mydisp;

        for (mydisp = datum->FirstDisplay; mydisp != NULL; mydisp = mydisp->NextDisplay) {
	    if (mydisp->UpdateAlways && mydisp->PreDrawFunction != NULL) {
		(*(mydisp->PreDrawFunction))(self, NEWVAL, mydisp);
	    }
	}
        datum->NeedsUpdate = TRUE;
        (self)->WantUpdate( self);
    }
}


void CheckFPA(class consoleClass  *self)
{
    mydbg(("entering: CheckFPA\n"));
    if (fpacheck()){
	/* this is open to revision, both in content and location */
	ReportInternalError(self, "ERROR: WARNING: This Workstation Has A Bad FPA");
    }
}
    


void CheckTrouble(class consoleClass  *self)
{
    int trct;
    static char TextDum[128]="ERROR:  ", *Tdum;
    struct display *mydisp;

    mydbg(("entering: CheckTrouble\n"));
    trct = 0;
    for (mydisp = VeryFirstDisplay; mydisp; mydisp = mydisp->NextOfAllDisplays) {
        if (mydisp->Trouble) {
            ++trct;
        }
    }
    if (trct != Numbers[TROUBLE].Value) {
        for (mydisp = VeryFirstDisplay; mydisp; mydisp = mydisp->NextOfAllDisplays) {
            if (mydisp->Trouble) {
                Tdum = &TextDum[7];
                maketext(self, Tdum, mydisp, 1);
                ReportInternalError(self, TextDum);
            }
        }
    }
    NewValue(self, &Numbers[TROUBLE], trct, NULL, FALSE);
}

static void Invert(class consoleClass  *self,struct display  *disp)
        {
    struct rectangle invertRect;

    mydbg(("entering: Invert\n"));
    rectangle_SetRectSize(&invertRect, disp->Xmin, disp->Ymin, disp->Width, disp->FullHeight);
        ClearRectangle(self, &invertRect, graphic::INVERT, (self)->BlackPattern());
    (self)->SetTransferMode( graphic::BLACK);
}

#define HIGHLIGHTTHICKNESS 3
void HighlightDisplay(class consoleClass  *self, struct display  *dp)
{
    static class fontdesc *shape10 = NULL;
    boolean Highlight;
    int i, j;
    struct rectangle highlightRect;

    mydbg(("entering: HighlightDisplay\n"));
    Highlight = (dp->FlashMin <= dp->WhatToDisplay->Value
                 && dp->FlashMax >= dp->WhatToDisplay->Value);
    if (shape10 == NULL) {
	shape10 = SetupFont("shape10");
    }
    (self)->SetFont( shape10);
    (self)->SetTransferMode( graphic::BLACK);
    i = dp->Xmax - dp->Xmin;
    j = dp->Ymax - dp->Ymin;
    if (i>j) i=j;

    if (dp->FlashStyle == 1) {
        /* This code draws a thick box around an instrument.  If the instrument is too
           small, it just inverts it. */

        if (! Highlight) {
            /* This causes BOGUS FLASHING -- must be improved */
            rectangle_SetRectSize(&highlightRect, dp->Xmin, dp->Ymin, dp->Width, dp->FullHeight + 1);
            ClearRectangle(self, &highlightRect, graphic::COPY, (self)->WhitePattern());
            (self)->SetTransferMode( graphic::BLACK);
            (*(dp->DrawFunction))(self, REDRAW, dp);
            return;
        }

        if ((dp->Width < HIGHLIGHTTHICKNESS * 5)
            || (dp->FullHeight < HIGHLIGHTTHICKNESS * 5))
            {
                Invert(self,dp);
            } else {
                rectangle_SetRectSize(&highlightRect, dp->Xmin, dp->Ymin, 3, dp->FullHeight);
                ClearRectangle(self, &highlightRect, graphic::COPY, (self)->BlackPattern());
                rectangle_SetRectSize(&highlightRect, dp->Xmin, dp->Ymin, dp->Width, 3);
                ClearRectangle(self, &highlightRect, graphic::COPY, (self)->BlackPattern());
                rectangle_SetRectSize(&highlightRect, dp->Xmin, dp->Ymax - 3, dp->Width, 3);
                ClearRectangle(self, &highlightRect, graphic::COPY, (self)->BlackPattern());
                rectangle_SetRectSize(&highlightRect, dp->Xmax - 3, dp->Ymin, 3, dp->FullHeight);
                ClearRectangle(self, &highlightRect, graphic::COPY, (self)->BlackPattern());
            }
    } else if (dp->FlashStyle == 2) {
        /* This code put a little notch in the upper left corner -- did not look good with
           unboxed displays of varying sizes */
        i = i / 5;
        if (i<8) i=8;
        if (Highlight) {
            printf("Notch highlightling not supported\n");
            fflush(stdout);
        /*	wm_FillTrapezoid(dp->Xmin, dp->Ymin,
           i, dp->Xmin, dp->Ymin + i, 0, shape10, '9'); */
        } else {
            printf("Notch Highlighting not supported\n");
            fflush(stdout);
        /*	wm_FillTrapezoid(dp->Xmin , dp->Ymin,
           i, dp->Xmin, dp->Ymin + i, 0, shape10, '0');*/
        }
    } else {
        if (Highlight) {
            dp->Inverted = TRUE;
            Invert(self,dp);
        }
    }
}


