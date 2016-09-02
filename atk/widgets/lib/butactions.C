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
ATK_IMPL("butactions.H")
#include <butactions.H>

#include <point.h>
#include <rect.h>
#include <fontdesc.H>
#include <proctable.H>
#include <aaction.H>
#include <text.H>
#include <figure.H>
#include <figobj.H>
#include <figattr.H>
#include <figotext.H>
#include <figoins.H>
#include <astring.H>
#include <aslot.H>
#include <ashadow.H>
#include <awgtfigview.H>
#include <awgtfig.H>


ASlot_NAME(set);	// boolean
ASlot_NAME(sibling);	// AWidget *
ASlot_NAME(activateCB);  // aaction
ASlot_NAME(contents);	// any type
ASlot_NAME(designFont);  // long
ASlot_NAME(designHeight);  // long
ASlot_NAME(designWidth);  // long
ASlot_NAME(buttonType);  // enum buttonType
ASlot_NAME(priorViewFont);  // Font
ASlot_NAME(buttonString);  // String
ASlot_NAME(buttonX);  // long
ASlot_NAME(buttonY);  // long
ASlot_NAME(rows);  // long
ASlot_NAME(cols);  // long
 
ATK_CLASS(AWidget);
ATK_CLASS(AWidgetView);
ATK_CLASS(ASlotString);
ATK_CLASS(ASlotFigure);
ATK_CLASS(ASlotDataObject);
ATK_CLASS(text);
ATK_CLASS(figure);
ATK_CLASS(fontdesc);
ATK_CLASS(ASlotInt);
ATK_CLASS(ASlotReal);

AACTION_ARG_CLASS(AWgtFig);

enum buttonType {
	Empty_Button,
	String_Button,
	Text_Button,
	Figure_Button,
	DObj_Button
};


ATKdefineRegistry(butactions, ATK,
		butactions::InitializeClass);

	void 
butactions::Init() {
	ATKinit;  // calls InitializeClass if needed
}

/* ============================
PushButton
	Sizing and creating the image

	Will select:   Text, Figure, String, DObj, or Empty
======================================
	note that setting a designSize will not affect the actual size
	the image will be scaled to actual fontsize in 
			awidgetview::DesiredSize
	changing scale will affect size of widget
*/
	static void
StringButton(AWidget *wgt, AWgtFigView *parent, 
		const char *str) {
	wgt->Set(slot_buttonType, String_Button);
	wgt->Set(slot_buttonString, str);

	long width, height, asc, desc, bx, by; 	 // distance in units
	width = parent->StringWidth((char *)str);
	parent->CharVertical('X', &asc, NULL);
	parent->CharVertical('y', NULL, &desc);
	height = asc + desc;	

// xxx padding should be proportional. here using 2 points
#define PADSTR 2
	width += 2*parent->ToUnitX(PADSTR);
	height += 2*parent->ToUnitY(PADSTR);
	bx = parent->ToUnitX(PADSTR);			// horizontal offset
	by = asc - parent->ToUnitY(PADSTR);	// vertical offset
		// positive values of `by' move text downward on screen
#undef PADSTR

	if (parent->GetBorder()) {
		rectangle inner, outer;
		long pixwidth = parent->ToPixX(width),
			pixheight = parent->ToPixY(height);
		rectangle_SetRectSize(&inner, 0, 0, pixwidth, pixheight);
		parent->GetBorder()->ExteriorRect(&inner, &outer);
		width = parent->ToUnitX(rectangle_Width(&outer));
		height = parent->ToUnitY(rectangle_Height(&outer));
	}

	wgt->Set(slot_buttonX, bx);
	wgt->Set(slot_buttonY, by);
	wgt->Set(slot_designWidth, width);
	wgt->Set(slot_designHeight, height);
}

	static void
EmptyButton(AWidget *wgt, AWgtFigView *parent) {
	wgt->Set(slot_buttonType, Empty_Button);
	StringButton(wgt, parent, "   ");
}

	static void
FigureButton(AWidget *wgt, AWgtFigView *parent, figure *) {
	wgt->Set(slot_buttonType, Figure_Button);
	// XXX
	StringButton(wgt, parent, "Figure");
}

	static void
TextButton(AWidget *wgt, AWgtFigView *parent, text *txt) {
	wgt->Set(slot_buttonType, Text_Button);
	// XXX
	AString str = "";
	long i, n = txt->GetLength();
	for (i=0; i<n; i++)
		str.Append((char)txt->GetChar(i));
	StringButton(wgt, parent, (const char *)str);
}

	static void
DObjButton(AWidget *wgt, AWgtFigView *parent, 
			dataobject *) {
	wgt->Set(slot_buttonType, DObj_Button);
	// XXX
	StringButton(wgt, parent, "DObj");
}

AACTION_FUNC(buttonDesign, AWgtFig) {
	ASlot *contents = self->Get(slot_contents);
	AWgtFigView *parent 
			= (AWgtFigView*)in[0].ATKObject();

	if ( ! contents) 		// empty button
		EmptyButton(self, parent);
	else if (contents->IsType(class_ASlotString)) 
		StringButton(self, parent,
				(const char *)*contents);
	else if (contents->IsType(class_ASlotFigure)) 
		FigureButton(self, parent, 
				(figure *)*contents);
	else if (contents->IsType(class_ASlotDataObject)) {
		dataobject *dobj = (dataobject *)*contents;
		if (dobj->IsType(class_text))
			TextButton(self, parent, (text *)dobj);
		else if (dobj->IsType(class_figure)) 
			FigureButton(self, parent, 
				(figure *)dobj);
		else 	// display arbitrary data object
			DObjButton(self, parent, dobj);
	}
	else if (contents->IsType(class_ASlotInt)) 
		StringButton(self, parent, 
				AString::itoa((long)*contents));
	else if (contents->IsType(class_ASlotReal)) 
		StringButton(self, parent,
				 AString::dtoa((double)*contents));
	else 
		// empty button anyway
		EmptyButton(self, parent);
}

AACTION_FUNC(buttonPrep, AWgtFig) {
	fontdesc *fdesc;
	figotext *ftxt;
	figattr fatr;
	figure *fig = (figure *)in[1].ATKObject();
	AWgtFigView *view = (AWgtFigView *)in[0].ATKObject();
	switch (self->Get(slot_buttonType, Empty_Button)) {
	case Empty_Button:
		break;
	case String_Button:
		// insert the chosen string into figure
		fdesc = (fontdesc *)self->priorViewFont;
		// xxx need to use bounds in in[2]
		ftxt = figotext::Create(
				(char *)self->Get(slot_buttonString, ""), 
				view->ToFigX(self->Get(slot_buttonX, 0)),
				view->ToFigY(self->Get(slot_buttonY, 0)));
		fatr.SetFontFamily(fdesc->GetFontFamily());
		fatr.SetFontSize(fdesc->GetFontSize());
		fatr.SetFontStyle(fdesc->GetFontStyle());
		fatr.SetTextPos(figattr_PosLeft);
		ftxt->UpdateVAttributes(&fatr, (long) 
				(1<<figattr_FontFamily) 
				| (1<<figattr_FontSize) 
				| (1<<figattr_FontStyle)
				| (1<<figattr_TextPos));
		fig->AlwaysInsertObject(ftxt, 
				fig->RootObjRef(), // put in outer group 
				-1);	   // put in front of other contents
		break;
	case Text_Button:
		// XXX
		break;
	case Figure_Button:
		// XXX
		break;
	case DObj_Button:
		// XXX
		break;
	}
}

AACTION_FUNC(widgetArm, AWgtFig) {
	view *parent = (view*)in[0].ATKObject();
	boolean within = (boolean)in[3].Integer();
	if ( ! parent->IsType(class_AWidgetView))
		return;  // cannot do borders without awidgetview
	AWidgetView *awv = (AWidgetView *)parent;
	if ( ! awv->GetBorder())  return;  // no border
	awv->GetBorder()->SetArmed(within);
	awv->WantUpdate(awv);
}

AACTION_FUNC(buttonLeftUp, AWgtFig)  {
	boolean within = (boolean)in[3].Integer();
	if ( ! within) return;
	view *parent = (view*)in[0].ATKObject();

	// call activatecb, if any
	aaction *doit = self->Get(slot_activateCB, NULLACT);
	if (doit) {
		avalueflex dummy;
		(*doit)(self, avalueflex()|parent, dummy); // xxx, will this work?
	}

	// disarm
	if ( ! parent->IsType(class_AWidgetView))
		return;  // cannot do borders without awidgetview
	AWidgetView *awv = (AWidgetView *)parent;
	if ( ! awv->GetBorder())  return;  // no border
	awv->GetBorder()->SetArmed(FALSE);
	awv->WantUpdate(awv);
}


// function used for the assign to slot_set by checkPrep
//
AACTION_ARG_CLASS(ASlot);
AACTION_FUNC(wantupdatefunc, ASlot) {
	const avalue_u *src 	// get the source from `in'
			= (avalue_u *)in[0].VoidPtr(ASlot::in0type);
	long nowset = src->integer;
	if ((long)*self == nowset) 
		return;		// no change
	self->UseOnlyFromAnAssignFunction(src);

	((AWgtFig *)aux[0].ATKObject())->
		NotifyObservers(awidget_DATACHANGED); 
}

AACTION_FUNC(checkPrep, AWgtFig)  {
	// establish setit function for slot_set
	ASlot *chkslot = self->Get(slot_set);
	if (chkslot && ! chkslot->GetAssign()) {
		// XXX FREE THIS
		chkslot->SetAssign(
			new ASlotAssign(wantupdatefunc, self));
	}
}


AACTION_FUNC(checkLeftUp, AWgtFig)  {
	ASlot *chkslot = self->Get(slot_set);
	if ( ! chkslot) 
		self->Set(slot_set, (long)TRUE);
	else 
		(*chkslot) = (long)(((long) *chkslot) 
						? FALSE : TRUE);
}


AACTION_FUNC(radioLeftUp, AWgtFig)  {
	boolean within = (boolean)in[3].Integer();
	if ( ! within) return;
	if (self->Get(slot_set, (long)FALSE))
		return;	// this one is already selected

	// turn off checks in siblings
	AWidget *sib = self;
	while (TRUE) {
		// get next sib, if any
		sib = (AWidget *)sib->GetATK(slot_sibling, 
				class_AWidget);
		if ( ! sib) {
			// XXX ERROR mal-formed siblings list
			break;
		}
		if (sib == self)
			break;	// loop is done
		// turn off sib's check
		sib->Set(slot_set, (long)FALSE);
	}
	
	// turn on check for this button
	self->Set(slot_set, (long)TRUE);

	// disarm
	view *parent = (view*)in[0].ATKObject();
	if ( ! parent->IsType(class_AWidgetView))
		return;  // cannot do borders without awidgetview
	AWidgetView *awv = (AWidgetView *)parent;
	if ( ! awv->GetBorder())  return;  // no border
	awv->GetBorder()->SetArmed(FALSE);
	awv->WantUpdate(awv);
}



AACTION_FUNC(textresDesign, AWgtFig) {
	// figure out size to fit textview and border
	AWgtFigView *parent 
			= (AWgtFigView*)in[0].ATKObject();

	long width, height, asc, desc, bx, by;
	width = parent->StringWidth("X");
	parent->CharVertical('X', &asc, NULL);
	parent->CharVertical('y', NULL, &desc);
	height = asc + desc;
	width *= self->Get(slot_cols, 10);
	height *= self->Get(slot_rows, 1);

// xxx padding should be proportional
#define PADSTR 2 
	width += 2*PADSTR;	// space at ends of string
	height += 2*PADSTR;	// space above/below
	bx = PADSTR;			// horizontal offset
	by = -PADSTR-desc;	// vertical offset
#undef PADSTR

	if (parent->GetBorder()) {
		rectangle inner, outer;
		rectangle_SetRectSize(&inner, 0, 0, width, height);
		parent->GetBorder()->ExteriorRect(&inner, &outer);
		width = rectangle_Width(&outer);
		height = rectangle_Height(&outer);
		bx += (width-rectangle_Width(&inner))/2;
		by -= (height-rectangle_Height(&inner))/2;
	}

	self->Set(slot_buttonX, bx);
	self->Set(slot_buttonY, by);
	self->Set(slot_designWidth, width);
	self->Set(slot_designHeight, height);
}

AACTION_FUNC(textresPrep, AWgtFig) {
	// insert textview into figure
	AWgtFigView *parent = (AWgtFigView *)in[0].ATKObject();
	figure *fig =(figure *)in[1].ATKObject();
	rectangle *bounds =(rectangle *)in[2].VoidPtr();
	rectangle inner;
	parent->GetBorder()->InteriorRect(bounds, &inner);
	
	long top, left, width, height;
	rectangle_GetRectSize(&inner, &top, &left, &width, &height);

	top += self->Get(slot_buttonX, 0);
	left += self->Get(slot_buttonY, 0);

	figoins *finset = figoins::Create(top, left, width, height, "text");
	fig->AlwaysInsertObject(finset, 
			fig->RootObjRef(), // put in outer group 
			-1);	   // put in front of other contents
}

AACTION_FUNC(textresLeftUp, AWgtFig) {
	view *parent = (view*)in[0].ATKObject();
	boolean within = (boolean)in[3].Integer();
	if ( ! within) return;

	// give input focus to text within figure
	// xxx
	// xxx the 'focus' border is needed when any child has focus
	//  xxx  or is it any immedaite child ???

	// xxx have to grab the textview within the figure 
	// xxx		and give it input focus

	// disarm
	if ( ! parent->IsType(class_AWidgetView))
		return;  // cannot do borders without awidgetview
	AWidgetView *awv = (AWidgetView *)parent;
	if ( ! awv->GetBorder())  return;  // no border
	awv->GetBorder()->SetArmed(FALSE);
	awv->WantUpdate(awv);
}


#if 0   /* old stuff saved for how-to's */

AACTION_FUNC(wsLeftUp, AWgtFig)  {
	// do something about left button going up
	/* if there is no WidgetSelector_State slot,
		then the initial image is up
			replace it  with scrollinglist
			and set state to list up
		if state is list up and hit is in a list entry,
			setresourceclass and fetch initialresources
	*/	
}

/* old stuff saved for how-to's */

AACTION_FUNC(wsRightUp, AWgtFig)  {
// xxx needs to revise screenImage figure
	figure *fig = wv->GetCurrFig();
	figobj *fobj = fig->FindObjectByName("buttext");
	figattr fatr;
	fatr.SetColor("red");
	fobj->UpdateVAttributes(&fatr, 1<<figattr_Color);
}

/* old stuff saved for how-to's */

AACTION_FUNC(checkUpdate, AWgtFig)  {
	// set "bkgd" color depending on slot_set
	AWidget *self = (AWidget *)wv->GetDataObject();
	ASlot *chkslot = self->Get(slot_set);
	boolean isset 
		= (long)self->Get(slot_set, (long)FALSE);

	figure *fig = wv->GetCurrFig();
	if ( ! fig) return;
	figobj *fobj = fig->FindObjectByName("bkgd");
	if ( ! figobj) return;
	figattr fatr;
	fatr.SetColor((isset) ? "grey" : "tan");
	fobj->UpdateVAttributes(&fatr, 1<<figattr_Color);
}

/* old stuff saved for how-to's */

	static void
ReArm(AWidget *self, boolean selected) {
	// xxx we need a general mechanism for the view state
	// in the meantime, we test the color of the border (ugh)
	// LightGray - disselected;   DimGray - selected

	figure *fig = self->GetFigure(slot_screenImage, 
			class_figure);
	if ( ! fig) return;		// xxx error
	figobj *border = fig->FindObjectByName("dum-border");
	char *currcolor;
		// I don't understand this next line, 
		//	but this is what 'figure' does
	currcolor = ((border)->GetVAttributes())
		->GetColor((border)->GetIVAttributes());

	if (selected && strcmp(currcolor, "DimGray") == 0)
		return;
	if ( (!selected) && strcmp(currcolor, "LightGray") == 0)
		return;
	figattr fatr;
	fatr.SetColor((selected) ? "DimGray" : "LightGray");
	border->UpdateVAttributes(&fatr, 1L<<figattr_Color);
}

#endif /* old stuff saved for how-to's */

	boolean 
butactions::InitializeClass() {
	static avalueflex noaux;
	ENTER_AACTION_FUNC(buttonDesign,
		"butactions-button-design", 
		noaux, "Push button: Compute Design Size");
	ENTER_AACTION_FUNC(buttonPrep,
		"butactions-button-prep",
		noaux, "Prepare image for push button");
	ENTER_AACTION_FUNC(widgetArm,
		"butactions-widget-arm",
		noaux, "Arm widget while mouse is down inside");
	ENTER_AACTION_FUNC(buttonLeftUp,
		"butactions-button-left-up",
		noaux, "Raise left mouse on a button");

	ENTER_AACTION_FUNC(checkPrep,
		"butactions-check-prep",
		noaux, "Prepare image for selectable button");
	ENTER_AACTION_FUNC(checkLeftUp,
		"butactions-check-left-up",
		noaux, "Toggle check button (L)");
	ENTER_AACTION_FUNC(radioLeftUp,
		"butactions-radio-left-up",
		noaux, "Change radio buttons (L)");
	ENTER_AACTION_FUNC(textresDesign,
		"butactions-textres-design",
		noaux, "Compute size of TextResponse button");
	ENTER_AACTION_FUNC(textresPrep,
		"butactions-textres-prep",
		noaux, "Prepare image for TextResponse button");
	ENTER_AACTION_FUNC(textresLeftUp,
		"butactions-textres-left-up",
		noaux, "Left mouse up on TextResponse button");

	return TRUE;
}

/* $Log: butactions.C,v $
 * Revision 1.16  1996/09/29  14:48:23  robr
 * Fixed to pass the view as per the standard widget aaction arguments.
 * BUG
 *
 * Revision 1.15  1996/09/25  15:29:22  wjh
 * change attribute name `selected' to `set'
 * convert PADSTR from pixels to units
 * and similarly for other pixel measurements
 * use direct access to priorViewFont
 * add WantUpdate in widgetArm and ButtonLeftUp
 * check `within' in RadioLeftUp
 * add code in RadioLeftUp to set armed appropriatel;y and call WantUpdate
 * add WnatUpdate to textresLeftUp
 *
 * Revision 1.14  1996/07/06  02:52:55  wjh
 * fix a core dump (Get can now return NULL)
 *
 * Revision 1.13  1996/05/20  18:01:58  robr
 * cleaned up warnings.
 * BUG
 *
 * Revision 1.12  1996/05/20  15:35:51  robr
 * Missing return values.
 * BUG
 * .,
 *
 * Revision 1.11  1996/05/13  15:53:33  wjh
 *  adapt buttonDesign and buttonPrep for new scheme\n	use AWidgetView::StringWidth instead of currfont\ninstall preliminary TextResponse actions
 *
 * Revision 1.10  1996/05/07  20:39:30  wjh
 * interim checkin
 *
 */
