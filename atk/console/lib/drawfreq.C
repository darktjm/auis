/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

#include <andrewos.h>
#include <consoleClass.H>
#include <fontdesc.H>
#include <graphic.H>
#include <rect.h>
#include <math.h>
#include <environ.H>
#include <environment.H>
#include <style.H>
#include <logview.H>
#include <scroll.H>
#include <text.H>
#include <view.H>
#include <console.h>
#include <sitevars.h>

extern int SineMult[], CosineMult[];





static boolean REVSCROLL = FALSE;

static const char * const ErrorParseTable[] = {
    "!@#TROUBLE:",
#define ERRFLAG_TROUBLE 0
        "!@#ERRORTROUBLE:",
#define ERRFLAG_ERRORTROUBLE 1
        "!@#EXEC:",
#define ERRFLAG_EXEC 2
        "!@#EXTERNAL:",
#define ERRFLAG_EXTERNAL 3
        "!@#FULLTIME:",
#define ERRFLAG_FULLTIME 4
        "!@#PRINTSTATUS:",
#define ERRFLAG_PRINTSTATUS 5
        "!@#TOGGLE:",
#define ERRFLAG_TOGGLE 6
        "!@#MAILSTATUS:",
#define ERRFLAG_MAILSTATUS 7
        0};

#if !POSIX_ENV

#endif

    /* 
       * 2*Handwidths is width in degrees
       * HandLength * radius /10 is  length of dial hand
       */

void DrawDialHand(class consoleClass  *self, struct display  *disp, int  DialPosition , int  Bend , int  CHL)
{
    int HandPosition, Xpart, Ypart;
    
    mydbg(("entering: DrawDialHand\n"));
    while (DialPosition < 1) DialPosition += 360;
    while (DialPosition > 360) DialPosition -= 360;
    (self)->MoveTo( disp->XCenter, disp->YCenter);
    HandPosition = DialPosition - disp->HandWidth;
    while (HandPosition < 1) HandPosition += 360;
    while (HandPosition > 360) HandPosition -= 360;
    Xpart = disp->XCenter + (Bend * SineMult[HandPosition]) / 10000;
    Ypart = disp->YCenter - (Bend * CosineMult[HandPosition]) / 10000;
    (self)->DrawLineTo( Xpart, Ypart);
    Xpart = disp->XCenter + (CHL * SineMult[DialPosition]) / 10000;
    Ypart = disp->YCenter - (CHL * CosineMult[DialPosition]) / 10000;
    (self)->DrawLineTo( Xpart, Ypart);
    HandPosition = DialPosition + disp->HandWidth;
    while (HandPosition < 1) HandPosition += 360;
    while (HandPosition > 360) HandPosition -= 360;
    Xpart = disp->XCenter + (Bend * SineMult[HandPosition]) / 10000;
    Ypart = disp->YCenter - (Bend * CosineMult[HandPosition]) / 10000;
    (self)->DrawLineTo( Xpart, Ypart);
    (self)->DrawLineTo( disp->XCenter, disp->YCenter);
    (self)->SetTransferMode( graphic::BLACK);
}

void DrawDial(class consoleClass  *self, int  Op, struct display  *disp)
{
    int DialPosition, Radius;
    int CHL, Bend;

    mydbg(("entering: DrawDial\n"));
    if (!PauseEnqueuedEvents && !RingingAlarm){
        Radius = disp->Width;
        if (Radius > disp->FullHeight)
            Radius = disp->FullHeight;
        Radius = Radius / 2 - 7;
        if (Radius <= 1)
            Radius = 1;
        CHL = disp->displayparam1;
        Bend = Radius * 6 / 10;
        if (Bend > CHL * 8 / 10) {
            Bend = CHL * 8 / 10;
        }
        if (Op == NEWVAL) {
            DialPosition = 360 * disp->displayparam2 / disp->ValueMax;
	    (self)->SetTransferMode( graphic::WHITE);
	    DrawDialHand(self, disp, DialPosition, Bend, CHL);
(self)->WantUpdate( self);
        }
	(self)->SetTransferMode( graphic::BLACK);
	if (disp->HandLength >= 10) disp->HandLength = 9;
	/* above is hack to keep hand lengths from erasing the tic's on the clock dial */

        if (Op == REDRAW) {
            (self)->SetTransferMode( graphic::BLACK);
            CHL = disp->displayparam1 = (Radius * disp->HandLength) / 11;
            Bend = Radius * 6 / 10;
            if (Bend > CHL * 8 / 10) {
                Bend = CHL * 8 / 10;
          }
            if (disp->DisplayStyle == SQUAREDIAL) {
                draw_dial(self, disp->XCenter, disp->YCenter, Radius + 5, FALSE, disp->ValueMax);
            } else if (disp->DisplayStyle != DIALHIDDEN) {
                draw_dial(self, disp->XCenter, disp->YCenter, Radius + 5, TRUE, disp->ValueMax);
	    }
  (self)->WantUpdate( self);
	}
        if (Op == REDRAW || Op == NEWVAL) {
            disp->displayparam2 = disp->WhatToDisplay->Value;
            DialPosition = (360 * disp->WhatToDisplay->Value) / disp->ValueMax;
	    DrawDialHand(self, disp, DialPosition, Bend, CHL);
(self)->WantUpdate( self);
	}
    }
}


void DrawIndicator(class consoleClass  *self,int  Op, struct display  *disp)
{
    long i, l, xoff = 0, yoff = 0, inc = 0;
    long    j, k;
    char    TextDum[256];
    class fontdesc *drawfont = NULL;
    struct fontdesc_charInfo CharInfo;

    /* This routine draws the Text string in font Textfont in the "right"
       * place.  If DisplayStyle == REPEATINDICATOR, it duplicates the string
       * Value times.
       */

    mydbg(("entering: DrawIndicator\n"));
    if (!PauseEnqueuedEvents && !RingingAlarm){
        if (Op == NEWVAL) {
	    ClearBox(self, disp->Xmin, disp->Ymin, disp->Width, disp->FullHeight + 1, graphic::COPY, (self)->WhitePattern());
            (self)->SetTransferMode( graphic::BLACK);
            (self)->SetFont( drawfont = disp->Textfont);
            if (disp->ParseDisplayText) {
                maketext(self, TextDum, disp, 0);
            }
            else {
                strcpy(TextDum, disp->disptext);
            }
            (self)->SetTransferMode( graphic::BLACK);
	}
        if (Op == REDRAW) {
            if (disp->ParseDisplayText) {
                maketext(self, TextDum, disp, 0);
	    } else {
                strcpy(TextDum, disp->disptext);
            	    }
	    if (TextDum[0] != '\0'){
		if (disp->AdjustTextFont) {
		    int     maxheight;

		    maxheight = disp->Ymax - disp->Ymin;
		    if (maxheight > disp->MaxTextFontSize) {
			maxheight = disp->MaxTextFontSize;
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
			(self)->SetFont(FontsAvail[l]);
			(FontsAvail[l])->StringSize( (self)->GetDrawable(), TextDum, &j, &k);
			k = FontsAvail[l]->summary.maxHeight;
		    } while ((j > disp->Width || (k * disp->AdjustTextFont) > maxheight)
			     && l > 0);
		    drawfont = disp->Textfont = FontsAvail[l];
		}
		else {
		    drawfont = disp->Textfont;
		    (self)->SetFont(drawfont);
		}
	    }
  (self)->WantUpdate( self);
	}
	if ((Op == NEWVAL || Op == REDRAW)
	    && (TextDum[0] != '\0')
	    && drawfont) {
	    (self)->SetTransferMode( graphic::BLACK);
	    if (disp->DisplayStyle == LOGINDICATOR) {
		strcpy(TextDum, "DisplayStyle LogIndicator is obsolete and no longer works\n");
            }
            if (disp->DisplayStyle == REPEATINDICATOR) {
                j = disp->WhatToDisplay->Value;
            }
            else {
                j = 1;
            }
            (self)->SetFont( drawfont);
            while (j-- > 0) {
                if (disp->Iconic) {
                    (drawfont)->CharSummary( (self)->GetDrawable(), *TextDum, &CharInfo);
                    xoff = CharInfo.width / 2 - CharInfo.xOriginOffset;
                    yoff = CharInfo.height / 2 - CharInfo.yOriginOffset;
                    TextDum[1] = '\0';
                    (self)->MoveTo( (disp->XCenter - xoff), (disp->YCenter - yoff));
                    (self)->SetFont( drawfont);
                    (self)->DrawString( TextDum, graphic::ATLEFT | graphic::ATBASELINE);
                    inc += drawfont->summary.maxWidth;
                }
                else {
                    if (disp->DisplayStyle == LEFTINDICATOR) {
                        (self)->MoveTo( disp->Xmin + inc, disp->YCenter);
                        (self)->SetFont( drawfont);
                        (self)->DrawString( TextDum, graphic::ATLEFT | graphic::BETWEENTOPANDBASELINE);
                    }
                    else {
                        (self)->MoveTo( disp->XCenter + inc, disp->YCenter);
                        (self)->SetFont( drawfont);
                        (self)->DrawString( TextDum, graphic::BETWEENLEFTANDRIGHT | graphic::BETWEENTOPANDBASELINE);
                    }
                    (drawfont)->StringSize( (self)->GetDrawable(), TextDum, &i, &k);
                    inc += i;
                }
            }
  (self)->WantUpdate( self);
	}
    }
}


void maketext(class consoleClass  *self, char  *target, struct display  *disp, int  Which)
                {
    char   *s, u[20];
    const char *t;
    const char *v;

    mydbg(("entering: maketext\n"));
    s = target;
    if (Which == 1) {
        t=disp->ClickStringLeft;
    } else if (Which == 2) {
        t=disp->label;
    } else {
        t=disp->disptext;
    }
    while (*t) {
        switch (*t) {
            case '*':
                v = disp->WhatToDisplay->RawText;
                while ((*s++ = *v++));
                s--;
                break;
            case '$':
                itoa(disp->WhatToDisplay->Value, u);
                v = u;
                while ((*s++ = *v++));
                s--;
                break;
            case '^':
                itoa(disp->ValueMax, u);
                v = u;
                while ((*s++ = *v++));
                s--;
                break;
            case '\\':
                ++t;
            /* Drop on through */
            default:
                *s++ = *t;
                break;
        }
        t++;
    }
    *s = '\0';
  (self)->WantUpdate( self);
}


void DrawBarGraph(class consoleClass  *self, int  Op, struct display  *disp)
{
    struct datum   *dat;
    int     BarSize;

    mydbg(("entering: DrawBarGraph\n"));
    if (!PauseEnqueuedEvents && !RingingAlarm){
        dat = disp->WhatToDisplay;
        if (Op == NEWVAL) {
            if (disp->DisplayStyle == HORIZONTAL) {
                BarSize = disp->Width * dat->Value / disp->ValueMax;
                if (BarSize > disp->Width)
                    BarSize = disp->Width;
		if (disp->displayparam2 > dat->Value) {
		    ClearBox(self, disp->Xmin + BarSize, disp->Ymin, disp->Width - BarSize, disp->FullHeight, graphic::COPY, (self)->WhitePattern());
                }
                else {
                    ClearBox(self, disp->Xmin, disp->Ymin, BarSize, disp->FullHeight, graphic::COPY, (self)->BlackPattern());
                }
  (self)->WantUpdate( self);
	    }
            else {
                BarSize = disp->FullHeight * dat->Value / disp->ValueMax;
                if (BarSize > disp->FullHeight)
                    BarSize = disp->FullHeight;
                if (disp->displayparam2 > dat->Value) {
                    ClearBox(self, disp->Xmin, disp->Ymin, disp->Width, (disp->FullHeight - BarSize) + 1, graphic::COPY, (self)->WhitePattern());
                }
                else {
                    ClearBox(self, disp->Xmin, disp->Ymax - BarSize, disp->Width, BarSize, graphic::COPY, (self)->BlackPattern());
                }
            }
            disp->displayparam2 = dat->Value;
  (self)->WantUpdate( self);
	}
        if (Op == REDRAW) {

            if (disp->DisplayStyle == HORIZONTAL) {
                BarSize = disp->Width * dat->Value / disp->ValueMax;
                if (BarSize > disp->Width)
                    BarSize = disp->Width;
                ClearBox(self, disp->Xmin, disp->Ymin, BarSize, disp->FullHeight, graphic::COPY, (self)->BlackPattern());
            }
            else {
                BarSize = disp->FullHeight * dat->Value / disp->ValueMax;
                if (BarSize > disp->FullHeight)
                    BarSize = disp->FullHeight;
                ClearBox(self, disp->Xmin, disp->Ymax - BarSize, disp->Width, BarSize, graphic::COPY, (self)->BlackPattern());
            }
  (self)->WantUpdate( self);
	}
    }
}

void itoa(int  n,char  s[])
{
    int i, sign;

    mydbg(("entering: itoa\n"));
    if ((sign = n) < 0)
        n = -n;
    i = 0;
    do {
        s[i++] = n % 10 + '0';
    } while ((n/=10) >0);
    if (sign<0)
        s[i++] = '-';
    s[i] = '\0';
    reverse(s);
}

void reverse(char  s[])
{
    int i,j;
    char c;

    mydbg(("entering: reverse\n"));
    for (i=0, j=strlen(s)-1; i<j; i++,j--) {
        c=s[i];
        s[i]=s[j];
        s[j]=c;
    }
}


void DrawEKGGraph(class consoleClass  *self, int  Op, struct display  *disp)
{
    int counter, omitted, oldheight, thisheight, xm, ym, i, xinc, fh, vmax;
    struct datum *dat;


    mydbg(("entering: DrawEKGGraph\n"));
    if (!PauseEnqueuedEvents && !RingingAlarm){
        dat = disp->WhatToDisplay;
        counter = disp->displayparam1;
        fh = disp->FullHeight;
        vmax = disp->ValueMax;
        if (disp->Width < DATAMAX*2) {
            omitted = DATAMAX - disp->Width/2;
            if (omitted < 0) omitted = 0;
            xinc = 2;
        } else {
            xinc = disp->Width/DATAMAX;
            omitted = 0;
        }
        if (Op == NEWVAL && (dat->ValueCtr != counter + 1)) {
            Op = REDRAW;
        }
        if (Op == NEWVAL) {
            oldheight = dat->Valuelist[counter+1]*fh/vmax;
            thisheight = dat->Value*fh/vmax;
            if (oldheight > fh) oldheight = fh;
            if (thisheight > fh) thisheight = fh;
            (self)->SetTransferMode( graphic::BLACK);
            (self)->MoveTo( disp->Xmin+xinc*(counter -omitted+1), disp->Ymax-oldheight);
            (self)->DrawLineTo( disp->Xmin+xinc*(counter+2-omitted), disp->Ymax-thisheight);
            disp->displayparam1 +=1;
  (self)->WantUpdate( self);
	}
        if (Op == REDRAW) {
            disp->displayparam1 = dat->ValueCtr;
            ClearBox(self, disp->Xmin, disp->Ymin, disp->Width, fh + 1, graphic::COPY, (self)->WhitePattern());
            (self)->SetTransferMode( graphic::BLACK);
            thisheight = dat->Valuelist[omitted]*fh/vmax;
            if (thisheight > fh) thisheight = fh;
            xm = disp->Xmin;
            ym = disp->Ymax;
            (self)->MoveTo( xm, ym-thisheight);
            for (i=0; i<=dat->ValueCtr-omitted; ++i, xm+=xinc) {
                thisheight = dat->Valuelist[i+omitted]*fh/vmax;
                if (thisheight > fh) thisheight = fh;
                (self)->DrawLineTo( xm, ym-thisheight);
            }
            thisheight = dat->Value*fh/vmax;
            if (thisheight > fh) thisheight = fh;
            (self)->DrawLineTo( xm, ym-thisheight);
  (self)->WantUpdate( self);
	}
    }
}


extern FILE *ExternalLogFP;
extern boolean LogErrorsExternally;

void PreLogError(class consoleClass  *self, int  Op, struct display  *disp)
{
    mydbg(("entering: PreLogError\n"));
    if (Op == NEWVAL) {
        if (LogErrorsExternally) {
            if (disp->ParseDisplayText) {
                char TextDum[256];

                maketext(self, TextDum, disp, 0);
                fprintf(ExternalLogFP, "%s\n", TextDum);
                fflush(ExternalLogFP);
            } else {
                fprintf(ExternalLogFP, "%s\n", disp->disptext);
                fflush(ExternalLogFP);
            }
        } else {
            AddToLog(self,disp, FALSE, &RegionLogs[ERRORREGIONLOG],FALSE);
        }
    } else if (Op == REDRAW) { /* Used to be INIT */
        Numbers[ERRORLOG].Value = ERRORREGIONLOG; /* Probably obsolete */
    }

}

void PreLogReport(class consoleClass  *self,int  Op, struct display  *disp)
            {
    mydbg(("entering: PreLogReport\n"));
    if (Op == NEWVAL) {
	AddToLog(self,disp, FALSE, &RegionLogs[REPORTREGIONLOG],FALSE);
    } else if (Op == REDRAW) { /* Used to be INIT */
	Numbers[REPORTLOG].Value = REPORTREGIONLOG;
    }
}

void PreLogUser(class consoleClass  *self, int  Op, struct display  *disp)
            {
    mydbg(("entering: PreLogUser\n"));
    if (Op == NEWVAL) {
	AddToLog(self,disp, FALSE, &RegionLogs[USERREGIONLOG],TRUE);
    } else if (Op == REDRAW) { /* Used to be INIT */
	Numbers[USERLOG].Value = USERREGIONLOG;
    }
}

void PreLogSilly(class consoleClass  *self, int  Op, struct display  *disp)
            {
    mydbg(("entering: PreLogSilly\n"));
    if (Op == NEWVAL) {
	AddToLog(self,disp, FALSE, &RegionLogs[SILLYREGIONLOG],TRUE);
    } else if (Op == REDRAW) { /* Used to be INIT */
	Numbers[SILLYLOG].Value = SILLYREGIONLOG;
    }
}

void LogError(class consoleClass  *self, int  Op, struct display  *disp)
            {
    mydbg(("entering: LogError\n"));
}

void LogReport(class consoleClass  *self, int  Op, struct display  *disp)
            {
    mydbg(("entering: LogReport\n"));
}

void LogUser(class consoleClass  *self, int  Op, struct display  *disp)
            {
    mydbg(("entering: LogUser\n"));
}

void LogSilly(class consoleClass  *self, int  Op, struct display  *disp)
            {
    mydbg(("entering: LogSilly\n"));
}

void AddToLog(class consoleClass  *self,struct display  *disp, boolean  IsClick, struct RegionLog  *logptr,boolean  IsUser)
{
    int     WhichExtern, i, j, k, troublecount;
    boolean HighlightTrouble = FALSE;
    static char TextDum[256], FirstWord[256];
    char   *strdum;
    struct display *mydisp;

    mydbg(("entering: AddToLog\n"));
    if (IsClick) {
        maketext(self, TextDum, disp, 1);
    }
    else
        if (disp->ParseDisplayText) {
            maketext(self, TextDum, disp, 0);
        }
        else {
            strcpy(TextDum, disp->disptext);
        }
    if (!strchr(TextDum, '\n') && (IsClick || TextDum[0])) {
        strcat(TextDum, "\n");
    }
    for (i = 0; TextDum[i] != ' ' && TextDum[i] != '\0'; ++i) {
        FirstWord[i] = TextDum[i];
    }
    if (disp->DisplayStyle == REVERSESCROLLING){
	REVSCROLL = TRUE;
    }
 
   FirstWord[i] = '\0';
    k = stablk(FirstWord, ErrorParseTable, 1);
    switch (k) {
        case ERRFLAG_ERRORTROUBLE:
            HighlightTrouble = TRUE;
        /* drop through to next case */
        case ERRFLAG_TROUBLE:
                troublecount = 0;
            for (mydisp = VeryFirstDisplay; mydisp; mydisp = mydisp->NextOfAllDisplays) {
                if (mydisp->Trouble) {
                    ++troublecount;
                    maketext(self, FirstWord, mydisp, 1);
                    if (HighlightTrouble) {
                        sprintf(TextDum, "ERROR: %s\n", FirstWord);
                    }
                    else {
                        sprintf(TextDum, "%s\n", FirstWord);
		    }
                    AddStringToLog(TextDum, logptr);
                    (self)->WantUpdate( self);
             }
            }
            if (!troublecount) {
                AddStringToLog("This workstation seems to be functioning normally.\n", logptr);
                (self)->WantUpdate( self);
         }
            break;
        case ERRFLAG_EXEC:
            strdum = TextDum + i + 1;

            {
            boolean allowexec;

            allowexec = ! environ::GetProfileSwitch("SecurityConscious", FALSE);
            if (allowexec && (IsClick || IsUser)) {
                sprintf(ErrTxt, "Executing command %s", strdum);
                AddStringToLog(ErrTxt, logptr);
                (self)->WantUpdate( self);
                if (osi_vfork() == 0) {
                    execl(_SITE_BIN_SH, "sh", "-c", strdum, (char *)NULL);
                    _exit(0);
                }
            } else {
                sprintf(ErrTxt, "Not executing: %s", strdum);
                AddStringToLog(ErrTxt, logptr);
                (self)->WantUpdate( self);
            }
            }

            break;
        case ERRFLAG_FULLTIME:
            sprintf(TextDum, "%s\n", Numbers[CLOCKALL].RawText);
            if (TextDum[0] == ' ') {
                AddStringToLog(&TextDum[1], logptr);
                (self)->WantUpdate( self);
            }
            else {
                AddStringToLog(TextDum, logptr);
                (self)->WantUpdate( self);
           }
           break;
        case ERRFLAG_EXTERNAL:
            for (j = i + 1; TextDum[j] != ' ' && TextDum[j] != '\0'; ++j) {
                FirstWord[j - i - 1] = TextDum[j];
            }
            FirstWord[j - i - 1] = '\0';
            for (WhichExtern = EXTERNAL1; WhichExtern > (EXTERNAL1 - ExternalsInUse) &&	strcmp(FirstWord, Numbers[WhichExtern].ExtName); --WhichExtern) {
                ;
            }
            if (WhichExtern > EXTERNAL1 - ExternalsInUse) {
                if (TextDum[j] == '\0') {
                    AddStringToLog(TextDum, logptr);
                    (self)->WantUpdate( self);
                   break;
            }
                for (i = j + 1; TextDum[i] != ' ' && TextDum[i] != '\0'; ++i) {
                    FirstWord[i - j - 1] = TextDum[i];
                }
                FirstWord[i - j - 1] = '\0';
                if (TextDum[i] == '\0' || TextDum[i + 1] == '\0') {
                    AddStringToLog(TextDum, logptr);
                    (self)->WantUpdate( self);
                 break;
                }
                TextDum[strlen(TextDum + j) + j - 1] = '\0';
                /* Nuke trailing \n */
                NewValue(self, &Numbers[WhichExtern], atoi(FirstWord), TextDum + i + 1, TRUE);
            }
            else {
                AddStringToLog(TextDum, logptr);
                (self)->WantUpdate( self);
           }
          break;
        case ERRFLAG_PRINTSTATUS:
            if (!DoPrintChecking) {
                InitPrint(self);
                CheckPrint(self);
            }
            if (Numbers[PRINTQUEUE].Value) {
                if (Numbers[PRINTSENT].Value) {
                    sprintf(TextDum, "%d file%s awaiting printing (%d shipped to printer).\n", Numbers[PRINTQUEUE].Value,(Numbers[PRINTQUEUE].Value == 1) ? " is" : "s are", Numbers[PRINTSENT].Value);
                }
                else {
                    if (Numbers[PRINTQUEUE].Value == 1){
                        sprintf(TextDum, "There is one file awaiting printing.\n");
                    }
                    else{
                        sprintf(TextDum, "There are %d files awaiting printing.\n", Numbers[PRINTQUEUE].Value);
                    }
                }
            }
            else {
                if (Numbers[PRINTERRORS].Value) {
                    sprintf(TextDum, "The printer queue is empty, but there have been printing errors (%d).\n", Numbers[PRINTERRORS].Value);
                }
                else {
                    strcpy(TextDum, "All printing requests have been completed successfully.\n");
                }
            }
            AddStringToLog(TextDum, logptr);
            (self)->WantUpdate( self);
           break;
	case ERRFLAG_MAILSTATUS:
	    {
	    int DidMailCheck = FALSE;
	    if (!DoMailChecking) {
		InitMail(self);
		CheckMail(self, TRUE);
		DidMailCheck = TRUE;
	    }
	    if (Numbers[MAIL].Value <= 0) {
		strcpy(TextDum, "You have no mail\n");
	    }
	    else{
		if (Numbers[MAIL].Value == 1) {
		    strcpy(TextDum, "You have one mail message.\n");
		}
		else {
		    sprintf(TextDum, "You have %d mail messages.\n", Numbers[MAIL].Value);
		}
	    }
	    AddStringToLog(TextDum, logptr);
                    (self)->WantUpdate( self);
     	    if (!DidMailCheck) CheckMail(self, TRUE);
	    }
           break;
        case ERRFLAG_TOGGLE:
            if (IsClick || IsUser) {
                TogVar(self, atoi(TextDum + i + 1));
            }
            break;
        default:
            AddStringToLog(TextDum, logptr);
            (self)->WantUpdate( self);
            break;
    }
}



void AddStringToLog(const char  *string, struct RegionLog  *logptr)
{
    static char Buffer[256];
    const char   *s,
        *t;

    mydbg(("entering: AddStringToLog\n"));
    if (LogErrorsExternally) {
	return;
    }
    t = string;
    for (s = string; *s; ++s) {
        if (*s == '\n'){
            strncpy(Buffer, t,(int)(s - t) + 1);
            Buffer[(int)(s - t) + 1] = '\0';
            AddLineToLog(Buffer, logptr);
            t = s + 1;
	}
	else{
	    if((s - t) == sizeof(Buffer) - 1){
		strncpy(Buffer, t,(int)(s - t));
		Buffer[(int)(s - t)] = '\0';
		AddLineToLog(Buffer, logptr);
		t = s;
	    }
	}
    }
    if (*t) {
        mydbg(("Throwing away %s\n", t));
    }
}

long GetMyPosition(class text  *textlog)
{
    mydbg(("entering: GetMyPosition\n"));
    if(REVSCROLL){
	return(0);
    }
    else{
	return((textlog)->GetLength());
    }
}

void SetLogFence(class text  *textlog)
{
    long pos = (textlog)->GetLength();
    mydbg(("entering: SetLogFence\n"));
    (textlog)->SetFence( pos + 1);
}

void ScrollToEnd(struct RegionLog  *rlogptr, int  Op)
{
    int i;
    long pos;
    int nObservers;
    observable * const * observers;

    mydbg(("entering: ScrollToEnd\n"));
    observers = rlogptr->TextLog->GetObservers(&nObservers);
    for(i = 0; i < nObservers; i++){
	long dotlength = 0;
	
	class logview *tempview;
	pos = GetMyPosition(rlogptr->TextLog);
	tempview = ((class logview *)observers[i]);
	dotlength = (tempview)->GetDotLength();
	if(dotlength <= 1){
	    if (!(tempview)->Visible( pos - 1)){
		(tempview)->FrameDot( pos - 1);
	    }
	    (tempview)->SetDotPosition( pos);
	}
    }
    (rlogptr->TextLog)->NotifyObservers( 0);
}

#define MAXLOGLENGTH 10000
#define BASELOGLENGTH 75

void AddLineToLog(char  *string, struct RegionLog  *rlogptr)
{
    char   *s = NULL;
    long pos1, len, StartPos;
    static long LengthDiff = (MAXLOGLENGTH *  BASELOGLENGTH / 100);
    boolean IsError = FALSE;
    static char timestring[100];
    static class style *BoldStyle = NULL, *TimeStyle=NULL;
    class environment *env = NULL;
    char newstring[256];

    mydbg(("entering: AddLineToLog\n"));
    if (LogErrorsExternally) {
	return;
    }
    if (*string == '\0') {
        return;
    }
    if(BoldStyle == NULL){
	BoldStyle = new style;
	(BoldStyle)->AddNewFontFace( fontdesc_Bold);
    }
    if(TimeStyle == NULL){
	TimeStyle = new style;
	(TimeStyle)->AddNoWrap();
    }

    if (! strncmp(string, "ERROR: ", 7)){
	string[strlen(string) - 1] = '\0'; /* get rid of \n */
	sprintf(newstring, "|>> %s <<|\n", string + 7);
	string = newstring;
	 IsError = TRUE;
    }
    if (string[strlen(string) - 1] == '\n' && strlen(string) > 1) {
        string[strlen(string) - 1] = '\0';
        timestring[0] = ' ';
        timestring[1] = '(';
        timestring[2] = '\0';
	strcat(timestring, Numbers[CLOCKALL].RawText);
	if (!strchr(timestring, 'A') && !strchr(timestring, 'P')){
	    timestring[10] = '\0';  /* handle 24 hour clock time */
	}
	else{
	    timestring[13] = '\0';	/* Truncate to time only */
	}
        strcat(timestring, " )\n");
    }
    else {
        timestring[0] = '\0';
    }
    s = (char *)malloc(strlen(string) + strlen(timestring) + 2);
    if (s == NULL) {
	fprintf(stderr, "CONSOLE FATAL ERROR: could not allocate memory for s in drawfreq.AddLineToLog");
	fflush(stderr);
	exit(-1);
    }
    strcpy(s, string);
    strcat(s, " ");
    strcat(s, timestring);

    if (rlogptr->ScrollReverse){
	REVSCROLL = TRUE;
    }
    else{
	REVSCROLL = FALSE;
    }

    len = (rlogptr->TextLog)->GetLength();
    if (len > MAXLOGLENGTH){
	long EndPos;
	StartPos = (REVSCROLL) ? (len - BASELOGLENGTH) : 0;
	EndPos = len - LengthDiff;
	while ((rlogptr->TextLog)->GetChar( EndPos) != '\n'){
	    EndPos++;
	}
	EndPos++;
	(rlogptr->TextLog)->AlwaysDeleteCharacters( StartPos, EndPos);
    }

    pos1 = GetMyPosition(rlogptr->TextLog);
    (rlogptr->TextLog)->AlwaysInsertCharacters( pos1, s, strlen(s));
    if (IsError){
	env = (rlogptr->TextLog)->AlwaysAddStyle( pos1, strlen(s)-15, BoldStyle);
	(env)->SetStyle( FALSE, FALSE);
    }
    env=(rlogptr->TextLog)->AlwaysAddStyle( (pos1 + strlen(s) - 15), 15, TimeStyle);
    if (env != NULL){
	(env)->SetStyle(FALSE,FALSE);
    }
    SetLogFence(rlogptr->TextLog);
    ScrollToEnd(rlogptr, NEWVAL);
    if(s != NULL) {
	free(s);
    }
}

void DrawLog(class consoleClass  *self, int  Op, struct display  *disp)
{
    mydbg(("entering: DrawLog\n"));
    if ((PauseEnqueuedEvents == FALSE) && (RingingAlarm == FALSE)){
	if(disp != NULL) {
	    if (Op == NEWVAL || Op == REDRAW){
		if(disp->AssociatedLogView != NULL) {
		    (disp->ScrollLogView)->LinkTree( self);
		    (disp->ScrollLogView)->InsertViewSize( self, disp->Xmin, disp->Ymin, disp->Width, disp->FullHeight);
		    (disp->AssociatedLogView)->SetBorder( 3, 0);
		    (disp->ScrollLogView)->FullUpdate( view::FullRedraw, disp->Xmin, disp->Ymin, disp->Width, disp->FullHeight);
		    ScrollToEnd(disp->AssociatedLog, Op);
		}
	    }
	} else {
	    fprintf(stderr, "FATAL ERROR, the disp is NULL in DrawLog!\n");
	    fflush(stderr);
	    exit(-1);
	}
    }
}

void DrawNothing(class consoleClass  *self, int  Op, struct display  *disp)
{
    mydbg(("entering: DrawNothing\n"));
/*
   * No-op.  The point of this is to allow for the definition
   * of instruments that do nothing but flash or highlight.
   * (It is more efficient than drawindicator with null text.)
   */
}

