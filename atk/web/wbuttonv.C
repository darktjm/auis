/* Copyright  Carnegie Mellon University 1995 -- All rights reserved */

#include <andrewos.h>
ATK_IMPL("wbuttonv.H")
#include <buttonV.H>
#include <wbuttonv.H>

#define NEEDSET(SELF) (SELF->needinit == TRUE \
			&& SELF->wlabel != NULL)

ATKdefineRegistry(wbuttonv, buttonV, wbuttonv::InitializeClass);
static void doset(class wbuttonv  *self);

	static void 
doset(class wbuttonv  *self)  {
	class buttonV *vself = (class buttonV *) self;

	((class valueview *)vself)->back = (self)->WhitePattern();
	vself->label = self->wlabel;
	vself->vertical = FALSE;
	vself->topdown = FALSE;
	vself->columns = 1;
	self->needinit = FALSE;
	vself->fontsize = 12;
	vself->fontname = "andysans";

	(self)->CacheSettings();
	self->needinit = FALSE;
}

	void 
wbuttonv::Update() {
	if (NEEDSET(this)) doset(this);
	(this)->buttonV::Update();
}

	void 
wbuttonv::FullUpdate( enum view_UpdateType  type, 
			long  x, long  y, long  width, long  height )  {
	if (NEEDSET(this)) doset(this);
	(this)->buttonV::FullUpdate(  type, x, y, width, height );
}

	void 
wbuttonv::SetLabel(const char  *label)  {
	if (this->wlabel) free(this->wlabel);
	if (label)
		this->wlabel = strdup(label);
	else this->wlabel = NULL;
	this->needinit = TRUE;
	(this)->WantUpdate(this);
}

wbuttonv::wbuttonv()  {
	ATKinit;

	(this)->SetFixedCount(0);
	(this)->SetFixedColumns(0);
	this->wlabel = NULL;
	THROWONFAILURE( TRUE);
}

	boolean 
wbuttonv::InitializeClass()  {
	return TRUE;
}
