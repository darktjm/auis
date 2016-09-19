/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

#include <andrewos.h>
#include <ctype.h>
#define AUXMODULE 1
#include <textview.H>
#include <txtvcmds.h>
#include <txtvinfo.h>

#include <text.H>
#include <im.H>
#include <message.H>
#include <mark.H>
#include <environ.H>
#include <txtproc.h>

#include "shared.h"

void textview_EndOfWordCmd (class textview  *self);
void textview_ForwardWordCmd (class textview  *self);
void textview_BackwardWordCmd (class textview  *self);
void textview_LineToTopCmd(class textview  *self);
void textview_ForwardParaCmd(class textview  *self);
void textview_BackwardParaCmd(class textview  *self);
void textview_GotoParagraphCmd(class textview  *self);
void textview_WhatParagraphCmd (class textview  *v);
void textview_ViGlitchUpCmd(class textview  *self);
void textview_ViGlitchDownCmd(class textview  *self);
void textview_DownCmd(class textview  *self);
void textview_UpCmd(class textview  *self);
void textview_GlitchUpCmd(class textview  *self);
void textview_GlitchDownCmd(class textview  *self);
static long PageOverlap(long  viewHeight);
void textview_NextScreenCmd(class textview  *self);
void textview_PrevScreenCmd(class textview  *self);
void textview_StartOfParaCmd (class textview  *self);
void textview_EndOfParaCmd (class textview  *self);
void textview_SelectRegionCmd(class textview  *self);
void textview_CtrlAtCmd(class textview  *self);
void textview_BackwardCmd(class textview  *self);
void textview_ForwardCmd(class textview  *self);
void textview_PreviousLineCmd (class textview  *self  /**/);
void textview_NextLineCmd (class textview  *self /**/);
void textview_EndOfTextCmd(class textview  *self);
void textview_BeginningOfTextCmd(class textview  *self);
void textview_EndOfLineCmd (class textview  *self);
void textview_BeginningOfLineCmd(class textview  *self);
void textview_EndOfWSWordCmd(class textview  *self);
void textview_ForwardWSWordCmd(class textview  *self);
void textview_BackwardWSWordCmd(class textview  *self);
void textview_BeginningOfFirstWordCmd(class textview  *self);
void textview_BeginningOfPreviousLineCmd(class textview  *self);
void textview_BeginningOfNextLineCmd(class textview  *self);
void textview_CursorToLine(class textview  *self, long  line);
void textview_CursorToTop(class textview  *self, long  key);
void textview_CursorToCenter(class textview  *self, long  key);
void textview_CursorToBottom(class textview  *self, long  key);
void textview_GoToLineCmd(class textview  *self);


void textview_EndOfWordCmd (class textview  *self)
    {/**/
    int j, ct, pos, dlen, testType = 0;
    class text *d;
    char	c = 0;
    /****/
    pos = (self)->CollapseDot();
    ct = ((self)->GetIM())->Argument();
    d=Text(self);
    dlen= (d)->GetLength();
    for ( j = 0; j < ct; j++ )
    {
 	while ( pos < dlen && (testType = charType(c = (d)->GetChar( pos))) == WHITESPACE )
	{
	    if ( c == '\n' && (d)->GetChar( pos + 1) == '\n' )
	    {
		/* stop at blank lines */
		break;
	    }
	    pos++;
	}
	pos++;

	if ( c != '\n' )
	    while (pos < dlen && charType((d)->GetChar( pos))== testType ) pos++;
    }
    (self)->SetDotPosition(pos);
    (self)->FrameDot( pos);
    (self)->WantUpdate( self);
}

void textview_ForwardWordCmd (class textview  *self)
    {
    int j, count, pos, dlen;
    class text *d;

    pos = (self)->CollapseDot();
    count = ((self)->GetIM())->Argument();
    d=Text(self);
    dlen= (d)->GetLength();
    for ( j = 0; j < count; j++ )
    {
	if ( self->editor == EMACS )
	{
	    while (pos<dlen && isalnum((d)->GetChar( pos))==0) pos++;
	    while (pos<dlen && isalnum((d)->GetChar(pos))!=0) pos++;
	}
	else {
	    int  testType;
	    char	c;

	    if ( (testType = charType((d)->GetChar( pos))) == WHITESPACE )
	    {
		while ( pos < dlen && (testType = charType(c = (d)->GetChar( pos))) == WHITESPACE )
		{
		    if ( c == '\n' && charType((d)->GetChar( pos - 1)) == WHITESPACE )
		    {
			/* stop at end of trailing white space */
			break;
		    }
		    pos++;
		}
	    }
	    else
	    {
		while ( pos < dlen && charType((d)->GetChar( pos)) == testType ) pos++;
		while ( pos < dlen && charType(c = (d)->GetChar( pos)) == WHITESPACE )
		{
		    if ( c == '\n' ) {
			if ( (d)->GetChar( pos + 1) == '\n' )
			{
			    /* stop at blank lines */
			    pos++;
			    break;
			}
			else
			    if ( charType((d)->GetChar( pos - 1)) == WHITESPACE )
			    {
				/* stop at end of trailing white space */
				break;
			    }
		    }
		    pos++;
		}
	    }
	}
    }

    (self)->SetDotPosition(pos);
    (self)->FrameDot( pos);
    (self)->WantUpdate( self);
}

void textview_ForwardBeginWordCmd (class textview *self)
{
    long pos, textSize;
    class text *d = Text(self);

    textview_ForwardWordCmd(self);
    textSize = d->GetLength();
    pos = self->GetDotPosition();
    while (pos < textSize && charType(d->GetChar(pos)) != WORD)
	pos++;
    self->SetDotPosition(pos);
    self->FrameDot(pos);
    self->WantUpdate(self);
}

void textview_BackwardWordCmd (class textview  *self)
    {
    int j, count, pos;
    class text *d;

    pos = (self)->CollapseDot();
    count = ((self)->GetIM())->Argument();
    d=Text(self);
    for ( j = 0; j < count; j++ )
    {
	if ( self->editor == EMACS )
	{
	    while (pos>0 && isalnum((d)->GetChar(pos-1))==0)  pos--;
	    while (pos>0 && isalnum((d)->GetChar(pos-1))!=0) pos--;
	}
	else
	{
	    int testType;
	    char	c = 0;

	    pos--;
	    while ( pos > 0 && (testType = charType(c = (d)->GetChar( pos))) == WHITESPACE )
	    {
		if ( c == '\n' && (d)->GetChar( pos - 1) == '\n' )
		{
		    /* stop at blank lines */
		    break;
		}
		pos--;
	    }
	    if ( c != '\n' )
		while ( pos > 0 && charType((d)->GetChar( pos - 1))== testType )  pos--;
	}
    }
    (self)->SetDotPosition(pos);
    (self)->FrameDot( pos);
    (self)->WantUpdate( self);
}

void textview_LineToTopCmd(class textview  *self)
{
    long pos;

    pos = (self)->GetDotPosition() + (self)->GetDotLength();
    pos = (self)->MoveBack( pos, 0, textview_MoveByLines, 0, 0);

    (self)->SetTopPosition( pos);
    (self)->WantUpdate( self);
}

void textview_ForwardParaCmd(class textview  *self)
{
    int j, ct, pos, dlen;
    class text *d;

    j = 0;
    ct = ((self)->GetIM())->Argument();
    d = Text(self);
    dlen = (d)->GetLength();
    pos =(self)->GetDotPosition();
    while (j<ct) {
	while (pos<dlen) {
	    pos++;	/* always move at least one character */
	    if ((d)->GetChar(pos) == '\n') break;
	}
	(self)->SetDotPosition( pos);
	if (pos < dlen) pos++;
	j++;
    }
    (self)->WantUpdate( self);
}

void textview_BackwardParaCmd(class textview  *self)
{
    int j, ct, pos;
    class text *d;

    j = 0;
    ct = ((self)->GetIM())->Argument();
    d=Text(self);
    pos=(self)->GetDotPosition();
    while (j<ct)  {
	if (pos > 0) pos--;
	while (pos>0 && (d)->GetChar(pos-1) != '\n') pos--;
	j++;
    }
    (self)->SetDotPosition(pos);
    (self)->WantUpdate( self);
}

void textview_GotoParagraphCmd(class textview  *self)
{
    char temp[100];
    int line, gf;
    int pos;
    class text *d;

    d = Text(self);

    if (((self)->GetIM())->ArgProvided())
        line = ((self)->GetIM())->Argument();
    else {
        gf = message::AskForString(self, 0, "What paragraph? ", 0, temp, 100);
        if (gf < 0)
            return;
        line = atoi(temp);
    }

    pos = (d)->GetPosForLine( line);

    (self)->SetDotPosition( pos);
    (self)->SetDotLength( 0);
    (self)->FrameDot( pos);
    (self)->WantUpdate( self);
}

void textview_WhatParagraphCmd (class textview  *v)
{
    char temp[100];
    int i, pos;
    class text *d;

    d=Text(v);
    pos = (v)->GetDotPosition();

    i = (d)->GetLineForPos( pos);
    
    sprintf(temp,"Paragraph %d, position %ld.", i, pos - d->GetBeginningOfLine(pos) + 1);
    message::DisplayString(v, 0, temp);
}

void textview_ViGlitchUpCmd(class textview  *self)
{
    int n;
    int pos;
    long 	dotPos;
    long 	dist, lines;

    n = (self->imPtr)->Argument();
    pos = (self)->GetTopPosition();
    pos = (self)->MoveBack( pos, 0, textview_MoveByLines, 0, 0);
    pos = (self)->MoveForward( pos, n, textview_MoveByLines, 0, 0);
    if (self->scroll == textview_ScrollBackward)
	self->scroll = textview_MultipleScroll;
    dotPos = (self)->GetDotPosition();
    if ( (self)->Visible( dotPos) )
	dotPos = (self)->MoveForward( dotPos, n, textview_MoveByLines, &dist, &lines);
    else
	dotPos = pos;
    (self)->SetDotPosition( pos);
    (self)->SetTopPosition( pos);
    (self)->WantUpdate( self);
}

void textview_ViGlitchDownCmd(class textview  *self)
{
    int n,pos;
    long	dotPos;
    long 	dist, lines;

    n = (self->imPtr)->Argument();
    pos = (self)->GetTopPosition();
    pos = (self)->MoveBack( pos, n, textview_MoveByLines, &dist, &lines);
    if (self->scroll == textview_ScrollForward)
	self->scroll = textview_MultipleScroll;
    else {
	if (self->scrollDist == -1) {
	    self->scrollDist = dist;
	    self->scrollLine = lines;
	}
	else {
	    self->scrollDist += dist;
	    if (self->scrollDist >= (self)->GetLogicalHeight())
		self->scrollDist = -1;
	    else
		self->scrollLine += lines;
	}
    }
    dotPos = (self)->GetDotPosition();
    if ( (self)->Visible( dotPos) )
	dotPos = (self)->MoveBack( dotPos, n, textview_MoveByLines, &dist, &lines);
    else
	dotPos = pos;
    (self)->SetDotPosition( pos);
    (self)->SetTopPosition( pos);
    (self)->WantUpdate( self);
}

void textview_DownCmd(class textview  *self)
    {
    if ( (self->imPtr)->Argument() == 1 )
    {
	/* default is half the screen */
	(self->imPtr)->argState.argProvided = TRUE;
	(self->imPtr)->argState.argument  = textview_GetLines(self)/2;
    }
    textview_ViGlitchUpCmd(self);
}

void textview_UpCmd(class textview  *self)
    {
    if ( (self->imPtr)->Argument() == 1 )
    {
	/* default is half the screen */
	(self->imPtr)->argState.argProvided = TRUE;
	(self->imPtr)->argState.argument  = textview_GetLines(self)/2;
    }
    textview_ViGlitchDownCmd(self);
}

void textview_GlitchUpCmd(class textview  *self)
{
    int n;
    int pos;

    n = ((self)->GetIM())->Argument();
    pos = (self)->GetTopPosition();
    pos = (self)->MoveBack( pos, 0, textview_MoveByLines, 0, 0);
    pos = (self)->MoveForward( pos, n, textview_MoveByPseudoLines, 0, 0);
    if (self->scroll == textview_ScrollBackward) {
        self->scroll = textview_MultipleScroll;
    }
    (self)->SetTopOffTop( pos, self->pixelsComingOffTop);
    (self)->WantUpdate( self);
}

void textview_GlitchDownCmd(class textview  *self)
{
    int n,pos;
    long dist, lines;

    n = ((self)->GetIM())->Argument();
    pos = (self)->GetTopPosition();
    pos = (self)->MoveBack( pos, n, textview_MoveByPseudoLines, &dist, &lines);
    if (self->scroll == textview_ScrollForward) {
        self->scroll = textview_MultipleScroll;
    }
    else {
	if (self->scrollDist == -1) {
	    self->scrollDist = dist;
	    self->scrollLine = lines;
	}
	else {
	    self->scrollDist += dist;
	    if (self->scrollDist >= (self)->GetLogicalHeight())
		self->scrollDist = -1;
	    else
		self->scrollLine += lines;
	}
    }
	
    (self)->SetTopOffTop( pos, self->pixelsComingOffTop);
    (self)->WantUpdate( self);
}

static long PageOverlap(long  viewHeight)
{
    return (viewHeight < 147) ? viewHeight / 3 : 49;
}

static void DoNextScreenCmd(class textview  *self, boolean move_dot)
{
    int argument = ((self)->GetIM())->Argument();
    int count;
    text *d = Text(self);

    ((self)->GetIM())->ClearArg();
    for (count = 0; count < argument; ++count) {
        int numLines = textview_GetLines(self);
        long pos;
        long overlap;
        long viewHeight;

        if (numLines == 0) {
            /* Do nothing; there is no text on the screen. */
            return;
        }

        viewHeight = (self)->GetLogicalHeight() - self->by;
        overlap = PageOverlap(viewHeight);

        pos = (self)->GetTopPosition();
        /* get line aligned */
        pos = (self)->MoveBack( pos, 0, textview_MoveByLines, 0, 0);
        pos = (self)->MoveForward( pos, viewHeight - overlap, textview_MoveByPixels, 0, 0);
        if (self->scroll == textview_ScrollBackward) {
            self->scroll = textview_MultipleScroll;
        }
	/* It is rare, but a partial line at the end of text can cause a pos
	   that is not the beginning of a line after the move. */
	pos = d->GetBeginningOfLine(pos);	// adjust pos--usually no change
        (self)->SetTopOffTop( pos, self->pixelsComingOffTop);

        if (move_dot) {
            self->SetDotPosition( pos);
	    self->SetDotLength(0);
        }
    }
    (self)->WantUpdate( self);
}

static void DoPrevScreenCmd(class textview  *self, boolean move_dot)
{
    int argument = ((self)->GetIM())->Argument();
    int count;

    ((self)->GetIM())->ClearArg();

    for (count = 0; count < argument; ++count) {
        long numLines = textview_GetLines(self);
        long pos;
        long viewHeight;
        long dist;
        long lines;
        long overlapHeight;
	long toplineHeight = (self->lines[0].y + self->lines[0].height);

        viewHeight = (self)->GetLogicalHeight() - self->by;
        if (numLines == 0) {
            overlapHeight = 0;
        }
        else {
            overlapHeight = PageOverlap(viewHeight);
        }

        pos = (self)->GetTopPosition();
        pos = (self)->MoveBack( pos, viewHeight - overlapHeight + toplineHeight, textview_MoveByPixels, &dist, &lines);

        if (self->scroll == textview_ScrollForward)
            self->scroll = textview_MultipleScroll;
        else if (self->scrollDist == -1)  {
            self->scrollDist = dist;
            self->scrollLine = lines;
        }
        else  {
            self->scrollDist = -1;
        }

        (self)->SetTopOffTop( pos, self->pixelsComingOffTop);

        if (move_dot) {
            self->SetDotPosition( pos);
	    self->SetDotLength(0);
        }
    }
    (self)->WantUpdate( self);
}

void textview_NextScreenCmd(class textview *self)
{
    DoNextScreenCmd(self, self->editor == VI);
}

void textview_NextScreenMoveCmd(class textview *self)
{
    DoNextScreenCmd(self, TRUE);
}

void textview_PrevScreenCmd(class textview *self)
{
    DoPrevScreenCmd(self, self->editor == VI);
}

void textview_PrevScreenMoveCmd(class textview *self)
{
     DoPrevScreenCmd(self, TRUE);
}

void textview_StartOfParaCmd (class textview  *self)
{
    class text *d;
    int pos;

    d = Text(self);
    pos = (self)->GetDotPosition();
    pos = (d)->GetBeginningOfLine( pos);
    (self)->SetDotPosition( pos);
    (self)->SetDotLength( 0);
    (self)->WantUpdate( self);
}

void textview_EndOfParaCmd (class textview  *self)
{
    class text *d;
    int pos;

    d = Text(self);
    pos = (d)->GetEndOfLine( (self)->GetDotPosition());
    (self)->SetDotPosition( pos);
    (self)->SetDotLength( 0);
    (self)->WantUpdate( self);
}

void textview_SelectRegionCmd(class textview  *self)
{
    int i;
    int dot, mark;

    mark = (self->atMarker)->GetPos();
    dot = (self)->GetDotPosition();
    if (mark > dot) {
	i = dot;
	dot = mark;
	mark = i;
    }
    /* Now assume that mark <= dot */
    (self)->SetDotPosition(mark);
    (self)->SetDotLength(dot-mark);
    (self)->WantUpdate( self);
}

void textview_CtrlAtCmd(class textview  *self)
{
    (self->atMarker)->SetPos((self)->GetDotPosition());
    (self->atMarker)->SetLength((self)->GetDotLength());
    message::DisplayString(self, 0, "Mark set.");
}

void textview_BackwardCmd(class textview  *self)
{
    long endpos, len;
    class text *d;

    endpos = (self)->CollapseDot();
    d = Text(self);
    len = ((self)->GetIM())->Argument();

    if (endpos == 0)
        return;
    if (endpos - len < 0)
	len = endpos;

    if ( self->editor == VI )
	for ( ; endpos > 0 && len > 0 && (d)->GetChar( endpos - 1) != '\n' ; endpos--)
	    len--;
    else
	endpos -= len;

    (self)->SetDotPosition(endpos);
    (self)->FrameDot( endpos);
    (self)->WantUpdate( self);
}

void textview_ForwardCmd(class textview  *self)
{
    long pos;
    long newPos;

    pos = (self)->CollapseDot();
    (self)->SetDotPosition( (newPos = pos + ((self)->GetIM())->Argument()));
    if (pos != (self)->GetDotPosition()) {
        (self)->FrameDot( newPos);
        (self)->WantUpdate( self);
    }
}

void textview_PreviousLineCmd (class textview  *self  /**/)
{
    int npos, j;
    int xpos;
    class mark tm;	/* note this mark is not on the document's marker chain */
    long pos, dist, lines, cumLines, currentline, startPos, nlPos, prevPos;
    class text		*text;

    startPos = (self)->GetDotPosition();
    pos = (self)->CollapseDot();
    text	= Text(self);
    if (((self)->GetIM())->GetLastCmd() == lcMove)
	xpos = self->movePosition;
    else
	self->movePosition = xpos = self->cexPos;
    if ( self->editor == VI )
    {
	currentline = (self)->FindLineNumber( startPos);
	/* ignore wrap-around lines */
	cumLines	=	lines	= 0;
	npos	= startPos;
	for (j = (self->imPtr)->Argument(); j > 0; j--)
	{
	    nlPos = (text)->GetBeginningOfLine( (text)->GetBeginningOfLine( npos) - 1);
	    do
	    {
		prevPos = npos;
		npos = (self)->MoveBack( prevPos, 1, textview_MoveByLines, &dist, &lines);
		cumLines++;
	    } while ( npos > nlPos && prevPos != npos);
	}
    }
    else {
	currentline = (self)->FindLineNumber( pos);
	npos = (self)->MoveBack( pos, ((self)->GetIM())->Argument(), textview_MoveByLines, &dist, &cumLines);
    }
    if(cumLines==currentline) {
	// If we're moving up to the top line make sure it is scrolled all the way
	// on screen. 
	self->SetTopPosition(self->GetTopPosition());
    }
    if(cumLines==currentline) {
	// If we're moving up to the top line make sure it is scrolled all the way
	// on screen. 
	self->SetTopPosition(self->GetTopPosition());
    }
    if (cumLines > currentline)  {
	/* Have moved back off the screen */
	if (self->scroll == textview_ScrollForward)
	    self->scroll = textview_MultipleScroll;
	if (self->scrollDist == -1 )  {
	    if (currentline != -1)  {
		self->scrollDist = dist - (self->lines[currentline].y - self->lines[0].y);
		self->scrollLine = cumLines - currentline;
		(self)->SetTopPosition( npos);
	    }
	    else  {
		long topPos;

		currentline = (self)->FindLineNumber( startPos);
		if (currentline != -1 || (startPos <= (topPos = (self)->GetTopPosition()) && startPos >= topPos))
		{
		    if ( self->editor == VI )
		    {
			(self->imPtr)->argState.argProvided = TRUE;
			(self->imPtr)->argState.argument  = cumLines;
			textview_ViGlitchDownCmd(self);
		    }
		    else
			(self)->FrameDot( npos);
		}
	    }
	}
	else  {
	    self->scrollDist += dist;
	    self->scrollLine += cumLines;
	    (self)->SetTopPosition( npos);
	}
    }
    else
	(self)->FrameDot( npos);
    (&tm)->SetPos( npos);
    (&tm)->SetLength( 0);
    if (self->csxPos != -1)  {
	struct formattinginfo info;

	npos = (self)->LineRedraw ( textview_GetPosition, &tm, self->bx,
	    self->by, (self)->GetLogicalWidth() - 2 * self->bx - self->para_width - self->hz_offset,
	    (self)->GetLogicalHeight() - 2 * self->by, xpos - self->para_width - self->hz_offset, (int *) 0, NULL, &info);
    }
    (self)->SetDotPosition( npos);
    (self->imPtr)->SetLastCmd( lcMove);
    (self)->WantUpdate( self);
}

/* original textview_NextLineCmd..., now only used for VI mode */
static void textview_VINextLineCmd (class textview  *self /**/)
{
    int npos, j;
    int xpos;
    class mark tm;	/* note this mark is not on the document's marker chain */
    long currentline, pos, lines, nlines, newline, startPos, nlPos, dsize;
    static int linetomove;

    startPos = (self)->GetDotPosition();
    pos = (self)->CollapseDot();
    dsize	= (Text(self))->GetLength();

    if ((self->imPtr)->GetLastCmd() == lcMove)
	xpos = self->movePosition;
    else
	self->movePosition = xpos = self->cexPos;
    if ( self->editor == VI )
    {
	currentline = (self)->FindLineNumber( startPos);
	npos = (self)->MoveBack(startPos,0, textview_MoveByLines, NULL, NULL);

	/* ignore wrap-around lines */
	nlines	= 0;
	npos	= startPos;
	for (j = ((self)->GetIM())->Argument(); j > 0; j--)
	{
	    if ( (nlPos = (Text(self))->GetEndOfLine( npos) + 1) == dsize )
		break;
	    do
	    {
		npos = (self)->MoveForward(npos, 1, textview_MoveByLines, NULL, &lines);
		nlines++;

	    } while ( npos < nlPos && npos < dsize );
	}
    }
    else {
	currentline = (self)->FindLineNumber( pos);
	npos = (self)->MoveBack(pos,0, textview_MoveByLines, NULL, NULL);
	npos = (self)->MoveForward(npos, nlines = ((self)->GetIM())->Argument(), textview_MoveByLines, NULL, NULL);
    }
    if (currentline != -1)  {
	/* Current Position is on the screen */
	
	newline = (self)->FindLineNumber( npos);
	if (newline == -1)  {
	    linetomove = nlines - (self->nLines - currentline - 1);
	    
	    if (linetomove < self->nLines && linetomove > 0)  {
		if (self->scroll == textview_ScrollBackward) self->scroll = textview_MultipleScroll;
		(self)->SetTopPosition( (self->lines[linetomove].data)->GetPos());
	    }
	    else
		if ( self->editor == VI )
		{
		    (self->imPtr)->argState.argProvided = TRUE;
		    (self->imPtr)->argState.argument  = nlines;
		    textview_ViGlitchUpCmd(self);
		}
		else
		    (self)->FrameDot( npos);
	}
    }
    else  {
	/* Current Position is off the screen */
	if (self->scroll == textview_ScrollForward)  {
	    linetomove += nlines;
	    if (linetomove < self->nLines)
		(self)->SetTopPosition( (self->lines[linetomove].data)->GetPos());
	    else
		(self)->FrameDot( npos);
	}
	else  {
	    long topPos;

	    currentline = (self)->FindLineNumber( startPos);
	    if (currentline != -1 || (startPos <= (topPos = (self)->GetTopPosition()) && startPos >= topPos))
		(self)->FrameDot( npos);
	}
    }

    (&tm)->SetPos( npos);
    (&tm)->SetLength( 0);
    if (self->csxPos != -1)  {
	struct formattinginfo info;

	npos = (self)->LineRedraw ( textview_GetPosition, &tm, self->bx,
	    self->by, (self)->GetLogicalWidth()-2*self->bx-self->para_width-self->hz_offset,
	    (self)->GetLogicalHeight()-2*self->by, xpos-self->para_width-self->hz_offset, (int *) 0, NULL, &info);
    }
    (self)->SetDotPosition(npos);
    ((self)->GetIM())->SetLastCmd( lcMove);
    (self)->WantUpdate( self);
}

void textview_NextLineCmd (class textview  *self /**/)
{
    if(self->editor==VI) {
	textview_VINextLineCmd(self);
	return;
    }
    long npos, i, j;
    int xpos;
    class mark tm;	/* note this mark is not on the document's marker chain */
    long pos, nlPos;
    long curx, cury, xs, ys;
    struct rectangle logical;
    self->GetLogicalBounds(&logical);
    struct formattinginfo info;
    textview_ComputeViewArea(self, logical, curx, cury, xs, ys);

    pos = (self)->CollapseDot();

    if ((self->imPtr)->GetLastCmd() == lcMove)
	xpos = self->movePosition;
    else
	self->movePosition = xpos = self->cexPos;

    j=self->FindLineNumber(pos);
    if(j<0) {
	npos=textview_LineStart(self, curx, cury, xs, ys, pos);
	for(j=self->GetIM()->Argument();j>0;j--) {
	    tm.SetPos(npos);
	    tm.SetLength(0);
	    self->LineRedraw(textview_GetHeight, &tm, curx, cury, xs, ys, 0, NULL, NULL, &info);
	    npos+=info.lineLength;
	}
	struct rectangle hit;
	long off;
	nlPos=textview_ExposeRegion(self, npos, 1, NULL, logical, hit, off, logical.height/3);
	self->SetTopOffTop(nlPos, off);
    } else {

	j+=self->GetIM()->Argument();
	long height=0;
	if(j>=self->nLines && self->nLines>0) {
	    struct linedesc *ld=(&self->lines[self->nLines-1]);
	    long bpos=ld->data->GetPos()+ld->nChars;
	    j-=self->nLines;
	    if(ld->y+ld->height>ys) height=ld->y+ld->height-ys;
	    tm.SetPos(bpos);
	    for(i=0;i<=j && bpos<=(Text(self))->GetLength();i++) {
		tm.SetPos(bpos);
		tm.SetLength(0);
		height+=self->LineRedraw(textview_GetHeight, &tm, curx, cury, xs, ys, 0, NULL, NULL, &info);
		bpos+=info.lineLength;
	    }
	    npos=tm.GetPos();
	    if(npos!=self->GetDotPosition()) {
		bpos=self->GetTopPosition();
		height+=self->pixelsReadyToBeOffTop;
		while(height>0) {
		    long lheight;
		    tm.SetPos(bpos);
		    tm.SetLength(0);
		    lheight=self->LineRedraw(textview_GetHeight, &tm, curx, cury, xs, ys, 0, NULL, NULL, &info);
		    if(lheight>height) break;
		    bpos+=info.lineLength;
		    height-=lheight;
		}
		if(bpos==npos) height=0;
		self->SetTopOffTop(bpos, height);
	    }
	} else {
	    if(self->nLines>0) npos=self->lines[j].data->GetPos();
	    else npos=pos;
	}
    }
    (&tm)->SetPos( npos);
    (&tm)->SetLength( 0);
    if (self->csxPos != -1)  {
	struct formattinginfo info;

	npos = (self)->LineRedraw ( textview_GetPosition, &tm, self->bx,
	    self->by, (self)->GetLogicalWidth()-2*self->bx-self->hz_offset-self->para_width,
	    (self)->GetLogicalHeight()-2*self->by, xpos-self->hz_offset-self->para_width, (int *) 0, NULL, &info);
    }
    (self)->SetDotPosition(npos);
    ((self)->GetIM())->SetLastCmd( lcMove);
    (self)->WantUpdate( self);
}

void textview_EndOfTextCmd(class textview  *self)
{
    class text *d;
    int e;

    textview_CtrlAtCmd(self);
    d = Text(self);
    e = (d)->GetLength();
    (self)->SetDotPosition(e);
    (self)->SetDotLength(0);
    (self)->FrameDot(e);
    (self)->WantUpdate( self);
}

void textview_BeginningOfTextCmd(class textview  *self)
{
    textview_CtrlAtCmd(self);
    (self)->SetDotPosition(0);
    (self)->SetDotLength(0);
    (self)->FrameDot(0);
    (self)->WantUpdate( self);
}

void textview_EndOfLineCmd(class textview *self)
{
    int startpos, npos, dsize;
    class text *d;

    d = Text(self);
    startpos= self->GetDotPosition();
    if ( self->editor == VI )
	npos = d->GetEndOfLine( startpos);
    else if ((dsize = self->GetDotLength()) > 0)
	npos = startpos + dsize;
    else {
	dsize = d->GetLength();
	npos = self->MoveBack( startpos, 0 , textview_MoveByLines, NULL, NULL);
	npos = self->MoveForward( npos, 1, textview_MoveByLines, NULL, NULL);
	if (npos > 0) {
	    if (npos == dsize) {
		if (d->GetChar( npos-1) == '\n') npos--;
		if (npos < startpos) npos = startpos;
	    } else if (d->GetChar( npos-1) == ' ' )
		while (npos > startpos && (d->GetChar( npos-1)) == ' ') npos--;
	    else if (d->GetChar( npos-1) == '\n') npos--;
	}
    }
    self->SetDotPosition( npos);
    self->SetDotLength( 0);
    self->FrameDot( npos);
    self->WantUpdate( self);
}

void textview_BeginningOfLineCmd(class textview *self)
{
    int pos;

    pos = self->GetDotPosition();
    if ( self->editor == VI )
	pos = (Text(self))->GetBeginningOfLine( pos);
    else if (self->GetDotLength() == 0)
	pos = self->MoveBack( pos,0, textview_MoveByLines, 0, 0);
    self->SetDotPosition( pos);
    self->SetDotLength( 0);
    self->FrameDot( pos);
    self->WantUpdate( self);
}

void textview_EndOfWSWordCmd(class textview  *self)
    {/**/
    long	pos, textSize;
    int	j, ct;
    /***/
    ct	= ((self)->GetIM())->Argument();
    ((self)->GetIM())->ClearArg();
    textSize = (Text(self))->GetLength();
    for (j = 0; j < ct; j++)
    {
	do
	{
	    textview_EndOfWordCmd(self);
	    pos = (self)->GetDotPosition();
	} while ( pos < textSize && charType((Text(self))->GetChar( pos)) != WHITESPACE );
    }
}

void textview_ForwardWSWordCmd(class textview  *self)
    {/**/
    long	pos, textSize;
    int	j, ct;
    /***/
    ct	= ((self)->GetIM())->Argument();
    ((self)->GetIM())->ClearArg();
    textSize = (Text(self))->GetLength();
    for (j = 0; j < ct; j++)
    {
	do
	{
	    textview_ForwardWordCmd(self);
	    pos = (self)->GetDotPosition();
	} while ( pos < textSize && charType((Text(self))->GetChar( pos )) != WHITESPACE );
    }
}

void textview_ForwardBeginWSWordCmd(class textview  *self)
{
    long pos, textSize;
    text *d = Text(self);

    textview_ForwardWSWordCmd(self);
    textSize = d->GetLength();
    pos = self->GetDotPosition();
    while (pos < textSize && charType(d->GetChar(pos)) != WORD)
	pos++;
    self->SetDotPosition(pos);
    self->FrameDot(pos);
    self->WantUpdate(self);
}

void textview_BackwardWSWordCmd(class textview  *self)
    {/**/
    long		pos;
    int	j, ct;
    /***/
    ct	= ((self)->GetIM())->Argument();
    ((self)->GetIM())->ClearArg();
    for (j = 0; j < ct; j++)
    {
	do
	{
	    textview_BackwardWordCmd(self);
	    pos = (self)->GetDotPosition();
	} while ( pos > 0 && charType((Text(self))->GetChar( pos - 1)) != WHITESPACE );
    }
}

void textview_BeginningOfFirstWordCmd(class textview  *self)
    {
    class text *d;
    char	c;

    ((self)->GetIM())->ClearArg(); 
    d=Text(self);

    textview_BeginningOfLineCmd(self);
    c = (d)->GetChar( (self)->GetDotPosition());
    if ( charType(c) == WHITESPACE && c != '\n' )
	textview_ForwardWordCmd(self);
}

void textview_BeginningOfPreviousLineCmd(class textview  *self)
    {
    int	ct;
    int    j;

    ct = ((self)->GetIM())->Argument();
    ((self)->GetIM())->ClearArg(); 

    for (j = 0; j < ct; j++)
    {
    textview_PreviousLineCmd(self);
    textview_BeginningOfFirstWordCmd(self);
    }
}

void textview_BeginningOfNextLineCmd(class textview  *self)
    {
    int	ct;
    int    j;

    ct = (self->imPtr)->Argument();
    (self->imPtr)->ClearArg(); 

    for (j = 0; j < ct; j++)
    {
    textview_NextLineCmd(self);
    textview_BeginningOfFirstWordCmd(self);
    }
}

void textview_CursorToLine(class textview  *self, long  line)
{
    if (line > 0 && line <= self->nLines) {
        (self)->SetDotPosition( (self->lines[line - 1].data)->GetPos());
        (self)->SetDotLength( 0);
        (self)->WantUpdate( self);
    }
}

/* Useful commands for our editor. */

void textview_CursorToTop(class textview  *self, long  key)
{
    textview_CursorToLine(self, 1);
}

void textview_CursorToCenter(class textview  *self, long  key)
{
    textview_CursorToLine(self, self->nLines / 2);
}

void textview_CursorToBottom(class textview  *self, long  key)
{
    textview_CursorToLine(self, self->nLines);
}

void textview_GoToLineCmd(class textview  *self)
    {

    int argument, pos;
    class text *text;

    argument = ((self)->GetIM())->Argument();
    if ( !((self)->GetIM())->ArgProvided() )
	textview_EndOfTextCmd(self);
    else
    {
	text = (class text *) self->dataobject;
	(self)->SetDotLength( 0);
	pos = (text)->GetPosForLine( argument);
	(self)->SetDotPosition( pos);
	(self)->FrameDot( pos);
	(self)->WantUpdate( self);
    }
}

void textview_ExtendForwardCmd(class textview *self)
{
    long len, pos, newlen;
    int extendcnt = self->GetIM()->Argument();

    len = self->GetDotLength();
    pos = self->GetDotPosition();
    if (self->extendDirection == textview_ExtendRight) {
	/* We've been extending to the right.  Keep going. */
	self->SetDotLength(len+extendcnt);
	self->FrameDot(pos+len+extendcnt);
    } else {
	/* We've been extending to the left.  Back up. */
	if (len >= extendcnt) {
	    /* Shorten selected region. */
	    self->SetDotPosition(pos + extendcnt);
	    self->SetDotLength(len - extendcnt);
	    self->FrameDot(pos + extendcnt); /* Frame left */
	} else {
	    /* Extend in the other direction. */
	    newlen = extendcnt - len;
	    self->SetDotPosition(pos + len);
	    self->SetDotLength(newlen);
	    self->extendDirection = textview_ExtendRight;
	    self->FrameDot(pos + len + newlen); /* Frame right */
	}
    }
    self->WantUpdate(self);
}

void textview_ExtendBackwardCmd(class textview *self)
{
    long len, pos, newlen;
    int extendcnt = self->GetIM()->Argument();

    len = self->GetDotLength();
    pos = self->GetDotPosition();
    if (self->extendDirection == textview_ExtendRight) {
	/* We've been extending to the right.  Back up. */
	if (len >= extendcnt) {
	    /* Shorten selected region. */
	    self->SetDotPosition(pos);
	    self->SetDotLength(len - extendcnt);
	    self->FrameDot(pos+len);	/* Frame right */
	} else {
	    /* Extend in the other direction. */
	    newlen = extendcnt - len;
	    if (newlen > pos)
		newlen = pos;
	    self->SetDotPosition(pos - newlen);
	    self->SetDotLength(newlen);
	    self->extendDirection = textview_ExtendLeft;
	    self->FrameDot(pos);	/* Frame left */
	}
    } else {
	/* We've been going left.  Extend to the left. */
	if (extendcnt > pos)
	    extendcnt = pos;
	self->SetDotPosition(pos-extendcnt);
	self->SetDotLength(len+extendcnt);
	self->FrameDot(pos);	/* Frame left */
    }
    self->WantUpdate(self);
}

void textview_ExtendForwardLineCmd(class textview *self)
{
    long len, pos, newpos;

    len = self->GetDotLength();
    pos = self->GetDotPosition();
    if (self->extendDirection == textview_ExtendRight) {
	/* We've been extending to the right (down).  Keep going. */
	self->CollapseDot();
	textview_NextLineCmd(self);
	newpos = self->GetDotPosition();
	self->SetDotPosition(pos);
	self->SetDotLength(newpos-pos);
    } else {
	/* We've been going left (up).  Back up. */
	self->SetDotLength(0);
	textview_NextLineCmd(self);
	newpos = self->GetDotPosition();
	if (newpos >= pos+len) {
	    /* gone too far...extend the other way. */
	    self->SetDotPosition(newpos);
	    self->SetDotLength(newpos-pos-len);
	    self->extendDirection = textview_ExtendRight;
	} else {
	    self->SetDotPosition(newpos);
	    self->SetDotLength(len+pos-newpos);
	}
    }
}


void textview_ExtendBackwardLineCmd(class textview *self)
{
    long len, pos, newpos;

    len = self->GetDotLength();
    pos = self->GetDotPosition();
    if (self->extendDirection == textview_ExtendRight) {
	/* We've been extending to the right (down).  Back up. */
	self->CollapseDot();
	textview_PreviousLineCmd(self);
	newpos = self->GetDotPosition();
	if (newpos >= pos) {
	    self->SetDotPosition(pos);
	    self->SetDotLength(newpos-pos);
	} else {
	    /* gone too far...extend the other way. */
	    self->SetDotPosition(newpos);
	    self->SetDotLength(pos-newpos);
	    self->extendDirection = textview_ExtendLeft;
	}
    } else {
	/* We've been going left (up).  Extend to the left. */
	self->SetDotLength(0);
	textview_PreviousLineCmd(self);
	newpos = self->GetDotPosition();
	self->SetDotPosition(newpos);
	self->SetDotLength(len+pos-newpos);
    }
}

void textview_ExtendForwardScreenCmd(class textview *self)
{
    long len, pos, newpos;

    len = self->GetDotLength();
    pos = self->GetDotPosition();
    if (self->extendDirection == textview_ExtendRight) {
	/* We've been extending to the right (down).  Keep going. */
	self->CollapseDot();
	textview_NextScreenMoveCmd(self);
	newpos = self->GetDotPosition();
	self->SetDotPosition(pos);
	self->SetDotLength(newpos-pos);
	self->FrameDot(newpos);	/* Frame bottom */
    } else {
	/* We've been going left (up).  Back up. */
	self->SetDotLength(0);
	textview_NextScreenMoveCmd(self);
	newpos = self->GetDotPosition();
	if (newpos >= pos+len) {
	    /* gone too far...extend the other way. */
	    self->SetDotPosition(newpos);
	    self->SetDotLength(newpos-pos-len);
	    self->extendDirection = textview_ExtendRight;
	} else {
	    self->SetDotPosition(newpos);
	    self->SetDotLength(len+pos-newpos);
	}
	self->FrameDot(newpos);			/* Frame top */
    }
    self->WantUpdate(self);
}


void textview_ExtendBackwardScreenCmd(class textview *self)
{
    long len, pos, newpos;

    len = self->GetDotLength();
    pos = self->GetDotPosition();
    if (self->extendDirection == textview_ExtendRight) {
	/* We've been extending to the right (down).  Back up. */
	self->CollapseDot();
	textview_PrevScreenMoveCmd(self);
	newpos = self->GetDotPosition();
	if (newpos >= pos) {
	    self->SetDotPosition(pos);
	    self->SetDotLength(newpos-pos);
	} else {
	    /* gone too far...extend the other way. */
	    self->SetDotPosition(newpos);
	    self->SetDotLength(pos-newpos);
	    self->extendDirection = textview_ExtendLeft;
	}
	self->FrameDot(newpos);			/* Frame bottom */
    } else {
	/* We've been going left (up).  Extend to the left. */
	self->SetDotLength(0);
	textview_PrevScreenMoveCmd(self);
	newpos = self->GetDotPosition();
	self->SetDotPosition(newpos);
	self->SetDotLength(len+pos-newpos);
	self->FrameDot(newpos);			/* Frame top */
    }
    self->WantUpdate(self);
}


/*
 * Extend dot to the end of the text.
 */
void textview_ExtendEndCmd(class textview *self)
{
    long len, pos;
    text *d = Text(self);

    pos = self->GetDotPosition();
    len = d->GetLength() - pos;
    self->SetDotLength(len);
}

/*
 * Extend dot to the beginning of the text.
 */
void textview_ExtendBeginCmd(class textview *self)
{
    long len;

    len = self->GetDotPosition() + self->GetDotLength();
    self->SetDotPosition(0);
    self->SetDotLength(len);
}

/* This is the default horizontal scroll interval (pixels).
 * We could parse numbers like 30%, 3cm, etc.
 */
int textview_get_hshift_interval(textview *self)
{
    static int hshift_interval = 0;
    if (hshift_interval == 0) {
	hshift_interval = environ::GetProfileInt("TextHorizontalShiftInterval", 30);
	if (hshift_interval <= 0)
	    hshift_interval = 30;
    }
    return hshift_interval;
}

/* Shift the view left n pixels.  This means the text goes right.
 */
void textview_ShiftLeftCmd(textview *self, char *arg)
{
    if (self->hz_offset == 0)
	return;	/* skip the update */
    self->hz_offset += textview_get_hshift_interval(self);
    if (self->hz_offset > 0)
	self->hz_offset = 0;
    self->force = TRUE;
    self->WantUpdate(self);
}

/* Shift the view right n pixels.  This means the text goes left. */
void textview_ShiftRightCmd(textview *self, char *arg)
{
    int viewwidth = self->GetLogicalWidth() - ((self->hasApplicationLayer) ? self->bx : self->ebx) * 2;
    int txtwidth = textview_SizeOfLongestLineOnScreen(self);
    if (txtwidth < viewwidth)
	txtwidth = viewwidth;
    self->hz_offset -= textview_get_hshift_interval(self);
    if (viewwidth - self->hz_offset > txtwidth)
	self->hz_offset = viewwidth - txtwidth;
    self->force = TRUE;
    self->WantUpdate(self);
}

/* Shift the view to the "home" position. */
void textview_ShiftHomeCmd(textview *self, char *arg)
{
    self->hz_offset = 0;
    self->force = TRUE;
    self->WantUpdate(self);
}
