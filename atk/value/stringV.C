/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1989 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

#include <andrewos.h>
ATK_IMPL("stringV.H")
#include <stringV.H>
#include <fontdesc.H>
#include <rect.h>
#include <buffer.H>
#include <proctable.H>
#include <atom.H>
#include <atomlist.H>
#include <graphic.H>
#include <rm.H>
#include <view.H>
#include <value.H>
static class atomlist *  AL_bodyfont;
static class atomlist *  AL_bodyfont_size;
static class atomlist *  AL_label;
static class atomlist *  AL_forecolor;
static class atomlist *  AL_backcolor;

static const class atom *  A_long;
static const class atom *  A_string;

#define InternAtoms ( \
   AL_bodyfont = atomlist::StringToAtomlist("bodyfont") ,\
   AL_bodyfont_size = atomlist::StringToAtomlist("bodyfont-size") ,\
   AL_label = atomlist::StringToAtomlist("label") ,\
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


ATKdefineRegistry(stringV, valueview, stringV::InitializeClass);
static void CarveFonts(class stringV  * self);
static void DrawLabel(class stringV  * self);
static const char *GetString(class stringV  * self);


static void CarveFonts(class stringV  * self)
{
    self->normalfont = fontdesc::Create( self->fontname, fontdesc_Plain, self->fontsize );
    self->boldfont   = fontdesc::Create( self->fontname, fontdesc_Bold,  self->fontsize );
    self->activefont = self->mouseIsOnTarget ? self->boldfont : self->normalfont;
}  


static void DrawLabel(class stringV  * self)
{
    if(self->foreground) (self)->SetForegroundColor( self->foreground, 0, 0, 0);
    if(self->background) (self)->SetBackgroundColor( self->background, 0, 0, 0);
    (self)->SetTransferMode(  graphic::COPY);
    (self)->EraseRectSize( self->x,self->y, self->width,self->height);

    if(self->label){	

	(self)->MoveTo(  self->width / 2 + self->x,self->height/2 + self->y);
	(self)->SetFont(  self->activefont );
	(self)->DrawString (  self->label,
				   graphic::BETWEENLEFTANDRIGHT | graphic::BETWEENTOPANDBOTTOM);
    }

}

static const char *GetString(class stringV  * self)
{
    const char *str;
    class value *w = (self)->Value();
    long len,val;
    const char * const *arr;
    str = (w)->GetString();
    if(str == NULL && 
	((len = (w)->GetArraySize()) != 0) && 
	((arr = (w)->GetStringArray()) != NULL) && 
	  ((val = (w)->GetValue())>= 0) && 
	  val < len)
	str = arr[val];
    return str;
}
/****************************************************************/
/*		class procedures 				*/
/****************************************************************/




boolean stringV::InitializeClass()
{
    InternAtoms;
    return TRUE;
}




#define BADVAL -22222
/****************************************************************/
/*		instance methods				*/
/****************************************************************/
stringV::stringV()
{
	ATKinit;

    this->plabel = NULL;
    this->label = NULL;
    this->fontname = NULL;
    this->fontsize = 0;
    this->UseAlt = TRUE;
    this->foreground = this->background = NULL;
    THROWONFAILURE( TRUE);
}


void stringV::LookupParameters()
{
    const char * fontname;
    long fontsize;
    static struct resourceList parameters[] = {
	{ AL_label, A_string }, /* 0 */
	{ AL_bodyfont, A_string }, /* 1 */
	{ AL_bodyfont_size, A_long }, /* 2 */
	{ AL_forecolor, A_string }, /* 3 */
	{ AL_backcolor, A_string }, /* 4 */
	{ NULL, NULL }
    };

    (this)->GetManyParameters( parameters, NULL, NULL);

    if (parameters[0].found)
	this->plabel = (char *)parameters[0].data;
    else
	this->plabel = NULL;
    this->label = this->plabel;
    if (parameters[1].found)
	fontname = (char *)parameters[1].data;
    else
	fontname = "andytype";

    if (parameters[2].found)
	fontsize = parameters[2].data;
    else
	fontsize = 10;

    this->foreground = this->background = NULL;

    if (parameters[3].found)
	this->foreground = (char *) parameters[3].data;

    if (parameters[4].found)
	this->background = (char *) parameters[4].data;

 
    if (fontsize != this->fontsize || fontname != this->fontname)
    {
	this->fontsize = fontsize;
	this->fontname = fontname;
	CarveFonts(this);
    }
}


void stringV::DrawFromScratch(long  x,long  y,long  width,long  height)
{
    const char *str;
    this->x = x; this->y = y;
    this->height = height; this->width = width;
    if (width > 0 && height > 0)
    {
	if(this->UseAlt) str = GetString(this);
	else str = NULL;
	this->activefont = this->mouseIsOnTarget ?
	  this->boldfont : this->normalfont;
	if(str != NULL && *str) this->label = str;
	else this->label = this->plabel;
	if (this->label != NULL)
	    DrawLabel(this);
    }
}


void stringV::DrawDehighlight()
{
    this->activefont = this->normalfont;
    DrawLabel( this );
}

void stringV::DrawHighlight()
{
    this->activefont = this->boldfont;
    DrawLabel( this );
}
 
void stringV::DrawNewValue( )
{
    const char *str;
    if(this->UseAlt) str = GetString(this);
    else str = NULL;
    if(str != NULL && *str) this->label = str;
    else this->label = this->plabel;
    if (this->label != NULL)
	DrawLabel(this);
}

view::DSattributes stringV::DesiredSize(long  width , long  height, enum view::DSpass  pass, long  *desiredwidth , long  *desiredheight)

{
    struct FontSummary *fs;
    int ret = view::WidthLarger|view::HeightLarger;

    if(!this->fontname) // params haven't been read yet; no idea what to return
	return valueview::DesiredSize(width, height, pass, desiredwidth, desiredheight);
    fs = boldfont->FontSummary(GetDrawable());
    *desiredheight = fs->newlineHeight == 0 ? fs->maxHeight : fs->newlineHeight;
    *desiredheight += 2 * y + 4; // y is border, 2*2 is padding
    if(label) {
	boldfont->StringSize(GetDrawable(), label, &width, &height);
	*desiredwidth = width + 2 * x + 4; // x is border, 2*2 is padding
    } else {
	*desiredwidth = 75;
	ret |= view::WidthSmaller;
    }
    return (view::DSattributes)ret;
}
