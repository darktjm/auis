/* Copyright 1995 Carnegie Mellon University All rights reserved.
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
ATK_IMPL("awidget.H");

#include <fontdesc.H>
#include <awidget.H>
#include <atom.H>
#include <aslot.H>
#include <astring.H>

ATKdefineRegistryNoInit(AWidget, AVarSet);

AWidget::AWidget() {
	ASlot::Init();
	SLOTINIT(units, 1.0);	// 100ths of a point
	SLOTINIT(designWidth, 1900);
	SLOTINIT(designHeight, 1900);
	SLOTINIT(designFont, NULLFONTDESC);
	SLOTINIT(scaleWidth, 1.0);
	SLOTINIT(scaleHeight, 1.0);
	SLOTINIT(resizable, FALSE);
	SLOTINIT(computeDesignSize, NULLACT);
	SLOTINIT(computeViewSize, NULLACT);
	SLOTINIT(sensitive, TRUE);

	TSLOT(priorViewFont);	// initially NULL
	TSLOTINIT(desiredWidth, 0);
	TSLOTINIT(desiredHeight, 0);

	SLOTINIT(mouseAny, NULLACT);
	SLOTINIT(mouseLeftDown, NULLACT);
	SLOTINIT(mouseLeftMove, NULLACT);
	SLOTINIT(mouseLeftUp, NULLACT);
	SLOTINIT(mouseRightDown, NULLACT);
	SLOTINIT(mouseRightMove, NULLACT);
	SLOTINIT(mouseRightUp, NULLACT);

	// printImage - (String or Function) 
	//	either a string giving postscript for the image
	//	or a function that generates and returns that string
	SLOTINIT(nominalPrintHeight, -1);
	SLOTINIT(nominalPrintWidth, -1);
}

ASlot_NAME(widgetDataClass);
ASlot_NAME(widgetViewClass);
// widgetDataClass gives the name of the dataobject to use for the widget.
// widgetViewClass gives the name of the view to use to display the widget.

static AWidget *MakeWidgetData(const char *dobjname) {
	ATKregistryEntry *reg=ATK::LoadClass(dobjname);
	if (!reg->IsType(&AWidget_ATKregistry_)) return NULL;
	return (AWidget *)ATK::NewObject(dobjname);
}

AWidget *AWidget::NewWidget(const char *widgettype) {
	char buf[MAXPATHLEN];
	buf[0]='A';
	strcpy(buf+1,widgettype);
	const char *dobjname = buf;
	AWidget *result=MakeWidgetData(buf);
	AVarSet *av=new AVarSet;
	if(av) {
		av->FetchInitialResources(atom_def(widgettype));
		// xxx should cache this avarset with all the resources.
		if(result==NULL) {
			dobjname=av->Get(slot_widgetDataClass,
						NULLSTR);
			if(dobjname==NULL) dobjname="AWidget";
			result=MakeWidgetData(dobjname);
			if(result==NULL && strcmp(dobjname, "AWidget") != 0) {
				fprintf(stderr, "AWidget: %s %s %s %s.\n",
						"couldn't create object", dobjname, 
						"for widget", widgettype);
				fprintf(stderr, "\tdefaulting to AWidget object.\n");
				dobjname="AWidget";
				result=MakeWidgetData(dobjname);
			}
		}
	}
	if(result) {
		av->CopySlots(*result, TRUE);
		result->SetResourceClass(atom_def(widgettype));
	}
	av->Destroy();
	if(result==NULL) {
		fprintf(stderr, "AWidget: %s %s %s %s.\n",
					"couldn't create object", dobjname, 
					"for widget", widgettype);
		return new AWidget;
	}
	return result;
}

extern ATKregistryEntry AWidgetView_ATKregistry_;
static AWidgetView *MakeWidgetView(const char *viewname) {
	ATKregistryEntry *reg=ATK::LoadClass(viewname);
	if(!reg->IsType(&AWidgetView_ATKregistry_)) return NULL;
	return (AWidgetView *)ATK::NewObject(viewname);
}

AWidgetView *AWidget::NewWidgetView(AWidget *dobj, 
				const char *widgettype) {
	const char *viewname=NULL;
	AWidgetView *result=NULL;
	if(dobj) {
		viewname=dobj->Get(slot_widgetViewClass,(char *)NULL);
		if(viewname) result=MakeWidgetView(viewname);
		if(result==NULL) {
			viewname=dobj->ViewName();
			if(viewname) result=MakeWidgetView(viewname);
		}
        }
        if(result) return result;
	if(widgettype==NULL) return NULL;
	if(result==NULL) {
		AString wt=widgettype;
		wt.Append("View");
		result=MakeWidgetView(wt);
	}
	if(result==NULL) {
		AString wt=widgettype;
		wt.Append("v");
		result=MakeWidgetView(wt);
	}
	if(result==NULL) {
		AString wt=widgettype;
		wt.Append("view");
		result=MakeWidgetView(wt);
	}
	if(result==NULL) {
		AString wt=widgettype;
		wt.Append("V");
		result=MakeWidgetView(wt);
	}
	if(result==NULL) {
		result=MakeWidgetView("AWidgetView");
	}
	return result;
}
