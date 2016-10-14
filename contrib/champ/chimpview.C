/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

#include <andrewos.h>
#include "scroll.H"
#include "chlistview.H"
#include "enode.H"
#include "enodeview.H"
#include "chimp.H"
#include "chimpview.H"


ATKdefineRegistryNoInit(chimpview, lpair);

chimpview::chimpview()
{
    class scroll *s = new scroll;

    this->env = new enodeview;
    this->lv = new chlistview;
    if (!this->lv || !s || !this->env) THROWONFAILURE((FALSE));
    (this->env)->SetChimpview( this);
    (s)->SetView( this->lv);
    (this)->SetUp( s, this->env, 50, lpair_PERCENTAGE, lpair_VERTICAL, TRUE);
    THROWONFAILURE((TRUE));
}

void chimpview::SetDataObject(class dataobject  *c)
{
    (this->lv)->SetDataObject( c);
    (this->env)->SetDataObject( ((class chimp *)c)->en);
}

void chimpview::LinkTree(class view *parent)
{
    this->lpair::LinkTree(parent);
    if(this->env) this->env->LinkTree(this);
    if(this->lv) this->lv->LinkTree(this);
}
