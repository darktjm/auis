/* 1995 Carnegie Mellon University All rights reserved.
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
ATK_IMPL("wsaction.H")

#include <proctable.H>
#include <aaction.H>
#include <figure.H>
#include <figobj.H>
#include <figattr.H>
#include <awidgetview.H>
#include <aslot.H>
#include <awidget.H>

#include <wsactions.H>

ATKdefineRegistry(wsactions, ATK,
		wsactions::InitializeClass);

// functions entered in proctable
static void WidgetSelector_LeftUp (
	AWidgetView *wv,
	const avalueflex &aux,
	const avalueflex &in,
	avalueflex &out
);
static void WidgetSelector_RightUp (
	AWidgetView *wv,
	const avalueflex &aux,
	const avalueflex &in,
	avalueflex &out
);

// aactions for the functions defined above
DEFINE_AACTION_FUNC_CLASS(WSactfunc,
			AWidgetView);
static avalueflex noaux;
static WSactfunc leftupfunc (&WidgetSelector_LeftUp,
			noaux);
static WSactfunc rightupfunc (&WidgetSelector_RightUp,
			noaux);

/* the following doesn't work because
	AWidgetView_ATKregistry_m is undefined
// definitions table for proctable_DefineActions
static proctable_ActionDescription actdescs[]={
	{"wsactions-mouse-left-up",
		&leftupfunc,
		&AWidgetView_ATKregistry_,
		"WidgetSelector - select", "wsactions"},
	{"wsactions-mouse-right-up",
		&righttupfunc,
		&AWidgetView_ATKregistry_,
		"WidgetSelector - mutate", "wsactions"},
	{NULL, NULL, NULL, NULL, NULL}
};
*/

	boolean 
wsactions::InitializeClass() {
	// proctable::DefineActions(actdescs);
	ATKregistryEntry *awvreg =
		ATK::LoadClass("AWidgetView");
	proctable::DefineAction(
		"wsactions-mouse-left-up", &leftupfunc,
		awvreg,
		"wsactions", "WidgetSelector - select");
	proctable::DefineAction(
		"wsactions-mouse-right-up", &rightupfunc,
		awvreg,
		"wsactions", "WidgetSelector - mutate");
	return TRUE;
}

	void 
wsactions::Init() {
	ATKinit;  // calls InitializeClass if needed
}

static void WidgetSelector_LeftUp (
	AWidgetView *wv,
	const avalueflex &aux,
	const avalueflex &in,
	avalueflex &out
) {
	// do something about left button going up
	/* if there is no WidgetSelector_State slot,
		then the initial image is up
			replace it  with scrollinglist
			and set state to list up
		if state is list up and hit is in a list entry,
			setresourceclass and fetchinitialresources
	*/	figure *fig = wv->GetCurrFig();
	figobj *fobj = fig->FindObjectByName("wstext");
	figattr fatr;
	fatr.SetColor("blue");
	fobj->UpdateVAttributes(&fatr, 1<<figattr_Color);
}

static void WidgetSelector_RightUp (
	AWidgetView *wv,
	const avalueflex &aux,
	const avalueflex &in,
	avalueflex &out
) {
	figure *fig = wv->GetCurrFig();
	figobj *fobj = fig->FindObjectByName("wstext");
	figattr fatr;
	fatr.SetColor("red");
	fobj->UpdateVAttributes(&fatr, 1<<figattr_Color);
}
