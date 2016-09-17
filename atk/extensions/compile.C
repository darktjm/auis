/*LIBS: -lbasics
*/

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

/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'     *
\* ********************************************************************** */

 

/* compile.c
 * compilation package for be2 based editor.
 */

#include <andrewos.h>
ATK_IMPL("compile.H")

#include <ctype.h>
#include <fcntl.h>
#include <signal.h>

#include <view.H>
#include <dataobject.H>
#include <im.H>
#include <message.H>
#include <text.H>
#include <textview.H>
#include <frame.H>
#include <buffer.H>
#include <proctable.H>
#include <search.H>
#include <filetype.H>
#include <style.H>
#include <rect.h>
#include <mark.H>
#include <environment.H>
#include <environ.H>
#include <stylesheet.H>

#include <compile.H>

static class style *boldStyle = NULL;

#define Text(thisView) ((class text *) thisView->dataobject)

struct lengthPair {
    long first, second;
};





ATKdefineRegistry(compile, ATK, compile::InitializeClass);
static boolean SetDotToEnd(class view  *applicationView , class view  *targetView , class view  *inputFocus, struct lengthPair  *lengths);
static void InsertMessage(class buffer  *b, long  pos, const char  *string, long  len);
static struct process *StartProcess(const char  *command, FILE  **inputFile , FILE  **outputFile);
static int FinishProcess(struct process  *process);
static class buffer *MakeCommandBuffer(const char  *command , const char  *buffername, im_filefptr handler);
static void compile_BuildHandler(FILE  *inputFile, struct processbuffer  *processBuffer);
static void compile_KillBuild(class view  *view, long  key);
static void compile_SetCommand(char  *command);
static boolean SaveModifiedBuffer(class buffer  *b, class view  *messageView);
static boolean SaveAllBuffers(class view  *view);
static void compile_Build(class view  *view, long  key);
static void resetErrors();
static int getlinepos(class text  *doc, int  line);
static long nextlinepos(class text  *doc, long  pos);
static int ParseCCError(class text  *doc, long  *startPos, char  *fileName, int  maxSize);
static int ParseEgrepError(class text  *doc, long  *startPos, char  *fileName, int  maxSize);
#ifdef SGI_4D_ENV
static int ParseSGIError(class text  *doc, long  *startPos, char  *fileName, int  maxSize);
#endif /* SGI_4D_ENV */
static int ParseHCError(class text  *doc, long  *startPos, char  *fileName, int  maxSize);
static int ParseCD(class text  *doc, long  *startPos, char  *directory, int  maxSize);
static int ParseEntry(char  *b, class text  *doc, long  *startPos, int  maxSize);
static struct errorList *MakeErrorList(class buffer  *errorBuffer);
static void compile_NextError(class view  *view, int  key);
static void compile_PreviousError(class view  *view, int  key);
static boolean FrameFinder(class frame  *frame, struct finderInfo  *info);
static int ViewEqual(class frame  *frame, class view  *view);
static class frame *FindByView(class view  *view);
static class view *PutInAnotherWindow(class view  *view, class buffer  *b, int  forceWindow);
static class view *PopToMark(class mark  *mark, int  useWindowFlag, class textview  *textview);


static boolean SetDotToEnd(class view  *applicationView , class view  *targetView , class view  *inputFocus, struct lengthPair  *lengths)
        {

    if ((targetView)->IsType( ATK::LoadClass("textview"))) {

        class textview *thisView = (class textview *) targetView;

        if ((thisView)->GetDotPosition() == lengths->first) {
            (thisView)->SetDotPosition( lengths->second);
            (thisView)->FrameDot( lengths->second);
        }
    }
    return FALSE; /* Keep on enumerating. */
}

static void InsertMessage(class buffer  *b, long  pos, const char  *string, long  len)
                {

    class environment *tempEnv;
    class text *doc = (class text *) (b)->GetData();
    struct lengthPair lengths;

    (doc)->InsertCharacters( pos, string, len);
    (doc)->InsertCharacters( pos + len, "\n", 1);
    if (boldStyle != NULL) {
        tempEnv = (doc)->AddStyle( pos, len + 1, boldStyle);
        (tempEnv)->SetStyle( FALSE, FALSE);
    }
    lengths.first = pos;
    lengths.second = pos + len + 1;
    (b)->EnumerateViews( (buffer_evfptr)SetDotToEnd, (long) &lengths);
    (doc)->NotifyObservers( 0);
}

/* Process handling stuff. */

#define READFD 0	/* The pipe index from which you can do a read. */
#define WRITEFD 1	/* The pipe index to which you can do a write. */

struct process {
    FILE *inFile, *outFile;
    short pid;
    boolean stopped;
};

struct processbuffer {
    struct process *process;
    class buffer *b;
};

static struct process *StartProcess(const char  *command, FILE  **inputFile , FILE  **outputFile)
        {

    int inpipe[2], outpipe[2], pid;
    struct process *thisProc;

/* Create pipe for parent reading from child. */
    if (inputFile != NULL) {
        if (pipe(inpipe) < 0)
            return 0;
        *inputFile = fdopen(inpipe[READFD], "r");
        fcntl(inpipe[READFD], F_SETFD, 1); /* Set close on exec flag. */
    }
    else
        if ((inpipe[WRITEFD] = open("/dev/null", O_WRONLY) < 0))
            return 0;

/* Create pipe for parent writing to child. */
    if (outputFile != NULL) {
        if (pipe(outpipe) < 0)
            return 0;
        *outputFile = fdopen(outpipe[WRITEFD], "w");
        fcntl(outpipe[WRITEFD], F_SETFD, 1); /* Set close on exec flag. */
    }
    else
        if ((outpipe[READFD] = open("/dev/null", O_RDONLY)) < 0)
            return 0;

    if ((pid = osi_vfork()) == 0) { /* In child */

        int numfds = FDTABLESIZE();
        int fd;

        dup2(outpipe[READFD], 0);
        dup2(inpipe[WRITEFD], 1);
        dup2(1, 2); /* Should have more control over this. */

        /* Don't leave any open file descriptors for child. */
        for (fd = 3; fd < numfds; fd++)
            close(fd);

	NEWPGRP();
	
        execlp("/bin/sh", "sh", "-c", command, (char *)NULL);
        exit(0);
    }

    close(outpipe[READFD]);
    close(inpipe[WRITEFD]);

    thisProc = (struct process *) malloc(sizeof(struct process));
    if (inputFile != NULL)
        thisProc->inFile = *inputFile;
    else
        thisProc->inFile = NULL;
    if (outputFile != NULL)
        thisProc->outFile = *outputFile;
    else
        thisProc->outFile = NULL;
    thisProc->pid = pid;
    thisProc->stopped = FALSE;
    return thisProc;
}

/* Kills process with terminate signal. Trys to cleanup everything. */
static int FinishProcess(struct process  *process)
    {

    if (process->pid > 0) /* If we have a process running. */
        KILLPG(process->pid, SIGTERM);
#ifdef SIGCONT	    /* SIGCONT not currently supported on hp9000s300 */
    if (process->stopped) /* I think this works. */
        KILLPG(process->pid, SIGCONT);
#endif /* SIGCONT	     */
    if (process->inFile != NULL)
        fclose(process->inFile);
    if (process->outFile != NULL)
        fclose(process->outFile);
    free(process);
    return 0;
}

/* Process and comman global state. */
static struct process *currentProcess = NULL; /* pid of compilation process, is 0 if none currently executing. */
static const char *defaultCommand;
static char *compileCommand;

/* Starts a process dumping output into a buffer. */
static class buffer *MakeCommandBuffer(const char  *command , const char  *buffername, im_filefptr handler)
        {

    class text *commandLogDoc;
    class buffer *commandLog;
    FILE *inputFile;

    if ((commandLog = buffer::FindBufferByName(buffername)) == NULL) {
        commandLog = buffer::Create(buffername, NULL, "text", NULL);
        (commandLog)->SetScratch( TRUE);
        commandLogDoc = (class text *) (commandLog)->GetData();
    }
    else {
        commandLogDoc = (class text *) (commandLog)->GetData();
        (commandLogDoc)->Clear();
        (commandLogDoc)->NotifyObservers( 0);
    }

    if ((commandLogDoc)->ReadTemplate( "compile", FALSE) < 0)
        boldStyle = NULL;
    else
        boldStyle = (commandLogDoc->styleSheet)->Find( "bold");

    if ((currentProcess = StartProcess(command, &inputFile, NULL)) != 0) {
        
        struct processbuffer *processBuffer = (struct processbuffer *) malloc(sizeof(struct processbuffer));

        processBuffer->b = commandLog;
        processBuffer->process = currentProcess;
	im::AddFileHandler(inputFile, handler, (char *) processBuffer, 3); /* Priority chosen at random! Should be checked and replaced. */
        return commandLog;
    }
    else
        return NULL;
}

static void compile_BuildHandler(FILE  *inputFile, struct processbuffer  *processBuffer)
        {

    char buffer[BUFSIZ];
    class text *logDoc = (class text *) (processBuffer->b)->GetData();

    if (fgets(buffer, sizeof(buffer), inputFile)) {

        long length = (logDoc)->GetLength(), len;
        struct lengthPair lengths;
        
        (logDoc)->InsertCharacters( length, buffer, len = strlen(buffer));
        lengths.first = length;
        lengths.second = length + len;
	(processBuffer->b)->EnumerateViews( (buffer_evfptr)SetDotToEnd, (long) &lengths);
        (logDoc)->NotifyObservers( 0);
    }
    else {
        if (FinishProcess(currentProcess) != -1) {
            currentProcess = NULL;
            InsertMessage(processBuffer->b, (logDoc)->GetLength(), "End", sizeof("End") - 1);
        }
        free(processBuffer);
        im::RemoveFileHandler(inputFile);
    }
}

static void compile_KillBuild(class view  *view, long  key)
        {

    if (currentProcess != NULL) {
        if ((((class view *) view)->GetIM())->ArgProvided()) {
            if (currentProcess->stopped)
                message::DisplayString(view, 0, "Build is already stopped.");
            else {
#ifdef SIGSTOP	 /* not currently supported on HP9000S300 */
                currentProcess->stopped = TRUE;
                KILLPG(currentProcess->pid, SIGSTOP);
                message::DisplayString(view, 0, "Build stopped.");
#else /* SIGSTOP	  */
		message::DisplayString(view, 0, "Build not stopped (job control not supported).");
#endif /* SIGSTOP	  */
            }
            return;
        }
        else {

            class buffer *logBuffer = buffer::FindBufferByName("Error-Log");
            class text *logDoc = (class text *) (logBuffer)->GetData();

            im::RemoveFileHandler(currentProcess->inFile);
            FinishProcess(currentProcess);
            InsertMessage(logBuffer, (logDoc)->GetLength(), "Killed!", sizeof("Killed!") - 1);
            currentProcess = NULL;
            message::DisplayString(view, 0, "Dead!");
        }
    }
    else
        message::DisplayString(view, 0, "You don't have a subprocess to kill (or stop).");
}

static void compile_SetCommand(char  *command)
    {

    if (compileCommand != defaultCommand)
        free(compileCommand);
    if (command == NULL) {
        compileCommand = (char *)defaultCommand; /* not freed if default */
        return;
    }
    compileCommand = strdup(command);
}

static boolean SaveModifiedBuffer(class buffer  *buffer, class view  *messageView)
        {

    if (((buffer)->GetWriteVersion() < ((buffer)->GetData())->GetModified()) && ((buffer)->GetFilename() != NULL))
        while (TRUE)
            if ((buffer)->WriteToFile( NULL, buffer_ReliableWrite) < 0) { /* Save to buffer filename */

                char message[150], response[2];

                sprintf(message, "Couldn't save \"%.100s\", Try again, Quit, Ignore [t]? ", (buffer)->GetFilename());
                if ((message::AskForString(messageView, 0, message, NULL, response, sizeof(response)) == -1) || *response == 'q')
                    return TRUE; /* Stop enumerating here... */
                else if (*response == 'i')
                    return FALSE; /* Oh well, continue anyway... */
            }
            else {

                long version = ((buffer)->GetData())->GetModified();

                unlink((buffer)->GetCkpFilename());
                (buffer)->SetCkpClock( 0);
                (buffer)->SetCkpVersion( version);
                (buffer)->SetWriteVersion( version);
                return FALSE; /* Keep on enumerating. */
            }
    return FALSE;
}

/* Save all modified buffers giving appropriate user feedback... */
static boolean SaveAllBuffers(class view  *view)
    {

    return (buffer::Enumerate((bufferlist_efptr)SaveModifiedBuffer, (long) view) == NULL);
}



static void compile_Build(class view  *view, long  key)
        {

    char buffer[BUFSIZ];
    int askForCommand;
    class buffer *commandBuffer;

    askForCommand = ((view)->GetIM())->ArgProvided();
    ((view)->GetIM())->ClearArg();

    if (currentProcess != NULL) {

        int needAnswer;

        if (currentProcess->stopped) {
            currentProcess->stopped = FALSE;
#ifdef SIGCONT
            KILLPG(currentProcess->pid, SIGCONT);
#endif /* SIGCONT */
            message::DisplayString(view, 0, "Build continuing");
            return;
        }

        do {
            needAnswer = 0;
            if (message::AskForString(view, 0, "You already have a compilation running, kill it [n]? ", NULL, buffer, 2) < 0)
                return;
            if (*buffer == '\0')
                *buffer = 'n';
            switch (*buffer) {
                case 'n':
                case 'N':
                    return;
                    /* break; */
                case 'y':
                case 'Y':
                    compile_KillBuild(view, key);
                    break;
                default:
                    message::DisplayString(view, 0, "You must answer 'y' or 'n'. (Press any key to continue).");
                    ((view)->GetIM())->GetCharacter();
                    needAnswer = 1;
                    break;
            }
        } while (needAnswer);
    }
    resetErrors();
    if (!SaveAllBuffers(view))
        return;

    if (askForCommand) {
        if (message::AskForString(view, 0, "Compile command: ", compileCommand, buffer, sizeof(buffer)) != -1)
            compile_SetCommand(buffer);
        else
            return;
    }
    commandBuffer = MakeCommandBuffer(compileCommand, "Error-Log", (im_filefptr) compile_BuildHandler);
    if (view->dataobject != (commandBuffer)->GetData()) /* If not already looking at the error log. */
        PutInAnotherWindow(view, commandBuffer, FALSE);
    InsertMessage(commandBuffer, 0, compileCommand, strlen(compileCommand));
    message::DisplayString(view, 0, "Started compile, output is in buffer \"Error-Log\".");
}

/* Error parsing stuff. */

/* Convert an entire doc full of error messages into an error list. */

/* Data structure for holding the list of errors. */
struct errorList {
    class mark *messageMark;
    class mark *offendingCodeMark;
    struct errorList *next;
    struct errorList *previous;
};

/* Globals for maintaining error state. Probably shouldn't be so singular. */
static int errorPos = 0; /* Position of last error parsed. */
static char currentDirectory[400] = ".";
static struct errorList *allErrors = NULL, *currentError = NULL, *lastParsed = NULL, **lastParsedLink = &allErrors;

/* Remove all pending errors from the error list. */
static void resetErrors()
{

    struct errorList *tempList;

    errorPos = 0;
    while (allErrors != NULL) {
        tempList = allErrors->next;
        free(allErrors);
        allErrors = tempList;
    }
    currentError = NULL;
    im::GetDirectory(currentDirectory);
    lastParsed = NULL;
    lastParsedLink = &allErrors;
}

static int getlinepos(class text  *doc, int  line)
        {

    int pos, len, i = 1;

    len = (doc)->GetLength();
    for (pos = 0; (i < line) && (pos < len); pos++)
	if ((doc)->GetChar( pos) == '\n')
	    ++i;
    return pos;
}

static long nextlinepos(class text  *doc, long  pos)
        {

    int tempChar;

    while ((tempChar = (doc)->GetChar( pos)) != '\n' && tempChar != EOF)
        pos++;
    if (tempChar == '\n')
        pos++;
    return pos;
}

 /* "foo.c".*line.*123:... */
static int ParseCCError(class text  *doc, long  *startPos, char  *fileName, int  maxSize)
                {

    long docPos = *startPos, currentChar;
    char lineNumberBuffer[32], *lineNumberChars = lineNumberBuffer, *origFilename = fileName;

    if ((doc)->GetChar( docPos) == '"') {
        while (((currentChar = (doc)->GetChar( ++docPos)) != '"') && (currentChar != '\n') &&
               (currentChar != EOF) && (fileName - origFilename < maxSize))
            *fileName++ = currentChar;
        *fileName = '\0';
        if (currentChar == '"') {
            while (((currentChar = (doc)->GetChar( ++docPos)) != 'l') && (currentChar != '\n') &&
                   (currentChar != EOF))
                ;
            if ((doc)->GetChar( docPos) == 'l' && (doc)->GetChar( ++docPos) == 'i' &&
                (doc)->GetChar( ++docPos) == 'n' && (doc)->GetChar( ++docPos) == 'e') {

                while ((doc)->GetChar( ++docPos) == ' ')
                    ;
                while (isdigit(*lineNumberChars = (doc)->GetChar( docPos))) {
                    lineNumberChars++;
                    docPos++;
                }
                *lineNumberChars = '\0';
                *startPos = nextlinepos(doc, docPos);
                return atoi(lineNumberBuffer);
            }
        }
        return -1; /* I don't know what this is... */
    }
    return -1;
}

/* foo.c:123:... */
static int ParseEgrepError(class text  *doc, long  *startPos, char  *fileName, int  maxSize)
                {

    long docPos = *startPos, currentChar;
    char lineNumberBuffer[32], *lineNumberChars = lineNumberBuffer, *origFilename = fileName;

    while (((currentChar = (doc)->GetChar( docPos++)) != ':') && (currentChar != ' ') &&
           (currentChar != '\n') && (currentChar != EOF) && (fileName - origFilename < maxSize))
        *fileName++ = currentChar;
    *fileName = '\0';
    if (currentChar == ':') {
        while (!isdigit(currentChar = (doc)->GetChar( docPos)) && (currentChar != '\n') &&
               (currentChar != EOF))
            docPos++;
        if (!isdigit(currentChar))
            return -1;
        while (isdigit(*lineNumberChars = (doc)->GetChar( docPos))) {
            lineNumberChars++;
            docPos++;
        }
        *lineNumberChars = '\0';
        *startPos = nextlinepos(doc, docPos);
        return atoi(lineNumberBuffer);
    }
    return -1;
}

#ifdef SGI_4D_ENV

/* ccom: Error: file.c, line 123:... */
static int ParseSGIError(class text  *doc, long  *startPos, char  *fileName, int  maxSize)
                {
    long docPos = *startPos, currentChar;
    long lineNumber;
    char *origFilename = fileName;
    char *line = " line ";

    /* Skip over program name */
    while ((currentChar = (doc)->GetChar( docPos++)) != ':') {
	if (currentChar == EOF || currentChar == '\n') {
	    return -1;
	}
    };
    /* Skip over type of error */
    while ((currentChar = (doc)->GetChar( docPos++)) != ':') {
	if (currentChar == EOF || currentChar == '\n') {
	    return -1;
	}
    };
    /* Skip White Space until file name */
    while ((currentChar = (doc)->GetChar( docPos++)) == ' ') {
	if (currentChar == EOF || currentChar == '\n') {
	    return -1;
	}
    }
    /* Get File Name */
    *fileName++ = currentChar;
    while ((currentChar = (doc)->GetChar( docPos++)) != ' '
	    && currentChar != ','
	    && (fileName - origFilename < maxSize)) {
	if (currentChar == EOF || currentChar == '\n') {
	    return -1;
	}
	*fileName++ = currentChar;
    }
    *fileName = '\0';
    /* Skip " line " */
    while (*line != '\0' && (currentChar = (doc)->GetChar( docPos++)) == *line++) {
	if (currentChar == EOF || currentChar == '\n') {
	    return -1;
	}
    }
    /* Get line number */
    lineNumber = 0;
    while ((currentChar = (doc)->GetChar( docPos++)) != ':') {
	if (currentChar == EOF || currentChar == '\n'
	    || ! isdigit(currentChar)) {
	    return -1;
	}
	lineNumber = (lineNumber * 10) + (currentChar - '0');
	*startPos = nextlinepos(doc, docPos);
    }
    return lineNumber;
}
#endif /* SGI_4D_ENV */

/* <'E' or 'w'> "foo.c", L<line>,<character>... */
static int ParseHCError(class text  *doc, long  *startPos, char  *fileName, int  maxSize)
                {

    long docPos = *startPos, currentChar;
    char lineNumberBuffer[32], *lineNumberChars = lineNumberBuffer, *origFilename = fileName;

    if (((currentChar = (doc)->GetChar( docPos++)) == 'E' ||
        currentChar == 'w') && (doc)->GetChar( docPos++) == ' ' &&
        (doc)->GetChar( docPos++) == '"') {

        while ((currentChar = (doc)->GetChar( docPos++)) != '\n' &&
               currentChar != '"' && (fileName - origFilename < maxSize))
            *fileName++ = currentChar;
        *fileName = '\0';
        if (currentChar != '"' || (doc)->GetChar( docPos++) != ',' ||
	    (doc)->GetChar( docPos++) != 'L')
	    return -1;
        while (isdigit(*lineNumberChars = (doc)->GetChar( docPos++)))
            lineNumberChars++;
	*lineNumberChars = '\0';
        do {
            docPos = nextlinepos(doc, docPos);
        } while ((doc)->GetChar( docPos) == '|'); /* While we have a continuation lines. */
        *startPos = docPos;
        return atoi(lineNumberBuffer);
    }
    return -1;
}

static int ParseCD(class text  *doc, long  *startPos, char  *directory, int  maxSize)
                {

    char *origDirectory = directory;
    long docPos = *startPos, currentChar;

    if (((doc)->GetChar( docPos++) == 'c') && ((doc)->GetChar( docPos++) == 'd')) { /* cd directory */
        while ((currentChar = (doc)->GetChar( docPos)) == ' ')
            docPos++;
        while ((currentChar = (doc)->GetChar( docPos)) != ' ' && currentChar != '\n' && (currentChar != EOF) && (directory - origDirectory < maxSize)) {
            *directory++ = currentChar;
            docPos++;
        }
        *directory = '\0';
        *startPos = nextlinepos(doc, docPos);
        return 0;
    }
    return -1;
}

/* Parse input file text into line number and filename (or cd command). */
static int ParseEntry(char  *buffer, class text  *doc, long  *startPos, int  maxSize)
                {

    int line;

    if ((line = ParseCCError(doc, startPos, buffer, maxSize)) > -1)
        return line;
#ifdef SGI_4D_ENV
    else if ((line = ParseSGIError(doc, startPos, buffer, maxSize)) > -1)
        return line;
#endif
    else if ((line = ParseEgrepError(doc, startPos, buffer, maxSize)) > -1)
        return line;
    else if ((line = ParseHCError(doc, startPos, buffer, maxSize)) > -1)
        return line;
    else if ((line = ParseCD(doc, startPos, buffer, maxSize)) > -1)
        return line;
    else {
        *startPos = nextlinepos(doc, *startPos);
        return -1;
    }
}


static struct errorList *MakeErrorList(class buffer  *errorBuffer)
    {

    long searchPos, startPos, lineNumber, lastLine = -1;
    char filenameBuffer[100], realFilename[200], lastFilename[200];
    class text *errorDoc = (class text *) (errorBuffer)->GetData();
    class buffer *buffer;
    struct errorList *firstError = NULL;

    lastFilename[0] = '\0'; /* lastFileName should be part of state. */
    searchPos = errorPos;
    while (searchPos < (errorDoc)->GetLength()) {
        startPos = searchPos; /* For making mark. */
        lineNumber = ParseEntry(filenameBuffer, errorDoc, &searchPos, sizeof(filenameBuffer) - 1);
        if (lineNumber > 0) {
            if (*filenameBuffer != '/') {
                strcpy(realFilename, currentDirectory);
                strcat(realFilename, "/");
            }
            else
                *realFilename = '\0';
            strcat(realFilename, filenameBuffer);
            if (strcmp(realFilename, lastFilename) || lineNumber != lastLine) {
                buffer = (class buffer *) buffer::GetBufferOnFile(realFilename, buffer_MustExist);
                if (buffer != NULL) {

                    int tempPos;
                    struct errorList *thisError = (struct errorList *) malloc(sizeof(struct errorList));

                    thisError->messageMark = (errorDoc)->CreateMark( startPos, searchPos - startPos);
                    (thisError->messageMark)->SetStyle( FALSE, FALSE);
                    tempPos = getlinepos((class text *) (buffer)->GetData(), lineNumber);
                    thisError->offendingCodeMark = ((class text *) (buffer)->GetData())->CreateMark(
                                                                   tempPos,
                                                                   nextlinepos((class text *) (buffer)->GetData(), tempPos) - tempPos);
                    thisError->previous = lastParsed;
                    thisError->next = NULL;
                    lastParsed = thisError;
                    *lastParsedLink = thisError;
                    lastParsedLink = &(thisError->next);

                    if (firstError == NULL)
                        firstError = thisError;

                    strcpy(lastFilename, realFilename);
                    lastLine = lineNumber;
                }
                else {

                    char errorMessageBuffer[100];

                    sprintf(errorMessageBuffer, "Couldn't find file: %s", filenameBuffer);
                    InsertMessage(errorBuffer, searchPos, errorMessageBuffer, strlen(errorMessageBuffer));
                    searchPos = nextlinepos(errorDoc, searchPos);
                }
            }
            else {
                (lastParsed->messageMark)->SetLength( searchPos - (lastParsed->messageMark)->GetPos());
            }
        }
        else if (lineNumber == 0) {

            char tempBuffer[400];

            if (*filenameBuffer != '/') {
                strcat(currentDirectory, "/");
                strcat(currentDirectory, filenameBuffer);
                filetype::CanonicalizeFilename(tempBuffer, currentDirectory, sizeof(tempBuffer));
                strcpy(currentDirectory, tempBuffer);
                if (*currentDirectory == '\0') { /* Keeps appending a slash from starting at root. */
                    currentDirectory[0] = '.';
                    currentDirectory[1] = '\0';
                }
            }
            else
                filetype::CanonicalizeFilename(currentDirectory, filenameBuffer, sizeof(currentDirectory));
        }
        errorPos = searchPos;
    }
    return firstError;
}

/* Pop to the next set of marks on the error list. */
static void compile_NextError(class view  *view, int  key)
        {

    if (currentError != NULL)
            currentError = currentError->next;
    if (currentError == NULL) {

        class buffer *errorBuffer = buffer::FindBufferByName("Error-Log");

        if (errorBuffer == NULL) {
            message::DisplayString(view, 0, "Can't find compilation buffer.");
            return;
        }

        message::DisplayString(view, 0, "Parsing errors...");
        im::ForceUpdate();
        currentError = MakeErrorList(errorBuffer);
    }

    if (currentError != NULL) {
        PopToMark(currentError->messageMark, FALSE, (class textview *)view);
        PopToMark(currentError->offendingCodeMark, TRUE, (class textview *)view);
    }
    else
        if (currentProcess != NULL)
            message::DisplayString(view, 0, "Not done finding all your mistakes yet.");
        else
            message::DisplayString(view, 0, "I find no more errors!");
}

/* Pop to the previous set of marks on the error list. */
static void compile_PreviousError(class view  *view, int  key)
        {

    if (currentError == NULL) {

        class buffer *errorBuffer = buffer::FindBufferByName("Error-Log");

        if (errorBuffer == NULL) {
            message::DisplayString(view, 0,  "Can't find compilation buffer.");
            return;
        }

        message::DisplayString(view, 0, "Parsing errors...");
        im::ForceUpdate();
        currentError = MakeErrorList(errorBuffer);
    }

    if (currentError == NULL)
        currentError = lastParsed;

    if (currentError != NULL) {
        if (currentError->previous != NULL)
            currentError = currentError->previous;
        PopToMark(currentError->messageMark, FALSE, (class textview *)view);
        PopToMark(currentError->offendingCodeMark, TRUE, (class textview *)view);
    }
    else
        message::DisplayString(view, 0, "There appears to be no errors here!");
}

struct finderInfo {
    class frame *myFrame, *otherFrame, *bestFrame;
    class buffer *myBuffer;
};

static boolean FrameFinder(class frame  *frame, struct finderInfo  *info)
        {

    struct rectangle bogus;

    if (info->myFrame == frame)
        return FALSE;
    if (info->otherFrame != NULL && info->bestFrame != NULL)
	return TRUE;
    (frame)->GetVisualBounds( &bogus);
    if (!rectangle_IsEmptyRect(&bogus)) {
        if ((frame)->GetBuffer() == info->myBuffer) {
            info->bestFrame = frame;
            return FALSE;
        }
        else {
            info->otherFrame = frame;
            return FALSE;
        }
    }
    return FALSE;
}

static int ViewEqual(class frame  *frame, class view  *view)
        {

#if 1
    return ((frame)->GetView() == view);
#else /* 1 */
    return (((class view *) frame)->GetIM() == (view)->GetIM())
#endif /* 1 */
}

static class frame *FindByView(class view  *view)
    {

    return frame::Enumerate((frame_effptr)ViewEqual, (long) view);
}

/* Find a window other that the one that contains this inset.  Create one if we have to. */
static class view *PutInAnotherWindow(class view  *view, class buffer  *buffer, int  forceWindow)
            {

    class frame *frame;
    struct finderInfo myInfo;

    myInfo.myFrame = FindByView(view);
    myInfo.otherFrame = NULL;
    myInfo.bestFrame = NULL;
    myInfo.myBuffer = buffer;
    frame::Enumerate(((frame_effptr)FrameFinder), (long) &myInfo);
    frame = (myInfo.bestFrame != NULL) ? myInfo.bestFrame : ((myInfo.otherFrame != NULL) ? myInfo.otherFrame : NULL);
    if (frame == NULL) {

        class im *newIM;

        if (!forceWindow)
            return NULL;
        if((newIM = im::Create(NULL)) == NULL) {
	    fprintf(stderr,"Could not create new window.\n");
	    return(NULL);
	}
        if((frame = frame::Create(buffer)) == NULL) {
	    fprintf(stderr,"Could not allocate enough memory.\n");
	    if(newIM) (newIM)->Destroy();
	    return(NULL);
	}
        (newIM)->SetView( frame);

/* This is here because the frame_Create call can't set the input focus
 * because the frame didn't have a parent when it called view_WantInputFocus.
 * This is bogus but hard to fix...
 */
        ((frame)->GetView())->WantInputFocus( (frame)->GetView());
    }
    else if ((frame)->GetBuffer() != buffer)
	(frame)->SetBuffer( buffer, TRUE);
    return (frame)->GetView();
}

/* A sort of kluge-o-matic routine.
 * Takes a mark, a flag and a window. The flag says whether to use the window,
 * or to not use the window if this mark's buffer is not already displayed.
 */
static class view *PopToMark(class mark  *mark, int  useWindowFlag, class textview  *textview)
            {

    class buffer *buffer = buffer::FindBufferByData((class dataobject *)mark->object);

    if (useWindowFlag) {
        if (Text(textview) != (class text *) mark->object) {

            class frame *frame = FindByView((class view *) textview);

            (frame)->SetBuffer( buffer, TRUE);
            textview = (class textview *) (frame)->GetView();
        }
    }
    else {
        textview = (class textview *) PutInAnotherWindow((class view *) textview, buffer, TRUE); /* Make sure it gets in a window */
	if(textview == NULL) return((class view*) NULL);

    }

    (textview)->SetDotPosition( (mark)->GetPos());
    (textview)->SetDotLength( (mark)->GetLength());
    (textview)->FrameDot( (mark)->GetPos());
    (textview)->WantUpdate( textview);
    return (class view *) textview;
}

boolean compile::InitializeClass()
    {

    struct ATKregistryEntry  *textviewType = ATK::LoadClass("textview");
    const char *command;

    proctable::DefineProc("compile-build", (proctable_fptr) compile_Build, textviewType, NULL, "Start a compilation.");
    proctable::DefineProc("compile-kill-build", (proctable_fptr) compile_KillBuild, textviewType, NULL, "Terminate a compilation.");
    proctable::DefineProc("compile-next-error", (proctable_fptr)compile_NextError, textviewType, NULL, "Step forward through compilation errors.");
    proctable::DefineProc("compile-previous-error", (proctable_fptr)compile_PreviousError, textviewType, NULL, "Step backward through compilation errors.");

    if ((command = environ::GetProfile("CompileCommand")) == NULL)
        defaultCommand = "build -k";
    else
        defaultCommand = strdup(command);
    compileCommand = (char *)defaultCommand; /* not freed if default */
    return TRUE;
}
