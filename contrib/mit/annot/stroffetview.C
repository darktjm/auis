/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'     *
\* ********************************************************************** */

/*
	$Disclaimer: 
 * Permission to use, copy, modify, and distribute this software and its 
 * documentation for any purpose and without fee is hereby granted, provided 
 * that the above copyright notice appear in all copies and that both that 
 * copyright notice and this permission notice appear in supporting 
 * documentation, and that the name of IBM not be used in advertising or 
 * publicity pertaining to distribution of the software without specific, 
 * written prior permission. 
 *                         
 * THE COPYRIGHT HOLDERS DISCLAIM ALL WARRANTIES WITH REGARD 
 * TO THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF 
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL ANY COPYRIGHT 
 * HOLDER BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL 
 * DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, 
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE 
 * OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION 
 * WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 * 
 *  $
*/

#include <andrewos.h>
#include <fontdesc.H>
#include "view.H"
#include "textview.H"
#include "stroffet.H"
#include "iconview.H"
#include "stroffetview.H"
#include "bind.H"
#include "menulist.H"
#include "keymap.H"
#include "text.H"
#include "proctable.H"

#define ICONFONT "icon"
#define ICONSTYLE fontdesc_Plain
#define ICONPTS 12
#define ICONCHAR '4'
#define TITLEFONT "andysans"
#define TITLESTYLE fontdesc_Plain
#define TITLEPTS 12

class menulist *stroffetviewMenus;
static class keymap *stroffetviewKeyMap;


/****************************************************************/
/*		private functions				*/
/****************************************************************/


ATKdefineRegistry(stroffetview, iconview, stroffetview::InitializeClass);
static void Close(class stroffetview  *v,long  l);
static void closeall(class view  *v,long  l);
static void openall(class view  *v,long  l);
static void insert(class textview  *tv,long  l);


static void
Close(class stroffetview  *v,long  l)
{
    (v)->Close();
}
/* static void
copen(v,l)
struct stroffetview *v;
long l;
{
    stroffetview_Open(v);
} */
static void
closeall(class view  *v,long  l)
{
    iconview::CloseRelated(v);
}
static void
openall(class view  *v,long  l)
{
    iconview::OpenRelated(v);
}

static void
insert(class textview  *tv,long  l)
{
    class text *t;
    long pos;
    t = (class text *) (tv)->GetDataObject();
    pos = (tv)->GetDotPosition() + (tv)->GetDotLength();
    tv->currentViewreference = (t)->InsertObject( pos,"stroffet", NULL);
    (t)->NotifyObservers(0);
}

static struct bind_Description stroffetviewBindings[]={
    {"stroffetview-close",NULL,0,"stroffets,close~2", 0,0,(proctable_fptr)Close,"close the stroffet"},
    {"stroffetview-closeall",NULL,0,"stroffets,close all~12", 0,0,(proctable_fptr)closeall,"close all the stroffets"},
    {"stroffetview-openall",NULL,0,"stroffets,open all~11", 0,0,(proctable_fptr)openall,"open all the stroffets"},
    NULL
};
void stroffetview::PostMenus(class menulist  *menulist)
{
    (this->menus)->ClearChain();
    (this->menus)->ChainBeforeML( menulist, (long)menulist);
    (this)->iconview::PostMenus( this->menus);
}


/****************************************************************/
/*		class procedures				*/
/****************************************************************/

boolean
stroffetview::InitializeClass()
    {
    struct ATKregistryEntry  *textviewtype = ATK::LoadClass("textview");
    struct ATKregistryEntry  *viewtype = ATK::LoadClass("view");

    stroffetviewMenus = new menulist;
    stroffetviewKeyMap =  new keymap;
    bind::BindList(stroffetviewBindings, stroffetviewKeyMap , stroffetviewMenus, &stroffetview_ATKregistry_ );
    proctable::DefineProc("stroffetview-insertstroffet",(proctable_fptr)insert,textviewtype,NULL,"Insert Stroffet Object");
    proctable::DefineProc("stroffetview-openallstroffets",(proctable_fptr)openall,viewtype,NULL,"open Stroffet Views");
    proctable::DefineProc("stroffetview-closeallstroffets",(proctable_fptr)closeall,viewtype,NULL,"close Stroffet Views");
    return TRUE;
}


stroffetview::stroffetview()
{
	ATKinit;


    this->menus = (stroffetviewMenus)->DuplicateML( this);
    (this)->SetIconFont(ICONFONT,ICONSTYLE,ICONPTS); 
    (this)->SetIconChar(ICONCHAR);
    (this)->SetTitleFont(TITLEFONT,TITLESTYLE,TITLEPTS);


    THROWONFAILURE( TRUE);
}

/****************************************************************/
/*		instance methods				*/
/****************************************************************/

void
stroffetview::Print(FILE  * file, const char  * processor, const char  * finalformat, boolean  toplevel)
                    {
  class textview * textvobj;
  class text * textobj;
  long c, pos = 0, textlength = 0;
  

  if (this->child == (class view *)0)
    return;


  textvobj = (class textview *) this->bottomview;
  textobj = (class text *) (textvobj)->GetDataObject();

  textlength = (textobj)->GetLength();


  /* 
   * We don't want to be the toplevel.  We also don't want to
   * process with anything other than troff.  If we are, then we pout,
   * and refuse to print anything. 
   */
  if (!toplevel
      && strcmp(processor,"troff") == 0) 
    {
      while ((c = (textobj)->GetChar(pos)) != EOF 
	     && pos < textlength)
	{
	  fputc(c,file);
	  /*
	   * Formatting: Just Say No.
	   */
	  pos++; 
	}

      /* 
       * We want this to be newline-terminated, even if the user
       * doesn't put a newline at the end of the code segment.
       *
       * I'm confused.  If you track down the tree, text_GetChar is 
       * actually simpletext_GetChar.  Now, go and look at the code for
       * simpletext__GetChar.  It is defined as returning a long, and it
       * happily passes back a char *. 
       */
      if ((textlength != 0) && ((textobj)->GetChar( textlength - 1) != '\n'))
	{
	  fputc('\n',file);
	}
    }
}

