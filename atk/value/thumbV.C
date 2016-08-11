/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1989 - All Rights Reserved      *
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
static char rcsid[]="$Header: /afs/cs.cmu.edu/project/atk-src-C++/atk/value/RCS/thumbV.C,v 1.3 1994/11/30 20:42:06 rr2b Stab74 $";
#endif


 


#include <andrewos.h>
ATK_IMPL("thumbV.H")
#include <thumbV.H>
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
#include <ctype.h>
static class atomlist *  AL_bodyfont;
static class atomlist *  AL_bodyfont_size;
static class atomlist *  AL_label;
static class atomlist *AL_max_value;
static class atomlist *AL_min_value;
static class atomlist *AL_increment;
static class atomlist *  AL_forecolor;
static class atomlist *  AL_backcolor;
static class atomlist *  AL_immediate;

static class atom *  A_long;
static class atom *  A_string;

#define InternAtoms ( \
   AL_bodyfont = atomlist::StringToAtomlist("bodyfont") ,\
   AL_bodyfont_size = atomlist::StringToAtomlist("bodyfont-size") ,\
   AL_label = atomlist::StringToAtomlist("label") ,\
   AL_max_value = atomlist::StringToAtomlist("max_value") ,\
   AL_immediate = atomlist::StringToAtomlist("immediate-update") ,\
   AL_min_value = atomlist::StringToAtomlist("min_value") ,\
   AL_increment = atomlist::StringToAtomlist("increment") ,\
   AL_forecolor = atomlist::StringToAtomlist("foreground-color")  ,\
   AL_backcolor = atomlist::StringToAtomlist("background-color") ,\
   A_long = atom::Intern("long") ,\
   A_string = atom::Intern("string") )


#define Min(X,Y) ((X) < (Y) ? (X) : (Y))
#define FUDGE 2
#define FUDGE2 4


/****************************************************************/
/*		private functions				*/
/****************************************************************/


ATKdefineRegistry(thumbV, valueview, thumbV::InitializeClass);
#ifndef NORCSID
#endif
static void CarveFonts(class thumbV  * self);
static void getsizes(class thumbV  * self);
static void DrawLabel(class thumbV  * self);
static void DrawValue(class thumbV  * self);
static void DrawKnurl(class thumbV  * self);
static void DrawThumbwheel(class thumbV  * self,boolean DoAll);


static void CarveFonts(class thumbV  * self)
{
    self->normalfont = fontdesc::Create( self->fontname, fontdesc_Plain, self->fontsize );
    self->boldfont   = fontdesc::Create( self->fontname, fontdesc_Bold,  self->fontsize );
    self->valuefont = fontdesc::Create( "values", fontdesc_Plain, 25);
    self->activefont = self->mouseIsOnTarget ? self->boldfont : self->normalfont;
}  
#define MAXWID 36
static void getsizes(class thumbV  * self)
{
    struct FontSummary *fs;
    long labelwidth, labelheight, valheight,junk,wp,ww;
    fs = (self->boldfont)->FontSummary((self)->GetDrawable());
/*
    sprintf(buf,"%ld",self->maxval);
    fontdesc_StringSize(self->boldfont,thumbV_GetDrawable(self), buf,&(valwidth),&(junk));
    sprintf(buf,"%ld",self->minval);
    fontdesc_StringSize(self->boldfont,thumbV_GetDrawable(self), buf,&(valwidth1),&(junk));
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
    rectangle_SetRectSize(&self->wheelrec,wp,valheight + FUDGE + self->y ,ww, self->height - labelheight - valheight - FUDGE2 - self->y - self->y );
    rectangle_SetRectSize(&self->labelrec,((self)->Width() - labelwidth) / 2,self->height - labelheight,labelwidth + FUDGE2,labelheight);
    rectangle_SetRectSize(&self->valrec,self->x , self->y + FUDGE ,self->width ,valheight);
    rectangle_SetRectSize(&self->inwheelrec,rectangle_Left(&self->wheelrec) + 1, rectangle_Top(&self->wheelrec) + 1,rectangle_Width(&self->wheelrec) - 2, rectangle_Height(&self->wheelrec) - 2);
}


static void DrawLabel(class thumbV  * self)
{
    if(self->label){	
	(self)->SetTransferMode(  graphic_COPY);
	(self)->EraseRect( &self->labelrec);

	(self)->SetTransferMode(  graphic_BLACK );
	(self)->MoveTo( self->x + ( self->width / 2),self->y + self->height);
	(self)->SetFont(  self->activefont );
	(self)->DrawString (  self->label,
				   graphic_BETWEENLEFTANDRIGHT | graphic_ATBOTTOM);
    }

}
char *thumbV::GetValueString()
{
    sprintf(this->buf,"%ld",(this)->GetTmpVal());
    return this->buf;
}
static void DrawValue(class thumbV  * self)
{
    char *buf;   
    buf = (self)->GetValueString();
    (self)->SetTransferMode(  graphic_COPY);
    (self)->EraseRect( &self->valrec);

    (self)->SetTransferMode(  graphic_BLACK );
    (self)->MoveTo(  self->x + self->width / 2, self->y + FUDGE);
    (self)->SetFont(  self->activefont );
    (self)->DrawString (  buf,
				graphic_BETWEENLEFTANDRIGHT | graphic_ATTOP);

}
#define HGH 3
#define SPACE 6
static void DrawKnurl(class thumbV  * self)
{
#ifdef USELINES
    long y,x1,x2,end,nl, hn,change,inc,minx,maxx;
    inc =  (self->rv % SPACE);
    (self)->SetTransferMode(  graphic_INVERT );
    minx =  10000;
    maxx = 0 ;
    end = rectangle_Bottom(&self->inwheelrec) - 1;
    x1 = rectangle_Left(&self->inwheelrec) + FUDGE;
    x2 = rectangle_Width(&self->inwheelrec) - FUDGE2;
    y = rectangle_Top(&self->inwheelrec) + inc + 1;
    nl = (end - y) / SPACE + 1;
    hn = rectangle_Height(&self->wheelrec) / 7 ;
    if(hn < 1) hn = 1;
    nl = hn;
    change = 3;
    for(; y < end ; y += SPACE){
	if((hn -= SPACE) <0 ){ 
	    change--;
	    if(hn + nl < 0 ) change--;
	    hn = nl;
	}
	x1 += change;
	(self)->MoveTo(x1,y);
	(self)->DrawLine(x2,0);
#if 0
	if( inc == count++ % SPACE){
	    (self)->MoveTo(x1,y + 1);
	    (self)->DrawLine(x2,0);
	}
#endif /* 0 */
	if(minx > x1) minx = x1;
	if(maxx < x2 + x1) maxx = x2 + x1;
    }
    if(minx <  rectangle_Left(&self->wheelrec)) 
	rectangle_Left(&self->wheelrec) = minx;
    if(maxx > rectangle_Right(&self->wheelrec)) 
	rectangle_Width(&self->wheelrec) = maxx -  rectangle_Left(&self->wheelrec) + 3;
#else /* USELINES */
    char ch;
    (self)->MoveTo((self->width - self->x)/ 2, (self->height - self->y)/ 2);
    (self)->SetFont(self->valuefont);
    ch =  (self->rv % 7)  + 'O';
    (self)->DrawText(&ch,1, 0 );
#endif /* USELINES */

}

static void DrawThumbwheel(class thumbV  * self,boolean DoAll)
{
    (self)->SetTransferMode(  graphic_COPY );
#if 0
    if(DoAll){
	(self)->EraseRect( &self->wheelrec);
	(self)->SetTransferMode(  graphic_BLACK );
	/*	thumbV_DrawRect( self,&self->wheelrec); 
	 thumbV_MoveTo(self,rectangle_Left(&self->wheelrec),rectangle_Top(&self->wheelrec));
	 thumbV_DrawLine(self,0,rectangle_Height(&self->wheelrec));
	 */
    }
    else {
    }
#endif /* 0 */
    (self)->EraseRect( &self->wheelrec);
    DrawKnurl(self);
}



/****************************************************************/
/*		class procedures 				*/
/****************************************************************/




boolean thumbV::InitializeClass()
{
    InternAtoms;
    return TRUE;
}




#define BADVAL -22222
/****************************************************************/
/*		instance methods				*/
/****************************************************************/
thumbV::thumbV()
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
    this->immediatedefault = FALSE;
    this->foreground = this->background = NULL;
    THROWONFAILURE( TRUE);
}


void thumbV::LookupParameters()
{
    char * fontname;
    long fontsize,diff;
    struct resourceList parameters[10];

    parameters[0].name = AL_label;
    parameters[0].type = A_string;
    parameters[1].name = AL_bodyfont;
    parameters[1].type = A_string;
    parameters[2].name = AL_bodyfont_size;
    parameters[2].type = A_long;
    parameters[3].name = AL_max_value;
    parameters[3].type = A_long;
    parameters[4].name = AL_min_value;
    parameters[4].type = A_long;
    parameters[5].name = AL_increment;
    parameters[5].type = A_long;
    parameters[6].name = AL_forecolor;
    parameters[6].type = A_string;
    parameters[7].name = AL_backcolor;
    parameters[7].type = A_string;
    parameters[8].name = AL_immediate;
    parameters[8].type = A_string;
    parameters[9].name = NULL;
    parameters[9].type = NULL;

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

    if(parameters[5].found)
	this->increment = parameters[5].data;
    else
	this->increment = 1;

    if (parameters[6].found)
	this->foreground = (char *) parameters[6].data;

    if (parameters[7].found)
	this->background = (char *) parameters[7].data;
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


    diff = this->maxval - this->minval;

    if(diff < 20) this->granular = 6;
    else if(diff < 50) this->granular = 4;
    else if(diff < 100) this->granular = 2;

    else this->granular = 0;

    if (fontsize != this->fontsize || fontname != this->fontname)
    {
	this->fontsize = fontsize;
	this->fontname = fontname;
	CarveFonts(this);
    }
}


void thumbV::DrawFromScratch(long  x,long  y,long  width,long  height)
{
    this->x = x;
    this->y = y;
    this->width = width;
    this->height = height;
    if(this->foreground) (this)->SetForegroundColor( this->foreground, 0, 0, 0);
    if(this->background) (this)->SetBackgroundColor( this->background, 0, 0, 0);
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
	DrawValue(this);	
	DrawThumbwheel(this,TRUE);
	if (this->label != NULL)
	    DrawLabel(this);
    }
}


void thumbV::DrawDehighlight()
{
    class value *w = (this)->Value();
    this->activefont = this->normalfont;
    this->tmpval = (w)->GetValue();
    DrawLabel( this );
    DrawValue(this);	
}

void thumbV::DrawHighlight()
{
    class value *w = (this)->Value();
    this->activefont = this->boldfont;
    DrawLabel( this );
    this->tmpval = (w)->GetValue();
    DrawValue(this);	
}


void thumbV::DrawNewValue( )
{
    class value *w = (this)->Value();
    if(this->tmpval != (w)->GetValue()){
	this->tmpval = (w)->GetValue();
	DrawValue(this);
    }
}



class valueview * thumbV::DoHit( enum view_MouseAction  type,long  x,long  y,long  hits )
{
    class value *tt = (this)->Value();
    long myval;
    static int moved;
    switch(type){
	case view_RightDown:
	case view_LeftDown:
	    this->tmpval = (tt)->GetValue();
	    this->lasty = y;
	    moved = 0;
	    break;
	case view_RightMovement:
	case view_LeftMovement	:
	    moved++;
	    if(this->granular){
		myval = this->tmpval;
		this->gran += ( this->lasty - y);
		while(this->gran > this->granular){
		    myval++;
		    this->gran -= this->granular;
		}
		while(this->gran < -this->granular){
		    myval--;
		    this->gran += this->granular;
		}
	    }
	    else myval = this->tmpval + (( this->lasty - y) * this->increment);
	    if(myval != this->tmpval){
		if(myval > this->maxval) this->tmpval = this->minval;
		else if(myval < this->minval) this->tmpval = this->maxval;
		else this->tmpval = myval;
		DrawValue(this);
		if(this->immediate)
		    (tt)->SetValue(this->tmpval);
	    }
/*	    DrawKnurl(self); */
	    this->rv += ( this->lasty - y);
	    this->lasty = y;
	    DrawThumbwheel(this,FALSE);
	    break;
	case view_RightUp:
	case view_LeftUp:
	    if(moved == 0){
		myval = (type == view_RightUp)? this->tmpval - this->increment : this->tmpval + this->increment ;
		if(myval > this->maxval) this->tmpval = this->minval;
		else if(myval < this->minval) this->tmpval = this->maxval;
		else this->tmpval = myval;
		DrawValue(this);
		this->rv += ( this->lasty - y);
		this->lasty = y;
		DrawThumbwheel(this,FALSE);
	    }
	    tt->string = (this)->GetValueString();
	    (tt)->SetValue(this->tmpval);
	    break;
    }  

    return this;
}




