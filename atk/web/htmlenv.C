/* File htmlenv.C created by Ward Nelson
     (c) Copyright IBM Corp 1995.  All rights reserved. 

  htmlenv, an environment that can also store a list of HTML+ tag attributes */

#include <andrewos.h>
#include "attlist.H"
#include "htmlenv.H"

ATKdefineRegistry(htmlenv, environment, NULL);

htmlenv::htmlenv()
{
    this->attribs = new attlist;
    THROWONFAILURE(TRUE);
}

htmlenv::~htmlenv()
{
    attribs->ClearAttributes();
    attribs->Destroy();
}

htmlenv *htmlenv::GetTextRootEnvironment(text *txt)
{
    htmlenv *newenv;
    newenv = new htmlenv;
    newenv->nestedmark::length = 2;	/* Probably should be proper setting of includebeginning and ending */
#ifdef SMARTSTYLES
    newenv->environment::textobj=txt;
#endif
    return newenv;
}

void htmlenv::ClearAttribs()
{
    (this->attribs)->ClearAttributes();
}

void htmlenv::SetAttribs(attlist *atts)
{
    (this->attribs)->ClearAttributes();
    attlist::Copy(this->attribs, atts);
}

const char *htmlenv::GetAttribValue(const char *name)
{
    struct htmlatts *atts = (this->attribs)->GetAttribute(name);
    return (atts) ? atts->value : NULL;
}

nestedmark *htmlenv::NewButSimilar()
{
    htmlenv *sib=(htmlenv *)(this)->environment::NewButSimilar();
    sib->attribs = (this->attribs)->CopyAttributes();
    return sib;
}

/* override */
/*XXX- Wrap is overridden because its "hack" to keep view environments innermost is done badly, and destroys attributes of the view environment in the process.  When the base toolkit one is fixed, this can go away. */
environment *htmlenv::Wrap(long pos, long length, enum environmenttype type, union environmentcontents data)
{
    htmlenv *newenv, *nm1, *nm2, *cp;
    nm1 = (htmlenv *)(this)->GetInnerMost(pos);
    nm2 = (htmlenv *)(this)->GetInnerMost(pos + length - 1);
    cp = (htmlenv *)(nm1)->GetCommonParent(nm2);

    if (cp->environment::type == environment_View) {
	long oldpos, oldlen;
	union environmentcontents olddata;

	/* Hack to prevent non-view envs from get inside view envs; */
	/* removes the view env, adds new env, adds back view env. */

	oldpos = (cp)->Eval();
	oldlen = (cp)->GetLength();
	olddata = cp->environment::data;

#if 0
	/* Null out cp->data so environment_Delete leaves viewref alone! */
	cp->data.viewref = NULL;
	(cp)->Delete();
#endif

	if ((newenv = (htmlenv *)(this)->Add(pos, length)) != NULL) {
	    newenv->environment::type = type;
	    newenv->environment::data = data;
#ifdef SMARTSTYLES
	    newenv->environment::textobj = this->environment::textobj;
#endif
	    if (type == environment_View)
		(newenv)->SetStyle(FALSE, FALSE);
#ifdef SMARTSTYLES
	    else htmlenv_ReplaceStyle(newenv, data.style);
#endif
	}

#if 0
	cp = (this)->Add(oldpos, oldlen);
	if (cp != NULL) {
	    cp->type = environment_View;
	    cp->data = olddata;
	    (cp)->SetStyle(FALSE, FALSE);
	}
#else
	{
	    htmlenv *newcp= (htmlenv *)(this)->Add(oldpos, oldlen);
	    if (newcp) {
		newcp->environment::type = environment_View;
		newcp->environment::data = olddata;
		(newcp)->SetStyle(FALSE, FALSE);
		newcp->attribs = (cp->attribs)->CopyAttributes(); /*XXX- this is obviously not an appropriate fix for the base toolkit.  But if we used the NewButSimilar() method, what would we do with the returned pointer?  How do we get it into the tree in the right place?  Hmmm... */
	    }
	}
	/* Null out cp->data so environment_Delete leaves viewref alone! */
	cp->environment::data.viewref = NULL;
	(cp)->Delete();
#endif

    } else if ((newenv = (htmlenv *)(this)->Add(pos, length)) != NULL) {
	newenv->environment::type = type;
	newenv->environment::data = data;
#ifdef SMARTSTYLES
	newenv->environment::textobj = this->environment::textobj;
#endif
	if (type == environment_View)
	    (newenv)->SetStyle(FALSE, FALSE);
#ifdef SMARTSTYLES
	else htmlenv_ReplaceStyle(newenv, data.style);
#endif
    }
    return newenv;
}
