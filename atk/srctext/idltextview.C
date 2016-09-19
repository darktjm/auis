/* File idltextview.C created by R S Kemmetmueller
   (c) Copyright IBM Corp.  1988-1995.  All rights reserved.

   idltextview, a view for editing SOM IDL code. */

#include <andrewos.h>

static UNUSED const char ibmid[] = "(c) Copyright IBM Corp.  1988-1995.  All rights reserved.";

#include <bind.H>
#include <keymap.H>
#include <keystate.H>
#include <menulist.H>

#include "idltextview.H"

static keymap *idl_Map;
static menulist *idl_Menus;

static struct bind_Description idltextBindings[]={
    NULL
};


ATKdefineRegistry(idltextview, cpptextview, idltextview::InitializeClass);


boolean idltextview::InitializeClass()
{
    idl_Menus = new menulist;
    idl_Map = new keymap;
    bind::BindList(idltextBindings,idl_Map,idl_Menus,ATK::LoadClass("srctextview"));
    return TRUE;
}

idltextview::idltextview()
{
    ATKinit;

    this->idl_state = keystate::Create(this, idl_Map);
    this->idl_menus = (idl_Menus)->DuplicateML(this);
    SetBorder(5,5);
    THROWONFAILURE(TRUE);
}

idltextview::~idltextview()
{
    ATKinit;

    delete this->idl_state;
    delete this->idl_menus;
}

void idltextview::PostMenus(menulist *menulist)
{
    (this->idl_menus)->ChainBeforeML(menulist, (long)this);
    (this)->cpptextview::PostMenus(this->idl_menus);
}

keystate *idltextview::PrependKeyState()
{
    this->idl_state->next= NULL;
    return (this->idl_state)->AddBefore((this)->cpptextview::PrependKeyState());
}
