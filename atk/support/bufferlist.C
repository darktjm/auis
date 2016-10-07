/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

#include <andrewos.h> /* sys/types.h sys/file.h */
ATK_IMPL("bufferlist.H")

#include <errno.h>
#include <sys/stat.h>

#include <observable.H>
#include <dataobject.H>
#include <view.H>
#include <im.H>
#include <filetype.H>
#include <attribs.h>
#include <environ.H>
#include <message.H>
#include <buffer.H>

#include <bufferlist.H>

#ifndef MAXPATHLEN 
#include <sys/param.h>
#endif
#define DEFAULTOBJECT "text"

struct listentry {
    class buffer *buffer;
    struct listentry *next;
};

static char defaultobjectname[64] = DEFAULTOBJECT;


ATKdefineRegistry(bufferlist, observable, NULL);

bufferlist::bufferlist()
        {
    this->head = NULL;

    THROWONFAILURE( TRUE);
}

bufferlist::~bufferlist()
        {
    struct listentry *traverse;
    struct listentry *next;

    for (traverse = this->head; traverse != NULL; traverse = next) {
	next = traverse->next;
	free(traverse);
    }
}

void bufferlist::ObservedChanged(class observable  *object, long  changeType)
{
    if (changeType == observable::OBJECTDESTROYED) {
	class buffer *buffer = (class buffer *) object;

	(buffer)->RemoveObserver( this);
	(this)->RemoveBuffer( buffer);
    }
}

void bufferlist::AddBuffer(class buffer  *buffer)
        {
    struct listentry *newElement;
    struct listentry *traverse;

    for (traverse = this->head; traverse != NULL && traverse->buffer != buffer; traverse = traverse->next) {
    }

    if (traverse == NULL) {
	newElement = (struct listentry *) malloc(sizeof(struct listentry));
	if (newElement != NULL) {
	    newElement->buffer = buffer;
	    newElement->next = this->head;
	    this->head = newElement;
	    (buffer)->AddObserver( this);
	    (this)->NotifyObservers( observable::OBJECTCHANGED);
	}
    }
}

void bufferlist::RemoveBuffer(class buffer  *buffer)
        {
    struct listentry **previous = &this->head;
    struct listentry *traverse;

    for (traverse = this->head; traverse != NULL && traverse->buffer != buffer; traverse = traverse->next) {
	previous = &traverse->next;
    }

    if (traverse != NULL) {    /* Bad error if this is false. */
        *previous = traverse->next;
	(this)->NotifyObservers( observable::OBJECTCHANGED);
	free(traverse);
    }
}

class buffer *bufferlist::CreateBuffer(const char  *bufferName , const char  *filename , const char  *objectName, class dataobject  *data)
            {
    char realName[MAXPATHLEN];
    class buffer *thisBuffer;

/* Probably ought to ensure that buffer names are unique. */
    thisBuffer = new buffer;

    (thisBuffer)->SetName( bufferName);

    if (filename != NULL) {
        filetype::CanonicalizeFilename(realName, filename, sizeof(realName) - 1);
	filename = realName;

	(thisBuffer)->SetFilename( filename);
    }

    (thisBuffer)->SetCheckpointFilename();

    if (data == NULL) {
        struct attributes *attributes = NULL;

        if (objectName == NULL)  {
            if (filename != NULL)
                objectName = filetype::Lookup(NULL, filename, NULL, &attributes);
            else
                objectName = filetype::Lookup(NULL, bufferName, NULL, &attributes);
	    if (objectName == NULL)
		objectName = defaultobjectname;
	}
	ATK::LoadClass(objectName);
	if(!ATK::IsTypeByName(objectName, "dataobject")) {
	    fprintf(stderr, "Warning: bad dataobject class name %s substituting %s.\n", objectName, defaultobjectname);
	    objectName = defaultobjectname;
	}
        if ((data = (class dataobject *) ATK::NewObject(objectName)) == NULL) {
            (thisBuffer)->Destroy();
            return NULL;
        }
        if (attributes != NULL) {
	    (data)->SetAttributes( attributes);
	}
	(thisBuffer)->SetData( data);
	(thisBuffer)->SetDestroyData( TRUE);
    }
    else {
	(thisBuffer)->SetData( data); 
    }

    (thisBuffer)->SetCkpVersion( (data)->GetModified());
    (thisBuffer)->SetWriteVersion( (data)->GetModified());
    (thisBuffer)->SetLastTouchDate( (thisBuffer)->GetFileDate());

    (this)->AddBuffer( thisBuffer);

    return thisBuffer;
}

class buffer *bufferlist::Enumerate(bufferlist_efptr mapFunction, long  functionData)
            {
    struct listentry *traverse, *next;

    for (traverse = this->head; traverse != NULL; traverse = next) {
        next = traverse->next; /* So mapFunction is allowed to delete the buffer. */
        if ((*mapFunction)(traverse->buffer, functionData))
            return traverse->buffer;
    }
    return NULL;
}

class buffer *bufferlist::FindBufferByFile(const char  *filename)
        {
    char realName[MAXPATHLEN];
    char *bufferFilename;
    struct listentry *traverse;

    filetype::CanonicalizeFilename(realName, filename, sizeof(realName) - 1);
    /* remove trailing slash from directory names to match GetBufferOnFile's naming convention */
    if (realName[0] && realName[strlen(realName) - 1] == '/')
	realName[strlen(realName) - 1] = '\0';

    for (traverse = this->head; traverse != NULL; traverse = traverse->next) {
	bufferFilename = (traverse->buffer)->GetFilename();
	if (bufferFilename != NULL && (strcmp(bufferFilename, realName) == 0)) {
	    return traverse->buffer;
	}
    }
    return NULL;
}

class buffer *bufferlist::FindBufferByData(class dataobject  *bufferData)
        {
    struct listentry *traverse;

    for (traverse = this->head; traverse != NULL; traverse = traverse->next) {
	if ((traverse->buffer)->GetData() == bufferData) {
	    return traverse->buffer;
	}
    }
    return NULL;
}

/* Changed Bufferlist */

class buffer *bufferlist::FindBufferByName(const char  *name)
        {
    char *bufferName;
    struct listentry *traverse;

    for (traverse = this->head; traverse != NULL; traverse = traverse->next) {
	bufferName = (traverse->buffer)->GetName();
	if (bufferName != NULL && (strcmp(bufferName, name) == 0)) {
	    return traverse->buffer;
	}
    }
    return NULL;
}

/* The interaction between this routine and buffer_Create is treacherous.
 * I have buffer_Create allocate the data object so buffer_Destroy will
 * deallocate it.  Yet this routine must look up the class of the data
 * object because it has the FILE *. Perhaps this is all bogus...
 */

class buffer *bufferlist::GetBufferOnFile(const char  *filename, long  flags)
{
    char realName[MAXPATHLEN];
    class buffer *thisBuffer;
    struct stat statBuf;
    boolean fileExists, fileIsDir;
    long objectID;
    long version;
    boolean readOnly;
    char bufferName[100];
    const char *objectName;
    FILE *thisFile;
    struct attributes *attributes, *tempAttribute, readOnlyAttribute, rawModeAttribute;

    filetype::CanonicalizeFilename(realName, filename, sizeof (realName) - 1);
    filename = realName;

    fileExists = fileIsDir = FALSE;
    if (stat(filename, &statBuf) >= 0) {
	fileExists = TRUE;
	if ((statBuf.st_mode & S_IFMT) == S_IFDIR) {
	    fileIsDir = TRUE;
	}
    }


    if (fileIsDir && ((flags & buffer_RawMode) == 0)) {
	struct attributes attr;
	class dataobject *dobj;
	char *tfilename = NULL;
	/* Force filename to not end in '/' before visiting directories unless only a '/'. */
	/* Do this so there aren't two different buffers on the same dir */
	/* with different names; one with slash, one without */
#ifdef DOS_STYLE_PATHNAMES
	if (strlen(filename) > 1 && filename[strlen(filename) - 1] == '/' && filename[strlen(filename) - 2] != ':') {
#else
	if (strlen(filename) > 1 && filename[strlen(filename) - 1] == '/') {
#endif
	    tfilename = strdup(filename);
	    /* tjm - not sure how to respond to errors, so ignore */
	    filename = tfilename;
	    tfilename[strlen(tfilename) - 1] = '\0';
	}
	if(!(objectName = environ::GetProfile("DirectoryEditor")))
	    objectName = "dired";
	/* Use existing dired buffer and dired if exists */
	(this)->GuessBufferName( filename, bufferName, sizeof(bufferName));
	thisBuffer = (this)->FindBufferByName( bufferName);
	if (thisBuffer) {
	    /* In case it exists by accident */
	    dobj = (thisBuffer)->GetData();
	    if (strcmp((dobj)->GetTypeName(), objectName) != 0)
		thisBuffer = NULL;
	} else {
	    /* Create a new dired and dired buffer */
	    if ((flags & buffer_MustExist) && ! fileExists) {
		if(tfilename)
		    free(tfilename);
		return NULL;
	    }
	    dobj = (class dataobject *) ATK::NewObject(objectName);
	    if (dobj == NULL)
		thisBuffer = NULL;
	    else
		thisBuffer = (this)->CreateBuffer( bufferName, NULL, NULL, dobj);
	}
	if (thisBuffer == NULL) {
	    errno = EISDIR;
	    if(tfilename)
		free(tfilename);
	    return NULL;
	}
	/* Tell it which dir to use */
	attr.key = "dir";
	attr.value.string = filename;
	attr.next = NULL;
	(dobj)->SetAttributes( &attr);
	(thisBuffer)->SetFilename(filename);
	if(tfilename)
	    free(tfilename);
    } else {

	if ((flags & buffer_MustExist) && ! fileExists) {
	    return NULL;
	}

	/* Try to find existing buffer. */

	if ((flags & buffer_ForceNew) == 0)
	    if ((thisBuffer = (this)->FindBufferByFile( filename)) != NULL)
		return thisBuffer;

	if ((thisFile = fopen(filename, "rb")) == NULL) {
	    if (access(filename, W_OK) < 0) {
		const char *slash;
		char *tf;
		if (errno != ENOENT)
		    return NULL;
		slash = strrchr(filename, '/');
		if (slash == NULL)
		    return NULL;
		tf = strdup(filename);
		if(!tf)
		    return NULL;
#ifdef DOS_STYLE_PATHNAMES
		/* If only the drive letter is left, we don't want to take the trailing slash away. Admittedly, this is not very attractive to implement. */
		if (*(slash-1) == ':')
		    tf[(int)(slash - filename)+1] = '\0';
		else tf[(int)(slash - filename)] = '\0';
		if (access(tf, W_OK) < 0) {
		    free(tf);
		    return NULL;
		}
#else
		tf[(int)(slash-filename)] = '\0';
		if (access(tf, W_OK) < 0) {
		    free(tf); return NULL;
		}
#endif
		free(tf);
	    }
	}

	if ((flags & buffer_RawMode) != 0) {
	    objectName = "text";
	    objectID = 0;
	    attributes = &rawModeAttribute;
	    rawModeAttribute.next = NULL;
	    rawModeAttribute.key = "datastream";
	    rawModeAttribute.value.string = "no";
	    readOnly = TRUE;
	}
	else {
	    objectName = filetype::Lookup(thisFile, filename, &objectID, &attributes);
	}

	readOnly = FALSE;
	for (tempAttribute = attributes; tempAttribute != NULL; tempAttribute = tempAttribute->next)
	    if (strcmp(tempAttribute->key, "readonly") == 0)
		readOnly = tempAttribute->value.integer;

	if (!readOnly && (flags & buffer_ReadOnly)) {
	    readOnlyAttribute.next = attributes;
	    readOnlyAttribute.key = "readonly";
	    readOnlyAttribute.value.integer = TRUE;
	    attributes = &readOnlyAttribute;
	    readOnly = TRUE;
	}

	(this)->GuessBufferName( filename, bufferName, sizeof(bufferName));
	thisBuffer = (this)->CreateBuffer( bufferName, filename, objectName, NULL);

	if (thisBuffer == NULL) {
	    errno = 0;      /* Don't signal Unix error unless one occurred */
	    return NULL;
	}

	((thisBuffer)->GetData())->SetAttributes( attributes);
	if (thisFile != NULL) {
	    ((thisBuffer)->GetData())->Read( thisFile, objectID);
	    fclose(thisFile);
	    (thisBuffer)->SetLastTouchDate( (long) statBuf.st_mtime);
	}

	(thisBuffer)->SetReadOnly( readOnly);
    }

    version = ((thisBuffer)->GetData())->GetModified();
    (thisBuffer)->SetCkpClock( 0);
    (thisBuffer)->SetCkpVersion( version);
    (thisBuffer)->SetWriteVersion( version);
    (thisBuffer)->SetIsRawFile( (flags & buffer_RawMode) != 0);
    return thisBuffer;
}

void bufferlist::GetUniqueBufferName(const char  *proposedName, char  *bufferName, int  nameSize)
{
    int uniquefier, nameLength;

    strcpy(bufferName, proposedName);

/* Find out if buffer exists. */
    if (buffer::FindBufferByName(bufferName) == NULL)
        return;

/* Otherwise we must uniquify it.
 * This is a bug, it may overflow the string buffer we were given...
 */
    nameLength = strlen(bufferName);
    for (uniquefier = 1; uniquefier < 100; uniquefier++) {
        sprintf(bufferName + nameLength, "-%d", uniquefier);
        if (buffer::FindBufferByName(bufferName) == NULL)
            return;
    }
    *bufferName = '\0'; /* Make sure we don't return a non-unique buffername. */
    return;
}

void bufferlist::GuessBufferName (const char  *filename , char  *bufferName, int  nameSize)
            {
    const char *slash;
    char newBufferName[MAXPATHLEN];

    slash = strrchr(filename, '/');
    if (slash != NULL) {
#ifdef DOS_STYLE_PATHNAMES
	if(strlen(slash)==1) { /*copy the drive letter and colon if at root so that we don't have just a '/' for every drive's root.*/
	    strncpy(newBufferName, filename, nameSize - 3);
	    newBufferName[2] = '\0';
	}
	else
#endif
	strncpy(newBufferName, ++slash, nameSize - 3); /* Save room for uniquefier. */
    }
    else
	strncpy(newBufferName, filename, nameSize - 3); /* Save room for uniquefier. */

    newBufferName[nameSize-3] = '\0';

    (this)->GetUniqueBufferName( newBufferName, bufferName, nameSize);

}


void bufferlist::SetDefaultObject(const char  *objectname)
{
    if (objectname != NULL)
        strncpy(defaultobjectname,objectname, sizeof(defaultobjectname));
    else
        strncpy(defaultobjectname, DEFAULTOBJECT, sizeof(defaultobjectname));
}

void bufferlist::Sort(bufferlist_sfptr compareRoutine)
{
    long cnt;
    struct listentry *traverse;
    class buffer **sortlist;

    /* Count number of buffers */
    for (cnt = 0, traverse = head; traverse; traverse = traverse->next, ++cnt);
    if (cnt < 2) return;	/* Nothing to sort */

    /* Build array of buffer *'s, and sort them with qsort and the compare routine passed to us */
    sortlist = (class buffer **)malloc(sizeof(class buffer *) * cnt);
    for (cnt = 0, traverse = head; traverse; traverse = traverse->next, ++cnt)
	sortlist[cnt] = traverse->buffer;
    qsort(sortlist, cnt, sizeof(class buffer *), QSORTFUNC(compareRoutine));

    /* Replace reordered buffer *'s in linked list */
    for (cnt = 0, traverse = head; traverse; traverse = traverse->next, ++cnt)
	traverse->buffer = sortlist[cnt];

    free(sortlist);
}
