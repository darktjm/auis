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

#include <andrewos.h>
#include <consoleClass.H>
#include <im.H>
#include <graphic.H>
#include <fontdesc.H>
#include <rect.h>
#include <view.H>
#include <console.h>
#include <sys/param.h>

int Pposx, Pposy;
char Pstring1[256] = "", Pstring2[256] = "", Pstring3[256] = "", Pstring4[MAXPATHLEN] = "";


void ClearRectangle(class consoleClass  *self, struct rectangle  *clpRect, short  Op1, class graphic  *Op2);
void ClearBox(class consoleClass  *self, int  x , int  y , int  w , int  h, short  Op1, class graphic  *Op2);
void ClearWindow(class consoleClass  *self);
void InvertWindow(class consoleClass  *self);
void InitPstrings();
void PromptToWindow(class consoleClass  *self);
void GetStringFromWindow(class consoleClass  *self, long  maxSize);
void RedrawPrompt(class consoleClass  *self);


void ClearRectangle(class consoleClass  *self, struct rectangle  *clpRect, short  Op1, class graphic  *Op2)
{
    mydbg(("entering: ClearRectangle\n"));
    (self)->SetTransferMode( Op1);
    (self)->FillRect( clpRect, Op2);
}

void ClearBox(class consoleClass  *self, int  x , int  y , int  w , int  h, short  Op1, class graphic  *Op2)
{
    struct rectangle tempRect;

    mydbg(("entering: ClearBox\n"));
    rectangle_SetRectSize(&tempRect, x, y, w, h);
    ClearRectangle(self, &tempRect, Op1, Op2);
}

void ClearWindow(class consoleClass  *self)
{
    struct rectangle windowRect;
    
    mydbg(("entering: ClearWindow\n"));
    (self)->GetLogicalBounds( &windowRect);
    ClearRectangle(self, &windowRect, graphic_COPY, (self)->WhitePattern());
}

void InvertWindow(class consoleClass  *self)
{
    struct rectangle windowRect;

    mydbg(("entering: InvertWindow\n"));
    (self)->GetLogicalBounds( &windowRect);
    ClearRectangle(self, &windowRect, graphic_INVERT, (self)->BlackPattern());
}

void InitPstrings()
{
    mydbg(("entering: InitPstrings\n"));
    Pstring1[0] = '\0';
    Pstring2[0] = '\0';
    Pstring3[0] = '\0';
    Pstring4[0] = '\0';
    Pposx = Pposy = 0;
}

void PromptToWindow(class consoleClass  *self)
{
    short *fontWidth = (EventFont)->WidthTable( (self)->GetDrawable());
    int width = (self)->GetLogicalWidth(), height = (self)->GetLogicalHeight();

    mydbg(("entering: PromptToWindow\n"));
    ClearWindow(self);
    (self)->SetFont( EventFont);
    (self)->MoveTo( width >> 1, height / 4);
    (self)->DrawString( Pstring1, graphic_BETWEENLEFTANDRIGHT | graphic_BETWEENTOPANDBASELINE);
    (self)->MoveTo( width >> 1, height / 2);
    (self)->DrawString( Pstring2, graphic_BETWEENLEFTANDRIGHT | graphic_BETWEENTOPANDBASELINE);
    if (!strcmp(Pstring3,"==>> ")){
	(self)->MoveTo( 10, (int) (height * 0.75));
	(self)->DrawString( Pstring3, graphic_ATLEFT | graphic_BETWEENTOPANDBASELINE);
	(self)->FlushGraphics();
	Pposy = (int) ((self)->GetLogicalHeight() *.75);
	Pposx = 10 + (fontWidth['='] * 2) + (fontWidth['>'] * 2) + fontWidth[' '];
    }
    else{
	(self)->MoveTo( width >> 1, (int) (height * 0.75));
	(self)->DrawString( Pstring3, graphic_BETWEENLEFTANDRIGHT | graphic_BETWEENTOPANDBASELINE);
    }
}    



void GetStringFromWindow(class consoleClass  *self, long  maxSize)
{
    int c;
    short   *fontWidth = (EventFont)->WidthTable( (self)->GetDrawable());
    char *tempString = Pstring4;

    mydbg(("entering: GetStringFromWindow\n"));
    (self)->SetFont( EventFont);

    while (((c = ((self)->GetIM())->GetCharacter()) != '\r') && (c != '\n') && (c != EOF) && ((tempString - Pstring4) < maxSize)) {
	switch(c){
	    case '\010': /* Backspace */
	    case '\177': /* Delete */
		if (tempString > Pstring4){
		    --tempString;
		    Pposx -= fontWidth[*tempString];
		    (self)->MoveTo( Pposx, Pposy);
		    (self)->SetTransferMode( graphic_INVERT);
		    (self)->DrawString( tempString, graphic_ATLEFT | graphic_BETWEENTOPANDBASELINE);
		    *tempString = '\0';
		}
		break;
	    case '\025': /* ^U */
		while((tempString - Pstring4) != 0){
		    --tempString;
		    Pposx -= fontWidth[*tempString];
		    (self)->MoveTo( Pposx, Pposy);
		    (self)->SetTransferMode( graphic_INVERT);
		    (self)->DrawString( tempString, graphic_ATLEFT | graphic_BETWEENTOPANDBASELINE);
		    *tempString = '\0';
		}
		Pstring4[0] = '\0';
		break;
	    case '\007': /* ^G */
		Pstring4[0] = '\0';
		return;
	    default:
		if (c > '\037'){ /* printable character */
		    *tempString = (char)c;
		    tempString[1] = '\0';
		    (self)->MoveTo( Pposx, Pposy);
		    Pposx += fontWidth[c];
		    (self)->DrawString( tempString++, graphic_ATLEFT | graphic_BETWEENTOPANDBASELINE);
		}
	}
        (self)->FlushGraphics();
        (self)->SetTransferMode( graphic_BLACK);
    }
}

void RedrawPrompt(class consoleClass  *self)
{
    short *fontWidth = (EventFont)->WidthTable( (self)->GetDrawable());
    int width = (self)->GetLogicalWidth(), height = (self)->GetLogicalHeight();

    mydbg(("entering: RedrawPrompt\n"));
    ClearWindow(self);
    (self)->SetFont( EventFont);
    (self)->MoveTo( width >> 1, height / 4);
    (self)->DrawString( Pstring1, graphic_BETWEENLEFTANDRIGHT | graphic_BETWEENTOPANDBASELINE);
    (self)->MoveTo( width >> 1, height / 2);
    (self)->DrawString( Pstring2, graphic_BETWEENLEFTANDRIGHT | graphic_BETWEENTOPANDBASELINE);
    if (!strcmp(Pstring3,"==>> ")){
	(self)->MoveTo( 10, (int) (height * 0.75));
	(self)->DrawString( Pstring3, graphic_ATLEFT | graphic_BETWEENTOPANDBASELINE);
	(self)->FlushGraphics();
	Pposy = (int) ((self)->GetLogicalHeight() *.75);
	Pposx = 10 + (fontWidth['='] * 2) + (fontWidth['>'] * 2) + fontWidth[' '];
	if (Pstring4[0] != '\0'){
	    int i;
	    (self)->MoveTo( Pposx, Pposy);
	    for(i = 0; Pstring4[i] != '\0'; i++){
		Pposx += fontWidth[Pstring4[i]];
	    }
	    (self)->DrawString( Pstring4, graphic_ATLEFT | graphic_BETWEENTOPANDBASELINE);
	}
    }
    else{
	(self)->MoveTo( width >> 1, (int) (height * 0.75));
	(self)->DrawString( Pstring3, graphic_BETWEENLEFTANDRIGHT | graphic_BETWEENTOPANDBASELINE);
    }
}    


