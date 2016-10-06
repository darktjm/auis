/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1989 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

#include <andrewos.h>
ATK_IMPL("controlV.H")
#include <buttonV.H>
#include <controlV.H>
#include <proctable.H>
#include <message.H>
#include <fontdesc.H>
#include <rect.h>
#include <value.H>
#include <atom.H>
#include <atomlist.H>
#include <rm.H>
#include <im.H>
#include <cursor.H>

static boolean Inhibit;
static class atomlist *  AL_bodyfont;
static class atomlist *  AL_bodyfont_size;
static class atomlist *  AL_label;
static class atomlist *  AL_class;
static class atomlist *  AL_function;
static class atomlist *  AL_AutoInit;
static class atomlist *  AL_forecolor;
static class atomlist *  AL_backcolor;
static class atomlist *  AL_style;

static const class atom *  A_long;
static const class atom *  A_string;
static class cursor *WaitCursor;
#define InternAtoms ( \
   AL_bodyfont = atomlist::StringToAtomlist("bodyfont") ,\
   AL_bodyfont_size = atomlist::StringToAtomlist("bodyfont-size") ,\
   AL_label = atomlist::StringToAtomlist("label") ,\
   AL_class = atomlist::StringToAtomlist("class") ,\
   AL_function = atomlist::StringToAtomlist("function") ,\
   AL_AutoInit = atomlist::StringToAtomlist("Auto-Init") ,\
   AL_forecolor = atomlist::StringToAtomlist("foreground-color")  ,\
   AL_backcolor = atomlist::StringToAtomlist("background-color") ,\
   AL_style = atomlist::StringToAtomlist("style") ,\
   A_long = atom::Intern("long") ,\
   A_string = atom::Intern("string") )


ATKdefineRegistry(controlV, buttonV, controlV::InitializeClass);

static boolean DoFunc(class controlV  * self)
{
    char iname[256];
    struct proctable_Entry *pr;
    proctable_fptr proc;
    int res;
    self->needsinit = FALSE;
    if(self->cclass && self->function){
	strcpy(iname,self->cclass);
	if(ATK::LoadClass(iname) != NULL){
	    strcat(iname,"-");
	    strcat(iname,self->function);
	    if((pr = proctable::Lookup(iname)) != NULL && proctable::Defined(pr) ){
		proc = proctable::GetFunction(pr) ;
		if(WaitCursor) im::SetProcessCursor(WaitCursor);
		res = (*proc)(self,0);
		if(WaitCursor) im::SetProcessCursor(NULL);
		return res;
	    }
	}
    }
    return FALSE;
}
class valueview *controlV::DoHit(enum view_MouseAction  type,long  x,long  y,long  hits )
{

    if (type == view_RightUp || type == view_LeftUp)
	DoFunc(this);
    return (this)->buttonV::DoHit(type,x,y,hits );
}

void controlV::LookupParameters()
     {
    class buttonV *bv = (class buttonV *) this;
  const char * fontname;
  long fontsize;
  static struct resourceList parameters[] = {
	{ AL_label, A_string }, /* 0 */
	{ AL_bodyfont, A_string }, /* 1 */
	{ AL_bodyfont_size, A_long }, /* 2 */
	{ AL_class, A_string }, /* 3 */
	{ AL_function, A_string }, /* 4 */
	{ AL_AutoInit, A_string }, /* 5 */
	{ AL_forecolor, A_string }, /* 6 */
	{ AL_backcolor, A_string }, /* 7 */
	{ AL_style, A_string }, /* 8 */
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
    this->cclass = (char *)parameters[3].data;
  else
    this->cclass = NULL;

  if (parameters[4].found)
    this->function = (char *) parameters[4].data;
  else
      this->function = "start";

    if(bv->mono == -10)
	bv->mono = ((bv)->DisplayClass() & graphic_Monochrome);

    if(!bv->mono){

      if (parameters[6].found)
	  bv->prefs->colors[sbutton_FOREGROUND] = (char *) parameters[6].data;

      if (parameters[7].found)
	  bv->prefs->colors[sbutton_BACKGROUND] = (char *) parameters[7].data;
  }
  if (parameters[8].found)
	(bv)->HandleStyleString( (char *)parameters[8].data);

  this->autoinit = FALSE;
  if (parameters[5].found){
      char *foo;
      foo = (char *) parameters[5].data;
      if(*foo == 't' || *foo == 'T')
	  this->autoinit = TRUE;
  }

  if (fontsize != bv->fontsize || fontname != bv->fontname)
    {
      bv->fontsize = fontsize;
      bv->fontname = fontname;
    }
  (this)->CacheSettings();

}

boolean controlV::InitializeClass()
{
  InternAtoms;
  WaitCursor = cursor::Create(NULL);
  if(WaitCursor) (WaitCursor)->SetStandard(Cursor_Wait);
  Inhibit = FALSE;
  return TRUE;
}

controlV::controlV()
{
	ATKinit;

    this->cclass = NULL;
    this->function = NULL;
    this->autoinit = FALSE;
    this->needsinit = TRUE;
    THROWONFAILURE( TRUE);
}
void controlV::DrawFromScratch(long  x,long  y,long  width,long  height)
{
    (this)->buttonV::DrawFromScratch(x,y,width,height);
    if(this->autoinit && this->needsinit && !Inhibit){
	DoFunc(this);
    }
}

void controlV::InhibitAutoInit()
{
	ATKinit;

    Inhibit = TRUE;
}
void controlV::SetAutoInit(boolean  val)
{
	ATKinit;

    Inhibit = !val;
}

