/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1989 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

#include <andrewos.h>
ATK_IMPL("fourwayV.H")
#include <fourwayV.H>
#include <atom.H>
#include <atomlist.H>
#include <rm.H>
#include <buttonV.H>
static class atomlist *  AL_bodyfont;
static class atomlist *  AL_bodyfont_size;
static class atomlist *  AL_label;
static class atomlist *  AL_forecolor;
static class atomlist *  AL_backcolor;
static class atomlist *  AL_style;

static const class atom *  A_long;
static const class atom *  A_string;
#define rectangle_TempSet(X,Y,W,H,R) ((R)->top = (Y), (R)->left = (X), \
				      (R)->height = (H), (R)->width = (W), (R))

#define InternAtoms ( \
   AL_bodyfont = atomlist::StringToAtomlist("bodyfont") ,\
   AL_bodyfont_size = atomlist::StringToAtomlist("bodyfont-size") ,\
   AL_label = atomlist::StringToAtomlist("':' separated labels") ,\
   AL_forecolor = atomlist::StringToAtomlist("foreground-color") ,\
   AL_backcolor = atomlist::StringToAtomlist("background-color") ,\
   AL_style = atomlist::StringToAtomlist("style") ,\
   A_long = atom::Intern("long") ,\
   A_string = atom::Intern("string") )



/****************************************************************/
/*		class procedures 				*/
/****************************************************************/





ATKdefineRegistry(fourwayV, buttonV, fourwayV::InitializeClass);


boolean fourwayV::InitializeClass()
{
    InternAtoms;
    return TRUE;
}
fourwayV::fourwayV()
{
	ATKinit;

    (this)->SetFixedCount(4);
    (this)->SetFixedColumns(2);
    (this)->SetFourway(TRUE);
    THROWONFAILURE( TRUE);
}

void fourwayV::LookupParameters()
{
    const char * fontname;
    long fontsize;
    class buttonV *bv;
    bv = (class buttonV *) this;
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
	bv->label = (char *)parameters[0].data;
    else
	bv->label = NULL;

    if (parameters[1].found)
	fontname = (char *)parameters[1].data;
    else
	fontname = "andytype";

    if (parameters[2].found)
	fontsize = parameters[2].data;
    else
	fontsize = 10;

     if(bv->mono == -10)
	bv->mono = ((bv)->DisplayClass() & graphic::Monochrome);

     if(!bv->mono){
	 if (parameters[3].found)
	     bv->prefs->colors[sbutton_FOREGROUND] = (char *) parameters[3].data;

	 if (parameters[4].found)
	     bv->prefs->colors[sbutton_BACKGROUND] = (char *) parameters[4].data;
     }
    if (parameters[5].found)
	(bv)->HandleStyleString( (char *)parameters[5].data);

    if (fontsize != bv->fontsize || fontname != bv->fontname)
    {
	bv->fontsize = fontsize;
	bv->fontname = fontname;
    }
    bv->vertical = FALSE;
    (this)->CacheSettings();

}

