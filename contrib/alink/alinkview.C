/* ********************************************************************** *\
 * Copyright (c) AT&T Bell Laboratories	1990  -	All Rights Reserved	  *
\* ********************************************************************** */

/*
 *  Audio link.
 *  
 *  Looks like a button, but when you press, it plays audio message.
 *  For sparcstation running Sun OS 4.1 or later.
 *
 *  Charles Hayden cch@mtgzfs3.att.com
 */

#include <andrewos.h>
#include <alinkview.H>
#include <alink.H>
#include <atom.H>
#include <bind.H>
#include <buffer.H>
#include <completion.H>
#include <dictionary.H>
#include <environ.H>
#include <im.H>
#include <matte.H>
#include <menulist.H>
#include <message.H>
#include <observable.H>
#include <proctable.H>
#include <pushbuttonview.H>
#include <text.H>
#include <textview.H>
#include <view.H>
#include <viewref.H>

#include <fcntl.h>
#include <sys/ioctl.h>
#include <errno.h>
#ifdef sun
#include <sun/audioio.h>
#endif

#define AUDIO_DEV "/dev/audio"
#define AUDIO_CTL "/dev/audioctl"

/* Defined constants and macros */
#define DBG(x) fprintf(stderr, "\nDebug: %s.", x);fflush(stderr);

/* Forward Declarations */
  
              
 

ATKdefineRegistry(alinkview, pushbuttonview, alinkview::InitializeClass);
static void InsertAlink(class textview  *tv,long  l );
static void CreateMatte(class textview  *tv, class viewref  *vr );
static void InsertRecord(class textview  *tv,long  l );
static void LabelProc(class alinkview  *self, long  param );
static void PlayAudio(class alinkview  *self, class observable  *triggerer, long  rock );
static void RecordAudio(class alinkview  *self, long  param );
static int OpenForPlay(class alinkview  *self, char  *device );
static void SetPlayGain(long  gain );
static void SetEarPhones(boolean  earphones );
static int OpenForRecord(class alinkview  *self, char  *device );
static void SetRecordGain(long  gain );



/* Global Variables */
static class menulist *alinkview_menulist = NULL;

static struct bind_Description alinkBindings[] = {
    {"alinkview-insert-alink", "", 0, "Inset~33,Insert Audio~20", 0, 0, (proctable_fptr)InsertAlink, "Insert audio link."},
    {"alinkview-insert-record", "", 0, "Inset~33,Record Audio~21", 0, 0, (proctable_fptr)InsertRecord, "Insert audio link and record."},
    NULL
};


boolean alinkview::InitializeClass()
{
/* 
  Initialize all the class data, particularly, set up the proc table entries 
  and the menu list (which is cloned for each instance of this class).
*/

  struct proctable_Entry *proc = NULL;
  struct ATKregistryEntry  *textviewtype = ATK::LoadClass("textview");

  alinkview_menulist = new menulist;

  proc = proctable::DefineProc("alinkview-play-audio", (proctable_fptr)PlayAudio, &alinkview_ATKregistry_ , NULL, "Plays the audio segment.");
  (alinkview_menulist)->AddToML( "Audio~1,Play Audio~1", proc, 0, 0);

  proc = proctable::DefineProc("alinkview-record-audio", (proctable_fptr)::RecordAudio, &alinkview_ATKregistry_ , NULL, "Records the audio segment.");
  (alinkview_menulist)->AddToML( "Audio~1,Change recording~11",
proc, 0, 0);

  proc = proctable::DefineProc("alinkview-set-label-text", (proctable_fptr)LabelProc, &alinkview_ATKregistry_ , NULL, "Prompts for user to set the text string of the link button.");
  (alinkview_menulist)->AddToML( "Audio~1,Change Label~12", proc, 0, 0);

#ifdef ATTBL_ENV
  attachBindings(alinkBindings, TRUE, environ::GetProfileSwitch("AlinkMenus", TRUE), "textview");
#else
  proc = proctable::DefineProc("alinkview-insert-alink", (proctable_fptr)InsertAlink, textviewtype, NULL, "Insert audio link.");

  proc = proctable::DefineProc("alinkview-insert-record", (proctable_fptr)InsertRecord, textviewtype, NULL, "Insert audio link and record.");

#endif

  return(TRUE);
}

alinkview::alinkview()
{
	ATKinit;

/*
  Set up the data for each instance of the object (i.e.: clone the menu
  list, add the trigger recipient).
*/

  this->ml = (alinkview_menulist)->DuplicateML( this);
  THROWONFAILURE(((this)->AddRecipient( atom::Intern("buttonpushed"), this, (observable_fptr)PlayAudio, 0L)));
}

alinkview::~alinkview()
{
	ATKinit;

  return;
}


void alinkview::PostMenus(class menulist  *ml )
{
/*
  Enable the menus for this object.
*/

  (this->ml)->ClearChain();
  if (ml) (this->ml)->ChainAfterML( ml, (long)ml);
  (this)->pushbuttonview::PostMenus( this->ml);
}

static void InsertAlink(class textview  *tv,long  l )
{
    long pos;
    class text *t = (class text *) (tv)->GetDataObject();
    pos = (tv)->GetDotPosition() + (tv)->GetDotLength();
    tv->currentViewreference = (t)->InsertObject( pos,"alink", NULL);
    (t)->NotifyObservers(0);
}

/* This was copied from textview.c */
static void CreateMatte(class textview  *tv, class viewref  *vr )
{
    class view *v =
	(class view *) dictionary::LookUp(tv, (char *) vr);

    if (v == NULL && ATK::IsTypeByName(vr->viewType, "view")) {
	class matte *m = matte::Create(vr, (class view *) tv);
	if (m != NULL) {
	    m->drawing = TRUE;
	    m->resizing = FALSE;
	    v = (class view *)m;
	    (vr)->AddObserver( tv);
	    dictionary::Insert(tv, (char *) vr, (char *) v);
	}
    }

    if (v != NULL) {
	if (v->parent != (class view *) tv)
	    (v)->LinkTree( (class view *) tv);
	(v)->InitChildren();
    }
}

static void InsertRecord(class textview  *tv,long  l )
{
    long pos;
    class matte *m;
    class alinkview *av;
    class text *t = (class text *) (tv)->GetDataObject();
    pos = (tv)->GetDotPosition() + (tv)->GetDotLength();
    tv->currentViewreference = (t)->InsertObject( pos,"alink", NULL);

    m = (class matte *)dictionary::LookUp(tv, (char *)tv->currentViewreference);
    if (m == NULL) {
	CreateMatte(tv, tv->currentViewreference);
	m = (class matte *) dictionary::LookUp(tv, (char *)tv->currentViewreference);
    }
    av = (class alinkview *)m->child;
    (av)->RecordAudio();

    (t)->NotifyObservers(0);
}

static void LabelProc(class alinkview  *self, long  param )
{
/*
  This is the routine which asks the user for a new text label.
*/

  char buf[255];
  class alink *l = (class alink *)(self)->GetDataObject();
  char *oldtext;

  oldtext = (l)->LabelSetP() ? (l)->GetText() : NULL;
  if (message::AskForString(self,50,"Enter new text for button: ",
			   oldtext, buf, sizeof(buf)) >= 0) {
    (l)->SetText( buf);
    (l)->NotifyObservers( observable_OBJECTCHANGED);
    (self)->WantNewSize(self);
    (self)->WantUpdate( self);
    message::DisplayString(self, 10, "Changed link text.");
  }
}
#ifdef sun
static void PlayAudio(class alinkview  *self, class observable  *triggerer, long  rock )
{

    /*
     * Play a message here
     */

    class alink *b = (class alink *)(self)->GetDataObject();
    char buf[256];
    char *p;
    long len, left;
    int audio_out;
    int interrupted = FALSE;
    class im *im = (self)->GetIM();

    if (!(b)->GetAudio()) {
	message::DisplayString(self, 10, "No message.");
	return;
    }
    if ((audio_out = OpenForPlay(self, AUDIO_DEV)) < 0)
	return;

    message::DisplayString(self, 10, "Playing... Type Control-G to interrupt.");
    while(im::Interact(0));

    p = (b)->GetAudio();
    left = len = (b)->GetAudioLength();
    while (left > 0) {
	int nread = (left > 256 ? 256 : left);
	int wrtn = 0;

	if (im::CheckForInterrupt(im)) {
	    interrupted = TRUE;
	    break;
	}
	bcopy(p, buf, nread);
	p += nread;
	left -= nread;
	while (wrtn == 0) {
	    wrtn = write(audio_out, buf, nread);
	    if (wrtn == 0) usleep(1000);
	}
	if (wrtn != nread) break;
    }

    close(audio_out);

    if (interrupted)
	message::DisplayString(self, 10, "Playback interrupted.");
    else
	message::DisplayString(self, 10, "Playback complete.");
}
static void RecordAudio(class alinkview  *self, long  param )
{

    /*
     * Record a message here.
     */
    class alink *b = (class alink *)(self)->GetDataObject();
    int audio_in;
    int rtn;
    int nread;
    char buf[256];
    long len = 0;
    char *abuf;
    long alen = 0;
    long alloc = 8192;
    class im *im = (self)->GetIM();

    if(environ::GetProfileSwitch("PromptForRecord", TRUE) == TRUE) {
	if (message::AskForString(self, 10, 
				 "Type RETURN to record.", 
				 NULL, buf, 256) < 0) 
	    return;
    }

    message::DisplayString(self, 10, "Type Control-G to end recording.");
    while(im::Interact(0));

    if ((audio_in = OpenForRecord(self, AUDIO_DEV)) < 0)
	return;

    abuf = malloc(alloc);
    alen = alloc;

    for ( ; ; ) {
	nread = read(audio_in, buf, 256);
	if (nread == 0) break;
	if (len + nread > alen) {
	    alen += alloc;
	    abuf = realloc(abuf, alen);
	}
	bcopy(buf, abuf + len, nread);
	len += nread;
	if (im::CheckForInterrupt(im)) break;
    }

    close(audio_in);

    abuf = realloc(abuf, len);

    (b)->SetAudio( len, abuf);
    if (!(b)->LabelSetP()) {
	char line[100];
	char *oldtext = (b)->LabelSetP() ? (b)->GetText() : NULL;
	message::AskForString(self, 10, "Label: ", oldtext, line, 100);
	(b)->SetText( line);
	(self)->WantNewSize( self);
	(self)->WantUpdate( self);
    }
    else
	message::DisplayString(self, 10, "Recording complete.");
    (b)->NotifyObservers( observable_OBJECTCHANGED);
    return;
}
#else
static void PlayAudio(class alinkview  *self, class observable  *triggerer, long  rock )
{
    message::DisplayString(self, 10, "Can't play audio on this machine, this only works on a Sun Sparc.");
}
static void RecordAudio(class alinkview  *self, long  rock )
{
    message::DisplayString(self, 10, "Can't record audio on this machine, this only works on a Sun Sparc.");
}
#endif

void alinkview::RecordAudio()
{
    ::RecordAudio(this, 0);
}

static int OpenForPlay(class alinkview  *self, char  *device ) 
{
#ifdef sun
    int audio_fd;
    long gain;
    boolean earphones;

    audio_fd = open(AUDIO_DEV, O_WRONLY | O_NDELAY);
    if (audio_fd < 0) {
	if (errno == EBUSY)
	    message::DisplayString(self, 10, "The audio device is busy.");
	else
	    message::DisplayString(self, 10, 
				  "The audio device can't be opened.");
	return audio_fd;
    }
    if (gain = environ::GetProfileInt("PlayLevel", 50))
	SetPlayGain(gain);
    if (earphones = environ::GetProfileSwitch("Earphones", FALSE))
	SetEarPhones(earphones);

    return audio_fd;
#else
    return -1;
#endif

}

static void SetPlayGain(long  gain )
{
#ifdef sun
    int audio_ctl_fd;
    audio_info_t audio_info;

    if ((audio_ctl_fd = open(AUDIO_CTL, O_RDWR)) < 0)
	return;
    AUDIO_INITINFO(&audio_info);

    /* gain must in the range 0 - 99 */
    if (gain > 99)
	gain = 99;
    else if (gain < 0)
	gain = 0;

    /* scale gain from 0 to 99 to 0 to AUDIO_MAX_GAIN */
    gain = (int) (((float)gain / 99.0) 
			* ((float) AUDIO_MAX_GAIN));
    audio_info.play.gain = gain;
    ioctl(audio_ctl_fd, AUDIO_SETINFO, &audio_info);

    close(audio_ctl_fd);
#endif
}

static void SetEarPhones(boolean  earphones )
{
#ifdef sun
    int audio_ctl_fd;
    audio_info_t audio_info;

    if ((audio_ctl_fd = open(AUDIO_CTL, O_RDWR)) < 0)
	return;
    AUDIO_INITINFO(&audio_info);
    if (earphones) 
	audio_info.play.port = AUDIO_HEADPHONE;
    else
	audio_info.play.port = AUDIO_SPEAKER;
    ioctl(audio_ctl_fd, AUDIO_SETINFO, &audio_info);
#endif
}


static int OpenForRecord(class alinkview  *self, char  *device ) 
{
#ifdef sun
    int audio_fd;
    long gain;

    audio_fd = open(AUDIO_DEV, O_RDONLY | O_NDELAY);
    if (audio_fd < 0) {
	if (errno == EBUSY)
	    message::DisplayString(self, 10, "The audio device is busy.");
	else
	    message::DisplayString(self, 10, 
				  "The audio device can't be opened.");
	return audio_fd;
    }

    if (gain = environ::GetProfileInt("RecordLevel", 50))
	SetRecordGain(gain);

    return audio_fd;
#else
    return -1;
#endif

}


static void SetRecordGain(long  gain )
{
#ifdef sun
    int audio_ctl_fd;
    audio_info_t audio_info;

    if ((audio_ctl_fd = open(AUDIO_CTL, O_RDWR)) < 0)
	return;
    AUDIO_INITINFO(&audio_info);

    /* gain must be in the range 0 - 99 */
    if (gain > 99)
	gain = 99;
    else if (gain < 0)
	gain = 0;

    /* scale gain from 0 to 99 to 0 to AUDIO_MAX_GAIN */
    gain = (int) (((float)gain / 99.0) 
			  * ((float) AUDIO_MAX_GAIN));
    audio_info.record.gain = gain;
    ioctl(audio_ctl_fd, AUDIO_SETINFO, &audio_info);

    close(audio_ctl_fd);
#endif
}

