

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
static UNUSED const char rcsid[]="$Header: /afs/cs.cmu.edu/project/atk-src-C++/atk/value/RCS/mbuttonv.C,v 1.3 1994/11/30 20:42:06 rr2b Stab74 $";
#endif

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
static class atom *  A_long;
static class atom *  A_string;

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
#ifndef NORCSID
#endif


void mbuttonv::LookupParameters()
     {
  char * fontname;
  long fontsize;
  struct resourceList parameters[8];
  char *plc;
  class buttonV *bv = (class buttonV *) this;

  parameters[0].name = AL_label;
  parameters[0].type = A_string;
  parameters[1].name = AL_bodyfont;
  parameters[1].type = A_string;
  parameters[2].name = AL_bodyfont_size;
  parameters[2].type = A_long;
  parameters[3].name = AL_placement;
  parameters[3].type = A_string;
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


