/* File attlist.C created by Ward Nelson
   (c) Copyright IBM Corp 1995.  All rights reserved.   */

#include <andrewos.h>
#include "attlist.H"

#define saferealloc(A,B) ((A==NULL)?malloc(B):realloc(A,B))

static char *attributes;		/* used for efficiency. */
static long attributesLength;

ATKdefineRegistry(attlist, list, attlist::InitializeClass);

boolean attlist::InitializeClass()
{
    attributes = NULL;
    attributesLength = 0;
    return TRUE;
}

attlist::attlist()
{
    ATKinit;
    refs=1;
    THROWONFAILURE(TRUE);
}

attlist::~attlist()
{
    ATKinit;
    ClearAttributes();
    return;
}

static boolean Print(char *attsneak, char *rock)
{
    struct htmlatts *att= (struct htmlatts *)attsneak;
    printf("[%p] name = [%s], value = [%s], quoted = [%d]\n", att, att->name, att->value, att->quoted);
    return TRUE;
}

void attlist::DebugPrint()
{
    fprintf(stderr, "\nattlist__DebugPrint: start\n");
    Enumerate(Print, NULL);
    fprintf(stderr, "attlist__DebugPrint: end\n");
}


void attlist::AddAttribute(const char *name, const char *value, boolean quoted)
{
    struct htmlatts *newnode = NULL;
    int namelen=0, vallen=0;

    if (!name || (namelen = strlen(name)) <= 0) return;		/* do nothing */
    if (!value || (vallen = strlen(value)) <= 0) {vallen = 0; value="";}

    newnode = (struct htmlatts *) malloc (sizeof(struct htmlatts));
    newnode->name = (char *) malloc (namelen + 1);
    newnode->value = (char *) malloc (vallen + 1);
    strcpy((char *)newnode->name, name);
    strcpy((char *)newnode->value, value);
    newnode->quoted = quoted;

    Enqueue((char *)newnode);
}

static int IsAttrib(char *attsneak, char *name)
{
    struct htmlatts *attrib= (struct htmlatts *)attsneak;
    return (cistrcmp(attrib->name, name));
}

/* will return NULL if the attribute was not found */
struct htmlatts *attlist::GetAttribute(const char *name)
{
    return (struct htmlatts *) Enumerate(IsAttrib, name);
}

char *skipwhitespace(const char *s)
{
    while (*s && isspace(*s)) ++s;
    return (char *)s;
}

char *skiptokenchars(const char *s)
{
    while (*s && !isspace(*s) && *s != '=') ++s;
    return (char *)s;
}

char *skipstring(const char *s)
{
    while (*s && *s != '"') ++s;
    return (char *)s;
}

void attlist::MakeFromString(const char *str)
{
    char *end, *name, *value, *copy;
    boolean quoted, equalsign=FALSE;

    if (!str) return;

    copy = (char *) malloc (strlen(str) + 1);
    strcpy(copy, str);

    name = end = copy;
    while (*name) {
	value = NULL; equalsign = quoted = FALSE;
	name = skipwhitespace(end);
	if (*name == '\0') break;

	end = skiptokenchars(name);
	equalsign = (*end == '=');

	if (*end != '\0') {*end = 0; ++end; }	/* make sure variable end doesn't go past the end of the string */
	value = skipwhitespace(end);	/* if end was zero this will also be zero and won't go though the next if */

	if (equalsign || *value == '=') {
	    if (!equalsign) value = skipwhitespace(value + 1);
	    if (*value == '"') {
		++value;			/* if starts with quote, don't include it */
		quoted = TRUE;
		end = skipstring(value);
	    } else end=value;
	    if (*value != '\0') {
		end = skiptokenchars(end);
		if (*(end-1) == '"') --end;			/* if ends with quote, don't include it */
		if (*end != '\0') {*end = 0; ++end;} /* make sure variable end doesn't go past the end of the string */
	    } else {value = NULL;}
	} else { value = NULL; }

	AddAttribute(name, value, quoted);
    }
    free(copy);
}

/* The return value is a pointer to a static char *.  Therefore, the return value of this should be used right away.  If a copy is needed, copy the return value.  DON'T just keep a pointer to it.  */

char *attlist::MakeIntoString()
{
    long len;
    struct htmlatts *temp;

    /* let's find out how much space we need */
    len = 1;	/* start out at 1 to account for the end of string char */
    Start();
    while (temp=(struct htmlatts *) Data()) {
	if (temp->value && strlen(temp->value) > 0)
	    len += strlen(temp->name) + strlen(temp->value) +2/* space and = */ +(2*!!temp->quoted)/* quotes */;
	else
	    len += strlen(temp->name) +1/* space */;
	Advance();
    }

    /* adjust the space if necessary */
    if (attributesLength<len) {
	attributesLength= len+32; /* add a little extra so we won't have to realloc so often */
	attributes = (char *)saferealloc(attributes, attributesLength);
    }

    /* fill the tag */
    attributes[0] = '\0';
    Start();
    while (temp=(struct htmlatts *)Data()) {
	if (attributes[0]) strcat(attributes, " ");
	strcat(attributes, temp->name);
	if (temp->value && strlen(temp->value) > 0) {
	    strcat(attributes, (temp->quoted)?"=\"":"=");
	    strcat(attributes, temp->value);
	    if (temp->quoted) strcat(attributes, "\"");
	}
	Advance();
    }
    return attributes;
}


boolean ClearAtts(char *attsneak, char *rock)
{
    struct htmlatts *attrib= (struct htmlatts *)attsneak;
    if (attrib->name) free((char *)attrib->name);
    if (attrib->value) free((char *)attrib->value);
    free(attrib);
    return TRUE;
}

void attlist::ClearAttributes()
{
    Enumerate(ClearAtts, NULL);
    Clear();
}

static boolean CopyAtts(char *attsneak, char *newlistsneak)
{
    struct htmlatts *attrib= (struct htmlatts *)attsneak;
    attlist *newlist= (attlist *)newlistsneak;
    (newlist)->AddAttribute(attrib->name, attrib->value, attrib->quoted);
    return TRUE;
}

attlist *attlist::CopyAttributes()
{
    attlist *newlist = new attlist;
    Enumerate(CopyAtts, (char *)newlist);
    return newlist;
}


void attlist::Copy(attlist *dst, attlist *src)
{
    ATKinit;
    (src)->Enumerate(CopyAtts, (char *)dst);
}

attlist *attlist::ParseAttributes(const char *str)
{
    ATKinit;
    attlist *alist = new attlist;
    (alist)->MakeFromString(str);
    return alist;
}

void attlist::Destroy() {
    if(refs>0) {
	refs--;
	if(refs==0) delete this;
    }
}
