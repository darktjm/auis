/* Copyright  Carnegie Mellon University 1995 -- All rights reserved */

#include <andrewos.h>
ATK_IMPL("webcom.H")
#include <environ.H>
#include <message.H>
#include <atom.H>
#include <image.H>
#include <im.H>
#include <event.H>
#include <flex.H>
#include <util.h>

#include <webcom.H>
#include <webparms.h>

#ifndef towlower
#define towlower(A) (isupper(A)? tolower(A):A)
#endif

#define  GUESSAHEAD
static const class atom *xbm;
static const class atom *gif;
static const class atom *htmlatom, *plainatom,  *atkatom, *jpegatom;
static int kidpid;
static FILE *inf, *outf; // * errf
#define QUESIZE 512
#define MAXPEND 2
static class webcom *que[QUESIZE], **st, **en;
#define nextque(A) ((A + 1 >= que + QUESIZE)?que:A + 1)
static class webcom *firstcom;
static class webcom *startpending;
static char lastcontext[1024]; /* bogus */
static int INPROMPT;
static class event *web_event;

ATKdefineRegistry(webcom,  observable,  webcom::InitializeClass);

static void loadimg(class image  *dat, class webcom  *self, int  status);
static void hweb(FILE  *f,  void  *td);
static FILE *setuppipes();
static int webcom_countpending();
static void webcom_RemovePending(class webcom  *self);
static void webcom_AddPending(class webcom  *self);
static void endwait(struct wev  *w, long);
#if 0
#ifdef GUESSAHEAD
static int mystrncmp(char  *a, char  *b, int  n);
#endif
#endif
static void webcom_DoCallback(class webcom  *self, int  value);
static void webcom_Que(class webcom  *self, webcom_cbptr proc,
		void *rock);
static void webcom_CheckQue();


		

/* move file pointer past mime header 
*/
void webcom::ReadContext(FILE  *fp, int  id)  {
    int state=0;
    int c;
    while(!feof(fp)  && ((c=fgetc(fp))!=EOF)) {
	if(c=='\r') continue;
	if(state==0 && c=='\n') state=1;
	else if(state==1 && c=='\n') return;
	else state=0;
    }
}

	void 
webcom::SetContext(char  *begin) {
	unsigned int len = strlen(begin);
	if (this->context != NULL){
		if (strlen(this->context) > len){
			free(this->context);
			this->context = NULL;
		}
	}
	if (this->context == NULL) 
		this->context = (char *)malloc(len + 1);
	strcpy(this->context, begin);
	strcpy(lastcontext, begin);
}


	char *
webcom::getfullname(char  *name) {
	/*	return getfullname(name, self->context); */
	return name;
}


	static void 
loadimg(class image  *dat, class webcom  *self, int  status)  {
	FILE *f;
	if (self->data != (class dataobject *) dat){
		/* shouldn't happen */
		 printf("wrong callback made \n");
		 return;
	}
	switch(status){
		case  WEBCOM_LOADERROR:
			fprintf(stderr, "web: Image load failed \n");
			f = fopen(BROKENIMGFILE, "r");
			if (f)  (dat)->Load( NULL,  f);
			break;
		case WEBCOM_SUCCESS_WEB:
		    if(self->file) f = fopen(self->file, "r");
		    else f=NULL;
			if (f) {
				// xxx should call webcom_Load here 
				// and read later
				// xxx also,  look for width and height
				// in url and set image size acordingly 
				(self)->ReadContext(f, 0);
				(dat)->Load( NULL,  f);
				(dat)->NotifyObservers(image_NEW);
			}
			else{
				// error,  couldn't load the file,  even though
				// status indicates it is available;
				// xxx could try reloading
				fprintf(stderr, "web: ERROR reading %s\n", self->file);

				//  WEBCOM_ClearLoadStatus(self);
				// self->Load((webcom_cbptr)loadimg, 
				//		(void *)dat);
			}
			break;
		default:
			break;
	}
}

	class image * 
webcom::getimage (char  *url, char  *type)  {
	class image* dat;
	class webcom *new_c;
	new_c = webcom::Create(url, this, 0, NULL);
	if (new_c == NULL) return NULL;
	dat = (class image*) ATK::NewObject("imageio");
	if (dat) {
		new_c->data = (class dataobject *) dat;
		if (new_c->status & WEBCOM_Loaded){
			loadimg(dat, new_c, 
				WEBCOM_SUCCESS_WEB);
		}
		else {
			// we are pretty sure what we got, 
			// que the load but load the file offline
			webcom_Que(new_c, (webcom_cbptr)loadimg, 
						(void *)dat);
			// xxx need to look for width and height info 
			// in url and set image size acordingly 
		}
		return dat;
	}
	return NULL;
}

	char *
	  webcom::GetTmpName()  {
	    return file;
}

static void HandleProgress(webcom *self, char *buf) {
    message::DisplayString(NULL, 0, buf);
    im::ForceUpdate();
}

static void HandleLoaded(webcom *self, char *buf) {
    self->status|=WEBCOM_Loaded;
    if(*buf) {
	if(self->file) free(self->file);
	self->file=strdup(buf);
    }
    webcom_DoCallback(self, WEBCOM_SUCCESS_WEB);
    webcom_CheckQue();
    message::DisplayString(NULL, 0, "Done.");
    im::ForceUpdate();
}

static void HandleDead(webcom *self, char *buf) {
    webcom **ptr=&startpending;
    while(*ptr && *ptr!=self) ptr=&((*ptr)->nextpending);
    if(*ptr==self) *ptr=self->nextpending;
    unlink(self->file);
    unlink(self->errfile);
}

static void HandleRedir(webcom *self, char *buf) {
    unsigned int ul=atol(buf);
    char *p=buf;
    while(*p && !isspace(*p)) p++;
    if(*p) p++;
    if(strlen(p)>=ul && p[ul]==' ') {
	p[ul]='\0';
    }
    self->url=atom::Intern(p);
}

static void HandleError(webcom *self, char *buf) {
    flex out;
    if(self->url) {
	static const char msg[]="URL %s had the following error:\n";
	char *p=out.Insert((size_t)0, strlen(msg)+strlen(self->url->Name()));
	if(p) sprintf(p, msg, self->url->Name());
    }
    size_t dummy;
    out.Insert(out.GetN(), strlen(buf)+1);
    char *p=out.GetBuf(0, out.GetN(), &dummy);
    strcat(p, buf);
    message::DisplayString(NULL, 100, p);
#if 0
    self->status |= WEBCOM_LoadFailed;
    webcom_DoCallback(self, WEBCOM_LOADERROR);
    webcom_CheckQue();
#endif
}

static void HandleType(webcom *self, char *buf) {
    self->mimetype=atom::Intern(buf);
    self->status |= WEBCOM_HasType;
}

static void hweb(FILE  *f,  void  *td)  {
    char buf[10240];
    if(!feof(f)  && fgets(buf, sizeof(buf), f)) {
	char *p=strrchr(buf, '\n');
	if(p) *p='\0';
	unsigned long reqid=atol(buf+1);
	class webcom *self, *nextself;
	for(self = startpending; self != NULL; self = nextself){
	    nextself = self->nextpending;
	    if(self->reqid==reqid) break;
	}
	if(self==NULL) return;
	p=buf+1;
	while(*p && !isspace(*p)) p++;
	while(*p && isspace(*p)) p++;
	char *q=p;
	while(*q && !isspace(*q)) q++;
	if(*q) {
	    *q='\0';
	    q++;
	    while(*q && isspace(*q)) q++;
	}
	if(strcmp(p, "progress")==0) HandleProgress(self, q);
	else if(strcmp(p, "loaded")==0) HandleLoaded(self, q);
	else if(strcmp(p, "dead")==0) HandleDead(self, q);
	else if(strcmp(p, "redir")==0) HandleRedir(self, q);
	else if(strcmp(p, "error")==0) HandleError(self, q);
	else if(strcmp(p, "type")==0) HandleType(self, q);
    }
}

#if 0
#define HTALERT '1'
#define HTPROGRESS '9'
#define HTCONFIRM '2'
#define HTPROMPT '3'
#define HTPASSWORD '4'
#define HTUandP '5'

	static void 
eweb(FILE  *f,  void  *td)  {
	/* deal with messages from stderr */
	class webcom *self, *nextself;
	char obuf[512];
	int goteof=FALSE;
	char buf[1024], *c;
	int i = 3999;
	c = buf;
	INPROMPT = TRUE;   /*
		prevent exiting from inner loops to call Poll while 
		possible interactions with the server are going on */
	*c = getc(f);
	printf("*****");
	putchar(*c);
	c++;
	while(--i && FILE_HAS_IO(f)) {
		int ch = getc(f);		// was wrong xxx
		if (ch == EOF) {
			printf("GOT EOF!!!\n");
			goteof = TRUE;
			break;
		}
		*c = ch;
		putchar(*c);
		if (*c == '\n') break;
		c++;
	}
	*c = '\0';
	/*  test for username and password prompts */
	/* check for error nessages ,  relate to appropriate
	 webcom object
	 also look for user prompts (may come in above) */
	im::ForceUpdate();
	if (strncmp(buf, "WWW", 3) == 0){
		/* look for interaction codes from our 
				version of HTAlert.c */
		c = buf + 5;
		if (*c == ':') c++;
		switch(buf[3]){
			case HTALERT :
				/* Sigh,  special case where we are about to 
				be prompted for a new username and 
				password. There are probably other cases 
				where we don't want to cancel here */
				if (strcmp(buf, 
"WWW1 Alert:  Access without authorization denied -- retrying") 
							!= 0)
					webcom::Cancel(c);
				break;
			case HTPROGRESS :
				printf("in eweb\n");
				message::DisplayString(NULL, 0, c);
				break;
			case HTCONFIRM :  {
				static char *yesOrNo[] = {"Yes",  "No",
						NULL};
				long answer;
				message::MultipleChoiceQuestion (NULL,
					60, c, 0, &answer, yesOrNo,  NULL);
				if (answer == 1) fprintf(outf, "N\n");
				else fprintf(outf, "Y\n");
				}  break;
			case HTPROMPT :
				printf("in HTPROMPT code %s\n", c);
				if (message::AskForString(NULL, 
						70, c, "", obuf, 512) < 0){
					fprintf(outf, "\n");
					printf("canceled HTPROMPT code %s\n", c);
				}
				else  fprintf(outf, "%s\n", obuf);
				
				break;
			case HTPASSWORD :
				if (message::AskForPasswd(NULL, 70, c, "",
						obuf, 512) < 0)
					fprintf(outf, "\n");
				else  fprintf(outf, "%s\n", obuf);
				break;
			case HTUandP:
				printf("in HTPROMPT code %s\n", c);
				if (message::AskForString(NULL, 70, 
						c, "", obuf, 512) < 0){
					fprintf(outf, "\n");
					printf("canceled HTPROMPT code '%s'\n", c);
				}
				else {
					printf("HTPROMPT sending '%s'\n",
						obuf);
					fprintf(outf, "%s\n", obuf);
				}
				if (message::AskForPasswd(NULL, 70,
						"Password", "", obuf, 512) < 0)
					fprintf(outf, "\n");
				else {
					printf("HTPROMPT sending '%s'\n", obuf);

					fprintf(outf, "%s\n", obuf);
				}
				break;

			default:
				printf("ERROR - unknown interaction code %s\n", buf);
				break;
		}
		fflush(outf);

	}
	if (FILE_HAS_IO(f)) 
		eweb(f, td);
	INPROMPT = FALSE;
/*	webcom_PollAll(); */  /* May need */
	if(goteof){
		printf("Restarting Server\n");
		setuppipes();
	}
	}
#endif
        

static	FILE *setuppipes();
static void zombie(int pid, long data, WAIT_STATUS_TYPE *zs) {
    fprintf(stderr, "\"awww\" exited.  Restarting.\n");
    message::DisplayString(NULL, 100, "\"awww\" exited.  Restarting.\n");
    im::ForceUpdate();
    sleep(1);
    fclose(inf);
    fclose(outf);
//    fclose(errf);
    setuppipes();
}

static	FILE *
setuppipes()  {
	int towww[2];
	int fromwww[2];
//	int errwww[2];
	pipe(towww);
	pipe(fromwww);
//	pipe(errwww);
	if ((kidpid = fork()) == 0){
		const char *foo[2];
		close(0);
		close(1);
//		close(2);
		/*	 write on fromwww[1]
			 read on towww[0]
		 errors on errwww[1] */
//		dup2(errwww[1], 2);
//		close(errwww[0]);
		dup2(fromwww[1], 1);
		close(fromwww[0]);
		dup2(towww[0], 0);
		close(towww[1]);
		foo[1] = NULL;
		/* not defined, try "awww" */
		foo[0] = (char *)environ::AndrewDir("/etc/awww");
		execvp(*foo, (char **)foo);

		foo[0] = "awww";
		execvp(*foo, (char **)foo);

		/* nope, no awww */
		fprintf(stderr, "web: Cannot find \"awww\", %s\n",
				"the web interface");
		exit(9);
	} else if(kidpid>0) {
//	    close(errwww[1]);
	    close(fromwww[1]);
	    close(towww[0]);
	    im::AddZombieHandler(kidpid, zombie, 0);
	    inf = fdopen(fromwww[0], "r");
	    outf = fdopen(towww[1], "w");
//	    errf =  fdopen(errwww[0], "r");
	    im::AddFileHandler(inf, (im_filefptr)hweb, NULL, 0); 
//	    im::AddFileHandler(errf, (im_filefptr)eweb, NULL, 0);
	} else {
	    outf=NULL;
	    fprintf(stderr, "web: Couldn't fork \"awww\".\n");
	}
	return outf;
}
	void 
webcom::KillChild(int restart)  {
	ATKinit;
	kill(kidpid, 9);
	if(restart){
		setuppipes();
	}
}


	int 
webcom::InitializeClass()  {
	struct timeval foo;
	int seed;
	web_event = NULL;
	gettimeofday(&foo, NULL);
	startpending = NULL;
	seed = getpid() +  foo.tv_sec + foo.tv_usec; 
	SRANDOM(seed);
        gif = atom::Intern("image/gif");
        jpegatom=atom::Intern("image/jpeg");
	xbm = atom::Intern("image/x-xbitmap");
	htmlatom = atom::Intern("text/html");
	plainatom = atom::Intern("text/plain");
	atkatom = atom::Intern("application/andrew-inset");
	setuppipes();
	if (outf == NULL) {
		fprintf(stderr, "web: Error ,  cannot open awww\n");
	}
	firstcom = NULL;
	st = en = que;
	INPROMPT = FALSE;
	return TRUE;
}

webcom::webcom()  {
	ATKinit;
	reqid=0;
	this->context = NULL;
	this->url = NULL;
	this->next = firstcom;
	this->status = WEBCOM_NoStatus;
	firstcom = this;
	this->parent = NULL;
	this->nextpending = NULL;
	this->data = NULL;
	this->context = NULL;
	this->type = NULL;
	this->url = NULL;
	this->proc = NULL;
	this->procdata = 0;
	this->mimetype = NULL;
	this->title = NULL;
	file=NULL;
        errfile=NULL;
        raw=FALSE;
        reload=FALSE;
        postfile=NULL;
	THROWONFAILURE( TRUE);
}

	class webcom *
webcom::FindWebcomFromData(class dataobject  *dat)  {
	ATKinit;
	class webcom *w;
	for(w = firstcom; w != NULL; w = w->next){
		if (w->data == dat) return w;
	}
	return NULL;
}

webcom::~webcom() {
    ATKinit;
    if(file) {
	unlink(file);
	free(file);
    }
    if(errfile) {
	unlink(errfile);
	free(errfile);
    }
    if(postfile) {
        unlink(postfile);
        free(postfile);
    }
    
    if (this->context != NULL) free(this->context);
    webcom_RemovePending(this);
    if (firstcom == this) firstcom = this->next;
    else {
	class webcom *w;
	for (w = firstcom ;w->next != this; w = w->next);
	w->next = this->next;
    }
}		

	class webcom *
webcom::CreateFromStream(FILE  *f)  {
	// xxx webcom::CreateFromStream
	    ATKinit;
	    return NULL;
}


	void
webcom::Cancel(const char  *buf){
	ATKinit;
	class webcom *self, *nextself;
	for(self = startpending; self != NULL; self = nextself){
		nextself = self->nextpending;
		self->status |= WEBCOM_LoadFailed;
		if(self->file) unlink(self->file);
		if(self->errfile) unlink(self->errfile);
		self->context = strdup(buf);
	}
	message::DisplayString(NULL, 0, buf);
	im::ForceUpdate();
}

	class webcom *
webcom::Create(const char  *url, class webcom  *parent, int  flags, const char *postdata)  {
	ATKinit;
	class webcom *w;
	const class atom *atm;
        atm=atom::Intern(url);
        w = new webcom;
        w->url = atm;
        w->parent = parent;
        if(flags&WEBCOM_ViewRaw) w->raw=TRUE;
        else w->raw=FALSE;
        if(flags&WEBCOM_Reload) w->reload=TRUE;
        else w->reload=FALSE;
        if(flags&WEBCOM_Post) {
	    w->postfile=(char *)malloc(strlen(P_tmpdir) + 10);
	    sprintf(w->postfile, "%s/pfXXXXXX", P_tmpdir);
	    int fd = mkstemp(w->postfile);
            FILE *fp=fdopen(fd, "w");
            if(fp==NULL) {
                fprintf(stderr, "web: Couldn't write %s for posting.\n", w->postfile);
                return w;
            }
            unsigned int wb=fwrite(postdata, 1, strlen(postdata), fp);
            if(wb<strlen(postdata)) {
                fprintf(stderr, "web: Short write of %s for posting.\n", w->postfile);
                return w;
            }
            if(fclose(fp)!=0) {
                fprintf(stderr, "web: Write of %s for posting failed.\n", w->postfile);
            }
        }
	return w;
}

	char *
webcom::GenPass(char  *name , char  *passwd)  {
	ATKinit;
	// xxx GenPass
	return NULL;
}

	char *
webcom::promptForPasswd(class view  *view)  {
	ATKinit;
	// xxx promptForPassword
	return NULL;
}

	int 
webcom::SetCacheSize(int  count)  {
	ATKinit;
	// xxx SetCacheSize
	return 0;
}

	int 
webcom::GetCacheSize(int  count)  {
	ATKinit;
	// xxx GetCacheSize
	return 0;
}

	int 
webcom::PromptForURL(class view  *v)  {
	ATKinit;
	// xxx PromptForURL
	return 0;
}

	int 
webcom::GoTo(class textview  *v)  {
	ATKinit;
	// xxx webcom::GoTo
	return 0;
}

	int 
webcom::Display(class view  *v)  {
	// xxx webcom::Display
/*
	if (ISVIEWER(self)){
		return webcom_Load(self, NULL, NULL);
	}
	 if ((r = webcom_GetRemote(self)) == NULL){
		message_
*/
	return 0;
}

	static int 
webcom_countpending()  {
	class webcom *w;
	int i = 0;
	for(w = startpending; w != NULL; w = w->nextpending){
		i++;
	}
	return i;
}

	static void 
webcom_RemovePending(class webcom  *self)  {
	    class webcom *w;
	if (startpending == self) 
		startpending = self->nextpending;
	else for(w = startpending; w != NULL; w = w->nextpending)
		if (w->nextpending == self) {
			w->nextpending = self->nextpending;
			break;
		}
	self->nextpending = NULL;
}

	static void 
webcom_AddPending(class webcom  *self)  {
	self->nextpending = startpending;
	startpending = self;
}


struct wev {
	int inwait;
	class event *e;
};

	static void 
endwait(struct wev  *w, long)  {
	w->e  = NULL;
	w->inwait = 0;
}

	int 
webcom::Wait(class view  *v)  {
	struct wev w;
	while((this->status&(WEBCOM_LoadFailed|WEBCOM_Loaded))==0){
		w.inwait = 1;
		w.e = im::EnqueueEvent((event_fptr)endwait, 
				(char *)&w, event_SECtoTU(1));
		while((INPROMPT || w.inwait == 1) // tjm - not sure if this is what was intended (was a || (b && c))
				&& (this->status&
				(WEBCOM_LoadFailed|WEBCOM_Loaded))
					 == 0){
			im::Interact(TRUE);
		}
		if (w.e != NULL) (w.e)->Cancel();
	}
	if (this->status & WEBCOM_LoadFailed )
	    return WEBCOM_LOADERROR;
	if (this->status & WEBCOM_Loaded) 
	    return  WEBCOM_SUCCESS_WEB;
	return WEBCOM_LOADERROR;
}

#if 0
#ifdef GUESSAHEAD
	static int 
mystrncmp(char  *a, char  *b, int  n)  {
	while(n-- > 0){
		if (towlower(*a) != towlower(*b)) return 1;
		a++; b++;
	}
	return 0;
}
#endif
#endif

	class web *
webcom::GetWeb()  {
	    // xxx webcom::GetWeb
	    return NULL;
}

	class dataobject *
webcom::GetObject()  {
	return this->data;
}

	void 
webcom::SetWeb(dataobject  *pweb)  {
	this->data = pweb;
}

	class buffer *
webcom::MakeBuffer(class frame  *frame, boolean  browse)  {
	    // xxx webcom::MakeBuffer
	    return NULL;
}

	void 
webcom::Adopt(class webcom  *child)  {
	// xxx webcom::Adopt
}

	void 
webcom::Cull()  {
	// xxx webcom::Cull
}

	void 
webcom::SetPasswd(char  *passwd)  {
	// xxx webcom::SetPasswd
}

	char *
webcom::GetPasswd()  {
	    // xxx webcom::GetPasswd
	    return NULL;
}

	class webcom *
webcom::GetParent()  {
	    // xxx webcom::GetParent
	    return NULL;
}

	int 
webcom::ReadWeb()  {
	    // xxx webcom::ReadWeb
	    return 0;
}

	char *
webcom::GetParentURL()  {
	    // xxx webcom::GetParentURL
	    return NULL;
}

	long 
webcom::Status()  {
		return this->status;
}

	void 
webcom::SetPreserve(boolean  doit)  {
	// xxx webcom::SetPreserve
}

	long 
webcom::Size()  {
	    // xxx webcom::Size
	    return 0;
}

	char *
webcom::Error()  {
	return this->context;
}

	const char *
webcom::GetURL()  {
	return (this->url)->Name();
}

	const char *
webcom::GetTitle ()  {
	    // xxx webcom::GetTitle
	    return NULL;
}

	void 
webcom::SetTitle (const char  *title)  {
	// xxx webcom::SetTitle
}

	FILE *
webcom::Open(boolean readcontext)  {
	FILE *f;
	if (file==NULL || (f = fopen(file, "r")) == NULL) return NULL;
	if(readcontext) (this)->ReadContext(f, 0);
	return f;
}

	void 
webcom::Close(FILE  *f)  {
	fclose(f);
}

	const class atom * 
webcom::Type()  {
	return this->mimetype;
}

	int 
webcom::ExternalView()  {
	char buf[1024];
#if 0
	if (this->mimetype == plainatom 
			|| this->mimetype == atkatom){
		/*	xxx webcom::ExternalView	??? */
	}
	else
#endif
	{
		sprintf(buf, "metamail -d -z %s&",
				file);
		system(buf);
	}
	return 0;
}

	const char * 
webcom::CanView()  {
	if (this->mimetype == htmlatom) return "web";
	else  if (this->mimetype == plainatom) 
		return "text";
	else if (this->mimetype == atkatom){
		return "atk";
        } else if(mimetype==gif || mimetype==jpegatom || mimetype==xbm) {
            return "imageio";
        }
	else return NULL;
}

	int 
webcom::Delete()  {
	    // xxx webcom::Delete
	    return 0;
}

	int 
webcom::Reload()  {
	    // xxx webcom::Reload
	    return 0;
}

void webcom_DoCallback(class webcom  *self, int  value)
{
	if (self->proc){
		(*self->proc)(self->procdata, self, value);

//	xxx	TEMP HACK ******
//	 (*self->NotifyProc)(self->rock, self, value); 
//		loadimg(self->rock, self, value); 

	}
}

	static void 
webcom_Que(class webcom  *self, webcom_cbptr proc, 
		void  *rock) {
	/* like webcom load ,  but lower priority
	 code tries to prevent too many request  going out
	 at once */
	if (INPROMPT || 
			(webcom_countpending() >= MAXPEND)){
		*en = self;
		self->proc = proc;
		self->procdata = rock;
		self->status |= WEBCOM_LoadPending; /* 
				xxx should make new flag */
		en = nextque(en);

	}
	else {
		(self)->Load(proc, rock);
	}
}

	static void 
webcom_CheckQue()  {
	if (!INPROMPT) {
		while(webcom_countpending() < MAXPEND 
					&& st != en) {
			(*st)->Load((*st)->proc, (*st)->procdata);
			st = nextque(st);
		}
	}
}
	void 
webcom::Load(webcom_cbptr proc, void *rock)  {
	const char *local_url = (this->url)->Name();
	if (INPROMPT) webcom_Que(this, proc, rock);
	
	this->proc = proc;
	this->procdata = rock;
	file = (char *)malloc(strlen(P_tmpdir) + 10);
	errfile = (char *)malloc(strlen(P_tmpdir) + 10);
	if(file==NULL || errfile==NULL) {
	    fprintf(stderr, "web: Out of memory loading %s\n", local_url);
	    if(file) {
		free(file);
		file=NULL;
	    }
	    if(errfile) {
		free(errfile);
		errfile=NULL;
	    }
	}
	sprintf(file, "%s/wcXXXXXX", P_tmpdir);
	sprintf(errfile, "%s/weXXXXXX", P_tmpdir);
	int tfd = mkstemp(file);
	if(tfd >= 0) {
	    close(tfd);
	    tfd = mkstemp(errfile);
	    if(tfd >= 0)
		close(tfd);
	    else
		remove(file);
	}
	if(tfd < 0) {
	    fprintf(stderr, "web: Error creating temp files loading %s\n", local_url);
	    free(file);
	    free(errfile);
	    file = errfile = NULL;
	}
	static unsigned long cnt=1;
	reqid=cnt;
	// file://localhost
	char cdir[MAXPATHLEN+17 /* file: prefix, and '/' suffix + NUL */];
	const char *referer=NULL;
	if(parent && parent->url) {
	    referer=parent->url->Name();
	} else {
	    sprintf(cdir, "file://localhost");
            im::GetDirectory(cdir+strlen(cdir));
            strcat(cdir, "/");
	    referer=cdir;
        }
        const char *get;
        if(reload) get="reget";
        else if(postfile) {
            get="post";
        } else get="get";
        static int debug=-1;
        if(debug<0) debug=getenv("AW3CDEBUG")?1:0;
        if(debug>0) fprintf(stdout, "%ld %s %s %s %s %ld %s %ld %s\n", cnt, get, postfile?postfile:"", file, errfile, strlen(local_url), local_url, strlen(referer), referer);
        if((fprintf(outf, "%ld %s %s %s %s %ld %s %ld %s\n", cnt, get, postfile?postfile:"", file, errfile, strlen(local_url), local_url, strlen(referer), referer) == EOF) || (fflush(outf) == EOF)){
		fprintf(stderr, "OUTPUT PIPE BROKEN - will restart\n");
		setuppipes();
		sleep(1);
		if((fprintf(outf, "%ld %s %s %s %s %ld %s %ld %s\n", cnt, get, postfile?postfile:"", file, errfile, strlen(local_url), local_url, strlen(referer), referer) == EOF) || (fflush(outf) == EOF)){	
			fprintf(stderr, "OUTPUT PIPE STILL BROKEN - GIVING UP\n");
			return;
		}
		fflush(outf);
	}
	cnt++;

  	this->status |= WEBCOM_LoadPending;
	/* add to list of objects to be polled */
	webcom_AddPending(this);
}

static char dontcode[128]={
    0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,
    0,1,0,0,1,0,0,1,
    1,1,1,0,1,1,1,0,
    1,1,1,1,1,1,1,1,
    1,1,0,0,0,0,0,0,
    0,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,
    1,1,1,0,0,0,0,1,
    0,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,
    1,1,1,0,0,0,0,0
};

void webcom::EncodeForURL(flex &url, const char *fragment) {
    const unsigned char *p=(const unsigned char *)fragment;
    while(*p) {
        if(*p<128 && dontcode[*p]) {
            *url.Append()=*p;
        } else {
            if(*p==' ') {
                *url.Append()='+';
            } else if(*p=='\n') {
                char *r=url.Insert(url.GetN(), 6);
                strcpy(r, "%0D%0A");
            } else {
                char *r=url.Insert(url.GetN(), 3);
                sprintf(r, "%%%02X",*p);
            }
        }
        p++;
    }
}
