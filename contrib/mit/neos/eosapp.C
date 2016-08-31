/*
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

/*
 * eosapp.c
 *
 * EOS application start-up.
*/

/* ************************************************************
 *  Copyright (C) 1989, 1990, 1991
 *  by the Massachusetts Institute of Technology
 *   For full copyright information see:'mit-copyright.h'     *
 ************************************************************ */

#include <andrewos.h>
#include <mit-copyright.h>

#include <buffer.H>
#include <environ.H>
#include <eosapp.H>
#include <eos.H>
#include <dataobject.H>
#include <view.H>
#include <frame.H>
#include <im.H>
#include <eos.h>
#include <text.H>
#include <cursor.H>
#include <message.H>
#include <rect.h>
#include <event.H>
#include <proctable.H>
#include <eosvers.h>

static char course[33];
static long CkpInterval; /* How often to run Checkpoint routine. */
static long CkpLatency; /* The minimum amount of time to wait to checkpoint a buffer. */
#define DEFAULTCKPINTERVAL 30 /* Default for CkpInterval. */
#define DEFAULTCKPLATENCY 4 /* Default fo CkpLatency. */

static class cursor *waitCursor;

/* The checkpointing algorithm used here is copied from eza.c
 * (Copyright IBM Corporation 1988,1989 - All Rights Reserved *
 *        For full copyright information see:'andrew/config/COPYRITE').
 * It based on the following notions:
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

struct bestbuffer {
    class buffer *buffer;
    long bufferclock;
};


ATKdefineRegistry(eosapp, application, eosapp__InitializeClass);
boolean FindCkpBuffer(class buffer  *buffer, struct bestbuffer  *best);
boolean CkpMessage(class view  *applicationView , class view  *targetView , class view  *inputFocusView, char  *message);
void Checkpoint(long  dummyData);
void SetBufferCkpLatency(class frame  *frame, long  key);
static void StartupError(class text  *error, char  *string);
static void addFile(class eosapp  *self,char  *name);
static void makeErrorBuf(class eosapp  *self);
boolean eosapp__InitializeClass();


boolean FindCkpBuffer(class buffer  *buffer, struct bestbuffer  *best)
        {

    if (!(buffer)->GetScratch() && ((buffer)->GetData())->GetModified() > (buffer)->GetCkpVersion()) {
        if ((buffer)->GetCkpClock() > best->bufferclock) {
            best->buffer = buffer;
            best->bufferclock = (buffer)->GetCkpClock();
        }
        (buffer)->SetCkpClock( (buffer)->GetCkpClock() + 1);
    }
    return FALSE;
}

#define view_Visible(view) (!rectangle_IsEmptyRect(&(((class graphic *) view)->visualBounds)))

boolean CkpMessage(class view  *applicationView , class view  *targetView , class view  *inputFocusView, char  *message)
        {
    if (inputFocusView == NULL) /* Handles case where input focus is not set... */
        inputFocusView = targetView;

    if (view_Visible(inputFocusView)) {
        message::DisplayString(inputFocusView, 0, message);
        return TRUE;
    }
    return FALSE;
}

void Checkpoint(long  dummyData)
    {
    struct bestbuffer result;

    result.buffer = NULL;
    result.bufferclock = CkpLatency - 1; /* (number + 1) * CKPINTERVAL seconds is how often a given buffer can be checkpointed. */

    buffer::Enumerate(FindCkpBuffer, &result);
    if (result.buffer != NULL) {

        int closeCode;

        im::SetProcessCursor(waitCursor);
        if ((result.buffer)->Visible())
            (result.buffer)->EnumerateViews( CkpMessage, "Checkpointing...");
        im::ForceUpdate();

        if ((closeCode = (result.buffer)->WriteToFile( (result.buffer)->GetCkpFilename(), 0)) >= 0) {
            (result.buffer)->SetCkpVersion( ((result.buffer)->GetData())->GetModified());
            (result.buffer)->SetCkpClock( (result.buffer)->GetCkpLatency());
        }

        if ((result.buffer)->Visible())
            (result.buffer)->EnumerateViews( CkpMessage, closeCode ? "Checkpoint Failed." : "Checkpointed.");
        im::SetProcessCursor(NULL);
    }
    im::EnqueueEvent(Checkpoint, 0, event_SECtoTU(CkpInterval));
}

void SetBufferCkpLatency(class frame  *frame, long  key)
        {

    class buffer *buffer;
    char answer[20];

    if ((buffer = (frame)->GetBuffer()) == NULL)
        return;

/* This stuff is a little hairy. Basically, it is converting a latency number to
 * seconds and back again. A latency number is shifted from 0, or specifically,
 * the code waits (latency + CkpLatency) * CkpInterval seconds before
 * considering checkpointing this buffer. 
 */
    sprintf(answer, "%d", (CkpLatency - (buffer)->GetCkpLatency()) * CkpInterval);
    if (message::AskForString(frame, 0, "Minimum checkpoint time in seconds: ", answer, answer, sizeof(answer)) != -1) {

        long latencyIntervals = atoi(answer) / CkpInterval;

        (buffer)->SetCkpLatency( CkpLatency - latencyIntervals);
        (buffer)->SetCkpClock( CkpLatency - latencyIntervals); /* Force new checkpoint time to take effect now. */
    }
}

static void StartupError(class text  *error, char  *string)
        {
    (error)->InsertCharacters( (error)->GetLength(), string, strlen(string));
}

static void addFile(class eosapp  *self,char  *name)
{
    /* Its a file right? */
    struct eosapp_fileList *fileEntry=
      (struct eosapp_fileList *) malloc(sizeof(struct eosapp_fileList));

    fileEntry->filename=name;
    fileEntry->ObjectName=self->defaultObject;
    fileEntry->next=NULL;
    *self->fileLink=fileEntry;
    self->fileLink=(&(fileEntry->next));
}

static void makeErrorBuf(class eosapp  *self)
{
    self->errorBuffer = buffer::Create("Startup-Errors", NULL, "text", NULL);
    (self->errorBuffer)->SetScratch( TRUE);
}

void eosapp::ReadInitFile()
{
    makeErrorBuf(this);

    (this)->SetErrorProc(StartupError);
    (this)->SetErrorRock((pointer)(this->errorBuffer)->GetData());

    (this)->application::ReadInitFile();
}


boolean eosapp::Start()
{
    class im *im;
    class eos *eos;
    struct eosapp_fileList *fileEntry, *next;
    class text *errtext;

    if (!(this)->application::Start())
        return FALSE;

    if (strcmp ((this)->GetName(), "grade") == 0)
	environ::Put("EOSTYPE", "grading");
    else
	environ::Put("EOSTYPE", "student");

    eos = new class eos;
    if (eos == NULL)
        return FALSE;

    if(this->errorBuffer==NULL)
	makeErrorBuf(this);

    errtext=(class text *)(this->errorBuffer)->GetData();

    buffer::SetDefaultObject(this->defaultObject);

    if(this->files==NULL)  {
	char *defFile;

	if ((defFile = environ::GetProfile("DefaultStartUpFile")) != NULL && *defFile != '\0')  {
	    addFile(this, defFile);
	}
    }

    for (fileEntry = this->files; fileEntry != NULL; fileEntry = next) {
        if(fileEntry->ObjectName != NULL){
            if(ATK::IsTypeByName(fileEntry->ObjectName,"dataobject"))
                buffer::SetDefaultObject(fileEntry->ObjectName);
            else {
                char errorMessage[200];
                sprintf(errorMessage,"%s is not a known dataobject\n",fileEntry->ObjectName);
                StartupError(errtext,errorMessage);
                eos->editregion = NULL;
                next = fileEntry->next;
                free(fileEntry);
                continue;
            }
        }
        (eos)->SetBuffer( fileEntry->filename, 0);

        if(fileEntry->ObjectName != NULL)
            buffer::SetDefaultObject(this->defaultObject);

        if (eos->editregion != NULL) {
            long version = ((eos->editregion)->GetData())->GetModified();

            (eos->editregion)->SetCkpClock( 0);
            (eos->editregion)->SetCkpVersion( version);
            (eos->editregion)->SetWriteVersion( version);
	}
        else {
            char errorMessage[200];
            sprintf(errorMessage, "File %s does not exist and could not be created.", fileEntry->filename);
            StartupError(errtext,errorMessage);
        }
        next = fileEntry->next;
        free(fileEntry);
    }

    if ((errtext)->GetLength() != 0) {
        (errtext)->InsertCharacters( 0, "Errors encountered during startup:\n", sizeof("Errors encountered during startup:\n") - 1);
    }
    else {
        (this->errorBuffer)->Destroy();
        this->errorBuffer = NULL;
    }

    im = im::Create(NULL);
    if (im == NULL) {
        (eos)->Destroy();
        return FALSE;
    }

    (eos)->SetProgram( (this)->GetName());

    if (eos->gradingflag)
	(eos)->SetTitle( "GRADE: Editor");
    else
	(eos)->SetTitle( "EOS Editor");

    (eos->frame)->PostDefaultHandler( "message", (eos->frame)->WantHandler( "message"));
    if (this->errorBuffer)
        (eos->frame)->SetBuffer( this->errorBuffer, TRUE);

    (im)->SetView( eos);
    (eos)->WantInputFocus( eos);
    waitCursor = cursor::Create(NULL);
    (waitCursor)->SetStandard( Cursor_Wait);
    if (*course) {
	(eos)->SetCourse( course);
    } else
	message::DisplayString(eos->frame, 0, "Warning: no course defined");
    return TRUE;
}

eosapp::eosapp()
{
    (this)->SetMajorVersion( EOS_MAJORVERSION);
    (this)->SetMinorVersion( EOS_MINORVERSION);
    this->files = NULL;
    this->fileLink = &this->files;
    this->errorBuffer = NULL;
    this->defaultObject = NULL;
    THROWONFAILURE( TRUE);
}


boolean eosapp::ParseArgs(int  argc, const char  **argv)
{
    boolean waiting_for_course  = FALSE;
    char *t; /* temporary string */

    t = environ::Get("COURSE");
    if (t)
	strncpy(course, t, 32);

    if (!(this)->application::ParseArgs( argc, argv))
      return FALSE;

    while (*++argv != NULL) {
        if (**argv == '-')
            switch ((*argv)[1]) {
            case 'c':
                waiting_for_course = TRUE;
                break;
            default:
                fprintf(stderr, "Unrecognised switch: -%c\n", (*argv)[1]);
                fprintf(stderr, "Usage: %s [-c course]\n", (this)->GetName());
                return FALSE;
            }
        else {
            if (waiting_for_course) {
                if (strlen(*argv) >32) {
                    strncpy(course, *argv, 32);
                    course[32] = '\0';
                } else
                    strncpy(course, *argv, 32);
                waiting_for_course = FALSE;
            } else
                addFile(this, *argv); 
        }
    }

    return TRUE;
}
                
int eosapp::Run()
{
 
    if(!(this)->Fork())
	return -1;

/* A CheckPointInterval of 0 means don't checkpoint. */
    if ((CkpInterval = environ::GetProfileInt("CheckpointInterval", DEFAULTCKPINTERVAL)) != 0) {
        CkpLatency = environ::GetProfileInt("CheckpointMinimum", DEFAULTCKPLATENCY * CkpInterval) / CkpInterval;
        im::EnqueueEvent(Checkpoint, 0, event_SECtoTU(CkpInterval));
    }

    im::KeyboardProcessor();

    return 0;
}

boolean eosapp__InitializeClass()
{

    proctable::DefineProc("eosapp-set-buffer-checkpoint-latency", SetBufferCkpLatency, ATK::LoadClass("frame"), NULL, "Set the number of checkpoint intervals to wait before checkpointing the current buffer.");

    return TRUE;
}
