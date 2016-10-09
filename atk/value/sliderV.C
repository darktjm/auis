/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1989 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

#include <andrewos.h>
ATK_IMPL("sliderV.H")
#include <sliderV.H>
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
#include <sbutton.H>
#include <sbuttonv.H>

#include <ctype.h>

static class atomlist *  AL_bodyfont;
static class atomlist *  AL_bodyfont_size;
static class atomlist *  AL_label;
static class atomlist *  AL_max_value;
static class atomlist *  AL_min_value;
static class atomlist *  AL_forecolor;
static class atomlist *  AL_backcolor;
static class atomlist *  AL_readonly;
static class atomlist *  AL_immediate;
static class atomlist *  AL_style;

static const class atom *  A_long;
static const class atom *  A_string;

#define InternAtoms ( \
   AL_bodyfont = atomlist::StringToAtomlist("bodyfont") ,\
   AL_bodyfont_size = atomlist::StringToAtomlist("bodyfont-size") ,\
   AL_label = atomlist::StringToAtomlist("label") ,\
   AL_readonly = atomlist::StringToAtomlist("read-only") ,\
   AL_immediate = atomlist::StringToAtomlist("immediate-update") ,\
   AL_max_value = atomlist::StringToAtomlist("max_value") ,\
   AL_min_value = atomlist::StringToAtomlist("min_value") ,\
   AL_forecolor = atomlist::StringToAtomlist("shade-color") ,\
   AL_backcolor = atomlist::StringToAtomlist("color") ,\
   AL_style = atomlist::StringToAtomlist("style") ,\
   A_long = atom::Intern("long") ,\
   A_string = atom::Intern("string") )

#define rectangle_TempSet(X,Y,W,H,R) ((R)->top = (Y), (R)->left = (X), \
				      (R)->height = (H), (R)->width = (W), (R))

#define Min(X,Y) ((X) < (Y) ? (X) : (Y))
#define FUDGE 2
#define FUDGE2 4
#define SLIDERWID 20
#define STYLEOPTIONSUPPORTED 1

ATKdefineRegistry(sliderV, valueview, sliderV::InitializeClass);
static void sliderV_HandleStyleString(class sliderV  *self,char  *s);
static void CarveFonts(class sliderV  * self);
static void getsizes(class sliderV  * self);
#ifdef DRAWOUTSIDELINES
void db(class sliderV  * self,struct rectangle  *foo,struct rectangle  *fo);
static void DrawButton(class sliderV  * self,long  x,long  y,long  width,long  height,MISSING_ARGUMENT_TYPE pct,boolean  drawborder);
#endif /* DRAWOUTSIDELINES */
static void DrawLabel(class sliderV  * self);
static void DrawValue(class sliderV  * self);


static void sliderV_HandleStyleString(class sliderV  *self,char  *s)
{
    boolean go;
    go = TRUE;
    if(self->mono == -10)
	self->mono = ((self)->DisplayClass() & graphic::Monochrome);

    if(s == NULL) return;
    while(*s != '\0'){
	switch (*s){
	    case 'c':
	    case 'C':
		if(self->mono) go = FALSE;
		else go = TRUE;
		break;
	    case 'M':
	    case 'm':
		if(self->mono) go = TRUE;
		else go = FALSE;
	    case ' ':
	    case '\t':
		break;
	    default:
		if(go && *s <= '9' && *s >= '0'){
		    self->prefs->style = *s - '0';
		    return;
		}
	}
	s++;
    }
}

/****************************************************************/
/*		private functions				*/
/****************************************************************/

static void CarveFonts(class sliderV  * self)
{
    self->normalfont = fontdesc::Create( self->fontname, fontdesc_Plain, self->fontsize );
    self->boldfont   = fontdesc::Create( self->fontname, fontdesc_Bold,  self->fontsize );
    self->valuefont = fontdesc::Create( "values", fontdesc_Plain, 25);
    self->activefont = self->mouseIsOnTarget ? self->boldfont : self->normalfont;
}  
#define MAXWID 36
#define ADDTOP 3
#define SUBHEIGHT 7
static void getsizes(class sliderV  * self)
{
    struct FontSummary *fs;
    long labelwidth, labelheight, valheight,junk,wp,ww,w ,exsp,exvh;
    fs = (self->boldfont)->FontSummary((self)->GetDrawable());
#ifdef DRAWOUTSIDELINES
    exsp = FUDGE2 + FUDGE2;
    exvh = FUDGE + 4;
#else
    exsp = 0;
    exvh = FUDGE + 4;
#endif
/*
    sprintf(buf,"%ld",self->maxval);
    fontdesc_StringSize(self->boldfont,sliderV_GetDrawable(self), buf,&(valwidth),&(junk));
    sprintf(buf,"%ld",self->minval);
    fontdesc_StringSize(self->boldfont,sliderV_GetDrawable(self), buf,&(valwidth1),&(junk));
    if(valwidth1 > valwidth) valwidth = valwidth1;
*/
    valheight = ( fs->newlineHeight == 0) ? fs->maxHeight : fs->newlineHeight;
    if(self->label){
	(self->boldfont)->StringSize((self)->GetDrawable(), self->label,&(labelwidth),&(junk));
	labelheight = valheight;
    }
    else{
	labelheight = labelwidth = 0;
    }
    ww = self->width - FUDGE2 - FUDGE2 - self->x - self->x;
    wp = self->x + FUDGE2;
    if(ww > MAXWID){
	ww = MAXWID;
	wp = self->x + (self->width - MAXWID) / 2;
    }
    rectangle_SetRectSize(&self->wheelrec,wp,valheight + FUDGE + self->y + ADDTOP,ww, self->height - labelheight - valheight - FUDGE2 - self->y - self->y - SUBHEIGHT);
    rectangle_SetRectSize(&self->labelrec,((self)->Width() - labelwidth) / 2,self->height - labelheight,labelwidth + FUDGE2,labelheight);
    rectangle_SetRectSize(&self->valrec,self->x + 6, self->y + exvh ,self->width -FUDGE - 12 ,valheight);
    w = self->x + (self->width / 2);
    rectangle_SetRectSize(&self->inwheelrec,w - (self->sliderwidth / 2), rectangle_Top(&self->wheelrec) + 1,self->sliderwidth, rectangle_Height(&self->wheelrec) - exsp);

}
#ifdef DRAWOUTSIDELINES

void db(class sliderV  * self,struct rectangle  *foo,struct rectangle  *fo)
{
    (self)->DrawRect(foo);   
    (self)->MoveTo(  fo->left, fo->top );
    (self)->DrawLineTo(  foo->left, foo->top);
    (self)->MoveTo(  fo->left + fo->width, fo->top );
    (self)->DrawLineTo(  foo->left + foo->width, foo->top);
    (self)->MoveTo(  fo->left , fo->top + fo->height);
    (self)->DrawLineTo(  foo->left, foo->top + foo->height);
    (self)->MoveTo(  fo->left + fo->width, fo->top + fo->height);
    (self)->DrawLineTo(  foo->left + foo->width, foo->top + foo->height);

}
static void DrawButton(class sliderV  * self,long  x,long  y,long  width,long  height,MISSING_ARGUMENT_TYPE pct,boolean  drawborder)
               {
  struct rectangle foo,fo;
  long vcut,wcut;
  if(!drawborder && pct > 0)(self)->SetClippingRect(  rectangle_TempSet(x,y,width + 2,height + 1,&fo)  ); 
  rectangle_TempSet(x,y,width,height,&fo);
 /* fo.width--; */  fo.height--;

 if(pct > 0){
  vcut = height /pct ;
  wcut =  width /pct;
  rectangle_TempSet(x + wcut ,y + 1,width - wcut - wcut,height - 3 - vcut - vcut - vcut,&foo);
 }
  else{
      vcut = pct; wcut = pct;
      rectangle_TempSet(x + wcut ,y  - 1,width - wcut - wcut,height - 5 - vcut - vcut - vcut,&foo);
  }
  if(pct <= 0)
      (self)->EraseRect(&foo);
  if(drawborder) (self)->DrawRect(&fo);   
  db(self,&foo,&fo);
}
#endif /* DRAWOUTSIDELINES */

static void DrawLabel(class sliderV  * self)
{
    if(self->label){	
	(self)->SetTransferMode(  graphic::COPY);
	(self)->EraseRect( &self->labelrec);

	(self)->SetTransferMode(  graphic::COPY );
	(self)->MoveTo( self->x + ( self->width / 2),self->y + self->height);
	(self)->SetFont(  self->activefont );
	(self)->DrawString (  self->label,
				   graphic::BETWEENLEFTANDRIGHT | graphic::ATBOTTOM);
    }

}
const char *sliderV::GetValueString()
{
    sprintf(this->buf,"%ld",(this)->GetTmpVal());
    return this->buf;
}
static void DrawValue(class sliderV  * self)
{
    const char *buf;   
    buf = (self)->GetValueString();
    (self)->SetTransferMode(  graphic::COPY);
    (self)->EraseRect( &self->valrec);

    (self)->SetTransferMode(  graphic::COPY );
    (self)->MoveTo(  self->x + (self->width / 2), self->y + FUDGE + 4);
/*
    sliderV_MoveTo( self, self->x + (self->width / 2),self->valrec.height); */
    (self)->SetFont(  self->activefont );
    (self)->DrawString (  buf,
				graphic::BETWEENLEFTANDRIGHT | graphic::ATTOP);

}
#define HGH 3
#define SPACE 6

 void sliderV::Drawslider(boolean  fullupdate,struct rectangle  *rr)
{
    int start,height,st;
    struct rectangle interior,ti;
    struct sbuttonv_view_info vi;
    boolean pushed ;

    st = this->prefs->style;
    sbuttonv::SaveViewState( this, &vi);
   if(st == 4 || st == 2){
	pushed = FALSE;
	if(!fullupdate)
	    (this)->SetClippingRect(&(this->lastdrawn));
	sbuttonv::DrawBorder(this, rr->left,rr->top,
			    rr->width, rr->height,
			    this->prefs,!pushed,TRUE,&interior);
	if(!fullupdate)
	    (this)->ClearClippingRect();
	height =  ((this->tmpval - this->minval )* (rectangle_Height(&interior))) / (this->maxval - this->minval);
	height += (((interior.height - height) * interior.width) / interior.height);
	start = rectangle_Top(&interior) + rectangle_Height(&interior) - height;
	sbuttonv::DrawBorder(this, interior.left, start,
			    interior.width,interior.width, this->prefs,pushed,TRUE,&ti);
	rectangle_SetRectSize(&(this->lastdrawn),interior.left, start,
			      interior.width,interior.width + 4);
#ifdef DRAW_NOTCH
	sbuttonv::DrawBorder(this, ti.left,start + (interior.width / 2) -1,
			    ti.width,2, this->prefs,!pushed,TRUE,NULL);
#endif
    }
    else {
	(this)->SetTransferMode( graphic::COPY);
	if(!this->mono){
	    if(this->prefs->colors[sbutton_FOREGROUND]) 
		(this)->SetForegroundColor( this->prefs->colors[sbutton_FOREGROUND], 0, 0, 0);
	    if(this->prefs->colors[sbutton_BACKGROUND]) 
		(this)->SetBackgroundColor( this->prefs->colors[sbutton_BACKGROUND], 0, 0, 0);
	}
	if(!fullupdate)
	    (this)->SetClippingRect(&(this->lastdrawn));
	(this)->EraseRect(rr);
	(this)->DrawRect(rr);
	if(!fullupdate)
	    (this)->ClearClippingRect();
	interior.top = rr->top + 2; interior.left = rr->left + 2;
	interior.width = rr->width -4; interior.height = rr->height - 4;
	height =  ((this->tmpval - this->minval )* (rectangle_Height(&interior))) / (this->maxval - this->minval);
	height += (((interior.height - height) * interior.width) / interior.height);
	start = rectangle_Top(&interior) + rectangle_Height(&interior) - height;
	rectangle_SetRectSize(&(this->lastdrawn),interior.left, start,
			      interior.width,interior.width);
	(this)->DrawRect( &(this->lastdrawn));
	if(st == 0 || st == 3)
	    (this)->FillRect( &(this->lastdrawn),(this)->BlackPattern());
	else
	    (this)->DrawRectSize(interior.left + 1, start + 1,
			      interior.width - 2,interior.width -2 );
	this->lastdrawn.width += 4;
	this->lastdrawn.height += 3;
    }
   sbuttonv::RestoreViewState( this, &vi);

}


/****************************************************************/
/*		class procedures 				*/
/****************************************************************/




boolean sliderV::InitializeClass()
{
    InternAtoms;
    return TRUE;
}




#define BADVAL -22222
/****************************************************************/
/*		instance methods				*/
/****************************************************************/
sliderV::sliderV()
{
	ATKinit;

    this->label = NULL;
    this->fontname = NULL;
    this->fontsize = 0;
    this->maxval = 100;
    this->minval = 0;
    this->increment = 1;
    this->tmpval = BADVAL;
    this->lasty = 0;
    this->rv = 1000000;
    this->granular = this->gran =  0;
    this->doclip = FALSE;
    this->prefs = sbutton::GetNewPrefs("adew");
    if(this->prefs == NULL) THROWONFAILURE( FALSE);
    sbutton::InitPrefs(this->prefs,"adew");
    this->sliderwidth = SLIDERWID;
    this->immediatedefault = FALSE;
    this->readonlydefault = FALSE;
    this->mono = -10;
    THROWONFAILURE( TRUE);
}

sliderV::~sliderV()
{
	ATKinit;
   
    sbutton::FreePrefs(this->prefs);
}
void sliderV::LookupParameters()
{
    const char * fontname;
    long fontsize,diff;
    static struct resourceList parameters[] = {
	{ AL_label, A_string }, /* 0 */
	{ AL_bodyfont, A_string }, /* 1 */
	{ AL_bodyfont_size, A_long }, /* 2 */
	{ AL_max_value, A_long }, /* 3 */
	{ AL_min_value, A_long }, /* 4 */
	{ AL_forecolor, A_string }, /* 5 */
	{ AL_backcolor, A_string }, /* 6 */
	{ AL_readonly, A_string }, /* 7 */
	{ AL_immediate, A_string }, /* 8 */
#ifdef STYLEOPTIONSUPPORTED
	{ AL_style, A_string }, /* 9 */
#endif
	{ NULL, NULL }
    };
    if(this->mono == -10)
	this->mono = ((this)->DisplayClass() & graphic::Monochrome);

    (this)->GetManyParameters( parameters, NULL, NULL);

    if (parameters[0].found)
	this->label = (char *)parameters[0].data;
    else
	this->label = NULL;

    if (parameters[1].found)
	fontname = (char *)parameters[1].data;
    else
	fontname = "andytype";

    if (parameters[2].found)
	fontsize = parameters[2].data;
    else
	fontsize = 10;

    if(parameters[3].found)
	this->maxval = parameters[3].data;
    else
	this->maxval = 100;

    if(parameters[4].found)
	this->minval = parameters[4].data;
    else
	this->minval = 0;

    diff = this->maxval - this->minval;
    if(diff == 0) (this->maxval)++;
    if(diff < 20) this->granular = 6;
    else if(diff < 50) this->granular = 4;
    else if(diff < 100) this->granular = 2;

    else this->granular = 0;

    if (parameters[5].found)
	this->prefs->colors[sbutton_FOREGROUND] = (char *) parameters[5].data;

    if (parameters[6].found)
	this->prefs->colors[sbutton_BACKGROUND] = (char *) parameters[6].data;

    this->readonly = this->readonlydefault;
    if (parameters[7].found){
	char *foo;
	foo = (char *) parameters[7].data;
	while(*foo && isspace(*foo)) foo++;
	if(*foo != '\0'){
	    if(*foo == 't' || *foo == 'T' || *foo == 'y' || *foo == 'Y')
		this->readonly = TRUE;
	    else this->readonly = FALSE;
	}
    }

    this->immediate = this->immediatedefault;
    if (parameters[8].found){
	char *foo;
	foo = (char *) parameters[8].data;
	while(*foo && isspace(*foo)) foo++;
	if(*foo != '\0'){
	    if(*foo == 't' || *foo == 'T' || *foo == 'y' || *foo == 'Y')
		this->immediate = TRUE;
	    else this->immediate = FALSE;
	}
    }
#ifdef STYLEOPTIONSUPPORTED
    if (parameters[9].found)
	sliderV_HandleStyleString(this,(char *) parameters[9].data);
#else
    this->prefs->style = 4;
#endif
    if(this->prefs->style == -1) this->prefs->style = 4;
    if (fontsize != this->fontsize || fontname != this->fontname)
    {
	this->fontsize = fontsize;
	this->fontname = fontname;
	CarveFonts(this);
    }
}


void sliderV::DrawFromScratch(long  x,long  y,long  width,long  height)
{
    this->x = x;
    this->y = y;
    this->width = width;
    this->height = height;
    this->doclip = FALSE;

    if (width > 0 && height > 0)
    {
	class value *w = (this)->Value();
	getsizes(this);
	if(this->tmpval == BADVAL){
	    this->tmpval = (w)->GetValue();
	    if(this->tmpval < this->minval || 
	       this->tmpval > this->maxval){
		(w)->SetValue(this->minval);
		this->tmpval = this->minval;
	    }
	}
	this->activefont = this->mouseIsOnTarget ?
	  this->boldfont : this->normalfont;
#ifdef DRAWOUTSIDELINES
	DrawButton(this,this->x + 5,this->y + 3,this->width - 10,this->wheelrec.top + this->wheelrec.height - 3,-3,TRUE);
#endif 
	DrawValue(this);	
	(this)->Drawslider(TRUE,&this->inwheelrec);
	if (this->label != NULL)
	    DrawLabel(this);
    }
}


void sliderV::DrawDehighlight()
{
    class value *w = (this)->Value();
    this->activefont = this->normalfont;
    this->tmpval = (w)->GetValue();
    DrawLabel( this );
    DrawValue(this);	
    (this)->Drawslider(TRUE,&this->inwheelrec);
}

void sliderV::DrawHighlight()
{
    class value *w = (this)->Value();
    this->activefont = this->boldfont;
    DrawLabel( this );
    this->tmpval = (w)->GetValue();
    DrawValue(this);
    (this)->Drawslider(TRUE,&this->inwheelrec);
}


void sliderV::DrawNewValue( )
{
    class value *w = (this)->Value();
    if(this->tmpval != (w)->GetValue()){
	this->tmpval = (w)->GetValue();
	DrawValue(this);
	(this)->Drawslider(TRUE,&this->inwheelrec);
    }
}



class valueview * sliderV::DoHit( enum view_MouseAction  type,long  x,long  y,long  hits )
{
    class value *tt = (this)->Value();
    long myval;
    static int moved;
    if(this->readonly) return this;
    switch(type){
	case view_LeftDown:
	case view_RightDown:
	    this->tmpval = (tt)->GetValue();
	    this->lasty = y;
	    moved = 0;
	    break;
	case view_LeftMovement	:
	case view_RightMovement:
	    moved++;
	    if(this->granular){
		myval = this->tmpval;
		this->gran += (this->lasty - y);
		while(this->gran > this->granular){
		    myval++;
		    this->gran -= this->granular;
		}
		while(this->gran < -this->granular){
		    myval--;
		    this->gran += this->granular;
		}
	    }
	    else 
	    myval = this->tmpval + (( this->lasty - y) /* * self->increment */ );
	    
	    if(myval > this->maxval)  myval = this->maxval;
	    else if(myval < this->minval) myval = this->minval;
	    if(myval != this->tmpval){
		this->tmpval = myval;
		DrawValue(this);
		(this)->Drawslider(FALSE,&this->inwheelrec);
	    }

	    this->lasty = y;
	    if(this->immediate)
		(tt)->SetValue(this->tmpval);

	    break;
	case view_LeftUp:
	case view_RightUp:
	    if(moved == 0){
		myval = (type == view_RightUp)? this->tmpval - this->increment : this->tmpval + this->increment ;
		if(myval <= this->maxval &&  myval >= this->minval){
		    this->tmpval = myval;
		    DrawValue(this);
		    this->lasty = y;
		    DrawValue(this);
		    (this)->Drawslider(FALSE,&this->inwheelrec);
		}
	    }
	    tt->string = (this)->GetValueString();
	    (tt)->SetValue(this->tmpval);
	    break;
	default:
	    break;
    }  

    return this;
}





