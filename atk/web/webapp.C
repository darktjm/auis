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
ATK_IMPL("webapp.H")

#ifndef NORCSID
static UNUSED const char rcsid[] = "$Header: /afs/cs.cmu.edu/project/atk-src-C++/atk/web/RCS/webapp.C,v 1.10 1996/11/07 02:00:24 robr Exp $";
#endif

#include <frame.H>
#include <buffer.H>
#include <im.H>
#include <ezapp.H>
#include <signal.h>
#include <environ.H>
#include <application.H>
#include <wbuttonv.H>
#include <value.H>
#include <lpair.H>
#include <proctable.H>

#include <webparms.h>
#include <webbut.h>
#include <webcom.H>
#include <webview.H>
#include <web.H>
#include <webapp.H>

extern "C" {
#include <webserver.h>
}

static struct webapp_webhist *fwh=NULL;
static class webapp *gself;

static char homeURL[1024];
static proctable_fptr frame_WriteFile = NULL;

ATKdefineRegistry(webapp, ezapp, webapp::InitializeClass);

static void lpinit();
static void viewfromframe(class frame  *f, char  *url);
static void web_view(class webapp *self, char  *url);
static void viewforclient(char  *url);
static void handlesig(int  sig, class webapp  *self);
static void ButtonCallBack(class frame  *frame,
		class value  *val, long  r1, long  r2);
static void initframe(class frame  *f, class im  *im);


	struct webapp_webhist *
webapp::getwebhist(class view  *view) {
	ATKinit;

	class im *im;
	struct webapp_webhist *wp;
	im = (view)->GetIM();
	for(wp = fwh; wp != NULL; wp = wp->next){
		if (im == wp->im) return wp;
	}
	wp = new webapp_webhist;
	wp->next = fwh;
	fwh = wp;
	wp->im = im;
	wp->hpos=0;
	return wp;
}

	static void 
 lpinit() {
	struct proctable_Entry *tempProc;
	ATK::LoadClass("frame");
	tempProc = proctable::Lookup("frame-write-file");
	if (tempProc != NULL)
		frame_WriteFile =  
				 proctable::GetFunction(tempProc);
	else {
		fprintf(stderr,
			"failed to find \"frame-write-file\"\n");
		frame_WriteFile = NULL;
	}
}

#if 0
	static class web *
getweb(class webapp  *self) {
	class frame *f;
	class buffer *b;
	class web *w;
	f = ((class ezapp *)self)->frame;
	b = (f)->GetBuffer();
	w = (class web *) (b)->GetData();
	if (ATK::IsTypeByName((w)->GetTypeName(), "web"))
		return w;
	return NULL;
}
#endif

	static class view *
getviewfromframe(class frame  *f) {
            class view *v = f->targetView;
            return v;
#if 0
	if (v && v->IsType("webview"))
		return v;
#if 0
	return NULL;

#else
	// have to create a webview		-wjh
	web *wv = new web;
	buffer *b = buffer::Create("http://dev/null", NULL, 
				"Empty Page", wv);
	f->SetBuffer(b, TRUE);
	v = f->targetView;
	if ( ! v) {
		fprintf(stderr, "Can't get a webview on frame\n");
		fflush(stderr);
	}
	return v;
#endif
#endif
}

	static class view *
getview(class webapp  *self) {
	class frame *f;
	f = ((class ezapp *)self)->framep;
	return  getviewfromframe(f);
}

	static void 
viewfromframe(class frame  *f,  char  *url) {
	class view *v;
	if ((v = getviewfromframe(f)) == NULL) {
		fprintf(stderr,  "web process can't find web view\n");
		return;
	}
	webview::VisitWeb(v,  url);
}

	static void 
web_view(class webapp *self,  char  *url)  {
	class view *v = getview(self);
	if (v == NULL) {
		fprintf(stderr,  "web process cannot find web view \n");
		return;
	}
	webview::VisitWeb(v,  url);
}

	static void 
viewforclient(char  *url)  {
	web_view(gself,  url);
}

	static void 
handlesig(int  sig,  class webapp  *self)  {
	char buf[256],  url[1024],  *c;
	FILE *f;
	im::SignalHandler(SIGUSR1,  (im_signalfptr)handlesig, 
			(char *)self);
	sprintf(buf,  WEBOPENFILE,  getpid());
	f = fopen(buf,  "r");
	if (f) {
		if (fgets(buf,  256,  f) && fgets(url,  1024,  f)){
			for(c = url ; *c != '\0' && *c != '\n'; c++) {}
			*c = '\0';
			web_view(self,  url);
		}
		fclose(f);
	}
}

	static void 
ButtonCallBack(class frame  *frame,  class value  *val,  
			long  r1,  long  r2)  {
	int res;
	class view *wv;
	res = (val)->GetValue();
	if (res == NOVAL) return;
	wv = getviewfromframe(frame);
	switch(res){
		case BACK :
		case FORWARD :
			webview::movehist(wv,  res,  wv);
			break;
		case HOME :
		// temporary test
		//	printf("restarting awww\n");
		//	webcom::KillChild(TRUE);
		// end test
			viewfromframe(frame,  homeURL);
			break;
                    case RELOAD :
                        {
                        class webcom *w =
                        webcom::FindWebcomFromData( (wv)->GetDataObject());
                        char *s = "";
                        if (w)  s = (w)->GetURL();
                        if (s == NULL) s = "";
                        webview::VisitWeb(wv, s, WEBCOM_Reload);
                        }
        
			break;
		case OPEN :
			webview::OpenURL(wv);
			break;
		case SAVE_AS :
			if (frame_WriteFile == NULL)lpinit();
			if (frame_WriteFile != NULL)
				frame_WriteFile(frame,  0);
			else message::DisplayString(NULL,  0,  
						"Function not available");
			break;
		case CLONE :
			webview::Clone(wv);
			break;
		case CLOSE :
			// xxx implement Close
			printf("Use the \"Quit\" menu option\n");
			break;
		case CANCEL:
			webcom::Cancel("Read terminated by user");
			break;
	}
	(val)->SetValue(NOVAL);
}

	static void 
initframe(class frame  *f,   class im  *im)  {
	class lpair *lp;
	class value *local_bt;
	class wbuttonv *local_bv;
	class lpair *local_lpMain;
	class buffer *b;

	local_bv = new  wbuttonv;
	(local_bv)->SetLabel( LABELSTRING);
	local_bt = new  value;
	(local_bt)->SetValue(-1);
	(local_bv)->SetDataObject(local_bt);

	(local_bt)->AddCallBackObserver(f,   
			(value_fptr)ButtonCallBack,  (long)f);
	local_lpMain = lpair::Create( f,   local_bv,   35);
	(im)->SetView(local_lpMain);
	lpinit();
	(f)->SetCommandEnable(TRUE);
	b = (f)->GetBuffer();
/*	buffer_SetDefaultObject(b,  NULL); */
}

webapp::webapp()  {
	ATKinit;
	const char *t = environ::GetProfile("WebHome");
	strcpy(homeURL,  (t) ? t : DEFAULTHOME);
	gself = this;
	THROWONFAILURE( TRUE);
}

webapp::~webapp() {
	ATKinit;
}

	boolean 
webapp::InitializeClass()  {
	fwh = NULL;
	return TRUE;
}

	boolean 
webapp::ParseArgs(int  argc,  const char  **argv)  {
	boolean res;
	const char **avx = argv;

	// need to pass switches to ezapp & application  -wjh
	while (*++avx){
		if(**avx == '-') continue;	// retain switch
		strcpy(homeURL,  *avx);
		application::DeleteArgs(avx--,  1);
		argc--;
	}
	(this)->ezapp::ParseArgs(argc, argv);
			// ezapp will create the Scratch buffer
	this->defaultObject = "web";
	return TRUE;
}

	boolean 
webapp::Start() {
	boolean res;
	im::SignalHandler(SIGUSR1,  (im_signalfptr)handlesig,
			(char *)this);
	res = (this)->ezapp::Start();

	  /* xxx code here to read in ???*/

	initframe(this->framep,  this->imp);

	// establish ourselves as a webserver
	// 	listen for requests from sibling processes
	int fd = weblisten();	// webserver.c
	if (fd != 1) {
		FILE *f = fdopen(fd, "r");
		if (f)  im::AddFileHandler(f,  
				(im_filefptr)webaccept,  
				(char *)viewforclient,  0);
	}
	return TRUE;
}

	boolean 
webapp::Fork()  {
	FILE *f;
	int res;
	res = (this)->ezapp::Fork();
	f = fopen(WEBPIDFILE,  "w");
	if (f){
		fprintf(f,  "%d\n",  getpid());
		fclose(f);
	}
	return res;
}

	int 
webapp::Run()  {
	// xxx if there is no home URL, could prompt here
	if (*homeURL){
		im::ForceUpdate();
		web_view(this,  homeURL);
	}
	(this)->ezapp::Run();
	unlink(WEBPIDFILE);
	webcom::KillChild(FALSE);
	return 0;
}
