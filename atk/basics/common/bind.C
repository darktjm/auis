/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/config/COPYRITE'     *
\* ********************************************************************** */

/*
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

#include <andrewos.h>

#ifndef NORCSID
#define NORCSID
static UNUSED const char rcsid[]="$Header: /afs/cs.cmu.edu/project/atk-src-C++/atk/basics/common/RCS/bind.C,v 3.6 1995/03/30 01:19:12 rr2b Stab74 $";
#endif


 

/* bind.c
 */

/*  */


ATK_IMPL("bind.H")
#include <keymap.H>
#include <menulist.H>
#include <proctable.H>
#include <bind.H>


ATKdefineRegistryNoInit(bind, ATK);

void bind::BindList(struct bind_Description  *bl, class keymap  *km, class menulist  *ml, struct ATKregistryEntry   *type)
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


void bind::BindList(struct bind_ActionDescription  *bl, class keymap  *km, class menulist  *ml, struct ATKregistryEntry   *type)
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
