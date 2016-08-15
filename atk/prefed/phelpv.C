/* Copyright 1992 Carnegie Mellon University. All rights Reserved.
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

/* $Header: /afs/cs.cmu.edu/project/atk-src-C++/atk/prefed/RCS/phelpv.C,v 1.4 1994/11/30 20:42:06 rr2b Stab74 $ */

#include <andrewos.h>

#ifndef NORCSID
static UNUSED const char *rcsid_phelpv_c = "$Header: /afs/cs.cmu.edu/project/atk-src-C++/atk/prefed/RCS/phelpv.C,v 1.4 1994/11/30 20:42:06 rr2b Stab74 $";
#endif 

ATK_IMPL("phelpv.H")


#include <message.H>
#include <text.H>
#include <list.H>
#include <environment.H>
#include <stylesheet.H>
#include <style.H>
#include <prefs.H>
#include <strcache.H>
#define SAVESTR(str) (str?strcache::SaveStr(str):NULL)
#include "phelpv.H"


#define TEXT(s) ((class text *)(s)->GetDataObject())
#define PREFS(s) ((s)->GetPrefs())


ATKdefineRegistry(phelpv, textview, phelpv::InitializeClass);
#ifndef NORCSID
#endif 
static boolean WarpToGroupHelp(struct prefgroup  *pg, struct fghrock  *rock);
static void DoGroupHelp(class phelpv  *self);


boolean phelpv::InitializeClass()
{
    return TRUE;
}

phelpv::phelpv()
{
	ATKinit;

    THROWONFAILURE( TRUE);
}

phelpv::~phelpv()
{
	ATKinit;

}

struct fghrock {
    class phelpv *self;
    char *name;
};

static boolean WarpToGroupHelp(struct prefgroup  *pg, struct fghrock  *rock)
{
    if(rock->name==pg->name) {
	class phelpv *self=rock->self;
	if(pg->grouphelp==-1) {
	    message::DisplayString(self, 0, "No help on this preference category.");
	    return FALSE;
	}
	(self)->textview::SetTopPosition( pg->grouphelp);
	(self)->textview::SetDotPosition( pg->grouphelp);
	(self)->textview::SetDotLength( 0);
    }
    return TRUE;
}

static void DoGroupHelp(class phelpv  *self)
{
    class text *txt=TEXT(self);
    class environment *renv = txt->rootEnvironment;
    class environment *env = (class environment *)(renv)->GetInnerMost( (self)->GetDotPosition());
    if(env != NULL && env->type==environment_Style) {
	class stylesheet *ss=(TEXT(self))->GetStyleSheet();
	class style *s=(ss)->Find( "groupname");
	if(s==env->data.style) {
	    struct fghrock rock;
	    char buf[1024];
	    long pos=(env)->Eval();
	    long len=(env)->GetLength();

	    if(len>sizeof(buf)-1) return;
	    (TEXT(self))->CopySubString( pos, len, buf, FALSE);
	    rock.self=self;
	    rock.name=SAVESTR(buf);
	    (PREFS(self)->categories)->Enumerate( (list_efptr)WarpToGroupHelp, (char *)&rock);
	}

    }
}
void phelpv::SetDotPosition(long  pos)
{
    (this)->textview::SetDotPosition( pos);
    DoGroupHelp(this);
}

class view *phelpv::Hit(enum view_MouseAction  action, long  x , long  y , long  clicks)
{
    class view *result=(this)->textview::Hit( action, x, y, clicks);
    if(result==(class view *)this && action==view_LeftUp || action==view_RightUp) {
	long pos=(this)->GetDotPosition();
	long len=(this)->GetDotLength();
	(this)->SetDotPosition( pos);
	(this)->SetDotLength( len);
    }
    return result;
}
