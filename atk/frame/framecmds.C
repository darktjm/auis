/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

/* framecmd.c
 * The user commands for the frame package.
 * Makes up the file, buffer, and window handling functions of the editor.
 */

/* 
 * How to improve this module:
 * 
 * 1. Improve the message interface and you can simplify functions 
 * like preventOverwriteLossage.
 * 
 * 2. Convince everyone to allow interface overhauls and export 
 * functions like saveBuffer and saveTheWorld.
 * 
 * 3. Think of clever ways to eliminate most of the file manipulation 
 * commands (hide the file system).
 * 
 */

#include <andrewos.h> /* sys/types.h sys/file.h */
ATK_IMPL("framecmds.H")
#include <util.h>

#include <frame.H>
#include <view.H>
#include <dataobject.H>
#include <buffer.H>
#include <im.H>
#include <message.H>
#include <keymap.H>
#include <menulist.H>
#include <print.H>
#include <text.H>
#include <environ.H>
#include <proctable.H>
#include <bind.H>
#include <cursor.H>
#include <completion.H>
#include <style.H>
#include <fontdesc.H>
#include <path.H>
#include <framecmds.H>

#include <sys/param.h>
#include <signal.h> /* needed for hp - sometimes included in sys/param.h */
#include <sys/stat.h>
#include <errno.h>
#include <setjmp.h>

static class keymap *framecmdsKeymap, *framecmdsDefaultKeymap;
static class menulist *framecmdsMenus;
static class menulist *framecmdsDefaultMenus;
static class cursor *waitCursor;

/* this used to be ifdef'd on CONTRIB, but now it's on all the time */
#define PRINTER_SETUP_DIALOG_ENV 1

static jmp_buf trap;
ATKdefineRegistry(framecmds, ATK, framecmds::InitializeClass);

typedef void (*excursionfptr)(class frame *self);
static void frame_Exit(class frame  *self);
static int LocalReadFile(class frame  *self, const char  *fname, boolean  preserveBuffer);
static int frame_VisitFilePrompting(class frame  *self, const char  *prompt, boolean  newWindow, boolean  rawMode);
extern NO_DLL_EXPORT int frame_VisitNamedFile(class frame  *self, const char  *filename, boolean  newWindow, boolean  rawMode);
static int frame_WriteFile(class frame  *self);

static SIGNAL_RETURN_TYPE SigHandler(int sig) {longjmp(trap, 1);}

/* isString(arg)
	tests arg to see if it is a string
	returns TRUE if it is and FALSE otherwise
*/
static boolean
isString(char  *arg)
	{
	SIGNAL_RETURN_TYPE (*oldBus)(int sig), (*oldSeg)(int sig); /* save signal handlers */
	char c;

	/* first test to see if it is a character instead of a pointer */
	if ((long)arg < (1<<16) && (long)arg > -(1<<16))  
		return FALSE;	/* too small an integer to be a pointer (in ATK) */

	/* now be sure we can fetch from the location */
	oldBus = signal(SIGBUS, SigHandler);
	oldSeg = signal(SIGSEGV, SigHandler);
	if (setjmp(trap) != 0) 
		/* return here from longjmp */
		c = '\1';	/* not a legal string */
	else 
		/* normal setjmp return location */
		c = *arg;		/* this could fail if arg were not a pointer */
	signal(SIGBUS, oldBus);
	signal(SIGSEGV, oldSeg);

	/* return value depending on whether it points to an ASCII printable character */
	return (c >= ' ' && c < '\177');
}

static boolean bufferDirtyP(class buffer  *buffer)
    {
    return !(buffer)->GetScratch() &&
	(buffer)->GetWriteVersion() <
	    ((buffer)->GetData())->GetModified();
}



/* 
 * preventOutofSyncLossage checks to see if a given buffer is in-sync 
 * with the version on disk.  If the version on disk has changed, the 
 * user may choose to re-read it.
 */

static const char SyncLossage[] =
    "The version on disk is more recent than the version in the buffer.";
static const char * const SyncChoices[] = {
    "Read the new version from disk",
    "Use the old version in the buffer",
    0};
#define SYNC_READ  0
#define SYNC_LOSE  1



static int
preventOutofSyncLossage(class frame  *outputFrame, class buffer  *buffer)
        {
    if ((buffer)->GetFileDate() <= (buffer)->GetLastTouchDate())
	return 0;
    else {
	long answer;

	while(im::Interact(FALSE));
	if (message::MultipleChoiceQuestion(outputFrame, 0, SyncLossage,
					   SYNC_READ, &answer, SyncChoices,
					   NULL) == -1)
	    answer = SYNC_LOSE;

	if (answer == SYNC_READ)
	    return LocalReadFile(outputFrame, NULL, TRUE);
	else {
	    message::DisplayString(outputFrame, 0, "");
	    return 0;
	}
    }
}

/* 
 * preventReversionLossage should be called when a buffer is about to 
 * re-read from disk.  It guards against the accidental loss of 
 * modifications without offering to trash the unmodified copy.
 */

static const char ReversionLossage [] =
    "Rereading this file will cause changes in the buffer to be lost.";
static const char * const ReversionChoices [] = {
    "Cancel",
    "Read the file from disk (losing the changes in the buffer)",
    0};
#define REVERSION_CANCEL 0
#define REVERSION_LOSE 1

static boolean
preventReversionLossage(class frame  *outputFrame, class buffer  *buffer)
        {
    if (!bufferDirtyP(buffer))
	return TRUE;
    else {
	long answer;

	if (message::MultipleChoiceQuestion(outputFrame, 0,
					   ReversionLossage, REVERSION_CANCEL,
					   &answer, ReversionChoices, NULL)
	    == -1)
	    return FALSE;
	else if (answer == REVERSION_LOSE)
	    return TRUE;
	else {
	    message::DisplayString(outputFrame, 0,
	  	"You can use `Save As' to save your changes to a different file.");
	    return FALSE;
	}
    }
}


/* 
 * preventOverwriteLossage guards against writing the contents of a 
 * buffer to a file that has been changed since it was last read.  
 * Returns true if the operation should proceed.
 */

static const char OverwriteLossage[] =
    "The file `%.*s' has changed on disk since it was last read.";
static const char * const OverwriteChoices[] = {
    "Cancel",
    "Proceed with save (losing the changes on disk).",
    0};
#define OVERWRITE_CANCEL 0
#define OVERWRITE_LOSE   1

static boolean
preventOverwriteLossage(class frame  *outputFrame, class buffer  *buffer)
        {
    long foo = (buffer)->GetFileDate();
    if ( foo == 0l || foo == (buffer)->GetLastTouchDate())
	return TRUE;
    else {
	long answer;
	char msg[sizeof(OverwriteLossage) + MAXPATHLEN];

	sprintf(msg, OverwriteLossage, MAXPATHLEN, (buffer)->GetFilename());
	if (message::MultipleChoiceQuestion(outputFrame, 0,
					   msg, OVERWRITE_CANCEL,
					   &answer, OverwriteChoices, NULL)
	    == -1)
	    return FALSE;
	else if (answer == OVERWRITE_LOSE)
	    return TRUE;
	else {
	    message::DisplayString(outputFrame, 0,
	  	"You can use `Save As' to write to a different file.");
	    return FALSE;
	}
    }
}



/*
 * saveBuffer tries to write a file to disk.  outputFrame is used 
 * for delivering messages to the user.
 * 
 * The buffer's default filename can be overridden by a non-null 
 * filename.
 * 
 * A number less than 0 is returned on failure.
 */

#define NOFILE_MSG \
   "No file associated with buffer. Use \"Save As\" to save buffer contents."
#define DIR_MSG \
   "Write aborted: specified output file is a directory."

static int
saveBuffer(class frame  *outputFrame, class buffer  *buffer, char  *filename)
            {
    int result;
    long version;
    char message[sizeof("Wrote file ''.") + sizeof("Could not save file") + MAXPATHLEN];
    char *buffile;
    struct stat statbuf;

    if (outputFrame == NULL || buffer == NULL)
	return -1;

    buffile = (buffer)->GetFilename();

    if (filename == NULL)
	filename = buffile;

    if (filename == NULL) {
        message::DisplayString(outputFrame, 0, NOFILE_MSG);
        return -1;
    }

    if (stat(filename, &statbuf) == 0 && (statbuf.st_mode & S_IFDIR)) {
	message::DisplayString(outputFrame, 0, DIR_MSG);
	return -1;
    }

    /* 
     * Unless we're changing the filename, make sure we're not 
     * overwriting changes that have been made on disk.
     */
    if (buffile != NULL && !strcmp(buffile, filename) &&
	!preventOverwriteLossage(outputFrame, buffer))
	return -1;


    im::SetProcessCursor(waitCursor);

    result = (buffer)->WriteToFile( filename,
				buffer_ReliableWrite | buffer_MakeBackup);

    if (result >= 0) {
        version = ((buffer)->GetData())->GetModified();

        unlink((buffer)->GetCkpFilename());
        (buffer)->SetCkpClock( 0);
        (buffer)->SetCkpVersion( version);
        (buffer)->SetWriteVersion( version);
        (buffer)->SetReadOnly( (access(filename, W_OK) == -1 && errno == EACCES));
        (buffer)->NotifyObservers( observable::OBJECTCHANGED);

        sprintf(message, "Wrote file '%.*s'.%s", MAXPATHLEN, filename,
		buffer->readOnly ? " File is now readonly." : "");
        message::DisplayString(outputFrame, 0, message);
    }
    else
        switch (errno) {
	    case 0:	/* no error--must've cancelled XXX bad use of errno XXX*/
		message::DisplayString(outputFrame, 0, "Cancelled.");
		break;
            case EACCES:
                message::DisplayString(outputFrame, 0,
				 "Could not save file; permission denied.");
                break;
#ifdef ETIMEDOUT
            case ETIMEDOUT:
                message::DisplayString(outputFrame, 0,
				 "Could not save file; a server is down.");
                break;
#endif /* ETIMEDOUT */
#ifdef EFAULT
            case EFAULT:
                message::DisplayString(outputFrame, 0,
				 "Could not save file; a server is down.");
                break;
#endif /* EFAULT */
#ifdef EDQUOT
            case EDQUOT:
                message::DisplayString(outputFrame, 0,
			    "Could not save file; you are over your quota.");
                break;
#endif /* EDQUOT */
            case ENOSPC:
                message::DisplayString(outputFrame, 0,
		      "Could not save file; no space left on partition.");
                break;
#ifdef EIO
            case EIO:
                message::DisplayString(outputFrame, 0,
	 "Could not save file; an I/O error occurred on the disk.");
                break;
#endif /* EIO */
            case EISDIR:
                message::DisplayString(outputFrame, 0,
	 "File not found; could not create. Attempt to write to a directory.");
                break;
            default:
                sprintf(message, "Could not save file: %s.", strerror(errno));
                message::DisplayString(outputFrame, 0, message);
        }

    im::SetProcessCursor(NULL);
    return result;
}



static boolean frame_clear2exit()
{
    return buffer::Enumerate((bufferlist_efptr)bufferDirtyP, 0) == NULL;
}


/* 
 * saveTheWorld iterates over all buffers, trying to save those which 
 * have been modified.  It ignores buffers without filenames.
 */

struct saveTheWorldRock {
    /* input fields: */
    class frame *outputFrame;

    /* output fields: */
    char *names;
    int lengthLeft;
    boolean anyModified;
    boolean modifiedWithoutFilenames;
};

static boolean
saveAllWork(class buffer  *buffer, struct saveTheWorldRock  *returnBuf)
        {
    int len;

    if (bufferDirtyP(buffer)) {
        if ((buffer)->GetFilename() != NULL) {
            returnBuf->anyModified = TRUE;
	    if (saveBuffer(returnBuf->outputFrame, buffer, NULL) < 0)
                if ((len = (strlen((buffer)->GetName()) + 1))
		    < returnBuf->lengthLeft) {
                    strcat(returnBuf->names, " ");
                    strcat(returnBuf->names, (buffer)->GetName());
                    returnBuf->lengthLeft -= len;
		    }
	} else
            returnBuf->modifiedWithoutFilenames = TRUE;
    }
    return FALSE; /* Keep on enumerating. */
}

/* 
 * The return value is TRUE if the save-all succeeded. We err in the 
 * direction of being conservative by returning FALSE if there are 
 * modified buffers without files.
 */
static boolean
saveTheWorld(class frame  *outputFrame)
    {
    struct saveTheWorldRock returnCode;
    char errors[1024];
    int origLen;
    boolean succeeded = FALSE;

    returnCode.outputFrame = outputFrame;
    returnCode.names = errors + sizeof("Couldn't save buffers:") - 1;
    *returnCode.names = '\0';
    origLen = returnCode.lengthLeft = sizeof(errors) -
	sizeof("Couldn't save buffers:") - 2;
    returnCode.anyModified = returnCode.modifiedWithoutFilenames = FALSE;

    im::SetProcessCursor(waitCursor);
    buffer::Enumerate((bufferlist_efptr)saveAllWork, (long) &returnCode);
    im::SetProcessCursor(NULL);

    if (returnCode.lengthLeft != origLen) {
        strncpy(errors, "Couldn't save buffers:",
		sizeof("Couldn't save buffers:") - 1);
        message::DisplayString(outputFrame, 0, errors);
	succeeded = FALSE;
    }
    else {

        char messageBuf[256]; /* This is enuf. */

	succeeded = TRUE;	/* but look out below */

        if (returnCode.anyModified)
            strcpy(messageBuf, "Saved all modified buffers.");
        else
            strcpy(messageBuf, "No buffers needed saving.");
        if (returnCode.modifiedWithoutFilenames) {
            strcat(messageBuf,
		   " There are modified buffers without associated files.");
	    succeeded = FALSE;
	}
        message::DisplayString(outputFrame, 0, messageBuf);
    }

    return succeeded;
}



/* 
 * preventBufferLossage checks to see that a buffer-destroying operation 
 * will not be disasterous.  If an unsaved, modified buffer will be 
 * lost, preventBufferLossage issues a standard warning and offers to save 
 * one or all buffers. 
 *
 * outputFrame is the frame in which the dialog box will pop up. 
 * 
 * preciousBuffer, if non-null, is the only buffer that 
 * preventBufferLossage will try to protect.  If it is null, 
 * preventBufferLossage will try to protect all buffers.
 * 
 * The return value is FALSE if the user wishes the operation to be 
 * aborted.
 */

static const char lossageWarning[] =
  "You have unsaved changes that would be lost by this operation.";
static const char * const lossageChoices[] = {
	"Cancel",
	"Save changes and proceed",
	"Proceed without saving changes",
	NULL};

#define LOSSAGE_CANCEL 0
#define LOSSAGE_SAVE   1
#define LOSSAGE_LOSE   2

static boolean countChangedBuffer(struct buffer *buf, long rock)
{
    int *counter = (int *)rock;

    if (bufferDirtyP(buf))
	++(*counter);
    return FALSE;
}

static boolean preventBufferLossage(class frame  *outputFrame, class buffer  * preciousBuffer)
        {
    long answer;
    const char *warningstring = lossageWarning;
    char bigLossageWarning[80];
    int numChanged;

    if (preciousBuffer != NULL ? !bufferDirtyP(preciousBuffer)
			       : frame_clear2exit())
	return TRUE;

    if (preciousBuffer==NULL) {
	numChanged = 0;
	buffer::Enumerate((bufferlist_efptr)countChangedBuffer, (long)&numChanged);
	if (numChanged>1) {
	    /* give a nastier-sounding message if there are modified buffers in OTHER WINDOWS, too --RSK*/
	    sprintf(bigLossageWarning, "Unsaved changes in %d buffers would be lost by this operation.", numChanged);
	    warningstring = bigLossageWarning;
	}
    }

    if (message::MultipleChoiceQuestion(outputFrame, 0,
				       warningstring, LOSSAGE_CANCEL,
				       &answer, lossageChoices, NULL)
	== -1)
	return FALSE;

    switch (answer) {
    case LOSSAGE_CANCEL:
	return FALSE;

    case LOSSAGE_LOSE:
	return TRUE;

    case LOSSAGE_SAVE:
	if(preciousBuffer != NULL)
	    return  (saveBuffer(outputFrame, preciousBuffer,NULL) >= 0);
	else 
	    return  saveTheWorld(outputFrame);
    }
    return FALSE;
}

static void
bufferFilename(class buffer  *buffer, char  *filename		/* Output: MAXPATHLEN */)
        {
    if ((buffer)->GetFilename() != NULL)
        strcpy(filename, (buffer)->GetFilename());
    else {
        im::GetDirectory(filename);
        strcat(filename, "/");
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


/* Used to implement recursive editing functions.
 * Executes function recursively and the pops back
 * to the current buffer.
 */
static void frame_SaveExcursion(class frame  *self, excursionfptr function)
        {
    char bufferName[100];
    class buffer *oldBuffer;

    if (self->buffer != NULL)
        strncpy(bufferName, (self->buffer)->GetName(), sizeof(bufferName));
    else
        *bufferName = '\0';
    (*function)(self);
    if (*bufferName != '\0') {
        oldBuffer = buffer::FindBufferByName(bufferName);
        if (oldBuffer != NULL)
            (self)->SetBuffer( oldBuffer, TRUE);
    }
}

/* Auxilliary function for RecursiveVisitFile */
static void frame_RecursiveVisitWork(class frame  *self)
    {
    long code;

    code = frame_VisitFilePrompting(self, "Recursive visit file: ", FALSE, FALSE);
    if (code >= 0)
        im::KeyboardProcessor();
}

/* Allows you to visit a file and then pop back to the current buffer. */
static void frame_RecursiveVisitFile(class frame  *self)
    {
    frame_SaveExcursion(self, (excursionfptr)frame_RecursiveVisitWork);
}

/* 
 * Used to get a function pointer to im_KeyboardProcessor (which is a 
 * macro...) 
 */ 
static void myKeyboardProcessor()
{
    im::KeyboardProcessor();
}
 
/* A recursive edit... Plain, simple, and useless. */
static void frame_RecursiveEdit(class frame  *self)
    {
    frame_SaveExcursion(self, (excursionfptr)myKeyboardProcessor);
}

static int countFrames(class frame  *self, long  *rock)
        {
    (*rock)++;
    return 0;
}



static const char lastWindowWarning[] =
"This is the last window.";
static const char * const lastWindowChoices[] = {
    "Continue Running",
    "Quit Application",
    NULL};

#define lastWindow_CANCEL 0
#define lastWindow_QUIT   1

static void frame_DeleteWindow(class frame  *self)
    {
    long count = 0;

    frame::Enumerate((frame_effptr)countFrames, (long) &count);
    if (count > 1) {
	class im *im = (self)->GetIM();
	if(self->realBuffer) (self->realBuffer)->RemoveObserver( self);
	if(self->buffer && self->buffer!=self->realBuffer) (self->buffer)->RemoveObserver( self);
	(self)->SetView(NULL);
	(im)->SetView(NULL);
	(self)->Destroy();
	(im)->Destroy();
    }
    else {
	long answer;

	if (!environ::GetProfileSwitch("LastWindowQuitWarning", TRUE)) {
	    /* User doesn't care about the last window.  */
	    frame_Exit(self);
	    return;	/* User aborted for some reason. */
	}
	if (message::MultipleChoiceQuestion(self, 0,
					   lastWindowWarning, lastWindow_CANCEL,
					   &answer, lastWindowChoices, NULL)
	    == -1)
	    return;
	switch(answer){
	    case lastWindow_CANCEL:
		return;

	    case lastWindow_QUIT :
		frame_Exit(self);
	}
    }
}


void framecmds::DeleteWindow(class frame  *self)
{
	ATKinit;

    frame_DeleteWindow(self);
}


static void frame_ExitRecursiveEdit(class frame  *self)
    {
    if (im::KeyboardLevel() <= 1)
        message::DisplayString(self, 0, "Not in recursive edit");
    else
        im::KeyboardExit();
}

struct findbuf {
    class buffer *buffer;
    class frame *exclude_frame;
};

static boolean
countSpecificBuffers(class frame  *f,struct findbuf  *fb)
{
  return(f != fb->exclude_frame && (f)->GetBuffer()==fb->buffer);
}

void frame_Exit(class frame  *self)
    {
    if((self)->GetQuitWindowFlag()) {
	long count = 0; 
	frame::Enumerate((frame_effptr)countFrames, (long) &count);
	if (count > 1) { /* if more than one window, don't quit */
	    class im *im = (self)->GetIM();
	    class buffer *b = (self)->GetBuffer();
	    struct findbuf fb;
	    boolean kill_buffer = FALSE;

	    fb.buffer = b;
	    fb.exclude_frame = self;
	    if ((self)->GetQuitWindowFlag() > 1 && !frame::Enumerate((frame_effptr)countSpecificBuffers, (long)&fb)) {
		/* The QuitBuffer preference is TRUE and no other frames are
		 * viewing this buffer.  Destroy the buffer and the window.
		 */
		if (!preventBufferLossage(self, b)) {
		    message::DisplayString(self, 0, "Quit aborted.");
		    return;
		}
		kill_buffer = TRUE;
	    }

	    if(b) b->RemoveObserver(self);
	    (self)->SetView(NULL);
	    (im)->SetView(NULL);
	    (self)->Destroy();
	    (im)->Destroy();
	    if (kill_buffer) {
		if(b) (b)->Destroy();
	    }
	    return;
	}
    }
    if (self->buffer && im::KeyboardLevel() == 1) {
	if (!preventBufferLossage(self, NULL)) {
            message::DisplayString(self, 0, "Exit aborted.");
            return;
        }
    }
    im::KeyboardExit();
}

static void frame_NewWindow(class frame  *self)
    {
    class buffer *buffer;
    class frame *newFrame;
    class im *window;
    int count;
    char host[100];

    strcpy(host, "");
    count = (self->GetIM())->Argument();
    if (count > 1) {
	if (message::AskForString(self, 10, "Host: ", "", host, 100) < 0)
	    return;
    }

    if((newFrame = new frame) == NULL) {
	fprintf(stderr,"Could not allocate enough memory to create new window.\n");
	return;
    }
    (newFrame)->SetCommandEnable( TRUE);

    if((window = im::Create(host)) == NULL) {
	if(*host != (char)0)
	    fprintf(stderr,"Could not create new window on host %s.\n", host);
	else
	    fprintf(stderr,"Could not create new window.\n");
	if(newFrame) (newFrame)->Destroy();
	return;
    }
    (window)->SetView( newFrame);

    (newFrame)->PostDefaultHandler( "message",
			     (newFrame)->WantHandler( "message"));

    buffer = (self)->GetBuffer();
    (newFrame)->SetBuffer( buffer, TRUE);
}

static boolean BufferCompletionWork(class buffer  *buffer, struct result  *data)
        {
    completion::CompletionWork((buffer)->GetName(), data);
    return FALSE;
}

/* ARGUSED */
static enum message_CompletionCode BufferComplete(char  *partial, long  dummyData /* Just along for the ride... */,
						  char  *bufout, int  bufoutSize)
                {
    struct result result;
    char textBuffer[100];

    *textBuffer = '\0';
    result.partial = partial;
    result.partialLen = strlen(partial);
    result.bestLen = 0;
    result.code = message_Invalid;
    result.best = textBuffer;
    result.max = sizeof(textBuffer) - 1; /* Leave extra room for a NUL. */

    buffer::Enumerate((bufferlist_efptr)BufferCompletionWork, (long) &result);

    strncpy(bufout, result.best, bufoutSize);
    if (result.bestLen == bufoutSize) /* Now make sure bufout ends in a NUL. */
        bufout[result.bestLen] = '\0';
    return result.code;
}

struct helpData {
    const char *partial;
    message_workfptr textFunction;
    long textRock;
};

static boolean BufferHelpWork(class buffer  *buffer, struct helpData  *helpData)
{
    char infoBuffer[1024];

    if (completion::FindCommon(helpData->partial,
       (buffer)->GetName()) == (int)strlen(helpData->partial)) {
        const char *modStatus;
        char sizeBuf[16];
        const char *className;
        char *fileName, shortFileName[256];
	long md;
	
	if((buffer)->GetScratch())
	    modStatus = "";
	else {
	    md = ((buffer)->GetData())->GetModified();
	    if ((buffer)->GetWriteVersion() < md) {
		if ((buffer)->GetCkpVersion() >= md)
		    modStatus = "Ckp";
		else
		    modStatus = "Mod";
	    } else if (buffer->readOnly)
		modStatus = "R/O";
	    else
		modStatus = "";
	}
        className = ((buffer)->GetData())->GetTypeName();

        sizeBuf[0] = '\0';
        if (ATK::IsTypeByName(className, "text"))
            sprintf(sizeBuf, "%ld",
              ((class text *) (buffer)->GetData())->GetLength());

        fileName = (buffer)->GetFilename();
        shortFileName[0] = '\0';

	if (fileName != NULL) {
	    path::TruncatePath(fileName, shortFileName, sizeof(shortFileName) - 1, TRUE);
        }

        sprintf(infoBuffer, "%-16s%7s %-3s %-8s %s",
          (buffer)->GetName(), sizeBuf, modStatus,
          className, shortFileName);

        (*helpData->textFunction)(helpData->textRock,
            message_HelpListItem, infoBuffer, NULL);
    }
    return FALSE; /* Keep on enumerating. */
}

static void BufferHelp(const char  *partial, long  listInfo, message_workfptr helpTextFunction, long  helpTextRock)
{
    struct helpData helpData;

    helpData.partial = partial;
    helpData.textFunction = helpTextFunction;
    helpData.textRock = helpTextRock;

    buffer::Enumerate((bufferlist_efptr)BufferHelpWork, (long) &helpData);
}

static char lastBuffer[100] = "";

static boolean FindFirstBuffer(class buffer  *tryBuffer , class buffer  *cannotMatchBuffer)
{
    /* Find first one not matching */
    return (tryBuffer != cannotMatchBuffer);
}

static void frame_OldBuffer(class frame  *self)
    {

    char bufferName[100], prompt[256];
    class buffer *buffer;

    if (*lastBuffer == '\0' || !buffer::FindBufferByName(lastBuffer)) {
        buffer = (self)->GetBuffer();   /* Any except current */
	buffer = buffer::Enumerate((bufferlist_efptr)FindFirstBuffer, (long) buffer);
        if (buffer != NULL)
            strcpy(lastBuffer, (buffer)->GetName());
	else
	    *lastBuffer = '\0';
    }
    if (*lastBuffer != '\0')
        sprintf(prompt, "Visit existing buffer [%.100s] : ", lastBuffer);
    else
        strcpy(prompt, "Visit existing buffer: ");
    if (message::AskForStringCompleted(self, 0, prompt,
				      (*lastBuffer == '\0') ?
				        NULL : lastBuffer, bufferName,
				       sizeof(bufferName), NULL, (message_completionfptr) BufferComplete,
				       (message_helpfptr) BufferHelp, 0,
				      message_MustMatch |
				        message_NoInitialString) == -1)
        return;
    buffer = buffer::FindBufferByName(bufferName);
    if (self->buffer != buffer)
        strncpy(lastBuffer, (self->buffer)->GetName(), sizeof(lastBuffer));
    (self)->SetBuffer( buffer, TRUE);
    message::DisplayString(self, 0, "Done.");
}

static void frame_VisitBuffer(class frame  *self)
    {
    char bufferName[100], prompt[100 + sizeof("Visit buffer [] : ") - 1];
    class buffer *buffer;

    if (*lastBuffer == '\0' || !buffer::FindBufferByName(lastBuffer)) {
        buffer = (self)->GetBuffer();     /* Any except current */
	buffer = buffer::Enumerate((bufferlist_efptr)FindFirstBuffer, (long) buffer);
        if (buffer != NULL)
            strcpy(lastBuffer, (buffer)->GetName());
        else
            strcpy(lastBuffer, "Scratch");
    }
    sprintf(prompt, "Visit buffer [%.100s] : ", lastBuffer);
    if (message::AskForStringCompleted(self, 0, prompt,
				       lastBuffer, bufferName, sizeof(bufferName), NULL, (message_completionfptr) BufferComplete,
				       (message_helpfptr) BufferHelp, 0, message_NoInitialString) == -1)
        return;
    buffer = buffer::FindBufferByName(bufferName);
    if (buffer == NULL)
        if ((buffer = buffer::Create(bufferName, NULL, NULL, NULL)) == NULL) {
            message::DisplayString(self, 0,
              "Buffer does not exist; could not create.");
            return;
        }
    if (self->buffer != buffer)
        strncpy(lastBuffer, (self->buffer)->GetName(), sizeof(lastBuffer));
    (self)->SetBuffer( buffer, TRUE);
    message::DisplayString(self, 0, "Done.");
}

static void ListBuffersWork(class text  *helpDoc, long  dummyData, char  *bufferInfo, char  *dummyInfo)
{
    int c;
    int c2;
    long initPos;
    long inc;
    long pos = 0;
    char *tempString;

    do {
        initPos = pos;
        tempString = bufferInfo;
        while (((c = (helpDoc)->GetChar( pos)) == *tempString) &&
	       (*tempString != '\0') && (c != EOF)) {
            pos++;
            tempString++;
        }
        while ((c2 = (helpDoc)->GetChar( pos)) != '\n' && c2 != EOF)
            pos++;
        if (c2 == '\n')
            pos++;
    } while (c != EOF && (c < *tempString));
    if (c == EOF)
        initPos = pos;

    (helpDoc)->InsertCharacters( initPos, bufferInfo,
			  (inc = strlen(bufferInfo)));
    (helpDoc)->InsertCharacters( initPos + inc, "\n", 1);
}

static void frame_ListBuffers(class frame  *self, long  key)
        {

    class buffer *helpBuffer = (self)->GetHelpBuffer();

    if ((self)->GetBuffer() == NULL)
        return;

    if (helpBuffer == NULL)
        message::DisplayString(self, 0, "Couldn't list buffers.");
    else {
        class text *listDoc = (class text *) (helpBuffer)->GetData();
        class buffer *oldBuffer;
        char oldBufferName[100];
        char dummy[2];
	const char *s;
        static class style *boldStyle = NULL,
           *ulineStyle = NULL, *fixedStyle = NULL;

        if (boldStyle == NULL) {
            boldStyle = new style;
            (boldStyle)->AddNewFontFace( fontdesc_Bold);
            ulineStyle = new style;
            (ulineStyle)->AddUnderline();
            fixedStyle = new style;
            (fixedStyle)->SetFontFamily( "andytype");
            (fixedStyle)->AddNewFontFace( fontdesc_Fixed);
            (fixedStyle)->SetFontSize( style_PreviousFontSize, -2);
            (fixedStyle)->AddNoWrap();
        }

        (listDoc)->Clear();
	BufferHelp("", 0, (message_workfptr) ListBuffersWork, (long) listDoc);

        s = "Name               Size Sav Object   File\n";
        (listDoc)->InsertCharacters( 0, s, strlen(s));
        (listDoc)->AddStyle( 0, 4, ulineStyle);
        (listDoc)->AddStyle( 19, 4, ulineStyle);
        (listDoc)->AddStyle( 24, 3, ulineStyle);
        (listDoc)->AddStyle( 28, 6, ulineStyle);
        (listDoc)->AddStyle( 37, 4, ulineStyle);

        (listDoc)->AddStyle( 0, (listDoc)->GetLength(), fixedStyle);

        s = "Current Buffers:\n\n";
        (listDoc)->InsertCharacters( 0, s, strlen(s));
        (listDoc)->AddStyle( 0, strlen(s), boldStyle);

        (listDoc)->NotifyObservers( 0);
        strcpy(oldBufferName, ((self)->GetBuffer())->GetName());
        (self)->SetBuffer( helpBuffer, TRUE);
        message::AskForString(self, 0,
          "Press Enter to continue.", "", dummy, 1);
        if ((oldBuffer = buffer::FindBufferByName(oldBufferName)) != NULL) {
            (self)->SetBuffer( oldBuffer, TRUE);
            message::DisplayString(self, 0, "Done.");
	}
        else
	    message::DisplayString(self, 0,"Couldn't restore old buffer; it has probably been deleted.");
/* Don't know why this was here... and it will coredump if the help buffer was deleted... -rr2b text_NotifyObservers(listDoc, 0);
			  */
    }
}

static void frame_DropBuffer(class frame  *self)
    {
    class buffer *b = (self)->GetBuffer();
    class im *im = (self)->GetIM();
    char *name;

    if (b && im) {
	name = (b)->GetFilename();
	if (name)
	    (im)->DropFile( name, NULL);
    }
}

struct bufferPair {
    class buffer *buffer1, *buffer2;
};

static boolean ReplaceBufferWork(class frame  *frame, struct bufferPair  *bufferPair)
        {

    if ((frame)->GetBuffer() == bufferPair->buffer1)
        (frame)->SetBuffer( bufferPair->buffer2, TRUE);
    return FALSE;
}

static void ReplaceBuffer(class buffer  *oldBuffer , class buffer  *newBuffer)
    {
        struct bufferPair buffers;

        buffers.buffer1 = oldBuffer;
        if ((buffers.buffer2 = newBuffer) == oldBuffer)
            return;
	frame::Enumerate((frame_effptr)ReplaceBufferWork, (long) &buffers);
}

static void DeleteABuffer(class frame *self, buffer *targetBuffer) {

    self->deleteTarget=targetBuffer;
    (self->deleteTarget)->AddObserver( self);
    if (!preventBufferLossage(self,targetBuffer)) {
	message::DisplayString(self, 0, "Buffer not deleted.");
	if (self->deleteTarget) (self->deleteTarget)->RemoveObserver(self); /*RSK*/

	return;
    }

    if(self->deleteTarget==NULL) {
	message::DisplayString(self, 0, "Buffer already deleted!");
	return;
    }
    (self->deleteTarget)->RemoveObserver( self);
    if ((targetBuffer)->Visible()) {
	class buffer *newBuffer;
	newBuffer = NULL;
	if (*lastBuffer != '\0')
	    newBuffer = buffer::FindBufferByName(lastBuffer);
	if (newBuffer == targetBuffer)
	    newBuffer = NULL;
	if (newBuffer == NULL)
	    newBuffer = buffer::Enumerate((bufferlist_efptr)FindFirstBuffer, (long) targetBuffer);
	if (newBuffer == NULL)
	    newBuffer = buffer::Create("Scratch", NULL, NULL, NULL);
	if (newBuffer == NULL) {
	    message::DisplayString(self, 0, "Buffer not deleted.");
	    return;
	}
	*lastBuffer = '\0';
	ReplaceBuffer(targetBuffer, newBuffer);
    } else
	*lastBuffer = '\0';

#if 0
    im::ForceUpdate(); /* Clear out any update requests having pointers to things we are going to nuke. */
#endif /* 0 */

    (targetBuffer)->Destroy();
    self->deleteTarget=NULL;
    message::DisplayString(self, 0, "Done.");
}

static void frame_DeleteBufferPrompt(class frame  *self)
    {
    buffer *thisBuffer, *targetBuffer;
    char bufferName[100], *defaultName = NULL, prompt[356];

    thisBuffer = (self)->GetBuffer();
    if (thisBuffer != NULL)
        defaultName = (thisBuffer)->GetName();
    sprintf(prompt, "Delete buffer [%.100s] : ", defaultName);
    if (message::AskForStringCompleted(self, 0, prompt, defaultName,
				      bufferName, sizeof(bufferName), NULL,
				       (message_completionfptr) BufferComplete, (message_helpfptr) BufferHelp, 0,
				      message_NoInitialString) == -1)
        return;

    targetBuffer = buffer::FindBufferByName(bufferName);

    if (!targetBuffer) {
	sprintf(prompt,"No buffer '%s'.",bufferName);
	message::DisplayString(self, 0, prompt);
	return;
    }
   DeleteABuffer(self, targetBuffer);
}

static long frame_DeleteThisBuffer(ATK *f, long rock)
{
    frame *self=(frame *)f;
    struct buffer *thisBuffer;
    thisBuffer = self->GetBuffer();
    if (!thisBuffer) return 0;

    DeleteABuffer(self, thisBuffer);
    return 0;
}

static class buffer *LocalGetBufferOnFile(class frame  *self, const char  *filename, int  flags)
{
    class buffer *buffer;
    int localerrno;
    char msgbuf[MAXPATHLEN + 100];

    im::SetProcessCursor(waitCursor);
    errno=0;
    buffer = buffer::GetBufferOnFile(filename, flags);
    localerrno=errno;
    /* sigh, SetProcessCursor will call X routines which may decide to flush all requests to the server, thus calling write(2) and trashing errno */
    im::SetProcessCursor(NULL);

    if (buffer == NULL)
        switch (localerrno) {
            case 0:
                message::DisplayString(self, 0,
				      "Could not create correct data object.");
                break;
            case EACCES:
                message::DisplayString(self, 0,
		     "File not found; could not create. Permission denied.");
                break;
#ifdef ETIMEDOUT
            case ETIMEDOUT:
                message::DisplayString(self, 0,
		 "File not found; could not create. A file server is down.");
                break;
#endif /* ETIMEDOUT */
#ifdef EIO
            case EIO:
                message::DisplayString(self, 0,
	 "File not found; could not create. An I/O error occurred on the disk.");
                break;
#endif /* EIO */
/* This next case is somewhat based on internal knowledge of the buffer package... */
            case EISDIR:
                message::DisplayString(self, 0,
			"Attempt to visit directory without trailing slash.");
                break;
            default:
                sprintf(msgbuf, "File not found; could not create. %s.", strerror(localerrno));
                message::DisplayString(self, 0, msgbuf);
                break;
        }


    return buffer;
}


/* 
 * The preserveBuffer parameter to LocalReadFile is a hack to satisfy 
 * the desired semantics of switch-file.  As with most elements of 
 * ATK, the frame package could be profitably replaced.  I find the 
 * current package's relation to the buffer class confusing.  There is 
 * little direct help given for common operations like reading and 
 * writing buffers.  Blah.
 * 
 * In the mean time, this is the solution that is minimally disruptive.
 * 
 * preserveBuffer should be a boolean value.  If true, LocalReadFile 
 * will make an effort (which is not guaranteed to succeed) to reuse 
 * the existing buffer.  If false, LocalReadFile is guaranteed to 
 * start with a new buffer.
 */

static const char MBufferWarning[]="That file is already in a buffer, it may be confusing...";

static const char * const MBufferChoices[]={
    "Make a new buffer for this file",
    "Visit the existing buffer",
    "Cancel",
    NULL
};

static int LocalReadFile(class frame  *self, const char  *fname, boolean  preserveBuffer)
{
    class buffer *buffer = (self)->GetBuffer(), *oldBuffer;
    char tempName[256];
    char filename[MAXPATHLEN];

    if (fname != NULL)
	strcpy(filename, fname);
    else if ((buffer)->GetFilename() == NULL)
	return -1;
    else
	strcpy(filename, (buffer)->GetFilename());

    /* don't use fname after this point, use filename. */

    if((oldBuffer=buffer::FindBufferByFile(filename)) && oldBuffer!=buffer) {
	long result;
	if (message::MultipleChoiceQuestion(self, 0, MBufferWarning, 2, &result, MBufferChoices, NULL)
	    == -1)
	    return -1;
	if(result==2) return -1;
	if(result==1) {
	    return frame_VisitNamedFile(self, filename, FALSE, FALSE);
	}
    }
    im::SetProcessCursor(waitCursor);

/* The purpose of this next piece of code is to support dataobjects which may
 * have some loose persistient state across reads. Therefore if we are
 * re-reading the same file, we try to read it into the same object using
 * buffer_ReadFile. Otherwise we create a new buffer from scratch. An example
 * of this is the text object which maintains the cursor position across read
 * calls.
 */
    if (!preserveBuffer || (buffer)->ReadFile( filename) < 0) {
        if ((buffer = LocalGetBufferOnFile(self, filename, buffer_ForceNew)) ==
	    NULL) {
            im::SetProcessCursor(NULL);
            return -1;
        }

        oldBuffer = (self)->GetBuffer();
        if (oldBuffer != NULL && oldBuffer != buffer) {
            ReplaceBuffer(oldBuffer, buffer);
            (oldBuffer)->Destroy();
            (self)->SetBuffer( buffer, TRUE);
        }
    } else {
        (buffer)->SetName( "");
        buffer::GuessBufferName(filename, tempName, sizeof(tempName));
        (buffer)->SetName( tempName);
    }

    im::SetProcessCursor(NULL);

    if (buffer->readOnly)
        message::DisplayString(self, 0, "File is read only.");
    else if (access((buffer)->GetFilename(), F_OK) >= 0)
        message::DisplayString(self, 0, "Done.");
    else
        message::DisplayString(self, 0, "New file.");

    if(buffer::FindBufferByName(lastBuffer) == NULL)
	strcpy(lastBuffer,"");

    return 0;
}



/* Not static so it can be used from eza.c */
int frame_VisitFilePrompting(class frame  *self, const char  *prompt, boolean  newWindow, boolean  rawMode)
                {
    char filename[MAXPATHLEN];
    class buffer *buffer;

    if ((buffer = (self)->GetBuffer()) == NULL)
        return -1;

    bufferDirectory(buffer, filename);

    if (completion::GetFilename(self, prompt, filename, filename,
			       sizeof(filename), FALSE, rawMode) == -1) {
        errno = 0;
        return -1;
    }

    return frame_VisitNamedFile(self, filename, newWindow, rawMode);
}

int frame_VisitNamedFile(class frame  *self, const char  *filename, boolean  newWindow, boolean  rawMode)
                {
    class buffer *buffer;
    long flags = 0;

    if (rawMode) {
	flags = buffer_RawMode;
    }

    if ((buffer = LocalGetBufferOnFile(self, filename, flags)) == NULL)
        return -1;

    if ((self)->GetBuffer() != buffer)
        strncpy(lastBuffer, ((self)->GetBuffer())->GetName(),
		sizeof(lastBuffer));

    if (newWindow) {
        class frame *newFrame;
        class im *window;

        if ((newFrame = new frame) != NULL) {
            if ((window = im::Create(NULL)) != NULL) {
                (newFrame)->SetCommandEnable( TRUE);
                (window)->SetView( newFrame);
                (newFrame)->PostDefaultHandler( "message",
				 (newFrame)->WantHandler( "message"));
                self = newFrame;
                message::DisplayString(self, 0, "");
            }
            else {
		fprintf(stderr,"Could not create new window.\n");
                (newFrame)->Destroy();
	    }
	}
    }

    (self)->SetBuffer( buffer, TRUE);
    return preventOutofSyncLossage(self, buffer);
}

/* like frame_VisitFilePrompting, but won't prompt if arg is non-NULL */
static int frame_VisitAFile(class frame  *self, char  *arg , const char  *prompt, boolean  newWindow)
            {
    if (isString(arg))
	return frame_VisitNamedFile(self, arg, newWindow, FALSE);
    else
	return frame_VisitFilePrompting(self, prompt, newWindow, FALSE);
}

static int frame_VisitFile(class frame  *self, char  *arg)
        {
    return frame_VisitAFile(self, arg, "Visit file: ",
				    ((self)->GetIM())->ArgProvided());
}

static int frame_VisitRawFile(class frame  *self, char  *arg)
        {
    return frame_VisitFilePrompting(self, "Visit file (raw mode): ",
				    ((self)->GetIM())->ArgProvided(), TRUE);
}

static int frame_VisitFileNewWindow(class frame  *self, char  *arg)
        {
    return frame_VisitAFile(self, arg, "Visit file: ", TRUE);
}

static int frame_SwitchFile(class frame  *self, long  key)
{
    char bufferfile[MAXPATHLEN];
    char filename[MAXPATHLEN];
    class buffer *buffer;

    if ((buffer = (self)->GetBuffer()) == NULL)
        return -1;

    bufferDirectory(buffer, filename);
    bufferFilename(buffer, bufferfile);

    if (completion::GetFilename(self, "Switch File: ", filename, filename,
			       sizeof(filename), FALSE, FALSE) == -1)
        return -1;

    if (strcmp(bufferfile, filename) ?
	!preventBufferLossage(self, (self)->GetBuffer()) :
	!preventReversionLossage(self, (self)->GetBuffer())) {

        message::DisplayString(self, 0, "Aborted");
        return -1;
    }

    return LocalReadFile(self, filename, FALSE);
}

static int frame_ReadFile(class frame  *self, long  key)
{
    char bufferfile[MAXPATHLEN];
    char filename[MAXPATHLEN];
    class buffer *buffer;

    if ((buffer = (self)->GetBuffer()) == NULL)
        return -1;


    bufferFilename(buffer, bufferfile);

    if (completion::GetFilename(self, "Read file: ", bufferfile, filename,
			       sizeof(filename), FALSE, FALSE) == -1)
        return -1;

    if (strcmp(bufferfile, filename) ?
	!preventBufferLossage(self, (self)->GetBuffer()) :
	!preventReversionLossage(self, (self)->GetBuffer())) {

        message::DisplayString(self, 0, "Aborted");
        return -1;
    }
    
    return LocalReadFile(self, filename, TRUE);
}


static int frame_SaveFile(class frame  *self, long  key)
{
    class buffer *buffer = (self)->GetBuffer();

    if (buffer == NULL)
        return -1;

    return saveBuffer(self, buffer, NULL);
}

/*
 * Prompt for a filename and write the file.
 * If the buffer data object is a subclass of text, remember
 * the old WriteStyle attribute, set the attribute to 
 * NoDatastream, write the file, and then restore the old
 * WriteStyle attribute.
 */

static int frame_WritePlainestFile(class frame  *self)
{
    class buffer *buffer = (self)->GetBuffer();
    class dataobject *contents;
    enum textwritestyle oldWriteStyle;
    const char *t;

    if (buffer == NULL)
        return -1;

    contents = (buffer)->GetData();
    t = (contents)->GetTypeName();

    if (ATK::IsTypeByName(t, "text")) {
	int retval;

	oldWriteStyle = ((class text *) contents)->GetWriteStyle();
	((class text *) contents)->SetWriteStyle( text_NoDataStream);
	retval = frame_WriteFile(self);
	((class text *) contents)->SetWriteStyle( oldWriteStyle);
	return retval;
    } else {
	return frame_WriteFile(self);
    }
}

int frame_WriteFile(class frame  *self)
{
    char filename[MAXPATHLEN], tempName[256];
    class buffer *buffer = (self)->GetBuffer();

    if (buffer == NULL)
        return -1;

    bufferFilename(buffer, filename);

    if (completion::GetFilename(self, "Write to file: ", filename, filename,
			       sizeof(filename), FALSE, FALSE) == -1)
        return -1;

    if (saveBuffer(self, buffer, filename) >= 0) {
        (buffer)->SetFilename( filename); /* Sets checkpoint filename... */
        (buffer)->SetName( ""); /* So buffer_GuessBufferName does */
				    /* not return "foo-1" if our name */
				    /* is "foo" and new filename is */
				    /* ".../foo". */
        buffer::GuessBufferName(filename, tempName, sizeof(tempName));
        (buffer)->SetName( tempName);
        return 0;
    }
    else
        return -1;
}


static void frame_SaveAll(class frame  *self, long  key)
        {
    (void)saveTheWorld(self);
}

static char *frame_pwd(class frame  *self, long  key)
        {

    static char wd[MAXPATHLEN];
    char message[MAXPATHLEN + 30];

    im::GetDirectory(wd);
    sprintf(message, "Current directory is %s.", wd);
    message::DisplayString(self, 0, message);
    return wd;
}

static boolean frame_cd (class frame  *self, char  *arg)
        {

    char newdir[MAXPATHLEN];

    if (isString(arg)) 
	return (im::ChangeDirectory(arg) >= 0);

    im::GetDirectory(newdir);
    strcat(newdir, "/");

    if (completion::GetFilename(self, "Change directory to: ", newdir, newdir,
			       sizeof(newdir), TRUE, TRUE) == -1)
	return FALSE;

    if (im::ChangeDirectory(newdir) < 0) {
	message::DisplayString(self, 0, "Change directory failed.");
	return FALSE;
    }
    else {
	message::DisplayString(self, 0, "Done");
	return TRUE;
    }
}

static void frame_PrintCmd(class frame  *self, char *usepsstr)
{
    class buffer *buf;
    boolean useps;
    int pmode, res;

    pmode = (int)(long)usepsstr;
    if (pmode >= 0 && pmode < 256) {
#ifdef PSPRINTING_ENV
	useps = TRUE;
#else
	useps = FALSE;
#endif
    }
    else {
	if (*usepsstr == 't' || *usepsstr == 'T')
	    useps = FALSE;
	else
	    useps = TRUE;
    }

    pmode = (useps ? print_PRINTPOSTSCRIPT : print_PRINTTROFF);

    im::SetProcessCursor(waitCursor);
    message::DisplayString(self, 0, "Processing print request.");
    im::ForceUpdate();

    if (ATK::LoadClass("print") == NULL) {
	message::DisplayString(self, 0, "Print aborted; could not load class \"print\".");
	im::SetProcessCursor(NULL);
	return;
    }
    buf = (self)->GetBuffer();
    res = print::ProcessView((self)->GetView(), pmode, 1, (buf)->GetFilename(), "");
    if (res==0) {
	message::DisplayString(self, 0, "Print request submitted.");
    }
    else {
	/* an error occurred -- we shouldn't display any error, because the print module already has. */
    }
    im::SetProcessCursor(NULL);
}

static void frame_PreviewCmd(class frame  *self, char *usepsstr)
{
    class buffer *buf;
    boolean useps;
    int pmode, res;

    pmode = (int)(long)usepsstr;
    if (pmode >= 0 && pmode < 256) {
#ifdef PSPRINTING_ENV
	useps = TRUE;
#else
	useps = FALSE;
#endif
    }
    else {
	if (*usepsstr == 't' || *usepsstr == 'T')
	    useps = FALSE;
	else
	    useps = TRUE;
    }

    pmode = (useps ? print_PREVIEWPOSTSCRIPT : print_PREVIEWTROFF);

    im::SetProcessCursor(waitCursor);
    message::DisplayString(self, 0, "Processing preview request.");
    im::ForceUpdate();

    if (ATK::LoadClass("print") == NULL) {
	message::DisplayString(self, 0, "Print aborted; could not load class \"print\".");
	im::SetProcessCursor(NULL);
	return;
    }
    buf = (self)->GetBuffer();
    res = print::ProcessView((self)->GetView(), pmode, 1, (buf)->GetFilename(), "");
    if (res == 0) {
	message::DisplayString(self, 0, "Preview window should appear soon.");
    }
    else {
	 /* an error occurred -- we shouldn't display any error, because the print module already has. */
    }
    im::SetProcessCursor(NULL);
}

static void frame_SetPrinter(class frame  *self)
{
    const char *currentPrinter, *defaultPrinter;
    char answer[256], prompt[sizeof("Current printer is . Set printer to []: ") + 128];

    currentPrinter = environ::GetPrinter();
    defaultPrinter = environ::GetProfile("print.printer");
    if (!defaultPrinter) defaultPrinter = environ::GetProfile("print.spooldir");
    if (currentPrinter != NULL && defaultPrinter != NULL)
        sprintf(prompt, "Current printer is %.64s. Set printer to [%.64s]: ", currentPrinter, defaultPrinter);
    else if (defaultPrinter != NULL)
        sprintf(prompt, "Set printer to [%.64s]: ", defaultPrinter);
    else
        strcpy(prompt, "Set printer to: ");
    if (message::AskForString(self, 0, prompt, NULL, answer, sizeof(answer)) == -1)
        return;
    if (*answer != '\0') {
        environ::PutPrinter(answer);
        defaultPrinter = answer;
    }
    else
        environ::DeletePrinter();
    if (defaultPrinter != NULL) {
        sprintf(prompt, "Printer set to %.64s.", defaultPrinter);
        message::DisplayString(self, 0, prompt);
    }
    else
        message::DisplayString(self, 0, "Printer not set.");
}

/* Finds the frame immediately before the one passed in as the rock (nextFrame). */
static int FindFrame(class frame  *frame , class frame  *nextFrame)
{
    if (frame->next == nextFrame || frame->next == NULL)
        return TRUE;
    else
        return FALSE;
}

/* Trivially finds the first frame on the list... that has an im */
static boolean FirstFrame(class frame  *frame, long  rock)
{
    return (frame->GetIM()) ? TRUE : FALSE;
}

static void frame_PreviousWindow(class frame  *self)
{
    class frame *desiredFrame;
    do  {
	desiredFrame = frame::Enumerate((frame_effptr)FindFrame, (long) self);
	self= desiredFrame;
    } while (self && !desiredFrame->GetIM());
    if (desiredFrame != NULL)
        ((desiredFrame)->GetIM( ))->SetWMFocus();
}

static void frame_NextWindow(class frame  *self)
{
    class frame *desiredFrame= self;
    do  {
	desiredFrame = desiredFrame->next;
    } while (desiredFrame && desiredFrame->GetIM()==NULL);
    if (desiredFrame == NULL) /* end of list, didn't find one; jump to start */
	desiredFrame = frame::Enumerate((frame_effptr)FirstFrame, 0);
    if (desiredFrame != NULL)
	((desiredFrame)->GetIM())->SetWMFocus();
}

static void frame_HideWindow(class frame  *self)
    {

    ((self)->GetIM())->HideWindow();
    frame_NextWindow(self);
}

static void frame_ExposeWindow(class frame  *self)
    {

    ((self)->GetIM( ))->ExposeWindow();
    ((self)->GetIM( ))->SetWMFocus();
}

/* Can't use frame_HideWindow because it does input focus operations also. */
static boolean HideWindow(class frame  *frame)
    {

    ((frame)->GetIM( ))->HideWindow();
    return FALSE;
}

static void frame_SingleWindow(class frame  *self)
    {

    frame::Enumerate((frame_effptr)HideWindow, 0);
    frame_ExposeWindow(self);
}

static void frame_SetBufferModified(class frame  *self)
    {

    long version;
    class buffer *thisBuffer = (self)->GetBuffer();

    if (thisBuffer == NULL)
        return;

    version = ((thisBuffer)->GetData())->GetModified();
    (thisBuffer)->SetCkpClock( 0);
    (thisBuffer)->SetCkpVersion( version);
    (thisBuffer)->SetWriteVersion( version);
    message::DisplayString(self, 0, "Reset buffer modified status.");
}


/* frame-open-file(filename) returns frame
	Ness-callable: create a window and edit file in it.  
	commands (framecommand_Bindings) are enabled if this is true
	arg is filename.   Returns im
*/
	static class frame *
framecmd_OpenFile(char  *filename)
	{
    class buffer *buffer;
    class frame *frame;
    class im *window;

    if ((window = im::Create(NULL)) == NULL) {
	fprintf(stderr,"Could not create new window.\n");
        return NULL;
    }

    if ((buffer = LocalGetBufferOnFile(NULL, filename, TRUE)) == NULL)
        return NULL;

    if ((frame = frame::Create(buffer)) == NULL)
        return NULL;
    (frame)->SetCommandEnable( FALSE);

    (window)->SetView( frame);
    (frame)->PostDefaultHandler( "message", (frame)->WantHandler( "message"));
    message::DisplayString(frame, 0, "");

    preventOutofSyncLossage(frame, buffer);
    ((frame)->GetView())->WantInputFocus( (frame)->GetView());
	/* (I don't know why this is needed since WantInputFocus should be done in 
	frame_SetBuffer as called from frame_Create -wjh) */
    return frame;
}

/* frame-open-view(object) returns frame
	Ness-callable: create a window and display the view in it.  
	args:  object, buffer name
*/
	static class frame *
framecmd_OpenView(class observable  *v)
	{
    class buffer *buffer;
    class frame *frame;
    class im *window;
    class dataobject *dobj;
    char name[50];
    struct osi_Times blk;
    struct tm *This;

    /* get a buffer on a view on the given data object   (if given a view, extract its data object)
	choose an artificial name of the form Ness-hh:mm */
    if ((v)->IsType( ATK::LoadClass("dataobject"))) 
	dobj = (class dataobject *)v;
    else if ((v)->IsType( ATK::LoadClass("view"))) {
	/* want to view a view.  create a new one for the same data object */
	dobj = ((class view *)v)->GetDataObject();
	if (dobj == NULL) return NULL;
    }
    else
	return NULL;

    osi_GetTimes(&blk);
    This = localtime((time_t *) &blk.Secs);
    sprintf(name, "Ness-%d:%02d", This->tm_hour, This->tm_min);

    buffer = buffer::Create(name, NULL, NULL, dobj);

    if ((window = im::Create(NULL)) == NULL) {
	fprintf(stderr,"Could not create new window.\n");
        return NULL;
    }
    if ((frame = frame::Create(buffer)) == NULL) {
	fprintf(stderr,"Could not allocate enough memory.\n");
        return NULL;
    }
    (frame)->SetCommandEnable( FALSE);
    (window)->SetView( frame);
    (frame)->PostDefaultHandler( "message", (frame)->WantHandler( "message"));
    message::DisplayString(frame, 0, "");

    (frame)->SetBuffer( buffer, TRUE);
    ((frame)->GetView())->WantInputFocus( (frame)->GetView());

    return frame;
}

/* frame-set-window-title(frame, title)
	Ness-callable: changes the title for the buffer currently displayed in the frame
*/
	static void
framecmd_SetTitle(class frame  *self, char  *title)
		{
	((self)->GetBuffer())->SetName( title);
}

/* framecmds-set-program-name
	Ness-callable: set program name (for preferences).  args: name
*/
	static void
framecmd_SetProgramName(char  *name)
	{
	im::SetProgramName(name);
}

/* frame-set-command-enable(frame, enable)
	Ness-callable: enables/disables the frame commands.  args: frame, boolean
	returns prior state
*/
	static boolean
framecmd_SetCommands(class frame  *self, boolean  enable)
		{
	boolean wasenabled = (self)->GetCommandEnable();
	(self)->SetCommandEnable( enable);
	return wasenabled;
}

/* framecmd_Interact(self)
	call im_Interact()
*/
	static void
framecmd_Interact(class frame  *self)
	{
	im::KeyboardProcessor();
}

static const struct bind_Description framecmdDefaultBindings[]={
    {"frame-exit", "\030\003", 0, "Quit~99", 0,     frame_DefaultMenus, (proctable_fptr)frame_Exit, 
    "Exit editor.  If any buffer is modified ask for confirmation."},
#if !RCH_ENV
    {"nessm-make-macro", NULL, 0, "Ness~99,Make Macro~1", 0,     frame_DefaultMenus, NULL, 
    "Make a Ness script from a keyboard macro.", "nessm"},
    {"nessview-visit-script", NULL, 0, "Ness~99,Visit Script~41", 0,     frame_DefaultMenus, NULL, 
    "Visit an existing named script, (e.g. one loaded with ness-load in an .XXXinit file)", "nessview"},
#endif
    NULL
};

static const struct bind_Description framecmdBindings[]={

    /* the functions called by these first few check their second argument.
	If it is a pointer, it is assumed to point to a character string
	and is used instead of prompting the user. */
    {"frame-change-directory",	    "\033c",0,	    NULL,0,frame_BufferMenus,	
		(proctable_fptr)frame_cd, "Change current working directory."},
    {"frame-visit-file",	    "\030\026",0,   NULL,0,frame_BufferMenus,	
		(proctable_fptr)frame_VisitFile, "Prompts for a file to visit."},
    {"frame-visit-file-uninterpreted",	    "\030\033v",0,   NULL,0,frame_BufferMenus,	
		(proctable_fptr)frame_VisitRawFile, "Prompts for a file to visit uninterpreted."},
    {"frame-visit-file-new-window", NULL,0,   NULL,0,frame_BufferMenus,	
		(proctable_fptr)frame_VisitFileNewWindow, 
		"Prompts for a file to visit. Creates a new window for it."},
    /* end of functions that check their second arg */

    {"frame-recursive-visit-file",  "\030\024",0,   NULL,0,frame_BufferMenus,	
		(proctable_fptr)frame_RecursiveVisitFile, 
		"Visit file saving current buffer state."},
    {"frame-recursive-edit",	    NULL,0,	    NULL,0,frame_BufferMenus,	
		(proctable_fptr)frame_RecursiveEdit, "Enter a recursive edit."},
    {"frame-exit-recursive-edit",   "\003",0,	    NULL,0,frame_BufferMenus,	
		(proctable_fptr)frame_ExitRecursiveEdit, 
		"Exit after frame-recursive-edit or frame-recursive-visit-file"},
    {"frame-delete-window",	    "\030\004",0,   "Delete Window~89",0,
                frame_BufferMenus, (proctable_fptr)frame_DeleteWindow, 
		"Delete the window in which command is executed."},
    {"frame-current-directory",	    "\033p",0,	    NULL,0,frame_BufferMenus,	
		(proctable_fptr)frame_pwd, "Show current working directory."},
    {"frame-new-window",	    "\0302",0,	    NULL,0,frame_BufferMenus,	
		(proctable_fptr)frame_NewWindow, "Creates a new window."},
    {"frame-delete-buffer",	    "\030k",0,	    NULL,0,frame_BufferMenus,	
    (proctable_fptr)frame_DeleteBufferPrompt, "Prompts for a buffer to delete."},
    {"frame-delete-this-buffer",    NULL,0,	    NULL,0,frame_BufferMenus,
    (proctable_fptr)frame_DeleteThisBuffer, "Deletes buffer in current window without prompting."},
    {"frame-old-buffer",	    "\030\017",0,   NULL,0,frame_BufferMenus,	
		(proctable_fptr)frame_OldBuffer, "Visits an already existing buffer."},
    {"frame-visit-buffer",	    "\030b",0,	    NULL,0,frame_BufferMenus,	
		(proctable_fptr)frame_VisitBuffer, "Changes to an arbitrarily named buffer."},
    {"frame-list-buffers",	    "\030\002",0,   NULL,0,frame_BufferMenus,	
		(proctable_fptr)frame_ListBuffers, "Lists current buffers."},
    {"frame-drop-buffer",	    NULL,0,   NULL,0,frame_BufferMenus,	
		(proctable_fptr)frame_DropBuffer, "Drop out the current buffer."},
    {"frame-switch-file",	    "\030\022",0,   "Switch File~30",0,frame_BufferMenus,
		(proctable_fptr)frame_SwitchFile, 
		"Switches to editing another buffer."},
    {"frame-read-file",		    NULL,0,   NULL,0,frame_BufferMenus,	
		(proctable_fptr)frame_ReadFile, 
		"Reads a file into the current buffer."},
    {"frame-save-file",		    "\030\023",0,   "Save~20",0,frame_BufferMenus, 
		(proctable_fptr)frame_SaveFile, 
		"Saves buffer into its current file."},
    {"frame-write-file",	    "\030\027",0,   "File~10,Save As~1",0,
                frame_BufferMenus, (proctable_fptr)frame_WriteFile,
		"Prompts for a file to save the current buffer in."},
    {"frame-write-plainest-file", "\030\033w",  0, NULL, 0,
                frame_BufferMenus, (proctable_fptr)frame_WritePlainestFile,
		"Prompts for a file to save the current buffer into with styles removed."},
    {"frame-save-all-files",	    "\030\015",0,   "File~10,Save All~2",0,
                frame_BufferMenus, (proctable_fptr)frame_SaveAll, "Saves all files."},
#ifdef SET_PRINTER_COMMAND_ENV
    {"frame-set-printer",	    NULL,0,	 "File~10,Set Printer~20",0,
                frame_BufferMenus, (proctable_fptr)frame_SetPrinter, 
		"Set the printer for the print command to spool to."},
#else /* SET_PRINTER_COMMAND_ENV */
/* Define the proctable entry but don't bind it to a menu.*/
    {"frame-set-printer",	    NULL,0, NULL,0,
                frame_BufferMenus, (proctable_fptr)frame_SetPrinter, 
		"Set the printer for the print command to spool to."},
#endif /* SET_PRINTER_COMMAND_ENV */
#ifdef RCH_ENV
#if !SY_OS2
		{"rchprint-print",	    NULL,0,	 "File~10,Print...~20",0,
		frame_BufferMenus, NULL, 
		"Print the current document.", "rchprint"},
		{"rchprint-preview",    NULL,0,	 "File~10,Preview...~21",0,
		frame_BufferMenus, NULL, 
		"Preview the current document.", "rchprint"},
		{"frame-preview",		    NULL,0,	    NULL,0,
		frame_BufferMenus, (proctable_fptr)frame_PreviewCmd, "Previews document."},
		{"frame-print",		    NULL,0,	    NULL,0,
		frame_BufferMenus, (proctable_fptr)frame_PrintCmd, "Prints document."},
#else /*!SY_OS2*/
#endif /*!SY_OS2*/
#else

#ifdef PRINTER_SETUP_DIALOG_ENV
    {"printopts-post-window",	    NULL,0,	 "File~10,Printer Setup~20",0,
                frame_BufferMenus, NULL, 
		"Pop up dialogue box to set printer name and other parameters.", "printopts"},
#endif /* PRINTER_SETUP_DIALOG_ENV */
    {"frame-preview",		    NULL,0,	    "File~10,Preview~21",0,
                frame_BufferMenus, (proctable_fptr)frame_PreviewCmd, "Previews document."},
    {"frame-print",		    NULL,0,	    "File~10,Print~22",0,
    frame_BufferMenus, (proctable_fptr)frame_PrintCmd, "Prints document."},
#endif
    {"frame-previous-window",	    NULL,0,	    NULL,0,frame_BufferMenus,	
		(proctable_fptr)frame_PreviousWindow, 
		"Moves the input focus to the \"previous\" window."},
    {"frame-next-window",	    NULL,0,	    NULL,0,frame_BufferMenus,	
		(proctable_fptr)frame_NextWindow, 
		"Moves the input focus to the \"next\" window."},
    {"frame-set-buffer-modified",   NULL,0,	    NULL,0,frame_BufferMenus,	
		(proctable_fptr)frame_SetBufferModified, 
		"Sets buffer to be unmodified, unless given a prefix argument, in which case it sets the buffer to be modified."},
    {"frame-hide-window",	    NULL,0,	    NULL,0,frame_BufferMenus,	
		(proctable_fptr)frame_HideWindow, 
		"Hides the window this command is executed in."},
    {"frame-expose-window",	    NULL,0,	    NULL,0,frame_BufferMenus,	  
		(proctable_fptr)frame_ExposeWindow, 
		"Exposes the window this command is executed in."},
    {"frame-single-window",	    NULL,0,	    NULL,0,frame_BufferMenus,	  
		(proctable_fptr)frame_SingleWindow, 
		"Hides all other windows and expands the current window."},
    NULL
};
static struct proctable_DescriptionWithType procswithframearg[] = {
	{"frame-visit-file", NULL, NULL, NULL, NULL, proctable_Long},
		/* returns 0 or greater for success */
	{"frame-visit-file-new-window", NULL, NULL, NULL, NULL, proctable_Long},
		/* returns 0 or greater for success */
	{"frame-change-directory", NULL, NULL, NULL, NULL, proctable_Boolean},
		/* returns TRUE for success */
	{"frame-current-directory", NULL, NULL, NULL, NULL, proctable_StaticString},
		/* returns a string with the current directory name */

	{"framecmds-set-window-title", (proctable_fptr)framecmd_SetTitle, NULL, 
		"Ness-callable: set window title.  args: frame, title", 
		"framecmds", proctable_Void},
	{"framecmds-set-command-enable", (proctable_fptr)framecmd_SetCommands, NULL, 
		"Ness-callable: enable/disable frame commands.  args: frame, enable.  Returns prior state.", 
		"framecmds", proctable_Boolean},
	NULL
};

static const struct proctable_DescriptionWithType procswithrandomarg[] = {
	{"framecmds-interact", (proctable_fptr)framecmd_Interact, NULL, 
		"Ness-callable: interact with user", 
		"framecmds", proctable_Void},
	{"framecmds-open-file", (proctable_fptr)framecmd_OpenFile, NULL, 
		"Ness-callable: create a window and edit a file in it.  arg: filename.  Returns frame", 
		"framecmds", proctable_Object},
	{"framecmds-open-view", (proctable_fptr)framecmd_OpenView, NULL, 
		"Ness-callable: create a window and display the view in it.  args: object.  Returns frame", 
		"framecmds", proctable_Object},
	{"framecmds-set-program-name", (proctable_fptr)framecmd_SetProgramName, NULL, 
		"Ness-callable: set program name (for preferences).  args:  name", 
		"framecmds", proctable_Void},
	NULL
};


class keymap *framecmds::InitKeymap(class menulist  **menuOut , class menulist  **menuDefaultOut, class keymap  **defkeymap)
{
	ATKinit;

    *menuOut = framecmdsMenus;
    *menuDefaultOut = framecmdsDefaultMenus;
    *defkeymap=framecmdsDefaultKeymap;
    return framecmdsKeymap;
}

boolean framecmds::InitializeClass()
{
    struct ATKregistryEntry  *classInfo;
    struct proctable_DescriptionWithType *pt;

    framecmdsKeymap = new keymap;
    framecmdsDefaultKeymap = new keymap;
    framecmdsMenus = new menulist;
    framecmdsDefaultMenus = new menulist;

    waitCursor = cursor::Create(NULL);
    (waitCursor)->SetStandard( Cursor_Wait);

    classInfo = ATK::LoadClass("frame");
    bind::BindList(framecmdBindings, framecmdsKeymap, framecmdsMenus, classInfo);
    bind::BindList(framecmdDefaultBindings, framecmdsKeymap, framecmdsDefaultMenus, classInfo);
    bind::BindList(framecmdDefaultBindings, framecmdsDefaultKeymap, NULL, classInfo);
    
    proctable::DefineProcsWithTypes(procswithrandomarg);
    for (pt = &procswithframearg[0]; pt->name != NULL; pt++)
	pt->type = classInfo;
    proctable::DefineProcsWithTypes(procswithframearg);
    return TRUE;
}
