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

#include <andrewos.h>

#ifndef NORCSID
#define NORCSID
static UNUSED const char rcsid[]="$Header: /afs/cs.cmu.edu/project/atk-src-C++/atk/value/RCS/checkv.C,v 1.3 1994/11/30 20:42:06 rr2b Stab74 $";
#endif


 


ATK_IMPL("checkv.H")
#include <checkv.H>
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
static class atomlist *  AL_bodyfont;
static class atomlist *  AL_bodyfont_size;
static class atomlist *  AL_label;
static class atomlist *  AL_checktype;
static class atomlist *  AL_forecolor;
static class atomlist *  AL_backcolor;

static class atom *  A_long;
static class atom *  A_string;

#define InternAtoms ( \
   AL_bodyfont = atomlist::StringToAtomlist("bodyfont") ,\
   AL_bodyfont_size = atomlist::StringToAtomlist("bodyfont-size") ,\
   AL_label = atomlist::StringToAtomlist("label") ,\
   AL_checktype = atomlist::StringToAtomlist("checktype") ,\
   AL_forecolor = atomlist::StringToAtomlist("foreground-color")  ,\
   AL_backcolor = atomlist::StringToAtomlist("background-color") ,\
   A_long = atom::Intern("long") ,\
   A_string = atom::Intern("string") )


#define Min(X,Y) ((X) < (Y) ? (X) : (Y))
#define FUDGE 2


/****************************************************************/
/*		private functions				*/
/****************************************************************/


ATKdefineRegistry(checkv, valueview, checkv::InitializeClass);
#ifndef NORCSID
#endif
static void CarveFonts(class checkv  * self);
static void Drawcheck(class checkv  * self);


static void CarveFonts(class checkv  * self)
{
    self->normalfont = fontdesc::Create( self->fontname, fontdesc_Plain, self->fontsize );
}  

static void Drawcheck(class checkv  * self)
{
    register int side;
    register int gap;
    struct rectangle r;
    char ch;
    (self)->SetTransferMode(  graphic_COPY );

    if (!self->BlackPattern)
	self->BlackPattern = (self)->view::BlackPattern();
    side = Min(self->width, self->height);
    gap  = FUDGE;
    side = side - (gap*2) + 1;
    (self)->EraseRectSize( self->x,self->y,self->width,self->height);
    (self)->SetTransferMode(  graphic_BLACK );
    (self)->MoveTo( self->x + gap, self->y + gap);
 
    /* Box */
    (self)->DrawLineTo( self->x + gap, self->y + side);
    (self)->DrawLineTo( self->x + side, self->y + side);
    (self)->DrawLineTo( self->x + side, self->y + gap);
    (self)->DrawLineTo( self->x + gap, self->y + gap);

    /* Check */
    if (self->tmpval)
	switch(self->checktype) {
	    case 2: /* Solid */
		r.top    = self->y + gap*2;
		r.left   = self->x + gap*2;
		r.width  = side - gap*2 - 1;
		r.height = side - gap*2 - 1;
		(self)->SetTransferMode(  graphic_COPY );
		(self)->FillRect( &r, self->BlackPattern);
		break;
	    default: /* Cross */
		(self)->DrawLineTo( self->x + side, self->y + side);
		(self)->MoveTo( self->x + gap, self->y +side);
		(self)->DrawLineTo( self->x + side, self->y + gap);
		break;
	}

    if(self->label){
	(self)->MoveTo( self->x + side + gap*2, self->y + (side+gap)/2);
	(self)->SetFont( self->normalfont);
	(self)->DrawString(self->label,graphic_ATLEFT | graphic_BETWEENTOPANDBASELINE);
    }
}



/****************************************************************/
/*		class procedures 				*/
/****************************************************************/




boolean checkv::InitializeClass()
{
    InternAtoms;
    return TRUE;
}


#define BADVAL -22222
/****************************************************************/
/*		instance methods				*/
/****************************************************************/
checkv::checkv()
{
	ATKinit;

    this->label = NULL;
    this->fontname = NULL;
    this->fontsize = 0;
    this->tmpval = BADVAL;
    this->BlackPattern = NULL;
    this->foreground = this->background = NULL;
   THROWONFAILURE( TRUE);
}

void checkv::LookupParameters()
{
    char * fontname;
    long fontsize;
    struct resourceList parameters[7];

    parameters[0].name = AL_label;
    parameters[0].type = A_string;
    parameters[1].name = AL_bodyfont;
    parameters[1].type = A_string;
    parameters[2].name = AL_bodyfont_size;
    parameters[2].type = A_long;
    parameters[3].name = AL_checktype;
    parameters[3].type = A_long;
    parameters[4].name = AL_forecolor;
    parameters[4].type = A_string;
    parameters[5].name = AL_backcolor;
    parameters[5].type = A_string;
    parameters[6].name = NULL;
    parameters[6].type = NULL;

    (this)->GetManyParameters( parameters, NULL, NULL);

    if (parameters[0].found)
	this->label = (char *)parameters[0].data;
    else
	this->label = NULL ;

    if (parameters[1].found)
	fontname = (char *)parameters[1].data;
    else
	fontname = "andysansb";

    if (parameters[2].found)
	fontsize = parameters[2].data;
    else
	fontsize = 10;

    if (parameters[3].found)
	this->checktype = (short) parameters[3].data;
    else
	this->checktype = 1;

    if (parameters[4].found)
	this->foreground = (char *) parameters[4].data;

    if (parameters[5].found)
	this->background = (char *) parameters[5].data;


    if (fontsize != this->fontsize || fontname != this->fontname)
    {
	this->fontsize = fontsize;
	this->fontname = fontname;
	CarveFonts(this);
    }
}


void checkv::DrawFromScratch(long  x,long  y,long  width,long  height)
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
	if(this->tmpval == BADVAL){
	    this->tmpval = (w)->GetValue();
	}
	Drawcheck(this);	
    }
}


void checkv::DrawDehighlight()
{

    class value *w = (this)->Value();
    this->tmpval = (w)->GetValue();
    Drawcheck(this);	

}

void checkv::DrawHighlight()
{
}


void checkv::DrawNewValue( )
{
    class value *w = (this)->Value();
    this->tmpval = (w)->GetValue();
    Drawcheck(this);	
}



class valueview * checkv::DoHit( enum view_MouseAction  type,long  x,long  y,long  hits )
{
    class value *tt = (this)->Value();
    switch(type){
	case view_LeftDown:
	    this->tmpval = !(tt)->GetValue();
	    Drawcheck(this);
	    break;
	case view_LeftUp:
	    (tt)->SetValue(this->tmpval);
	    break;
    }  

    return this;
}





