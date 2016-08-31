/* Copyright  Carnegie Mellon University 1995 -- All rights reserved

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
	if (label) {
		this->wlabel = (char *)malloc(strlen(label) + 1);
		strcpy(this->wlabel, label);
	}
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
