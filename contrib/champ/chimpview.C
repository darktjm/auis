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
#include "scroll.H"
#include "chlistview.H"
#include "enode.H"
#include "enodeview.H"
#include "chimp.H"
#include "chimpview.H"


ATKdefineRegistry(chimpview, lpair, NULL);

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
