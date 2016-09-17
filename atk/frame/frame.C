/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'     *
\* ********************************************************************** */

/*
	$Disclaimer: 
// Permission to use, copy, modify, and distribute this software and its 
// documentation for any purpose and without fee is hereby granted, provided 
// that the above copyright notice appear in all copies and that both that 
// copyright notice and this permission notice appear in supporting 
// documentation, and that the name of IBM not be used in advertising or 
// publicity pertaining to distribution of the software without specific, 
// written prior permission. 
//                         
// THE COPYRIGHT HOLDERS DISCLAIM ALL WARRANTIES WITH REGARD 
// TO THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF 
// MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL ANY COPYRIGHT 
// HOLDER BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL 
// DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, 
// DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE 
// OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION 
// WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
// 
//  $
*/

/* frame.c
 * Provides the toplevel view on a buffer and message line.
  */

#include <andrewos.h>
ATK_IMPL("frame.H")

#ifndef MAX
#define MAX(A,B) ((A > B) ? A: B)
#endif /* MAX */
#include <im.H>

#include <view.H>
#include <dataobject.H>
#include <buffer.H>
#include <style.H>
#include <environ.H>
#include <fontdesc.H>
#include <keystate.H>
#include <menulist.H>
#include <frameview.H>
#include <framecmds.H>
#include <framemessage.H>
#include <cursor.H>
#include <graphic.H>
#include <frame.H>
#include <proctable.H>
#include <message.H>
#include <path.H>

#include <text.H>
#include <dialogv.H>

#include <ctype.h>

static class frame *allFrames = NULL; /* Maintains a list of all frames for enumeration purposes. */
static class keymap *frameKeymap = NULL, *frameDefaultKeymap=NULL;
static class menulist *frameMenus = NULL, *frameDefaultMenus=NULL;












#define DEFAULTHEIGHT 20

#define ACCEPT 1
#define CANCEL 2
#define COMPLETE 3
#define HELP 4
#define frame_SetUpperShade(SELF,P) frame_setShade(SELF,((P)?200:(self->mono ?25:45)))
#define frame_SetLowerShade(SELF,P) frame_setShade(SELF,((P)?(self->mono ?25:45):200))
#define FORESHADE 30
struct pendingupdates {
    class view *v;
    struct pendingupdates *next;
};

#define frame_FONTSIZE 12
#define frame_FONTPAD 10 /* bogus */
#define OFFSET 5
#define BUTTONON 1
#define BUTTONOFF 3

#define frame_SEPARATION 2
#define frame_TWOSEPARATIONS (2*frame_SEPARATION) /* 4 */
#define frame_HALFPADDING (frame_TWOSEPARATIONS+1) /* 5 */
#define frame_TOTALPADDING ((2*frame_HALFPADDING)+2) /* 10 */
#define frame_DOUBLEPADDING (2*frame_TOTALPADDING) /* 20 */
#define MINSIDEBYSIDEWIDTH 400
static int frame_GlobalStackCount = 0; /* a hack to prevent ugly recursion */
static class keymap *mykm;
static struct proctable_Entry *returnconsidered, *cancel, *confirmAnswer, *gotkey;
static boolean QuitWindowOnlyDefault = -1; /* uninitialized */
static boolean ResizableMessageLine=TRUE;

static double foreground_color[3],background_color[3];
#define BUTTONDEPTH 3
#define BUTTONPRESSDEPTH 2
#define TEXTPAD 2


ATKdefineRegistry(frame, lpair, frame::InitializeClass);
static void frame_setShade(class frame  *self, int  val			/* 0 - 200*/);
static const char * GetProfileString (const char  *pref, const char  *defalt);
static void frame_CacheSettings(class frame  *self);
static void drawButton(class frame  * self,struct rectangle  *rect,const char  *text,boolean  pushed,boolean  borderonly,boolean  blit);
static int CalculateLineHeight(class frame  *self);
static void handleNewData(class frame  *self);
static void SetTitle(class frame  *self);
static void ConsiderReturning(class frame  *self, int  Choice);
static boolean InRectangle(struct rectangle  *r, long  x , long  y);
static void drawshadow(class frame  *self,struct rectangle  *r);
static void DoUpdate(class frame  *self);
static void SaveBits(class frame  *self);
static int ButtonInteract(class frame  *self, const char  **AnswerList, long  DefaultWildestAnswer , long  *WildestAnswer, int  flags);
static void PrepareMenus(class frame  *self);
static void PurifyString(char  *s);
static boolean RestoreBits(class frame  *self);
static void CannotRestoreBits(class frame  *self);
static void ComputeSize(class frame  *self);
static void GotKey(class frame  *self, char  c);
static void ConfirmDefaultAnswer(class frame  *self);
static void Cancel(class frame  *self);
static void retractCursors(class frame  *self);
static void TidyUp(class frame  *self);
static int isDialogChild(class frame  *self, class view  *v);
static void delete_window_request(class im  *im, class frame  *self);
static void ReturnInterface(class frame  *rock, int  ind, long  brock);
static class view *PrepareForStringInput(class frame  *self,const char  *prompt,int  bufferSize /* Is actual sizeof buffer including NUL. */,boolean  CompButtons);
static void SingleLine(class frame  *self, long  key);
static boolean FindBuffer(class frame  *f,class buffer  *b);


static void frame_setShade(class frame  *self, int  val			/* 0 - 200*/)
{
    double pct;
    if(val > 100){
/*	frame_SetFGColor(self,.95,.95,.95); */
	if (self->mono) pct = .75;
	else {
	    /*	frame_SetFGColor(self,.95,.95,.95); */
	    (self)->SetForegroundColor(NULL,105*256,105*256,105*256);
	    return;
	}
    }
    else if(val == 100){
	(self)->SetFGColor( 
			    foreground_color[0], 
			    foreground_color[1], 
			    foreground_color[2]);
	return ;
    }
    else if(val == 0){
	(self)->SetFGColor( 
			    background_color[0], 
			    background_color[1], 
			    background_color[2]);
	return;
    }
			    
    else pct = (double)val/ 100.;
    (self)->SetFGColor( 
			 foreground_color[0]* pct 
			 + background_color[0]*(1.0-pct), 
			 foreground_color[1]*pct 
			 + background_color[1]*(1.0-pct), 
			 foreground_color[2]*pct 
			 + background_color[2]*(1.0-pct));

} /* frame_setShade */

#define CFGCOLOR "white"
#define CBGCOLOR "blue"

#define BWFGCOLOR "black"
#define BWBGCOLOR "white"

static const char *
GetProfileString (const char  *pref, const char  *defalt)
          {
  const char *p = environ::GetProfile (pref);
  if (p == NULL)
    return (defalt);
  else
    return (p);
}

static void frame_CacheSettings(class frame  *self)
{
    const char *fgcolor, *bgcolor;
    unsigned char fg_rgb[3], bg_rgb[3];
    self->mono = ((self )->DisplayClass() & graphic_Monochrome);
    if ( self->mono || environ::GetProfileSwitch("frame.MonochromeDialogBoxes", FALSE)) {
	fgcolor = GetProfileString("frame.DialogBoxMonochromeForeground", BWFGCOLOR);
	bgcolor = GetProfileString("frame.DialogBoxMonochromeBackground", BWBGCOLOR);
    }
    else {  
	fgcolor = GetProfileString("frame.DialogBoxColorForeground", CFGCOLOR);
	bgcolor = GetProfileString("frame.DialogBoxColorBackground", CBGCOLOR);
    }
	
    if ((self)->GetIM() != NULL) {
	/* these calls don't make sense if there is no IM,
	    (in fact they seg fault!) */

	(self)->SetForegroundColor( fgcolor, fg_rgb[0]*256L, fg_rgb[1]*256L, fg_rgb[2]*256L);
	(self)->SetBackgroundColor( bgcolor, bg_rgb[0]*256L, bg_rgb[1]*256L, bg_rgb[2]*256L);
	(self)->GetFGColor( 
			 &(foreground_color[0]), 
			 &(foreground_color[1]), 
			 &(foreground_color[2]));
	(self)->GetBGColor( 
			 &(background_color[0]), 
			 &(background_color[1]), 
			 &(background_color[2]));
    }
}
static void drawButton(class frame  * self,struct rectangle  *rect,const char  *text,boolean  pushed,boolean  borderonly,boolean  blit)
{

    struct rectangle Rect2;
    int r2_bot, r_bot;
    int tx = 0, ty = 0;
    short t_op;
    long offset;
    struct rectangle r;
    r = *rect;
    r.left +=frame_SEPARATION; r.top +=frame_SEPARATION;
    r.width = self->buttonmaxwid + frame_TOTALPADDING - frame_TWOSEPARATIONS;
    r.height -= frame_TWOSEPARATIONS;
    rect = &r; /* BOGUS FIX THIS */
    offset = 0;
/*    frame_SetFont(self, self->activefont); */

    (self)->SetTransferMode( graphic_SOURCE);
    t_op = graphic_BETWEENLEFTANDRIGHT | graphic_BETWEENTOPANDBOTTOM;
    Rect2.top = rect->top + BUTTONDEPTH + offset;
    Rect2.left = rect->left + BUTTONDEPTH  + offset;
    Rect2.width = rect->width - 2*BUTTONDEPTH ;
    Rect2.height = rect->height - 2*BUTTONDEPTH ;
    r2_bot = (Rect2.top)+(Rect2.height);
    r_bot = (rect->top)+(rect->height);
    tx = Rect2.left + (Rect2.width/ 2);

    ty = Rect2.top + ( Rect2.height/ 2);

    (self)->SetTransferMode( graphic_COPY);
    if(!borderonly && (!self->mono || !blit)){
	frame_setShade(self, ((self->mono) ? 0:((pushed)? 0: FORESHADE)));
	(self)->FillRect( &Rect2, NULL); /* the middle box */
	if(text && *text){
	    long len = strlen(text);
	    frame_setShade(self, 100);
	    (self)->SetTransferMode( graphic_BLACK); 
	    (self)->MoveTo( tx, ty);
	    (self)->DrawText( text, len,t_op);
	    (self)->SetTransferMode( graphic_COPY);
	}
    }
    if(self->mono && (blit || ((!blit) && pushed))){
	(self)->SetTransferMode( graphic_INVERT);
	(self)->FillRect( rect, NULL);
	(self)->SetTransferMode( graphic_COPY);
    }
    if(self->mono){
	frame_setShade(self, 100);
	(self)->DrawRect(rect); 
/*	frame_DrawRect(self,&Rect2); */
    }
    else {
	/* Drawing the button is too slow on mono displays */
	frame_SetUpperShade(self,pushed) ;
	(self)->FillRectSize( rect->left, rect->top, BUTTONDEPTH + offset, rect->height, NULL);	/* left bar */

	frame_SetLowerShade(self,pushed) ;
	(self)->FillRectSize( rect->left + rect->width - BUTTONDEPTH + offset, rect->top, BUTTONDEPTH - offset, rect->height, NULL); /* right bar */
	(self)->FillTrapezoid( Rect2.left, r2_bot, Rect2.width, rect->left, r_bot, rect->width, NULL); /* lower trapz */

	frame_SetUpperShade(self,pushed) ;

	(self)->FillTrapezoid( rect->left, rect->top, rect->width, Rect2.left, Rect2.top, Rect2.width, NULL); /* upper trapz */
	(self)->SetTransferMode( graphic_COPY);
    }
}

static int CalculateLineHeight(class frame  *self)
    {

    class style *defaultStyle;
    class fontdesc *defaultFont;
    struct FontSummary *fontSummary;
    char fontFamily[256];
    long refBasis, refOperand, fontSize;

    if ((defaultStyle = (self->messageView)->GetDefaultStyle()) == NULL)
        return DEFAULTHEIGHT;
    (defaultStyle)->GetFontFamily( fontFamily, sizeof (fontFamily));
    (defaultStyle)->GetFontSize( (enum style_FontSize *) &refBasis, &fontSize);
    refOperand = (defaultStyle)->GetAddedFontFaces();
    defaultFont = fontdesc::Create(fontFamily, refOperand, fontSize);
    if ((fontSummary = (defaultFont)->FontSummary( (self)->GetDrawable())) == NULL)
        return DEFAULTHEIGHT;
    return fontSummary->maxHeight + 4; /* Two for top and bottom border. */
}

void frame::SetReturnFocus(class view  *v)
{
    if(this->returnFocus) (this->returnFocus)->RemoveObserver( this);
    if(v) {
	(v)->AddObserver( this);
    }
    this->returnFocus=v;
}

void frame::ReturnFocus()
{
    class view *focus;
    if(this->returnFocus) {
	(this->returnFocus)->WantInputFocus( this->returnFocus);
	(this->returnFocus)->RemoveObserver( this);
	this->returnFocus=NULL;
    } else {
	focus=(this)->GetView();
	if(focus) (focus)->WantInputFocus( focus);
    }
}

frame::frame()
        {
	ATKinit;

    extern class keymap *frame_InitKeymap();

    this->deleteTarget=NULL;
    this->returnFocus=NULL;
    this->realBuffer=NULL;
    this->realView=NULL;
    this->revertToReal=FALSE;
    this->height=0;
    this->mono=FALSE;
    this->PositionalPreference = message_Center;
    this->dv=NULL;
    this->IsAsking=FALSE;
    this->UsingDialog=FALSE;
    this->UseBuiltinDialogs= environ::GetProfileSwitch("UseBuiltinDialogs", FALSE);
    this->buffer = NULL;
    this->targetView = NULL;
    this->childView = NULL;
    this->messageLine = framemessage::Create(this);
    this->messageView = this->messageLine->messageView;
    (this)->SetNth( 1, this->messageView);
    this->dialogLine = framemessage::Create(this);
    this->dialogView = this->dialogLine->messageView;
    (this->messageLine)->SetCompanion((class msghandler *)this->dialogLine);
    this->title = NULL;

    frameKeymap = framecmds::InitKeymap(&frameMenus, &frameDefaultMenus, &frameDefaultKeymap);
    this->keystate = keystate::Create(this, frameDefaultKeymap);
    this->menulist = (frameMenus)->DuplicateML( this);
    this->defaultmenulist = (frameDefaultMenus)->DuplicateML( this);
    (this->defaultmenulist)->SetMask( frame_DefaultMenus);
    (this->menulist)->SetMask(frame_DefaultMenus);
    this->helpBuffer[0] = '\0';
    this->commandEnable = FALSE;
    this->lineHeight = 0;
    this->next = allFrames;
    allFrames = this;
    if (QuitWindowOnlyDefault == -1) {
	/* Can't do this in InitializeClass because the app name may not be
	 * known there.  We don't want to do it every time we create a frame
	 * either.  So we get it now...but just once.
	 */
	QuitWindowOnlyDefault = environ::GetProfileSwitch("QuitWindowOnly", FALSE);
	if (QuitWindowOnlyDefault) {
	    if (environ::GetProfileSwitch("QuitBuffer", FALSE))
		QuitWindowOnlyDefault++;
	}
    }
    this->QuitWindowOnly = QuitWindowOnlyDefault ;

    this->AwaitingFocus = 0;
    this->DialogBuffer = NULL;
    this->DialogBufferView = NULL;
    this->DialogTargetView = NULL;
    this->hasDialogMessage = FALSE;
    
    /* Create menulist/keystate */
    this->octcursor = cursor::Create((class view *) this);
    this->arrowcursor = cursor::Create((class view *) this);
    (this->octcursor)->SetStandard( Cursor_Octagon);
    (this->arrowcursor)->SetStandard( Cursor_Arrow);
    this->mymenus = menulist::Create(this);
    this->myfontdesc = fontdesc::Create("andysans", fontdesc_Bold, frame_FONTSIZE);
    this->mykeystate = keystate::Create(this, mykm);
    this->HeightsOfAnswer = NULL;
    this->uplist = NULL;
    this->mesrec.height = 0;
    this->UpdateRequested = FALSE;
    TidyUp(this);
    this->object = NULL;
    this->dataModified = FALSE;
    THROWONFAILURE( TRUE);
}

frame::~frame()
        {
	ATKinit;


    class frame *traverse, **previous;

    if((this)->GetIM()) ((this)->GetIM())->SetDeleteWindowCallback( NULL, 0);
    
    if(this->deleteTarget) {
	(this->deleteTarget)->RemoveObserver( this);
    }
    
    if(this->returnFocus) {
	(this->returnFocus)->RemoveObserver( this);
    }
    
#if 0
    if (this->buffer != NULL) {
        if (this->childView != NULL) {
	    (this)->SetNth( 0, NULL); /* Remove the view from the lpair so we can deallocate it. */
            (this->buffer)->RemoveView( this->childView);
	}
        (this->buffer)->RemoveObserver( this);
    }
#else
    (this)->SetBuffer( NULL, FALSE);
#endif
    (this)->SetNth( 1, NULL);    /* Remove the view from the lpair */	
    delete this->messageLine;
    delete this->dialogLine;
    delete this->octcursor;	
    delete this->arrowcursor;
    delete this->mykeystate;
    if(this->menulist) delete this->menulist;
    if(this->defaultmenulist) delete this->defaultmenulist;
    if (this->title != NULL)
        free(this->title);

    previous = &allFrames;
    for (traverse = allFrames; traverse != NULL && traverse != this; traverse = traverse->next)
        previous = &traverse->next;
    if (traverse != NULL) /* Bad error if this is false. */
        *previous = traverse->next;

    if(this->mymenus) {
	delete this->mymenus;
	this->mymenus=NULL;
    }
}



void frame::FullUpdate(enum view_UpdateType  type, long  left , long  top , long  width , long  height)
{
    if (this->lineHeight == 0) {
	long dw, dh;
	this->lineHeight = CalculateLineHeight(this);
	(this->messageView)->DesiredSize( (this)->GetLogicalWidth(), this->lineHeight, view_NoSet, &dw, &dh);
	(this)->VFixed( this->childView, this->messageView, dh, ResizableMessageLine);

	((class lpair *)this)->needsfull=3;
    }

    ::SetTitle(this);
    this->drawn = FALSE;
    (this)->lpair::FullUpdate( type, left, top, width, height);
    if (this->IsAsking && !this->UsingDialog) {
	ComputeSize(this);
	DoUpdate(this);
    }
}
static void handleNewData(class frame  *self)
{
    class view *inputFocus, *targetView,*oldchild;
    oldchild = self->childView;
    (oldchild)->UnlinkTree();
    (self->targetView)->UnlinkTree();
    if (self->object)
        (self->object)->RemoveObserver( self);
    self->object = (self->buffer)->GetData();
    (self->object)->AddObserver( self);
    /* self->realView = we don't set realView here since this only gets done for buffers.*/
    self->childView = (self->buffer)->GetView( &inputFocus, &targetView, NULL);
    self->targetView = targetView;
    (self)->VFixed( self->childView, self->messageView, (self->messageView)->GetLogicalHeight(), ResizableMessageLine);
 /*   buffer_RemoveView(self->buffer,oldchild); */

}

#include <lpair.H>
void frame::WantNewSize(class view  *req)
{
    long dw=(-1), dh=(-1);
    
    if(req!=(class view *)this->messageView) return;

    (this->messageView)->DesiredSize( (this)->GetLogicalWidth(), this->lineHeight, view_NoSet, &dw, &dh);
    /* (this->messageView)->DesiredSize( (this)->GetLogicalWidth(), this->lineHeight, view_NoSet &dw, &dh);
*/
    (this)->VFixed( this->childView, this->messageView, dh, ResizableMessageLine);

    ((class lpair *)this)->needsfull=3;
    
    (this)->WantUpdate( this);
}

void frame::Update()
    {
    struct pendingupdates *pu;

    this->UpdateRequested = FALSE;
    if(this->buffer != NULL && this->object != (this->buffer)->GetData()){
	handleNewData(this);
    }
    ::SetTitle(this);
    if (this->IsAsking && this->uplist) {
	RestoreBits(this);
/* 	CannotRestoreBits(self);    Will work but is too slow to be worth it */ 
    }
    while (this->uplist) {
	(this->uplist->v)->Update();
	pu = this->uplist->next;
	free(this->uplist);
	this->uplist = pu;
    }
    if (this->IsAsking && !this->UsingDialog) {
	DoUpdate(this);
    } 
    else (this)->lpair::Update();
}


void frame::ObservedChanged(class observable  *changed, long  value)
{    
    if((class observable *)this->returnFocus==changed && value==observable_OBJECTDESTROYED) {
	this->returnFocus=NULL;
	return;
    }
    if(value==observable_OBJECTDESTROYED && changed==(class observable *)this->deleteTarget) {
	this->deleteTarget=NULL;
	return;
    }
    
    if (value!=observable_OBJECTDESTROYED && this->buffer != NULL && changed == (class observable *) this->buffer && this->object != (this->buffer)->GetData()) {
	if (this->object)
	    (this->object)->RemoveObserver( this);
	this->object = (this->buffer)->GetData();
	(this->object)->AddObserver( this);
    }
    else if (changed == (class observable *) this->object) {
	if (value == observable_OBJECTDESTROYED) {
	    this->object = NULL;
	}
	else if (this->dataModified || (this->buffer)->GetScratch()) {
	    return;
	}
    }

    if (!this->UpdateRequested)
	(this)->WantUpdate( this);
}

void frame::SetBuffer(class buffer  *buffer, boolean  setInputFocus)
            {

    class view *inputFocus=NULL, *targetView=NULL;

    this->realView = NULL;
    
    if(this->childView != NULL  && ((buffer != this->buffer) || buffer==NULL)) {
	/* moved from inside the two ifs below... */
	(this)->SetNth( 0, NULL); /* Remove the view from the lpair so we can deallocate it. */
    }
    if (buffer != this->buffer) {
        if (this->buffer != NULL) {
            if (this->childView != NULL) {
                (this->buffer)->RemoveView( this->childView); /* Deallocate the view... */
            }
            if (this->object)
                (this->object)->RemoveObserver( this);
            if(this->buffer!=this->deleteTarget) (this->buffer)->RemoveObserver( this);
        }
        this->realBuffer = this->buffer = buffer;
#if 1
        if (buffer != NULL) {
#endif
            (buffer)->AddObserver( this);
            this->object = (this->buffer)->GetData();
            (this->object)->AddObserver( this);
	    this->childView = (buffer)->GetView( &inputFocus, &targetView, NULL);
            this->targetView = targetView;
	    (this)->VFixed( this->childView, this->messageView, (this->messageView)->GetLogicalHeight(), ResizableMessageLine);
	    if(inputFocus || targetView) {
		(this)->SetReturnFocus( inputFocus==NULL ? targetView : inputFocus);
		if (setInputFocus) {
		    if (inputFocus == NULL) {
			if(targetView) (targetView)->WantInputFocus( targetView);
		    } else {
			(inputFocus)->WantInputFocus( inputFocus);
		    }
		}
	    }
#if 1
        }
#endif
	NotifyObservers(0);
    }
}

void frame::SetView(class view  *view)
{
    this->realBuffer=NULL;
    
    if(this->childView != NULL) {
	(this)->SetNth( 0, NULL); /* Remove the view from the lpair so we can deallocate it. */;
    }
    
    if (this->buffer != NULL) {
	if (this->childView != NULL) {
	    (this->buffer)->RemoveView( this->childView); /* Deallocate the view... */
	}
	if(this->deleteTarget==this->buffer) (this->buffer)->RemoveObserver( this);
	if (this->object)
	    (this->object)->RemoveObserver( this);
	this->object = NULL;
	this->buffer = NULL;
    }
    else if(view == NULL && this->childView != NULL){
	(this)->SetNth( 0, NULL); /* Remove the view from the lpair so it can be
				      destroyed by whoever created it */
    }
    (this)->SetReturnFocus( view);
    this->realView = this->childView = this->targetView = view;
    if (view != NULL) {
	(this)->VFixed( this->childView, this->messageView, (this->messageView)->GetLogicalHeight(), ResizableMessageLine);
	(view)->WantInputFocus( view);
    }
}


/* framecmds.C */
	int frame_VisitNamedFile(class frame  *self, const char  *filename, 
			boolean  newWindow, boolean  rawMode);
	int 
frame::VisitNamedFile(const char  *filename, 
			boolean  newWindow, boolean  rawMode) {
	return frame_VisitNamedFile(this, filename, 
			newWindow, rawMode);
}


/* Create a frame suitable for use as a buffer window. */
class frame *frame::Create(class buffer  *buffer)
        {
	ATKinit;


    class frame *tempFrame = new frame;

    (tempFrame)->SetBuffer( buffer, TRUE);
    (tempFrame)->SetCommandEnable( TRUE);
    return tempFrame;
}

/* Iterates over all frames.
 */
class frame *frame::Enumerate(frame_effptr mapFunction, long  functionData)
            {
	ATKinit;


    class frame *traverse, *next;

    for (traverse = allFrames; traverse != NULL; traverse = next) {
        next = traverse->next; /* So mapFunction is allowed to delete the frame. */
        if ((*mapFunction)(traverse, functionData))
            return traverse;
    }
    return NULL;
}

ATK  *frame::WantHandler(const char  *handlerName)
{
    if (strcmp(handlerName, "message") == 0)
	return (ATK  *) this->messageLine;
    else if (this->parent == NULL) return NULL;
    else
	return (this->parent)->WantHandler( handlerName);
}

const char *frame::WantInformation(const char  *key)
{
    if (!strcmp(key, "filename")) {
	if (this->GetBuffer())
	    return this->GetBuffer()->GetFilename();
    }

    return this->lpair::WantInformation(key);
}

void frame::PostKeyState(class keystate  *keystate)
{
    if(this->IsAsking){
	if (keystate == NULL || keystate->object != (ATK  *) this->dialogView) {
	    this->mykeystate->next = NULL;
	    /* (this->mykeystate)->Reset(); */
	    (this->mykeystate)->AddAfter( keystate);
	}
	if (keystate == NULL) {
	    (this->parent)->PostKeyState( this->mykeystate);
	}
	else{
	    if (keystate == this->mykeystate->next) {
		this->mykeystate->next = NULL;
	    }
	    (this->parent)->PostKeyState( keystate);
	}
    }
    else if (keystate == NULL || keystate->object != (ATK  *) this->messageView) {
            this->keystate->next = NULL;
	    /* (this->keystate)->Reset(); */
	    (this->keystate)->AddAfter( keystate);
	    if (keystate == NULL) {
		(this->parent)->PostKeyState( this->keystate);
	    } else {
		(this->parent)->PostKeyState( keystate);
	    }
    } else (this->parent)->PostKeyState( keystate);
}


void frame::PostMenus(class menulist  *menulist)
{

    if(this->IsAsking ){
	if((menulist != NULL) && this->hasDialogMessage && menulist->object == (ATK  *) this->dialogView){
	    (this->mymenus)->ClearChain();
	    (this->mymenus)->ChainBeforeML( menulist, (long) menulist);
	    (this->parent)->PostMenus(this->mymenus);
	}
	else {
	    (this->parent)->PostMenus( this->mymenus);
	}
    }
    else if ((menulist == NULL) || (menulist->object != (ATK  *) this->messageView)) {
	(this->defaultmenulist)->ClearChain();
	(this->menulist)->ClearChain();
	(this->defaultmenulist)->ChainBeforeML( menulist, (long) menulist);
	if(this->commandEnable) (this->defaultmenulist)->ChainBeforeML( this->menulist, (long)this->menulist);
	(this->parent)->PostMenus( this->defaultmenulist);
    }
    else
	(this->parent)->PostMenus( menulist);
}

void frame::SetCommandEnable(boolean  enable)
        {

    if ((this->commandEnable = enable) == TRUE) {
	if(this->keystate) delete this->keystate;
	this->keystate=keystate::Create(this, frameKeymap);
	(this->menulist)->SetMask((this->menulist)->GetMask() |
			 frame_BufferMenus);
	(this->defaultmenulist)->SetMask((this->defaultmenulist)->GetMask() |
			 frame_BufferMenus);
    } else {
	if(this->keystate) delete this->keystate;
	this->keystate=keystate::Create(this, frameDefaultKeymap);
	(this->menulist)->SetMask((this->menulist)->GetMask() &
			 ~frame_BufferMenus);
	(this->defaultmenulist)->SetMask((this->defaultmenulist)->GetMask() &
			 ~frame_BufferMenus);
    }
}

class buffer *frame::GetHelpBuffer()
    {

    int i;
    class buffer *b;

    for (i = 1; i < 100 && ((b = buffer::FindBufferByName(this->helpBuffer)) == NULL); i++) {

        char bufferName[30];

        sprintf(bufferName, "Help-Buffer-%d", i);
        if (((b = buffer::FindBufferByName(bufferName)) == NULL) || !(b)->Visible()) {
            if (b == NULL)
                b = buffer::Create(bufferName, NULL, "text", NULL);
            strncpy(this->helpBuffer, bufferName, sizeof(this->helpBuffer));
        }
    }
    (b)->SetScratch( TRUE);
    return b;
}
class buffer *frame::SetHelpBuffer(class buffer  *buf, boolean  setInputFocus)
{   /* Returns the last buffer IF it will need to be restored */
    class view *inputFocus, *targetView;
    int NeedUpdate = 0;
    if(!this->IsAsking) {
	class buffer *LastBuffer = (this)->GetBuffer();
	class buffer *realBuffer = this->realBuffer;
	class view *realView = this->realView;
	(this)->SetBuffer( buf, setInputFocus);
	this->realBuffer = realBuffer;
	this->realView = realView;
	this->revertToReal=TRUE;
	return LastBuffer;
    }
    /* Put the buffer in the dialog box */
    if(this->DialogBuffer == NULL){
	this->drawn = FALSE;
	NeedUpdate++;
    }
    if (buf != this->DialogBuffer) {
	if (this->DialogBuffer != NULL) {
	    if (this->DialogBufferView != NULL)
		(this->DialogBuffer)->RemoveView( this->DialogBufferView);
	    (this->DialogBuffer)->RemoveObserver( this);
	}
	(buf)->AddObserver( this);
	this->DialogBuffer = buf;
	this->DialogBufferView = (buf)->GetView( &inputFocus, &targetView,"helptextview");
	if(this->dv) {
	    (this->dv)->InstallSidekick( this->DialogBufferView);
	    ((this->dv)->GetIM())->RedrawWindow();
	}
	this->DialogTargetView = targetView;
    }
    if(NeedUpdate){
	ComputeSize(this);
	(this)->WantUpdate(this);
    }
    return NULL;
}

void frame::RemoveHelp()
{
    if(this->revertToReal) {
	this->revertToReal=FALSE;
	if(this->realView) (this)->SetView( this->realView);
	if(this->realBuffer) (this)->SetBuffer( this->realBuffer, TRUE);
    }
}

void frame::SetTitle(const char  *title)
        {
    if (this->title != NULL)
        free(this->title);

    if (title != NULL)
        this->title = strdup(title);
    else
        this->title = NULL;

    if (! this->UpdateRequested)
        (this)->WantUpdate( this);
}

/*
 * This code sets the title bar such that the end of a filename is visible.
 * Seldom is there so much code to accomplish so little.
 * The "Magic hack" 70-char limit has been removed, since xim is fully capable of truncating the front of the filename by itself, and it can be customized with the MaxTitleLength preference. -RSK
 */
static void SetTitle(class frame  *self)
{
    static const char *readonly_flag = NULL;

    if (!readonly_flag) {
	/* first time this function has been called; we must read in the readonly_flag, or set it to the default. */
	 readonly_flag = GetProfileString("ReadOnlyTitle", "(readonly)");
    }

#define WMTITLELEN 1050  /* room for MAXPATHLEN + "*" + "(readonly)" */

    if (self->title == NULL) {
        if ((self)->GetBuffer() == NULL)
            return;
        else {
            char titleBuffer[WMTITLELEN], *titleLine;
            int maxLen = sizeof(titleBuffer) - 2;

	    *titleBuffer = '\0';

	    if (self->buffer != NULL){
		if((self->buffer)->GetScratch())
		    self->dataModified = FALSE;
		else if ((self->dataModified = ((self->buffer)->GetWriteVersion() < ((self->buffer)->GetData())->GetModified()))) {
		    --maxLen; /* Make room for '*' */
		}
		if ((self->buffer)->GetReadOnly()) {
		    maxLen -= (strlen(readonly_flag)+1);	/* Make room for readonly flag and the space before it */
		}
	    }
	    titleLine=((self)->GetBuffer())->GetFilename();
	    if (titleLine!= NULL && *titleLine != '\0'){
		path::TruncatePath(titleLine, titleBuffer, maxLen, TRUE);
	    }
	    else {
		titleLine=((self)->GetBuffer())->GetName();
		if (titleLine==NULL) {
		    ((self)->GetIM())->SetTitle( "");
		    return;
		}
		else{
		    strcpy(titleBuffer, "Buffer: ");
		    maxLen-=sizeof("Buffer: ")-1;
		    strncat(titleBuffer,titleLine,maxLen);
		}
	    }

	    if (self->dataModified)
		strcat(titleBuffer, "*");
	    if ((self->buffer)->GetReadOnly()) {
		strcat(titleBuffer, " ");
		strcat(titleBuffer, readonly_flag);
	    }

	    ((self)->GetIM())->SetTitle( titleBuffer);
	}
    }
    else
	((self)->GetIM())->SetTitle( self->title);
}

/* The following is code to support the dialog box */

static void ConsiderReturning(class frame  *self, int  Choice)
{
    if (self->StackPos != frame_GlobalStackCount) {
	(self->messageLine)->DisplayString( 0, "Please answer the other dialog box first.");
	return;
    }
    if (Choice <= 0 || Choice > self->NumAnswerChoices) {
	(self->messageLine)->DisplayString( 0, "That is not an answer; please try again.");
	return;
    }
    if(self->hasDialogMessage){
	switch(Choice){
	    case ACCEPT:
		(self->dialogView)->Return((long)'\n');
		break;
	    case CANCEL:
		(self->dialogLine)->CancelQuestion();
		break;
	    case COMPLETE:
		(self->dialogView)->Complete((long)' ');
		if(!self->UsingDialog) drawButton(self,&self->HeightsOfAnswer[self->DefaultWildestAnswer], self->MultipleAnswers[self->DefaultWildestAnswer],FALSE,FALSE,self->drawn);
		self->DefaultWildestAnswer = 0;
		return;
	    case HELP:
		(self->dialogView)->Help((long)'?');
		if(!self->UsingDialog) drawButton(self,&self->HeightsOfAnswer[self->DefaultWildestAnswer], self->MultipleAnswers[self->DefaultWildestAnswer],FALSE,FALSE,self->drawn);
		self->DefaultWildestAnswer = 0;
		return;
	}
    }
    self->IsAsking = 0;
    self->WildestAnswer = Choice;
}

class view *
frame::Hit(enum view_MouseAction  action, long  x , long  y , long  nclicks)
{
    int i;
    struct rectangle r;
    class view *v;
    if(this->hasDialogMessage){
	if( this->dialogView && InRectangle(&this->mesrec, x, y))
	    return (this->dialogView)->Hit( action, ((class view *)(this->dialogView))->EnclosedXToLocalX( x), ((class view *)(this->dialogView))->EnclosedYToLocalY( y), nclicks);
	else if (this->DialogBufferView && InRectangle(&this->bufferrec, x, y))
	    return (this->DialogBufferView)->Hit( action, ((class view *)(this->DialogBufferView))->EnclosedXToLocalX( x), ((class view *)(this->DialogBufferView))->EnclosedYToLocalY( y), nclicks);
    }
    if (this->UsingDialog || !this->IsAsking
	 || (!this->IsBlocking && !InRectangle(&this->AnswerBox, x, y))) {
	/* Normal, non-dialog hit. */
	v = (this)->lpair::Hit( action, x, y, nclicks);
	/* Support for DRAG/DROP protocol.
	 * ==============================
	 * If frame commands are enabled, and the action was a file drop do:
	 *    1) Visit file for LeftFileDrop, and
         *    2) Visit file in new window for RightFileDrop.
	 * Note that many files can be dropped simultaneously.  In this case
	 * the files are loaded into buffers and the last is displayed on
	 * a visit-file, and all are displayed on a visit-file-new-window.
	 * The host is ignored for now.
	 * An improvement would have a wait cursor appear while files are fetched.
	 */
	if (this->commandEnable && (action == view_LeftFileDrop || action == view_RightFileDrop)) {
	    char **files;
	    int i;
	    class buffer *b = NULL;
	    class frame *f;
	    class view *v=this;
	    class im *im = (this)->GetIM();

	    if (im == NULL) return v;
	    files = (im)->GetDroppedFiles();
	    if (files) {
		if (files[0] != NULL)
		    free(files[0]); /* ignore host for now */
		for (i = 1; files[i] != NULL; i++) {
		    b = buffer::GetBufferOnFile(files[i], buffer_MustExist);
		    if (b != NULL && action == view_RightFileDrop) {
			f = frame::GetFrameInWindowForBuffer(b);
			im = (f)->GetIM();
			if (im)
			    (im)->ExposeWindow();
			v = (f)->GetView();
			if (v)
			    (v)->WantInputFocus( v);
		    }
		    free(files[i]);
		}
		if (b != NULL && action == view_LeftFileDrop)
		    (void)(this)->SetBuffer( b, TRUE); /* Show last file. */
		free(files);
	    }
	}
	return v;
    }
    if (action == view_LeftDown || action == view_RightDown) {
	r = this->HeightsOfAnswer[this->DefaultWildestAnswer];
	r.width = this->buttonmaxwid + frame_TOTALPADDING - frame_TWOSEPARATIONS;
	if(!(InRectangle(&r, x, y))){
	    for (i=1; i<=this->NumAnswerChoices; ++i) {
		r = this->HeightsOfAnswer[i];
		r.width = this->buttonmaxwid + frame_TOTALPADDING - frame_TWOSEPARATIONS;
		if (InRectangle(&r, x, y)) {
		    if(this->DefaultWildestAnswer != 0)
			drawButton(this,&this->HeightsOfAnswer[this->DefaultWildestAnswer], this->MultipleAnswers[this->DefaultWildestAnswer],FALSE,FALSE,this->drawn);
		    this->DefaultWildestAnswer = i;
		    drawButton(this,&this->HeightsOfAnswer[this->DefaultWildestAnswer], this->MultipleAnswers[this->DefaultWildestAnswer],TRUE,FALSE,this->drawn);
		}
	    }
	}
	return((class view *) this);
    }
    else if (action == view_LeftMovement || action == view_RightMovement) {
	if(this->DefaultWildestAnswer != 0){
	    r = this->HeightsOfAnswer[this->DefaultWildestAnswer];
	    r.width = this->buttonmaxwid + frame_TOTALPADDING - frame_TWOSEPARATIONS;
	    if(!(InRectangle(&r, x, y))){
		drawButton(this,&this->HeightsOfAnswer[this->DefaultWildestAnswer], this->MultipleAnswers[this->DefaultWildestAnswer],FALSE,FALSE,this->drawn);
		this->PotentialChoice = this->DefaultWildestAnswer;
		this->DefaultWildestAnswer = 0;
	    }
	}
	else 	if(this->PotentialChoice != 0){
	    r = this->HeightsOfAnswer[this->PotentialChoice];
	    r.width = this->buttonmaxwid + frame_TOTALPADDING - frame_TWOSEPARATIONS;
	    if((InRectangle(&r, x, y))){
		drawButton(this,&this->HeightsOfAnswer[this->PotentialChoice], this->MultipleAnswers[this->PotentialChoice],TRUE,FALSE,this->drawn);
		this->DefaultWildestAnswer = this->PotentialChoice ;
		this->PotentialChoice = 0;
	    }
	}

	return((class view *) this);
    }
    /* Choose the answer here */
    i = this->DefaultWildestAnswer ;
    if(i > 0){
	r = this->HeightsOfAnswer[i];
	r.width = this->buttonmaxwid + frame_TOTALPADDING - frame_TWOSEPARATIONS;
	if (InRectangle(&r, x, y)) {
	    /* found right answer */
	    ConsiderReturning(this, i);
	    return((class view *) this);
	}
    }
    if(!this->hasDialogMessage && InRectangle(&this->AnswerBox, x, y)) 
	ConsiderReturning(this, this->DefaultWildestAnswer);
    return((class view *) this);
}
static boolean
InRectangle(struct rectangle  *r, long  x , long  y)
{
    if ((x < r->left) || (x > (r->left + r->width)) || (y < r->top) || (y > (r->top + r->height))) {
	return(0);
    }
    return(1);
}
static void drawshadow(class frame  *self,struct rectangle  *r)
{
    (self)->FillRectSize(r->left + OFFSET,r->top + r->height,r->width,OFFSET,(self)->GrayPattern(8,16));
    (self)->FillRectSize(r->left + r->width,r->top + OFFSET,OFFSET,r->height - OFFSET,(self)->GrayPattern(8,16));
}
static void
DoUpdate(class frame  *self)
{
    struct rectangle *r;
    int i;
    if(!self->drawn){
	class graphic *pattern;
	frame_CacheSettings(self);
	pattern = ((self->mono) ? (self)->WhitePattern():(self)->BlackPattern());
	self->drawn = TRUE;
	SaveBits(self);
	if(!self->mono)frame_setShade(self,FORESHADE);
	(self)->SetFont( self->myfontdesc);
	(self)->SetTransferMode( graphic_COPY);
	if(self->DialogBuffer){
	    (self)->FillRect( &self->bufferrec, pattern);
	    (self)->FillRect( &self->AnswerBox, pattern);
	    /* Also clear area between boxes */
	    if(!self->mono)(self)->SetFGColor(  1.,1.,1.);
	    if(self->AnswerBox.top == self->bufferrec.top) 	
		(self)->FillRectSize(self->AnswerBox.left + self->AnswerBox.width ,  self->AnswerBox.top,10, self->AnswerBox.height + OFFSET , pattern );
	    else {
		(self)->FillRectSize(self->AnswerBox.left,  self->AnswerBox.height +self->AnswerBox.top, self->AnswerBox.width  + OFFSET ,10, pattern);
	    }

	}
	else 	
	    (self)->FillRect(&self->AnswerBox, pattern);
	frame_setShade(self,100);
	if(self->DialogBuffer)   { 
	    drawshadow(self,&self->bufferrec);
	    (self)->DrawRect(&self->bufferrec);
	}
	drawshadow(self,&self->AnswerBox);
	(self)->DrawRect(&self->AnswerBox);
	for (i=1; i<= self->NumAnswerChoices; ++i) {
	   drawButton(self,&self->HeightsOfAnswer[i],self->MultipleAnswers[i],(i == self->DefaultWildestAnswer),FALSE,FALSE);
	}
	frame_setShade(self,100);
	if(self->hasDialogMessage){
	    r = &self->mesrec;
	    (self)->DrawRectSize( r->left - 1, r->top -1, r->width + 2, r->height + 2);
	}
	r = &self->HeightsOfAnswer[0];
	(self)->MoveTo( r->left + frame_SEPARATION + 3, r->top + frame_SEPARATION + 3);
	(self)->DrawString( self->MultipleAnswers[0], graphic_ATLEFT | graphic_ATTOP);
	(self)->PostCursor( &self->AnswerBox, self->octcursor);
	if(self->hasDialogMessage){
	    (self->dialogView)->InsertView( self, &self->mesrec);
	    (self->dialogView)->FullUpdate( view_FullRedraw , 0, 0, 0, 0);
	    (self)->PostCursor( &self->mesrec, self->arrowcursor);
	    if(self->DialogBuffer){
		struct rectangle nr;
		nr = self->bufferrec;
		nr.top++;nr.left++; nr.width += -2; nr.height += -2;
		(self->DialogBufferView)->InsertView( self, &nr);
		(self->DialogBufferView)->FullUpdate( view_FullRedraw , 0, 0, 0, 0);
	    }
	}
	if ((self)->GetIM() != NULL) {
	    (self)->SetBGColor(  1.,1.,1.);
	    (self)->SetFGColor(0.,0.,0.);
	    (self)->SetTransferMode( graphic_INVERT);
	}
    }
}

static void
SaveBits(class frame  *self)
{
    /* A no-op for now */
}

static int ButtonInteract(class frame  *self, const char  **AnswerList, long  DefaultWildestAnswer , long  *WildestAnswer, int  flags)
{
    int i;
    int answer;
    class view *focus;
    class im *im=(self)->GetIM();
    if (!im || self->IsAsking || self->AwaitingFocus) return(-1);
    while(im::Interact(0)); /*Clear out any pending updates */
    for (i = 0; AnswerList[i] && *AnswerList[i]; ++i) {
 	;
    }
    self->NumAnswerChoices = --i;
    if (i <= 0) {
	return(-1);
    }
    self->MultipleAnswers = AnswerList;
    self->DefaultWildestAnswer = DefaultWildestAnswer;
    self->StackPos = ++frame_GlobalStackCount;
    ComputeSize(self);
    self->IsAsking = 1;
    focus = (im)->GetInputFocus();
    if (focus && focus != (class view *)self) (focus)->LoseInputFocus();
    (self)->SetReturnFocus( focus);
    self->dv=NULL;
    if(!self->UseBuiltinDialogs && (((im)->SupportsOverride() && self->IsBlocking) || ((im)->SupportsTransient() && !self->IsBlocking))) {
	self->dv=dialogv::Create(AnswerList+1, NULL, environ::GetProfileInt("dialogstyle", 4 /* sbutton_MOTIF */));
	if(self->dv) {
	    self->UsingDialog=TRUE;
	    ((self->dv)->GetTextData())->ReadTemplate( "dialog", FALSE);
	    ((self->dv)->GetTextData())->InsertCharacters(  0, *AnswerList, strlen(*AnswerList));
	}
    }
    PrepareMenus(self);
    if(! self->UpdateRequested) (self)->WantUpdate( self);
    
    if(self->dv && self->UsingDialog) {
	while(self->IsAsking) {
	    long pos=im_Centered;
	    switch(self->PositionalPreference) {
		case message_OnTop:
		    pos|=im_AtTop;
		    break;
		case message_OnBottom:
		    pos|=im_AtBottom;
		    break;
		case message_Center:
		    pos|=im_InMiddle;
		default:
		    break;
	    }
	    answer=(self->dv)->PostChoice( (self)->GetIM(), (self)->GetIM(), &self->IsAsking, (self->DefaultWildestAnswer<1)?-1:self->DefaultWildestAnswer-1, self->IsBlocking, pos)+1;
	    if(answer>0) ConsiderReturning(self, answer);
	}
    } else {
	self->UsingDialog=FALSE;
	while (self->IsAsking) {
	    im::Interact(1);
	}
    }
    --frame_GlobalStackCount;
    retractCursors(self);
    if(!self->UsingDialog && !RestoreBits(self))
	CannotRestoreBits(self);
    if ((self->WildestAnswer <=0) || (self->WildestAnswer > self->NumAnswerChoices)) {
	*WildestAnswer = -1;
    } else {
	*WildestAnswer = self->WildestAnswer;
    }
    TidyUp(self);
    if (focus != (class view *)self ) (self)->ReturnFocus();
    return((*WildestAnswer == -1) ? -1 : 0);
}
static void
PrepareMenus(class frame  *self)
{
    int i, plen;
    char Priority[10], QBuf[600], BigBuf[1200]; /* No one in his right mind will ever exceed these */

    /* if usemine, force out my keystate & menulist.  Otherwise restore the
    last non-mine version. */
    if (self->IsAsking) {
	(self->mymenus)->ClearML();
#ifdef PROBABLYHOPLESS /* wm bug on long menu items */
	strcpy(QBuf, self->MultipleAnswers[0]);
	PurifyString(QBuf);
	strcat(QBuf, ",");
#else /* PROBABLYHOPLESS  */
	strcpy(QBuf, "Dialog Box~1,");
#endif /* PROBABLYHOPLESS  */
	plen = strlen(QBuf);
	for (i=1; i<= self->NumAnswerChoices; ++i) {
	    strcpy(BigBuf, QBuf);
	    strcat(BigBuf, self->MultipleAnswers[i]);
	    PurifyString(&BigBuf[plen]);
	    if (strlen(BigBuf) > 80) { /* wm bug on long menu items */
		BigBuf[78] = '\0';
		strcat(BigBuf, "...");
	    }
	    sprintf(Priority, "~%d", i);
	    strcat(BigBuf, Priority);
	    (self->mymenus)->AddToML( BigBuf, returnconsidered, i,0);
	}
	(self->mymenus)->AddToML( "Quit", 0, 0,0);
	if(self->dv) {
	    class menulist *dup=(self->mymenus)->DuplicateML( self);
	    class keystate *dupk = keystate::Create(self, mykm);
	    (self->dv)->SetExtraMenus( dup);
	    (self->dv)->SetExtraKeyState( dupk);
	}
	(self)->PostMenus( self->mymenus); 
	(self)->PostKeyState( self->mykeystate);
    }
}

/* This next routine tries to make sure a string won't blow up in a menu */
static void
PurifyString(char  *s) 
{
    if (!s) return;
    while (*s) {
	switch(*s) {
	    case ',':
		*s++ = '.';
		break;
	    case ':':
	    case '~':
		*s++ = '-';
		break;
	    default:
		++s;
	}
    }
}
static boolean
RestoreBits(class frame  *self)
{
    /* for now, a no-op */
    return FALSE;
}

/* this next routine should go away once SaveBits and RestoreBits work. */
static void
CannotRestoreBits(class frame  *self)
{
    struct rectangle r;

    (self)->GetLogicalBounds( &r);
    (self)->lpair::FullUpdate( view_FullRedraw, r.left, r.top, r.width, r.height);
}

void frame::Advice(enum message_Preference  pp)
{
    if(pp == message_NoBlock) this->IsBlocking = 0;
    else this->PositionalPreference = pp;
}

static void
ComputeSize(class frame  *self)
{
    long i, xw, yw, curht, totht, totwid,realwid, curleft, maxheight, boxheight,maxwid = 0;

    maxheight = 0;
    if (!self->HeightsOfAnswer) {
	self->HeightsOfAnswer = (struct rectangle *) malloc((2+self->NumAnswerChoices) * sizeof(struct rectangle));
    }
    self->buttonmaxwid = 0;
    realwid = (self)->GetLogicalWidth();
    curht = frame_HALFPADDING;
    if(self->DialogBuffer && realwid > MINSIDEBYSIDEWIDTH){
	totwid = realwid / 2;
    }
    else totwid = realwid;
    /* get max button sizes */
    for(i = 1; i <= self->NumAnswerChoices; i++) {

        struct FontSummary *fontSummary;

	(self->myfontdesc)->StringSize( (self)->GetDrawable(), self->MultipleAnswers[i], &xw, &yw);
	if ((i > 0) && (xw > self->buttonmaxwid))
            self->buttonmaxwid = xw;

/* This code should really be factored out of this loop, but I don't have
 * time to test that now.
 */
        if ((fontSummary = (self->myfontdesc)->FontSummary(
                                                (self)->GetDrawable())) == NULL) {
            if (yw < (frame_FONTSIZE+frame_FONTPAD))
                yw = frame_FONTSIZE+frame_FONTPAD;
        }
        else
            yw = fontSummary->maxHeight + frame_FONTPAD;

	if(maxheight < yw) maxheight = yw;
	self->HeightsOfAnswer[i].height = yw;
	self->HeightsOfAnswer[i].width = xw;
    }
    curleft = (totwid - self->buttonmaxwid - frame_TOTALPADDING) / 2;

    /* calculate the header placement */
    (self->myfontdesc)->StringSize( (self)->GetDrawable(), self->MultipleAnswers[0], &xw, &yw);
    if (yw < (frame_FONTSIZE+frame_FONTPAD)) yw = frame_FONTSIZE+frame_FONTPAD; /* bogus fontdesc bug */
    self->HeightsOfAnswer[0].left = (totwid - xw - frame_TOTALPADDING) / 2;
    self->HeightsOfAnswer[0].width = xw+frame_TOTALPADDING;
    if (xw > maxwid) maxwid = xw;
    self->HeightsOfAnswer[0].top = curht;
    self->HeightsOfAnswer[0].height = yw;
    curht += frame_TWOSEPARATIONS + yw;

    if(self->hasDialogMessage){
	/* calculate the dialog box placement */
	if(self->mesrec.height == 0)
	    self->mesrec.height = self->lineHeight;
	xw = totwid - frame_TOTALPADDING - frame_TOTALPADDING - frame_TOTALPADDING - frame_TOTALPADDING;
	self->mesrec.left = (totwid - xw - frame_TOTALPADDING) / 2;
	self->mesrec.width = xw+frame_TOTALPADDING;
	self->mesrec.top = curht;
	curht += frame_TWOSEPARATIONS + self->mesrec.height;
    } 

    /* calculate button placement */

    for (i=1; i<= self->NumAnswerChoices; ++i) {
	self->HeightsOfAnswer[i].left = curleft;
	self->HeightsOfAnswer[i].top = curht;
	curht += frame_TWOSEPARATIONS + maxheight;
    }

    if(self->hasDialogMessage && (self->buttonmaxwid + frame_TOTALPADDING) * 2 < totwid){
	/* double up buttons */
	curht = self->HeightsOfAnswer[1].top;
	for(i = 1; i< self->NumAnswerChoices; i += 2) {
	    self->HeightsOfAnswer[i].left += -10 -(self->buttonmaxwid / 2);
	    self->HeightsOfAnswer[i+1].left += 10 + (self->buttonmaxwid / 2);
	    self->HeightsOfAnswer[i + 1].top = curht;
	    self->HeightsOfAnswer[i].top = curht;
	    self->HeightsOfAnswer[i].height = maxheight;
	    self->HeightsOfAnswer[i + 1].height = maxheight;
	    curht += frame_TWOSEPARATIONS + maxheight;
	}
	if(i == self->NumAnswerChoices){
	    self->HeightsOfAnswer[i].top = curht;
	    curht += frame_TWOSEPARATIONS + self->HeightsOfAnswer[i].height;
	}
	maxwid = MAX((self->buttonmaxwid + frame_TOTALPADDING)* 2 ,maxwid);
    }
    else maxwid = MAX((self->buttonmaxwid + frame_TOTALPADDING) ,maxwid);
    if(self->hasDialogMessage)
	maxwid =  self->mesrec.width - frame_TOTALPADDING;

    /* Decide where in the view rect to put the box rect */
    self->AnswerBox.height = curht + frame_TOTALPADDING;
    totht = (self)->GetLogicalHeight();
    boxheight = (self->DialogBuffer && realwid <= MINSIDEBYSIDEWIDTH)?   (self->AnswerBox.height + OFFSET)* 2 : self->AnswerBox.height ;
    if (boxheight > (totht + frame_TOTALPADDING)) {
	self->AnswerBox.top = 0;
    } else if (self->PositionalPreference == message_OnTop) {
	self->AnswerBox.top = frame_HALFPADDING;
    } else if (self->PositionalPreference  == message_OnBottom) {
	self->AnswerBox.top = totht - boxheight - frame_HALFPADDING;
    } else {
	self->AnswerBox.top = (totht - boxheight) / 2;
    }
    self->AnswerBox.width = maxwid + frame_DOUBLEPADDING + frame_HALFPADDING;
    self->AnswerBox.left = (totwid - maxwid - frame_DOUBLEPADDING - frame_HALFPADDING) / 2;
    /* Now go back and fix up all the offsets */
    curht = self->AnswerBox.top;
    if (curht) {
	for (i=0; i<=self->NumAnswerChoices; ++i) {
	    self->HeightsOfAnswer[i].top += curht;
	}
	if(self->hasDialogMessage) self->mesrec.top += curht;
    }

    if(self->DialogBuffer){
	if(totwid < realwid){
	    self->bufferrec.top = self->AnswerBox.top ;	
	    self->bufferrec.left = self->AnswerBox.left + self->AnswerBox.width  + OFFSET + OFFSET;
	    self->bufferrec.width = realwid - self->AnswerBox.width - (OFFSET * 6);
	    self->bufferrec.height = self->AnswerBox.height ;
	}
	else {
	    self->bufferrec.top = self->AnswerBox.top + self->AnswerBox.height + OFFSET + OFFSET;	
	    self->bufferrec.left = self->AnswerBox.left;
	    self->bufferrec.width = self->AnswerBox.width ;
	    self->bufferrec.height = self->AnswerBox.height ;

	}
    }
}


static void GotKey(class frame  *self, char  c)
{
    int curpt, startpt=self->DefaultWildestAnswer;
    char c1;

    if (startpt <= 0 || startpt > self->NumAnswerChoices) {
	startpt = 1;
    }
    curpt = startpt;
    c1 = isupper(c) ? tolower(c) : toupper(c);
    while (1) {
	if (++curpt > self->NumAnswerChoices) curpt = 1;
	if ((*self->MultipleAnswers[curpt] == c)
	|| (*self->MultipleAnswers[curpt] == c1)) {
	     if(self->DefaultWildestAnswer != 0 && self->UseBuiltinDialogs)    drawButton(self,&self->HeightsOfAnswer[self->DefaultWildestAnswer],	    self->MultipleAnswers[self->DefaultWildestAnswer],FALSE,FALSE,self->drawn);
	    self->DefaultWildestAnswer = curpt;
	    if(self->UseBuiltinDialogs) drawButton(self,&self->HeightsOfAnswer[self->DefaultWildestAnswer], self->MultipleAnswers[self->DefaultWildestAnswer],TRUE,FALSE,self->drawn);
	    else if(self->dv) (self->dv)->ActivateButton( curpt-1);
	    return;
	}
	if (curpt == startpt) {
	    (self->messageLine)->DisplayString( 0, "That keystroke does not match any of the choices.");
	    return;
	}
    }
}

static void ConfirmDefaultAnswer(class frame  *self)
{
    ConsiderReturning(self, self->DefaultWildestAnswer);
}

static void Cancel(class frame  *self)
{
    /* ^G key binding */
    if (self->StackPos != frame_GlobalStackCount) {
	(self->messageLine)->DisplayString( 0, "Please answer the other dialog box first.");
	return;
    }
#ifdef MUSTANSWER
    if (self->IsBlocking) {
	(self->messageLine)->DisplayString( 0, "This is a question that MUST be answered.");
    } else {
	(self->messageLine)->DisplayString( 0, "Question Cancelled.");
	self->IsAsking = 0;
	self->WildestAnswer = -1;
	if(self->dv) (self->dv)->CancelQuestion();
    }	
#else /* MUSTANSWER */
    (self->messageLine)->DisplayString( 0, "Question Cancelled.");
    self->IsAsking = 0;
    self->WildestAnswer = -1;
    if(self->dv) (self->dv)->CancelQuestion();
    
#endif /* MUSTANSWER */
    if(self->hasDialogMessage){
	(self->dialogLine)->CancelQuestion();
    }

}

static void retractCursors(class frame  *self)
{
    if((self->octcursor)->IsPosted())
	(self)->RetractCursor( self->octcursor);
    if((self->arrowcursor)->IsPosted())
	(self)->RetractCursor( self->arrowcursor);
    (self)->PostMenus(NULL);
}

static void
TidyUp(class frame  *self)
{
    struct pendingupdates *pu;
    if(!self->UseBuiltinDialogs) {
	self->UsingDialog=FALSE;
	if(self->dv) {
	    class menulist *dup=(self->dv)->GetExtraMenus();
	    class keystate *dupk=(self->dv)->GetExtraKeys();
	    (self->dv)->Destroy();
	    if(dup!=NULL) (dup)->ClearChain();
		delete dup;
	    if(dupk!=NULL) delete dupk;
	    self->dv=NULL;
	}
    }
    self->IsAsking =  self->NumAnswerChoices = 0;  
    self->IsBlocking = TRUE;
    self->drawn = FALSE;
    self->DefaultWildestAnswer = self->StackPos = -1;
    self->PositionalPreference = message_Center;
    self->MultipleAnswers = NULL;
    while (self->uplist) {
	pu = self->uplist->next;
	free(self->uplist);
	self->uplist = pu;
    }
    if (self->HeightsOfAnswer) {
	free(self->HeightsOfAnswer);
	self->HeightsOfAnswer = NULL;
    }
    if(self->hasDialogMessage && self->dialogView  && self->UseBuiltinDialogs) (self->dialogView)->UnlinkTree();
    if (self->DialogBuffer != NULL) {
	if (self->DialogBufferView != NULL)
	    (self->DialogBuffer)->RemoveView( self->DialogBufferView);
	(self->DialogBuffer)->RemoveObserver( self);
    }
    self->DialogBuffer = NULL;
    self->DialogBufferView = NULL;
    self->DialogTargetView = NULL;
    self->hasDialogMessage = 0;
    self->PotentialChoice = 0;
}
static int isDialogChild(class frame  *self, class view  *v)
{
    while(v->parent != NULL && v->parent != (class view *) self) 
	v = v->parent;
    return(v == self->DialogBufferView || v == (class view *)self->dialogView);
}
void frame::WantUpdate(class view  *v)
{
    if (this->IsAsking && !this->UsingDialog) {
	struct pendingupdates *pu;
	if(isDialogChild(this,v)) {
	    (this)->lpair::WantUpdate(v);
	    return;
	}
	if (v != (class view *) this) {
	    pu = (struct pendingupdates *) malloc(sizeof (struct pendingupdates));
	    pu->next = this->uplist;
	    pu->v = v;
	    this->uplist = pu;
	}
	(this)->lpair::WantUpdate( this); 
	this->UpdateRequested = TRUE;
    } else {
	(this)->lpair::WantUpdate( v);
    }
}
/* destroywindowlater() performs the WindowManager-requested window destroy operation AFTER the im is back in its event loop -RSK*/
static void destroywindowlater(void *pdata, long time)
{
    framecmds::DeleteWindow((frame *)pdata);
}
/*
  * This function is called when the window manager requests
  * a delete window.
  */
static void delete_window_request(class im  *im, class frame  *self)
{
    /*framecmds::DeleteWindow(self);*/ /* this made AskForString crash if the window was prompting when you destroyed it -RSK*/
    if (self->dialogLine && (self->dialogLine)->Asking())
	(self->dialogLine)->CancelQuestion();
    if (self->messageLine && (self->messageLine)->Asking())
	(self->messageLine)->CancelQuestion();
    /*self->ReturnFocus();*/
    im->EnqueueEvent((event_fptr)destroywindowlater, (char *)self, 0);
}

void frame::LinkTree(class view  *parent)
{
    const char *fgcolor;
    class im *oldim=(this)->GetIM();
    (this)->lpair::LinkTree( parent);
    if(oldim) {
	(oldim)->SetDeleteWindowCallback( NULL, 0);
    }
    if((this)->GetIM() && ((this)->GetIM())->GetDeleteWindowCallback() == NULL) {
	((this)->GetIM())->SetDeleteWindowCallback( (im_deletefptr)delete_window_request, (long)this);
	fgcolor = environ::GetProfile("MessageLineBoundaryColor");
	if (fgcolor)
	    SetForegroundColor(fgcolor, 0, 0, 0);
    }
}


#define ERROR (-1)
int frame::DisplayString(int  priority, const char  *string)
{
    const char *mychoices[3];
    long result,defaultChoice;
    defaultChoice = 1;
    mychoices[0] = string;
    mychoices[1] = "continue";
    mychoices[2] = NULL;
    if(!ButtonInteract(this,mychoices,defaultChoice,&result,priority > frame_MUSTANSWER)){
	return(0);
    }
    return ERROR;
}

static void ReturnInterface(class frame  *rock, int  ind, long  brock)
{
    ConsiderReturning(rock, ind+1);
}


static class view *PrepareForStringInput(class frame  *self,const char  *prompt,int  bufferSize /* Is actual sizeof buffer including NUL. */,boolean  CompButtons)
{
    static const char *ans[6];
    class view *focus;
    self->AwaitingFocus = 1;
    focus = ((self)->GetIM())->GetInputFocus();
    if (focus) (self)->WantInputFocus(self); /* we take the focus ourself so that when the dialogview asks for it, no-one else has to redraw */
    while(im::Interact(0)); /* let the current focus get his update before the box is drawn */
    self->AwaitingFocus = 0;
    self->MultipleAnswers = ans;
    self->MultipleAnswers[0] = prompt;
    self->MultipleAnswers[ACCEPT] = "accept";
    self->MultipleAnswers[CANCEL] = "cancel";
    if(CompButtons) {
	self->MultipleAnswers[COMPLETE] = "complete";
	self->MultipleAnswers[HELP] = "help";
	self->MultipleAnswers[5] = NULL;
	self->NumAnswerChoices = 4 ;
    }
    else {
	self->MultipleAnswers[3] = NULL;
	self->NumAnswerChoices = 2 ;
    }
    self->IsAsking = 1;
    self->hasDialogMessage = TRUE;
    self->DefaultWildestAnswer = 0;
    self->WildestAnswer = -1;
    self->StackPos = ++frame_GlobalStackCount;
    ComputeSize(self);
    self->dv=NULL;
    if(!self->UseBuiltinDialogs) {
	self->dv=dialogv::Create(self->MultipleAnswers+1, NULL /*	Default	Font */, environ::GetProfileInt("dialogstyle", 4 /* sbutton_MOTIF */));
	if(self->dv) {
	    self->UsingDialog=TRUE;

	    if(CompButtons) (self->dv)->SetLayout( 2, 2);
	    else (self->dv)->SetLayout( 1, 2);

	    ((self->dv)->GetTextData())->InsertCharacters( 0, *self->MultipleAnswers, strlen(*self->MultipleAnswers));
	    ((self->dv)->GetTextData())->ReadTemplate( "dialog", FALSE);
	    (self->dv)->InstallRider( (class view *)self->dialogLine->messageView);
	}
    }
    PrepareMenus(self);
    if(self->dv) {
	long pos=im_Centered;
	switch(self->PositionalPreference) {
	    case message_OnTop:
		pos|=im_AtTop;
		break;
	    case message_OnBottom:
		pos|=im_AtBottom;
		break;
	    case message_Center:
		pos|=im_InMiddle;
	    default:
		break;
	}
	(self->dv)->PostInput( (self)->GetIM(), (self)->GetIM(), (dialogv_choicefptr)ReturnInterface, (long)self, self->IsBlocking, pos);
	(self->dv)->PostDefaultHandler( "message", self->messageLine);
    }
    if(!self->UpdateRequested)(self)->WantUpdate( self);
    while(im::Interact(0)); 
   	    
    return focus;
}
int frame::AskForString(int  priority, const char  *prompt , const char  *defaultString , char  *bufout, int  bufferSize /* Is actual sizeof buffer including NUL. */)
{
    int i;
    class view *focus;
    char *answer;
    if (this->IsAsking || this->AwaitingFocus) return(-1);
    answer=im::GetAnswer();
    if(answer==NULL) {
	if(im::AnswerWasCancel()) return (-1);

	focus = PrepareForStringInput(this,prompt,bufferSize,FALSE);
	(this)->SetReturnFocus( focus);
	i = (this->dialogLine)->AskForString( 0 ,"", defaultString, bufout, bufferSize);
	while(im::Interact(0));
	--frame_GlobalStackCount;
	retractCursors(this);
	if(!this->UsingDialog && !RestoreBits(this))
	    CannotRestoreBits(this);
	TidyUp(this);
	if(i>=0) im::RecordAnswer(bufout);
	else im::RecordCancellation();
	(this)->ReturnFocus();
    } else {
	strncpy(bufout, answer, bufferSize-1);
	bufout[bufferSize-1]='\0';
	return 0;
    }
    
    return i;
}
int frame::AskForPasswd(int  priority, const char  *prompt , const char  *defaultString , char  *bufout, int  bufoutSize /* Is actual sizeof bufout including NUL. */)
{
    int i;
    class view *focus;
    char *answer;
    if (this->IsAsking || this->AwaitingFocus) return ERROR;
    answer=im::GetAnswer();
    if(answer==NULL) {
	if(im::AnswerWasCancel()) return ERROR;

	focus = PrepareForStringInput(this,prompt,bufoutSize,FALSE);
	(this)->SetReturnFocus( focus);
	i = (this->dialogLine)->AskForPasswd( 0 ,"", defaultString, bufout, bufoutSize);
	while(im::Interact(0));
	--frame_GlobalStackCount;
	retractCursors(this);
	if(!this->UsingDialog && !RestoreBits(this))
	    CannotRestoreBits(this);
	TidyUp(this);
	if(i>=0) im::RecordAnswer(bufout);
	else im::RecordCancellation();
	(this)->ReturnFocus();
    } else {
	strncpy(bufout, answer, bufoutSize);
	bufout[bufoutSize-1]='\0';
	return 0;
    }
    return i;
}


int frame::AskForStringCompleted(int  priority, const char  *prompt , const char  *defaultString , char  *bufout, int  bufferSize, class keystate  *ks, message_completionfptr completionProc, message_helpfptr helpProc, long  completionData, int  flags)
{  
    int i=ERROR;
    class view *focus;
    char *answer;
    if (this->IsAsking || this->AwaitingFocus) return(-1);
    answer=im::GetAnswer();
    if(answer==NULL) {
	if(im::AnswerWasCancel()) return ERROR;

	focus = PrepareForStringInput(this,prompt,bufferSize,TRUE);
	(this)->SetReturnFocus( focus);
	i = (this->dialogLine)->AskForStringCompleted( 0 ,"", defaultString, bufout, bufferSize, ks, (message_completionfptr)completionProc, (message_helpfptr)helpProc, completionData, flags);

	while(im::Interact(0));

	--frame_GlobalStackCount;
	retractCursors(this);

	if(!this->UsingDialog && !RestoreBits(this))
	    CannotRestoreBits(this);

	TidyUp(this);

	if(i>=0) im::RecordAnswer(bufout);
	else im::RecordCancellation();
	

	(this)->ReturnFocus();
    } else {
	strncpy(bufout, answer, bufferSize);
	bufout[bufferSize-1]='\0';
	return 0;
    }
    return i;
}

int frame::MultipleChoiceQuestion(int  priority, const char  *prompt, long  defaultChoice, long  *result, const char  * const *choices, const char  *abbrevKeys)
{
    const char *mychoices[100];
    int i;
    char *answer=im::GetAnswer();
    if(answer==NULL) {
	if(im::AnswerWasCancel()) return ERROR;

	mychoices[0] = prompt;
	defaultChoice++;
	i = 0;
	do { 
	    mychoices[i+1] = choices[i];
	    if(choices[i] == '\0') break;
	} while (choices[i++]);
	if(!ButtonInteract(this,mychoices,defaultChoice,result,priority > frame_MUSTANSWER)){
	    (*result)--;
	    im::RecordAnswer(choices[*result]);
	    return(0);
	} else im::RecordCancellation();
    } else {
	i=0;
	while(*choices) {
	    if(strcmp(*choices, answer)==0) {
		*result=i;
		return 0;
	    }
	    i++;
	    choices++;
	}
    }
    return ERROR;
}

static void SingleLine(class frame  *self, long  key)
{
    self->messageView->lines=1;
    (self->messageView)->WantNewSize( self->messageView);

}


boolean frame::InitializeClass()
    {
    char c[2], *s;

    ResizableMessageLine = environ::GetProfileSwitch("ResizableMessageLine", TRUE);
    mykm = new keymap;
    returnconsidered = proctable::DefineProc("frame-consider-coming",  (proctable_fptr) ConsiderReturning, &frame_ATKregistry_ , NULL, "consider selecting an answer");
    cancel = proctable::DefineProc("frame-cancel",  (proctable_fptr) Cancel, &frame_ATKregistry_ , NULL, "Try to cancel dialog box");
    confirmAnswer = proctable::DefineProc("frame-confirm",  (proctable_fptr) ConfirmDefaultAnswer, &frame_ATKregistry_ , NULL, "Select the default answer");
    gotkey = proctable::DefineProc("frame-got-key",  (proctable_fptr) GotKey, &frame_ATKregistry_ , NULL, "change the default answer");

    proctable::DefineProc("frame-single-line-message", (proctable_fptr)SingleLine, &frame_ATKregistry_ , NULL, "Reset the message line to one line.");
    
    c[1] = '\0';
    s = c;
    for (*s = 'a'; *s <= 'z'; ++*s) {
	(mykm)->BindToKey( s, gotkey, *s);
    }
    for (*s = 'A'; *s <= 'Z'; ++*s) {
	(mykm)->BindToKey( s, gotkey, *s);
    }
    for (*s = '0'; *s <= '9'; ++*s) {
	(mykm)->BindToKey( s, gotkey, *s);
    }
    (mykm)->BindToKey( "\007", cancel, 0);
    (mykm)->BindToKey( "\015", confirmAnswer, 0);
    return TRUE;
}

static boolean
FindBuffer(class frame  *f,class buffer  *b)
{
/*
  Little, dippy routine passed to frame_Enumerate to find the
  frame which contains the buffer we want.
*/

  return((f)->GetBuffer()==b);
}

class frame *frame::FindFrameForBuffer(class buffer  *b)
{
	ATKinit;

/*
  This tries to find the frame of our buffer.  If there is no
  such frame, make one
*/

  class frame *f;

  if ((f = frame::Enumerate((frame_effptr)FindBuffer, (long) b)) == NULL) {
    /* No frame--need to map buffer to new window */

    if((f = new frame) == NULL) {
	fprintf(stderr,"frame: Could not allocate enough memory.\n");
	return NULL;
    }

    (f)->SetCommandEnable( TRUE);
    (f)->SetBuffer( b, TRUE);
  }
  return f;
}

class frame *frame::GetFrameInWindowForBuffer(class buffer  *b)
{
	ATKinit;

    class frame *f;
    class im *im;

    f = frame::FindFrameForBuffer(b);

    if (f != NULL) {
	if ((f)->GetIM() == NULL) {
	    if((im = im::Create(NULL)) == NULL) {
		fprintf(stderr,"frame: Could not create new window.\n");
		return NULL;
	    }
	    (im)->SetView( f);
	    (f)->PostDefaultHandler( "message", (f)->WantHandler( "message"));
	}
    }

    return f;
}

boolean frame::RecSearch(struct SearchPattern *pat, boolean toplevel)
{
    if (this->childView)
	return this->childView->RecSearch(pat, toplevel);
    return FALSE;
}

boolean frame::RecSrchResume(struct SearchPattern *pat)
{
    if (this->childView)
	return this->childView->RecSrchResume(pat);
    return FALSE;
}

boolean frame::RecSrchReplace(class dataobject *text, long pos, long len)
{
    if (this->childView)
	return this->childView->RecSrchReplace(text, pos, len);
    return FALSE;
}
