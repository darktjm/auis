/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/config/COPYRITE'     *
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
#include "note.H"
#include "iconview.H"
#include "noteview.H"
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

class menulist *noteviewMenus;
static class keymap *noteviewKeyMap;


/****************************************************************/
/*		private functions				*/
/****************************************************************/


ATKdefineRegistry(noteview, iconview, noteview::InitializeClass);
static void Close(class noteview  *v,long  l);
static void closeall(class view  *v,long  l);
static void openall(class view  *v,long  l);
static void insert(class textview  *tv,long  l);


static void
Close(class noteview  *v,long  l)
{
    (v)->Close();
}
/* static void
copen(v,l)
struct noteview *v;
long l;
{
    noteview_Open(v);
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
    tv->currentViewreference = (t)->InsertObject( pos,"note", NULL);
    (t)->NotifyObservers(0);
}

static const struct bind_Description noteviewBindings[]={
    {"noteview-close",NULL,0,"notes,close~2", 0,0,(proctable_fptr)Close,"close the note"},
    {"noteview-closeall",NULL,0,"notes,close all~12", 0,0,(proctable_fptr)closeall,"close all the notes"},
    {"noteview-openall",NULL,0,"notes,open all~11", 0,0,(proctable_fptr)openall,"open all the notes"},
    NULL
};
void noteview::PostMenus(class menulist  *menulist)
{
    (this->menus)->ClearChain();
    (this->menus)->ChainBeforeML( menulist, (long)menulist);
    (this)->iconview::PostMenus( this->menus);
}


/****************************************************************/
/*		class procedures				*/
/****************************************************************/

boolean
noteview::InitializeClass()
    {
    struct ATKregistryEntry  *textviewtype = ATK::LoadClass("textview");
    struct ATKregistryEntry  *viewtype = ATK::LoadClass("view");

    noteviewMenus = new menulist;
    noteviewKeyMap =  new keymap;
    bind::BindList(noteviewBindings, noteviewKeyMap , noteviewMenus, &noteview_ATKregistry_ );
    proctable::DefineProc("noteview-insertnote",(proctable_fptr)insert,textviewtype,NULL,"Insert Note Object");
    proctable::DefineProc("noteview-openallnotes",(proctable_fptr)openall,viewtype,NULL,"open Note Views");
    proctable::DefineProc("noteview-closeallnotes",(proctable_fptr)closeall,viewtype,NULL,"close Note Views");
    return TRUE;
}


noteview::noteview()
{
	ATKinit;


    this->menus = (noteviewMenus)->DuplicateML( this);
    (this)->SetIconFont(ICONFONT,ICONSTYLE,ICONPTS); 
    (this)->SetIconChar(ICONCHAR);
    (this)->SetTitleFont(TITLEFONT,TITLESTYLE,TITLEPTS);


    THROWONFAILURE( TRUE);
}

/****************************************************************/
/*		instance methods				*/
/****************************************************************/

void
noteview::Print(FILE  * file, const char  * processor, const char  * finalformat, boolean  toplevel)
                    {
    short doit;
    if (this->child == (class view *)0)
	return;

    doit = 0;

    if (!toplevel
	 && strcmp(processor,"troff") == 0) {
	fputs("\\**\n.FS\n",file);
	doit = 1;
    }
    (this->bottomview)->Print(file,
		processor,finalformat,toplevel);
    if (doit)
	fputs(".FE\n",file);
}

