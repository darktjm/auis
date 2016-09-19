/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1989 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

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
    class buttonV *bv = (class buttonV *) this;
    static struct resourceList parameters[] = {
	{ AL_top_label, A_string }, /* 0 */
	{ AL_bottom_label, A_string }, /* 1 */
	{ AL_bodyfont, A_string }, /* 2 */
	{ AL_bodyfont_size, A_long }, /* 3 */
	{ AL_forecolor, A_string }, /* 4 */
	{ AL_backcolor, A_string }, /* 5 */
	{ AL_style, A_string }, /* 6 */
	{ NULL, NULL }
    };

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

