/* This is a HTML viewing/editing object */
/* Unfortunately, there's lots of duplicate information between the stylesheet
 * info and the tables below.  It would be nice if we could snarf the info 
 * from the stylesheet, but it's just too much hassle
 */

/*
 * Copyright 1993, City University
 * Copyright 1993, 1994, Nick Williams. 
 * 
 * Permission to use, copy, modify, and distribute this software and its 
 * documentation for any purpose is hereby granted without fee, 
 * provided that the above copyright notice appear in all copies and that 
 * both that copyright notice, this permission notice, and the following 
 * disclaimer appear in supporting documentation, and that the names of 
 * City University, Nick Williams, and other copyright holders, not be 
 * used in advertising or publicity pertaining to distribution of the software 
 * without specific, written prior permission.
 *
 * City University, Nick Williams, AND THE OTHER COPYRIGHT HOLDERS 
 * DISCLAIM ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING 
 * ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.  IN NO EVENT 
 * SHALL City University, Nick Williams, OR ANY OTHER COPYRIGHT HOLDER 
 * BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY 
 * DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, 
 * WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS 
 * ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE 
 * OF THIS SOFTWARE.
 */

#include <andrewos.h> /* strings.h */

#include <ctype.h>
#include <stdio.h>

#include <attribs.h>
#include <buffer.H>
#include <dataobject.H>
#include <environ.H>
#include <environment.H>
#include <imageio.H>
/* #include <icon.H>
#include <iconview.H> */
#include <im.H>
#include <image.H>
#include <imagev.H>
#include <keystate.H>
#include <proctable.H>
#include <stylesheet.H>
#include <style.H>
#include <nestedmark.H>
#include <tree23int.H>
#include <text.H>
#include <view.H>
#include <viewref.H>

/* So that we can feedback. Ick. */
#include <message.H>
#include <htmlview.H>
#include <html.H>

#include "html.h"

#define NEED_STRING_PROTO

/* 
 * The InitializeClass() is here to parse the mapping file which translates 
 * between special characters (e.g. >, <, and &) and the HTML entity which 
 * represents them (e.g. "&gt;", "&lt;" and "&amp;").
 */

/* The following entities are supported as basic types: */
enum entityCodeEnum {
    entityHead =1,
    entityTitle,
    entityBody,
    entityHTML,

    entityAnchor,
    entityPre,
    entityHeader1,
    entityHeader2,
    entityBold,
    entityBlockQuote,
    entityEmphasised, /* 10 */
    entityAddress,
    entityParagraph,
    entityImage,
    entityIsIndex,
    entityNextId,
    entityListItem,
    entityDescTerm,
    entityDescDefun,
    entityEmpty,

    entityDescription, /* 20 */
    entityEnumerate,
    entityMenu,
    entityItemize,
    entityDirectory,

    entityItalic,
    entityUnderline,
    entityTypewriter,
    entityStrong,
    entityVariable,
    entityCitation,
    entityCode,
    entitySample,
    entityKeyboard,

    entityRule,
    entityBreak,
    entityForm,
    entityInput,
    entitySelection,
    entityOption,

    entityList = (1<<8),
};

ATKdefineRegistry(html, text, html::InitializeClass);
static char* html_MagicToString(char * x,int * len);
static const char* html_StringToMagic(const char * str);
static void  storeVar(class style * style, const char * key, char * value);
static void addVars(class style * style, char * vars);
static char* html_StyleToVariables(class style * style);
static void ChangeTitle(class html * self, struct entityElement * ep, char * buf, long  len);
static void ChangeIndexable(class html * self, struct entityElement * ep, char * buf, long  len);
static const struct entityMapping* getEntityMapping(const char * string);
static void  popEntity(class html * self);
static struct entityElement* pushEntity(class html * self, long * pos, const struct entityMapping * em, const char * name, char * vars, int  force);
static struct entityElement* entityPeek(class html * self);
static struct entityElement* withinEntity(class html * self, enum entityCodeEnum  code);
static struct entityElement* withinEntityClass(class html * self, enum entityCodeEnum  code);
static int html_FindEntity(char * buf, long * pos, char * entity, char * vars);
static void closeEntity(class html * self, struct entityElement * ep, long * pos, int  force);
static void maybeDisplay(class html * self, long * pos, char * buf, long * inlen);
static int newpar(class html * self, const struct entityMapping * eMapping, long  pos);
static void hrule(class html * self, long * pos);
static int  fixStyles(long  rock, class text * self, long  pos, class environment * curenv);
static void writeHeader(class html * self, FILE * file);
static void  PutsRange(class html * self, char  *p, FILE  *fp, char  *ep);
static const char* getHTML(class style * style);
static char* outputNewlines(int  newlines, int  parImplied, int  brImplied, char * outp);
static int html_StyleToCodes(class style * style);
static char* findLocalFile(char * path, char * relativeRoot);



#define entFlag(x) ((1)<<((x)+16))
#define entFlagsEnvironment 0
#define entFlagsNewline entFlag(0)
#define entFlagsSingle  entFlag(1)
#define entFlagsCompact entFlag(2)
#define entFlagsReplace entFlag(3)
#define entFlagsParagraph entFlag(4)
#define entFlagsParExplicit entFlag(10)

#define entFlagsNoStyle entFlag(6)
#define entFlagsUnknown entFlag(7)
#define entFlagsAdornment entFlag(8)

typedef void (*html_EntityFptr)(class html * self, struct entityElement * ep, char * buf, long  len);
/* And this is how they look... */
static const struct entityMapping {
    const char* string;
    enum entityCodeEnum code;
    int flags;
    html_EntityFptr fn; /* Function to call when environment ends */
} basicEntities[] = {
    /* Top level things */
    { "head",    entityHead,       0 },
    { "body",    entityBody,       entFlagsNoStyle },
    { "html", entityHTML, entFlagsSingle|entFlagsNoStyle },

    /* Things that go into header */
    { "title",   entityTitle,      0, ChangeTitle },
    { "nextid",  entityNextId,     entFlagsSingle },
    { "isindex", entityIsIndex,    entFlagsSingle, ChangeIndexable },

    /* Things that go into the body */
    { "a",	 entityAnchor,	   0 },
    { "pre",     entityPre,        entFlagsNewline|entFlagsParagraph },
    { "h1",      entityHeader1,    entFlagsNewline|entFlagsParagraph },
    { "h2",      entityHeader2,    entFlagsNewline|entFlagsParagraph },
    { "h3",      entityHeader2,    entFlagsNewline|entFlagsParagraph },
    { "h4",      entityHeader2,    entFlagsNewline },
    { "h5",      entityHeader2,    entFlagsNewline },
    { "h6",      entityHeader2,    entFlagsNewline },
    { "b",       entityBold, 0 },
    { "i",       entityItalic, 0 },
    { "u",       entityUnderline, 0 },
    { "tt",      entityTypewriter, 0 },
    { "strong",  entityStrong, 0 },
    { "em",      entityEmphasised, 0 },
    { "var",     entityVariable, 0 },
    { "cite",    entityCitation, 0 },
    { "code",    entityCode, 0 },
    { "samp",    entitySample, 0 },
    { "kbd",     entityKeyboard, 0 },
    { "address", entityAddress,    entFlagsNewline|entFlagsCompact },
    { "p",       entityParagraph,  entFlagsNewline|entFlagsParagraph|entFlagsParExplicit|entFlagsSingle|entFlagsNoStyle },
    { "blockquote", entityBlockQuote, entFlagsNewline },
    { "img",     entityImage, entFlagsSingle },
    { "hr", entityRule, entFlagsSingle|entFlagsUnknown|entFlagsNewline },
    { "br", entityBreak, entFlagsNewline|entFlagsParExplicit|entFlagsSingle|entFlagsNoStyle },

    /* List environments */
    { "dl",	 (enum entityCodeEnum)(entityList|entityDescription), entFlagsNewline },
    { "ul",	 (enum entityCodeEnum)(entityList|entityItemize),    entFlagsNewline },
    { "ol",	 (enum entityCodeEnum)(entityList|entityEnumerate),  entFlagsNewline },
    { "menu",	 (enum entityCodeEnum)(entityList|entityMenu),	      entFlagsNewline },
    { "dir",	 (enum entityCodeEnum)(entityList|entityDirectory),  entFlagsNewline },

    /* The elements of lists */
    { "li",	entityListItem,	    entFlagsReplace|entFlagsNewline }, 
    { "dt",	entityDescTerm,	    entFlagsReplace|entFlagsNewline },
    { "dd",	entityDescDefun,    entFlagsReplace|entFlagsNewline },

    /* Forms support */
    { "form", entityForm, 0 },
    { "input", entityInput, entFlagsSingle|entFlagsUnknown },
    { "select", entitySelection, entFlagsUnknown },
    { "option", entityOption, entFlagsReplace |entFlagsUnknown|entFlagsNewline },
    { 0 }
};

const struct entityMapping noEntity = { 0, entityEmpty, entFlagsUnknown };

struct styleMapping {
    const char* styleName;	
    const char* entityName;
};
static const struct styleMapping stylemap[] = {
    { "address", "address" },
    { "anchor",  "a" },
    { "header1", "h1" },
    { "header2", "h2" },
    { "header3", "h3" },
    { "header4", "h4" },
    { "header5", "h5" },
    { "header6", "h6" },
    { "header7", "h7" },
    { "header8", "h8" },
    { "bold",    "b" },
    { "italic",  "i" },
    { "typewriter", "tt" },
    { "underline", "u" },
    { "emphasised", "em" },
    { "preformatted", "pre" },
    { "list-item", "li" },
    { "bulleted-list", "ul" },
    { "enumerated-list", "ol" },
    { "directory", "dir" },
    { "data-tag", "dt" },
    { "data-description", "dd" },
    { 0, 0 }
};

/* The entities are as follows */
struct entityElement {
    char string[256];
    long start;
    long length;
    long data;
    class style* style;
    const struct entityMapping* em;
    struct entityElement* prev;
};

/* This is for the keys of variables stored as attributes on tags */
struct keylist {
    char name[80];
    struct keylist* next;
};
static struct keylist* keyList = 0;

/* Some constants... */
/* These are the return results of the FindEntity routine */
#define htmlFoundEntity   0
#define htmlPartialEntity 1
#define htmlNoEntity      2

static const char whiteSpace[] = " \t\n";
static const char parBreakString[] = "\n\n<p>";
static const char breakString[] = "<br>\n";

static char errbuf[200]; /* A string buffer to temporarily play with error messages */
static const char errString[] = "\nErrors encountered while reading document:\n\n";

/* This is the name of the attribute attached to the styles */
static const char styleHTMLCodes[] = "htmlcodes";

/* ------------------------------------------------------------------------ */
/*  Management and utility routines for the magic mappings                   */
/* ------------------------------------------------------------------------ */

/*
 * The next few declarations are for managing the translation file which maps 
 * special characters to html entities 
 */
struct HTMLMagicMapping {
    const char* magicstring;
    int magiclen;
    const char* string;
};
static struct HTMLMagicMapping HTMLDefaultCharMap[] = {
    { "<", 1, "lt" },
    { ">", 1, "gt" },
    { "&", 1, "amp" },
    { "\"", 1, "quot" },
    { 0, 0 }
};
static struct HTMLMagicMapping* HTMLCharMap = 0;
static char* HTMLMagicCharacters = 0;

boolean 
html::InitializeClass()
{
    struct HTMLMagicMapping* hmap;
    const char* HTMLMagicFile;
    int mapcount = 64; 
    char* hmstr;
    FILE* fp;

    if (!(HTMLMagicFile = environ::Get("HTMLMAGICFILE"))) {
        HTMLMagicFile = environ::AndrewDir("/lib/htmlmagic");
    }

    fp = fopen(HTMLMagicFile, "r");
    if (fp) {
	int mc = 0;
	char buf[256];
	char* mstr;
	char* estr;	

	fgets(buf, sizeof(buf), fp);
	if (isdigit(*buf)) {
	    mapcount = atoi(buf);
	    HTMLCharMap = (struct HTMLMagicMapping*)malloc(sizeof(struct HTMLMagicMapping)*(mapcount+1));
	
	    if (HTMLCharMap) {
	        hmap = HTMLCharMap;
	        while (fgets(buf, sizeof(buf), fp)) {
		    if (buf[0] == '#') {
		        continue;
		    }
		    mstr = strtok(buf, " \t");
		    if (!mstr) {
		        continue;
		    }
		    estr = strtok(0, " \t\n");
		    if (estr) {
		        hmap->magicstring = strdup(mstr);
		        hmap->string = strdup(estr);
		        if (!(hmap->magicstring && hmap->string)) {
			    fclose(fp);
			    return FALSE;
		        }
		        hmap->magiclen=strlen(mstr);
		        hmap++,mc++;
		        if (mc==mapcount) {
			    /* Stop here. Broken magic file*/
			    break;
		        }
		    }
	        }
	        hmap->magicstring = 0;
	    }	    
	}
	fclose(fp);
    }

    if (HTMLCharMap == 0) {
	HTMLCharMap = HTMLDefaultCharMap;
    }

    /* Build up the string containing all our special characters */
    HTMLMagicCharacters = (char*) malloc(sizeof(char)*mapcount+1);
    hmstr = HTMLMagicCharacters;
    for (hmap = HTMLCharMap; hmap && hmap->magicstring; hmap++) {
	*hmstr++ = *(hmap->magicstring);
    }
    *hmstr='\0';
    return TRUE;
}

/*
 * Take a "magic" character and return a string containing the html entity
 * which represents that character.  len is set to the length of the return
 * string.
 */  
static char*
html_MagicToString(char * x,int * len)
{
    struct HTMLMagicMapping* hm;
    static char buf[32];

    for (hm = HTMLCharMap; hm && hm->magicstring; hm++) {
	if (strncmp(hm->magicstring, x, hm->magiclen)==0) {
	    sprintf(buf, "&%s;", hm->string);
	    *len = hm->magiclen;
	    return buf;
	}
    }
    *len = 0;
    return 0;
}

/* 
 * Take a string which may be a html entity referring to a character.
 * If it is, then the "magic" string which the html translates to is returned.
 * else 0 is returned
 */
static const char*
html_StringToMagic(const char * str)
{
    struct HTMLMagicMapping* hm;
    for (hm = HTMLCharMap; hm && hm->magicstring; hm++) {
	if (strcmp(hm->string, str)==0) {
	    return hm->magicstring;
	}
    }
    return 0;
}

/*
*/
void
html::Inform(const char * msg)
{
    class text* errtext;
    static char buf[256];

    if (!this->errorBuffer) {
	this->errorBuffer = buffer::Create("HTML-Errors", NULL, "text", NULL);
	(this->errorBuffer)->SetScratch( TRUE);
	errtext =(class text *)(this->errorBuffer)->GetData();

	(errtext)->InsertCharacters( 0, versionString, strlen(versionString));
	(errtext)->InsertCharacters( (errtext)->GetLength(), errString, strlen(errString));
    }

    sprintf(buf, "line %ld: %s\n", this->lineNumber, msg);

    if (this->errorBuffer) {
	errtext =(class text *)(this->errorBuffer)->GetData();
	(errtext)->InsertCharacters( (errtext)->GetLength(), buf, strlen(buf));
    } else {
	fprintf(stderr, buf);
    }
}

boolean
html::HasErrors()
{
    if (this->errorBuffer) {
	return TRUE;
    } else {
	return FALSE;
    }
}

/* ------------------------------------------------------------------------ */
/* Management of instances of entities: Each instance of an entity may have */
/* variables attached to it                                                 */
/* ------------------------------------------------------------------------ */

/* Attach a variable assignment to a style */
/* If the value is NULL, then empty string ("") is placed as value */
static void 
storeVar(class style * style, const char * key, char * value)
{
    struct keylist* k = keyList;
    char buf[strlen(key)+1];
    char* s;
    strcpy(buf, key);
    for (s = buf; s && *s; s++) {
	if (isupper(*s)) {
	    *s = tolower(*s);
	}
    }

    for (k = keyList; k && (strcmp(k->name, buf)!=0) ; k = k->next)
	;

    if (!k) {
	/* Record the key for later use... */
	struct keylist* nk = (struct keylist*) malloc(sizeof(struct keylist));
	if (nk) {
	    strcpy(nk->name, buf);
	    nk->next = keyList;
	    keyList = nk;
	}
    }
    (style)->AddAttribute( buf, value ? value : "");
}

/* Parse a variable list from a tag calling storeVar on all the variables */
static void
addVars(class style * style, char * vars)
{
    char* s = vars;
    char* value = 0;
    char* key = s;
    int inquote = 0;

    for ( ; s && *s; s++) {
	if (*s == '\"') {
	    inquote = !inquote;
	} else if (*s == '=' && !inquote) {
	    *s = '\0';
	    value = s+1;
	} else if (strchr(whiteSpace, *s)) {
	    if (!inquote) {
		/* We have the end of a variable. Store it */
		*s = '\0';
		storeVar(style, key, value);
		value = 0;
		key = s+1;
	    } /* else ignore it */
	}
    }
    if (*key != '\0') {
	storeVar(style, key, value);
    }
}

static char*
html_StyleToVariables(class style * style)
{
    /* XXX: We don't watch to see if buf is overrun! */
    struct keylist* k;
    char* s;
    static char buf[256];
    char* bufptr = buf;

    buf[0] = '\0';
    for (k = keyList; k; k = k->next) {
	s = (style)->GetAttribute( k->name);
	if (s) {
	    *bufptr++ = ' ';
	    /* We have a valid variable attached to this style. Dump it */
	    if (s && *s != '\0') {
		sprintf(bufptr, "%s=%s", k->name, s);
	    } else {
		strcpy(bufptr, k->name);
	    }
	    bufptr += strlen(bufptr);
	}
    }
    return buf;
}

/* ------------------------------------------------------------------------ */
/* Some routines for outside use.  I.e. the view object                     */
/* ------------------------------------------------------------------------ */

void
html::ChangeTitle(const char * name)
{
    class buffer* buf = buffer::FindBufferByData(this);
    if (this->title)  {
	free(this->title);
    }
    this->title = strdup(name);
    if (this->title) {
	if (buf) {
	    (buf)->SetName( this->title);
	}
	(this)->SetModified();
	(this)->NotifyObservers( observable::OBJECTCHANGED);
    } else if (buf) {
	(buf)->SetName( "Untitled");
    }
}

void
html::ChangeIndexable(int  flag)
{
    this->isindex = flag;
    (this)->SetModified();
    (this)->NotifyObservers( observable::OBJECTCHANGED);
}

void
html::AddLink(long  inpos, long  len, char * uri)
{
    long pos = inpos;
    char vars[256];
    const struct entityMapping* em;
    struct entityElement* ep;

    em = getEntityMapping("a"); /* Get the stuff about this */
    sprintf(vars, "href=%s", uri);
    ep = pushEntity(this, &pos, em, "a", vars, 1);
    pos += len;
    closeEntity(this, ep, &pos, 1);
    popEntity(this);
    (this)->RegionModified( inpos, len);
    (this)->NotifyObservers( observable::OBJECTCHANGED);
}

char*
html::GetAnchorDest(long  pos)
{
    class environment* env;

    env = (class environment *)(this->text::rootEnvironment)->GetInnerMost( pos);
    while (env && !(env->type == environment_Style && strcmp(env->data.style->name, "anchor") == 0)) {
	env = (class environment*)(env)->GetParent();
    }
    if (!env) {
	return 0;
    }

    return html_StyleToVariables(env->data.style);
}

char*
html::GetAttribute(class environment * env, const char * attr)
{
    if (env->type != environment_Style) {
	return 0;
    } else {
	return (env->data.style)->GetAttribute( attr);
    }
}

void
html::ChangeAttribute(class view * tv, class environment * env, const char * attr, char * value)
{
    char cb[256];
    char* ptr;
    cb[0] = '0';

    /* XXX:  BUG! NYI
     * We need to take a copy of the style and free the old one, otherwise
     * we'll be adding this attribute onto all styles... 
     */
    if (env->type == environment_Style) {
	if (value && (*value=='\0' || strcmp(value, "\"\"")) == 0) {
	    (env->data.style)->RemoveAttribute( attr);
	} else {
	    storeVar(env->data.style, attr, value);
	}

	/* Check to see if this modification should trigger any callback */
	strcpy(cb, "callback:");
	strcat(cb, attr);
	if ((ptr = (env->data.style)->GetAttribute( cb))) {
	    /* Now we use the proctable to call the callback */
	    /* This code is taken from the metax class */
	    struct proctable_Entry *proc;
	    class im *im = (tv)->GetIM();

	    printf("doing a callback: %s\n", ptr);
	    proc = proctable::Lookup(ptr);
	    if(proc) {
		switch((im->keystate)->DoProc( proc, 0, tv)) {
		    case keystate_NoProc:
			message::DisplayString(im, 0, "Could not load procedure");
			break;
		    case keystate_TypeMismatch:
			message::DisplayString(im, 0, "Bad command");
			break;
		    case keystate_ProcCalled:
		        break;
		}
	    } else {
		message::DisplayString(im, 0, "Do nothing");
	    }
	}
	/* We're done */
	(this)->SetModified();
	(this)->NotifyObservers( observable::OBJECTCHANGED);
    }
}

void
html::GetAttributeList(class environment * env, char * list[], int * count)
{
    struct keylist* k;
    char* s;

    *count = 0;
    if (env->type != environment_Style) {
	return;
    }
    
    for (k = keyList; k; k = k->next) {
	s = (env->data.style)->GetAttribute( k->name);
	if (s) {
	    /* We have a valid variable attached to this style. Dump it */
		list[*count] = strdup(k->name);
		(*count)++;
	}
    }
}

/* ------------------------------------------------------------------------ */
/* Internal callbacks do not need to mess with modified flags or anything
 * like that: they are purely used during parsing. However, at the end of 
 * a parse run, you should make sure you call the SetModified and
 * NotifyObservers methods.                                                 */
/* ------------------------------------------------------------------------ */
static void
ChangeTitle(class html * self, struct entityElement * ep, char * buf, long  len) /* Internal Callback */
{
    if (self->title)  {
	free(self->title);
    }
    self->title = (char*) malloc(len+1);
    strncpy(self->title, buf, len);
    self->title[len]='\0';
}

static void
ChangeIndexable(class html * self, struct entityElement * ep, char * buf, long  len) /* Internal Callback */
{
    self->isindex = 1;
}


/* ------------------------------------------------------------------------ */
/* Play around with entity environments                                     */
/* ------------------------------------------------------------------------ */

/* getEntityCode: take a string, do a lookup and return the code */
static const struct entityMapping*
getEntityMapping(const char * string)
{
    const struct entityMapping* b;
    for (b = basicEntities; b->string; b++) {
	if (strcmp(b->string, string) == 0) {
	    return b;
	}
    }
    return &noEntity;
}

/* Take an entity off the stack */
static void 
popEntity(class html * self)
{
    struct entityElement* e = self->entities;
    if (e) {
	self->entities = e->prev;
	/* We don't destroy the sub objects, as these are now hooked into the text object... */
	free(e);
    }
}

/* Push an entity onto the stack. This should be called when parsing
 * It records the current position and prepares a style to plop down
 * when the end of the entity is known
 */
static struct entityElement*
pushEntity(class html * self, long * pos, const struct entityMapping * em, const char * name, char * vars, int  force)
{   
    const char* string;
    char buf[256];
    struct entityElement* e;
    class environment *env;

    e = (struct entityElement*) malloc(sizeof(struct entityElement));
    if (e) {
	/* adjust the list with the new entity, default values */
	e->prev  = self->entities;
	e->em    = em;
	if (pos) {
	    e->start = *pos;
	} else {
	    e->start = 0;
	}
	e->style = 0;
	e->data  = 0;
	strcpy(e->string, name);

	if (force >= 0) {
	    if (!withinEntity(self, entityHead) || force) {
		class style* thisStyle;
		/* We find the stylename no matter what, as it's useful */
		const struct styleMapping* sm = stylemap;
		for (; sm && sm->styleName && strcmp(sm->entityName, name)!=0; sm++) {
		    /* empty loop */;
				}
		if (sm->styleName) {
		    string = sm->styleName;
		} else {
		    string = name;
		}

		if (!(em->flags & entFlagsNoStyle)) {
		    thisStyle = ((self)->GetStyleSheet())->Find( string);

		    /* Get the thisStyle for this entity, make one up if it's bogus */
		    if (!thisStyle) {
			sprintf(errbuf, "could not load the style \"%s\". An error in the html template?", name);
			(self)->Inform( errbuf);
			/* If things go wrong, who you gunna call? */
			thisStyle = new style;
			(thisStyle)->SetName( name);
			(thisStyle)->AddAttribute( "color", "blue");
		    }

		    /* Create a new thisStyle, which is copy of the norm as handle for these specific attributes */
		    e->style = new style;
		    (thisStyle)->Copy( e->style);
		    /* We store all sorts of useful stuff attached to the style */
		    sprintf(buf, "%d", (em->flags | (int)em->code));
		    (e->style)->AddAttribute( styleHTMLCodes, buf);
		    if (vars && *vars) {
			addVars(e->style, vars);
		    }
		}

		if (em->code == entityRule) {
		    hrule(self, pos);
		} else if (em->code == entityImage) {
		    (self)->AddImage( pos, (e->style)->GetAttribute( "src"));
		    self->withinPar = 1;
		    (*pos)++;
		} else if (em->flags & entFlagsUnknown) {
		    /* Unknown entities: dump out the name of the entity, with spaces around it */
		    /* The spaces ensure that we get the style wrapping in the right order, by */
		    /* Frigging the start pos of this entity */
		    int len;
		    /* XXX: Should read the ALT variable and use that text if available */
		    sprintf(buf, " <%s> ", name);
		    len = strlen(buf);
		    (self)->AlwaysInsertCharacters( *pos, buf, len);
		    if (self->tagStyle) {
			env = (self)->AlwaysAddStyle( *pos+1, len-2, self->tagStyle);
			if (env) (env)->SetStyle( FALSE, FALSE);
		    }
		    self->withinPar = 1;
		    *pos += len;
		}
	    }
	}
    } else {
	(self)->Inform( "there's no memory left to parse this document.");
    }
    self->entities = e;
    return e;
}

static struct entityElement*
entityPeek(class html * self)
{
    return self->entities;
}

/* Run down the stack, returns TRUE if we can find the entityCode anywhere */
static struct entityElement*
withinEntity(class html * self, enum entityCodeEnum  code)
{
    struct entityElement* e;
    for (e = self->entities; e; e=e->prev) {
	if (e->em->code == code) {
	    return e;
	}
    }
    return 0;
}

/* Run down the stack, returns entity if we can find the entityCode anywhere */
static struct entityElement*
withinEntityClass(class html * self, enum entityCodeEnum  code)
{
    struct entityElement* e;
    for (e = self->entities; e; e=e->prev) {
	if ((int)e->em->code & (int)code) {
	    return e;
	}
    }
    return 0;
}

/* 
  Find first entity in buffer.  Return strlower(entity name) (vars) (pos) and
  delete the entity from the input buffer.
  Return value:

  If the return value signifies a token, then varPos is 
  set to the character position at which the token should take effect.
  Note: The following routine is destructive to buf.
 */
static int
html_FindEntity(char * buf, long * pos, char * entity, char * vars)
{
    char* s;
    /* .....<idXXXXXX varsXXXXXXX>.....  */
    /*      \         \          \       */
    /*       posStart  posVar     posEnd */

    char* posStart;
    char* posEnd; 
    char* posVar;
    *entity='\0';
    *pos = 0;

    if ((posStart = strchr(buf+*pos, '<'))) {
	/* We have at least a start token... */
	*pos = posStart - buf;
	if ((posEnd = strchr(posStart, '>'))) {
	    /* We have an end of a token */
	    /* token is buf[posStart..posEnd]) */
	    posVar= strpbrk(posStart, whiteSpace);
	    if (posVar && posVar < posEnd) {
		/* we have to break up entity name and variables */
		strncpy(entity, posStart+1, (posVar)-(posStart+1));
		/* Strncpy does not neccessarily null terminate! */
		entity[(posVar)-(posStart+1)] = '\0';
		strncpy(vars,   posVar+1, (posEnd)-(posVar+1));
		vars[(posEnd)-(posVar+1)] = '\0';
	    } else {
		strncpy(entity, posStart+1, (posEnd)-(posStart+1));
		entity[(posEnd)-(posStart+1)] = '\0';
		vars[0] = '\0';
	    }

	    for (s=entity;*s;s++) {	
		if (isupper(*s)) {
		    *s = tolower(*s);
		}
	    }
	    strcpy(posStart, posEnd+1); /* delete the token from the string */
	    return htmlFoundEntity;
	} else {
	    return htmlPartialEntity;
	}
    }
    return htmlNoEntity;
}

/* Call this when you find a '>' (endofentity) during read.
 * it takes the entity and plops a style into the text object
 * to represent the entity
 */
static void closeEntity(class html * self, struct entityElement * ep, long * pos, int  force)
{
    struct entityElement* e;
    class environment *env;
    if (ep == 0) {
	return;
    }
    ep->length = *pos - ep->start; 

    if (ep->style == 0) {
	return;
    }
    if (ep->em->code == entityListItem) {
	if ((e = withinEntityClass(self, entityList)) == 0) {
	    /* They've given us crap! */
	    /* XXX: Tell stupid user about it */
	    (self)->Inform( "a list tag was found with no enclosing list");
	    return;
	} else {
	    char buf[5];
	    switch((int)(e->em->code) & 0xff) {
		case entityEnumerate:
		    sprintf(buf, "%ld.\t", ++e->data);
		    break;
		case entityItemize:
		case entityMenu:
		case entityDirectory:
		default:
		    strcpy(buf, "*\t");
		    break;
	    }
	    if (!(self)->TagItem( ep->start, (*pos - ep->start)-1, buf, 0, 0)) {
		(self)->Inform( "An internal error occurred: the item could not be tagged");
	    }
	    *pos += strlen(buf);
	}
    } else {
	if (!withinEntity(self, entityHead) || force) {
	    env = (self)->AlwaysAddStyle( ep->start, (*pos - ep->start) ,ep->style);
	     /* So the next inserted characters aren't part of this style */
	    if(env) {
		(env)->SetStyle( FALSE, FALSE);
	    }
	}
    }
}

void
html::AddEntity(long  pos, long  len, const char * name, char * vars)
{   
    /* Map name to style */
    const struct entityMapping* em = getEntityMapping(name);
    if (em == 0) {
	return;
    }
    pushEntity(this, &pos, em, name, vars, 1);
    pos += len;
    closeEntity(this, entityPeek(this), &pos, 1);
    popEntity(this);
    (this)->RegionModified( pos, len);
    (this)->NotifyObservers( observable::OBJECTCHANGED);
}

/* ------------------------------------------------------------------------ */
/* The class maintenance methods                                            */
/* ------------------------------------------------------------------------ */

html::html()
{
	ATKinit;

    char tbuf[32];
    class style* is;

    this->lineNumber=0;
    (this)->SetCopyAsText( TRUE);
    (this)->SetExportEnvironments( FALSE);
    this->errorBuffer = 0;
    this->withinPar = 0;
    this->entities = 0;
    this->base = 0;
    this->title = 0;
    this->isindex = 0;
    if ((this)->ReadTemplate( "html", FALSE)) {
	(this)->Inform( "could not read the html template");
    }

    this->tagStyle = ((this)->GetStyleSheet())->Find("tag");
    if (!this->tagStyle) {
	this->tagStyle = new style;
	    (this->tagStyle)->SetName( "tag");
    }
    (this->tagStyle)->AddAttribute( "htmladornment", "true");
    sprintf(tbuf, "%d", entFlagsAdornment);
    (this->tagStyle)->AddAttribute( styleHTMLCodes, tbuf); 

    is = ((this)->GetStyleSheet())->Find("list-item");
    if (!is) {
	(this)->Inform( "The template was lacking a list-item, probably a problem with the template?");
	is = new style;
	(is)->SetName( "list-item");
    }
    this->itemStyle = new style;
    (is)->Copy( this->itemStyle);
    sprintf(tbuf, "%d", entFlagsReplace|entFlagsNewline|entityListItem);
    (this->itemStyle)->AddAttribute( styleHTMLCodes, tbuf); 

    THROWONFAILURE( TRUE);
}

html::~html()
{
	ATKinit;

    if (this->title) {
	free(this->title);
    }
    if (this->base) {
	free(this->base);
    }
    while (entityPeek(this)) {
	popEntity(this);
    }
}

void
html::Clear()
{
    (this)->text::Clear();
}

/* ------------------------------------------------------------------------ */
/* and actually displaying, reading and writing of HTML is done like so...  */
/* ------------------------------------------------------------------------ */

/*
 * Put text into the display buffer to be viewed 
 * The text will only be displayed if we are not within HEAD right now
 * Whitespace is optimised such that multiple whitespace is compressed to a 
 * single whitespace character.  All whitespace is mapped into ' ' (spaces).
 * Magic HTML thangs (such as "&lt;") are mapped into real characters.
 */
static void
maybeDisplay(class html * self, long * pos, char * buf, long * inlen)
{
    long realLen = *inlen;
    long len = *inlen;
    int justspaced = 0;

    char* s;

    char* rdPos;
    char* wrPos = buf+len;

    if (buf==0 || (withinEntity(self, entityHead)) || buf[0] == '\0' || len == 0) {
	return;
    }

    if (!self->withinPar) {
	justspaced = 1;
    }

    if (justspaced == 0 && *pos > 0) {
      justspaced = (strchr(whiteSpace, (self)->GetChar( (*pos)-1)) ? 1:0);
    }

    if (!withinEntity(self, entityPre)) {
	/* Run thru the string removing extraneous whitespace, and making 
	 * all whitespace appear as real spaces */
	for (rdPos = wrPos = buf; len && *rdPos; len--, rdPos++) {
	    if (strchr(whiteSpace, *rdPos)) {
		/* We're looking at whitespace */
		if (justspaced == 0) {
		    *wrPos++ = ' '; justspaced++;
		} /* else, ignore multiple whitespace */
	    } else {
		*wrPos++ = *rdPos;
		justspaced=0;
	    }
	}
    }
    /* We've possibly adjusted the length of the string */
    len = (int) (wrPos - buf);	

    /* Look for	magic '&' name ';' */    
    s = buf;
    while (s && len && (s = strchr(s, '&'))) {
	char* endtok = strchr(s, ';');
	const char* newstring;
	char c;
	if (s > (buf + len)) {
	    /* Past the limit */
	    break;
	}
	if (endtok) {
	    c = *endtok;
	    *endtok++='\0';
	    newstring = html_StringToMagic(s+1);
	    if (newstring != 0) {
		int x;
		/* We need to adjust length as we're about to replace chars */
		x = (strlen(s+1) + 2)-(strlen(newstring)) ;
		realLen -= x;
		len -= x -1; /* -1 coz we're adjusting it a few lines later */
		/* Put the new string in there and the rest of the buffer */
		sprintf(s, "%s%s", newstring, endtok);
	    } else {
		*(endtok-1) = c;
	    }
	}
	s++, len--;
    }
    *inlen = realLen;
    (self)->AlwaysInsertCharacters( *pos, buf, len);
    *pos += len;
    if (len) {
	self->withinPar = 1;
    }
}

/* Output a paragraph break or a small line break */
/* Calling this will only output newlines if the entityMapping indicates that it is a good time to do this */
static int
newpar(class html * self, const struct entityMapping * eMapping, long  pos) 
{
    char c;
    long xpos;
    int count = 0;

    if ((eMapping->flags & entFlagsNewline) && self->withinPar) {
	for (xpos=pos-1; xpos > 0 && (c=(self)->GetChar( xpos)) == '\n'; xpos--)
	{ count++;}

	if (eMapping->flags & entFlagsParagraph) {
	    /* Want to output a paragraph break */
	    if (count < 2) {
		(self)->AlwaysInsertCharacters( pos, "\n\n", 2-count);
		pos += 2-count;
	    }
	    self->withinPar = 0;
	} else {
	    /* Want to output a line break */
	    if (count == 0) {
		(self)->AlwaysInsertCharacters( pos, "\n", 1);
		pos++;
	    }
	}
    }
    return pos;
}

static void
hrule(class html * self, long * pos)
{
    if ((self)->GetChar( (*pos)-1) != '\n') {
	(self)->AlwaysInsertCharacters( *pos, "\n", 1);
	(*pos)++;
    }
    (self)->InsertObject( *pos, "bp", "bpv");
    (*pos)++;
    (self)->AlwaysInsertCharacters( *pos, "\n", 1);
    (*pos)++;
    (self)->NotifyObservers( 0);
}

void
html::AddImage(long * pos, char * file)	/*  */
{
    class image* dat;
    char* filename;
    class buffer* buf;
    static char defaultImage[256];

    buf = buffer::FindBufferByData(this);
    if (buf) {
	filename = findLocalFile(file, (buf)->GetFilename());
    } else {
	filename = file;
    }

    if ((dat = (class image*) ATK::NewObject("imageio"))) {
	if (filename) {
	    (dat)->Load( filename, NULL);
	} else {
	    /* This is all a bit of a frig to get something reasonable instead of a 200x200 blank */
	    if (!defaultImage[0]) {
		const char*s = environ::GetProfile("defaultImage");
		if (s) {
		    strcpy(defaultImage, s);
		}
	    }
	    if (defaultImage[0]) {
		(dat)->Load( defaultImage, NULL);
	    } else {
		/* XXX: What to do with the image? 
		image_Destroy(dat);
		dat = 0; */
	    }
	}
    }
    if (dat) {
	(this)->AlwaysAddView( *pos, "imagev", dat);
	(this)->NotifyObservers( 0);
    }
}

static int 
fixStyles(long  rock, class text * self, long  pos, class environment * curenv)
{
    if(curenv->type == environment_Style)
    {
        (curenv)->SetStyle( TRUE, TRUE);
    }

    /* FALSE means don't stop */
    return FALSE;
}


long
html::Read(FILE * file, long  id)
{
    (this)->ReadSubString( 0, file, 1);

    /* Set all the environments back with SetStyle to TRUE,
     * TRUE so that we behave like an editor
     */
    (this)->EnumerateEnvironments( 0, (this)->GetLength(), (text_eefptr) fixStyles, 1);
    return dataobject::NOREADERROR;
}

long
html::ReadSubString(long  startPos, FILE * file, int  quoteCharacters)
{
    char buf[1024];
    char entity[80]; /* The entity names are usually small. Badness */
    char vars[1024];
    long pos = 0;
    long myPos = startPos;
    char* s;
    const struct entityMapping* eMapping;
    struct entityElement* ep = NULL;

    buf[0]='\0';
    *vars = '\0';
    s = fgets(buf, 1024, file);
    this->lineNumber = 1;
    while (s) {
	switch (html_FindEntity(buf, &pos, entity, vars)) {
	    case htmlFoundEntity:
		/* Print out what we have so far */
		if (pos > 0) {
		    maybeDisplay(this, &myPos, buf, &pos);
		}
		if (*entity == '/') {
		    /* End of environment */
		    /* Close all replaceable entities within our body */
		    eMapping = getEntityMapping((entity+1));
		    ep = entityPeek(this);
		    while (ep && (ep->em->code != eMapping->code) &&
			   (ep->em->flags & entFlagsReplace)) {
			closeEntity(this, ep, &myPos, 0);
			popEntity(this);
			ep = entityPeek(this);
		    }
		    if (ep == 0) {
			sprintf(errbuf, "a %s close entity was found with no matching start entity", entity);
			(this)->Inform( errbuf);
			strcpy(buf, buf+pos);
			*vars = '\0';
			break;
		    }
		    if (ep->em->code != eMapping->code) {
			sprintf(errbuf, "unmatched environments: a <%s> follows <%s>\n", entity, ep->string);
			(this)->Inform( errbuf);
			/* Get rid of the text that we've printed */
			strcpy(buf, buf+pos);
			*vars = '\0';
			break;
		    } else {
			/* Valid start/end pair. Wrap an environment up */
			closeEntity(this, ep, &myPos, 0);
			if (ep->em->fn) {
			    ep->em->fn(this, ep, buf, pos);
			}
		        popEntity(this);
		        myPos = newpar(this, eMapping, myPos);
		    }
		} else {
		    eMapping = getEntityMapping(entity);

		    /* Start of environment */
		    myPos = newpar(this, eMapping, myPos);
		    if (eMapping->flags & entFlagsReplace) {
			ep = entityPeek(this);
			if (ep && ep->em->flags & entFlagsReplace) {
			    closeEntity(this, ep, &myPos, 0);
			    popEntity(this);
			    myPos = newpar(this, eMapping, myPos);
			}
		    }   
		    pushEntity(this, &myPos, eMapping, entity, vars, 0);
		}

		if (eMapping->flags & entFlagsSingle) {
		    /* The closeEntity get's optimised to nothing, but hey */
		    closeEntity(this, entityPeek(this), &myPos, 0);
		    if (eMapping->fn) {
			eMapping->fn(this, ep, buf, pos);
		    }
		    popEntity(this);
		    myPos = newpar(this, eMapping, myPos);
		}
		/* Get rid of the text that we've printed */
		strcpy(buf, buf+pos);
		*vars = '\0';
		break;
	    case htmlNoEntity:
		pos = strlen(buf);
		maybeDisplay(this, &myPos, buf, &pos);
		buf[0] = '\0';
		pos = 0;
		/* fall thru to get more */
	    case htmlPartialEntity:
		/* Append to end of current live data */
		this->lineNumber++; /* Well, this is only approximate, because the prev. fgets may be partial */
		s = fgets(buf+strlen(buf), 1024 - strlen(buf), file);
		break;
	}

    }
    return myPos-startPos;
}

static void
writeHeader(class html * self, FILE * file)
{
    fprintf(file, "<head>\n");
    if (self->title) {
	fprintf(file, "<title>%s</title>\n", self->title);
    }
    if (self->base) {
	fprintf(file, "<base href=%s>\n", self->base);
    }
    if (self->isindex) {
	fprintf(file, "<isindex>\n");
    }
    /* The nextid */
    /* The links.. ? */
    
    fprintf(file, "</head>\n\n");
}

/* Used by the write method to write some stuff */
static void 
PutsRange(class html * self, char  *p, FILE  *fp, char  *ep)
{
    if (!self->adornment) {
	while (p < ep)
	    putc(*p++, fp);
    }
}

long
html::Write(FILE * file, long  id, int  level)
{
    writeHeader(this, file);

    fprintf(file, "<body>\n");
    (this)->WriteSubString( 0, (this)->GetLength(), file, 1);
    fprintf(file, "\n</body>\n");
    return this->GetID();
}

/* Take a style and produce the html entity name which corresponds to it */
static const char*
getHTML(class style * style)
{
    char* s = (style)->GetAttribute( "html");
    if (s && *s) {
	return s;
    } else {
	/* Check our mapping table */
	const struct styleMapping* sm = stylemap;
	for (; sm && sm->styleName; sm++) {
	    if (strcmp(style->name, sm->styleName) == 0) {
		return sm->entityName;
	    }
	}
	return style->name;
    }
}

char*
html::EnvStart(char * outp, class style * style, int * parImply, int * brImply, int * newlines)
{
    const char* name;
    char* vars;
    char* s;
    const struct entityMapping* em;

    s = (style)->GetAttribute( "htmladornment");
    if (s) {
	/* XXX: This is a horrible hack and shouldn't be done like this. 
	 * It should be handled at a higher level, but it's too hard right
	 * now to grok the ATK text__write() code... 
	 */
	this->adornment++;
	return outp;
    }
    if (this->adornment) {
	return outp;
    }
		 
    /* A valid entity, let's put it on the stack, so we can keep tabs... */
    name = getHTML(style);
    vars = html_StyleToVariables(style);
    em = getEntityMapping(name);
    pushEntity(this, 0, em, name, vars, -1);

    if (em->flags & entFlagsParagraph) {
	*parImply = 1;
    }
    if (em->flags & entFlagsNewline) {
	*brImply = 1;
    }
    outp = outputNewlines(*newlines, *parImply, *brImply, outp);
    *newlines = 0;
    *parImply = 0;
    *brImply = 0;

    if (em->flags & entFlagsNewline) {
	*outp++ = '\n';
    }
    if (em->flags & entFlagsParagraph) {
	*outp++ = '\n';
    }
    /* And now dump it out */
    *outp++ = '<';
    while (*name) {
	*outp++ = *name;
	name++;
    }
    while (*vars) {
	*outp++ = *vars;
	vars++;
    }
    *outp++ = '>';
    return outp;
}

char*
html::EnvEnd(char * outp, class style * style, int * parImply, int * brImply)
{
    const char* temp;
    const char* s;
    int code;

    if ((s = (style)->GetAttribute( "htmladornment"))) {
	/* XXX: It's that hack again */
	this->adornment--;
	return outp;
    }
    if (this->adornment) {
	return outp;
    }
    popEntity(this); /* XXX: Should verify this matched the peek */
    code = html_StyleToCodes(style);
    if (code & entFlagsParagraph) {
	*parImply = 1;
    }
    if (code & entFlagsNewline) {
	*brImply = 1;
    }

    if (code & (entFlagsSingle|entFlagsReplace)) {
	return outp;
    }
    s = getHTML(style);
    *outp++ = '<';
    *outp++ = '/';
    for(temp = s; *temp; temp++){
	*outp++ = *temp;
    }
    *outp++ = '>';
    if (code & (entFlagsNewline|entFlagsParagraph)) {
	*outp++ = '\n';
    }
    return outp;
}

static char*
outputNewlines(int  newlines, int  parImplied, int  brImplied, char * outp)
{
    const char* temp;
    if (newlines >= 2) {
	if (!parImplied) {
	    for(temp = parBreakString; *temp; temp++){
		*outp++ = *temp;
	    }
	}
	newlines = 0;
    }			    
    parImplied = 0;
    if (newlines == 1) {
	if (!parImplied && !brImplied) {
	    for (temp = breakString; *temp; temp++) {
		*outp++ = *temp;
	    }
	}
	newlines = 0;
    }   
    return outp;
}

void
html::WriteSubString(long  pos, long  len, FILE * file, int  quoteCharacters)
{
    class environment* rootenv;
    class environment *startenv;
    class environment *endenv;
    class environment *curenv;
    class environment *newenv;
    class environment *parentenv;

    int lastblankset = TRUE; /* IF last character was whitespace */
    /* The following two are set if an explicit <p> or <br> is detected, OR
      * if an implied one (e.g. from a title, or somesuch) is detected.
      */
    int parImplied = 0; /* IF parbreak implied from previous style */
    int brImplied  = 0; /* IF  a line break has been implied by the last style */
    int newlines = 0;

    long end;
    long i;
    long elen;
    int levels;
    char c;
    long envpos;

    char outbuf[180],*outp,*endp;
    char *buf = NULL;
    long bufLen;

    /* Quicky optimisation */
    if (len <= 0 ) return;
    this->lineNumber = 0; /* In case any errors are produced during a write. Can it happen? */

    /* fprintf(stderr, "WriteSubString(%ld, %ld)\n", pos, len); */
    /* Make sure we start in the right state... */
    /* This should be set by looking at curenv(pos) */
    this->adornment = 0;
      
    endp = outbuf + 78; /* Hackety hack */
    outp = outbuf;

    startenv = (class environment *)(this->text::rootEnvironment)->GetInnerMost( pos);
    endenv = (class environment *)(this->text::rootEnvironment)->GetInnerMost( pos + len - 1);
    rootenv = (class environment *)(startenv)->GetCommonParent( endenv);

    /* Determine the root of the environment, by going back up the tree */
    for (envpos = (rootenv)->Eval();
	 pos == envpos && pos + len == envpos + rootenv->nestedmark::length;
	 rootenv = (class environment *) rootenv->nestedmark::parent,
	 envpos = (rootenv)->Eval());

    /* Now open all starting environments up */
    curenv = rootenv;
    while (curenv != startenv)  {
	curenv = (class environment *)(curenv)->GetChild( pos);
	/* Make sure we write out buffer if it gets too full */
	if ((curenv->type == environment_Style && 
	     (outp + 2 + strlen(curenv->data.style->name) > endp))
	    || (curenv->type == environment_View && outp != outbuf)){
	    PutsRange(this, outbuf, file, outp);
	    outp = outbuf;
	}
	/* The actual open environment */
	if (curenv->type == environment_Style){
	    outp = (this)->EnvStart( outp, curenv->data.style, &parImplied, &brImplied, &newlines);
	}
    }

    /* Now, we go through the document looking at each environment */
    i = pos;
    end = pos + len;
    while (i < end)  {
        newenv = (class environment *)(this->text::rootEnvironment)->GetInnerMost( i);
        elen = (this->text::rootEnvironment)->GetNextChange( i);
        if (elen + i > end)
            elen = end - i;
        if (curenv != newenv)  {
	    /* A new environment, so make sure previous ones are closed */
            parentenv = (class environment *)(curenv)->GetCommonParent( newenv);
            levels = (curenv)->Distance( parentenv);
            while (levels > 0)  {
		if (curenv->type == environment_Style) {
		    outp = (this)->EnvEnd( outp, curenv->data.style, &parImplied, &brImplied);
		}
		curenv = (class environment*)(curenv)->GetParent();
                levels--;
            }  
            curenv = parentenv;
	    /* Now we're in a new environment, curenv is the parent? */
            if (curenv != newenv)  {
                class environment *stack[100];
                class environment *te;
                int level = 0;

		/* Open all of the environments that start at this spot */
                for (te = newenv; te != parentenv; te = (class environment *) te->nestedmark::parent)
                    stack[level++] = te;
                while (level > 0)  {
                    curenv = stack[--level];
                    if ((curenv->type == environment_Style && 
                          (outp + 2 + strlen(curenv->data.style->name) > endp))
                         || (curenv->type == environment_View && outp != outbuf)){
			PutsRange(this, outbuf, file, outp);
			outp = outbuf;
                    }
                    if (curenv->type == environment_Style){
			outp = (this)->EnvStart( outp, curenv->data.style, &parImplied, &brImplied, &newlines);
                    }
                }
            }
        }
        if (curenv->type == environment_View)  {
	    /* Views (inner sub-objects) are ignored */
            if(outp != outbuf){
                /* flush out remaining cached characters */
		PutsRange(this, outbuf, file, outp);
		outp = outbuf;
            }
	    i += curenv->nestedmark::length;
	    elen = 0;
	}
        elen += i;

        bufLen = 0;
	if (this->adornment) {
	    i = elen; /* Jump to end of text */
	}
        while (i < elen)  {
            /* Code for writing out actual text */
            if (bufLen == 0) {
                buf = (this)->GetBuf( i, 1024, &bufLen);
	    }
            bufLen--, c = *buf++;

	    if (strchr(HTMLMagicCharacters, c)) {
		char* s;
		int len;
		s = html_MagicToString(buf-1,&len); /* tjm - probably fails if i is close to 1024 */
		while (s && *s) {
		    *outp++ = *s++;
		}
		buf+=len-1;
		lastblankset = FALSE;
	    } else {
		switch(c) {
		case '\n':
		    /* Make sure pre-formatted doesn't get munched. */
		    if (withinEntity(this, entityPre)) {
			*outp++ = c;
		    } else {
		        newlines++;
		    }
		    break;
		default:
		    /* This is a normal character */
		    /* However, multiple spaces get shrunk... unless PRE */
		    if (withinEntity(this, entityPre)) {
			*outp++ = c;
		    } else {
			outp = outputNewlines(newlines, parImplied, brImplied, outp);
			newlines = 0;
			parImplied = 0;
			brImplied = 0;

			if(c == ' ' || c == '\t' || c == '\n'){
			    if(lastblankset == FALSE){
				*outp++ = ' ';
				lastblankset = TRUE;
			    }
			} else {
			    *outp++ = c;
			    lastblankset = FALSE;
			}
		    }
		}
	    }

	    /* Buffer management */
	    if (outp > endp) {
		PutsRange(this, outbuf, file, outp);
		outp = outbuf;
            }
            i++;
        }
    }

    /* flush out cached characters */
    if(outp != outbuf){
        PutsRange(this, outbuf,file,outp);
    }
    /* And close all remaining environments */
    levels = (curenv)->Distance( rootenv);
    while (levels-- > 0) {
	if (curenv->type == environment_Style) {
	    outp = (this)->EnvEnd( outbuf, curenv->data.style, &parImplied, &brImplied);
	}
	curenv = (class environment*)(curenv)->GetParent();
	PutsRange(this, outbuf, file, outp);
    }
}


/* Make the (pos,len) region a list item, with a tagged bit inserted at the start */
boolean
html::TagItem(long  pos , long  len, const char * text, const char * itemS, class style * extraStyle)
{
    class environment *env;
    int tlen = strlen(text);
    class style* iStyle = this->itemStyle;

    if (itemS) {
	iStyle = ((this)->GetStyleSheet())->Find(itemS);
    }

    if (!(this->tagStyle && this->itemStyle)) {
	return FALSE;
    }
    (this)->AlwaysInsertCharacters( pos, text, tlen);
    if (extraStyle) {
	env = (this)->AlwaysAddStyle( pos, tlen, extraStyle);
	if (env) {
	    (env)->SetStyle( FALSE, FALSE);
	}
    }
    env = (this)->AlwaysAddStyle( pos, tlen, this->tagStyle);
    if (env) {
	(env)->SetStyle( FALSE, FALSE);
    }
    env = (this)->AlwaysAddStyle( pos, len+tlen, iStyle);
    if (env) {
	(env)->SetStyle( FALSE, FALSE);
    }
    (this)->RegionModified( pos, len+tlen);
    (this)->NotifyObservers( observable::OBJECTCHANGED);
    return TRUE;
}

static int
html_StyleToCodes(class style * style)
{
    char* s = (style)->GetAttribute( styleHTMLCodes);
    if (!s) {
	return 0;
    }

    return atoi(s);
}

/* Look at pos and untag the paragraph, getting rid of tag and styles */
int
html::UntagItem(long  pos)
{
    class environment* env;
    class environment* item;
    class environment* list;
    class environment* enext;
    int itempos, itemlen;
    int code;
    long len;

    /* Find the tag */
    enext = (class environment *)(this->text::rootEnvironment)->GetInnerMost( pos);

    do {
	env = enext;
	while (env && env->type != environment_Style){
	    env = (class environment*)(env)->GetParent();
	}
	if (!env) {
	    return 0;
	}
	code = html_StyleToCodes(env->data.style);
	if (code & (int)entityList) {
	    /* We couldn't find the item before we left the list */
	    return 0; 
	}
	enext = (class environment*)(env)->GetParent();
    } while (!(code & entFlagsAdornment));

    /* Find the item */
    do {
	item = enext;
	while (item && item->type != environment_Style){
	    item = (class environment*)(item)->GetParent();
	}
	if (!item) {
	    return 0;
	}
	code = html_StyleToCodes(item->data.style);
	if (code & (int)entityList) {
	    /* We couldn't find the item before we left the list */
	    return 0; 
	}
	enext = (class environment*)(item)->GetParent();
    } while ((code & 0xff) != (int)entityListItem);

    /* Find the list */
    do {
	list = enext;
	while (list && list->type != environment_Style){
	    list = (class environment*)(list)->GetParent();
	}
	if (!list) {
	    return 0;
	}
	code = html_StyleToCodes(list->data.style);
	enext = (class environment*)(list)->GetParent();
    } while (!(code & (int)entityList));

    /* Delete the list-item style */
    itempos = (item)->Eval();
    itemlen = (item)->GetLength();
    (item)->Delete();
    /* delete the tag. This will automatically remove the adorning style */
    len = (env)->GetLength();
    (this)->AlwaysDeleteCharacters( (env)->Eval(), len);
    /* Delete the list style */
    (this->text::rootEnvironment)->Remove( itempos, itemlen-len+1, environment_Style, FALSE);

    /* Tell people about it */
    (this)->RegionModified( itempos, itemlen);
    (this)->NotifyObservers( observable::OBJECTCHANGED);
    return len;
}


class environment*
html::GetEntityEnvironment(long  pos, class environment * env)
{
    class environment* e, *parent;
    char* s;

    if (env) {
	e = env;
    } else {
	e = (class environment *)(this->text::rootEnvironment)->GetInnerMost( pos);
    }

    while(e) {
	/* Check if this is the one. */
	if (e->type == environment_Style && (s = (this)->GetAttribute( e, "htmladornment")) == 0) {
	    break;
	}

	parent = (class environment*)(e)->GetParent();
	e = parent;
    };

    return e;
}

/*
 * Find a file by looking either at the relative file passed in as
 * second parameter, or by looking along a list of pseudo-roots
 */
static char*
findLocalFile(char * path, char * relativeRoot)
{
    static char buf[256];
    const char* s;
    const char* ptr;

    if (*path == '/') {
	ptr = environ::GetProfile("webPath");
	while (ptr) {
	    s = strchr(ptr, ',');
	    if (s) {
		strncpy(buf, ptr, s-ptr);
		buf[s-ptr] = '\0';
		ptr = s+1;
	    } else {
		strcpy(buf, ptr);
		ptr = 0;
	    }
	    strcat(buf, path);
	    /* Check buf */
	    if (access(buf, R_OK) == 0) {
		return buf;
	    }
	}
	return 0;
    } else {
	/* Relative pathname, use current buffer */
	char *sl;
	strcpy(buf, relativeRoot);
	if ((sl = strrchr(buf, '/'))) {
	    sl[1] = '\0';
	} else {
	    /* Bizarreness happening! */
	    return 0;
	}
	strcat(buf, path);
	if (access(buf, R_OK) == 0) {
	    return buf;
	}
	return 0;
    }
}

