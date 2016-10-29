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
 *
 *  Modified by Thomas J. Moore:
 *
 *  Now supports anything sox supports, since it uses that instead of
 *  direct /dev/audio access.  I was going to leave it at /dev/audio, but
 *  Linux ALSA /dev/audio no longer works like Sun's, and even "VOX"/"OSS",
 *  the "portable" audio driver, never really supported /dev/audioctl.
 *  I was also thinking of using portaudio (like I did for the beeps), but
 *  portaudio doesn't support ulaw conversion, whereas sox does.  Maybe I
 *  should drop portaudio in favor of sox there as well for consistency.
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
#include <pshbttnv.H>
#include <text.H>
#include <textview.H>
#include <view.H>
#include <viewref.H>
#include <util.h>

#include <fcntl.h>
#include <errno.h>

/* Defined constants and macros */
#define DBG(x) fprintf(stderr, "\nDebug: %s.", x);fflush(stderr);

// main issue with pipe is that it's impossible to make it start exactly the
// right time (and probably stop properly as well).
#define DEFAULT_REC_FMT "-t au -r 8000 -c 1 -e u-law" // Old sparcstation audio
#define DEFAULT_PLAY_FMT "" // use magic number + metadata to identify
//#define DEFAULT_SOX_REC "exec rec -qV0 %s -"
#define DEFAULT_SOX_REC "exec sox -qV0 -d %s -"
//#define DEFAULT_SOX_PLAY "exec play -qV0 %s -"
#define DEFAULT_SOX_PLAY "exec sox -qV0 %s - -d"

// maybe some day I'll use the API instead.  The problem with the API:
//  - default device is not found in API (only sox mainline).
//  - AUDIODEV/AUDIODRIVER env vars are only in sox mainline.
//  - multiple prefs needed to set individual aspects of format
//  - API is not well documented (doxy file, but only brief descriptions)

/* Forward Declarations */
  
              
 

ATKdefineRegistry(alinkview, pushbuttonview, alinkview::InitializeClass);
static void InsertAlink(class textview  *tv,long  l );
static void CreateMatte(class textview  *tv, class viewref  *vr );
static void InsertRecord(class textview  *tv,long  l );
static void LabelProc(class alinkview  *self, long  param );
static void PlayAudio(class alinkview  *self, class observable  *triggerer, long  rock );
static void RecordAudio(class alinkview  *self, long  param );
static FILE *OpenForPlay(class alinkview *self, int *pgrp);
static FILE *OpenForRecord(class alinkview  *self, int *pgrp);



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

  alinkview_menulist = new menulist;

  proc = proctable::DefineProc("alinkview-play-audio", (proctable_fptr)PlayAudio, &alinkview_ATKregistry_ , NULL, "Plays the audio segment.");
  (alinkview_menulist)->AddToML( "Audio~1,Play Audio~1", proc, 0, 0);

  proc = proctable::DefineProc("alinkview-record-audio", (proctable_fptr)::RecordAudio, &alinkview_ATKregistry_ , NULL, "Records the audio segment.");
  (alinkview_menulist)->AddToML( "Audio~1,Change recording~11",
proc, 0, 0);

  proc = proctable::DefineProc("alinkview-set-label-text", (proctable_fptr)LabelProc, &alinkview_ATKregistry_ , NULL, "Prompts for user to set the text string of the link button.");
  (alinkview_menulist)->AddToML( "Audio~1,Change Label~12", proc, 0, 0);

  ATK_CLASS(textview);

  bind::BindList(alinkBindings, NULL, alinkview_menulist, class_textview);

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
	if (v->GetParent() != (class view *) tv)
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
  const char *oldtext;

  oldtext = (l)->LabelSetP() ? (l)->GetText() : NULL;
  if (message::AskForString(self,50,"Enter new text for button: ",
			   oldtext, buf, sizeof(buf)) >= 0) {
    (l)->SetText( buf);
    (l)->NotifyObservers( observable::OBJECTCHANGED);
    (self)->WantNewSize(self);
    (self)->WantUpdate( self);
    message::DisplayString(self, 10, "Changed link text.");
  }
}

static void PlayAudio(class alinkview  *self, class observable  *triggerer, long  rock )
{

    /*
     * Play a message here
     */

    class alink *b = (class alink *)(self)->GetDataObject();
    char *p;
    long len, left;
    FILE *audio_out;
    int audio_out_pid;
    int interrupted = FALSE;
    class im *im = (self)->GetIM();

    if (!(b)->GetAudio()) {
	message::DisplayString(self, 10, "No message.");
	return;
    }
    if (!(audio_out = OpenForPlay(self, &audio_out_pid)))
	return;

    message::DisplayString(self, 10, "Playing... Type Control-G to interrupt.");
    while(im::Interact(0));

    p = (b)->GetAudio();
    left = len = (b)->GetAudioLength();
    while (left > 0) {
	int nread = (left > 256 ? 256 : left);
	int wrtn = 0;

	if (im->CheckForInterrupt()) {
	    interrupted = TRUE;
	    kill(-audio_out_pid, SIGINT);
	    break;
	}
	wrtn = fwrite(p, 1, nread, audio_out);
	fflush(audio_out);
	if (wrtn == 0) usleep(1000); // usleep is obsolete, but I'll use it anyway
	if(ferror(audio_out))
	    break;
	p += wrtn;
	left -= wrtn;
    }
    // this is necessary to force sox to exit when done
    fclose(audio_out);

    /* one problem when using a pipe is that it may suck in data faster than it
     * can play */
    while(!interrupted) {
	pid_t x = waitpid(audio_out_pid, NULL, WNOHANG);
	if(x == audio_out_pid)
	    break;
	if(!x || errno == EINTR || errno == EAGAIN) {
	    usleep(1000); // usleep is obsolete, but I'll use it anyway
	    interrupted = im->CheckForInterrupt();
	    if(interrupted)
		kill(-audio_out_pid, SIGINT);
	} else
	    break;
    }
    if(interrupted)
	waitpid(audio_out_pid, NULL, 0);

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
    FILE *audio_in;
    int audio_pid;
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

    if (!(audio_in = OpenForRecord(self, &audio_pid)))
	return;

    abuf = (char *)malloc(alloc);
    alen = alloc;

    for ( ; ; ) {
	nread = fread(buf, 1, sizeof(buf), audio_in);
	if (nread <= 0) break;
	if (len + nread > alen) {
	    alen += alloc;
	    abuf = (char *)realloc(abuf, alen);
	}
	memcpy(abuf + len, buf, nread);
	len += nread;
	if (im->CheckForInterrupt()) break;
    }

    kill(-audio_pid, SIGINT);
    qclose(audio_in);

    abuf = (char *)realloc(abuf, len);

    (b)->SetAudio( len, abuf);
    if (!(b)->LabelSetP()) {
	char line[100];
	const char *oldtext = (b)->LabelSetP() ? (b)->GetText() : NULL;
	message::AskForString(self, 10, "Label: ", oldtext, line, 100);
	(b)->SetText( line);
	(self)->WantNewSize( self);
	(self)->WantUpdate( self);
    }
    else
	message::DisplayString(self, 10, "Recording complete.");
    (b)->NotifyObservers( observable::OBJECTCHANGED);
    return;
}

void alinkview::RecordAudio()
{
    ::RecordAudio(this, 0);
}

static FILE *OpenForPlay(class alinkview *self, int *pgrp) 
{
    FILE *audio_fd;
    long gain;
//    boolean earphones;
    char buf[256]; // FIXME: arbitrarily sized and no bounds check
    const char *argv[4];

    const char *cmd = environ::GetProfile("SOXPlay");
    if(!cmd)
	cmd = DEFAULT_SOX_PLAY;
    // FIXME: ensure exactly 1 %s
    const char *fmt = environ::GetProfile("SOXPlayFmt");
    if(!fmt)
	fmt = DEFAULT_PLAY_FMT;
    sprintf(buf, cmd, fmt);
    // "gain" 0 - 99.  50 is default, so I'll make that 0.  +50 = +5, -50 = -5.
    // If that's inadequate, use the mixer instead of the preference.
    char *gainp = buf + strlen(buf);
    gain = environ::GetProfileInt("PlayLevel", 50);
    if(gain != 50) {
	if(gain < 0)
	    gain = 0;
	if(gain > 99)
	    gain = 99;
	sprintf(gainp, " gain %.1f", (double)(gain - 50)/10.0);
    }
    // no support for switching outputs
    // to switch outputs, change the sox command.
#if 0
    if ((earphones = environ::GetProfileSwitch("Earphones", FALSE)))
	SetEarPhones(earphones);
#endif
    argv[0] = "sh";
    argv[1] = "-c";
    argv[2] = buf;
    argv[3] = NULL;
    audio_fd = topen("/bin/sh", argv, "wb", pgrp);
    if (!audio_fd) {
	message::DisplayString(self, 10,
			       "The audio device can't be opened.");
	return audio_fd;
    }
    return audio_fd;

}

static FILE *OpenForRecord(class alinkview  *self, int *pgrp )
{
    FILE *audio_fd;
    long gain;
    char buf[256]; // FIXME: arbitrarily sized and no bounds check
    const char *argv[4];

    const char *cmd = environ::GetProfile("SOXRecord");
    if(!cmd)
	cmd = DEFAULT_SOX_REC;
    // FIXME: ensure exactly 1 %s
    const char *fmt = environ::GetProfile("SOXRecordFmt");
    if(!fmt)
	fmt = DEFAULT_REC_FMT;
    sprintf(buf, cmd, fmt);
    // "gain" 0 - 99.  50 is default, so I'll make that 0.  +50 = +5, -50 = -5.
    // If that's inadequate, use the mixer instead of the preference.
    char *gainp = buf + strlen(buf);
    gain = environ::GetProfileInt("RecordLevel", 50);
    if(gain != 50) {
	if(gain < 0)
	    gain = 0;
	if(gain > 99)
	    gain = 99;
	sprintf(gainp, " gain %.1f", (double)(gain - 50)/10.0);
    }
    argv[0] = "sh";
    argv[1] = "-c";
    argv[2] = buf;
    argv[3] = NULL;
    audio_fd = topen("/bin/sh", argv, "rb", pgrp);
    if (!audio_fd) {
	message::DisplayString(self, 10,
			       "The audio device can't be opened.");
	return audio_fd;
    }

    return audio_fd;

}
