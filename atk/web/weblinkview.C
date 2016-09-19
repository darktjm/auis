/* Copyright  Carnegie Mellon University 1995 -- All rights reserved */

#include <andrewos.h>
ATK_IMPL("weblinkview.H")
#include <sys/param.h>		/* for MAXPATHLEN */
#include <menulist.H>
#include <message.H>
#include <proctable.H>

#include <weblink.H>
#include <weblinkview.H>

static class menulist *weblinkview_menulist = NULL;


ATKdefineRegistry(weblinkview, linkview, 
		weblinkview::InitializeClass);

static void LinkProc(class weblinkview  *self, char  *param);

// LinkProc  asks the user for the url target of the link.
//
	static void
LinkProc(class weblinkview  *self, char  *param) {
	char buf[MAXPATHLEN];
	class weblink *l = (class weblink *)(self)->GetDataObject();

	if (param) 
		strcpy(buf, param);
	else {
		if(message::AskForString(self, 0, "New URL for link:",
				(l)->GetRawLink(), buf, sizeof(buf)) < 0)
			return;
	}

	(l)->SetLink( buf);
	if ((l)->GetText() == NULL) {
		(l)->SetText( buf);
		message::DisplayString(self, 10, 
				"Changed link target (and label).");
	} 
	else {
		message::DisplayString(self, 10, 
				"Changed link target.");
	}
}

// Initialize all the class data, particularly, set up 
//  the proc table entries and the menu list 
//      (which is cloned for each instance of this class).
//
	boolean
weblinkview::InitializeClass() {
	struct proctable_Entry *proc = NULL;

	weblinkview_menulist = new menulist;
/*
	if (tempProc = proctable_Lookup("frame-visit-file"))
		frame_VisitFile = proctable_GetFunction(tempProc);
*/
	proc = proctable::DefineProc("weblinkview-set-urllink",
		(proctable_fptr) LinkProc, 
		&weblinkview_ATKregistry_, NULL, 
		"Prompts for user to set target url of the link button.");
	(weblinkview_menulist)->AddToML(
			"Link~1,Set URL~11", proc, 0, 0);
	return(TRUE);
}

weblinkview::weblinkview() {
	ATKinit;
	this->ml = (weblinkview_menulist)->DuplicateML( this);
	THROWONFAILURE( TRUE);
}

weblinkview::~weblinkview() {
	ATKinit;
	return;
}

	void
weblinkview::PostMenus(class menulist  *ml) {
	(this->ml)->ClearChain();
	if (ml) 
		(this->ml)->ChainAfterML( ml,0);
	(this)->linkview::PostMenus( this->ml);
}

	void 
weblinkview::LinkFile(char  *dest )  {
	LinkProc(this, dest);
}
