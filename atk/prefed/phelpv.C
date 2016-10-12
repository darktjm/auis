/* Copyright 1992 Carnegie Mellon University. All rights Reserved. */

#include <andrewos.h>
ATK_IMPL("phelpv.H")


#include <message.H>
#include <text.H>
#include <list.H>
#include <environment.H>
#include <stylesheet.H>
#include <style.H>
#include <prefs.H>
#include <strcache.H>
#define SaveStr strcache::SaveStr
#define SAVESTR(str) (str?SaveStr(str):NULL)
#include "phelpv.H"


#define TEXT(s) ((class text *)(s)->GetDataObject())
#define PREFS(s) ((s)->GetPrefs())


ATKdefineRegistry(phelpv, textview, phelpv::InitializeClass);
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
    const char *name;
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

	    if(len>(int)sizeof(buf)-1) return;
	    (TEXT(self))->CopySubString( pos, len, buf, FALSE);
	    rock.self=self;
	    rock.name=SaveStr(buf);
	    (PREFS(self)->categories)->Enumerate( (list_efptr)WarpToGroupHelp, (char *)&rock);
	}

    }
}
void phelpv::SetDotPosition(long  pos)
{
    (this)->textview::SetDotPosition( pos);
    DoGroupHelp(this);
}

class view *phelpv::Hit(enum view::MouseAction  action, long  x , long  y , long  clicks)
{
    class view *result=(this)->textview::Hit( action, x, y, clicks);
    /* tjm - unsure if this is the desired grouping (was (a&&b)||c) */
    if(result==(class view *)this && (action==view::LeftUp || action==view::RightUp)) {
	long pos=(this)->GetDotPosition();
	long len=(this)->GetDotLength();
	(this)->SetDotPosition( pos);
	(this)->SetDotLength( len);
    }
    return result;
}
