/* 
	htmlform.C - keep track of the nodes in an html <FORM>

	Copyright Carnegie Mellon, 1996 - All Rights Reserved
	For full copyright information see: andrew/config/COPYRITE

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

/* 	
	Stores an array of wdgnode-s refering 
		to the widgets allocated for a <FORM>
	Indices 0...GetN()-1 access the nodes.
*/
 
#include <andrewos.h>
ATK_IMPL("htmlform.H")

#include <util.h>

#include <awidget.H>
#include <attlist.H>
#include <atom.H>
#include <aaction.H>

#include <htmlform.H>
#include <htmltext.H>
#include <webview.H>
#include <webcom.H>
#include <web.H>
#include <alist.H>
#include <aoption.H>
#include <astring.H>
#include <image.H>
#include <fontdesc.H>

ATKdefineRegistryNoInit(htmlform, ATK);

// for callbacks from widgets
DEFINE_AACTION_FUNC_CLASS(formCB, AWidget);


// slot names (of form slot_xyz)
ASlot_NAME(designFont);
ASlot_NAME(labelActivateCallback);
ASlot_NAME(labelData);
ASlot_NAME(labelViewClass);
ASlot_NAME(hitX);
ASlot_NAME(hitY);
ASlot_NAME(align);
ASlot_NAME(borderShape);
ASlot_NAME(cols);
ASlot_NAME(contents);
ASlot_NAME(source);
ASlot_NAME(columns);
ASlot_NAME(text);
ASlot_NAME(value);
ASlot_NAME(activateCallback);
ASlot_NAME(scaleWidth);
ASlot_NAME(image);
ASlot_NAME(initiallyselected);
ASlot_NAME(initval);
ASlot_NAME(maxlen);
ASlot_NAME(selectionPolicy);
ASlot_NAME(parentform);
ASlot_NAME(pixheight);
ASlot_NAME(pixwidth);
ASlot_NAME(rows);
ASlot_NAME(set);	// was 'selected'
ASlot_NAME(sibling);
ASlot_NAME(optionValues);

ATK_CLASS(htmlform);	
ATK_CLASS(text);	
ATK_CLASS(AWidget);

struct tagTableType {
	enum htmlformTag tag;
	char *name;	
} tagTable [] = {
	{htmlform_InText, "text"},		// default is first in table
	{htmlform_InImage, "image"},
	{htmlform_InPassword, "password"},
	{htmlform_InHidden, "hidden"},
	{htmlform_InRadio, "radio"},
	{htmlform_InCheckbox, "checkbox"},
	{htmlform_InSubmit, "submit"},
	{htmlform_InReset, "reset"},
	{htmlform_InText, NULL}		// default is Text
};

	static void
SetInt(AWidget *wdg, const atom *slotat, attlist *atts, char *attr, int val) {
	struct htmlatts *intat = atts->GetAttribute(attr);
	if (intat && isdigit(*intat->value))
		val = atoi(intat->value);
	wdg->Set(slotat, (long)val);
}


// link into sibling list
// if this is CHECKED, clear selected attr of all others
	static void
SetupRadioButton(htmlform *self, AWidget *w, attlist *atts) {
	struct htmlatts *wname, *chatt;
	int fx, maxfx = self->fields.GetN();
	AWidget *firstsibling, *prevsibling;
	char *sibname;

	// if has CHECKED attribute, set slot_set
	chatt = atts->GetAttribute("checked");
	w->Set(slot_set, (abool)chatt);
        
	wname = atts->GetAttribute("name");
	if ( ! wname) {
		// XXX ERROR.  RadioButton without name
		w->Set(slot_sibling, w);
		return;
	}
	// find first sibling radio button
	for (fx = 0; fx < maxfx; fx++) {
		if (self->fields[fx].tag != htmlform_InRadio) 
			continue;
		sibname = self->GetAtt(fx, "name");
		if (sibname 
				&& cistrcmp(wname->value, sibname) == 0)
			break;
	}
	if (fx >= maxfx) {
		w->Set(slot_sibling, w);
		return;
	}
	firstsibling = self->fields[fx].wgt;

	// go around sibling ring to get predecessor
	//   and if w is CHECKED, 
	//		clear selected bit in all others
	prevsibling = firstsibling;
	while (TRUE) {
		if (chatt) 
			prevsibling->Set(slot_set, (abool)FALSE);
		AWidget *tsibling = (AWidget *)
				prevsibling->GetATK(slot_sibling, 
						class_AWidget);
		if ( ! tsibling) {
			// XXX error non-circular sibling list
			return;
		}
		if (tsibling == firstsibling) break;
		prevsibling = tsibling;
	}

	//  link w into ring of siblings
	w->Set(slot_sibling, firstsibling);
	prevsibling->Set(slot_sibling, w);
}

//====================================
//  CALLBACKS
//====================================


        ATK_CLASS(webview);
//CBSubmission
static void CBSubmissionFunc (AWidget *self, 
			const avalueflex &aux,  
			const avalueflex &in, 
			avalueflex &out) {
	htmlform *form = (htmlform *)
          self->GetATK(slot_parentform, class_htmlform);
        view *vself=(view *)in[0].ATKObject();
        while(vself) {
            if(vself->IsType(class_webview)) break;
            vself=vself->parent;
        }
        webview *wv=(webview *)vself;
        if(wv==NULL || form==NULL) {
            fprintf(stderr, "web: form submission can only be done from within a form displayed in a webview.\n");
            return;
        }
        web *wd=(web *)wv->GetDataObject();
        char *err=form->Submission(wd, wv);
        if(err) {
            fprintf(stderr, "web: form submission failed with error: %s\n", err);
        }
}
static formCB CBSubmission(&CBSubmissionFunc, 0L);

//CBReset
static void CBResetFunc (AWidget *self, 
			const avalueflex &aux,  
			const avalueflex &in, 
			avalueflex &out) {
	
	int fx;
	struct htmlatts *attx;
	char *cval;
        text *textarea, *ttxt;

	htmlform *form = (htmlform *)
			self->GetATK(slot_parentform, class_htmlform);
	if ( ! form) {
		// ERROR: submission from object not within a form
		return;
	}

	for (fx = 0; fx < form->fields.GetN(); fx ++) {
		// reset fx'th field
		attlist *atts = form->fields[fx].atts;
		AWidget *wgt = form->fields[fx].wgt;
		switch (form->fields[fx].tag) {

		case htmlform_InPassword:
		case htmlform_InText:
			// set text to atts->VALUE
			attx = atts->GetAttribute("value");
			cval = (attx) ? attx->value : (char *)"";

			ttxt = (text *)
                          wgt->GetATK(slot_source, class_text);
			if ( ! ttxt) break;
			ttxt->Clear();
			ttxt->AlwaysInsertCharacters(0, cval, strlen(cval));
                        ttxt->NotifyObservers( observable_OBJECTCHANGED);
			break;

		case htmlform_InCheckbox:
		case htmlform_InRadio:
			// reset selected from initiallyselected
			;
			wgt->Set(slot_set,
					(abool)wgt->Get(slot_initiallyselected, (abool)FALSE));
			break;

		case htmlform_Select:
		case htmlform_TextArea:
			// reset slot_source per 
			//	form->fields[fx].initialvalue
			textarea = (text *)
				wgt->GetATK(slot_source, class_text);
			if ( ! textarea) break;
			ttxt = form->fields[fx].initialvalue;
			if ( ! ttxt) break;
			textarea->Clear();
			textarea->AlwaysCopyText(0, ttxt, 0, ttxt->GetLength());
                        textarea->NotifyObservers( observable_OBJECTCHANGED);
			break;

		case htmlform_InReset:
		case htmlform_InSubmit:
		case htmlform_InHidden:
		case htmlform_InImage:
			// nothing to reset
			break;
		}
	}

}
static formCB CBReset(&CBResetFunc, 0L);

//CBMapClick
static void CBMapClickFunc (AWidget *self, 
			const avalueflex &aux,  
			const avalueflex &in, 
			avalueflex &out) {
	htmlform *form = (htmlform *)
          self->GetATK(slot_parentform, class_htmlform);
	if ( ! form) {
		// ERROR: mapclick from object outside form
		return;
	}
        view *vself=(view *)in[0].ATKObject();
        while(vself) {
            if(vself->IsType(class_webview)) break;
            vself=vself->parent;
        }
        webview *wv=(webview *)vself;
        if(wv==NULL || form==NULL) {
            fprintf(stderr, "web: form submission can only be done from within a form displayed in a webview.\n");
            return;
        }
        web *wd=(web *)wv->GetDataObject();
	long x = in[1].Integer();
	long y = in[2].Integer();
        self->Set(slot_hitX, x);
        self->Set(slot_hitY, y);
        char *err=form->Submission(wd, wv);
        if(err) {
            fprintf(stderr, "web: form submission failed with error: %s\n", err);
        }
        
}
static formCB CBMapClick(&CBMapClickFunc, 0L);


//====================================
//  CONSTRUCTORS / DESTRUCTORS
//====================================


htmlform::htmlform()  {
    containingtext = NULL;
    atts=NULL;
    THROWONFAILURE(TRUE);
}

htmlform::htmlform(htmltext *container, attlist *a)  {
    containingtext = container;
    a->Reference();
    atts=a;
    THROWONFAILURE(TRUE);
}

htmlform::~htmlform()  {
	// Destroy widgets and atts
	int i, n = fields.GetN();
	for (i = 0;  i < n;  i++) {
		struct wdgnode *wn = &fields[i];
		if (wn->wgt)  wn->wgt->Destroy();
		if (wn->atts)  wn->atts->Destroy();
        }
        atts->Destroy();
}


// adds a field to the form
	struct wdgnode *
htmlform::AddNode(AWidget *f, attlist *a,
			enum htmlformTag t) {
	struct wdgnode *wn = fields.Append();
	wn->wgt = f;
	wn->atts = a;
	a->Reference();
	wn->tag = t;
	f->Set(slot_parentform, this);
	return wn;
}

// create and add node for <INPUT ...>
	AWidget *
htmlform::AddInputNode(attlist *atts) {
	AWidget *w;
	long cols;
	struct htmlatts *attx;
	char *cval;
	text *wtext;
	ASlot *tslot;

	// get type field from atts
	attx = atts->GetAttribute("type");

	// find type in table 
	struct tagTableType *tx = tagTable;  // pts to default
	if (attx) for (tx = tagTable; tx->name; tx++)
		if (cistrcmp(tx->name, attx->value) == 0) break;
	// now tx pts to tagTable entry

	switch(tx->tag) {

	case htmlform_InText:
		w = AWidget::NewWidget("TextResponse");
		w->Set(slot_rows, 1L);			// rows
		SetInt(w, slot_columns, atts, "size", 20);
		attx = atts->GetAttribute("value");
		cval = (attx) ? attx->value : (char *)"";
		if (*cval) {
			// put initial value into widget text
			wtext = (text *)
				w->GetATK(slot_source, class_text);
			if (wtext)
                        wtext->AlwaysInsertCharacters(0, cval,
							strlen(cval));
		}
		break;

	case htmlform_InImage:
		w = AWidget::NewWidget("ImageMap");
		attx = atts->GetAttribute("align");
		if (attx) 
			w->Set(slot_align, NewString(attx->value));
		w->Set(slot_labelActivateCallback, &CBMapClick);

		// get URL - initiate fetch
		attx = atts->GetAttribute("src");
		if (attx) {
                    if(containingtext) w->Set(slot_labelData, containingtext->GetImage(attx->value, atts));
                    w->Set(slot_labelViewClass, "htmlimagev");
		}

		// xxx ignore these attributes:  border  vspace  hspace

		break;

	case htmlform_InPassword:
		w = AWidget::NewWidget("Password");
		SetInt(w, slot_columns, atts, "size", 20);
		attx = atts->GetAttribute("value");
		cval = (attx) ? attx->value : (char *)"";
		if (*cval) {
			// put initial value into widget text
			wtext = (text *)
				w->GetATK(slot_source, class_text);
			if (wtext)
				wtext->AlwaysInsertCharacters(0, cval,
							strlen(cval));
		}
		break;

	case htmlform_InHidden:
		w = AWidget::NewWidget("Widget");
		w->Set(slot_scaleWidth, 0.0);
                w->Set(slot_borderShape, 0);
                w->Set(slot_designFont, fontdesc::Create("andy", fontdesc_Plain, 12));
		break;

	case htmlform_InRadio:
		w = AWidget::NewWidget("RadioButton");
		SetupRadioButton(this, w, atts);
		break;

	case htmlform_InCheckbox:
		w = AWidget::NewWidget("CheckButton");
		attx = atts->GetAttribute("checked");
		w->Set(slot_set, (abool)attx);
		break;

	case htmlform_InSubmit:
		w = AWidget::NewWidget("PushButton");
		attx = atts->GetAttribute("value");
		cval = (attx) ? attx->value : (char *)"Submit";
		w->Set(slot_text, cval);
		w->Set(slot_activateCallback, &CBSubmission);
		break;

	case htmlform_InReset:
		w = AWidget::NewWidget("PushButton");
		attx = atts->GetAttribute("value");
		cval = (attx) ? attx->value : (char *)"Reset";
		w->Set(slot_text, cval);
		w->Set(slot_activateCallback, &CBReset);
		break;

	}
	AddNode(w, atts, tx->tag);
	return w;
}

// create and add node for <SELECT>...</>
	AWidget *
htmlform::AddSelectNode(attlist *atts, htmltext **obj) {
	AWidget *w;
	long size = 1;
	struct htmlatts *tatt;
        htmltext *ttext;

	tatt = atts->GetAttribute("size");
	if (tatt && isdigit(*tatt->value))
		size = atoi(tatt->value);

	tatt = atts->GetAttribute("multiple");
	if (size == 1 && tatt==NULL) 
 		w = AWidget::NewWidget("OptionMenu"); 
	else {
 		w = AWidget::NewWidget("List"); 
                w->Set(slot_rows, size);
                if (tatt)
                    w->Set(slot_selectionPolicy, "E");
                else w->Set(slot_selectionPolicy, "B");
	}

	//	set w->slot_contents = options
	//	set wn->initialvalue = options
	//	set wn->optionattrs = attrs


        w->Set(slot_source, ttext = new htmltext);

        *obj=ttext;
        
	struct wdgnode *wn = AddNode(w, atts, htmlform_Select);
	wn->optionattrs = new htmltext;
	wn->initialvalue = new htmltext;

	return w;
}

// create and add node for <TEXTAREA>...</>
	AWidget *
htmlform::AddTextareaNode(attlist *atts, htmltext **obj) {
	AWidget *w = AWidget::NewWidget("TextResponse"); 
	struct wdgnode *wn 
			= AddNode(w, atts, htmlform_TextArea);

	SetInt(w, slot_rows, atts, "rows", 3);
	SetInt(w, slot_cols, atts, "cols", 45);
        wn->initialvalue = new htmltext;

        w->Set(slot_source, *obj = new htmltext);

	return w;
}

void htmlform::FinishNode() {
    if(fields.GetN()==0) return;
    text *txt=fields[fields.GetN()-1].initialvalue;
    text *src=(text *)fields[fields.GetN()-1].wgt->GetATK(slot_source, class_text);
    if(txt && src) txt->AlwaysCopyText(0, src, 0,
				src->GetLength());
}

        ATK_CLASS(AList);
        ATK_CLASS(AOptionMenu);
void htmlform::FinishSelectNode() {
    FinishNode();
    if(fields.GetN()==0) return;
    AWidget *wgt=fields[fields.GetN()-1].wgt;
    if(wgt->IsType(class_AList)) {
        AList *al=(AList *)fields[fields.GetN()-1].wgt;
        long items=al->ItemCount();
        if(items>al->VisibleItemCount()) al->SetScrolled(TRUE);
    }
}
void htmlform::ApplyOptionAttrs(attlist *atts) {
    if(fields.GetN()==0) return;
    AWidget *wgt=fields[fields.GetN()-1].wgt;
    if(wgt->IsType(class_AList)) {
        AList *al=(AList *)fields[fields.GetN()-1].wgt;
        htmlatts *att=atts->GetAttribute("selected");
        if(att) al->Select(al->ItemCount());
        att=atts->GetAttribute("value");
        const char **values=(const char **)al->Get(slot_optionValues, NULLVOID);
        if(values==NULL) values=(const char **)malloc(al->ItemCount()*sizeof(const char *));
        else values=(const char **)realloc(values, al->ItemCount()*sizeof(const char *));
        al->Set(slot_optionValues, values);
        if(att && att->value) {
            values[al->ItemCount()-1]=NewString(att->value);
        } else values[al->ItemCount()-1]=NULL;
    } else if(wgt->IsType(class_AOptionMenu)) {
        AOptionMenu *am=(AOptionMenu *)fields[fields.GetN()-1].wgt;
        htmlatts *att=atts->GetAttribute("selected");
        if(att) am->SetSelected(am->ItemCount());
        att=atts->GetAttribute("value");
        const char **values=(const char **)am->Get(slot_optionValues, NULLVOID);
        if(values==NULL) values=(const char **)malloc(am->ItemCount()*sizeof(const char *));
        else values=(const char **)realloc(values, am->ItemCount()*sizeof(const char *));
        am->AWidget::Set(slot_optionValues, values);
        if(att && att->value) {
            values[am->ItemCount()-1]=NewString(att->value);
        } else values[am->ItemCount()-1]=NULL;
    }
}

// no more fields
//	set initiallyselected for radio buttons
//	set activateCallBack for a single InText or InPassword field 
//		if there are no other text fields
	void 
htmlform::Completed() {
	int i;
	int nother = 0;
	struct wdgnode *firstintext = NULL;

	for (i = 0; i < fields.GetN(); i ++)  {
		struct wdgnode *wn = &fields[i];
		switch(wn->tag) {
		case htmlform_InPassword:
		case htmlform_InText:
			if ( ! firstintext)
				firstintext = wn;
                        else nother++;
			break;
		case htmlform_InCheckbox:
		case htmlform_InRadio:
			// these don't count against having just 1 text
			// if selected, make it initiallyselected
			if (wn->wgt->Get(slot_set, 
						(abool)FALSE))
				wn->wgt->Set(slot_initiallyselected,
						(abool)TRUE);
			// DROP THRU
		default:
                    ;
                }
	}
	if (firstintext && nother==0)
		firstintext->wgt->Set(slot_activateCallback, 
					&CBSubmission);
}

static long EncodeSelect(wdgnode *wn, const char *name, flex &query, boolean separator) {
    if(wn->wgt->IsType(class_AList)) {
        AList *al=(AList *)wn->wgt;
        const char **values=(const char **)al->Get(slot_optionValues, NULLVOID);
        long selcount=al->SelectedItemCount();
        if(selcount==0) return FALSE;

        if(separator) {
            *query.Append()='&';
        }
        const AListSelectedItem *selections=al->SelectedItems();
        long i;
        for(i=0;i<selcount;i++) {
            const char *value;
            if(values && values[selections[i].ind-1]) value=values[selections[i].ind-1];
            else value=al->Item(selections[i].ind);
            webcom::EncodeForURL(query, name);
            *query.Append()='=';
            webcom::EncodeForURL(query, value);
            if(i!=selcount-1) *query.Append()='&';
        }
    } else  if(wn->wgt->IsType(class_AOptionMenu)) {
        AOptionMenu *am=(AOptionMenu *)wn->wgt;
        long selected=am->Selected();
        
        if(separator) {
            *query.Append()='&';
        }

        webcom::EncodeForURL(query, name);
        *query.Append()='=';
        webcom::EncodeForURL(query, am->Item(selected));
    }
    return TRUE;
}

// constructs the message for a form
char *htmlform::Submission(web *wd, webview *wv) {
    size_t i;
    webcom *wp=wd->getwebcom();
    if(wp==NULL) {
        return "No webcom available.";
    }
    char *act=wp->GetURL();
    char *method="GET";
    char *enctype="application/x-www-form-urlencoded";
    htmlatts *acta=atts->GetAttribute("action");
    htmlatts *methoda=atts->GetAttribute("method");
    htmlatts *enctypea=atts->GetAttribute("enctype");
    if(acta) act=acta->value;
    if(methoda) method=methoda->value;
    if(enctypea) enctype=enctypea->value;
    printf("act:%s\n", act);
    printf("method:%s\n", method);
    printf("enctype:%s\n", enctype);
    boolean get=FALSE;
    boolean post=FALSE;
    if(FOLDEDEQ(method, "get")) {
        get=TRUE;
    }
    if(FOLDEDEQ(method, "post")) {
        post=TRUE;
    }
    if(!FOLDEDEQ(enctype, "application/x-www-form-urlencoded")) {
        return "Unknown encoding type.";
    }
    flex query;
    if(get) {
        char *p=query.Insert(0, strlen(act));
        strcpy(p, act);
        *query.Append()='?';
    }
    boolean separator=FALSE;
    for(i=0;i<fields.GetN();i++) {
        wdgnode *wn=&fields[i];
        char *name=GetName(i);
        const char *value=GetValue(i);
        boolean set=TRUE;
        if(wn->tag==htmlform_Select) {
            if(EncodeSelect(wn, name, query, separator)) separator=TRUE;
        } else {
            switch(wn->tag) {
                case htmlform_InRadio:
                case htmlform_InCheckbox:
                    if(wn->wgt) {
                        ASlot *slot=wn->wgt->Get(slot_set);
                        // if it's an unset radio or checkbox skip it.
                        if(slot==NULL) continue;
                        if(!(abool)*(ASlotBool *)slot) continue;
                    } else continue;
                    break;
                case htmlform_InSubmit:
                case htmlform_InReset:
                    continue;
                case htmlform_InImage:
                    if(separator) {
                        *query.Append()='&';
                        separator=FALSE;
                    }
                    {
                    long x=wn->wgt->Get(slot_hitX, 0L);
                    long y=wn->wgt->Get(slot_hitY, 0L);
                    AString namexy(name);
                    AString valuexy;
                    namexy.Append(".x");
                    valuexy<<x;
                    webcom::EncodeForURL(query, (const char *)namexy);
                    *query.Append()='=';
                    webcom::EncodeForURL(query, (const char *)valuexy);
                    *query.Append()='&';
                    namexy=name;
                    namexy.Append(".y");
                    valuexy.Delete(0, valuexy.Length());
                    valuexy<<y;
                    webcom::EncodeForURL(query, (const char *)namexy);
                    *query.Append()='=';
                    webcom::EncodeForURL(query, (const char *)valuexy);
                    }
                    separator=TRUE;
                    continue;
                case htmlform_InText:
                case htmlform_InPassword:
                case htmlform_TextArea:
                    value=wn->wgt->Get(slot_value, value);
                    break;
                    break;
            }
            if(separator) {
                *query.Append()='&';
                separator=FALSE;
            }
            webcom::EncodeForURL(query, name);
            *query.Append()='=';
            webcom::EncodeForURL(query, value);
            separator=TRUE;
        }
    }
    *query.Append()='\0';
    size_t dummy;
    if(FOLDEDEQ(method, "get")) webview::VisitWeb(wv, query.GetBuf(0, query.GetN(), &dummy));
    else if(FOLDEDEQ(method, "post")) webview::VisitWeb(wv, act, WEBCOM_Post, query.GetBuf(0, query.GetN(), &dummy));
    else {
        return "Unsupported submission method.";
    }
    return NULL;
}

	static int
NextColon(text *t, int loc) {
	int tlen = t->GetLength();
	for ( ; loc < tlen; loc++)
		if (t->GetChar(loc) == ':')
			return loc;
	return tlen;
}

// write a line for each option/value pair
	void
WriteSelectOptions(FILE *file, text *options, text *values) {
	int optstart, optend, valstart, valend;
	   // xyzstart is index of first char of option or vlaue
	   // xyzend is index of next char after end
	int optslen = options->GetLength();

	optstart = 0;
	valstart = 0;
	optend = NextColon(options, optstart);
	valend = NextColon(values, valstart);
	if (optend > optstart) {
		// stuff prior to first <OPTION ... >
		options->WriteSubString(optstart, optend-optstart,
				file, FALSE);	// xxx should do quoting !!!
		fprintf(file, "\n");
	}
	optstart = optend + 1;
	valstart = valend + 1;

	while (optstart < optslen) {
		optend = NextColon(options, optstart);
		valend = NextColon(values, valstart);
		if (options->GetChar(optstart)=='*')
			optstart++;

		// output option line for opstart...optend-1
		fprintf(file, "<OPTION%s",
				(valend > valstart) ? " " : "");
		if (valend > valstart) {
			values->WriteSubString(valstart, valend-valstart,
				file, FALSE);	// xxx should do quoting !!!
		}
		fprintf(file, ">");

		options->WriteSubString(optstart, optend-optstart,
				file, FALSE);	// xxx should do quoting !!!
		fprintf(file, "\n");

		optstart = optend + 1;
		valstart = valend + 1;
	}
}

/* 
XXX we need WriteID for writing insets (see TextArea, below)
	other insets use text::WriteSubString, which skips the WriteID check
	however, this has the problem of multiply writing 
		things which might have been originally shared
*/

// send the proper html text for w to file
//
	void 
htmlform::WriteTaggedText(FILE *file, AWidget *w) {
	struct wdgnode *wn = FindNodeByWidget(w);
	htmltext **opts;

	switch (wn->tag) {
	case htmlform_InText:
	case htmlform_InImage:
	case htmlform_InPassword:
	case htmlform_InHidden:
	case htmlform_InRadio:
	case htmlform_InCheckbox:
	case htmlform_InSubmit:
	case htmlform_InReset:
		fprintf(file, "<INPUT %s>\n", 
				wn->atts->MakeIntoString());
		break;

	case htmlform_Select:
		fprintf(file, "<SELECT %s>\n", 
				wn->atts->MakeIntoString());
		WriteSelectOptions(file, wn->initialvalue, 
				wn->optionattrs);
		fprintf(file, "</SELECT>\n");
		break;
	case htmlform_TextArea:
		fprintf(file, "<TEXTAREA %s>\n", 
				wn->atts->MakeIntoString());
		wn->initialvalue->WriteSubString(0,
				wn->initialvalue->GetLength(),
				file, FALSE);
			// xxx actually need to use html quoting
		fprintf(file, "</TEXTAREA>\n");
		break;
	}
}


// finds the node with the given name;  returns index
	struct wdgnode *
htmlform::FindNodeByName(char *name) {
	int i, n = fields.GetN();
	for (i = 0;  i < n;  i++) {
		char *nm = GetName(i);
		if (nm && strcmp(nm, name) == 0)
			return &fields[i];
	}
	return NULL;
}

// finds the node having the given widget
// NULL for failure
	struct wdgnode *
htmlform::FindNodeByWidget(AWidget *w) {
	int i, n = fields.GetN();
	for (i = 0;  i < n;  i++) 
		if (fields[i].wgt == w)
			return &fields[i];
	return NULL;
}

/* gets the value of the attribute named id */
	char *
htmlform::GetAtt(int index, char *id)  {
	struct htmlatts *temp 
			= fields[index].atts->GetAttribute(id);
	return (temp) ? temp->value : NULL;
}


#ifndef NORCSID
#define NORCSID
	static char rcsid[]="$Header: /afs/cs.cmu.edu/project/atk-src-C++/atk/web/RCS/htmlform.C,v 1.17 1996/11/07 19:26:38 robr Exp $";
#endif
/*
 *    $Log: htmlform.C,v $
 * Revision 1.17  1996/11/07  19:26:38  robr
 * Implemented form imagemaps.
 * Fixed detection of when a text should have it's activatecallback set
 * to submit the form.  (should be done when there is only one text)
 * Fixed hidden form field to be better hidden :-)
 * BUG
 *
 * Revision 1.16  1996/11/04  03:11:05  robr
 * Fixed to use AOptioMenu when a single item is requested from a select
 * of size==1.
 * BUG
 *
 * Revision 1.15  1996/10/24  20:00:27  robr
 * Updated for the new widgets.
 * FEATURE
 *
 * Revision 1.14  1996/10/15  00:34:02  robr
 * Updated for the new widgets implemented suubmission of text.
 * FEATURE
 *
 * Revision 1.13  1996/09/27  19:19:07  robr
 * Added form submission via POST.
 * FEATURE
 *
 * Revision 1.12  1996/09/26  18:14:53  robr
 * First pass at form submission.  Radio & Check buttons work, hidden inputs should
 * work too...
 * Probably need to arrange to pull data out of the resources for the text entry,
 * option and select inputs.
 * Also, only the GET method is implemented.
 * FEATURE
 *
 * Revision 1.11  1996/09/26  16:56:30  robr
 * Added attlist argument to htmlform constructor,
 * the beginnings of the implementation of submission.
 * FEATURE
 *
 * Revision 1.10  1996/09/24  21:23:11  wjh
 * changed attribute name "selected" to "set"
 * adjust because attribute lists became reference counted (grr)
 *
 * Revision 1.9  1996/05/13  15:38:33  wjh
 * remove static from ASlot_NAME and ATK_CLASS remove borderwidth and hscale use borderShape and scaleWidth to vanish Hidden inset
 *
 * Revision 1.8  1996/05/02  02:02:40  wjh
 * revised to use new slot definitions
 * removed ATK_def
 * switch from new-AWidget to AWidget::NewWidget
 *
 * Revision 1.7  1996/04/12  01:21:19  wjh
 * cast
   . . .
 * Revision 1.1  1996/04/01  21:48:21  wjh
 * Initial revision
 * 
 */
