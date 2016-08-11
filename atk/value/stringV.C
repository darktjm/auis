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
static char rcsid[]="$Header: /afs/cs.cmu.edu/project/atk-src-C++/atk/value/RCS/stringV.C,v 1.3 1994/11/30 20:42:06 rr2b Stab74 $";
#endif


 


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

static class atom *  A_long;
static class atom *  A_string;

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
#ifndef NORCSID
#endif
static void CarveFonts(class stringV  * self);
static void DrawLabel(class stringV  * self);
static char *GetString(class stringV  * self);


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
    (self)->SetTransferMode(  graphic_COPY);
    (self)->EraseRectSize( self->x,self->y, self->width,self->height);

    if(self->label){	

	(self)->MoveTo(  self->width / 2 + self->x,self->height/2 + self->y);
	(self)->SetFont(  self->activefont );
	(self)->DrawString (  self->label,
				   graphic_BETWEENLEFTANDRIGHT | graphic_BETWEENTOPANDBOTTOM);
    }

}

static char *GetString(class stringV  * self)
{
    char *str;
    class value *w = (self)->Value();
    long len,val;
    char **arr;
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
    char * fontname;
    long fontsize;
    struct resourceList parameters[7];

    parameters[0].name = AL_label;
    parameters[0].type = A_string;
    parameters[1].name = AL_bodyfont;
    parameters[1].type = A_string;
    parameters[2].name = AL_bodyfont_size;
    parameters[2].type = A_long;
    parameters[3].name = AL_forecolor;
    parameters[3].type = A_string;
    parameters[4].name = AL_backcolor;
    parameters[4].type = A_string;
    parameters[5].name = NULL;
    parameters[5].type = NULL;

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
    char *str;
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
    char *str;
    if(this->UseAlt) str = GetString(this);
    else str = NULL;
    if(str != NULL && *str) this->label = str;
    else this->label = this->plabel;
    if (this->label != NULL)
	DrawLabel(this);
}
