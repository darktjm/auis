/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1989 - All Rights Reserved      *
 *        For full copyright information see:'andrew/config/COPYRITE'     *
\* ********************************************************************** */

/*
	$Disclaimer: 
// Permission to use, copy, modify, and distribute this software and its 
// documentation for any purpose and without fee is hereby granted, provided 
// that the above copyright notice appear in all copies and that both that 
// copyright notice and this permission notice appear in supporting 
// documentation, and that the name of IBM not be used in advertising or 
// publicity pertaining to distribution of the software without specific, 
// written prior permission. 
//                         
// THE COPYRIGHT HOLDERS DISCLAIM ALL WARRANTIES WITH REGARD 
// TO THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF 
// MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL ANY COPYRIGHT 
// HOLDER BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL 
// DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, 
// DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE 
// OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION 
// WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
// 
//  $
*/

#ifndef NORCSID
#define NORCSID
static char rcsid[]="$Header: /afs/cs.cmu.edu/project/atk-src-C++/atk/ez/RCS/ezapp.C,v 3.8 1996/03/12 17:56:55 robr Stab74 $";
#endif


 

#include <andrewos.h> /* sys/types.h sys/file.h */
ATK_IMPL("ezapp.H")

#include <sys/errno.h>
#include <sys/param.h> /* For MAXPATHLEN. */

#include <attribs.h>

#include <im.H>
#include <graphic.H>
#include <view.H>
#include <frame.H>
#include <dataobject.H>
#include <buffer.H>
#include <text.H>
#include <textview.H>

#include <message.H>
#include <cursor.H>
#include <event.H>
#include <filetype.H>
#include <rect.h>
#include <init.H>
#include <environ.H>
#include <application.H>
#include <proctable.H>
#include <completion.H>

#include <ezapp.H>

#include <util.h>

static class cursor *waitCursor;
static char *procForDefaultWindow= NULL;

#define INITIALHELP "EZ Multi-Media Editor\n\n"

ATKdefineRegistry(ezapp, application, ezapp::InitializeClass);

static void StartupError(class text  *errorDoc, char  *string);
static boolean FindCkpBuffer(class buffer  *buffer, struct bestbuffer  *best);
static boolean CkpMessage(class view  *applicationView , class view  *targetView , class view  *inputFocusView, char  *message);
static void Checkpoint(long  dummyData);
static void SetBufferCkpLatency(class frame  *frame, long  key);
static void Startup(class frame  *frame);
static void addFile(class ezapp *self, char *name, char *newWinProc, boolean ro, int initline);
static void makeErrorBuf(class ezapp  *self);
static void GotoLine(class text  *text, class textview  *view, int  line);


ezapp::ezapp()
{
	ATKinit;

    this->initFile=TRUE;
    this->files=NULL;
    this->fileLink= &this->files;
    this->haveBufferInWindow = FALSE;
    this->first_im = FALSE;
    this->imp = NULL;
    this->framep = NULL;
    this->bufferp = NULL;
    this->errorBuffer = NULL;
    this->defaultObject = NULL;
    this->stdinput = FALSE;
    this->ezTitle = NULL;
    (this)->SetMajorVersion( 7);
    (this)->SetMinorVersion( 0);
    THROWONFAILURE( TRUE);
}

/* The checkpointing algorithm used here is based on the following notions:
 *
 * 1) At Most, one buffer will be checkpointed every T seconds.
 * 2) At most, a given buffer will be checkpointed once in an (a * T) second
 *    interval.
 * 3) Only buffers which have been modified since the last time they were
 *    checkpointed (or written) are checkpointed.
 * 4) All buffers that meet criterion 3 will be checkpointed before any buffer is
 *    checkpointed twice.
 * 5) Scratch buffers are not checkpointed.
 *
 * The algorithm is implemented using two fields in the buffer structure. The
 * first is the ckpVersion field, which holds the dataobject version number as of
 * the last checkpoint (or file write). The second is the ckpClock field which is
 * reset on each checkpoint (or file write), and is incremented every T seconds.
 * To find which buffer to checkpoint, take the one with the highest ckpClock
 * greater than a. If there are none greater than a, don't checkpoint any.
 *
 * A third field in the buffer structure is used to allow the user to specify
 * the checkpointing behavior on a per buffer basis. This field is called the
 * latency and is basically the number of clock intervals above (or below if
 * negative) the system should wait before checkpointing this buffer. The code
 * is somewhat hairier than it might be since the buffer package initializes
 * both the ckpClock and ckpLatency  fields to 0 so we bias it by the value of
 * CkpLatency. This is the price paid for generality in the buffer package.
 *
 * Currently T (CkpInterval) is 30 seconds and a (CkpLatency) is 4 (i.e. 2
 * minutes checkpoint latency). However, a can be changed on a per
 * buffer basis.
 */

static long CkpInterval; /* How often to run Checkpoint routine. */
#define DEFAULTCKPINTERVAL 30 /* Default for CkpInterval. */
static long CkpLatency; /* The minimum amount of time to wait to checkpoint a buffer. */
#define DEFAULTCKPLATENCY 4 /* Default fo CkpLatency. */
static long MaxCkpSize;	/* Max filesize we are willing to checkpoint. */
#define DEFAULTMAXCKPSIZE (2*1024*1024)	/* 2mb */


static void StartupError(class text  *errorDoc, char  *string)
{
    (errorDoc)->InsertCharacters( (errorDoc)->GetLength(), string, strlen(string));
}

struct bestbuffer {
    class buffer *bufferp;
    long bufferclock;
};

static boolean FindCkpBuffer(class buffer  *bufferp, struct bestbuffer  *best)
{
    if (!(bufferp)->GetScratch() && ((bufferp)->GetData())->GetModified() > (bufferp)->GetCkpVersion()) {
        if ((bufferp)->GetCkpClock() > best->bufferclock) {
            best->bufferp = bufferp;
            best->bufferclock = (bufferp)->GetCkpClock();
        }
        (bufferp)->SetCkpClock( (bufferp)->GetCkpClock() + 1);
    }
    return FALSE;
}

#define view_Visible(view) (!rectangle_IsEmptyRect(&(((class graphic *) view)->visualBounds)))

static boolean CkpMessage(class view  *applicationView , class view  *targetView , class view  *inputFocusView, char  *message)
        {
    if (inputFocusView == NULL) /* Handles case where input focus is not set... */
        inputFocusView = targetView;

    if (view_Visible(inputFocusView)) {
        message::DisplayString(inputFocusView, 0, message);
        return TRUE;
    }
    return FALSE;
}

static void Checkpoint(long  dummyData)
    {
    struct bestbuffer result;

    result.bufferp = NULL;
    result.bufferclock = CkpLatency - 1; /* (number + 1) * CKPINTERVAL seconds is how often a given buffer can be checkpointed. */

    buffer::Enumerate((bufferlist_efptr)FindCkpBuffer, (long) &result);
    if (result.bufferp != NULL) {
	int closeCode;
	dataobject *dobj = result.bufferp->GetData();
	if (dobj && dobj->IsType("text")) {
	    /* Get length if it's a text object. */
	    text *t = (text *)dobj;
	    long datalen = t->GetLength();
	    if (MaxCkpSize != 0 && datalen > MaxCkpSize) {
		char msg[4096];

		sprintf(msg, "Checkpoint:  %s is too large to checkpoint (over %d bytes).  See the CheckpointMaxSize preference.\n",
			result.bufferp->GetName(), MaxCkpSize);
		if (result.bufferp->Visible())
		    result.bufferp->EnumerateViews((buffer_evfptr)CkpMessage, (long) msg);
		/* Pretend we checkpointed it. */
		result.bufferp->SetCkpVersion(dobj->GetModified());
		/* Resume checkpointing as usual. */
		im::EnqueueEvent((event_fptr) Checkpoint, 0, event_SECtoTU(CkpInterval));
		return;
	    }
	}

        im::SetProcessCursor(waitCursor);
        if ((result.bufferp)->Visible())
	    (result.bufferp)->EnumerateViews((buffer_evfptr) CkpMessage, (long) "Checkpointing...");
        im::ForceUpdate();

        if ((closeCode = (result.bufferp)->WriteToFile( (result.bufferp)->GetCkpFilename(), 0)) >= 0) {
            (result.bufferp)->SetCkpVersion( ((result.bufferp)->GetData())->GetModified());
            (result.bufferp)->SetCkpClock( (result.bufferp)->GetCkpLatency());
        }

        if ((result.bufferp)->Visible())
	    (result.bufferp)->EnumerateViews((buffer_evfptr) CkpMessage, (long)(closeCode ? "Checkpoint Failed." : "Checkpointed."));
        im::SetProcessCursor(NULL);
    }
    im::EnqueueEvent((event_fptr) Checkpoint, 0, event_SECtoTU(CkpInterval));
}

static void SetBufferCkpLatency(class frame  *frame, long  key)
        {

    class buffer *bufout;
    char answer[20];

    if ((bufout = (frame)->GetBuffer()) == NULL)
        return;

/* This stuff is a little hairy. Basically, it is converting a latency number to
 * seconds and back again. A latency number is shifted from 0, or specifically,
 * the code waits (latency + CkpLatency) * CkpInterval seconds before
 * considering checkpointing this buffer. 
 */
    sprintf(answer, "%d", (CkpLatency - (bufout)->GetCkpLatency()) * CkpInterval);
    if (message::AskForString(frame, 0, "Minimum checkpoint time in seconds: ", answer, answer, sizeof(answer)) != -1) {

        long latencyIntervals = atoi(answer) / CkpInterval;

        (bufout)->SetCkpLatency( CkpLatency - latencyIntervals);
        (bufout)->SetCkpClock( CkpLatency - latencyIntervals); /* Force new checkpoint time to take effect now. */
    }
}

static void
bufferDirectory(class buffer  *buffer, char  *dir			/* Output: At least MAXPATHLEN, please. */)
        {
    char *slash, *fname = (buffer)->GetFilename();

    if ((fname != NULL) && (*fname != '\0')) {
        strcpy(dir, fname);
        slash = strrchr(dir, '/');
        if (slash != NULL)
            slash[1] = '\0';
    } else {
        im::GetDirectory(dir);
        strcat(dir, "/");
    }
}

extern int frame_VisitNamedFile(class frame *f, char *file, boolean b1, boolean b2);

/* Not static so it can be used from eza.c */
static int VisitFilePrompting(class frame  *self, char  *prompt, boolean  newWindow, boolean  rawMode)
                {
    char filename[MAXPATHLEN];
    class buffer *buffer;

    if ((buffer = (self)->GetBuffer()) == NULL)
        return -1;

    bufferDirectory(buffer, filename);

    if ((completion::GetFilename(self, prompt, filename, filename,
			       sizeof(filename), FALSE, rawMode) == -1) || (filename[0]=='\0')) {
        errno = 0;
	message::DisplayString(self, 100, "Using a scratch buffer.");
	buffer=buffer::Create("Scratch", NULL, NULL, NULL);
	if(buffer==NULL) return 0;
	self->SetBuffer( buffer, /*FALSE was silly -RSK*/TRUE);
	return 0;
    }
    return frame_VisitNamedFile(self, filename, newWindow, rawMode);
}
static void Startup(class frame  *frame)
    {
    class buffer *bufout;
    long count = 0;
    int cc;
/* Loop until we have a file in a buffer. */
    while (VisitFilePrompting(frame, "File to edit [or ^G for none]: ", FALSE, FALSE) < 0) {

        boolean first = TRUE;

/* This next code handles a mouse (or menu) hit.
 * We must process it to get rid of it, and then make the view
 * give up the input focus it may have aquired. The loop is
 * so the error message doesn't time out and get erased.
 */
        do {

            if (!first) {
	        im::Interact(FALSE);
                ((frame)->GetView())->LoseInputFocus();
            }
            else
                first = FALSE;

            switch (errno) {
                case EACCES:
                    message::DisplayString(frame, 0, "File not found; could not create. Permission denied. Press ctrl-C to quit, or any other key to continue.");
                    break;
#ifdef AFS_ENV
                case ETIMEDOUT:
                    message::DisplayString(frame, 0, "File not found; could not create. A server is down. Press ctrl-C to quit, or any other key to continue.");
                    break;
#endif /* AFS_ENV */
                case 0: /* Message line was punted. */
                    message::DisplayString(frame, 0, "You must specify a file. Press ctrl-C to quit, or any other key to continue.");
                    break;
                default:
                    message::DisplayString(frame, 0, "File could not be created. Press ctrl-C to quit, or any other key to continue.");
            }
	    if(count++ > 10) /* the user probably quit */ exit(0);
        } while ((cc = (((class view *) frame)->GetIM())->GetCharacter()) == EOF);
	if(cc == 3) exit(0);
	count = 0;
    }
    bufout = (frame)->GetBuffer();
    if (bufout->readOnly)
        message::DisplayString(frame, 0, "File is read only.");
    /* OS/2 access can't handle a NULL file name */
    else if (access(((bufout)->GetFilename()==NULL) ? "":(bufout)->GetFilename(), F_OK) >= 0)
	message::DisplayString(frame, 0, "Done.");
    else
	message::DisplayString(frame, 0, "New file.");
}

static void addFile(class ezapp *self, char *name, char *newWinProc, boolean ro, int initline)
{
    /* Its a file right? */
    struct ezapp_fileList *fileEntry=
      (struct ezapp_fileList *) malloc(sizeof(struct ezapp_fileList));

    fileEntry->filename=name;
    fileEntry->ObjectName=self->defaultObject;
    fileEntry->newWindowProc=newWinProc;
    fileEntry->readOnly=ro;
    fileEntry->initLine=initline;
    fileEntry->next=NULL;
    *self->fileLink=fileEntry;
    self->fileLink=(&(fileEntry->next));
}

boolean ezapp::ParseArgs(int  argc,char  **argv)
{
    int maxInitWindows;
    char *procNewWindow = NULL;
    boolean pendingReadOnly = FALSE;
    boolean pendingObject=FALSE;
    boolean pendingTitle=FALSE;
    int pendingInitLine=0;

    char *name=(this)->GetName();

    if(!(this)->application::ParseArgs(argc,argv))
	return FALSE;

    if(name!=NULL && strcmp(name,"ez")!=0 && ATK::IsTypeByName(name,"dataobject"))
	this->defaultObject=name;

    maxInitWindows=environ::GetProfileInt("MaxInitWindows", 2);
    if (maxInitWindows < 1)
	maxInitWindows = 1;

    while(*++argv!=NULL){
	if(**argv=='-')
	    switch((*argv)[1]){
		case '\0': /* read stdin */
		    stdinput = TRUE;
		    stdinputObject = defaultObject;
		    break;
		case 'h': /* a little help here, eh? */
		    fprintf(stderr, "Usage: %s [options] [[fileopts] [filename | -]]\n", name);
		    fprintf(stderr, "  Options:  -o obj     Set default dataobject (mostly useless)\n");
		    fprintf(stderr, "            -d         Don't fork\n");
		    fprintf(stderr, "            -fl -ni    Initfile options (see: help ez)\n");
		    fprintf(stderr, "            -fg color  Set foreground color\n");
		    fprintf(stderr, "            -bg color  Set background color\n");
		    fprintf(stderr, "            -geometry geom  Set size/location of windows\n");
		    fprintf(stderr, "            -pref pref:val  Set temporary preference\n");
		    fprintf(stderr, "            -appname name   Pretend to not be %s\n", name);
		    fprintf(stderr, "  FileOpts: -r         Readonly buffer\n");
		    fprintf(stderr, "            -w[:proc]  Put in a window [run proc there]\n");
		    fprintf(stderr, "            +num       Start cursor at line num\n");
		    return FALSE;
		    break;
		case 'o': /* Create a buffer on object */
		    pendingObject=TRUE;
		    break;
		case 'r': /* Next file should be readonly. */
		    pendingReadOnly = TRUE;
		    break;
		case 't': /* Next argument should be the title. */ /* This doesn't work anyway */
		    pendingTitle = TRUE;
		    break;
		case 'w': /* New window. */
		    procNewWindow = ""; /* empty string sufficient to be non-null */
		    if ((*argv)[2]==':' && (*argv)[3]!='\0') { /* specified a proc to be called in the new window, copy for posterity */
			char *parg= &((*argv)[3]);
			procNewWindow= (char *)malloc(strlen(parg)+1); /* this will get freed when the proc has been called */
			strcpy(procNewWindow, parg);
		    }
		    break;
		default:
		    fprintf(stderr,"%s: unrecognized switch: %s\n", (this)->GetName(), *argv);
		    return FALSE;
	    }
	else if (**argv=='+') {
	    pendingInitLine=atoi((*argv+1));
	}
	else{
	    if(pendingTitle) {
		this->ezTitle= *argv;
		pendingTitle=FALSE;
	    } else if(pendingObject){
		this->defaultObject= *argv;
		pendingObject=FALSE;
	    }else{
		boolean mustHaveWindow= (maxInitWindows-->0);
		addFile(this, *argv,
			(procNewWindow ? procNewWindow : (mustHaveWindow ? (char *)"" : NULL)),
			pendingReadOnly, pendingInitLine);
		procNewWindow=NULL;
		pendingReadOnly=FALSE;
		pendingInitLine=0;
	    }
	}
    }
    if (procNewWindow) /* new window proc wasn't used (memory leak); store globally for use by the DefaultStartUpFile or ScratchBuffer */
	procForDefaultWindow= procNewWindow;
    return TRUE;
}

static void makeErrorBuf(class ezapp  *self)
{
    self->errorBuffer = buffer::Create("Startup-Errors", NULL, "text", NULL);
    (self->errorBuffer)->SetScratch( TRUE);
}

void ezapp::ReadInitFile()
{
    makeErrorBuf(this);

    (this)->SetErrorProc((init_fptr)StartupError);
    (this)->SetErrorRock((long)(this->errorBuffer)->GetData());

    (this)->application::ReadInitFile();
}

static void GotoLine(class text  *text, class textview  *view, int  line)
{
    int argument, pos, endpos;
    register int count;

    pos = (text)->GetPosForLine( line);
    (view)->SetDotPosition( pos);
    endpos = (text)->GetEndOfLine( pos);
    if (line > 0) (view)->SetDotLength( endpos - pos);
    else (view)->SetDotLength( 0);

    (view)->FrameDot( pos);
    (view)->WantUpdate( view);

    return;
}

/* LoadProcClass is stolen from metax.c, return value changed for use in expressions */
static boolean LoadProcClass(char *partial)
{
    char class_c[100];
    int i;
    for(i = 0; (i < strlen(partial)) && (partial[i] != '-'); i++) class_c[i] = partial[i];
    if(i <= 99) {
	class_c[i]= '\0';
	(void) ATK::LoadClass(class_c);
	return TRUE;
    }
    return FALSE;
}

static void runProcInWindow(ezapp *self, view *v, char *pname)
{
    char error[280];
    long procarg= 0;
    struct proctable_Entry *proc;
    int i= 0;
    /* determine arg type, if any */
    while (i>=0)
	switch (pname[i]) {
	    case '\0':
		i= -1;
		break;
	    case '"':
		pname[i]= '\0';
		procarg= (long)pname+i+1;
		i= -1;
		break;
	    case '#':
		pname[i]= '\0';
		procarg= atol(pname+i+1);
		i= -1;
		break;
	    case '\'':
		pname[i]= '\0';
		procarg= pname[i+1];
		i= -1;
		break;
	    default:
		++i;
		break;
	}
    /* call proc with arg */
    if (((proc= proctable::Lookup(pname)) && proctable::Defined(proc)) || (LoadProcClass(pname) && (proc= proctable::Lookup(pname)) && proctable::Defined(proc)))
	switch ((self->imp->keystate)->DoProc(proc, procarg, v)) {
	    case keystate_NoProc:
		sprintf(error, "Could not load procedure '%s'.", pname);
		message::DisplayString(v, 0, error);
		break;
	    case keystate_TypeMismatch:
		sprintf(error, "Bad command '%s'.", pname);
		message::DisplayString(v, 0, error);
		break;
	}
    else {
	sprintf(error,"Unknown procedure '%s'.", pname);
	message::DisplayString(v, 0, error);
    }
}

boolean ezapp::Start()
{
    if(!(this)->application::Start())
	return FALSE;

    if(this->errorBuffer==NULL) /* ? */
	makeErrorBuf(this);

    if (!this->ezTitle)
	this->ezTitle=environ::GetProfile("WindowTitle");

    buffer::SetDefaultObject(this->defaultObject);
    if(this->files==NULL)  {
	char *defFile;

	if ((defFile = environ::GetProfile("DefaultStartUpFile")) != NULL && *defFile != '\0')  {
	    addFile(this, defFile, procForDefaultWindow?procForDefaultWindow:(char *)"", FALSE, 0);
	} else if (environ::GetProfileSwitch("DefaultIsScratchBuffer", TRUE)) {
	    /* Create a scratch buffer instead. */
	    this->bufferp = buffer::Create("Scratch", NULL, NULL, NULL);
	    if (this->bufferp != NULL) {
                if(!this->framep && (this->framep = new frame) == NULL) {
		    fprintf(stderr,"Could not allocate enough memory.\nexiting.\n");
		    return(FALSE);
		}
                (this->framep)->SetCommandEnable( TRUE);
                if(!this->imp && (this->imp = im::Create(NULL)) == NULL) {
		    fprintf(stderr,"Could not create new window.\nexiting.\n");
		    return(FALSE);
		}
                (this->imp)->SetView( this->framep);
		(this->framep)->SetTitle( this->ezTitle);
                (this->framep)->PostDefaultHandler( "message", (this->framep)->WantHandler( "message"));
                (this->framep)->SetBuffer( this->bufferp, TRUE);
		this->haveBufferInWindow = TRUE;
		if (procForDefaultWindow && procForDefaultWindow[0]) {
		    runProcInWindow(this, (this->framep)->GetView(), procForDefaultWindow);
		    free(procForDefaultWindow);
		    procForDefaultWindow= NULL;
		}
	    }
	}
    }
    return TRUE;
}

int ezapp::Run()
{
    struct ezapp_fileList *fileEntry, *next;
    class text *errtext;

    // This code has experimentally been moved from ezapp::Start to here.
    // Technically, it is better to do the initial buffer reading in start because
    // errors can be given to the user better for bad failures (such as no window).

    // BUT...creating an im right away gives the user the impression that the
    // startup is faster!  As a bonus, the user can place the window while files
    // are being read.  We do create the im before the fork because it might fail
    // and we'd be stuck with no place to report an error.
    if (!this->haveBufferInWindow) { /* we don't have a scratch buffer up already -RSK*/
	this->first_im = TRUE;
	this->imp = im::Create(NULL);	// Create initial im now to keep the user busy...
	if (this->imp == NULL) {
	    fprintf(stderr,"ez:  Could not create window -- exiting.\n");
	    return(FALSE);
	}
    }
    if(!(this)->Fork())
	return -1;

    im::ForceUpdate();		// get it visible!

    if(this->errorBuffer==NULL) /* ? just to be safe -RSK*/
	makeErrorBuf(this);
    errtext=(class text *)(this->errorBuffer)->GetData();


    for (fileEntry = this->files; fileEntry != NULL; fileEntry = next) {
        if(fileEntry->ObjectName != NULL){
            if(ATK::IsTypeByName(fileEntry->ObjectName,"dataobject"))
                buffer::SetDefaultObject(fileEntry->ObjectName);
            else {
                char errorMessage[MAXPATHLEN+200];
                sprintf(errorMessage,"%s is not a known dataobject\n",fileEntry->ObjectName);
                StartupError(errtext,errorMessage);
                this->bufferp = NULL;
                next = fileEntry->next;
                free(fileEntry);
                continue;
            }
        }
        this->bufferp = buffer::GetBufferOnFile(fileEntry->filename, fileEntry->readOnly ? buffer_ReadOnly : 0);

        if(fileEntry->ObjectName != NULL)
            buffer::SetDefaultObject(this->defaultObject);

        if (this->bufferp != NULL) {
            long version = ((this->bufferp)->GetData())->GetModified();

            (this->bufferp)->SetCkpClock( 0);
            (this->bufferp)->SetCkpVersion( version);
            (this->bufferp)->SetWriteVersion( version);
            if (fileEntry->newWindowProc) {
		class dataobject *d;
		class frame *f;
		class view *v;
                if((this->framep = new frame) == NULL) {
		    fprintf(stderr,"Could not allocate enough memory.\nexiting.\n");
		    return(FALSE);
		}
                (this->framep)->SetCommandEnable( TRUE);
		if (!this->first_im) {
		    if((this->imp = im::Create(NULL)) == NULL) {
			fprintf(stderr,"Could not create new window.\nexiting.\n");
			return(FALSE);
		    }
		}
		this->first_im = FALSE;
		(this->imp)->SetView( this->framep);
                (this->framep)->PostDefaultHandler( "message", (this->framep)->WantHandler( "message"));
                (this->framep)->SetBuffer( this->bufferp, TRUE);
                this->haveBufferInWindow = TRUE;

		/* go to line */
		d= (this->bufferp)->GetData();
		f= frame::GetFrameInWindowForBuffer(this->bufferp);
		v= (f)->GetView();

		if (v && d && v->IsType("textview") && d->IsType("text")) {
		    GotoLine((class text *)d, (class textview *)v, fileEntry->initLine);
		}

		/* call window-startup proc */
		if (fileEntry->newWindowProc[0]) {
		    runProcInWindow(this, v, fileEntry->newWindowProc);
		    free(fileEntry->newWindowProc);
		    fileEntry->newWindowProc= NULL;
		}
	    }
        }
        else {
            char errorMessage[MAXPATHLEN+200];
	    switch (errno) {
		case 0:
		    sprintf(errorMessage,"Could not create correct data object for file '%s'.",  fileEntry->filename );
		    break;
		case EACCES:
		    sprintf(errorMessage,"File '%s' not found; could not create. Permission denied.",  fileEntry->filename );
		    break;
#ifdef ETIMEDOUT
		case ETIMEDOUT:
		    sprintf(errorMessage,"File '%s' not found; could not create. A file server is down.",  fileEntry->filename );
		    break;
#endif /* ETIMEDOUT */
#ifdef EIO
		case EIO:
		    sprintf(errorMessage,"File '%s' not found; could not create. An I/O error occurred on the disk.",  fileEntry->filename );
		    break;
#endif /* EIO */
		    /* This next case is somewhat based on internal knowledge of the buffer package... */
		case EISDIR:
		    sprintf(errorMessage,"Attempt to visit directory without trailing slash.");
		    break;
		default:
		    sprintf(errorMessage, "File '%s' not found; could not create. %s.", fileEntry->filename, UnixError(errno));
		    break;
	    }
            StartupError(errtext,errorMessage);
        }
        next = fileEntry->next;
        free(fileEntry);
    }

    if ((errtext)->GetLength() != 0) {
        (errtext)->InsertCharacters( 0, INITIALHELP, sizeof(INITIALHELP) - 1);
        (errtext)->InsertCharacters( sizeof(INITIALHELP) - 1, "Errors encountered during startup:\n", sizeof("Errors encountered during startup:\n") - 1);
    }
    else {
        (this->errorBuffer)->Destroy();
        this->errorBuffer = NULL;
    }

    if (this->framep == NULL || this->errorBuffer != NULL)  {
	/*
	    Dont have a window yet, or need one for error buffer.
	*/

	this->framep = new frame;
	(this->framep)->SetCommandEnable( TRUE);
        if (this->haveBufferInWindow)
            (this->framep)->SetTitle( "Startup Errors.");
        else
            (this->framep)->SetTitle( "No files specified."); /* This will get reset below. */
	if (!this->first_im)
	    this->imp = im::Create(NULL);
	if(this->imp == NULL) {
	    /* Cannot run any further anyway, so just exit instead of dumping core! j_mukerji@att.com*/
	    exit(1);
	}
	(this->imp)->SetView( this->framep);
	(this->framep)->PostDefaultHandler( "message", (this->framep)->WantHandler( "message"));
    }

    if (this->errorBuffer)
	(this->framep)->SetBuffer( this->errorBuffer,	TRUE);
    waitCursor = cursor::Create(NULL);
    (waitCursor)->SetStandard( Cursor_Wait);

    if (! this->haveBufferInWindow)  {
        class text *text;
        class view *inputFocus;

        if (this->errorBuffer == NULL) {
            this->bufferp = buffer::Create("", NULL, "text", NULL);
            text = (class text *) (this->bufferp)->GetData();
            (this->framep)->SetBuffer( this->bufferp, TRUE);
            (text)->InsertCharacters( 0, INITIALHELP, sizeof(INITIALHELP) - 1);
        }
        inputFocus = (this->imp)->GetInputFocus();
        (inputFocus)->LoseInputFocus();
        Startup(this->framep);
        if (this->errorBuffer == NULL) /* If we had to create a buffer to hold the error text. */
            (this->bufferp)->Destroy();
        (this->framep)->SetTitle( NULL);
    }

/* A CheckPointInterval of 0 means don't checkpoint. */
    if ((CkpInterval = environ::GetProfileInt("CheckpointInterval", DEFAULTCKPINTERVAL)) != 0) {
        CkpLatency = environ::GetProfileInt("CheckpointMinimum", DEFAULTCKPLATENCY * CkpInterval) / CkpInterval;
	im::EnqueueEvent((event_fptr) Checkpoint, 0, event_SECtoTU(CkpInterval));
	MaxCkpSize = environ::GetProfileInt("CheckpointMaxSize", DEFAULTMAXCKPSIZE);
    }

    im::KeyboardProcessor();

    return 0;
}

boolean ezapp::InitializeClass()
{
    proctable::DefineProc("ezapp-set-buffer-checkpoint-latency", (proctable_fptr) SetBufferCkpLatency, ATK::LoadClass("frame"), NULL, "Set the number of checkpoint intervals to wait before checkpointing the current buffer.");

    return TRUE;
}
