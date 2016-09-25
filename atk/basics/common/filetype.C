/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

#include <andrewos.h> /* sys/types.h sys/file.h */
ATK_IMPL("filetype.H")
#include <stdio.h>
#include <util.h>

#include <sys/param.h>
#include <errno.h>
#include <sys/stat.h>
#include <ctype.h>
#include <pwd.h>

/* #include <im.ih> */
#include <attribs.h>
#include <environ.H>
#include <path.H>
#include <filetype.H>

#define DEFAULTTYPE "text"

static struct mapEntry {
    struct mapEntry *next;
    char *fileExtension;
    char *dataName;
    struct attributes *newAttributes;
    struct attributes *existingAttributes;
} *allEntries = NULL, defaultMapping = {NULL, NULL, NULL, NULL, NULL};

/* These next statics are fairly bogus. */
static char lastFilename[MAXPATHLEN];
static char lastExtension[32]; /* Extensions > 32 char's are trunc'ed. */

/* Some static space for holding file dependent attributes (as opposed to
 * extension specific ones).
 */
static struct attributes hardWiredAttributes[4];

#define SIZEATTRINDEX 0 /* Where in above array to put file's size. */
#define NAMEATTRINDEX 1 /* Where to put filename attribute. */
#define EXTATTRINDEX  2 /* Where to put file's extension. */
#define READONLYINDEX 3 /* Where to put the readonly flag. */


ATKdefineRegistry(filetype, ATK, NULL);


void filetype::FreeAttributes(struct attributes *attributes)
{
    struct attributes *thisAttr, *nextAttr;

    /* This assumes an allocated list of attributes */
    for (thisAttr = attributes; thisAttr != NULL; thisAttr = nextAttr) {
        free((char *)thisAttr->key);
        if (thisAttr->value.string != NULL) /* Attributes on this list are guaranteed to be string attributes... */
            free((char *)thisAttr->value.string);
        nextAttr = thisAttr->next;
        free(thisAttr);
    }
}

static struct mapEntry *GetEntry(const char  *extension, const char  *dataName)
{
    struct mapEntry *thisEntry;

    if (strcmp(extension, "*") == 0) {
        thisEntry = &defaultMapping;
	defaultMapping.fileExtension = (char *)"*"; /* never freed */
    }
    else {
        for (thisEntry = allEntries; thisEntry != NULL && strcmp(thisEntry->fileExtension, extension); thisEntry = thisEntry->next)
	    ;
    }

    if (thisEntry == NULL) { /* Allocate an entry if we didn't find one. */
        thisEntry = (struct mapEntry *) malloc(sizeof(struct mapEntry));
        thisEntry->fileExtension = strdup(extension);
	thisEntry->dataName = NULL;
	thisEntry->newAttributes = NULL;
	thisEntry->existingAttributes = NULL;
        thisEntry->next = allEntries;
        allEntries = thisEntry;
    }

/* Fill in name. */
    if (thisEntry->dataName != NULL)
	free(thisEntry->dataName);
    thisEntry->dataName = strdup(dataName);
    
    return thisEntry;
}

struct attributes *filetype::ParseAttributes(const char *attributes)
{
/* If necessary, parse the semicolon-seperated string of assignments to generate the
 * values list. The string looks like "key1=value1;key2=value2;...". All values
 * that result from this parsing are string values, integer values are never
 * generated. The only way integer values show up at present is in the hardwired
 * attributes from filetype_Lookup below.
 *
 * This function does no whitespace parsing in order to be "literal" about the
 * values it generates.
 */
    struct attributes *attr = NULL;

    if (attributes != NULL) {
        const char *thisKey = attributes, *thisVal;
        struct attributes *thisAttr;

        while (*thisKey != '\0') {
            for (thisVal = thisKey; *thisVal != '=' && *thisVal != '\0'; ++thisVal)
                ; /* Skip to find value */
            if (thisVal != thisKey) { /* If we found something. */
		char *s;
                thisAttr = (struct attributes *) malloc(sizeof(struct attributes));
                thisAttr->key = s = (char *) malloc(thisVal - thisKey + 1);
                strncpy(s, thisKey, thisVal - thisKey);
                s[thisVal - thisKey] = '\0';
                if (*thisVal != '\0') /* Guaranteed to be either '=' or '\0' */
                    thisVal++;
                for (thisKey = thisVal; *thisKey != ';' && *thisKey != '\0'; ++thisKey)
                    ;
                if (thisKey != thisVal) {
                    thisAttr->value.string = s = (char *) malloc(thisKey - thisVal + 1);
                    strncpy(s, thisVal, thisKey - thisVal);
                    s[thisKey - thisVal] = '\0';
                }
                else
                    thisAttr->value.string = NULL;
                thisAttr->next = attr;
                attr = thisAttr;
                if (*thisKey == ';')
                    ++thisKey;
            }
	    else /* Else, guarantee the loop will terminate. */
		if (*thisKey != '\0')
		    ++thisKey;
        }
    }

    return attr;
}

void filetype::AddEntry(const char  *extension , const char  *dataName, const char  *attributes)
{
    struct mapEntry *thisEntry;

    thisEntry = GetEntry(extension, dataName);

    FreeAttributes(thisEntry->newAttributes);
    thisEntry->newAttributes = ParseAttributes(attributes); 
}

void filetype::AddExistingAttributes(const char  *extension , const char  *dataName, const char  *attributes)
{
    struct mapEntry *thisEntry;

    thisEntry = GetEntry(extension, dataName);

    FreeAttributes(thisEntry->existingAttributes);
    thisEntry->existingAttributes = ParseAttributes(attributes); 
}

int filetype::DeleteEntry(char  *extension)
{
    struct mapEntry *traverse, **previous;

    if (strcmp(extension, "*")) {
        if (defaultMapping.dataName)
            free(defaultMapping.dataName);
	FreeAttributes(defaultMapping.newAttributes);
	defaultMapping.newAttributes = NULL;
	FreeAttributes(defaultMapping.existingAttributes);
	defaultMapping.existingAttributes = NULL;
        defaultMapping.fileExtension = NULL;
        return 1;
    }
    previous = &allEntries;
    for (traverse = allEntries; traverse != NULL; traverse = traverse->next) {
        if (strcmp(traverse->fileExtension, extension) == 0) {
            free(traverse->fileExtension);
            free(traverse->dataName);
            FreeAttributes(traverse->newAttributes);
            FreeAttributes(traverse->existingAttributes);
            *previous = traverse->next;
            free(traverse);
            return 1;
        }
        previous = &traverse->next;
    }
    return 0;
}

/* NOTE: filetype_Lookup.
 *    The value returned through the attributes parameter to this function
 *    contains pointers to static storage. Its value should be used before
 *    the next call to this routine. If this can't be guaranteed, the
 *    programmer must copy the attributes list.
 */
char *filetype::Lookup(FILE  *file, const char  *filename, long  *objectID, struct attributes  **attributes)
{
    struct mapEntry *thisEntry = &defaultMapping;
    const char *extension;
    static char objectName[100]; /* The place to put the name of the class that should be used to read this file. */
    const char *targetObject = NULL; /* Holds potential value for objectName. */
    struct attributes *newAttributes = NULL; /* Only used if the file is in non-datastream format. */
    struct attributes *existingAttributes = NULL;

    if (attributes)
        *attributes = NULL; /* In case we don't set it otherwise. */
/* First, if possible, do the lookup on the filename extension
 * to get the attributes and possibly the object (class) type.
 * This information may be overrriden by that in the file.
 */
    if (filename != NULL && filename[0] != '\0') {
        const char *s;
	char prefix[256];
	char *dotpos;

	if ((s = strrchr(filename, '/'))) ++s;
	else s = filename;
	strncpy(prefix, s, sizeof(prefix));
	prefix[sizeof(prefix)-1] = '\0';
	if ((dotpos = strchr(prefix, '.'))) *(dotpos+1) = '\0';

        extension = strrchr(filename, '.');
        if (extension == NULL || (s != NULL && s > extension))
            extension = "";

	for(thisEntry = allEntries; thisEntry != NULL; thisEntry = thisEntry->next) {
	    if (FoldedWildEQ(extension, thisEntry->fileExtension, FALSE))  {
                break;
	    }
	    if (FoldedWildEQ(prefix, thisEntry->fileExtension, FALSE))  {
		extension = prefix;
		break;
	    }
	}

	if (thisEntry == NULL) {
	    thisEntry = &defaultMapping;
	}

	targetObject = thisEntry->dataName;
	newAttributes = thisEntry->newAttributes;
	existingAttributes = thisEntry->existingAttributes;


/* Setup hardwired attributes in static storage.
 * Currently, we support the following hardwired attributes:
 *    "filename" => The full pathname of the file as given to this routine.
 *    "extension" => The file's extension, either of the form ".x"
 *                    (where x is a string) or "".
 *    "filesize" => An integer attribute giving the size (in bytes) of the file.
 *                  This is filled in below in the file part of this routine.
 */
/* hardwiredAttributes are never freed, so it's OK to use read-only strings */
        if (attributes != NULL) {
            strcpy(lastFilename, filename);
            /* Why can't the string library do the right thing? */
            strncpy(lastExtension, extension, sizeof(lastExtension));
            lastExtension[sizeof(lastExtension) - 1] = '\0';

            hardWiredAttributes[EXTATTRINDEX].key = "extension";
            hardWiredAttributes[EXTATTRINDEX].value.string = lastExtension;
	    hardWiredAttributes[EXTATTRINDEX].next = existingAttributes;

            hardWiredAttributes[NAMEATTRINDEX].key = "filename";
            hardWiredAttributes[NAMEATTRINDEX].value.string = lastFilename;
            hardWiredAttributes[NAMEATTRINDEX].next = &hardWiredAttributes[EXTATTRINDEX];

            if ((access(filename, W_OK) < 0) && (errno == EACCES || errno == EROFS)) {
                hardWiredAttributes[READONLYINDEX].key = "readonly";
                hardWiredAttributes[READONLYINDEX].value.integer = TRUE;
                hardWiredAttributes[READONLYINDEX]. next = &hardWiredAttributes[NAMEATTRINDEX];
                *attributes = &hardWiredAttributes[READONLYINDEX];
            }
            else
                *attributes = &hardWiredAttributes[NAMEATTRINDEX];
        }
    }

/* Now try and get any info we need out of the file. */
    if (file != NULL) {
        long origPos;
        int c;
	struct stat statBuf;

        origPos = ftell(file);

        if ((attributes != NULL) && (fstat(fileno(file), &statBuf) >= 0)) { /* This may not work for pipes... */
            hardWiredAttributes[SIZEATTRINDEX].key = "filesize";
            hardWiredAttributes[SIZEATTRINDEX].value.integer = statBuf.st_size;
            hardWiredAttributes[SIZEATTRINDEX].next = *attributes;
            *attributes = &hardWiredAttributes[SIZEATTRINDEX];
	}

	switch (getc(file)) {
	    case '\\': { /* "\begindata{"? */
		const char *s= "begindata{";
		char objectIDBuffer[20], *readID= objectIDBuffer;

		while (getc(file) == *s && *++s != '\0')
		    ;
		if (*s == '\0') {
		    char *o = objectName;
		    while ((c = getc(file)) != EOF && c != ',')
			*o++ = c;
		    if (c == ',') {
			*o = '\0';
			while ((c= getc(file)) != EOF && c != '}' && (readID < (objectIDBuffer + (sizeof(objectIDBuffer) - 1))))
			    *readID++ = c;
			if (c == '}') {
			    if ((c = getc(file)) != '\n')
				ungetc(c, file);
			    if (objectID != NULL) {
				*readID = '\0';
				*objectID = atoi(objectIDBuffer);
			    }
			    return objectName;
			}
		    }
		}
		break; }
	    case '#': { /* "# ! /../shell"? */
		if (thisEntry != &defaultMapping) break; /* extension is explicitly bound to something; don't even look */
#define MAXSHELLNAME 256
		char shellname[MAXSHELLNAME], *readsh= shellname;
		while ((c= getc(file))==' ' || c=='\t')
		    ; /* skip whitespace */
		if (c!='!') break;
		shellname[0]= '\0';
		do  {
		    c= getc(file);
		    if (c=='/')
			readsh= shellname;
		    else {
			if (readsh>shellname && (c==' ' || c=='\t')) c= '\0';
			*readsh++= c;
			if (readsh >= shellname+MAXSHELLNAME)
			    c= EOF;
		    }
		} while (c!='\n' && c!=EOF);
		*readsh= '\0';
		/* shellname should now contain everything after the last slash; look for a recognizable shell name */
		if (strstr(shellname, "perl"))
		    targetObject= "perltext";
		else if (strstr(shellname, "rexx"))
		    targetObject= "rexxtext";
		else if (strstr(shellname, "sh")) {
		    /* use asmtext with static attribute to recognize comment delimiters, for ksh, zsh, bsh, etc */
		    /* note that this will be freed, since it's newAttributes */
		    static struct attributes shellcommentdelimatt;
		    shellcommentdelimatt.key= "bang-comment";
		    shellcommentdelimatt.value.string= "#";
		    shellcommentdelimatt.next= newAttributes;
		    newAttributes= &shellcommentdelimatt;
		    targetObject= "asmtext";
		    /*XXX- eventually, write a zsh and ksh and bsh source view, and use those dataobjects instead */
		}
		break; }
	    case 'G': { /* "GIF87" or "GIF89"? */
		if (thisEntry != &defaultMapping) break; /* extension is explicitly bound to something; don't even look */
		if (getc(file)!='I') break;
		if (getc(file)!='F') break;
		if (getc(file)!='8') break;
		if ((c= getc(file))=='7' || c=='9')
		    targetObject= "imageio";
		break; }
#if SY_OS2
	    case '/': { // "/*"?
		if (thisEntry != &defaultMapping) break; /* extension is explicitly bound to something; don't even look */
		if (getc(file) == '*')
		    /* almost any file starting with a slash-star in OS/2 is a REXX script */
		    targetObject= "rexxtext";
		break; }
#endif /*SY_OS2*/
	}

        fseek(file, origPos > 0 ? origPos : 0, 0L);
    }

    hardWiredAttributes[EXTATTRINDEX].next = newAttributes;

    if (objectID != NULL)
        *objectID = 0;

    if (targetObject != NULL) {
        strcpy(objectName, targetObject);
        return objectName;
    }
    else
        return NULL;
}

void filetype::CanonicalizeFilename(char  *canonicalName , const char  *name, int  maxSize)
{
    char fullName[MAXPATHLEN];

#ifdef DOS_STYLE_PATHNAMES
    char *slash;
    char *tmpstr;
    tmpstr = NewString(name);
    /* Change all backslashes to forward slashes */
    while ((slash = strchr(tmpstr, '\\')) != NULL)
	*slash = '/';
    strncpy(canonicalName, path::UnfoldFileName(tmpstr, fullName, 0), maxSize);
    free(tmpstr);
#else
    strncpy(canonicalName, path::UnfoldFileName(name, fullName, 0), maxSize);
#endif
    canonicalName[maxSize-1]='\0';
}
