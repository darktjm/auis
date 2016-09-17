/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1989 - All Rights Reserved      *
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

#include <andrewos.h>
ATK_IMPL("buttonV.H")
#include <buttonV.H>
#include <fontdesc.H>
#include <rect.h>
#include <value.H>
#include <buffer.H>
#include <proctable.H>
#include <atom.H>
#include <atomlist.H>
#include <graphic.H>
#include <rm.H>
#include <view.H>
#include <point.h>
#include <list.H>
#include <sbutton.H>
#include <sbuttonv.H>

static class atomlist *  AL_bodyfont;
static class atomlist *  AL_bodyfont_size;
static class atomlist *  AL_label;
static class atomlist *  AL_forecolor;
static class atomlist *  AL_backcolor;
static class atomlist *  AL_style;
static class atom *  A_long;
static class atom *  A_string;
#define InternAtoms ( \
AL_bodyfont = atomlist::StringToAtomlist("bodyfont") ,\
AL_bodyfont_size = atomlist::StringToAtomlist("bodyfont-size") ,\
AL_label = atomlist::StringToAtomlist("label") ,\
AL_forecolor = atomlist::StringToAtomlist("foreground-color") ,\
AL_backcolor = atomlist::StringToAtomlist("background-color") ,\
AL_style = atomlist::StringToAtomlist("style") ,\
A_long = atom::Intern("long") ,\
A_string = atom::Intern("string") )
struct buttonV_rl {
    struct rectangle rect;
    char *string;
    boolean pushed;
    long key,len;
    long width;
};

#define Min(X,Y) ((X) < (Y) ? (X) : (Y))
#define Graphic(A) (((class view *)(A))->drawable)
#define CLOSEBUTTON TRUE
/****************************************************************/
/*		private functions				*/
/****************************************************************/

#define DEFAULTPCT 10

#define MINSIZE 3
/* Rect2 is the inner (Text) region */

#define PROMPTFONT "andysans12b"
#define FONT "andysans"
#define FONTTYPE fontdesc_Bold
#define FONTSIZE 12
#define BUTTONDEPTH 3
#define BUTTONPRESSDEPTH 2
#define TEXTPAD 2
#define buttonV_STRING_END ':'
#define buttonV_STRING_ESCAPE '\\'


ATKdefineRegistry(buttonV, valueview, buttonV::InitializeClass);
static boolean clearrl(struct buttonV_rl  *rl,class buttonV  *self);
static boolean vsetrec(struct buttonV_rl  *rl,class buttonV  *self);
static boolean wsetrec(struct buttonV_rl  *rl,class buttonV  *self);
static void calcRec(class buttonV  * self);
int fourwaysort(struct buttonV_rl  *rl1,struct buttonV_rl  *rl2);
static void DrawButton(class buttonV  * self,struct buttonV_rl  *rl,long  left,long  top,long  width,long  height,boolean  borderonly,boolean  blit);
static boolean enclosed(struct buttonV_rl  *rl,long  x,long  y);
static boolean drl(struct buttonV_rl  *rl,class buttonV  *self);
static boolean findcurrent(struct buttonV_rl  *rl,class buttonV  *self);
static boolean findkey(struct buttonV_rl  *rl,long  i);
static void DrawAllButtons(class buttonV  * self);


static boolean clearrl(struct buttonV_rl  *rl,class buttonV  *self)
{
    if(rl->len > 0) free(rl->string);
    free(rl);
    return TRUE;
}
static boolean vsetrec(struct buttonV_rl  *rl,class buttonV  *self)
{
    if(!self->topdown) self->rtl += -self->rhw;
    rectangle_SetRectSize(&(rl->rect),self->valueview::x + self->offset ,self->rtl,
		 self->bsize, self->rhw);
    if(self->topdown) self->rtl += self->rhw;
    if(--(self->bcnt) == 0){
	self->bcnt = self->count / self->columns;
	self->offset += self->bsize;
	self->rtl = self->topdown ?  self->valueview::y:self->height;
    }
    return TRUE;
}
void buttonV::HandleStyleString(char  *s)
{
    boolean go;
    go = TRUE;
    if(this->mono == -10)
	this->mono = ((this)->DisplayClass() & graphic_Monochrome);

    if(s == NULL) return;
    while(*s != '\0'){
	switch (*s){
	    case 'c':
	    case 'C':
		if(this->mono) go = FALSE;
		else go = TRUE;
		break;
	    case 'M':
	    case 'm':
		if(this->mono) go = TRUE;
		else go = FALSE;
	    case ' ':
	    case '\t':
		break;
	    default:
		if(go && *s <= '9' && *s >= '0'){
		    this->prefs->style = *s - '0';
		    return;
		}
	}
	s++;
    }
}
static boolean wsetrec(struct buttonV_rl  *rl,class buttonV  *self)
{
    rectangle_SetRectSize(&(rl->rect),self->rtl,self->valueview::y + self->offset,
		   self->rhw,self->bsize);
    self->rtl += self->rhw;
    if(--(self->bcnt) == 0){
	self->bcnt = self->count / self->columns;
	self->offset += self->bsize;
	self->rtl =  self->valueview::x;
    }
    return TRUE;
}
static void calcRec(class buttonV  * self)
{
    if(self->columns == 0) return;
    self->offset = 0;
    self->bcnt = self->count / self->columns;
    if(self->vertical){
	self->rtl = self->topdown ? self->valueview::y:self->height;
	self->rhw = (self->height * self->columns )/ self->count;
	self->bsize =  self->width / self->columns;
	(self->listp)->Enumerate((list_efptr)vsetrec,(char *)self);
    }
    else {
	self->rtl =  self->valueview::x;
	self->rhw = (self->width  * self->columns )/ self->count;
	self->bsize = self->height / self->columns;
	(self->listp)->Enumerate((list_efptr)wsetrec,(char *)self);
    }
}
int fourwaysort(struct buttonV_rl  *rl1,struct buttonV_rl  *rl2)
{
   if(rl1->key == 3 && rl2->key == 2) return -1;
    else if (rl2->key == 3 && rl1->key == 2) return 1;
    else return (rl1->key - rl2->key);
}
 void buttonV::CacheSettings()
{
    char tmp[256],*t;
    const char *chr;
    class graphic *my_graphic;
    struct buttonV_rl *rl;
    long i,j;
    long max = 0;
    this->activefont = fontdesc::Create( this->fontname, fontdesc_Plain, this->fontsize );
    my_graphic = (class graphic *)(this)->GetDrawable();
    (this->listp)->Enumerate((list_efptr)clearrl,(char *)this) ;
    (this->listp)->Clear();
    i = 0;
    chr = this->label;
    this->mono = ((this )->DisplayClass() & graphic_Monochrome);
    do{
	j = 0;
	rl = (struct buttonV_rl *)
	  malloc(sizeof(struct buttonV_rl));
	rl->string = (char *)""; /* not freed if len == 0 */
	t = tmp;
	if(chr == NULL || *chr == '\0'){
	    if(i < 4 && this->l[i] != NULL){
		strcpy(tmp,this->l[i]);
		j = strlen(tmp);
		t = tmp + j;
	    }
	}
	else {
	    if(i + 1 == this->fixedcount){
		while(*chr != '\0'){
		    *t++ = *chr++;
		    j++;
		}
	    }
	    else {
		while(*chr != buttonV_STRING_END && *chr != '\0'){
		    if(*chr == buttonV_STRING_ESCAPE && *(chr + 1) == buttonV_STRING_END){
			chr++;
		    }
		    *t++ = *chr++;
		    j++;
		}
	    }
	}
	rl->len = j;
	*t = '\0';
	if(j > 0){
	    rl->string = (char *) malloc(j + 1);
	    strcpy(rl->string,tmp);
	}
	rl->width = (this->activefont)->StringSize(my_graphic, tmp,NULL,NULL);
	if(rl->width > max) max = rl->width;
	rl->pushed = FALSE;
	rl->key = i;
	if(this->fourway) (this->listp)->InsertSorted((char *) rl,(list_greaterfptr)fourwaysort);
	else 
	    (this->listp)->InsertEnd((char *)rl);
	if(this->fixedcount > 0){
	    if((i + 1 >= this->fixedcount) && ((i + 1) % this->columns == 0) ) break;
	}
	else if(chr == NULL || *chr == '\0'){
	    if((i + 1) % this->columns == 0)
		break;
	}
	if(chr != NULL && *chr != '\0')chr++;
    }while (++i < 128);
    this->current = NULL;
    this->count = i + 1;
    this->max = max;
}
void buttonV::DrawButtonText(const char  *text,long  len,struct rectangle  *rect,struct rectangle  *rect2,boolean  pushd)
{
/* assumes '\0' terminated text */
    if(text != NULL && len > 0) 
	sbuttonv::DrawButtonLabel(this, text, rect2, this->prefs,pushd);
}

static void DrawButton(class buttonV  * self,struct buttonV_rl  *rl,long  left,long  top,long  width,long  height,boolean  borderonly,boolean  blit)
{
    struct rectangle Rect,*rect,in;
    const char *text ;
    boolean ped ;
    if(rl == NULL) {
	rect = &Rect;
	rectangle_SetRectSize(rect,left,top,width,height);
	ped =  (self->pushed);
	text = "";
    }
    else{
	rect = &(rl->rect);
	ped = (rl->pushed);
	text = rl->string;
    }
    if(text == NULL ) text = "";
    self->prefs->font = self->activefont;
    if(self->buttontype){
	sbuttonv::DrawRectBorder(self,rect,self->prefs,ped,TRUE,&in);
	(self)->DrawButtonText(text,strlen(text),rect,&in,ped);
    }
    else {
	self->si.lit = ped;
	self->si.sensitive=TRUE;
	self->si.label = text;
	sbuttonv::DrawButton(self,&(self->si),rect); 
    }
}
static boolean enclosed(struct buttonV_rl  *rl,long  x,long  y)
{
    if(x < rl->rect.left ||
	y < rl->rect.top ||
	x >= rl->rect.left + rl->rect.width ||
	y >= rl->rect.top + rl->rect.height)
	return FALSE;
    return TRUE;
}

static boolean drl(struct buttonV_rl  *rl,class buttonV  *self)
{
    DrawButton(self,rl,0,0,0,0,FALSE,FALSE);
    return TRUE;
}
static boolean findcurrent(struct buttonV_rl  *rl,class buttonV  *self)
{
    return !enclosed(rl,self->x,self->y);
}
static boolean findkey(struct buttonV_rl  *rl,long  i)
{
    return !(i == rl->key);
}
static void DrawAllButtons(class buttonV  * self)
{
    (self->listp)->Enumerate((list_efptr)drl,(char *)self);
}

/****************************************************************/
/*		class procedures 				*/
/****************************************************************/

boolean buttonV::InitializeClass()
{
  InternAtoms;
  return TRUE;
}

/****************************************************************/
/*		instance methods				*/
/****************************************************************/
buttonV::buttonV()
{
	ATKinit;

    this->label = NULL;
    this->fontname = NULL;
    this->fontsize = 0;
    this->buttontype = FALSE;
    this->pushed = FALSE;
    this->listp = new list;
    this->current = NULL;
    this->fixedcount = 1;
    this->fixedcolumns = 1;
    this->l[0] = this->l[1] = this->l[2] = this->l[3] = NULL;
    this->columns = 1;
    this->vertical = TRUE;
    this->topdown = FALSE;
    this->fourway = FALSE;
    this->si.prefs = sbutton::GetNewPrefs("adew");
    this->prefs = this->si.prefs;
    if(this->prefs == NULL) THROWONFAILURE( FALSE);
    sbutton::InitPrefs(this->prefs,"adew");
    this->si.rock = 0;
    this->si.trigger = NULL;
    this->si.sensitive=TRUE;
    this->mono = -10;
    THROWONFAILURE( TRUE);
}

buttonV::~buttonV()
{
	ATKinit;

    sbutton::FreePrefs(this->prefs); 
}

void buttonV::LookupParameters()
     {
  const char * fontname;
  long fontsize;
  static struct resourceList parameters[] = {
	{ AL_label, A_string }, /* 0 */
	{ AL_bodyfont, A_string }, /* 1 */
	{ AL_bodyfont_size, A_long }, /* 2 */
	{ AL_forecolor, A_string }, /* 3 */
	{ AL_backcolor, A_string }, /* 4 */
	{ AL_style, A_string }, /* 5 */
	{ NULL, NULL }
  };
  (this)->GetManyParameters( parameters, NULL, NULL);

  if (parameters[0].found)
      this->label = (char *)parameters[0].data;
  else
      this->label = "";

  if (parameters[1].found)
    fontname = (char *)parameters[1].data;
  else
    fontname = "andytype";

  if (parameters[2].found)
    fontsize = parameters[2].data;
  else
      fontsize = 10;
  if(this->mono == -10)
      this->mono = ((this)->DisplayClass() & graphic_Monochrome);

 if(!this->mono){
      if (parameters[3].found)
	  this->prefs->colors[sbutton_FOREGROUND] = (char *) parameters[3].data;

      if (parameters[4].found)
	  this->prefs->colors[sbutton_BACKGROUND] = (char *) parameters[4].data;
  }
 
 if (parameters[5].found)
      (this)->HandleStyleString( (char *) parameters[5].data);


  this->fontsize = fontsize;
  this->fontname = fontname;
  (this)->CacheSettings();

}


void buttonV::DrawFromScratch(long  x,long  y,long  width,long  height)
{
    if (width > 0 && height > 0)
    {
	calcRec(this);
	DrawAllButtons(this);
	(this )->DrawNewValue( );
    }
}


void buttonV::DrawDehighlight()
{
}

void buttonV::DrawHighlight()
{

}


void buttonV::DrawNewValue( )
     {
    long i= ((this)->Value())->GetValue();
    struct buttonV_rl *rl = this->current;
    if(this->count > 1){
	if(rl == NULL ||  i  != rl->key){
	    if(rl){  
		rl->pushed = FALSE;
		DrawButton(this,rl,0,0,0,0,FALSE,TRUE);
	    }
	    rl = (struct buttonV_rl *) (this->listp)->Enumerate((list_efptr)findkey,(char *)i);
	    if(rl){
		rl->pushed = TRUE;
		DrawButton(this,rl,0,0,0,0,FALSE,TRUE);
	    }
	    this->current = rl;
	}
    }
    this->valueset = TRUE;
}

class valueview * buttonV::DoHit( enum view_MouseAction  type,long  x,long  y,long  hits )
{
    struct buttonV_rl *rl;
    long v;
    rl = this->current;
    if(this->count == 1){
	class valueview *vself = (class valueview *) this;
	switch(type){
	    case view_RightUp:
	    case view_LeftUp:
		if(rl){
		    rl->pushed = FALSE;
		    DrawButton(this,rl,0,0,0,0,FALSE,TRUE); 
		}
		((this)->Value())->SetValue(((this)->Value())->GetValue() + 1);
		this->current = NULL;
		return this;
	    case view_LeftDown:
	    case view_RightDown:
		break;
	    case view_LeftMovement:
	    case view_RightMovement:
		if(rl && !vself->mouseIsOnTarget){
		    rl->pushed = FALSE;
		    DrawButton(this,rl,0,0,0,0,FALSE,TRUE); 
		    this->current = NULL;
		}
		return this;
	    default:
	        break;
	}
    }
    switch(type){
	case view_RightUp:
	case view_LeftUp:
	    if(rl) v = rl->key;
	    else v = -1;
	    if(((this)->Value())->GetValue() != v)
		((this)->Value())->SetValue(v);
	    break;
	case view_LeftMovement:
	case view_RightMovement:
	case view_LeftDown:
	case view_RightDown:
	    if(rl == NULL || !enclosed(rl,x,y)){
		this->x = x;
		this->y = y;
		if(rl){  
		    rl->pushed = FALSE;
		    DrawButton(this,rl,0,0,0,0,FALSE,TRUE);
		}
		this->valueset = FALSE;
		rl = (struct buttonV_rl *) (this->listp)->Enumerate((list_efptr)findcurrent,(char *)this);
		if(rl == NULL){
		    this->current = NULL;
		    (this )->DrawNewValue( );
		    break;
		}
		if(rl){
		    rl->pushed = TRUE;
		    DrawButton(this,rl,0,0,0,0,FALSE,TRUE);
		}
		this->current = rl;
	    }
	    break;
	default:
	    break;
    } 
    return this;
}

class view * buttonV::Hit(enum view_MouseAction  type, long  x , long  y , long  numberOfClicks)
                    {/* should probably just restore this functionality to valueview,
	with a way to optionly set it */
	 short sendEvent = FALSE;
	 class valueview *vself = (class valueview *) this;
	 if(((class valueview *) this)->HasInputFocus == FALSE)
	     (this)->WantInputFocus(this);

	 if (vself->active)
	 {
	     switch (type)
	     {
		 case view_NoMouseEvent:
		     sendEvent = FALSE;
		     break;
		 case view_LeftDown:
		     vself->mouseState = valueview_LeftIsDown;
		     (this)->Highlight();
		     sendEvent = TRUE;
		     this->movedoff = FALSE;
		     break;
		 case view_RightDown:
		     vself->mouseState = valueview_RightIsDown;
		     (this)->Highlight();
		     sendEvent = TRUE;
		     this->movedoff = FALSE;
		     break;
		 case view_LeftUp:
		 case view_RightUp:
		     if(this->movedoff) sendEvent = FALSE;
		     else if (vself->mouseIsOnTarget)
		     {
			 (this)->Dehighlight();
			 sendEvent = TRUE;
		     }
		     else sendEvent = TRUE;
		     break;
		 case view_LeftMovement:
		 case view_RightMovement:
		     if(this->movedoff) sendEvent = FALSE;
		     else  if((this)->OnTarget(x,y))
		     {
			 (this)->Highlight();
			 sendEvent = TRUE;
		     }
		     else
		     {
			 (this)->Dehighlight();
			/* self->movedoff = TRUE; */
			 if(this->valueset) sendEvent = FALSE;
			 else 
			     sendEvent = TRUE;

		     }
		     break;
		 default:
		     break;
	     }
	 }
	 else
	     sendEvent = FALSE;
	 if (sendEvent){
	     return (class view *) (this)->DoHit( type, x, y, numberOfClicks);
	 }
	 else
	     return (class view *) this;
     }
