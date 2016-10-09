/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1989 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

#include <andrewos.h>
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

static const class atom *  A_long;
static const class atom *  A_string;

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
static void CarveFonts(class checkv  * self);
static void Drawcheck(class checkv  * self);


static void CarveFonts(class checkv  * self)
{
    self->normalfont = fontdesc::Create( self->fontname, fontdesc_Plain, self->fontsize );
}  

static void Drawcheck(class checkv  * self)
{
    int side;
    int gap;
    struct rectangle r;
    (self)->SetTransferMode(  graphic::COPY );

    if (!self->BlackPattern)
	self->BlackPattern = (self)->view::BlackPattern();
    side = Min(self->width, self->height);
    gap  = FUDGE;
    side = side - (gap*2) + 1;
    (self)->EraseRectSize( self->x,self->y,self->width,self->height);
    (self)->SetTransferMode(  graphic::BLACK );
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
		(self)->SetTransferMode(  graphic::COPY );
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
	(self)->DrawString(self->label,graphic::ATLEFT | graphic::BETWEENTOPANDBASELINE);
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
    const char * fontname;
    long fontsize;
    static struct resourceList parameters[] = {
	{ AL_label, A_string }, /* 0 */
	{ AL_bodyfont, A_string }, /* 1 */
	{ AL_bodyfont_size, A_long }, /* 2 */
	{ AL_checktype, A_long }, /* 3 */
	{ AL_forecolor, A_string }, /* 4 */
	{ AL_backcolor, A_string }, /* 5 */
	{ NULL, NULL }
    };

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
	default:
	    break;
    }  

    return this;
}





