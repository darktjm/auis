/* ********************************************************************** *\
 *         Copyright IBM Corporation 1991 - All Rights Reserved           *
 *        For full copyright information see:'andrew/config/COPYRITE'     *
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

#ifndef NORCSID
#define NORCSID
static char rcsid[]="$Header: /afs/cs.cmu.edu/project/atk-src-C++/atk/hyplink/RCS/pshbttnv.C,v 1.22 1996/05/06 17:45:17 robr Exp $";
#endif

#include <andrewos.h>
ATK_IMPL("pshbttnv.H")
#include <sys/param.h>	/* for MAXPATHLEN */
#include <stdio.h>

#include <pushbutton.H>
#include <buffer.H>
#include <completion.H>
#include <cursor.H>
#include <environ.H>
#include <fontdesc.H>
#include <style.H>
#include <frame.H>
#include <graphic.H>
#include <im.H>
#include <menulist.H>
#include <message.H>
#include <observable.H>
#include <proctable.H>
#include <view.H>
#include <simpletext.H>
#include <textview.H>
#include <texttroff.H>
#include <search.H>
#include <print.H>

#include <pshbttnv.H>

/* Defined constants and macros */
#define CURSORON 1		/* yes, use crosshairs cursor over button */
#define NOROUNDRECT 1		/* round rect is broken */
#if 0
#define DEBUG 1			/* turn on debugging */
#endif
#ifdef DEBUG
#define DBG(x) fprintf(stderr, "\nDebug: %s.", x);fflush(stderr);
#else
#define DBG(x) ;
#endif

#define NO_MSG "Push Me"

#define PROMPTFONT "andysans12b"
#define FONT "andysans"
#define FONTTYPE fontdesc_Bold
#define FONTSIZE 12
#define BUTTONDEPTH 4
#define ULSHADE 0.25 /* upper & left sides */
#define LRSHADE 0.75 /* lower & right sides */
#define TOPSHADE 0.50 /* face of button */
#define MOTIFBUTTONDEPTH 2
#define MOTIFULSHADE 0.10
#define MOTIFLRSHADE 0.50
#define MOTIFTOPSHADE 0.25
#define BUTTONPRESSDEPTH 2
#define TEXTPAD 2

/* External Declarations */

/* Forward Declarations */
   

/* Global Variables */
static class atom *pushedtrigger;
static class menulist *pbvmenulist = NULL;
static class style *printstyle;


ATKdefineRegistry(pushbuttonview, view, pushbuttonview::InitializeClass);

static void pushbuttonview_setShade(class pushbuttonview  *self, double  pct);	/* 0.0 -> 1.0 */
static void pushbuttonview_CacheSettings(class pushbuttonview  *self, class pushbutton  *b, int  updateflag);	/* call WantUpdate if necessary */
static int RectEnclosesXY(struct rectangle  *r, long  x , long  y);
static void HighlightButton(class pushbuttonview  *self);
static void UnhighlightButton(class pushbuttonview  *self);
static void LabelProc(class ATK  *self, long  param);
static void FontProc(class ATK  *self, long  param);
static void StyleProc(class ATK  *self, long  param);
static void ColorProc(class ATK  *self, long  param);
static void OutputLabel(FILE  *f, char  *l);


static void
pushbuttonview_setShade(class pushbuttonview  *self, double  pct			/* 0.0 -> 1.0 */)
          {
    (self)->SetFGColor( 
			      self->foreground_color[0]*pct 
			      + self->background_color[0]*(1.0-pct), 
			      self->foreground_color[1]*pct 
			      + self->background_color[1]*(1.0-pct), 
			      self->foreground_color[2]*pct 
			      + self->background_color[2]*(1.0-pct));

} /* pushbuttonview_setShade */

#define DEFINEPROC(name, proc, registry, module, doc) proctable::DefineProc(name, (proctable_fptr)proc, registry, module, doc)

boolean
pushbuttonview::InitializeClass()
     {
    /* 
      Initialize all the class data.
      Set up the proc table entries and the menu list 
      (which is cloned for each instance of this class).
      */
    struct proctable_Entry *proc = NULL;

    if ((pushedtrigger = atom::Intern("buttonpushed")) == NULL) return(FALSE);

    if ((pbvmenulist = new menulist) == NULL) return(FALSE);

    if ((proc = DEFINEPROC("pushbuttonview-set-label-text", LabelProc, &pushbuttonview_ATKregistry_ , NULL, "Prompts for user to set the text string of the pushbutton.")) == NULL) return(FALSE);
    (pbvmenulist)->AddToML( "Pushbutton~1,Set Label~11", proc, 0, 0);
    
    if ((proc = DEFINEPROC("pushbuttonview-set-font", FontProc, &pushbuttonview_ATKregistry_ , NULL, "Prompts for user to set the font of the pushbutton.")) == NULL) return(FALSE);
    (pbvmenulist)->AddToML( "Pushbutton~1,Set Font~12", proc, 0, 0);
    
    if ((proc = DEFINEPROC("pushbuttonview-set-style", StyleProc, &pushbuttonview_ATKregistry_ , NULL, "Prompts for user to set the appearance of the pushbutton.")) == NULL) return(FALSE);
    (pbvmenulist)->AddToML( "Pushbutton~1,Set Style~13", proc, 0, 0);
    
    if ((proc = DEFINEPROC("pushbuttonview-set-color", ColorProc, &pushbuttonview_ATKregistry_ , NULL, "Prompts for user to set the foreground and background color of the pushbutton.")) == NULL) return(FALSE);
#ifndef PL8			/* users of PL8 can't set color */
    (pbvmenulist)->AddToML( "Pushbutton~1,Set Color~14", proc, 0, 0);
#endif /* PL8 */

    printstyle = new style;
    printstyle->SetName("pushbutton");
    printstyle->AddDottedBox();

    return(TRUE);
}


pushbuttonview::pushbuttonview()
{
	ATKinit;

/*
  Set up the data for each instance of the object.
*/
    int i;

    this->lit = 0;
    this->awaitingUpdate = 0;
    this->ml = (pbvmenulist)->DuplicateML( this);
    this->cached_label = "NOT YET CACHED";
    this->cached_style = -1;
    this->cached_fontdesc = NULL;
    this->recsearchvalid = FALSE;
    for (i = 0 ; i < 3; i++) {
	this->foreground_color[i] = 0.0;
	this->background_color[i] = 0.0;
    }
    
#if CURSORON
    rectangle_EmptyRect(&crect);
    if (!(this->cursor = cursor::Create(this))) THROWONFAILURE((FALSE));
    (this->cursor)->SetStandard( Cursor_Gunsight);
#endif /* CURSORON */
    observable::DefineTrigger(this->ATKregistry(), pushedtrigger);
    THROWONFAILURE((TRUE));
}


void
pushbuttonview::PostMenus(class menulist  *ml)
          {
    /*
      Enable the menus for this object.
      */

    (this->ml)->ClearChain();
    if (ml) (this->ml)->ChainAfterML( ml, (long)ml);
    (this)->view::PostMenus( this->ml);
}


static void
pushbuttonview_CacheSettings(class pushbuttonview  *self, class pushbutton  *b, int  updateflag		/* call WantUpdate if necessary */)
               {
    char *fgcolor, *bgcolor;
    unsigned char fg_rgb[3], bg_rgb[3];
    double get_rgb[3];
    int i;
    char *pb_label;
    int pb_style;
    class fontdesc *pb_font;
    int UpdateNeeded, NewSize;

    if (b && (self)->GetIM()) {
	    /* these calls don't make sense if there is no IM or button, 
	       (in fact they seg fault!) */

	UpdateNeeded = 0;
	NewSize = 0;		/* A new size requires an update */

	pb_label = (b)->GetText();
	if (pb_label != self->cached_label) {
	    self->cached_label = pb_label;
	    NewSize = 1;
	}

	pb_style = (b)->GetStyle();
	if (pb_style != self->cached_style) {
	    self->cached_style = pb_style;
	    NewSize = 1;
	}

	pb_font = (b)->GetButtonFont();
	if (pb_font != self->cached_fontdesc) {
	    self->cached_fontdesc = pb_font;
	    NewSize = 1;
	}

	fgcolor = (b)->GetFGColor( fg_rgb);
	bgcolor = (b)->GetBGColor( bg_rgb);

	/* We need to set the colors to find out their interpretation 
	   on the window server. */
	(self)->SetForegroundColor( 
					  fgcolor, 
					  fg_rgb[0]*256L, 
					  fg_rgb[1]*256L,
					  fg_rgb[2]*256L);
	(self)->SetBackgroundColor(
					  bgcolor,
					  bg_rgb[0]*256L,
					  bg_rgb[1]*256L,
					  bg_rgb[2]*256L);
	
	(self)->GetFGColor( 
				  &(get_rgb[0]), 
				  &(get_rgb[1]), 
				  &(get_rgb[2]));
	for(i = 0; i < 3; i++) {
	    if (get_rgb[i] != self->foreground_color[i]) {
		self->foreground_color[i] = get_rgb[i];
		UpdateNeeded = 1; /* color changed, not size */
	    }
	}

	(self)->GetBGColor( 
				  &(get_rgb[0]), 
				  &(get_rgb[1]), 
				  &(get_rgb[2]));
	for(i = 0; i < 3; i++) {
	    if (get_rgb[i] != self->background_color[i]) {
		self->background_color[i] = get_rgb[i];
		UpdateNeeded = 1; /* color changed, not size */
	    }
	}

	if (NewSize) {
	    (self)->WantNewSize(self);
	}
	if (updateflag && (NewSize || UpdateNeeded)) {
	    (self)->WantUpdate( self);
	}
    } /* if (b && im) */

}     


void
pushbuttonview::LinkTree(class view  *parent)
          {
    (this)->view::LinkTree( parent);

    pushbuttonview_CacheSettings(this, (class pushbutton *) (this)->GetDataObject(), 0);

} /* pushbuttonview__LinkTree */


pushbuttonview::~pushbuttonview()
{
	ATKinit;

#if CURSORON
  if (this->cursor) delete this->cursor;
  this->cursor = NULL;
#endif /* CURSORON */
  if(this->ml) delete this->ml;
  return;
}

static long ComputePos(class pushbuttonview *self, char *msg, class fontdesc *font, long left, long width, boolean *cont=NULL) {
    int mw, mh;
    font->StringBoundingBox(self->GetDrawable(), msg, &mw, &mh);
    if(mw+TEXTPAD>width) {
	if(cont) *cont=TRUE;
	return left+mw/2;
    }
    if(cont) *cont=FALSE;
    return left+width/2;
}

static struct point poly[3];
static void DrawContinued(class pushbuttonview *self, struct rectangle *inner) {
    poly[0].x=inner->left+inner->width-10;
    poly[0].y=inner->top;
    poly[1].x=inner->left+inner->width-1;
    poly[1].y=inner->top+inner->height/2;
    poly[2].x=poly[0].x;
    poly[2].y=inner->top+inner->height-1;
    self->FillPolygon(poly, 3, NULL);
}

static void DoSelectionHighlight(class pushbuttonview *self,char *textl, class fontdesc *my_fontdesc, long tx, struct rectangle *r, struct rectangle *hit, boolean draw)
{

    
    if(my_fontdesc && self->recsearchvalid && self->GetIM()) {
	int width, height;
	int swidth, sheight;
	long start=self->recsearchsubstart;
	long slen=self->recsearchsublen;
	char k;
	my_fontdesc->StringBoundingBox(self->GetDrawable(), textl, &width, &height);
	tx-=width/2;
	k=textl[start];
	textl[start]='\0';
	my_fontdesc->StringBoundingBox(self->GetDrawable(), textl, &swidth, &sheight);
	textl[start]=k;
	tx+=swidth;
	width-=swidth;
	my_fontdesc->StringBoundingBox(self->GetDrawable(), textl+start+slen, &swidth, &sheight);
	width-=swidth;
	if(hit) {
	    hit->top=r->top;
	    hit->left=tx;
	    hit->width=width;
	    hit->height=r->height;
	}
	if(draw) {
	    self->SetTransferMode(graphic_INVERT);
	    self->FillRectSize(tx, r->top, width, r->height, NULL);
	}
    }
}

static void LocateHit(class pushbuttonview *pv, const struct rectangle &logical, struct rectangle &hit) {
    struct rectangle Rect=logical, Rect2;
    class pushbutton *b = (class pushbutton *) (pv)->GetDataObject();
    int bdepth, r2_bot, r_bot;
    int tx = 0;
    char *textl;
    int style;
    class fontdesc *my_fontdesc;
    class graphic *my_graphic;
    b=(class pushbutton *)pv->dataobject;
    if (b) {
	style = (b)->GetStyle();
	my_graphic = (class graphic *)(pv)->GetDrawable();
	if (!(my_fontdesc = (b)->GetButtonFont())) {
	    my_fontdesc= fontdesc::Create(FONT, FONTTYPE, FONTSIZE);
	}
	if (my_fontdesc) {
	    (pv)->SetFont( my_fontdesc);
	}


	if(b->GetText()) {
	    textl = (b)->GetText();
	} else {
	    static char nomsgbuf[sizeof(NO_MSG)];
	    strcpy(nomsgbuf, NO_MSG);
	    textl=nomsgbuf;
	}

	switch (style) {
	    case pushbutton_BOXEDRECT:
		Rect.width--;
		Rect.height--;
		Rect2.top = Rect.top + BUTTONDEPTH;
		Rect2.left = Rect.left + BUTTONDEPTH;
		Rect2.width = Rect.width - 2*BUTTONDEPTH;
		Rect2.height = Rect.height - 2*BUTTONDEPTH;
		//tx = TEXTPAD + Rect2.left + (Rect2.width / 2);
		tx = ComputePos(pv, textl, my_fontdesc, Rect2.left, Rect2.width);
		Rect2.top++;
		Rect2.height--;
		DoSelectionHighlight(pv,textl,my_fontdesc, tx, &Rect2, &hit, FALSE);
		break;

	    case pushbutton_ROUNDRECT:

		Rect2.top = Rect.top;
		Rect2.left = Rect.left;
		Rect2.height = BUTTONDEPTH;
		Rect2.width = BUTTONDEPTH;
		Rect.width -= 1;
		Rect.height -= BUTTONDEPTH;

		//		tx = TEXTPAD + Rect.left + (Rect.width / 2);
		tx=ComputePos(pv, textl, my_fontdesc, Rect.left, Rect.width);
		DoSelectionHighlight(pv,textl,my_fontdesc, tx, &Rect, &hit, FALSE);
		break;

	    case pushbutton_MOTIF:
	    case pushbutton_THREEDEE:
		if (style == pushbutton_MOTIF) {
		    bdepth = MOTIFBUTTONDEPTH;
		} else {
		    bdepth = BUTTONDEPTH;
		}
		Rect2.top = Rect.top + bdepth;
		Rect2.left = Rect.left + bdepth;
		Rect2.width = Rect.width - 2*bdepth;
		Rect2.height = Rect.height - 2*bdepth;
		r2_bot = (Rect2.top)+(Rect2.height);
		r_bot = (Rect.top)+(Rect.height);

		// tx = TEXTPAD + Rect2.left + (Rect2.width / 2);
		tx=ComputePos(pv, textl, my_fontdesc, Rect2.left, Rect2.width);
		DoSelectionHighlight(pv,textl,my_fontdesc, tx, &Rect2, &hit, FALSE);
		break;

	    case pushbutton_PLAINBOX:
		Rect.width--;
		Rect.height--;

	//	tx = Rect.left + (Rect.width / 2);
		tx=ComputePos(pv, textl, my_fontdesc, Rect.left, Rect.width);
		Rect.top++;
		Rect.height--;
		DoSelectionHighlight(pv,textl,my_fontdesc, tx, &Rect, &hit, FALSE);

		break;

	    default: 
		//		tx = Rect.left + (Rect.width/ 2);
		tx=ComputePos(pv, textl, my_fontdesc, Rect.left, Rect.width);
		DoSelectionHighlight(pv,textl,my_fontdesc, tx, &Rect, &hit, FALSE);
		break;
	}
    }
}

void
pushbuttonview::FullUpdate(enum view_UpdateType  type, long  left , long  top , long  width , long  height)
{
/*
  Redisplay this object.  Specifically, set my font, and put my text label
  in the center of my display box.
*/

  struct rectangle Rect, Rect2;
  class pushbutton *b = (class pushbutton *) (this)->GetDataObject();
  int bdepth, r2_bot, r_bot;
  double ulshade, lrshade, topshade;
  int tx = 0, ty = 0;
  short t_op;
  char *textl;
  int style;
  class fontdesc *my_fontdesc;
  class graphic *my_graphic;
  struct FontSummary *my_FontSummary;
  int redraw;
  boolean cont=FALSE;
  if(GetIM()==NULL) return;
  
  this->awaitingUpdate = 0;
  (this)->GetLogicalBounds( &Rect);

  switch (type) {
    case view_FullRedraw:
    case view_LastPartialRedraw:
#if CURSORON
	if(!rectangle_IsEqualRect(&Rect,&crect)) {
	    if(!rectangle_IsEmptyRect(&crect)) {
		(this)->RetractCursor( this->cursor);
	    }
	    crect=Rect;
	    (this)->PostCursor( &Rect, this->cursor);
	}
#endif
      redraw = 1;
      break;
    case view_MoveNoRedraw:
#if CURSORON
	crect=Rect;
      (this)->PostCursor( &Rect, this->cursor);
#endif /* CURSORON */
      redraw = 0;
      break;
    case view_PartialRedraw:
      redraw = 0;
      break;
    case view_Remove:
#if CURSORON
	rectangle_EmptyRect(&crect);	
      (this)->RetractCursor( this->cursor);
#endif /* CURSORON */
      redraw = 0;
      break;
    default:
      redraw = 1;
  }


  if (b && redraw) {
    style = (b)->GetStyle();
    my_graphic = (class graphic *)(this)->GetDrawable();
    if (!(my_fontdesc = (b)->GetButtonFont())) {
      my_fontdesc= fontdesc::Create(FONT, FONTTYPE, FONTSIZE);
    }
    if (my_fontdesc) {
      (this)->SetFont( my_fontdesc);
      my_FontSummary =  (my_fontdesc)->FontSummary( my_graphic);
    }

    (this)->SetTransferMode( graphic_SOURCE);
    if ((style != pushbutton_THREEDEE) && (style != pushbutton_MOTIF)) {
	/* Erase with BG color, only if style is not 3-D (3-D draws all bits) */
	pushbuttonview_setShade(this, 0.0);
	(this)->FillRect( &Rect, NULL);
    }
    pushbuttonview_setShade(this, 1.0);

    t_op =  graphic_ATBASELINE;
    if(b->GetText()) {
	textl = (b)->GetText();
    } else {
	static char nomsgbuf[sizeof(NO_MSG)];
	strcpy(nomsgbuf, NO_MSG);
	textl=nomsgbuf;
    }
	
    ty = 0;			/* default, in case my_FontSummary is NULL */

    switch (style) {
    case pushbutton_BOXEDRECT:
      Rect.width--;
      Rect.height--;
      /* Rect2 is the inner rect */
      Rect2.top = Rect.top + BUTTONDEPTH;
      Rect2.left = Rect.left + BUTTONDEPTH;
      Rect2.width = Rect.width - 2*BUTTONDEPTH;
      Rect2.height = Rect.height - 2*BUTTONDEPTH;
      //	tx = TEXTPAD + Rect2.left + (Rect2.width / 2);
      tx=ComputePos(this, textl, my_fontdesc, Rect2.left, Rect2.width, &cont);
      if (my_FontSummary)
	ty = TEXTPAD + (Rect2.top + my_FontSummary->maxHeight - my_FontSummary->maxBelow);

      (this)->SetTransferMode( graphic_COPY);
      (this)->DrawRect( &Rect);
      (this)->DrawRect( &Rect2);
      (this)->MoveTo( tx, ty);
      (this)->DrawString( textl, t_op | graphic_BETWEENLEFTANDRIGHT);
      Rect2.top++;
      Rect2.height--;
      if(cont) DrawContinued(this, &Rect2);
      DoSelectionHighlight(this,textl,my_fontdesc, tx, &Rect2, NULL, GetIM()->GetInputFocus()==this);
      break;

    case pushbutton_ROUNDRECT:
      /* THIS CODE IS STILL BROKEN, DUE TO A LACK OF DOCUMENTATION ON RRect! */
      /* Rect2 is the inner rect for DrawRRect */
      Rect2.top = Rect.top;
      Rect2.left = Rect.left;
      Rect2.height = BUTTONDEPTH;
      Rect2.width = BUTTONDEPTH;
      Rect.width -= 1;
      Rect.height -= BUTTONDEPTH;

      // tx = TEXTPAD + Rect.left + (Rect.width / 2);
      tx=ComputePos(this, textl, my_fontdesc, Rect.left, Rect.width, &cont);
      if (my_FontSummary)
	ty = TEXTPAD + (Rect.top + my_FontSummary->maxHeight - my_FontSummary->maxBelow);

      (this)->SetTransferMode( graphic_COPY);
      (this)->DrawRRect( &Rect, &Rect2);
      (this)->MoveTo( tx, ty);
      (this)->DrawString( textl, t_op| graphic_BETWEENLEFTANDRIGHT);
      if(cont) DrawContinued(this, &Rect);
      DoSelectionHighlight(this,textl,my_fontdesc, tx, &Rect, NULL, GetIM()->GetInputFocus()==this);
      break;

    case pushbutton_MOTIF:
    case pushbutton_THREEDEE:
      if (style == pushbutton_MOTIF) {
	  bdepth = MOTIFBUTTONDEPTH;
	  ulshade = MOTIFULSHADE;
	  lrshade = MOTIFLRSHADE;
	  topshade = MOTIFTOPSHADE;
      } else {
	  bdepth = BUTTONDEPTH;
	  ulshade = ULSHADE;
	  lrshade = LRSHADE;
	  topshade = TOPSHADE;
      }


      /* Rect2 is the inner (Text) region */
      Rect2.top = Rect.top + bdepth;
      Rect2.left = Rect.left + bdepth;
      Rect2.width = Rect.width - 2*bdepth;
      Rect2.height = Rect.height - 2*bdepth;
      r2_bot = (Rect2.top)+(Rect2.height);
      r_bot = (Rect.top)+(Rect.height);

     // tx = TEXTPAD + Rect2.left + (Rect2.width / 2);
      tx=ComputePos(this, textl, my_fontdesc, Rect2.left, Rect2.width, &cont);
      if (my_FontSummary) {
	ty = TEXTPAD + (Rect2.top + my_FontSummary->maxHeight - my_FontSummary->maxBelow);
      }

#ifdef WM_ENV
      if (((this)->GetIM())->WhichWS()[0] == 'w') {
          (this)->FillRectSize( Rect.left, Rect.top, bdepth, Rect.height, (this)->GrayPattern( 1, 4));	/* left bar */

          (this)->FillRectSize( Rect.left + Rect.width - bdepth, Rect.top, bdepth, Rect.height, (this)->GrayPattern( 3, 4)); /* right bar */

          (this)->FillTrapezoid( Rect2.left, r2_bot, Rect2.width, Rect.left, r_bot, Rect.width, (this)->GrayPattern( 3, 4)); /* lower trapz */

          (this)->FillTrapezoid( Rect.left, Rect.top, Rect.width, Rect2.left, Rect2.top, Rect2.width, (this)->GrayPattern( 1, 4)); /* upper trapz */

          (this)->FillRect( &Rect2, (this)->GrayPattern(1,2)); /* the middle box */
      }
      else
#endif /* WM_ENV */
      {
          (this)->SetTransferMode( graphic_COPY);
          pushbuttonview_setShade(this, ulshade);
          (this)->FillRectSize( Rect.left, Rect.top, bdepth, Rect.height, NULL);	/* left bar */

          pushbuttonview_setShade(this, lrshade);
          (this)->FillRectSize( Rect.left + Rect.width - bdepth, Rect.top, bdepth, Rect.height, NULL); /* right bar */
          (this)->FillTrapezoid( Rect2.left, r2_bot, Rect2.width, Rect.left, r_bot, Rect.width, NULL); /* lower trapz */

          pushbuttonview_setShade(this, ulshade);
          (this)->FillTrapezoid( Rect.left, Rect.top, Rect.width, Rect2.left, Rect2.top, Rect2.width, NULL); /* upper trapz */

          pushbuttonview_setShade(this, topshade);
          (this)->FillRect( &Rect2, NULL); /* the middle box */

      }

      (this)->SetTransferMode( graphic_BLACK);
      if (style != pushbutton_MOTIF) {
	  pushbuttonview_setShade(this, 0.0);
          (this)->MoveTo( tx+1, ty);
          (this)->DrawString( textl, t_op| graphic_BETWEENLEFTANDRIGHT);
          (this)->MoveTo( tx, ty+1);
          (this)->DrawString( textl, t_op| graphic_BETWEENLEFTANDRIGHT);
          (this)->MoveTo( tx+1, ty+1);
          (this)->DrawString( textl, t_op| graphic_BETWEENLEFTANDRIGHT);
      }
      pushbuttonview_setShade(this, 1.0);
      (this)->MoveTo( tx, ty);
      (this)->DrawString( textl, t_op| graphic_BETWEENLEFTANDRIGHT);
      if(cont) DrawContinued(this, &Rect2);
      DoSelectionHighlight(this,textl,my_fontdesc, tx, &Rect2, NULL, GetIM()->GetInputFocus()==this);
      break;

    case pushbutton_PLAINBOX:
      Rect.width--;
      Rect.height--;
      (this)->DrawRect( &Rect);

    //  tx = Rect.left + (Rect.width / 2);
      tx=ComputePos(this, textl, my_fontdesc, Rect.left, Rect.width, &cont);
      ty = Rect.top + (Rect.height / 2);

      (this)->MoveTo( tx, ty);
      t_op =  graphic_BETWEENTOPANDBOTTOM;
      (this)->DrawString( textl, t_op| graphic_BETWEENLEFTANDRIGHT);
      Rect.top++;
      Rect.height--;
      if(cont) DrawContinued(this, &Rect);
      DoSelectionHighlight(this,textl,my_fontdesc, tx, &Rect, NULL, GetIM()->GetInputFocus()==this);

      break;
 
   default: /* PLAIN */
   //  tx = Rect.left + (Rect.width/ 2);
      tx=ComputePos(this, textl, my_fontdesc, Rect.left, Rect.width, &cont);
      if (my_FontSummary)
	ty = (Rect.top + my_FontSummary->maxHeight - my_FontSummary->maxBelow);

      (this)->SetTransferMode( graphic_COPY);
      (this)->MoveTo( tx, ty);
      (this)->DrawString( textl, t_op| graphic_BETWEENLEFTANDRIGHT);
      if(cont) DrawContinued(this, &Rect);
      DoSelectionHighlight(this,textl,my_fontdesc, tx, &Rect, NULL, GetIM()->GetInputFocus()==this);
      break;
    } /* switch (style) */
    this->lit=!this->lit;
    if(!this->lit) HighlightButton(this);
    else this->lit=FALSE;
  } /* if (b && redraw) */
  
}


void
pushbuttonview::Update()
{
/*
  Do an update.  Just set up the call to FullUpdate method.
*/
    struct rectangle r;

#if 0
    /* Shouldn't need this 'cuz full update fills the BG already  */
    (this)->EraseVisualRect();
#endif /* 0 */
    (this)->GetLogicalBounds( &r);
    (this)->FullUpdate( view_FullRedraw, r.left, r.top, r.width, r.height);
}

void pushbuttonview::ReceiveInputFocus()
{
    this->view::ReceiveInputFocus();
    this->WantUpdate(this);
}

void pushbuttonview::LoseInputFocus()
{
    this->view::LoseInputFocus();
    this->WantUpdate(this);
}


static int
RectEnclosesXY(struct rectangle  *r, long  x , long  y)
{
  return(   ( ((r->top)  <= y) && ((r->top + r->height) >= y) )
	 && ( ((r->left) <= x) && ((r->left + r->width) >= x) )
	 );
}


static void
HighlightButton(class pushbuttonview  *self)
{
  class pushbutton *b = (class pushbutton *) (self)->GetDataObject();
  struct rectangle Rect, Rect2;
  int style;
  class fontdesc *my_fontdesc;
  class graphic *my_graphic;
  struct FontSummary *my_FontSummary;
  int tx, ty;
  short t_op;
  char *text;
  int bdepth, r2_bot, r_bot;
  double ulshade, lrshade;
  char *textl;
  if(self->GetIM()==NULL) return;
  if(b->GetText()) {
      textl = (b)->GetText();
  } else {
      static char nomsgbuf[sizeof(NO_MSG)];
      strcpy(nomsgbuf, NO_MSG);
      textl=nomsgbuf;
  }
  if (!(self->lit)) {
    style = (b)->GetStyle();
    (self)->GetLogicalBounds( &Rect);
    
    switch (style) {
    case pushbutton_PLAIN:
    case pushbutton_PLAINBOX:
      (self)->SetTransferMode( graphic_INVERT);
      (self)->FillRect(&Rect,(self)->BlackPattern());
      break;

    case pushbutton_BOXEDRECT:
      /* Rect2 is the inner rect */
      Rect2.top = Rect.top + BUTTONDEPTH;
      Rect2.left = Rect.left + BUTTONDEPTH;
      Rect2.width = Rect.width - 2*BUTTONDEPTH;
      Rect2.height = Rect.height - 2*BUTTONDEPTH;

      (self)->SetTransferMode( graphic_INVERT);
      (self)->FillRect( &Rect,(self)->BlackPattern());
      (self)->FillRect( &Rect2,(self)->BlackPattern());

      break;
    case pushbutton_ROUNDRECT:
      /* Rect2 is the inner rect for DrawRRect */
      Rect2.top = Rect.top;
      Rect2.left = Rect.left;
      Rect2.height = BUTTONDEPTH;
      Rect2.width = BUTTONDEPTH;
      Rect.width -= 1;
      Rect.height -= BUTTONDEPTH;

      (self)->SetTransferMode( graphic_INVERT);
      (self)->FillRRect( &Rect, &Rect2, (self)->BlackPattern());

      break;
  
    case pushbutton_MOTIF:
    case pushbutton_THREEDEE:
      if (style == pushbutton_MOTIF) {
	  bdepth = MOTIFBUTTONDEPTH;
	  ulshade = MOTIFLRSHADE;
	  lrshade = MOTIFULSHADE;
      } else {
	  bdepth = BUTTONDEPTH;
	  ulshade = ULSHADE;
	  lrshade = LRSHADE;
      }

      /* Rect2 is the inner (Text) region */
      Rect2.top = Rect.top + bdepth;
      Rect2.left = Rect.left + bdepth;
      Rect2.width = Rect.width - 2*bdepth;
      Rect2.height = Rect.height - 2*bdepth;
      r2_bot = (Rect2.top)+(Rect2.height);
      r_bot = (Rect.top)+(Rect.height);

      if (style == pushbutton_MOTIF) {
          (self)->SetTransferMode( graphic_COPY);
          pushbuttonview_setShade(self, ulshade);
          (self)->FillRectSize( Rect.left, Rect.top, bdepth, Rect.height, NULL);	/* left bar */
	  
          pushbuttonview_setShade(self, lrshade);
          (self)->FillRectSize( Rect.left + Rect.width - bdepth, Rect.top, bdepth, Rect.height, NULL); /* right bar */
          (self)->FillTrapezoid( Rect2.left, r2_bot, Rect2.width, Rect.left, r_bot, Rect.width, NULL); /* lower trapz */
	  
          pushbuttonview_setShade(self, ulshade);
          (self)->FillTrapezoid( Rect.left, Rect.top, Rect.width, Rect2.left, Rect2.top, Rect2.width, NULL); /* upper trapz */
	  
          pushbuttonview_setShade(self, 1.0);
	  
      } else {
	  boolean cont=FALSE;
	  my_graphic = (class graphic *)(self)->GetDrawable();
	  if (!(my_fontdesc = (b)->GetButtonFont())) {
	      my_fontdesc= fontdesc::Create(FONT, FONTTYPE, FONTSIZE);
	  }
	  if (my_fontdesc) {
	      (self)->SetFont( my_fontdesc);
	      my_FontSummary =  (my_fontdesc)->FontSummary( my_graphic);
	  }
	  t_op = graphic_ATBASELINE;
	  text = (b)->GetText() ? (b)->GetText() : NO_MSG;
	  // tx = TEXTPAD + (Rect2.left + Rect2.width) / 2;
	  tx=ComputePos(self, textl, my_fontdesc, Rect2.left, Rect2.width, &cont);
	  if (my_FontSummary)
	    ty = TEXTPAD + (Rect2.top + my_FontSummary->maxHeight - my_FontSummary->maxBelow);
	  
	  pushbuttonview_setShade(self, 0.0);
	  (self)->SetTransferMode( graphic_BLACK);
	  (self)->MoveTo( tx, ty);
	  (self)->DrawString( text, t_op| graphic_BETWEENLEFTANDRIGHT);
	  pushbuttonview_setShade(self, 1.0);
	  if(cont) DrawContinued(self, &Rect);
	  DoSelectionHighlight(self,textl,my_fontdesc, tx, &Rect, NULL, self->GetIM()->GetInputFocus()==self);
      } /* MOTIF? */
      
      break;
    }
  }
  self->lit = 1;
}


static void
UnhighlightButton(class pushbuttonview  *self)
{
  class pushbutton *b = (class pushbutton *) (self)->GetDataObject();
  struct rectangle Rect, Rect2;
  int style;
  class fontdesc *my_fontdesc;
  class graphic *my_graphic;
  struct FontSummary *my_FontSummary;
  int tx, ty;
  short t_op;
  char *text;
  int bdepth, r2_bot, r_bot;
  double ulshade, lrshade;
  char *textl;
  if(self->GetIM()==NULL) return;
  if(b->GetText()) {
      textl = (b)->GetText();
  } else {
      static char nomsgbuf[sizeof(NO_MSG)];
      strcpy(nomsgbuf, NO_MSG);
      textl=nomsgbuf;
  }
  if (self->lit) {
    style = (b)->GetStyle();
    (self)->GetLogicalBounds( &Rect);
    
    switch (style) {
    case pushbutton_PLAIN:
    case pushbutton_PLAINBOX:
      (self)->SetTransferMode( graphic_INVERT);
      (self)->FillRect(&Rect,(self)->BlackPattern());
      break;

    case pushbutton_BOXEDRECT:
      /* Rect2 is the inner rect */
      Rect2.top = Rect.top + BUTTONDEPTH;
      Rect2.left = Rect.left + BUTTONDEPTH;
      Rect2.width = Rect.width - 2*BUTTONDEPTH;
      Rect2.height = Rect.height - 2*BUTTONDEPTH;

      (self)->SetTransferMode( graphic_INVERT);
      (self)->FillRect( &Rect,(self)->BlackPattern());
      (self)->FillRect( &Rect2,(self)->BlackPattern());

      break;
    case pushbutton_ROUNDRECT:
      /* Rect2 is the inner rect for DrawRRect */
      Rect2.top = Rect.top;
      Rect2.left = Rect.left;
      Rect2.height = BUTTONDEPTH;
      Rect2.width = BUTTONDEPTH;
      Rect.width -= 1;
      Rect.height -= BUTTONDEPTH;

      (self)->SetTransferMode( graphic_INVERT);
      (self)->FillRRect( &Rect, &Rect2, (self)->BlackPattern());

      break;

    case pushbutton_MOTIF:
    case pushbutton_THREEDEE:
      if (style == pushbutton_MOTIF) {
	  bdepth = MOTIFBUTTONDEPTH;
	  ulshade = MOTIFULSHADE;
	  lrshade = MOTIFLRSHADE;
      } else {
	  bdepth = BUTTONDEPTH;
	  ulshade = ULSHADE;
	  lrshade = LRSHADE;
      }


      /* Rect2 is the inner (Text) region */
      Rect2.top = Rect.top + bdepth;
      Rect2.left = Rect.left + bdepth;
      Rect2.width = Rect.width - 2*bdepth;
      Rect2.height = Rect.height - 2*bdepth;
      r2_bot = (Rect2.top)+(Rect2.height);
      r_bot = (Rect.top)+(Rect.height);

      if (style == pushbutton_MOTIF) {
          (self)->SetTransferMode( graphic_COPY);
          pushbuttonview_setShade(self, ulshade);
          (self)->FillRectSize( Rect.left, Rect.top, bdepth, Rect.height, NULL);	/* left bar */
	  
          pushbuttonview_setShade(self, lrshade);
          (self)->FillRectSize( Rect.left + Rect.width - bdepth, Rect.top, bdepth, Rect.height, NULL); /* right bar */
          (self)->FillTrapezoid( Rect2.left, r2_bot, Rect2.width, Rect.left, r_bot, Rect.width, NULL); /* lower trapz */
	  
          pushbuttonview_setShade(self, ulshade);
          (self)->FillTrapezoid( Rect.left, Rect.top, Rect.width, Rect2.left, Rect2.top, Rect2.width, NULL); /* upper trapz */
	  
          pushbuttonview_setShade(self, 1.0);
	  
      } else {
	  boolean cont=FALSE;
	  my_graphic = (class graphic *)(self)->GetDrawable();
	  if (!(my_fontdesc = (b)->GetButtonFont())) {
	      my_fontdesc= fontdesc::Create(FONT, FONTTYPE, FONTSIZE);
	  }
	  if (my_fontdesc) {
	      (self)->SetFont( my_fontdesc);
	      my_FontSummary =  (my_fontdesc)->FontSummary( my_graphic);
	  }
	  t_op = graphic_ATBASELINE;
	  text = (b)->GetText() ? (b)->GetText() : NO_MSG;
	  //  tx = TEXTPAD + (Rect2.left + Rect2.width) / 2;
	  tx=ComputePos(self, textl, my_fontdesc, Rect2.left, Rect2.width, &cont);
	  if (my_FontSummary)
	    ty = TEXTPAD + (Rect2.top + my_FontSummary->maxHeight - my_FontSummary->maxBelow);
	  
	  (self)->SetTransferMode( graphic_BLACK);
	  (self)->MoveTo( tx, ty);
	  (self)->DrawString( text, t_op| graphic_BETWEENLEFTANDRIGHT);
	  if(cont) DrawContinued(self, &Rect);
	  DoSelectionHighlight(self,textl,my_fontdesc, tx, &Rect, NULL, self->GetIM()->GetInputFocus()==self);
      } /* MOTIF? */
      
      break;
    }
  }
  self->lit = 0;
}


class view *
pushbuttonview::Hit(enum view_MouseAction  action, long  x , long  y, long  numclicks  )
{
/*
  Handle the button event.  Currently, semantics are:
    
    Left Down  -- Draw button pressed
    Right Down -- select button (Receive input focus, for menuing without activating)
    Left Up    -- draw button at rest, pull trigger
    Right Up   -- No Op
    Left Movement     -- unhighlight if moved off, highlight if moved on
    Right Movement -- No Op
*/
  class cursor *wait_cursor;
  
  switch (action) {
  case view_LeftDown: 
    HighlightButton(this);
    (this)->WantInputFocus(this);
    break;
  case view_LeftMovement:
    {
      struct rectangle r;

      (this)->GetVisualBounds( &r);
      if (RectEnclosesXY(&r, x, y))
	HighlightButton(this);
      else
	UnhighlightButton(this);
    }
    break;
  case view_LeftUp:
    {
      short litp = this->lit;

      UnhighlightButton(this);
      if (litp) {
	if (wait_cursor = cursor::Create(this)) {
	  (wait_cursor)->SetStandard( Cursor_Wait);
	  im::SetProcessCursor(wait_cursor);
	  ((class pushbutton *) (this)->GetDataObject())->PullTrigger( pushedtrigger);
	  (this)->PullTrigger( pushedtrigger);
	  im::SetProcessCursor(NULL);
	  delete wait_cursor;
	}
      }
    }
    break;
  case view_RightDown:
    (this)->WantInputFocus( this);
    break;
  }
  return((class view *)this);
}


void
pushbuttonview::ObservedChanged(class observable  *b, long  v)
               {
    (this)->view::ObservedChanged( b, v);
    pushbuttonview_CacheSettings(this, (class pushbutton *)b, 1);
}


view_DSattributes 
pushbuttonview::DesiredSize(long  width, long  height, enum view_DSpass  pass, long  *desired_width, long  *desired_height)
{
/* 
  Tell parent that this object  wants to be as big as the box around its
  text string.  For some reason IM allows resizing of this object. (BUG)
*/

  class fontdesc *my_fontdesc;
  struct FontSummary *my_FontSummary;
  class graphic *my_graphic;
  class pushbutton *b = (class pushbutton *) (this)->GetDataObject();
  int style;

  style = (b)->GetStyle();

  my_graphic = (class graphic *)(this)->GetDrawable();
  if (!(my_fontdesc = (b)->GetButtonFont())) {
    my_fontdesc= fontdesc::Create(FONT, FONTTYPE, FONTSIZE);
  }
  if (my_fontdesc) {
    (my_fontdesc)->StringSize( my_graphic, (b)->GetText() ? (b)->GetText() : NO_MSG, desired_width, desired_height);
    my_FontSummary =  (my_fontdesc)->FontSummary( my_graphic);
  }

  switch (style) {
  case pushbutton_PLAIN:
  case pushbutton_PLAINBOX:
    if (my_FontSummary)
	*desired_height = my_FontSummary->maxHeight;
      *desired_width+=2*TEXTPAD;
    break;
  case pushbutton_MOTIF:
    *desired_width = *desired_width + 2*TEXTPAD + 2*MOTIFBUTTONDEPTH;
    if (my_FontSummary)
      *desired_height = my_FontSummary->maxHeight + 2*TEXTPAD + 2*MOTIFBUTTONDEPTH;
    break;
  case pushbutton_BOXEDRECT:
  case pushbutton_ROUNDRECT:
  case pushbutton_THREEDEE:
    *desired_width = *desired_width + 2*TEXTPAD + 2*BUTTONDEPTH;
    if (my_FontSummary)
      *desired_height = my_FontSummary->maxHeight + 2*TEXTPAD + 2*BUTTONDEPTH;
    break;
  }

/*
  (BUG) I don't check to see if I can specify a size, I just do it.
  Will this break things?  What if I can't change my size?  Will I be
  Ugly?  What to do, what to do....
*/

  return(view_Fixed); /* (BUG) should disable user sizing, but this doesn't */
}


void
pushbuttonview::GetOrigin(long  width , long  height, long  *originX , long  *originY)
{
/*
  We want this object to sit in-line with text, not below the baseline.
  Simply, we could negate the height as the originX, but then our
  text would be too high.  So, instead, we use the height of
  our font above the baseline
*/

  struct FontSummary *my_FontSummary;
  class fontdesc *my_fontdesc;
  class graphic *my_graphic;
  class pushbutton *b = (class pushbutton *) (this)->GetDataObject();
  int style;

  style = (b)->GetStyle();

  my_graphic = (class graphic *)(this)->GetDrawable();
  if (!(my_fontdesc = (b)->GetButtonFont())) {
    my_fontdesc= fontdesc::Create(FONT, FONTTYPE, FONTSIZE);
  }
  if (my_fontdesc) {
    my_FontSummary =  (my_fontdesc)->FontSummary( my_graphic);
  }

  *originX = 0;
  switch (style) {
    case pushbutton_PLAIN:
    case pushbutton_PLAINBOX:
      if (my_FontSummary)
	*originY = (my_FontSummary->maxHeight) - (my_FontSummary->maxBelow) + 1;
      break;
    case pushbutton_MOTIF:
      if (my_FontSummary) 
	*originY = (my_FontSummary->maxHeight) - (my_FontSummary->maxBelow) + 1 + TEXTPAD + MOTIFBUTTONDEPTH;
      break;
    case pushbutton_BOXEDRECT:
    case pushbutton_ROUNDRECT:
    case pushbutton_THREEDEE:
      if (my_FontSummary) 
	*originY = (my_FontSummary->maxHeight) - (my_FontSummary->maxBelow) + 1 + TEXTPAD + BUTTONDEPTH;
      break;
  }
  return;
}


static void
LabelProc(class ATK  *aself, long  param)
{
    class pushbuttonview *self=(class pushbuttonview *)aself;
    /*
      This is the routine which asks the user for a new text label.
      (BUG) Should not allow newlines (or should it?)
      */

    char buf[MAXPATHLEN];
    class pushbutton *b = (class pushbutton *)(self)->GetDataObject();
    char *oldtext;

    oldtext = (b)->GetSafeText();
    if (message::AskForString(self,50,"Enter new text for button: ",
			     oldtext, buf, sizeof(buf)) >= 0) {
	(b)->SetText( buf);
	message::DisplayString(self, 10, "Changed button text.");
			     }
}


static void
FontProc(class ATK  *aself, long  param)
{
    class pushbuttonview *self=(class pushbuttonview *)aself;
/*
  This is the routine which asks the user for a new font.
  It sucks, but I don't know how to smoothly integrate this button
  with a textview-like font change.  Oh well.
*/

  char buf[MAXPATHLEN], name[MAXPATHLEN];
  long style, size;
  class pushbutton *b = (class pushbutton *)(self)->GetDataObject();
  class fontdesc *fd;

  if (message::AskForString(self,50,"Enter new fontname for pushbutton: ", PROMPTFONT, buf, sizeof(buf)) >= 0) {
      if (!fontdesc::ExplodeFontName(buf, name, sizeof(name), &style, &size)) {
	  message::DisplayString(self, 50, "Couldn't parse fontname.");
	  return;
      }
      if ((fd = fontdesc::Create(name,style,size))!=NULL) {
	  (b)->SetButtonFont( fd);
	  message::DisplayString(self, 10, "Changed font.");
      } else {
	  message::DisplayString(self, 50, "Font change failed.  Using old font.");
      }
  }
}


static void
StyleProc(class ATK  *aself, long  param)
{
    class pushbuttonview *self=(class pushbuttonview *)aself;
    /*
      This is the routine which asks the user for a new pushbutton appearance.
      */

    class pushbutton *b = (class pushbutton *)(self)->GetDataObject();
    static char *style_menu[] = {
	"Plain Text",
	"Boxed Text",
	"Three Dimensional",
#ifndef NOROUNDRECT
	"Rounded Rect",
#endif
        "Simple Boxed Text",
        "OSF/Motif",
	NULL
    };
    long choice;

    if (message::MultipleChoiceQuestion(self,99,"Pick a new style:", (b)->GetStyle(), &choice, style_menu, NULL)>= 0) {
#ifdef NOROUNDRECT
        if (choice >= pushbutton_ROUNDRECT) {
            choice++;
        }
#endif
	(b)->SetStyle( choice);
	message::DisplayString(self, 10, "Changed button style.");
    } else {
	message::DisplayString(self, 10, "Choice cancelled.");
    }
}


static void
ColorProc(class ATK  *aself, long  param)
{
    class pushbuttonview *self=(class pushbuttonview *)aself;
    /*
      This is the routine which asks the user for a new pushbutton colors.
      */

    char buf[MAXPATHLEN], rgb_buf[10];
    class pushbutton *b = (class pushbutton *)(self)->GetDataObject();
    char *oldcolor;
    unsigned char rgb[3];

    oldcolor = (b)->GetFGColor( rgb);
    if (oldcolor == NULL) {
	sprintf(rgb_buf, "0x%02x%02x%02x", rgb[0], rgb[1], rgb[2]);
	oldcolor = rgb_buf;
    }
    if (message::AskForString(self,50,"Enter new foreground color for button: ", oldcolor, buf, sizeof(buf)) >= 0) {
	if ((strncmp(buf,"0x",2) != 0) 
	    && (strncmp(buf,"0X",2) != 0)) {
	    (b)->SetFGColor( buf, 0, 0, 0);
	    message::DisplayString(self, 10, "Changed foreground color.");
	} else {
	    (b)->ParseRGB( buf, rgb);
	    (b)->SetFGColor( NULL, rgb[0], rgb[1], rgb[2]);
	    sprintf(buf, "Changed button foreground color to 0x%x%x%x.", rgb[0], rgb[1], rgb[2]);
	    message::DisplayString(self, 10, buf);
	}
    }

    oldcolor = (b)->GetBGColor( rgb);
    if (oldcolor == NULL) {
	sprintf(rgb_buf, "0x%02x%02x%02x", rgb[0], rgb[1], rgb[2]);
	oldcolor = rgb_buf;
    }
    if (message::AskForString(self,50,"Enter new background color for button: ", oldcolor, buf, sizeof(buf)) >= 0) {
	if ((strncmp(buf,"0x",2) != 0) 
	    && (strncmp(buf,"0X",2) != 0)) {
	    (b)->SetBGColor( buf, 0, 0, 0);
	    message::DisplayString(self, 10, "Changed background color.");
	} else {
	    (b)->ParseRGB( buf, rgb);
	    (b)->SetBGColor( NULL, rgb[0], rgb[1], rgb[2]);
	    sprintf(buf, "Changed button background color to 0x%x%x%x.", rgb[0], rgb[1], rgb[2]);
	    message::DisplayString(self, 10, buf);
	}
    }
}


void pushbuttonview::WantUpdate(class view  *requestor)
          {
    if ((class view *) this == requestor) {
	if (this->awaitingUpdate) {
	    return;
	}
	this->awaitingUpdate = TRUE;
    }
    (this)->view::WantUpdate( requestor);
} /* pushbuttonview__WantUpdate */

static void OutputLabel(FILE  *f, char  *l)
{
    if(l==NULL) l=NO_MSG;
    while(*l) {
	if(l[0]=='\\') fprintf(f, "\\\\\\\\");
	else if(l[0]=='\"') fprintf(f, "\"\"");
	else fprintf(f, "%c", l[0]);
	l++;
    }
}

void pushbuttonview::Print(register FILE   *file, char    *processor, char    *format, boolean    topLevel)
{
    int count;
    register class pushbutton *dobj = (class pushbutton *)this->dataobject;
    if (strcmp(processor, "troff") == 0) {
	/* output to troff */
	if (topLevel)
	    /* take care of initial troff stream */
	    texttroff::BeginDoc(file);

	fprintf(file, ".de bx\n\\(br\\|\\\\$1\\|\\(br\\l'|0\\(rn'\\l'|0\\(ul'\n..\n");
	fprintf(file, ".bx \"");
	OutputLabel(file, (dobj)->GetSafeText());
	fprintf(file, "\"");
	fprintf(file, "\n");
	if (topLevel)
	    texttroff::EndDoc(file);

    } else {
	/* guess we're trying to write in postscript */
	class fontdesc *fd = (dobj)->GetButtonFont();
	char *ffam = FONT;
	long fsiz = FONTSIZE;
	long fsty = FONTTYPE;
	char fontname[256], *prefix;
	short *encoding;

	if (strcmp(format, "troff") == 0)
	    prefix = "\\!  ";
	else prefix = "";

	if (fd) {
	    ffam = (fd)->GetFontFamily();
	    fsiz = (fd)->GetFontSize();
	    fsty = (fd)->GetFontStyle();
	}

	print::LookUpPSFont(fontname, &encoding, NULL, ffam, fsiz, fsty);

	fprintf(file, "%s  /%s findfont %d scalefont setfont\n", prefix, fontname, fsiz);

	fprintf(file, "%s  0 0 moveto (", prefix);	
	print::OutputPSString(file, (dobj)->GetSafeText(), -1);
	fprintf(file, ") show\n");
    }
}

static char *PrintLump(void *rock, class style **styptr)
{
    class pushbuttonview *self = (class pushbuttonview *)rock;
    class pushbutton *dobj = (class pushbutton *)(self->GetDataObject());
    char *tx, *res;

    if (styptr) {
	/*  *styptr = NULL;  */
	*styptr = printstyle;
    }

    tx = dobj->GetSafeText();
    res = (char *)malloc(strlen(tx)+1);
    strcpy(res, tx);
    return res;
}

void *pushbuttonview::GetPSPrintInterface(char *printtype)
{
    static struct textview_insetdata dat;

    if (!strcmp(printtype, "text")) {
	dat.type = textview_StitchString;
	dat.u.StitchString.rock = (void *)this;
	dat.u.StitchString.func = PrintLump;
	return (void *)(&dat);
    }

    if (!strcmp(printtype, "generic")) {
	return (void *)this;
    }

    return NULL;

}

void pushbuttonview::PrintPSRect(FILE *outfile, long logwidth, long logheight, struct rectangle *visrect)
{
    class pushbutton *dobj = (class pushbutton *)(this->GetDataObject());
    class fontdesc *fd = dobj->GetButtonFont();
    char *ffam = FONT;
    long fsiz = FONTSIZE;
    long fsty = FONTTYPE;
    char fontname[256];
    short *encoding;
    int val;

    if (fd) {
	ffam = (fd)->GetFontFamily();
	fsiz = (fd)->GetFontSize();
	fsty = (fd)->GetFontStyle();
    }

    print::LookUpPSFont(fontname, &encoding, NULL, ffam, fsiz, fsty);
    val = print::PSRegisterFont(fontname);

    fprintf(outfile, "%s%d %d scalefont setfont\n", print_PSFontNameID, val, fsiz);

    fprintf(outfile, "%d %d moveto\n", logwidth/2, (logheight-fsiz)/2);	
    fprintf(outfile, "(");	
    print::OutputPSString(outfile, (dobj)->GetSafeText(), -1);
    fprintf(outfile, ")\n");
    fprintf(outfile, "dup stringwidth pop -2 div 0 rmoveto show\n");

}

boolean pushbuttonview::RecSearch(struct SearchPattern *pat, boolean toplevel)
{
    char *ts;
    int substart;
    class pushbutton *d;

    d = (class pushbutton *) (this)->GetDataObject();
    if (!d || !(ts = d->GetSafeText())) {
	this->recsearchvalid = FALSE;
	return FALSE;
    }

    substart = search::MatchPatternStr((unsigned char *)ts, 0, strlen(ts), pat);
    if (substart>=0) {
	this->recsearchvalid = TRUE;
	this->recsearchsubstart = substart;
	this->recsearchsublen = search::GetMatchLength();
	return TRUE;
    }

    this->recsearchvalid = FALSE;
    return FALSE;
}

boolean pushbuttonview::RecSrchResume(struct SearchPattern *pat)
{
    if(this->recsearchvalid) {
	char *ts;
	int substart=(-1);
	class pushbutton *d;
	int len;
	d = (class pushbutton *) (this)->GetDataObject();
	if (!d || !(ts = d->GetSafeText())) {
	    this->recsearchvalid = FALSE;
	    return FALSE;
	}
	this->recsearchsubstart++;
	this->recsearchsublen=0;
	len=strlen(ts);
	if(len-this->recsearchsubstart>0) substart = search::MatchPatternStr((unsigned char *)ts, this->recsearchsubstart, len-this->recsearchsubstart, pat);
	if (substart>=0) {
	    this->recsearchvalid = TRUE;
	    this->recsearchsubstart = substart;
	    this->recsearchsublen = search::GetMatchLength();
	    return TRUE;
	}
	this->recsearchvalid = FALSE;
	return FALSE;
    } else {
	this->recsearchvalid = FALSE;
    }
    return FALSE;
}

boolean pushbuttonview::RecSrchReplace(class dataobject *srcdobj, long srcpos, long srclen)
{
    class pushbutton *d = (class pushbutton *) (this)->GetDataObject();

    char *buf;
    char *ts;
    int substart, sublen;
    class simpletext *srctext;

    if (!this->recsearchvalid)
	return FALSE;

    if (!srcdobj->IsType("simpletext")) {
	message::DisplayString(this, 0, "Replacement object is not text.");
	return FALSE;
    }
    srctext = (class simpletext *)srcdobj;

    ts = d->GetSafeText();
    if (!ts)
	ts = "";
    substart = this->recsearchsubstart;
    sublen = this->recsearchsublen;
    buf = (char *)malloc(strlen(ts) - sublen + srclen + 2);
    strncpy(buf, ts, substart);
    srctext->CopySubString(srcpos, srclen, buf+substart, FALSE);
    strcpy(buf+substart+srclen, ts+substart+sublen);
    this->recsearchsublen = srclen;

    d->SetText(buf);
    free(buf);

    return TRUE;
}

void pushbuttonview::RecSrchExpose(const struct rectangle &logical, struct rectangle &hit)
{
    if (!this->recsearchvalid)
	return;
  
    LocateHit(this, logical, hit);
    this->WantInputFocus(this);
    this->WantUpdate(this);
}
