/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

/* bind.c
 */

/*  */


#include <andrewos.h>
ATK_IMPL("bind.H")
#include <keymap.H>
#include <menulist.H>
#include <proctable.H>
#include <bind.H>


ATKdefineRegistryNoInit(bind, ATK);

void bind::BindList(const struct bind_Description  *bl, class keymap  *km, class menulist  *ml, const struct ATKregistryEntry   *type)
{
    while(bl && (bl->procName || bl->keyVector || bl->menuEntry)) {
	struct proctable_Entry *pe;

	if(bl->procName)
	    pe = proctable::DefineProc(bl->procName,  bl->proc, type, bl->module, bl->doc);
	else
	    pe = NULL;

	if(km && bl->keyVector)
	    (km)->BindToKey( bl->keyVector, pe, bl->keyRock);
	if(ml && bl->menuEntry)
	    (ml)->AddToML( bl->menuEntry, pe, bl->menuRock, bl->menuMask);

	bl++;
    }
}


void bind::BindList(const struct bind_ActionDescription  *bl, class keymap  *km, class menulist  *ml, const struct ATKregistryEntry   *type)
{
    while(bl && (bl->procName || bl->keyVector || bl->menuEntry)) {
	struct proctable_Entry *pe;

	if(bl->procName)
	    pe = proctable::DefineAction(bl->procName,  bl->act, type, bl->module, bl->doc);
	else
	    pe = NULL;

	if(km && bl->keyVector)
	    (km)->BindToKey( bl->keyVector, pe, (long)bl->keyRock);
	if(ml && bl->menuEntry)
	    (ml)->AddToML( bl->menuEntry, pe, (long)bl->menuRock, bl->menuMask);

	bl++;
    }
}
