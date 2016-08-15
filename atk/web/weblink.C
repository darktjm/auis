
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
ATK_IMPL("weblink.H")

#ifndef NORCSID
static UNUSED const char rcsid[] = "$Header: /afs/cs.cmu.edu/project/atk-src-C++/atk/web/RCS/weblink.C,v 1.2 1995/12/12 18:53:04 wjh Stab74 $";
#endif

#include <text.H>
#include <message.H>
#include <view.H>
#include <textview.H>
#include <proctable.H>
#include <view.H>
#include <frame.H>
#include <im.H>

#include <weblink.H>

extern "C" {
#include <webserver.h>
};

static proctable_fptr frame_VisitFile = NULL;
#define getframe(DV)  \
		((frame *) (((view *)DV)->GetIM())->topLevel)

char *lnames[] = {"http:", ""};

ATKdefineRegistry(weblink, link, weblink::InitializeClass);

static long shouldBeALink(class text  *text, long  pos);
static void weblink_replacetext(class view  *v, long  dat);
static void weblink_warpto(class view  *v, long  dat);
static void weblink_RequestURL(class view  *v, long  dat);
static boolean weblink_OpenRemote(char  *url);

	static boolean 
weblink_OpenRemote(char  *url) {
	char rbuf[1024];
	if (webclient(url, 5, rbuf, 1024) < 0){
		printf("Starting up web server\n");
		sprintf(rbuf, "runapp webapp %s", url);
		system(rbuf);
		return TRUE;
	}
	return FALSE;
}


	weblink *
weblink::GetNewWebLink(char  *url)  {
	ATKinit;
	weblink *w;
	w = new weblink;
	(w)->SetLink(url);
	return w;
}

	boolean 
weblink::isLinkString(char  * s)  {
	ATKinit;
	char **nm;
	for(nm = lnames; **nm != '\0'; nm++) {
		if (**nm == *s 
				&& (strncmp(s, *nm, strlen(*nm)) == 0))
			return TRUE;
	}
	return FALSE;
}

	static long 
shouldBeALink(text  *text, long  pos)  {
	int tlen, len, c;
	char **nm;

	for(nm = lnames; **nm != '\0'; nm++) {
		if ((text)->Strncmp(pos, *nm, strlen(*nm)) == 0)
			break;
	}
	if (**nm == '\0') return 0;
	tlen = (text)->GetLength();
	len = 0;
	while(pos + len < tlen ) {
		c = (text)->GetChar(pos + len );
		if (c == '"' || c == '>' || isspace(c)) break;
		len++;
	}
	return len;
}

	void 
weblink::ReplaceTextWithLinks(class text *ptext, 
			long  pos, long  len)  {
	ATKinit;
	int tlen;
	mark *m;
	weblink *w;
	char buf[1024];
	int lenset;
	if (len == 0) {
		/* if length is 0,  deal with the current word */
		tlen = (ptext)->GetLength();
		while(pos > 0 
				&& ! (isspace((ptext)->GetChar(pos - 1))))
			pos--;
		while(pos + len < tlen && 
				! (isspace((ptext)->GetChar(pos+len+1))))
			len++;
		lenset = FALSE;
	}
	else lenset = TRUE;
	/* create a mark to keep track of end pos 
		while we are modifying the text */
	m = (ptext)->CreateMark(pos, len);
	for(; pos < (m)->GetPos() + (m)->GetLength(); pos++) {
		if (lenset || ((len = shouldBeALink(ptext, pos)) > 0)) {
			(ptext)->CopySubString(pos, len, buf, FALSE);
			w = weblink::GetNewWebLink(buf);
			(ptext)->DeleteCharacters(pos, len);
			(ptext)->AddView(pos, "weblinkv", w);
		}
	}
	(ptext)->NotifyObservers(0);
}

	char * 
weblink::warpToURL(char  *url, view  *v)  {
	ATKinit;
	char *fname = NULL;
	frame *f;

/*
	if (v == NULL) {
		fprintf(stdout, 
			"ERROR,  no view passwd to warp func\n");
		return NULL;
	}
	f = getframe(v);
	if (f == NULL || frame_VisitFile == NULL) {
		fprintf(stdout, 
			"ERROR Null frame or vf, frame = %d \n", f);
		return NULL;
	}
*/

	weblink_OpenRemote(url);
	return NULL;
}

	void 
weblink::warpToURLinText(class text *ptext,
			long  pos, long  len, view  *v)  {
	ATKinit;
	int tlen;
	mark *m;
	weblink *w;
	char buf[1024], *fname;
	int lenset;

	if (len == 0) {
		/* if length is 0,  deal with the current word */
		tlen = (ptext)->GetLength();
		while(pos > 0 && 
				!(isspace((ptext)->GetChar(pos - 1))))
			pos--;
		while(pos + len < tlen && 
				!(isspace((ptext)->GetChar(pos + len + 1)))) 
			len++;
		lenset = FALSE;
	}
	else lenset = TRUE;
	m = (ptext)->CreateMark(pos, len);
	for(; pos < (m)->GetPos() + (m)->GetLength(); pos++) {
		if (lenset || ((len = shouldBeALink(ptext, pos)) > 0)) {
			(ptext)->CopySubString(pos, len, buf, FALSE);
			if (weblink::warpToURL(buf, v)) 
				break;
		}
	}
}

	static void 
weblink_replacetext(view  *v, long  dat)  {
	textview *tv;
	if ( ! v->IsType("textview")) {
		message::DisplayString(v,  0,  
				"link replace works on text files only");
		return;
	}
	tv = (textview *) v;
	weblink::ReplaceTextWithLinks(
			(text*)(v)->GetDataObject(),
			(tv)->GetDotPosition(),  (tv)->GetDotLength());
}

	static void 
weblink_warpto(view  *v, long  dat)  {
	textview *tv;
	if ( ! v->IsType("textview")) {
		message::DisplayString(v,  0,  
				"link warpto works on text files only");
		return;
	}
	tv = (textview *) v;
	weblink::warpToURLinText((text *) (v)->GetDataObject(),
		(tv)->GetDotPosition(),  (tv)->GetDotLength(), v);
}

	static void 
weblink_RequestURL(view  *v, long  dat)  {
	char buf[1024];
	if (message::AskForString(NULL, 0, "URL:", "", buf, 1024)
				< 0)
		message::DisplayString(NULL, 0, "Canceled");
	else{
		message::DisplayString(NULL, 0, "requesting URL");
		weblink_OpenRemote(buf);
	}
}

	boolean 
weblink::InitializeClass()  {
	struct ATKregistryEntry  *textviewtype = 
			ATK::LoadClass("textview");
	struct ATKregistryEntry  *viewtype = 
			ATK::LoadClass("view");
	struct proctable_Entry *tempProc;
	ATK::LoadClass("frame");
	if (tempProc = proctable::Lookup("frame-visit-file"))
		frame_VisitFile = proctable::GetFunction(tempProc);
	proctable::DefineProc("weblink-ReplaceText", 
			(proctable_fptr)weblink_replacetext, 
			textviewtype, NULL, "replace text with links");
	proctable::DefineProc("weblink-RequestURL",
			(proctable_fptr)weblink_RequestURL, 
			viewtype, NULL, "Prompt for a URL to display");
	proctable::DefineProc("weblink-WarpToURL", 
			(proctable_fptr)weblink_warpto, textviewtype, 
			NULL, "Display URL document");
	return TRUE;
}

	const char *
weblink::GetResolvedLink()  {
	char *filename;

	printf("calling weblnch class load\n");
	ATK::LoadClass("weblnch");
	filename=(this)->GetRawLink();
	if ( ! filename) {
		message::DisplayString(NULL,  10,  "No link");
		return NULL;
	}
	if (weblink::isLinkString(filename)) {
		printf("weblnch_OpenRemote called %s\n", filename);
		fflush(stdout);

		if ( ! weblink_OpenRemote(filename)) {
			message::DisplayString(NULL,  10,  
			"Remote link failed");
		}
		return NULL;
	}
	return (this)->link::GetResolvedLink();
}
