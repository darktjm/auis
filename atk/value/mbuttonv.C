#include <andrewos.h>
ATK_IMPL("mbuttonv.H")
#include <mbuttonv.H>
#include <atom.H>
#include <atomlist.H>
#include <rm.H>
#include <buttonV.H>
#include <ctype.h>

static class atomlist *  AL_bodyfont;
static class atomlist *  AL_bodyfont_size;
static class atomlist *  AL_label;
static class atomlist *  AL_placement;
static class atomlist *  AL_forecolor;
static class atomlist *  AL_backcolor;
static class atomlist *  AL_style;
static const class atom *  A_long;
static const class atom *  A_string;

#define InternAtoms ( \
AL_bodyfont = atomlist::StringToAtomlist("bodyfont") ,\
AL_bodyfont_size = atomlist::StringToAtomlist("bodyfont-size") ,\
AL_label = atomlist::StringToAtomlist("label") ,\
AL_placement = atomlist::StringToAtomlist("placement") ,\
AL_forecolor = atomlist::StringToAtomlist("foreground-color")  ,\
AL_backcolor = atomlist::StringToAtomlist("background-color") ,\
AL_style = atomlist::StringToAtomlist("style") ,\
A_long = atom::Intern("long") ,\
A_string = atom::Intern("string") )



ATKdefineRegistry(mbuttonv, buttonV, mbuttonv::InitializeClass);

void mbuttonv::LookupParameters()
     {
  const char * fontname;
  long fontsize;
  char *plc;
  class buttonV *bv = (class buttonV *) this;
  static struct resourceList parameters[] = {
	{ AL_label, A_string }, /* 0 */
	{ AL_bodyfont, A_string }, /* 1 */
	{ AL_bodyfont_size, A_long }, /* 2 */
	{ AL_placement, A_string }, /* 3 */
	{ AL_forecolor, A_string }, /* 4 */
	{ AL_backcolor, A_string }, /* 5 */
	{ AL_style, A_string }, /* 6 */
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

  if (parameters[3].found)
      plc = (char *)parameters[3].data;
  else
      plc = NULL;

  if(bv->mono == -10)
      bv->mono = ((bv)->DisplayClass() & graphic_Monochrome);

  if(!bv->mono){
      if (parameters[4].found)
	  bv->prefs->colors[sbutton_FOREGROUND] = (char *) parameters[4].data;

      if (parameters[5].found)
	  bv->prefs->colors[sbutton_BACKGROUND] = (char *) parameters[5].data;
  }
  bv->columns = 1;
  bv->topdown = FALSE;
  bv->vertical = TRUE;

  if(plc != NULL){
      if(*plc) {
	  /* restore defaults */
	  bv->columns = 1;
	  bv->topdown = FALSE;
	  bv->vertical = TRUE;
      }
      while (*plc != '\0'){
	  switch (isupper(*plc)?tolower(*plc):*plc){
	      case 'h':
	      case 'c':
		  bv->vertical = FALSE;
		  break;
	      case 'v':
	      case 'r':
		  bv->vertical = TRUE;
		  break;
	      case 't':
		  bv->topdown = TRUE;
		  break;
	      case 'b':
		  bv->topdown = FALSE;
		  break;
	      case '1': case '2': case '3':
	      case '4': case '5': case '6':
	      case '7': case '8': case '9':
		  if(bv->fixedcolumns == 0) bv->columns = (int) (*plc - '0');
		  break;
	  }
	  plc++;
      }
  }
  else bv->vertical = TRUE;
  if (parameters[6].found)
	(bv)->HandleStyleString( (char *)parameters[6].data);

  bv->fontsize = fontsize;
  bv->fontname = fontname;
  (this)->CacheSettings();

}

mbuttonv::mbuttonv()
{
	ATKinit;

    (this)->SetFixedCount(0);
    (this)->SetFixedColumns(0);
    THROWONFAILURE( TRUE);
}

boolean mbuttonv::InitializeClass()
{
    InternAtoms;
    return TRUE;
}


