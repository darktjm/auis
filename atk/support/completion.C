/*LIBS: -lutil
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
 *        For full copyright information see:'andrew/config/COPYRITE'     *
\* ********************************************************************** */

#include <andrewos.h> /* sys/types.h */
ATK_IMPL("completion.H")

#include <filetype.H>
#include <keystate.H>
#include <keymap.H>
#include <im.H>
#include <environ.H>
#include <message.H>
#include <cursor.H>

#include <completion.H>

#include <sys/param.h>
#include <sys/stat.h>

#include <util.h>

static boolean useCurrentWorkingDirectory = FALSE;
static class cursor *waitCursor;
#undef min
#define min(x, y) (((x) < (y)) ? (x) : (y))


ATKdefineRegistry(completion, ATK, completion::InitializeClass);
static void FileHelp(const char  *partialPath, long  dummyData , message_workfptr helpTextFunction, long  helpTextRock);
static enum message_CompletionCode FileComplete(char  *pathname, long  directory, char  *buffer, int  bufferSize);
static enum keymap_Types FileHack(struct fileRock  *rock, long  key, ATK   **entry, long  *rockP);


 long completion::FindCommon(const char  *string1 , const char  *string2)
        {
	ATKinit;

    long i = 0;

#ifdef DOS_STYLE_PATHNAMES
    /* Case doesn't matter */
    while (tolower(*string1++) == tolower(*string2++) && (*(string1 - 1) != '\0'))
        i++;
#else
    while (*string1++ == *string2++ && (*(string1 - 1) != '\0'))
        i++;
#endif
    return i;
}

 void completion::CompletionWork(const char  *string, struct result  *data)
            {
	ATKinit;


    int partialCommon, nameLen;

    partialCommon = completion::FindCommon(string, data->partial);
    if (partialCommon == data->partialLen) { /* Possible to extend complete. */
        nameLen = strlen(string);
        if (partialCommon > data->bestLen) { /* This is a completion */
            data->bestLen = min(data->max, nameLen);
            strncpy(data->best, string, data->bestLen + 1); /* Safe since we left room below... */
            data->code = message_Complete;
        }
        else { /* Merge posibilities which have partial as a common substr. */

            int bestCommon = completion::FindCommon(string, data->best);

            if (bestCommon < data->bestLen) {
                data->bestLen = bestCommon;
                data->best[data->bestLen] = '\0';
                if (bestCommon == nameLen)
                    if (data->code != message_Invalid)
                        data->code = message_CompleteValid;
                    else
                        data->code = message_Complete;
                else
                    data->code = message_Valid;
            }
            else if ((bestCommon == data->bestLen) && ((bestCommon == nameLen) || (data->code == message_Complete)))
                    data->code = message_CompleteValid;
        }
    }
    else {
        if (partialCommon > data->bestLen) { /* Try to getter a longer initial substr. */
            data->bestLen = min(data->max, partialCommon);
            strncpy(data->best, string, data->bestLen);
            data->best[data->bestLen] = '\0'; /* Safe since we left room below... */
        }
        if (partialCommon >= data->bestLen)
            data->code = (partialCommon == strlen(string)) ? message_Complete : message_Invalid;
    }
}

 static void FileHelp(const char  *partialPath, long  dummyData /* Just along for the ride. */, message_workfptr helpTextFunction, long  helpTextRock)
                {

    int namelen;
    char *slash, dirbuf[MAXPATHLEN], namebuf[MAXNAMLEN];
    DIR *thisDir;
    DIRENT_TYPE *dirEntry;

#ifdef AFS_ENV
    boolean inVICE = FALSE;
#endif /* AFS_ENV */

    struct stat statBuf;

    filetype::CanonicalizeFilename(dirbuf, partialPath, sizeof(dirbuf));
    slash = strrchr(dirbuf, '/');
#ifdef DOS_STYLE_PATHNAMES
    if (slash == NULL)
	slash = strrchr(dirbuf, '\\');
#endif
    if (slash == NULL) {
        im::GetDirectory(dirbuf);
        strcat(dirbuf, "/");
        *namebuf = '\0';
    }
    else {
        strcpy(namebuf, slash + 1); /* Skip '/'. */
        slash[1] = '\0';
    }
    namelen = strlen(namebuf);

    im::SetProcessCursor(waitCursor);

    if ((thisDir = opendir(dirbuf)) == NULL) { /* Should try to back up to the first completed directory. */
        (*helpTextFunction)(helpTextRock, message_HelpGenericItem, "Couldn't access directory \"", NULL);
        (*helpTextFunction)(helpTextRock, message_HelpGenericItem, dirbuf, NULL);
        (*helpTextFunction)(helpTextRock, message_HelpGenericItem, "\"\n", NULL);
        im::SetProcessCursor(NULL);
        return;
    }


#ifdef AFS_ENV /* Enable the wonderous VICE hack... */
#if 0
#define VICEMAGICGID 32767

    if ((stat(dirbuf, &statBuf) >= 0) && (statBuf.st_gid == VICEMAGICGID))
        inVICE = TRUE;
#else /* 0 */
    inVICE = IsOnVice(thisDir->dd_fd);
#endif /* 0 */
#endif /* AFS_ENV  */

    while ((dirEntry = readdir(thisDir)) != NULL) {
        if (completion::FindCommon(namebuf, dirEntry->d_name) == namelen) {

            boolean isDirectory = FALSE;
            char fullName[MAXPATHLEN];

#ifdef AFS_ENV
            if (inVICE) {
                    if ((dirEntry->d_ino % 2) == 1)
                        isDirectory = TRUE;
            }
            else
#endif /* AFS_ENV */
            {
                strcpy(fullName, dirbuf); /* dir is guaranteed to end in a /. */
                strcat(fullName, dirEntry->d_name);
                stat(fullName, &statBuf);
                if ((statBuf.st_mode & S_IFMT) == S_IFDIR)
                    isDirectory = TRUE;
            }
            if (*partialPath == '\0') {
                strcpy(fullName, dirbuf); /* dir is guaranteed to end in a /. */
                strcat(fullName, dirEntry->d_name);
		strcat(fullName, "/");
                (*helpTextFunction)(helpTextRock, message_HelpListItem, fullName, NULL);
            }
            else {

                char *item;

                if (isDirectory) {
                    strcpy(fullName, dirEntry->d_name);
		    strcat(fullName,"/");
                    item = fullName;
                }
                else
                    item = dirEntry->d_name;
                (*helpTextFunction)(helpTextRock, message_HelpListItem, item, NULL);
            }
        }
    }
    closedir(thisDir);

    im::SetProcessCursor(NULL);
}

 void completion::FileHelp(const char  *partialPath, long  dummyData /* Just along for the ride. */, message_workfptr helpTextFunction, long  helpTextRock)
                    {
	ATKinit;

    ::FileHelp(partialPath, dummyData, helpTextFunction, helpTextRock);
}

static enum message_CompletionCode FileComplete(char  *pathname, long  directory, char  *buffer, int  bufferSize)
                {

    int len;
    char *slash, *dir, pathbuf[MAXPATHLEN];
    struct result result;
    char textBuffer[256];
    DIR *thisDir;
    DIRENT_TYPE *dirEntry;
    struct stat statBuf;
    boolean isDirectory;

#ifdef AFS_ENV
    boolean inVICE = FALSE;
#endif /* AFS_ENV */

    im::SetProcessCursor(waitCursor);

    filetype::CanonicalizeFilename(buffer, pathname, bufferSize);
    if (directory && (buffer[len = (strlen(buffer) - 1)] == '/'))
        buffer[len] = '\0';
#ifdef DOS_STYLE_PATHNAMES
    else
	if (directory && (buffer[len = (strlen(buffer) - 1)]=='\\'))
	    buffer[len] = '\0';
#endif

    slash = strrchr(buffer, '/');
#ifdef DOS_STYLE_PATHNAMES
    if (slash == NULL)
	slash = strrchr(buffer, '\\');
#endif

    if (slash == NULL) {
        dir = pathbuf;
        im::GetDirectory(dir);
        strcat(dir, "/");
        result.partial = buffer;
    }
    else {
        if (slash[1] == '\0') { /* Special case to handle expanding directory only paths. */

            int returnValue;

            if ((stat(buffer, &statBuf) >= 0) && ((statBuf.st_mode & S_IFMT) == S_IFDIR))
                returnValue = (int)message_CompleteValid;
            else
                returnValue = (int) message_Invalid;
            im::SetProcessCursor(NULL);
            return((enum message_CompletionCode)returnValue);
        }
        strcpy(pathbuf, slash + 1); /* Skip '/'. */
        slash[1] = '\0';
        dir = buffer;
        result.partial = pathbuf;
    }

    if ((thisDir = opendir(dir)) == NULL) { /* Should try to back up to the first completed directory. */
        *buffer = '\0';
        im::SetProcessCursor(NULL);
        return message_Invalid;
    }

#ifdef AFS_ENV /* Enable the wonderous VICE hack... */
#if 0
#define VICEMAGICGID 32767

    if ((stat(dir, &statBuf) >= 0) && (statBuf.st_gid == VICEMAGICGID))
        inVICE = TRUE;
#else /* 0 */
    inVICE = IsOnVice(thisDir->dd_fd);
#endif /* 0 */
#endif /* AFS_ENV  */

    *textBuffer = '\0';
    result.partialLen = strlen(result.partial);
    result.bestLen = 0;
    result.code = message_Invalid;
    result.best = textBuffer;
    result.max = sizeof(textBuffer) - 1; /* Leave extra room for the NUL. */
    result.best[result.max] = '\0';

    while ((dirEntry = readdir(thisDir)) != NULL) {
        if (directory) { /* Total hack to get directory completion to work efficiently. */
            if (completion::FindCommon(dirEntry->d_name, result.partial) == 0)
                continue;
#ifdef AFS_ENV
            if (inVICE && (dirEntry->d_ino % 2) == 0)
#endif /* AFS_ENV */
            {

                char fullName[MAXPATHLEN];


                strcpy(fullName, dir);
                strcat(fullName, dirEntry->d_name);
                if ((stat(fullName, &statBuf) < 0) || !((statBuf.st_mode & S_IFMT) == S_IFDIR))
                    continue;
            }
        }
        completion::CompletionWork(dirEntry->d_name, &result);
    }

    closedir(thisDir);

/* Really ought to check for buffer overflow here. */
    if(dir != buffer)
        strncpy(buffer, dir, bufferSize);
    strcat(buffer, result.best);
    isDirectory = (stat(buffer, &statBuf) >= 0) && ((statBuf.st_mode & S_IFMT) == S_IFDIR);
    if ((result.code == message_Complete) && (buffer[(len = strlen(buffer)) - 1] != '/') && isDirectory) {
        buffer[len] = '/';
        buffer[len + 1] = '\0';
        im::SetProcessCursor(NULL);
        result.code = message_CompleteValid;
    }
    else if ((result.code == message_CompleteValid) && isDirectory &&
             !directory)
        result.code = message_Valid;
    im::SetProcessCursor(NULL);
    return result.code;
}

 enum message_CompletionCode completion::FileComplete(char  *pathname, boolean  directory, char  *buffer, int  bufferSize)
                    {
	ATKinit;

    return ::FileComplete(pathname, (long) directory, buffer, bufferSize);
}

struct fileRock {
    class view *view;
    long messageLen;
    class keystate *keystate;
};

static enum keymap_Types CheckForColon(struct fileRock  *rock, long  key, ATK   **entry, long  *rockP)
{
    if (key == ':')
        message::DeleteCharacters(rock->view, 0, rock->messageLen);
    (rock->keystate)->SetOverride( NULL, 0);
    return keymap_Empty;
}

static enum keymap_Types FileHack(struct fileRock  *rock, long  key, ATK   **entry, long  *rockP)
{
    struct fileRock tmpRock;

    if (key == '/' || key == '~' || key == '\\') {
        message::DeleteCharacters(rock->view, 0, rock->messageLen);
	(rock->keystate)->SetOverride( NULL, 0);	
    }
#if SY_OS2
    else {
	if (isalpha(key)) {
	    /* check to see if it's a valid drive letter */
	    ULONG drivenum, logdrivemap;
	  
	    DosQueryCurrentDisk(&drivenum, &logdrivemap);
	    if (logdrivemap & (0x01 << (key-65))) {
	    /* Valid drive letter */
		(rock->keystate)->SetOverride( (keystate_fptr)CheckForColon, (long) rock);
	    }
	}
	else
	    (rock->keystate)->SetOverride( NULL, 0);
    }
#else
    (rock->keystate)->SetOverride( NULL, 0);
#endif
    return keymap_Empty;
}

 int completion::GetFilename(class view  *view, const char  *prompt, const char  *startPath, char  *buffer, long  bufsiz, boolean  directoryP, boolean  mustMatch)
                                {
	ATKinit;


    struct fileRock fileRock;
    const char *initialString;
    int code;
    int len;

    if (view == NULL) {
        view = (class view *) im::GetLastUsed();
        if (view == NULL)
            return -1;
    }

    fileRock.view = view;
    fileRock.keystate = new keystate;
    (fileRock.keystate)->SetOverride( (keystate_fptr)FileHack, (long) &fileRock);

    if (startPath == NULL)
        startPath = "";

/* This code makes UseCurrentWorkingDirectory behave reasonably for Read File.
 * Basically, it says if the user has chosen to have all paths default to the
 * current working directory and the initial string is a directory not a file do
 *  not use a default. Otherwise it is a file, in which case we want it to
 * default to that file, or UseCurrentWorkingDirectory is FALSE in which case we
 *  want to behave as we logically should have in the first place.
 * UseCurrentWorkingDirectory is a pain in the neck..
 */
    if (!useCurrentWorkingDirectory || ((len = strlen(startPath)) != 0 && startPath[len - 1] != '/'))
        initialString = startPath;
    else
        initialString = "";

    fileRock.messageLen = strlen(initialString);

    if ((code = message::AskForStringCompleted(view, 40, prompt,
					       initialString, buffer, bufsiz, fileRock.keystate, (message_completionfptr) ::FileComplete,
					       (message_helpfptr) ::FileHelp, (long) directoryP, mustMatch ? message_MustMatch : 0)) != -1)
        filetype::CanonicalizeFilename(buffer, buffer, bufsiz);
    delete fileRock.keystate;
    return code;
}

boolean completion::InitializeClass()
    {

    waitCursor = cursor::Create(NULL);
    (waitCursor)->SetStandard( Cursor_Wait);

/* For Sherri Menees and David Nichols... */
    useCurrentWorkingDirectory = environ::GetProfileSwitch("UseCurrentWorkingDirectory", FALSE);

    return TRUE;
}
