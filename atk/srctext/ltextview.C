/* File ltextview.C
   (c) Copyright IBM Corp.  1988-1995.  All rights reserved.

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

   ltextview, a view for editing Lisp code. */

#include <andrewos.h>

static UNUSED const char ibmid[] = "(c) Copyright IBM Corp.  1988-1995.  All rights reserved.";

#include <ctype.h>

#include <bind.H>
#include <keymap.H>
#include <keystate.H>
#include <menulist.H>

#include "ltext.H"
#include "ltextview.H"
#include "srctext.H"

static class keymap *L_Map;
static class menulist *L_Menus;

static struct bind_Description ltextBindings[]={
    {"srctextview-style-string","\"",'"'},
    {"srctextview-start-linecomment",";",';'},
    NULL
};

ATKdefineRegistry(ltextview, srctextview, ltextview::InitializeClass);

boolean ltextview::InitializeClass()
{
    L_Menus = new menulist;
    L_Map = new keymap;
    bind::BindList(ltextBindings, L_Map, L_Menus, ATK::LoadClass("srctextview"));
    return TRUE;
}

ltextview::ltextview()
{
    ATKinit;
    this->l_state = keystate::Create(this, L_Map);
    this->l_menus = (L_Menus)->DuplicateML(this);
    SetBorder(5,5);
    THROWONFAILURE(TRUE);
}

ltextview::~ltextview()
{
    ATKinit;
    delete this->l_state;
    delete this->l_menus;
}

class keystate *ltextview::PrependKeyState()
{
    this->l_state->next= NULL;
    return (this->l_state)->AddBefore((this)->srctextview::PrependKeyState());
}

void ltextview::PostMenus(menulist *menulist)
{
    (this->l_menus)->ChainBeforeML(menulist, (long)this);
    (this)->srctextview::PostMenus(this->l_menus);
}
