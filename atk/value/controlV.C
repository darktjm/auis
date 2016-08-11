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
static char rcsid[]="$Header: /afs/cs.cmu.edu/project/atk-src-C++/atk/value/RCS/controlV.C,v 1.3 1994/11/30 20:42:06 rr2b Stab74 $";
#endif


 



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

static class atom *  A_long;
static class atom *  A_string;
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
#ifndef NORCSID
#endif


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
  char * fontname;
  long fontsize;
  struct resourceList parameters[10];

  parameters[0].name = AL_label;
  parameters[0].type = A_string;
  parameters[1].name = AL_bodyfont;
  parameters[1].type = A_string;
  parameters[2].name = AL_bodyfont_size;
  parameters[2].type = A_long;
  parameters[3].name = AL_class;
  parameters[3].type = A_string;
  parameters[4].name = AL_function;
  parameters[4].type = A_string;
  parameters[5].name = AL_AutoInit;
  parameters[5].type = A_string;
  parameters[6].name = AL_forecolor;
  parameters[6].type = A_string;
  parameters[7].name = AL_backcolor;
  parameters[7].type = A_string;
  parameters[8].name = AL_style;
  parameters[8].type = A_string;
  parameters[9].name = NULL;
  parameters[9].type = NULL;

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

