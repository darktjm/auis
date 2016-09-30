#ifndef _htmlform_H_
#define _htmlform_H_
/** \addtogroup libhtmltext
 * @{ */
/* 
    htmlform.H - an object for managing a form in an html file 

	Copyright Carnegie Mellon University, 1996 - All rights reserved. 
	For full copyright information see: andrew/doc/COPYRITE
*/

/* An htmlform stores the collection of widgets created 
	to implement a form.  Each widget is represented by
	a wdgnode, which is allocated within the htmlform
	and accessed via integer indices.
*/

class AWidget;
class attlist;
class htmltext;

enum htmlformTag {
	htmlform_InText, htmlform_InImage,
	htmlform_InPassword, htmlform_InHidden,
	htmlform_InRadio, htmlform_InCheckbox,
	htmlform_InSubmit, htmlform_InReset,
	htmlform_Select, htmlform_TextArea
};

struct wdgnode {
	enum htmlformTag tag;
	AWidget *wgt;
	attlist *atts;
	htmltext *initialvalue;
		// for Textarea is the initial string
		// for Select, is initial options string
	htmltext *optionattrs;
		// for Select
};
/*
	An options string (in initialvalue) for select has each option 
preceded by a colon and possibly an asterisk.  Before the first colon is 
any text appearing between <SELECT> and <OPTION>.  
The asterisk appears if the option has the SELECTED attribute.  
	The optionattrs string has one colon for every colon in the
options string.  After the colon is the attributes for the option, 
including VALUE and SELECTED, if specified.

XXX Woe betide us if an option contains a colon.

*/

#include <andrewos.h>
#include <mflex.H>
class NO_DLL_EXPORT fieldlist;
DEFINE_MFLEX_CLASS(fieldlist, struct wdgnode, 10)
	// to iterate through the nodes of `form' use code like
	//	int i;
	//	for (i = 0; i < form->GetN(); i++) 
	//		process wdgnode as    
	//			form[i].field, form[i].atts, form[i].selected
	// use AddNode to add an additional node

#include <ATK.H>
class attlist;
class web;
class webview;
class htmlform: public ATK {
  public:
	virtual ATKregistryEntry *ATKregistry();
	htmlform();
		// create object
	htmlform(htmltext *container, attlist *atts);
		// create object within given text
	virtual ~htmlform();
		// Destroy widgets and htmlatt-s 
	struct wdgnode *AddNode(AWidget *field, attlist *atts, 
				enum htmlformTag tagtype);
		// adds a field to the form;  returns ptr to field
	AWidget *AddInputNode(attlist *atts);
		// create and add node for <INPUT ...>
	AWidget *AddSelectNode(attlist *atts, htmltext **ptxtobj);
		// create and add node for <SELECT>...</>
                // caller retains ownership of all args
	AWidget *AddTextareaNode(attlist *atts, htmltext **ptxtobj);
        // create and add node for <TEXTAREA>...</>
        void FinishSelectNode();
          // copies the initial text for safe keeping (via FinishNode) and
          // sets the scrolled attribute if there are more items than are visible.
        void FinishNode();
        // copies the initial text for safe keeping.
        void ApplyOptionAttrs(attlist *atts);
        // applies the given attributes to the last option entered in a select node.
	void Completed();
		// no more fields
	const char *Submission(web *wd, webview *wv);
		// constructs the message for a form
	void WriteTaggedText(FILE *file, AWidget *w);
	struct wdgnode *FindNodeByName(char *name);
		// finds the node having the given name
		// -1 for failure
	struct wdgnode *FindNodeByWidget(AWidget *w);
		// finds the node having the given widget
		// NULL for failure
	const char *GetAtt(int index, const char *id);
	inline const char *GetName(int index) 
		{ return GetAtt(index, "name"); }
	inline const char *GetValue(int index)
		{ return GetAtt(index, "value"); }

	fieldlist fields;
        htmltext *containingtext;
        attlist *atts;
};
/** @} */
#endif /* _htmlform_H_ */
