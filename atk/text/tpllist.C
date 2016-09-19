/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

#include <andrewos.h>
#include <text.H>

#include <environ.H>
#include <filetype.H>
#include <style.H>
#include <stylesheet.H>


#define StateInit		0
#define StateSawSlash		1
#define StateSawKeyword		2

struct templatelist  {
    class stylesheet *sSheet;
    struct templatelist *next;
    boolean hasText;
};

static struct templatelist *tlHead = NULL;

static struct templatelist *FindTemplate(class stylesheet  *ssptr);
static struct templatelist *text_AddTemplate(class stylesheet  *ssptr);
static struct templatelist *text_FindTemplateByName(const char  *templateName);
static void text_OverrideStyles(class stylesheet  *ssptr, class stylesheet  *templateptr);
static void SetGlobalStyleInText(class text  *self);


static struct templatelist *FindTemplate(class stylesheet  *ssptr)
    {
    struct templatelist *tPtr;
    
    for (tPtr = tlHead; tPtr != NULL && tPtr->sSheet != ssptr; tPtr = tPtr->next)
        ;
    return tPtr;
}

static struct templatelist *text_AddTemplate(class stylesheet  *ssptr)
    {
    struct templatelist *newTL;

    if (FindTemplate(ssptr) != NULL) return NULL;
    newTL = (struct templatelist *) malloc(sizeof(struct templatelist));
    newTL->next = tlHead;
    tlHead = newTL;
    newTL->sSheet = ssptr;
    newTL->hasText = TRUE;
    return newTL;
}

static struct templatelist *text_FindTemplateByName(const char  *templateName)
    {
    struct templatelist *tPtr;
    
    for (tPtr = tlHead; tPtr != NULL && strcmp(tPtr->sSheet->templateName, templateName) != 0; tPtr = tPtr->next)
        ;
    return tPtr;
}

static void text_OverrideStyles(class stylesheet  *ssptr, class stylesheet  *templateptr)
        {
    int i;
    boolean forcecopy=FALSE;
    class style **styles, *overridestyle;
    if(ssptr->si==templateptr->si) {
	// The document is already using the same exact template.
	return;
    }
    
    for (i = 0, styles = ssptr->styles; i < ssptr->nstyles; i++, styles++) {
	if(!forcecopy && !templateptr->Find((*styles)->GetName())) forcecopy=TRUE;
    }
    if(forcecopy) {
	ssptr->PrepareForChanges();
	for (i = 0, styles = ssptr->styles; i < ssptr->nstyles; i++, styles++) {
	    (*styles)->template_c = 0;
	}
	// If there were styles in ssptr not present in templateptr, then we'll just have to
	// copy the styles from templateptr into ssptr
	for (i = 0, styles = templateptr->styles; i < templateptr->nstyles; i++, styles++) {
	    if ((overridestyle = (ssptr)->Find( (*styles)->name))) {
		((*styles))->Copy( overridestyle);
	    } else {
		overridestyle = new style;
		((*styles))->Copy( overridestyle);
		(ssptr)->Add( overridestyle);
	    }
	}
    } else {
	// If all the styles in ssptr are also available in templateptr, then
	// we can just do a lazy copy of the whole stylesheetInternal.
	templateptr->Copy(ssptr);
    }
    
    (ssptr)->SetTemplateName( templateptr->templateName);
    ssptr->version++;
}

static void SetGlobalStyleInText(class text  *self)
{
    class style *styleptr = (self->styleSheet)->GetGlobalStyle();

    if (styleptr == NULL) {
	/* Need to add global style to template */

	styleptr = new style;
	(styleptr)->SetName( "global");
	styleptr->template_c = 1;
	(self->styleSheet)->Add( styleptr);
    }
    (self)->SetGlobalStyle( styleptr);
}

	static FILE *
TryPath(char *filename, const char *path, const char *tplnm, const char *ext) {
		// (filename must be a buffer of size MAXPATHLEN+1)
	char rawname[MAXPATHLEN+1];
	sprintf(rawname, "%s/%s%s", path, tplnm, ext);
	filetype::CanonicalizeFilename(filename, rawname, 
			MAXPATHLEN);
	return fopen(filename, "r");
}

/* This routine parses the contents of a template file */
long text::ReadTemplate(const char  *templateName, boolean  inserttemplatetext)
            {
    FILE *fileptr;
    int c, statecode, i;
    class stylesheet *templateptr = NULL;
    struct templatelist *tlPtr;
    char filename[MAXPATHLEN+1], string[1000];
    const char *tpath;
    boolean overrideTemplate = TRUE;

    if ((tlPtr = text_FindTemplateByName(templateName)) != NULL && (templateptr = tlPtr->sSheet) != NULL)   {
	if  (inserttemplatetext && tlPtr->hasText && ! this->pendingReadOnly)
	    overrideTemplate = FALSE;
	else  {
	    /* probably should stat file here to see if it's changed */
	    text_OverrideStyles(this->styleSheet, templateptr);
	    SetGlobalStyleInText(this);
	    return 0;
	}
    }

    if (templateName[0] == '/') {
	strcpy(filename, templateName);
	fileptr = fopen(filename, "r");
    }
    else {
        tpath = environ::Get("TEMPLATEPATH");
        if (!tpath || !*tpath)
            tpath = environ::GetProfile("atktemplatepath");
        if (!tpath || !*tpath)
            tpath = environ::GetProfile("be2templatepath");
	if (!tpath || !*tpath) {
	    tpath = environ::AndrewDir("/lib/tpls");
	}
	if (!tpath || !*tpath) {
	    tpath = environ::AndrewDir("/lib/templates");
	}
        if (!tpath || !*tpath)
            tpath = ".";
        do {
            char pathBuffer[100], *thisPath;

            thisPath = pathBuffer;
#ifdef DOS_STYLE_PATHNAMES
#define TEMPLATESEP ';'
#else
#define TEMPLATESEP ':'
#endif
            while ((*tpath == ' ') || (*tpath == TEMPLATESEP))
                tpath++;
            while (thisPath < pathBuffer + sizeof(pathBuffer) - 1
				&& *tpath != ' ' && *tpath != '\0'
				&& *tpath != TEMPLATESEP)
                *thisPath++ = *tpath++;
            *thisPath = '\0';
	    if ((fileptr=TryPath(filename, pathBuffer, templateName, 
				".tpl"))) break;
	    if ((fileptr=TryPath(filename, pathBuffer, templateName, 
				".template"))) break;
	    if ((fileptr=TryPath(filename, pathBuffer, templateName, 
				""))) break;
        } while (*tpath);
    }

    if (!fileptr) {
	fprintf(stderr, "Template '%s' not found.\n", templateName);
	return -1;
    }

    if (templateptr == NULL)  {
	templateptr = (class stylesheet *) new stylesheet;
	(templateptr)->SetTemplateName( templateName);
    }
    if (tlPtr == NULL)  {
	tlPtr = text_AddTemplate(templateptr);
    }
    else  {
        tlPtr->sSheet = templateptr;
	tlPtr->hasText = TRUE;
    }

    if (inserttemplatetext && ! this->pendingReadOnly) {
	long objectID;
	char *tempTemplateName;

	(void) filetype::Lookup(fileptr, filename, &objectID, NULL);
	tempTemplateName = this->templateName;
	this->templateName = NULL;
	(this)->Read( fileptr, objectID);	/* only for new files */
	this->templateName = tempTemplateName;
	tlPtr->hasText = ((this)->GetLength() != 0);
	fclose(fileptr);
	if (overrideTemplate)  {
	    long i;
	    class style **styles;

	    (this->styleSheet)->SetTemplateName( templateptr->templateName);
	    text_OverrideStyles(templateptr, this->styleSheet);
	    for (i = 0, styles = templateptr->styles; i < templateptr->nstyles; i++, styles++) {
		(*styles)->template_c = 1;
	    }
	    for (i = 0, styles = this->styleSheet->styles; i < this->styleSheet->nstyles; i++, styles++) {
		(*styles)->template_c = 1;
	    }
	}
	else
	    text_OverrideStyles(this->styleSheet, templateptr);
	SetGlobalStyleInText(this);
	return 0;
    }
    i = 0;
    statecode = StateInit;
    if ((statecode != StateInit) && (statecode != StateSawSlash) &&
	(statecode != StateSawKeyword)) {
	if (fileptr) fclose(fileptr);
	return -2;
    }
    while ((c = getc(fileptr)) != EOF) {
        switch (statecode) {
	    case StateInit:
		if (c == '\\') {	    /* beginning of keyword */
		    statecode = StateSawSlash;
		}
		else if ((c == 0x83) || ((0xc0 & c) == 0xc0) || (c == 0x80)) {
		    fprintf(stderr, "Old style template - ignoring.\n");
		    fclose(fileptr);
		    return -1;
		}
		/* else ; // only used for existing files, don't put out text */
		break;
	    case StateSawSlash:
		if ((c != '\\') && (c != '{') && (c != '}') && c!='\n'/* just in case there is a long line of spaces followed by more default text than fits into string[] -RSK */) {
		    string[i++] = c;
		    statecode = StateSawKeyword;
		}
		else {
		    statecode = StateInit;		/* don't put out text */
		}
		break;
	    case StateSawKeyword:
		if (c == '{') {
		    string[i] = '\0'; i = 0;
		    if (strcmp(string, "textdsversion") == 0)  {
			long versionnumber = 0;

			while ((c = getc(fileptr)) != EOF && c != '}')
			    versionnumber = versionnumber * 10 + (c - '0');
			while (c != EOF && (c = getc(fileptr)) != '\n');

			/* Handle outdated data stream versions here */
			statecode = StateInit;
		    }
		    else if (strcmp(string, "define") == 0) {
			if ((templateptr)->Read( fileptr, 1) != 0) {
			    if (fileptr) fclose(fileptr);
			    return -2;
			}
			statecode = StateInit;
		    }
		    else if (strcmp(string, "template") == 0) {
			fprintf(stderr, "Recursive templates disallowed - ignoring.\n");
			statecode = StateInit;
		    }
		    else if (strcmp(string, "global") == 0) {
			/* handle global attributes here */
			statecode = StateInit;
		    }
		    else {
			statecode = StateInit;		/* ignore other keywords */
		    }
		}
		else string[i++] = c;
		break;
	}
    }
    fclose(fileptr);
    text_OverrideStyles(this->styleSheet, templateptr);
    SetGlobalStyleInText(this);
    return 0;
}

