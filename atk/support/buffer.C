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

#include <andrewos.h> /* sys/types.h sys/file.h */

#ifndef NORCSID
#define NORCSID
static UNUSED const char rcsid[]="$Header: /afs/cs.cmu.edu/project/atk-src-C++/atk/support/RCS/buffer.C,v 3.9 1995/11/08 21:41:07 robr Stab74 $";
#endif

/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/config/COPYRITE'     *
\* ********************************************************************** */

 

/*  Fixed problem in buffer_WriteToFile 3/2/90 cch@mtgzx.att.com.

  The problem was that it is creating a temporary filename by appending 
  the string .NEW (and even .<number> if necessary).  If the file is on a 
  Unix V file system, filenames are truncated to 14 characters, and there is 
  no guarantee that the temp filename will be unique.  This sends the routine 
  into an infinite loop. This fix uncovers a bug elsewhere, in which the 
  file can be deleted if the checkpoint filename (when truncated to 14 
  characters ) is the same as the file being saved. In this case the file is 
  deleted as soon as it is stored, with no notice or warning.

  To use short temporary filenames, define USESHORTFILENAMES in your 
  site.h file.
*/

ATK_IMPL("buffer.H")
#include <andyenv.h>
#include <errno.h>

#include <sys/stat.h>


#include <observable.H>
#include <dataobject.H>
#include <view.H>
#include <im.H>
#include <path.H>
#include <filetype.H>
#include <attribs.h>
#include <environ.H>
#include <message.H>
#include <bufferlist.H>

#include <buffer.H>

#ifndef MAXPATHLEN 
#include <sys/param.h>
#endif

#include <util.h>

static class bufferlist *allBuffers;
static char *backupExtension = NULL;
static char *checkpointExtension = ".CKP";
static char *checkpointDirectory = NULL;
static boolean overwriteFiles = TRUE;
static boolean checkpointGawdyNames = FALSE;


ATKdefineRegistry(buffer, observable, buffer::InitializeClass);

void buffer::SetCheckpointFilename()
{
    /* For this routine to work properly buffer names must be unique. */
    /* If they are not then the user might lose some information */

    if (this->ckpFilename != NULL)  {
	free(this->ckpFilename);
    }

    if (checkpointDirectory != NULL) {
#ifdef USESHORTFILENAMES
      if (FALSE)
#else
      if (checkpointGawdyNames)
#endif /* USESHORTFILENAMES */
      {
          int i;
          char tname[MAXPATHLEN];
	  sprintf (tname, "#%d%s#", getuid(), (this->filename ? this->filename : this->bufferName) );
          for (i = 0; i < MAXPATHLEN && tname[i] != '\0'; i++)
              if (tname[i] == '/') tname [i] = '@';
          this->ckpFilename = (char *)malloc(strlen(checkpointDirectory) + strlen (tname) + strlen(checkpointExtension) + 2);
          sprintf(this->ckpFilename, "%s/%s%s", checkpointDirectory, tname, checkpointExtension);
      } else {
	char *basename;

	basename = strrchr(this->bufferName, '/');
	if (basename == NULL)  {
	    basename = this->bufferName;
	}
	else  {
	    basename++;
	}
	this->ckpFilename = (char *)malloc(strlen(checkpointDirectory) + strlen(basename) + strlen(checkpointExtension) + 2);
	sprintf(this->ckpFilename, "%s/%s%s", checkpointDirectory, basename, checkpointExtension);
      }
    }
    else {
	char *filename = (this->filename != NULL) ? this->filename : this->bufferName;

	this->ckpFilename = (char *)malloc(strlen(filename) + strlen(checkpointExtension) + 1);
	strcpy(this->ckpFilename, filename);
	strcat(this->ckpFilename, checkpointExtension);
    }
}


buffer::buffer()
        {
	ATKinit;

    this->bufferData = NULL;
    this->bufferName = NULL;
    this->filename = NULL;
    this->ckpFilename = NULL;
    this->viewList = (struct bufferContents *) malloc(sizeof(struct bufferContents) * 3);
    this->viewsAllocated = 3;
    this->viewsUsed = 0;
    this->ckpVersion = this->writeVersion = 0;
    this->ckpClock = 0;
    this->ckpLatency = 0;
    this->lastTouchDate = 0L;
    this->scratch = FALSE;
    this->readOnly = FALSE;
    this->madeBackup = FALSE;
    this->isModified = FALSE;
    this->askedAboutSymlink = FALSE;
    this->clobberSymlink = environ::GetProfileSwitch("ClobberSymlinks", TRUE);
    this->isRawFile = FALSE;
    this->viewname = NULL;
    
    THROWONFAILURE( TRUE);
}

buffer::~buffer()
        {
	ATKinit;

    int counter;

    for (counter = 0; counter < this->viewsUsed; counter++) {
        if (this->viewList[counter].bufferApplicationView != NULL)
            (this->viewList[counter].bufferView)->DeleteApplicationLayer( this->viewList[counter].bufferApplicationView);
        (this->viewList[counter].bufferView)->Destroy();
    }
    free(this->viewList);
    if (this->bufferData) {
	(this->bufferData)->RemoveObserver( this);
	if(this->destroyData) (this->bufferData)->Destroy();
    }
    if (this->filename != NULL)
        free(this->filename);
    if (this->ckpFilename != NULL)
        free(this->ckpFilename);
    if (this->bufferName != NULL)
        free(this->bufferName);
    if(this->viewname != NULL)
	free(this->viewname);
}

/* Changed bufferlist */
class buffer *buffer::Create(const char  *bufferName , const char  *filename , const char  *objectName, class dataobject  *data)
            {
	ATKinit;

    return (allBuffers)->CreateBuffer( bufferName, filename, objectName, data);
}

class view *buffer::GetView(class view  **inputFocus , class view  **targetView, char  *viewName)
            {
    int counter;

    for (counter = 0; (counter < this->viewsUsed) && this->viewList[counter].used && (viewName == NULL ||
      strcmp(viewName, (this->viewList[counter].bufferView)->GetTypeName()) != 0); counter++)
        ;

    if (viewName == NULL) { /* Must decide what type view is desired */
	if (this->viewname != NULL) viewName = this->viewname;
	else viewName = this->bufferData->ViewName();
    }

    /* Search for an unused view which matches the view name we are looking for */
    for (counter = 0; (counter < this->viewsUsed) && (this->viewList[counter].used || strcmp(viewName, (this->viewList[counter].bufferView)->GetTypeName())); counter++);
  
    if (counter >= this->viewsUsed) { /* Didn't find an unused view of proper type, must create one */
        if (this->viewsAllocated <= this->viewsUsed) {
            this->viewList = (struct bufferContents *) realloc(this->viewList, sizeof(struct bufferContents) * (this->viewsAllocated + 3));
            this->viewsAllocated += 3;
        }
        if ((this->viewList[counter].bufferView = (class view *) ATK::NewObject(viewName)) == NULL)
            return NULL;
        (this->viewList[counter].bufferView)->SetDataObject( this->bufferData);
        this->viewList[counter].bufferInputFocus = NULL;
        ++this->viewsUsed;
    }

    this->viewList[counter].bufferApplicationView = (this->viewList[counter].bufferView)->GetApplicationLayer();
    this->viewList[counter].used = TRUE;
    if (inputFocus != NULL)
        *inputFocus = this->viewList[counter].bufferInputFocus;
    if (targetView != NULL)
        *targetView = this->viewList[counter].bufferView;

    return this->viewList[counter].bufferApplicationView;
}

void buffer::RemoveView(class view  *unusedView)
        {
    int counter;

    for (counter = 0; counter < this->viewsUsed; counter++)
        if ((this->viewList[counter].bufferApplicationView == unusedView) && (this->viewList[counter].used == TRUE)) {
            (unusedView)->UnlinkTree(); /* Remove it from tree. */
            (this->viewList[counter].bufferView)->DeleteApplicationLayer( unusedView); /* Deallocate application layer to save space. */
            this->viewList[counter].bufferApplicationView = NULL; /* NULL out field for safety. */
            this->viewList[counter].used = FALSE;
            break;
        }
}

boolean buffer::Visible()
    {
    int counter;

    for (counter = 0; counter < this->viewsUsed; counter++)
        if (this->viewList[counter].used)
            return TRUE;
    return FALSE;
}

/* Changed bufferlist */
class buffer *buffer::Enumerate(bufferlist_efptr mapFunction, long  functionData)
            {
	ATKinit;

    return (allBuffers)->Enumerate( mapFunction, functionData);
}

void buffer::SetData(class dataobject  *bufferData)
        {
    this->bufferData = bufferData;
    (bufferData)->AddObserver( this);
    this->isModified = FALSE;

    this->ckpVersion = this->writeVersion = (bufferData)->GetModified();
    this->destroyData = FALSE;
    if(this->viewname != NULL){
	free(this->viewname);
	this->viewname = NULL;
    }
    (this)->NotifyObservers( 0);
}

void buffer::SetName(const char  *bufferName)
        {
    if (this->bufferName != NULL)
        free(this->bufferName);
    this->bufferName = (char *)malloc(strlen(bufferName) + 1);
    strcpy(this->bufferName, bufferName);
    (this)->NotifyObservers( 0);
}

void buffer::SetFilename(const char  *filename)
        {

    char realName[MAXPATHLEN];
    int len;

    if (this->filename)
        free(this->filename);
    if(filename != NULL && *filename != '\0') {
	filetype::CanonicalizeFilename(realName, filename, sizeof(realName) - 1);
    }
    else {
	realName[0] = '\0';
    }
    filename = realName;
    len = strlen(filename);
    this->filename = (char *)malloc(len + 1);
    strcpy(this->filename, filename);
    (this)->SetCheckpointFilename();
    this->lastTouchDate = (this)->GetFileDate();
    (this)->NotifyObservers( 0);/* Tuck it into slot. */
}

void buffer::SetWriteVersion(long  version)
        {
    long dobjVersion = ((this)->GetData())->GetModified();

    this->writeVersion = version;

    (this)->SetIsModified( version < dobjVersion);

    (this)->NotifyObservers( 0);/* Tuck it into slot. */
}

void buffer::SetCkpVersion(long  version)
        {

    this->ckpVersion = version;
    (this)->NotifyObservers( 0);/* Tuck it into slot. */
}

void buffer::SetCkpClock(long  clock)
        {

    this->ckpClock = clock;
}

void buffer::SetCkpLatency(long  latency)
        {
    this->ckpLatency = latency;
}

void buffer::SetScratch(boolean  scratch)
        {

    this->scratch = scratch;
    (this)->NotifyObservers( 0);/* Tuck it into slot. */
}

class view *buffer::EnumerateViews(buffer_evfptr  mapFunction, long  functionData)
            {

    int counter;
    class view *value;

    value = NULL;

    for (counter = 0; counter < this->viewsUsed; counter++) {
        if (this->viewList[counter].used) {
	    if ((value = mapFunction(this->viewList[counter].bufferApplicationView,
					this->viewList[counter].bufferView,
					this->viewList[counter].bufferInputFocus,
					functionData)) != NULL) {
		return value;
	    }
	}
    }
		
    return NULL;
}

/* Changed bufferlist */
class buffer *buffer::FindBufferByFile(const char  *filename)
        {
	ATKinit;

    return (allBuffers)->FindBufferByFile( filename);
}

/* Changed Bufferlist */
class buffer *buffer::FindBufferByData(class dataobject  *bufferData)
        {
	ATKinit;

    return (allBuffers)->FindBufferByData( bufferData);
}

/* Changed Bufferlist */

class buffer *buffer::FindBufferByName(const char  *bufferName)
        {
	ATKinit;

    return (allBuffers)->FindBufferByName( bufferName);
}

/* This routine is supposed to be used for re-reading a file into a buffer.
 * Unfortunately, at present it is somewhat special cased for use within ez and
 * does not handle reseting all buffer state "intelligently."
 * Refuses to read directory; the caller (framecmds) tries GetBufferOnFile
 * if this call fails.
 */

int buffer::ReadFile(const char  *filename)
        {
    long objectID;
    int returnCode = 0;
    char realName[MAXPATHLEN];
    char *objectName;
    FILE *thisFile;
    struct stat stbuf;
    struct attributes *attributes;
    struct attributes *tempAttribute;

    filetype::CanonicalizeFilename(realName, filename, sizeof(realName) - 1);
    filename = realName;

    if (stat(filename, &stbuf) < 0 || (stbuf.st_mode & S_IFMT) == S_IFDIR)
        return -1;

    if ((thisFile = fopen(filename, "rb")) == NULL)
            return -1;

    this->lastTouchDate = (long) stbuf.st_mtime;

    objectName = filetype::Lookup(thisFile, filename, &objectID, &attributes);

/* This next thing is a hack. Really need a flags parameter so we can keep
 * the thing readonly if need be.
 */
    this->readOnly = FALSE;
    for (tempAttribute = attributes; tempAttribute != NULL; tempAttribute = tempAttribute->next)
        if (strcmp(tempAttribute->key, "readonly") == 0)
            this->readOnly = tempAttribute->value.integer;

    if (objectName == NULL)
        objectName = "text"; /* The default... */

    if (strcmp(((this)->GetData())->GetTypeName(), objectName) == 0) {

	long version;

        ((this)->GetData())->SetAttributes( attributes);
        ((this)->GetData())->Read( thisFile, objectID);
        ((this)->GetData())->NotifyObservers( 0);
        (this)->SetFilename( filename);

	version = ((this)->GetData())->GetModified();
        (this)->SetCkpClock( 0);
        (this)->SetCkpVersion( version);
	(this)->SetWriteVersion( version);
    }
    else
        returnCode = -1;

    fclose(thisFile);
    (this)->NotifyObservers( 0);/* Tuck it into slot. */
    return returnCode;
}

/* Changed bufferlist */

class buffer *buffer::GetBufferOnFile(const char  *filename, long  flags)
{
	ATKinit;

    return (allBuffers)->GetBufferOnFile( filename, flags);
}

/* Changed bufferlist */

void buffer::GuessBufferName (char  *filename , char  *bufferName, int  nameSize)
            {
	ATKinit;

    (allBuffers)->GuessBufferName( filename, bufferName, nameSize);
}

void buffer::GetUniqueBufferName (char  *proposedBufferName , char  *bufferName, int  nameSize)
            {
	ATKinit;

    (allBuffers)->GuessBufferName( proposedBufferName, bufferName, nameSize);
}

static int ResolveLink(char  *linkname , char  *bufname)
{
#ifdef HAVE_SYMLINKS
    struct stat sbuf;
    char name[MAXPATHLEN];
    int len;

    strcpy(name, linkname);
    do {
	char *p=strrchr(name, '/');
	if ((len = readlink(name, bufname, MAXPATHLEN)) <= 0) {
	    return -1;
	}
	bufname[len] = '\0';
	if (p==NULL || *bufname=='/') p=name;
	else p=p+1;
	strcpy(p, bufname);
	if (lstat(name, &sbuf) < 0) {
	    return -1;
	}
    } while ((sbuf.st_mode & S_IFMT) == S_IFLNK);
    if (*bufname != '/') {	/* It's a relative link.  Make it absolute. */
	char *dirname = strrchr(linkname, '/');

	if (dirname) {
	    strncpy(name, linkname, dirname - linkname + 1);
	    strcpy(&(name[0]) + (dirname - linkname + 1), bufname);
	    filetype::CanonicalizeFilename(bufname, name, MAXPATHLEN);
	}
    }
    return 0;
#else
    return -1;	/* No symlinks */
#endif /* HAVE_SYMLINKS */
}

int buffer::WriteToFile(char  *filename, long  flags)
            {

    char realName[MAXPATHLEN], linkdest[MAXPATHLEN];
    char tempFilename[MAXPATHLEN];
    char shortName[65], shortLinkdest[129];
    char *originalFilename = NULL;
    int closeCode;
    int errorCode;
    int originalMode;
    int fd;
    FILE *outFile;
    struct stat statBuf;
    boolean makeBackup = !(this)->GetMadeBackup();
    boolean differentFile = FALSE;
    boolean fileExists = TRUE;
    boolean alreadyAsked = FALSE;
    boolean isSymlink = FALSE;

    errorCode = 0;
    *linkdest = '\0';
    if (filename == NULL) {
	if ((filename = this->filename) == NULL)
	    return -1;
    }
    else {
	filetype::CanonicalizeFilename(realName, filename, sizeof(realName) - 1);
	filename = realName;
	if ((this->filename == NULL) || (strcmp(filename, this->filename) != 0)) { /* If writing to a different file than normal. */
	    differentFile = TRUE;
	    makeBackup = TRUE;
	}
    }

    if (access(filename, W_OK) < 0) {
	if (errno == EACCES) {
	    return -1;
	} else if (errno == ENOENT) {
	    fileExists = makeBackup = FALSE;
	}
    }

#ifdef HAVE_SYMLINKS
    if (lstat(filename, &statBuf) >= 0) {
	if ((statBuf.st_mode & S_IFMT) == S_IFLNK) {
	    isSymlink = TRUE;
	    if (environ::GetProfileSwitch("AskAboutSymlinks", FALSE)
		&& !(this->askedAboutSymlink)) {
		char prompt[100 + MAXPATHLEN];
		long choice;

		path::TruncatePath(filename, shortName,
				  (sizeof (shortName)) - 1, TRUE);
		if (ResolveLink(filename, linkdest)) {
		    static char *choices[] = {
			"Replace link with file contents",
			"Cancel",
			NULL
		    };

		    sprintf(prompt, "%s is an unresolvable symbolic link",
			    shortName);
		    im::SetProcessCursor(NULL);
		    errno = 0;
		    if (message::MultipleChoiceQuestion(NULL, 80, prompt, 0,
						       &choice, choices,
						       "rc") < 0)
			return -1;
		    if (choice == 1)
			return -1;
		    this->clobberSymlink = TRUE;
		    this->askedAboutSymlink = TRUE;
		    alreadyAsked = TRUE;
		}
		else {
		    static char *choices[] = {
			"Replace link with file contents",
			"Follow link, replacing pointed-to file",
			"Cancel",
			NULL
		    };

		    path::TruncatePath(linkdest, shortLinkdest,
				      (sizeof (shortLinkdest)) - 1, TRUE);
		    sprintf(prompt, "%s is a symbolic link pointing to %s",
			    shortName, shortLinkdest);
		    im::SetProcessCursor(NULL);
		    errno = 0;
		    if (message::MultipleChoiceQuestion(NULL, 80,
						       prompt,
						       (this->clobberSymlink ?
							0 : 1),
						       &choice,
						       choices,
						       "roc") < 0) {
			return -1;
		    }
		    if (choice == 2)
			return -1;
		    this->clobberSymlink = (choice == 0);
		    this->askedAboutSymlink = TRUE;
		    alreadyAsked = TRUE;
		}
	    }
	    if (!(this->clobberSymlink)) {
		if (*linkdest == '\0') {
		    if (ResolveLink(filename, linkdest)) {
			return -1;
		    }
		}
		if (stat(linkdest, &statBuf) < 0) {
		    return -1;
		}
	    }
	    originalMode = statBuf.st_mode & (~S_IFMT);
	} else {
	    originalMode = statBuf.st_mode & (~S_IFMT);
	}
    }
    else
#endif /*HAVE_SYMLINKS */
	originalMode = 0666; /* Default open mode. */

    if (!overwriteFiles && differentFile && makeBackup && !alreadyAsked) {
        char prompt[MAXPATHLEN + sizeof("``'' already exists. Overwrite? ")];
        char answer[5];

        sprintf(prompt, "``%s'' already exists. Overwrite? ", filename);

	im::SetProcessCursor(NULL);
	errno = 0;
        if (message::AskForString(NULL, 0, prompt, NULL, answer, sizeof(answer)) < 0 || (answer[0] != 'y' && answer[0] != 'Y'))
            return -1;
    }

    if ((flags & buffer_MakeBackup) && backupExtension && makeBackup) {
	strcpy(tempFilename, filename);
	strcat(tempFilename, backupExtension);
	if (isSymlink && !(this->clobberSymlink)) {
	    FILE *infp, *outfp;
	    int c;

	    if (!(infp = fopen(filename, "rb"))
		|| !(outfp = fopen(tempFilename, "wb"))) {
		return -1;
	    }
	    while ((c = fgetc(infp)) != EOF)
		fputc(c, outfp);
	    fclose(infp);
	    fclose(outfp);
	} else {
	    if ((rename(filename, tempFilename) < 0) && errno != ENOENT)
		return -1;
	}
	(this)->SetMadeBackup( TRUE);
    }
    else if ((flags & buffer_ReliableWrite) && fileExists) {
	char *endString;
	int counter = 1;

#ifdef USESHORTFILENAMES
	char *basename;
#endif /* USESHORTFILENAMES */

	if (isSymlink && !(this->clobberSymlink)) {
	    if (*linkdest == '\0') {
		if (ResolveLink(filename, linkdest)) {
		    return -1;
		}
	    }
	    originalFilename = linkdest;
	    filename = linkdest;
	}
#ifndef USESHORTFILENAMES
	strcpy(tempFilename, filename);
#ifdef USEREALLYSHORTFILENAMES
	/*OS/2 can have short or long names, depending on the file system.  We'll use a short name for both cases by stripping off the extension and only appending '$'.*/
	char *firstDot = strchr(tempFilename,'.');
	if(!firstDot)
	    firstDot = tempFilename + strlen(tempFilename)-1;
	*(firstDot++) = '$';
	*firstDot = (char)0;
#else
	strcat(tempFilename, ".NEW");
#endif
	endString = tempFilename + strlen(tempFilename);
	while (access(tempFilename, F_OK) >= 0) /* While the file exists. */
	    sprintf(endString, ".%d", counter++);
#else /* USESHORTFILENAMES */
	strcpy(tempFilename, filename);
	basename = strrchr(tempFilename, '/');
	if (basename == NULL) basename = tempFilename;
	else basename++;
	if (strlen(basename) > 8) basename[8] = '\0';
	strcat(tempFilename, ".NEW");
	endString = tempFilename + strlen(tempFilename);
	while (access(tempFilename, F_OK) >= 0 && counter < 10) /* While the file exists. */
	    sprintf(endString, ".%d", counter++);
	if (counter == 10) return -1;
#endif /* USESHORTFILENAMES */
	if (!originalFilename)
	    originalFilename = filename;
	filename = tempFilename;
    }
    else {
	if (isSymlink && !(this->clobberSymlink)) {
	    if (*linkdest == '\0') {
		if (ResolveLink(filename, linkdest)) {
		    return -1;
		}
	    }
	    filename = linkdest;
	}
    }

    if ((fd = open(filename, O_WRONLY | O_TRUNC
#if SY_OS2
		   | O_BINARY
#endif
		   | O_CREAT, originalMode)) < 0
	 || (outFile = fdopen(fd, "wb")) == NULL)
	return -1;
    (this->bufferData)->Write( outFile, im::GetWriteID(), 0);
    fflush(outFile);

/* This next test is somewhat bogus. In theory, if an error occured while
 * writing the file, this will catch it. In practice, dataobjects are not
 * required to use stdio to write objects and stdio also seems to lose error
 * indications on occasion. In any event, we assume that if the error has
 * occured, there is no point in using vclose to close the file and we
  * preserve the original file. The code here actually simulates the error
  * return (with an unknown error 0 since stdio doesn't give any indication of
  * what actually went wrong.
  */
    if (ferror(outFile)) {
	fclose(outFile);
	errorCode = 0;
	closeCode = -1;
    }
    else {
	/* Now for GNU-Emacs compatibility, we chmod the file. */
	/* This is so that we preserve the modes of the original */
	/* file, un-modified by umask. */
	if (fileExists) chmod(filename, originalMode);
#ifdef AFS_ENV
	if (flags & buffer_ReliableWrite) { /* Go for the expensive but safe operation. */
	    if ((closeCode = vclose(fileno(outFile))) < 0) /* stdio can trash errno. */
		errorCode = errno; /* Protect it from the fclose below. */
	    else
		if (originalFilename != NULL)
		    if ((closeCode = rename(filename, originalFilename)) < 0)
			errorCode = errno;
	}
	else /* Fast and loose. */
	    if ((closeCode = close(fileno(outFile))) < 0) /* stdio can trash errno. */
		errorCode = errno; /* Protect it from the fclose below. */
#else /* AFS_ENV */
	if ((closeCode = close(fileno(outFile))) < 0) /* stdio can trash errno. */
	    errorCode = errno; /* Protect it from the fclose below. */
	else
	    if (originalFilename != NULL) {
		if ((closeCode = rename(filename, originalFilename)) < 0)
		    errorCode = errno;
}
#endif /* AFS_ENV */
	fclose(outFile); /* Free stdio resources. */
#if 0
	/* This code resets the readonly flag based on the file
	 * we just wrote.  Often the file is a checkpoint of the
	 * buffer, so this may set the buffer to read/write
	 * by accident.  The fix:  comment out this code and let
	 * the caller reset the readonly flag.
	 */
	if (closeCode >= 0) { /* Reset readonly mode. */

	    struct attributes attributes;

	    attributes.next = NULL;
	    attributes.key = "readonly";
	    if (access(filename, W_OK) == -1 && errno == EACCES)
		attributes.value.integer = this->readOnly = TRUE;
	    else
		attributes.value.integer = this->readOnly = FALSE;
	    ((this)->GetData())->SetAttributes( &attributes);
	}
#endif
    }
    errno = errorCode;
/* Get a fresh stat() on the file after we've now written it. */
    if(differentFile == FALSE){
	/* don't modify if writing to another file */
	this->lastTouchDate = (this)->GetFileDate();
    }
    return closeCode;
}

long buffer::GetFileDate()
{
    struct stat stbuf;
    if (this->filename == NULL)
        return 0L;
    if (stat(this->filename, &stbuf) < 0)
        return 0L;
    return (long) stbuf.st_mtime;
}

boolean buffer::InitializeClass()
    {
    const char *s;

    if ((s = environ::GetProfile("BackupExtension")) != NULL) {
        backupExtension = (char *)malloc(strlen(s) + 1);
        strcpy(backupExtension, s);
    }

    if ((s = environ::GetProfile("CheckpointExtension")) != NULL) {
        checkpointExtension = (char *)malloc(strlen(s) + 1);
        strcpy(checkpointExtension, s);
    }

    if ((s = environ::GetProfile("CheckpointDirectory")) != NULL) {
        checkpointDirectory = (char *)malloc(strlen(s) + 1);
        strcpy(checkpointDirectory, s);
    }

    overwriteFiles = environ::GetProfileSwitch("OverwriteFiles", overwriteFiles);
    checkpointGawdyNames = environ::GetProfileSwitch("CheckpointGawdyNames", FALSE);

    allBuffers = new bufferlist;
    return TRUE;
}

/* Changed BufferList */

void buffer::SetDefaultObject(const char  *objectname)
{
	ATKinit;

    (allBuffers)->SetDefaultObject( objectname);
}

void buffer::SetDefaultViewname(char  *name)
{
    if(this->viewname != NULL)
	free(this->viewname);
    if(name == NULL) this->viewname = NULL;
    else {
	this->viewname = (char *)malloc(strlen(name) + 1);
	if(this->viewname == NULL) return;
	strcpy(this->viewname, name);
    }
}

void buffer::SetDestroyData(boolean  destroy)
        {
    this->destroyData = destroy;
}

void buffer::SetLastTouchDate(long  dateStamp)
        {
    this->lastTouchDate = dateStamp;;
}

void buffer::SetReadOnly(boolean  readOnly)
        {
    struct attributes attributes;

    attributes.next = NULL;
    attributes.key = "readonly";
    attributes.value.integer = readOnly;
    ((this)->GetData())->SetAttributes( &attributes);
    this->readOnly = readOnly;
    (this)->NotifyObservers( observable_OBJECTCHANGED);
}

class bufferlist *buffer::GetGlobalBufferList()
    {
	ATKinit;

    return allBuffers;
}

void buffer::ObservedChanged(class observable  *object, long  value)
{
   if (value == observable_OBJECTDESTROYED) {
	/* this makes little to no sense.
    observable_RemoveObserver(object, self); */
    }
    else if (! (this)->GetIsModified() && (this)->GetWriteVersion() < (this)->GetData()->GetModified()) {
	(this)->SetIsModified(TRUE);
    }
}


void buffer::SetIsModified(boolean  value)
{
    this->isModified = value;
    (this)->NotifyObservers( observable_OBJECTCHANGED);
}

void buffer::SetIsRawFile(boolean  value)
{
    this->isRawFile = value;
    (this)->NotifyObservers( observable_OBJECTCHANGED);
}
