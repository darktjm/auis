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
ATK_IMPL("onoffV.H")
#include <onoffV.H>
#include <atom.H>
#include <atomlist.H>
#include <rm.H>
#include <buttonV.H>
static class atomlist *  AL_bodyfont;
static class atomlist *  AL_bodyfont_size;
static class atomlist *  AL_bottom_label;
static class atomlist *  AL_top_label;
static class atomlist *  AL_forecolor;
static class atomlist *  AL_backcolor;
static class atomlist *  AL_style;

static class atom *  A_long;
static class atom *  A_string;

#define InternAtoms ( \
   AL_bodyfont = atomlist::StringToAtomlist("bodyfont") ,\
   AL_bodyfont_size = atomlist::StringToAtomlist("bodyfont-size") ,\
   AL_bottom_label = atomlist::StringToAtomlist("top label") ,\
   AL_top_label = atomlist::StringToAtomlist("bottom label") ,\
   AL_forecolor = atomlist::StringToAtomlist("foreground-color")  ,\
   AL_backcolor = atomlist::StringToAtomlist("background-color") ,\
   AL_style = atomlist::StringToAtomlist("style") ,\
   A_long = atom::Intern("long") ,\
   A_string = atom::Intern("string") )




ATKdefineRegistry(onoffV, buttonV, onoffV::InitializeClass);

boolean onoffV::InitializeClass()
{
    InternAtoms;
    return TRUE;
}




#define BADVAL -22222
/****************************************************************/
/*		instance methods				*/
/****************************************************************/
onoffV::onoffV()
{
	ATKinit;

    (this)->SetFixedCount(2);
    THROWONFAILURE( TRUE);
}


void onoffV::LookupParameters()
{
    const char * fontname;
    long fontsize;
    struct resourceList parameters[8];
    class buttonV *bv = (class buttonV *) this;

    parameters[0].name = AL_top_label;
    parameters[0].type = A_string;
    parameters[1].name = AL_bottom_label;
    parameters[1].type = A_string;
    parameters[2].name = AL_bodyfont;
    parameters[2].type = A_string;
    parameters[3].name = AL_bodyfont_size;
    parameters[3].type = A_long;
    parameters[4].name = AL_forecolor;
    parameters[4].type = A_string;
    parameters[5].name = AL_backcolor;
    parameters[5].type = A_string;
    parameters[6].name = AL_style;
    parameters[6].type = A_string;
    parameters[7].name = NULL;
    parameters[7].type = NULL;

    (this)->GetManyParameters( parameters, NULL, NULL);

    if (parameters[0].found)
	bv->l[0] = (char *)parameters[0].data;
    else
	bv->l[0] = NULL ;

    if (parameters[1].found)
	bv->l[1] = (char *)parameters[1].data;
    else
	bv->l[1] = NULL;

    if (parameters[2].found)
	fontname = (char *)parameters[2].data;
    else
	fontname = "andytype";

    if (parameters[3].found)
	fontsize = parameters[3].data;
    else
	fontsize = 10;
    if(bv->mono == -10)
	bv->mono = ((bv)->DisplayClass() & graphic_Monochrome);

    if(!bv->mono){
	if (parameters[4].found)
	    bv->prefs->colors[sbutton_FOREGROUND] = (char *) parameters[4].data;

	if (parameters[5].found)
	    bv->prefs->colors[sbutton_BACKGROUND] = (char *) parameters[5].data;
    }
    if (parameters[6].found)
	(bv)->HandleStyleString( (char *)parameters[6].data);

    if (fontsize != bv->fontsize || fontname != bv->fontname)
    {
	bv->fontsize = fontsize;
	bv->fontname = fontname;
    }
    bv->label = NULL;
    bv->vertical = TRUE;
    (this)->CacheSettings();
}

