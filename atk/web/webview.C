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
ATK_IMPL("webview.H")

#ifndef NORCSID
static UNUSED const char rcsid[] = "$Header: /afs/cs.cmu.edu/project/atk-src-C++/atk/web/RCS/webview.C,v 1.26 1996/11/07 23:54:55 robr Exp $";
#endif

#include <im.H>
#include <frame.H>
#include <buffer.H>
#include <view.H>
#include <message.H>
#include <proctable.H>
#include <lpair.H>
#include <wbuttonv.H>
#include <value.H>
#include <bind.H>
#include <menulist.H>
#include <environ.H>
#include <dataobject.H>
#include <matte.H>
#include <search.H>
#include <filetype.H>
#include <completion.H>
#include <physical.h>
#include <astring.H>
#include <table.H>

#include  <htmltextview.H>

#include <web.H>
#include <webcom.H>
#include <webview.H>
#include <webapp.H>
#include <htmlimagev.H>
#include <htmlenv.H>
#include <dictionary.H>
#include <webbut.h>
#include <webparms.h>
#include <util.h>
#include <environment.H>
#include <viewref.H>

static char *starttm;

static class menulist *web_Menus;

static char *lastPattern ;

static proctable_fptr frame_VisitFile;
static proctable_fptr frame_NewWindow;
static proctable_fptr frame_WriteFile;
static proctable_fptr frame_OpenFile;


ATKdefineRegistry(webview, htmltextview, webview::InitializeClass);

static void lpinit();
static void phist(char  *s, struct webapp_webhist  *wp);
static int HLtoHTML(FILE  *in, FILE  *outf);


	static webview *
getwebview(class im  *im) {
	class lpair *tv;
	class frame *f;
	tv = (class lpair *)im->topLevel;
	if ((tv)->IsType("frame"))
		f = (class frame *)tv;
	else f = (class frame *)(tv)->GetNth(0);
	if (f) 
		return (webview *)(f)->GetView();
	return NULL;
}

	static class frame *
getframe(class view  *v) {
	class lpair *tv;
	class frame *f;
	tv = (class lpair *)(v)->GetIM()->topLevel;
	if (ATK::IsTypeByName((tv)->GetTypeName(),
			 "frame"))	
		return (class frame *)tv;
	f = (class frame *)(tv)->GetNth(0);
	if (ATK::IsTypeByName((f)->GetTypeName(), "frame"))
		return f;
	return NULL;
}

static	class buffer *
getbuffer(class view  *v) {
	class frame *f;
	if ((f = getframe(v)) == NULL) return NULL;
	return (f)->GetBuffer();
}

static	void 
Back(view  *self) {
	if ((self)->GetIM() == NULL) {
		/* BS this shouldn't happen,  
			must be a bug in frame_setbuffer() */
		fprintf(stderr, "web: NO IMPTR!!!!\n");
		return;
	}
	webview::movehist(self, BACK, self);
}

static	void 
Forward(view  *self) {
		webview::movehist(self, FORWARD, self);
}

static	 void 
CloneWindow(view  *self) {
	class frame *f;
	if (frame_NewWindow == NULL) lpinit();
	if (frame_NewWindow == NULL) {
		message::DisplayString(self, 0, "Not Implemented");
		return;
	}
	f = getframe(self);
	if (f == NULL) {
		message::DisplayString(self, 0, 
					"Can't clone this window");
		return;
	}
	frame_NewWindow(f, 0);
}

	void 
webview::Clone(class view  *self) {
	ATKinit;
	CloneWindow(self);
}


	static void 
ReloadCurrent(class view  *self) {
	class webcom *w =
		webcom::FindWebcomFromData(
					(self)->GetDataObject());
	char *s = "";
	if (w)  s = (w)->GetURL();
	if (s == NULL) s = "";
	webview::VisitWeb(self, s, WEBCOM_Reload);
 }
        
static	void 
OpenFile(class view  *self) {
    static char prefix[]="file://localhost";
    char buf[MAXPATHLEN+sizeof(prefix)];
    char dir[MAXPATHLEN+1];
    strcpy(buf, prefix);
    class webcom *wc = webcom::FindWebcomFromData( (self)->GetDataObject());
    if(wc && wc->url && wc->url->Name() && strncmp(wc->url->Name(), prefix, sizeof(prefix)-1)==0) {
        strncpy(dir, wc->url->Name()+sizeof(prefix)-1, MAXPATHLEN);
        dir[MAXPATHLEN]='\0';
        char *p=strrchr(dir, '/');
        if(p) *p='\0';
        else dir[0]='\0';
    } else {
        im::GetDirectory(dir);
        strcat(dir, "/");
    }
    if (completion::GetFilename(NULL, "Local file:", dir, buf+sizeof(prefix)-1, sizeof(buf)-sizeof(prefix), FALSE, TRUE) == -1) {
        message::DisplayString(NULL, 0, "Cancelled.");
        return;
    }
    webview::VisitWeb(self, buf);
}

static	void 
Home(class view  *self) {
	webview::VisitWeb(self, starttm);
}

static	void 
History(class view  *self) {
	struct webapp_webhist *wp= webapp::getwebhist(self);
	class web *w;
	class webcom *wc;
	size_t i;
	int j=0;
	long res;
	char **hcp=new char *[wp->history.GetN()+2];
	for(i = 0; i < wp->history.GetN(); i++) {
		w = (class web *)(wp->history[i])->GetData();
		wc = (w)->getwebcom();
		if (wc != NULL && wc->title != NULL) {
		    hcp[j] = (wc->title)->Name();
		    j++;
		}
	}
	hcp[j] = "Cancel";
	j++;
	hcp[j] = NULL;
	if (message::MultipleChoiceQuestion(self,  60, "Choose Title", j,  &res,  hcp,  NULL) < 0) {
	    delete [] hcp;
	    return;
	}
	delete [] hcp;
	if (res < 0 || res>=wp->history.GetN()) return;
	webview::movehist(self, res + 100,  self);
}

	static class view *
getbv(class view  *v, class frame  *f, class im  *pim, 
		char *label, value_fptr callback) {
	class value *local_bt;
	class wbuttonv *local_bv;
	class lpair *local_lpMain;
	
	local_bv = new wbuttonv;
	(local_bv)->SetLabel( label);
	local_bt = new value;
	(local_bt)->SetValue(-1);
	(local_bv)->SetDataObject(local_bt);

	(local_bt)->AddCallBackObserver(v,  callback,  (long)v);
	local_lpMain = lpair::Create( f,  local_bv,  35);
	(pim)->SetView(local_lpMain);
	/*	frame_SetCommandEnable(f, TRUE); */
	return local_bv;
}

	static void 
hlcallback(webview *self, class value *val, long  , long  ) {
	class im *im;
	int res = (val)->GetValue();
	if (res == NOVAL) 
		return;
	switch(res) {
	case 0:
		im = (self)->GetIM();
		if (self->remoteim != NULL)
			(self->remoteim)->RemoveObserver(self);
		(im)->Destroy();
		break;
	default:
		break;
	}
}

static	char * 
gethotlist(char *buf) {
	const char *home;
	home = environ::Get("HOME");
	sprintf(buf, "%s/%s", home, HOTLIST);
	return buf;
}

static 	void 
Hotlist(view  *self) {
	class web *pweb;
	class webview *pwebview;
	class frame *newFrame;
	class im *window;
	char buf[1024];
	FILE *f, *outf;

	f = fopen(gethotlist(buf), "r");
	if (f == NULL) {
		message::DisplayString(self, 0, "No Hotlist Found");
		return;
	}
	outf = fopen(TMPHOTLIST, "w");
	if (outf == NULL) {
		message::DisplayString(self, 0, 
				"can't create tmp hotlist");
		return;
	}
	if (HLtoHTML(f, outf) == 0) {
		message::DisplayString(self, 0, "hotlist empty");
		return;
	}
	fclose(f); fclose(outf);
	f = fopen(TMPHOTLIST, "r");
	if ((newFrame = new class frame) == NULL) {
		fprintf(stderr, 
"Could not allocate enough memory to create new window.\n");
		return;
	}
	window = im::CreateTransient((self)->GetIM());
	if (window == NULL) {
		fprintf(stderr, "Could not create new window.\n");
		if (newFrame) (newFrame)->Destroy();
		return;
	}
	pweb = new web;
	pwebview = new webview;
	(pwebview)->SetRemote((self)->GetIM());
	(pwebview)->SetDataObject(pweb);
	(pweb)->ReadHTML(f, 0);

	(newFrame)->SetView((pwebview)->GetApplicationLayer());
	
	getbv(pwebview, newFrame, window, "dismiss", 
			(value_fptr)hlcallback);
	(newFrame)->SetCommandEnable( FALSE);

	(newFrame)->PostDefaultHandler( "message", 
			 (newFrame)->WantHandler( "message"));
}

static	void 
AddCurrent(view  *self) {
	class web *web = (class web *)(self)->GetDataObject();
	class webcom *wc;
	int need = FALSE;
	FILE *f;
	char buf[1024], obuf[1024];
	char *u = NULL;
	long t;
	gethotlist(buf);
	if (web && (wc = web->getwebcom()))
		u = (wc)->GetURL();
	if (u == NULL) {
		message::DisplayString(NULL, 0, "URL Unknown");
		return;
	}
	if (access(buf, R_OK) != 0) {
		need = TRUE;
	}
	if (message::AskForString(NULL, 0, "Label:", "", 
				obuf, 512) < 0) {
		message::DisplayString(NULL, 0, "Canceled");
		return;
	}
	if ((f = fopen(buf, "a")) == NULL) {
		
		message::DisplayString(self, 0, "Can't open file");
		return;
	}
	if (need) {
		fprintf(f, "ncsa-xmosaic-hotlist-format-1\nDefault\n");
	}
	time(&t);
	fprintf(f, "%s %s%s\n", u, ctime(&t), obuf);
	fclose(f);
	message::DisplayString(self, 0, "URL added to hotlist");
}

static	void 
MetaIndex(view  *self) {
	webview::VisitWeb(self, NCSAMETAINDEX);
}

static	void 
InternetStartingPoints(view  *self) {
	webview::VisitWeb(self, NCSASTARTINGPOINTS);
}

static	void 
Annotate(view  *self) {
	// xxx Annotate
	message::DisplayString(self, 0, "Not Implemented");
}

static	void 
EditAnnotation(view  *self) {
	// xxx EditAnnotation
	message::DisplayString(self, 0, "Not Implemented");
}

static	void 
DeleteAnnotation(view  *self) {
	// xxx DeleteAnnotation
	message::DisplayString(self, 0, "Not Implemented");
}

static	void 
lpinit() {
	struct proctable_Entry *tempProc;
	ATK::LoadClass("frame");
	tempProc = proctable::Lookup("frame-write-file");
	if (tempProc)
		frame_WriteFile = proctable::GetFunction(tempProc);
	else{
		fprintf(stderr, "web: Proctable look up failed\n");
		frame_WriteFile = NULL;
	}
	tempProc = proctable::Lookup("framecmds-open-file");
	if (tempProc)
		frame_OpenFile = proctable::GetFunction(tempProc);
	else frame_OpenFile = NULL;
}

static	void 
SaveAs(view  *self) {
	class frame *f;
	f = getframe(self);
	if (frame_WriteFile == NULL)lpinit();
	if (f != NULL && frame_WriteFile != NULL)
		frame_WriteFile(f, 0);
	else message::DisplayString(NULL, 0, 
			"Function not available");
}

	static void 
MyOpenURL(class view  *self) {
	char buf[1024];
	class webcom *w =
		webcom::FindWebcomFromData(
					(self)->GetDataObject());
	char *s = "";
	if (w)  s = (w)->GetURL();
	if (s == NULL) s = "";
	
	if (message::AskForString(NULL, 0, "URL:", s, buf, 1024) 
				< 0)
		message::DisplayString(NULL, 0, "Canceled");
	else webview::VisitWeb(self, buf, WEBCOM_TopLevel);
}

	static void 
ViewSrc(class view  *self) {
	class webcom *w =
		webcom::FindWebcomFromData(
					(self)->GetDataObject());
	char *s = "";
	if (w)  s = (w)->GetURL();
	if (s == NULL) s = "";
	webview::VisitWeb(self, s, WEBCOM_ViewRaw);
}

	void
webview::OpenURL(class view  *self) {
	ATKinit;
	MyOpenURL(self);
}

static struct bind_Description webBindings[]={
	{"webview-open-URL",  NULL, 0, 
		"Web File~4, Open URL~1", 0, 0, 
		(proctable_fptr)MyOpenURL, 
		"prompt for a URL ",  NULL}, 
	{"webview-open-local",  NULL, 0, 
		"Web File~4, Open File~2", 0, 0, 
		(proctable_fptr)OpenFile , 
		"Use the HTML inset to visit a local file",  NULL}, 
	{"webview-Reload-Current",  NULL, 0, 
		"Web File~4, Reload Current~3", 0, 0, 
		(proctable_fptr)ReloadCurrent , 
		"Reload Current",  NULL}, 
	{"webview-Save-As",  NULL, 0, 
		"Web File~4, Save As~5", 0, 0, 
		(proctable_fptr)SaveAs , 
		"Writes current document to file ",  NULL}, 
	{"webview-View-Src",  NULL, 0, 
		"Web File~4, View Src~7", 0, 0, 
		(proctable_fptr)ViewSrc , 
		"View Src ",  NULL}, 
	{"webview-Clone Window",  NULL, 0, 
		"Web File~4, Clone Window~8", 0, 0, 
		(proctable_fptr)CloneWindow , 
		"Clone Window ",  NULL}, 
	{"webview-Back",  NULL, 0, 
		"Web Navigate~5, Back~1", 0, 0, 
		(proctable_fptr)Back , 
		"Back",  NULL}, 
	{"webview-Forward",  NULL, 0, 
		"Web Navigate~5, Forward~2", 0, 0, 
		(proctable_fptr)Forward , 
		"Forward",  NULL}, 
	{"webview-Home",  NULL, 0, 
		"Web Navigate~5, Home~3", 0, 0, 
		(proctable_fptr)Home , 
		"Home",  NULL}, 
	{"webview-History",  NULL, 0, 
		"Web Navigate~5, History~10", 0, 0, 
		(proctable_fptr)History , 
		"History",  NULL}, 
	{"webview-Hotlist",  NULL, 0, 
		"Web Navigate~5, Hotlist~11", 0, 0, 
		(proctable_fptr)Hotlist , 
		"Hotlist",  NULL}, 
	{"webview-Add-Current-to-Hotlist",  NULL, 0, 
		"Web Navigate~5, Add Current to Hotlist~12", 0, 0, 
		(proctable_fptr)AddCurrent , 
		"Add Current to Hotlist",  NULL}, 
	{"webview-Internet-Starting-Points~13",  NULL, 0, 
		"Web Navigate~5, Internet Starting Points~13", 0, 0, 
		(proctable_fptr)InternetStartingPoints , 
		"Internet Starting Points",  NULL}, 
	{"webview-Internet-Resources-Meta-Index",  NULL, 0, 
		"Web Navigate~5, Internet Resources Meta Index~14",
				0, 0,  
		(proctable_fptr)MetaIndex, 
		"Internet Resources Meta Index",  NULL}, 
	{"webview-Annotate",  NULL, 0, 
		"Web Annotate~6, Annotate~1", 0, 0, 
		(proctable_fptr)Annotate , 
		"Annotate",  NULL}, 
	{"webview-Edit-Annotation",  NULL, 0, 
		"Web Annotate~6, Edit Annotation~2", 0, 0,  
		(proctable_fptr)EditAnnotation, 
		"Edit Annotation",  NULL}, 
	{"webview-Delete-Annotation",  NULL, 0, 
		"Web Annotate~5, Delete Annotation~3", 0, 0, 
		(proctable_fptr)DeleteAnnotation , 
		"Delete annotation",  NULL}, 
	NULL
};

	void 
webview::PostMenus(class menulist  * /* menulist */) {
	/* ignore child menus  XXX what justification is there for this? -robr */
	(this)->htmltextview::PostMenus( this->web_menus);
}

	int 
webview::InitializeClass() {
	struct proctable_Entry *tempProc;
	web_Menus = new menulist;
	bind::BindList(webBindings, NULL, web_Menus, 
		&webview_ATKregistry_ );

	if (tempProc = proctable::Lookup("frame-visit-file"))
		frame_VisitFile = proctable::GetFunction(tempProc);
	if (tempProc = proctable::Lookup("frame-new-window"))
		frame_NewWindow 
				= proctable::GetFunction(tempProc);
	lastPattern = NULL;

	return TRUE;
}


	static void 
phist(char  *s, struct webapp_webhist  *wp) {
	class web *w;
	class webcom *wc;
	size_t i;
	printf("history dump:\n");
	for(i = 0; i < wp->history.GetN(); i++) {
		w = (class web *)(wp->history[i])->GetData();
		wc = (w)->getwebcom();
		printf("%c [%ld] %s %s\n", (i == wp->hpos)?'>':' ', i,
		       (wc == NULL || wc->url==NULL || wc->url->Name()==NULL)? "NULL":(wc->url)->Name(),
			(wc == NULL || wc->title == NULL || wc->title->Name()==NULL)
				? "NULL":(wc->title)->Name());
	}
	printf("---\n");
}

buffer *pushbuf(buffer  *b, struct webapp_webhist  *wp) {
    size_t n=wp->history.GetN();
    size_t i;
    // grn... wp->hpos<n is only true after the first pushbuf right...? -robr
    if(wp->hpos<n) wp->hpos++;
    
    // clear out the old buffers.
    for(i=wp->hpos;i<n;i++) {
	wp->history[i]->Destroy();
    }
    // clear out the pointers to those old buffers
    if(wp->hpos<n) {
	wp->history.Remove(wp->hpos+1, n-wp->hpos-1);
	// or make space for the new buffer if needed...
    } else if(wp->hpos==n) wp->history.Append();
    else {
	fprintf(stderr, "web: pushbuf internal error... wp->hpos>n.\n");
	return b;
    }
    wp->history[wp->hpos]=b;
    return b;
}

buffer *popbuf(buffer *, struct webapp_webhist  *wp) {
    if(wp->hpos>0) {
	wp->hpos--;
	return wp->history[wp->hpos];
    } else return NULL;
}


buffer *fwdbuf(buffer *, struct webapp_webhist  *wp) {
    if(wp->hpos<wp->history.GetN()-1) {
	wp->hpos++;
	return wp->history[wp->hpos];
    } else return NULL;
}

webview::webview()
{
	ATKinit;
	const char *t;
	this->bt = NULL;
	this->bv = NULL;
	this->subview = NULL;
	this->lpMain = NULL;
	this->remoteim = NULL;
	this->web_menus = (web_Menus)->DuplicateML( this);
	if(starttm==NULL) {
	    if ((t = environ::GetProfile("WebHome")) != NULL)
		starttm=NewString(t);
	    else starttm=NewString(DEFAULTHOME);
        }
        mouseFocus=NULL;
	THROWONFAILURE( TRUE);
}

	void 
webview::SetRemote(class im  *im) {
	this->remoteim = im;
	(im)->AddObserver( this);
}

	void 
webview::ObservedChanged(class observable  *changed,  
			long  value) {
	if (changed == (class observable *) this->remoteim)  {
		if (value == observable_OBJECTDESTROYED) {
			/* XXX may want to destroy self here */
			this->remoteim = NULL;
		}
        }
        
        if(mouseFocus && changed==mouseFocus && value==observable_OBJECTDESTROYED) mouseFocus=NULL;
        
	(this)->htmltextview::ObservedChanged( changed,  value);
}

webview::~webview()  {
	ATKinit;
	if (this->bt) {
		(this->bt)->RemoveCallBackObserver(this);
#if 0
		if (this->lpMain) {
			(this->lpMain)->Destroy();
			this->lpMain = NULL;
		}
		if (this->bv) {
			(this->bv)->Destroy();
			this->bv = NULL;
		}
		(this->bt)->Destroy();
#endif
		this->bt = NULL;
	}
}

	void
webview::movehist(class view  *self, int  which,  class view  *v) {
	ATKinit;
	struct webapp_webhist *wp;
	class buffer *buffer, *cbuf;
	class web *web;
        class webcom *wc;
        frame *f=getframe(self);
        // clear any pending updates for the current document
        im::ForceUpdate();
	wp = webapp::getwebhist(self);
	if ((cbuf = getbuffer(self)) == NULL)
		return ;
	
	buffer = NULL;
	if (which == BACK) {
		if ((buffer = popbuf(cbuf, wp)) == NULL) return;
	}
	else if (which == FORWARD) {
		if ((buffer = fwdbuf(cbuf, wp)) == NULL) return;
	}
	else if (which >= 100) {
	    which = which - 100;
	    if(which<wp->history.GetN()) {
		wp->hpos=which;
		buffer=wp->history[wp->hpos];
	    }
	}
        if (buffer != NULL && f!=NULL) {
            f->SetBuffer(buffer, TRUE);
            web =(class web *) (buffer)->GetData();
            if ((web)->GetLength() == 0) {
                wc = (web)->getwebcom();
                web->webcomp = NULL;
                (web)->SetWebcom(wc);
            }
        }
}

static	void 
webview_back(class view  *v, enum view_MouseAction  action,  
				view  *self) {
	if (action == view_LeftUp) {
		webview::movehist(self, BACK, v);
	}
	else if (action == view_RightUp) {
		webview::movehist(self, FORWARD, v);
	}
}

#ifndef TEXTVIEWREFCHAR
#define TEXTVIEWREFCHAR ((unsigned char)255)
#endif

extern dataobject *NewFixupText(dataobject *obj);
// FixupBuffer will examine the buffer's data, if it is a text with only whitespace,
// hidden insets, and one table inset it will replace the text with the table.
static void FixupBuffer(buffer *buf) {
#if 0
    dataobject *obj=buf->GetData();
    dataobject *nobj=NewFixupText(obj);
    if(obj!=nobj) {
        printf("using table instead of text!\n");
        buf->SetData(nobj);
        buf->SetDestroyData(TRUE);
        buf->SetDefaultViewname("htablev");
        obj->Destroy();
    }
#endif
    
}


void webview_handleload(class view  *self, class webcom	 *ww, int  )
{
    class frame *frame;
    class buffer *buffer;
    class dataobject *dd;
    struct webapp_webhist *wp;
    // Clear out any pending updates, e.g. for the document which is about to be replaced.
    // (in case that document might be destroyed before we would otherwise perform the updates)
    im::ForceUpdate();
    if (ww->status &  WEBCOM_Loaded) {
	char *obtype;
	obtype = (ww)->CanView();
	if (obtype) {
	    FILE *f = NULL;
	    struct attributes *attrs=NULL;
	    long objectID = 0;
	    int isweb = (strcmp(obtype, "web") == 0);
	    frame = getframe(self);
	    if (frame) {
		wp = webapp::getwebhist(self);
		f = (ww)->Open(ww->raw || !isweb);
                if(ww->raw) obtype="text";
                else {
                    if ( ! isweb) {
                        if (f /*&&strcmp(obtype, "atk")== 0*/) {
                            obtype = filetype::Lookup(f,(ww)->GetURL(), &objectID, &attrs);
                            if (obtype == NULL) {
                                obtype = "text";
                            }
                            if(objectID!=0) {
                                int c;
                                while((c=getc(f))!='\n' && c!=EOF);
                            }
                        }
                    }
                }
                class buffer *old=frame->GetBuffer();
                dd= (class dataobject *)
                  ATK::NewObject(obtype);

                if(dd==NULL) {
                    message::DisplayString(self, 100, AString()<<"Couldn't create "<<obtype<<" object for URL "<<ww->GetURL());
                    return;
                }
                
                buffer = buffer::Create((ww->url)->Name(),
                                        NULL, NULL, dd);
                buffer->SetScratch(TRUE);
                if(!ww->reload) pushbuf(buffer, wp);
                
                (buffer)->SetName((ww)->GetURL());

                ATK_CLASS(web);

                isweb=dd->IsType(class_web);
		if (isweb) {
		    ((class web *) dd)->SetWebcom(ww);
                }
                ww->SetWeb(dd);
                if (f != NULL) {
                    if(attrs) dd->SetAttributes(attrs);
                    (dd)->Read(f, objectID);
                    (ww)->Close(f);
                    if(isweb) {
                        web *wd=(web *)dd;
                        if(wd->Title()) {
                            ww->title= atom::Intern(wd->Title());
                        } else ww->title=ww->url;
                    } else ww->title=ww->url;
                }
                FixupBuffer(buffer);
                
                (frame)->SetBuffer(buffer, TRUE);
                
                if(ww->reload) {
                    if(old) old->Destroy();
                    wp->history[wp->hpos]=buffer;
                }
                
	    }
	}
	else (ww)->ExternalView();
    }
    else  if (ww->status &  WEBCOM_LoadFailed) {
	message::DisplayString(self,  0,  (ww)->Error());
    }
}

	int 
webview::VisitWeb(class view  *self, char  *url, int r, const char *postdata) {
	ATKinit;
	class dataobject *w= (self)->GetDataObject();
        webcom *ww=NULL;
        if(r!=WEBCOM_Reload && r!=WEBCOM_ViewRaw && r!=WEBCOM_TopLevel) ww=webcom::FindWebcomFromData(w);
	if(url[0]=='#') {
	    const char *oldurl=NULL;
	    if(ww->url) oldurl=ww->url->Name();
	    char *copy=(char *)malloc(strlen(url)+(oldurl?strlen(oldurl):0));
	    if(oldurl) {
		strcpy(copy, oldurl);
		char *p=strrchr(copy, '#');
		if(p) {
		    strcpy(p, url);
		} else strcat(copy, url);
	    } else strcpy(copy, url);
            ww->url=atom::Intern(copy);
            ATK_CLASS(webview);
            if(self->IsType(class_webview)) {
                webview *wv=(webview *)self;
                wv->MoveToAnchor();
            }
	    return 0;
	}
        ww = webcom::Create(url, ww , r, postdata);
	(ww)->Load((webcom_cbptr)webview_handleload, self);
	return 0;
}

struct findimagerock {
    htmltextview *tv;
    htmlimagev *target;
};

static boolean FindImage(htmltext *self, long pos, htmlenv *env, dataobject *dobj, arbval rock) {
    if(env && env->type==environment_View) {
	class viewref *ref=env->data.viewref;
	if(ref==NULL) return TRUE;
	findimagerock *fir=(findimagerock *)rock.obj;
	class view *vw=(class view *)dictionary::LookUp(fir->tv,(char *)ref);
	if(fir->target->IsAncestor(vw)) return TRUE;
    }
    return FALSE;
}

class view *webview::Hit(enum view_MouseAction  action, long  x,  long  y,  long  numberOfClicks) {
    ATK_CLASS(htmltext);
    ATK_CLASS(htmltextview);
    ATK_CLASS(htmlimagev);
    class web *web = (class web *) this->view::dataobject;
    long pos;
    char *s=NULL;
    view *hit=NULL;

    view *nf=NULL;
    if(mouseFocus && mouseFocus!=this) {
        graphic *drawable=GetDrawable();
        long mx=physical_LogicalXToGlobalX(drawable, x);
        long my=physical_LogicalYToGlobalY(drawable, y);
        nf=(mouseFocus)->Hit( action,                          physical_GlobalXToLogicalX( (mouseFocus)->GetDrawable(), mx),                          physical_GlobalYToLogicalY( (mouseFocus)->GetDrawable(), my),                          numberOfClicks);
    } else nf=htmltextview::Hit( action,  x,  y, numberOfClicks);

    if(nf->GetIM()!=GetIM()) {
        if(mouseFocus) mouseFocus->RemoveObserver(this);
        mouseFocus=NULL;
        return nf;
    }
        
    hit=nf;
    
    if(action==view_LeftUp || action==view_RightUp || action==view_UpMovement) nf=NULL;
    
    if(nf!=mouseFocus && (action==view_LeftDown || action==view_RightDown || nf==NULL)) {
        if(mouseFocus) mouseFocus->RemoveObserver(this);
        if(nf) nf->AddObserver(this);
        mouseFocus=nf;
    }
    
    if(hit==NULL || web->Imbedded()) return this;
    
    if(numberOfClicks!=1 || action!=view_LeftUp) return this;
    
    if(hit->IsType(class_htmlimagev)) {
	htmlimagev *iv=(htmlimagev *)hit;
	htmltextview *tv=iv->HTMLTextView();
	htmltext *t=(htmltext *)tv->GetDataObject();
	iv->GetHitPos(&x, &y);
	findimagerock fir;
	fir.tv=tv;
	fir.target=iv;
        arbval dumb;
        dumb.real=0.0;  // for purify
	dumb.obj=(pointer)&fir;
	pos=t->EnumerateInsets(FindImage, dumb);
	if(pos>=0) {
	    s=t->GetURL(pos, x, y);
	}
    } else if(hit->IsType(class_htmltextview)) {
	htmltextview *tv=(htmltextview *)hit;
	htmltext *t=(htmltext *)tv->GetDataObject();
	s=t->GetURL(tv->HitPos());
    } else return this;
    if(s==NULL) return this;
    if (this->remoteim != NULL) {
	view *v = getwebview(this->remoteim);
	class im *im;
	if (v && (v)->IsType(this)) {
	    /* xxx should we call webcom_Cancel here to 
	     flush que of unread images ??
	     If the read fails,  can we need to restart it?
	     Do we need to ?
	     struct webcom *wc;
	     if (web && (wc=(web)->getwebcom())) 
	     webcom_Cancel(wc, "Read canceled");
	     */
		   webview::VisitWeb(v, s);
		   im = (this)->GetIM();
		   if (this->remoteim)
		       (this->remoteim)->
			 RemoveObserver(this);
		   (im)->Destroy();
	}
	else
	    message::DisplayString(NULL, 0, 
				   "Can't locate remote view");
	
    }
    else webview::VisitWeb(this, s);
    return this;
}

	void
webview::WantInputFocus(class view  *requestor)  {
	/* if (requestor == (class view *)this) */
		(this)->htmltextview::WantInputFocus(requestor);
}

	class view *
webview::GetApplicationLayer() {
	class view *v = (this)->htmltextview::GetApplicationLayer();
	return v;
}


// If char c is found in s, it is replaced with  \0 and
// a ptr to the subsequent part of the original s is returned.
// Thus we can loop through a string
//    */ }
	static char *
trun(char  *s, char c) {
	if ((s = strchr(s, c)) != NULL)
		*s++ = '\0';
	return s;
}

	static int 
HLtoHTML(FILE  *in, FILE  *outf) {
	int cnt=0;
	char buf[1024], buf2[1024];
	fgets(buf, 1024, in);
	trun(buf, '\n');
/*	if (strcmp(buf, "ncsa-xmosaic-hotlist-format-1") != 0) {
		return 0;
	} */
	fgets(buf, 1024, in);	// skip second line
	while(fgets(buf, 1024, in)) {
		cnt++;
		trun(buf, ' ');
		trun(buf, '\t');
		fprintf(outf, "<A href=\"%s\">", buf);
		if (fgets(buf2, 1024, in))
			fprintf(outf, "%s</A><br>\n", buf2);
		else{
			fprintf(outf, "%s</A><br>\n", buf);
			break;
		}
	}
	fflush(outf);
	return cnt;
}

struct perlog{
	class atom *url;
	int *logs, cnt;
	struct perlog *next;
};

// never called 
	static struct perlog *
readlogs(char  *filename)  {
	FILE *f;
	char buf[1024], *c;
	int pt[256], *ip, cnt, *op;
	struct perlog *p, *last;
	if ((f = fopen(filename, "r")) == NULL) return NULL;
	fgets(buf, 1024, f);
	trun(buf, '\n');
	if (strcmp(buf,
		"ncsa-mosaic-personal-annotation-log-format-1") 
				!= 0) {
		return NULL;
	}
	fgets(buf, 1024, f);
	last = NULL;
	while(fgets(buf, 1024, f)) {
		// xxx I added  ", '\n'" twice.  No idea if this is right.
		for(c = trun(buf, '\n'), ip = pt; c; c = trun(c, '\n'), ip++) {
			*ip = atoi(c);
		}
		cnt = ip - pt;
		p = (struct perlog *)malloc(sizeof(struct perlog) );
		p->logs = (int *)malloc(sizeof(int) * cnt);
		p->cnt = cnt;
		for(op = p->logs,  ip = pt; cnt > 0; cnt--, op++, ip++)
			*op = *ip;
		p->url = atom::Intern(buf);
		p->next = last;
		last = p;
	}
	fclose(f);
	return last;
}

	// these two should be in text.C and exported as methods.
static class environment *CheckHidden(class text *self, long pos)
{
    class environment *env=(class environment *)self->rootEnvironment->GetInnerMost(pos);
    while(env!=NULL && env->type==environment_Style) {
	struct style *s=env->data.style;
	if(s && s->IsHiddenAdded()) break;
	env=(struct environment *)env->GetParent();
    }
    if(env && env->type==environment_Style) return env;
    return NULL;
}


static long ReverseNewline(class text *self, long pos)
{
    long tp;
    for (tp = pos-1; tp >= 0; tp--) {
	unsigned char c = self->GetChar(tp);

	if (c  == '\n' || c == '\r') {
	    class environment *env=CheckHidden(self, tp);
	    if(env) {
		tp=env->Eval();
		continue;
	    } else break;
	}
    }
    return tp;
}

void webview::MoveToAnchor() {
    web *w=(web *)GetDataObject();
    if(w->webcomp && w->webcomp->url && w->webcomp->url->Name()) {
	char *url=w->webcomp->url->Name();
	char *p=strrchr(url, '#');
	if(p) {
	    long pos=w->FindNamedAnchor(p+1);
	    if(pos>=0) {
		pos=ReverseNewline(w, pos);
		SetTopPosition(pos+1);
	    }
	}
    }
}

void webview::SetDataObject(class dataobject *obj) {
    htmltextview::SetDataObject(obj);
    MoveToAnchor();
}
