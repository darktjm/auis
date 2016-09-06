/* Copyright Carnegie Mellon University 1995 -- All rights reserved

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
ATK_IMPL("web.H")
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <errno.h>
#include <ctype.h>

#include <atom.H>
#include <environ.H>
#include <image.H>
#include <environment.H>
#include <message.H>
#include <im.H>
#include <buffer.H>
#include <filetype.H>
#include <dataobject.H>
#include <path.H>
#include <attribs.h>

#include <htmlenv.H>
#include <attlist.H>
#include <style.H>

#include <web.H>
#include <webcom.H>
#include <webparms.h>
#include <table.H>

ATKdefineRegistry(web, htmltext, web::InitializeClass);

	boolean 
web::InitializeClass()  {
	filetype::AddEntry(".web", "web", "");
	return TRUE;
}

void web::ImbedTextInTable(table *t, chunk *c) {
    t->Imbed((char *)GetTypeName(), c);
    cell *cll = t->GetCell(c->TopRow,c->LeftCol);
    web *txtobj = (web *) cll->interior.ImbeddedObject.data;
    txtobj->imbedded=TRUE;
    if(webcomp) {
	webcom *wc=new webcom;
	wc->url=webcomp->url;
	wc->parent=webcomp;
	txtobj->SetWebcom(wc);
    }
}

image *web::GetImage(const char *file, attlist *atts) {

    class image* dat;
    struct htmlatts *hw, *hh;

    if (this->webcomp == NULL) 
        (this)->SetWebcom( new webcom);
    if (*file == '"') file++;
    /*  webcom will start transfer and wait
     for mime header to be transmitted, so object
     can be created, then return the object to insert.
     object read will happen when read is complete
     */
    dat = (this->webcomp)->getimage((char *)file, NULL);
    if (dat == NULL) {
        dat =(this)->htmltext::GetImage(BROKENIMGFILE, atts);
    } else {
        long w=dat->Width(), h=dat->Height();
        if (atts) {
            hw = (atts)->GetAttribute("width");
            if (hw)  w=atoi(hw->value);
            hh = (atts)->GetAttribute("height");
            if (hh) h=atoi(hh->value);
#define ZOOMVALUE(n,d) ((unsigned int)((((double)n)/d)*100))
            if(w!=dat->Width() || h!=dat->Height()) {
                image *ndat=dat->Zoom( ZOOMVALUE(w, dat->Width()), ZOOMVALUE(h, dat->Height()));
                if(ndat) {
                    dat->Destroy();
                    return ndat;
                }
            }
        }
    }
    return dat;
}

	long 
web::Read(FILE * file, long  id) {  	
	long res;
	if (this->webcomp == NULL)
		this->webcomp = new webcom;
	(this)->SetReadOnly(TRUE);
	(this->webcomp)->ReadContext(file, id);
	res = (this)->htmltext::Read(file, id);
	return res;
}

	long 
web::ReadHTML(FILE * file, long  id) {
	long res;
	res = (this)->htmltext::Read(file,  id);
	return res;
}

web::web() {
	ATKinit;

	(this)->ReadTemplate("html", FALSE);	/* 
		icky but it Cures problem of insets in tables 
		and users who don't map a template to 
		the html extension */
	this->webcomp = NULL;
	imbedded=FALSE;
	THROWONFAILURE(TRUE);
}

static	int 
mybuffer_ReadFile(class buffer  *self, char  *filename)  {
	long objectID;
	int returnCode = 0;
	char realName[MAXPATHLEN];
	const char *objectName;
	FILE *thisFile;
	struct stat stbuf;
	struct attributes *attributes;
	struct attributes *tempAttribute;
	filetype::CanonicalizeFilename(realName, filename, 
				sizeof(realName) - 1);
	filename = realName;
/*
	sprintf(foo, "ls -l %s", filename);
	system(foo);
*/
	if (stat(filename, &stbuf) < 0) {
		return -2;
	}
	if ((stbuf.st_mode & S_IFMT) == S_IFDIR) {
		return -7;
	}

	if ((thisFile = fopen(filename, "r")) == NULL)
		return -3;

	self->lastTouchDate = (long) stbuf.st_mtime;

	objectName = filetype::Lookup(thisFile, filename, 
				&objectID, &attributes);

/* This next thing is a hack. Really need a flags parameter so we
 *  can keep the thing readonly if need be.  xxx
 */
	self->readOnly = FALSE;
	for (tempAttribute = attributes; 
			tempAttribute != NULL; 
			tempAttribute = tempAttribute->next)
		if (strcmp(tempAttribute->key, "readonly") == 0)
			self->readOnly = tempAttribute->value.integer;

	if (objectName == NULL)
		objectName = "text";   /* The default... */

	if (strcmp(((self)->GetData())->GetTypeName(),
			objectName) == 0) {
		long version;

		((self)->GetData())->SetAttributes(attributes);
		((self)->GetData())->Read(thisFile, objectID);
		((self)->GetData())->NotifyObservers(0);
		(self)->SetFilename(filename);

		version = ((self)->GetData())->GetModified();
		(self)->SetCkpClock(0);
		(self)->SetCkpVersion(version);
		(self)->SetWriteVersion(version);
	}
	else {
		returnCode = -4;
	}
	fclose(thisFile);
	(self)->NotifyObservers(0);/* Tuck it into slot. */
	return returnCode;
}

	void 
web::SetWebcom(class webcom  *c) {
	class buffer *buf;
	this->webcomp  = c;
	(c)->SetWeb(this);
	if ((c)->Status() & WEBCOM_Loaded) {
		(this)->ClearCompletely();
		(this)->ReadTemplate("html", FALSE);
		buf = buffer::FindBufferByData(this);
		mybuffer_ReadFile(buf, (c)->GetTmpName());
	}
}

web::~web()
{
		ATKinit;
}
