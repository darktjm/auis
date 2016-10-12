/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

#include <andrewos.h>
ATK_IMPL("frameview.H")
#include <im.H>
#include <event.H>
#include <graphic.H> /* Prevents include nesting problems. */
#include <keymap.H>
#include <proctable.H>
#include <keystate.H>
#include <buffer.H>
#include <mark.H>
#include <text.H>
#include <bind.H>
#include <menulist.H>
#include <message.H> /* For message_HelpItem enum declaration. */
#include <framemessage.H>
#include <frame.H>
#include <style.H>
#include <stylesheet.H>
#include <fontdesc.H>
#include <environ.H>

#include <frameview.H>

static class keymap *frameviewKeymap;
static class menulist *frameviewMenulist;

#define Text(self) ((class text *) self->GetDataObject())


ATKdefineRegistry(frameview, textview, frameview::InitializeClass);
static int CalculateLineHeight(class frameview  *self);
static void EraseMessage(class frameview  *self);
static void CleanMessageState (class frameview  *self);
static enum keymap_Types KeyEraseMessage(long s, long  key, struct proctable_Entry **entry, long  *rockP);
static void TransientMessage(class frameview  *self, const char  *message);
static long Punt(ATK *self, long  key);
static void CompletionMessage(class frameview  *self, enum message_CompletionCode  code);
static long InsertSorted(class text  *doc, long  pos, char  *string);
static void HelpWork(struct helpRock  *helpRock, enum message_HelpItem  helpType, char  *itemString, char  *itemInfo);
static long Kill(ATK  *self, long  key);
static long Minimize(ATK  *self, long  key);


frameview::frameview()
        {
	ATKinit;


    class style *defaultStyle;

    this->messageLine = NULL;
    this->keystatep = keystate::Create(this, frameviewKeymap);
    this->menulistp = (frameviewMenulist)->DuplicateML( this);
    this->eventp = NULL;
    this->transientMark = NULL;
    this->amLosingInputFocus = FALSE;
    (this)->SetBorder( 2, 2);
    /* This next piece of code depends on textview allocating a new style for every textview's default style. That way we can change the meaning of this style for our textview only without copying the style. If the textview behavior changes, this will have to change as well. */
    if ((defaultStyle = (this)->GetDefaultStyle()) != NULL) {

        (defaultStyle)->SetJustification( style_LeftJustified);
    }
    this->lineHeight = (-1);
    this->minlines=environ::GetProfileInt("MinimumMessageLines", 1);
    this->lines = this->minlines;
    this->wantsize = 0;
    this->dynamicsize= environ::GetProfileSwitch("DynamicMessageLineSize", FALSE);
    
    THROWONFAILURE( TRUE);
}

frameview::~frameview()
        {
	ATKinit;


    delete this->keystatep;
    if(this->menulistp) delete this->menulistp;
    if (this->transientMark != NULL) {
        (this->messageLine->messageText)->RemoveMark( this->transientMark);
        delete this->transientMark;
    }
}

class frameview *frameview::Create(class framemessage  *messageLine)
{
	ATKinit;


    class frameview *temp = new frameview;

    (temp)->SetMessageLine( messageLine);
    return temp;
}

#define DEFAULTHEIGHT 22
static int CalculateLineHeight(class frameview  *self)
{

    class style *defaultStyle;
    class fontdesc *defaultFont;
    struct FontSummary *fontSummary;
    char fontFamily[256];
    long refBasis, refOperand, fontSize;

    if ((defaultStyle = (self)->GetDefaultStyle()) == NULL)
        return DEFAULTHEIGHT;
    (defaultStyle)->GetFontFamily( fontFamily, sizeof (fontFamily));
    (defaultStyle)->GetFontSize( (enum style_FontSize *) &refBasis, &fontSize);
    refOperand = (defaultStyle)->GetAddedFontFaces();
    defaultFont = fontdesc::Create(fontFamily, refOperand, fontSize);
    if ((fontSummary = (defaultFont)->FontSummary( (self)->GetDrawable())) == NULL)
        return DEFAULTHEIGHT;
    return fontSummary->maxHeight + 4; /* Two for top and bottom border. */
}


view::DSattributes frameview::DesiredSize(long  width, long  height, enum view::DSpass  pass, long  *dWidth, long  *dHeight)
{
    
    *dWidth = width;
    
    if((this)->GetIM()==NULL) {
	*dHeight = DEFAULTHEIGHT;
    } else {
	if(this->lineHeight==-1) this->lineHeight=CalculateLineHeight(this);
	*dHeight = this->lineHeight * this->lines;
    }
    return (view::DSattributes)(view::HeightFlexible | view::WidthFlexible);
}

void frameview::Update()
{

    class text *t=(class text *)(this)->GetDataObject();
    long pos;
    long lines;
    
    if(this->lineHeight==-1) this->lineHeight=CalculateLineHeight(this);
    
    lines = (this)->GetLogicalHeight()/this->lineHeight;
    if(lines<this->minlines) lines=this->minlines;
    if(this->wantsize && this->lines>lines) this->wantsize=(-1);
    this->lines=lines;
    
    if ((this)->GetDotPosition() < (pos = (Text(this))->GetFence())) {
	(this)->SetDotPosition( pos);
	(this)->SetDotLength( 0);
    }
    (this)->textview::Update();
    pos = (t)->GetLength();
    if(this->wantsize==0 && ((this->dynamicsize && ((this)->GetTopPosition()!=0 || (pos>0 && !(this)->Visible( pos-1)))) || this->lines<this->minlines))  {
	long lines = (t)->GetLineForPos( (t)->GetLength()) + 1;
	if(this->lines < lines) this->lines=lines;
	else this->lines++;
	if(this->lines<this->minlines) this->lines=this->minlines;
	(this)->WantNewSize( this);
	(this)->SetTopPosition( 0);
    }
}

void frameview::SetWantedLines(int  lines)
{
    if(this->dynamicsize && this->lines!=lines) {
	if(lines<this->minlines) {
	    if(this->lines==this->minlines) return;
	    lines=this->minlines;
	}
	this->lines=lines;
	(this)->WantNewSize( this);
    }
}

void frameview::WantNewSize(class view  *req)
{
    if(this!=req) return;
    if(this->wantsize>0) return;
    this->wantsize=1;
    (this)->textview::WantNewSize( req);
    im::ForceUpdate();
    this->WantUpdate(this);
    im::ForceUpdate();
    if(this->wantsize==1) this->wantsize=0;
}

void frameview::LinkTree(class view *parent)
{
    const char *fgcolor, *bgcolor;

    textview::LinkTree(parent);
    if (GetIM()) {
	/* Part of a valid view tree.  Set the foreground/background colors. */
	fgcolor = environ::GetProfile("MessageLineForegroundColor");
	bgcolor = environ::GetProfile("MessageLineBackgroundColor");
	if (fgcolor)
	    SetForegroundColor(fgcolor, 0, 0, 0);
	if (bgcolor)
	    SetBackgroundColor(bgcolor, 0, 0, 0);
    }
}

void frameview::WantInputFocus(class view  *requestor)
        {

    if (requestor != (class view *) this || (this->messageLine)->Asking())
        (this)->textview::WantInputFocus( requestor);
}



void frameview::LoseInputFocus()
    {
    (this)->textview::LoseInputFocus();

    if ((! this->amLosingInputFocus) && (this->messageLine)->Asking() && (!(this->messageLine->flags & message_Mandatory))) {
       /* self->messageLine->oldInputFocus = NULL; is this needed? --rr2b Aug92 */
        (this->messageLine)->CancelQuestion();
        (this->messageLine)->DisplayString( 0, "Cancelled.");
    }
    this->amLosingInputFocus = FALSE;
}

void frameview::PostMenus(class menulist  *menulistp)
        {
    if (menulistp != this->menulistp) {
        (this->menulistp)->ClearChain();
        (this->menulistp)->ChainAfterML( menulistp, (long) menulistp);
    }
    (this)->textview::PostMenus( this->menulistp);
}

void frameview::PostKeyState(class keystate  *keystate)
        {

    if (this->messageLine->keystatep != NULL) {
        (this->keystatep)->AddBefore( keystate);
        (this->messageLine->keystatep)->AddBefore( this->keystatep);
        (this)->textview::PostKeyState( this->messageLine->keystatep);
    }
    else {
        (this->keystatep)->AddBefore( keystate);
        (this)->textview::PostKeyState( this->keystatep);
    }
}

void frameview::SetMessageLine(class framemessage  *messageLine)
        {

    if (this->messageLine != NULL) {

        (this->messageLine->messageText)->RemoveMark( this->transientMark);
        delete this->transientMark;

        if ((this->messageLine)->Asking())
            (this->messageLine)->CancelQuestion();
    }

    this->messageLine = messageLine;
    this->transientMark = (messageLine->messageText)->CreateMark( 0, 0);
    (this->transientMark)->SetStyle( FALSE, FALSE);
}

class framemessage *frameview::GetMessageLine()
    {

    return this->messageLine;
}

static void EraseMessage(class frameview  *self)
    {

    self->eventp = NULL;
    if ((self->transientMark)->GetLength() == 0) /* Be safe... */
        return;
    (Text(self))->AlwaysDeleteCharacters( (self->transientMark)->GetPos(), (self->transientMark)->GetLength());
    (self)->WantUpdate( self);
}
static void CleanMessageState (class frameview  *self)
    {
    if (self->eventp != NULL)
        (self->eventp)->Cancel();

    EraseMessage(self);
    (self->keystatep)->SetOverride( NULL, 0);
}

static enum keymap_Types KeyEraseMessage(long s, long  key, struct proctable_Entry **entry, long  *rockP)
{
    class frameview  *self=(class frameview *)s;
    CleanMessageState(self);
    return (self->keystatep->curMap)->Lookup( key, (ATK **)entry, rockP);
}

static void TransientMessage(class frameview  *self, const char  *message)
        {

    int pos, len;

    pos = (Text(self))->GetLength();
    (self->transientMark)->SetPos( pos);
    (Text(self))->AlwaysInsertCharacters( pos, message, len = strlen(message));
    (self->keystatep)->SetOverride( KeyEraseMessage, (long) self);
     (self)->WantUpdate( self);
    (self->transientMark)->SetLength( len);
    self->eventp = im::EnqueueEvent((event_fptr) EraseMessage, (char *) self, event_SECtoTU(4));
}

static long Punt(ATK *s, long  key)
        {
    class frameview  *self=(class frameview *)s;
    if (!(self->messageLine->flags & message_Mandatory)) {
        (self->messageLine)->CancelQuestion();
        (self->messageLine)->DisplayString( 0, "Cancelled.");
    }
    else
	(self->messageLine)->DisplayString( 0, "Question must be answered.");
    return 0;
}

static void CompletionMessage(class frameview  *self, enum message_CompletionCode  code)
        {
    switch (code) {
        case message_Invalid:
            TransientMessage(self, "  [No Match]");
            break;
        case message_Valid:
            TransientMessage(self, "  [Incomplete]");
            break;
        case message_Complete:
            TransientMessage(self, "  [Confirm]");
            break;
        case message_CompleteValid:
            TransientMessage(self, "  [Others]");
            break;
    }
}

void frameview::Return(long  key)
        {
    class framemessage *messageLine = this->messageLine;
    int startpos, endpos;

    /* give feedback that Return was pressed   -wjh */
    this->amLosingInputFocus = TRUE;
    (this)->WantInputFocus( NULL);
    im::ForceUpdate( );		/* display text with it all highlighted */

    if ((this->transientMark)->GetLength() != 0) CleanMessageState(this);
    if ((messageLine->flags & message_MustMatch) 
		&& (messageLine->completionProc != NULL)) {
	/* the MustMatch flag is on and there is a completion proc::
		check to see if the string entered matches */
        (messageLine)->GetCurrentString( messageLine->textBuffer, 
				messageLine->maxTextSize);
        if (!((messageLine->flags & message_NoInitialString) &&
              *messageLine->textBuffer == '\0' && messageLine->hasDefault)) {
	    /* If not default... */

            enum message_CompletionCode code;

            code = (*messageLine->completionProc)(messageLine->textBuffer, 
			messageLine->completionData, messageLine->textBuffer,
			messageLine->maxTextSize);
            if ((code != message_Complete) && (code != message_CompleteValid)) {
		CompletionMessage(this, code);
		(this)->WantInputFocus( this);
		im::ForceUpdate();
                return;		/* user is NOT really done. */
            }
            else {
		/* copy the possibly-modified text back to the display text */
                startpos = (Text(this))->GetFence();
                endpos = (Text(this))->GetLength();

                (Text(this))->AlwaysDeleteCharacters( startpos, 
				endpos - startpos);
                (Text(this))->AlwaysInsertCharacters( startpos, 
				messageLine->textBuffer, 
				strlen(messageLine->textBuffer));
            }
        }
    }

    if ((messageLine)->Asking())
        im::KeyboardExit();
}

void frameview::Complete(long  key)
        {
    class framemessage *messageLine = this->messageLine;
    enum message_CompletionCode code;
    int len, startpos, endpos;

    if ((this->transientMark)->GetLength() != 0)
        CleanMessageState(this);

    if (messageLine->completionProc != NULL) {
        (messageLine)->GetCurrentString(
            messageLine->textBuffer, messageLine->maxTextSize);

        code = (*messageLine->completionProc)(messageLine->textBuffer,
            messageLine->completionData, messageLine->textBuffer,
            messageLine->maxTextSize);

        if ((code != message_Invalid) ||
          (this->messageLine->flags & message_MustMatch)) {
	    startpos = (Text(this))->GetFence();
	    endpos = (Text(this))->GetLength();
	    (Text(this))->AlwaysDeleteCharacters( startpos, endpos - startpos);
            len = strlen(messageLine->textBuffer);
            (Text(this))->AlwaysInsertCharacters( startpos,
                messageLine->textBuffer, len);
	    (this)->SetDotPosition( startpos + len);
	    (this)->FrameDot( startpos + len);
        }
        CompletionMessage(this, code);
    } else if (this->keystatep->next != NULL) {
        struct proctable_Entry *procTableEntry;
        ATK  *object;
	long rock;

        if ((this->keystatep->next)->ApplyKey(key,
          &procTableEntry, &rock, &object) == keystate_ProcFound)
            (this->keystatep->next)->DoProc( procTableEntry, rock, object);
    }
}

/* This function sucks. -Z- */
static long InsertSorted(class text  *doc, long  pos, char  *string)
            {
    int c, c2;
    long initPos, len;
    char *tempString;

    do {
        initPos = pos;
        tempString = string;
        while (((c = (doc)->GetChar( pos)) == *tempString) && (*tempString != '\0') && (c != EOF)) {
            pos++;
            tempString++;
        }
        while ((c2 = (doc)->GetChar( pos)) != '\n' && c2 != EOF)
            pos++;
        if (c2 == '\n')
            pos++;
    } while (c != EOF && (c < *tempString));
    if (c == EOF)
        initPos = pos;

    len = strlen(string);
    (doc)->InsertCharacters( initPos, string, len);
    return initPos + len;
}

struct helpRock {
    class text *doc;
    long insertPos;
};

static void HelpWork(struct helpRock  *helpRock, enum message_HelpItem  helpType, char  *itemString, char  *itemInfo)
{
    if (helpType == message_HelpGenericItem) {
        long len;

        len = strlen(itemString);
        (helpRock->doc)->InsertCharacters(
            helpRock->insertPos, itemString, len);
        helpRock->insertPos += len;
    } else { /* helpType == message_HelpListItem */
        long pos, len;

        pos = InsertSorted(helpRock->doc, helpRock->insertPos, itemString);

        if (itemInfo != NULL) {
            (helpRock->doc)->InsertCharacters( pos++, "\t", 1);
            len = strlen(itemInfo);
            (helpRock->doc)->InsertCharacters( pos, itemInfo, len);
            pos += len;
        }

        if ((helpRock->doc)->GetChar( pos) != '\n')
            (helpRock->doc)->InsertCharacters( pos++, "\n", 1);
    }
}

void frameview::Help(long  key)
{
    class framemessage *messageLine = this->messageLine;

    /* For now, there is no help for frames without buffers. */

    if ((this->transientMark)->GetLength() != 0)
        CleanMessageState(this);

    if (messageLine->helpProc != NULL /* &&
      frame_GetBuffer(messageLine->frame) != NULL */) {
        class buffer *helpBuffer = (messageLine->frame)->GetHelpBuffer();

        if (helpBuffer != NULL && (helpBuffer)->GetData() != NULL) {
            class text *helpDoc = (class text *) (helpBuffer)->GetData();
            struct helpRock helpRock;

            (helpDoc)->Clear();

            helpRock.doc = helpDoc;
            helpRock.insertPos = 0;

            (messageLine)->GetCurrentString(
              messageLine->textBuffer, messageLine->maxTextSize);

            (*messageLine->helpProc)(messageLine->textBuffer, messageLine->completionData, (message_workfptr)HelpWork, (long)&helpRock);

	    if (helpRock.insertPos == 0) {
                const char *s;
		int len;
                static class style *boldStyle = NULL,
                   *ulineStyle = NULL, *fixedStyle = NULL;

                if (boldStyle == NULL) {
                    boldStyle = new style;
                    (boldStyle)->AddNewFontFace( fontdesc_Bold);
                    ulineStyle = new style;
                    (ulineStyle)->AddUnderline();
                    fixedStyle = new style;
                    (fixedStyle)->SetFontFamily( "andytype");
                    (fixedStyle)->AddNewFontFace( fontdesc_Fixed);
                    (fixedStyle)->SetFontSize( style_PreviousFontSize, -2);
                    (fixedStyle)->AddNoWrap();
                }

                s = "Name               Size Sav Object   File\n";
                (helpDoc)->InsertCharacters( 0, s, strlen(s));
                (helpDoc)->AddStyle( 0, 4, ulineStyle);
                (helpDoc)->AddStyle( 19, 4, ulineStyle);
                (helpDoc)->AddStyle( 24, 3, ulineStyle);
                (helpDoc)->AddStyle( 28, 6, ulineStyle);
                (helpDoc)->AddStyle( 37, 4, ulineStyle);

                (helpDoc)->AddStyle( 0, (helpDoc)->GetLength(), fixedStyle);

                if (messageLine->textBuffer[0] == '\0') {
                    s = "Possible completions:\n\n";
                    len = strlen(s);
                    (helpDoc)->InsertCharacters( helpRock.insertPos, s, len);
                    helpRock.insertPos += len;
                } else {
                    s = "Possible completions for ``";
                    len = strlen(s);
                    (helpDoc)->InsertCharacters( helpRock.insertPos, s, len);
                    helpRock.insertPos += len;

                    len = strlen(messageLine->textBuffer);
                    (helpDoc)->InsertCharacters( helpRock.insertPos,
                        messageLine->textBuffer, len);
                    helpRock.insertPos += len;

                    s = "'':\n\n";
                    len = strlen(s);
                    (helpDoc)->InsertCharacters( helpRock.insertPos, s, len);
                    helpRock.insertPos += len;
                }

                (helpDoc)->AddStyle( 0, helpRock.insertPos, boldStyle);
	    }

            /* class buffer *LastBuffer = */ (messageLine->frame)->SetHelpBuffer(
                helpBuffer, FALSE);
	/*    if ((messageLine->realBuffer == NULL))
		messageLine->realBuffer = LastBuffer;
	  */  
            (helpDoc)->NotifyObservers( 0);
        }
    } else if (this->keystatep->next != NULL) {
        struct proctable_Entry *procTableEntry;
        ATK  *object;
	long rock;

        if ((this->keystatep->next)->ApplyKey( (long) '?',
          &procTableEntry, &rock, &object) == keystate_ProcFound)
            (this->keystatep->next)->DoProc( procTableEntry, rock, object);
    }
}

static long Kill(ATK *s, long  key)
        {
    class frameview  *self=(class frameview *)s;
    int len = (Text(self))->GetLength();
    int pos = (Text(self))->GetFence();

    (Text(self))->DeleteCharacters( pos, len - pos);
    (self)->SetWantedLines( 1);
    (self)->WantUpdate( self);
    return 0;
}

static long Minimize(ATK *s, long  key)
{
    class frameview  *self=(class frameview *)s;
    self->lines=self->minlines;
    (self)->WantNewSize( self);
    return 0;
}

static long frameview_Return(ATK *f, long c)
{
    ((class frameview *)f)->Return(c);
    return 0;
}

static long frameview_Complete(ATK *f, long c)
{
    ((class frameview *)f)->Complete(c);
    return 0;
}

static long frameview_Help(ATK *f, long c)
{
    ((class frameview *)f)->Help(c);
    return 0;
}
static struct bind_Description frameviewBindings[]={
    {"frameview-punt","\003",0,NULL,0,0,Punt,"^G in message line."},
    {"frameview-punt","\007",0},
    {"frameview-punt","\030\003",0},
    {"frameview-return","\r",0,NULL,0,0,frameview_Return,"<CR> in message line."},
    {"frameview-complete"," ",' ',NULL,0,0,frameview_Complete,"Attempt to complete a user response."},
    {"frameview-kill","\025",0,NULL,0,0,Kill,"Kill to beginning of line."},
    {"frameview-help","?",0,NULL,0,0,frameview_Help,"Provide help on current question."},
    {"frameview-minimize", "\0301", 0, NULL, 0, 0, Minimize, "Reset the message line to the size needed for a single line."},
    {NULL, NULL, 0, "Plainer"},
    {NULL, NULL, 0, "Plainest"},
    {NULL, NULL, 0, "Quit"},
    {NULL, NULL, 0, "File,Insert File"},
    {NULL, NULL, 0, "File,Add Template"},
    {NULL, NULL, 0, "Search/Spell,Forward"},
    {NULL, NULL, 0, "Search/Spell,Backward"},
    {NULL, NULL, 0, "Search/Spell,Search Again"},
    {NULL, NULL, 0, "Search/Spell,Query Replace"},
    {NULL, NULL, 0, "Search/Spell,Check Spelling"},
    {NULL, NULL, 0, "Page,Insert Pagebreak"},
    {NULL, NULL, 0, "Page,Next Page"},
    {NULL, NULL, 0, "Page,Previous Page"},
    {NULL, NULL, 0, "Page,Insert Footnote"},
    {NULL, NULL, 0, "Page,Open Footnotes"},
    {NULL, NULL, 0, "Page,Close Footnotes"},
    {NULL, NULL, 0, "Page,Table of Contents"},
    NULL
};

boolean frameview::InitializeClass()
    {
    frameviewKeymap = new keymap;
    frameviewMenulist = new menulist;
    bind::BindList(frameviewBindings, frameviewKeymap, frameviewMenulist, &frameview_ATKregistry_ );
    return TRUE;
}
