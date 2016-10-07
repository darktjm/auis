/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

#include <andrewos.h>
ATK_IMPL("box.H")
#include <stdio.h>
#include <assert.h>
#include <ctype.h>
#include <dataobject.H>
#include <rect.h>
#include <view.H>
#include <graphic.H>

#include <box.H>

static boolean debug=FALSE;

#define classname(do) ((do) == NULL ? "<NO OBJECT>" : (do)->GetTypeName())
#define safename(c) ((c) == NULL ? "<NULL DATA>" : classname(c->data))

#define SafeFromNull(s) (((s) != NULL) ? (s) : "(null)")

#define getDataObject(self) (*(self))


/* initialize entire class */


ATKdefineRegistry(box, dataobject, box::InitializeClass);
static void objectto(FILE  *f				/* input file containing offending material */, const char  *message				/* error message */);
static boolean			    /* returns TRUE for success */ fgetstring(FILE  *f			    /* input file */, const char  *string			    /* desired input string */);
static long			    /* returns read error status */ readASCII(class box  *self, FILE  *f			    /* input file */, long  id			    /* unique identifier in data stream */);


boolean
box::InitializeClass()
{
    if (debug)
	printf("box_InitializeClass()\n");

    return TRUE;
}

/* get corresponding view name */

const char *					/* returns "boxview */
box::ViewName()
{
    return "boxview";
}

/* Initialize new data box */

box::box()
{
	ATKinit;


    if (debug)
	printf("box_InitializeObject\n");

    this->inGetModified = FALSE;
    this->contents = NULL;
    THROWONFAILURE( TRUE);
}

/* tear down a box */

box::~box()
{
	ATKinit;

    if (debug)
	printf("box_FinalizeObject\n");
}

/* toggle debugging flag */

void
box::ToggleDebug()
{
    if (debug) {
	printf("box debugging off\n");
	debug = 0;
    } else {
	debug = 1;
	printf("box debugging on - ESC-ESC turns it off\n");
    }
}

/* write box to file */

long					/* returns id of object written */
box::Write(FILE  * f				/* file to be written */, long  writeID				/* unique ID of object in output file */, int  level				/* nesting level */)
{

    if (debug)
	printf("box_Write(%ld, %d)\n", writeID, level);

    if (getDataObject(this).GetWriteID() != writeID) {
	getDataObject(this).SetWriteID(writeID);
	fprintf (f, "\\begindata{%s,%ld}\n", (this)->GetTypeName(), (this)->GetID());
	if (this->contents != NULL) {
	    if (debug)
		printf("Writing out %s\n", (this->contents)->GetTypeName());
	    (this->contents)->Write ( f, (this)->dataobject::GetWriteID(), level + 1);
	}
	fprintf (f, "\\enddata{%s,%ld}\n", (this)->GetTypeName(), (this)->GetID());
    }
    return (this)->GetID();
}

/* object to and print out bad input */

static void
objectto(FILE  *f				/* input file containing offending material */, const char  *message				/* error message */)
{
    int ch;

    printf("%s:  \"", message);
    for (;;) {
	ch = getc(f);
	if (ch == '\n' || ch == EOF || ch == '\0')
	    break;
	putchar(ch);
    }
    printf("\"\n");
}

/* scan input for a specific string */

static boolean			    /* returns TRUE for success */
fgetstring(FILE  *f			    /* input file */, const char  *string			    /* desired input string */)
{
    int ch;

    ch = getc(f);
    while (ch == *string && *string != '\0') {
	ch = getc(f);
	string++;
    }
    ungetc(ch, f);
    if (*string != '\0')
	return TRUE;
    else
	return FALSE;
}

/* read ASCII representation of a box */

/*
   box data stream:

The box data stream follows ATK standards, starting with a begindata and
ending with an enddata directive.  In between comes the contained object.

*/

static long			    /* returns read error status */
readASCII(class box  *self, FILE  *f			    /* input file */, long  id			    /* unique identifier in data stream */)
{
    int ch;
    char dataname[256];
    char *np;
    long uniqueID;

    while ((ch = getc(f)) != EOF) {
	if (ch == 0) {
	    printf("stopped on zero character");
	    return dataobject::BADFORMAT;
	}
	if (ch != '\\') {
	    objectto(f, "box:  missing begindata or enddata\n");
	    return dataobject::BADFORMAT;
	}

	ch = getc(f);
	ungetc(ch, f);
	if (ch == 'b') {	    /* begindata coming */

	    if (fgetstring(f, "begindata{") != 0) {
		objectto(f, "box:  missing begindata");
		return dataobject::BADFORMAT;
	    }
	    for (np = dataname; np < dataname + sizeof dataname - 1 && (ch = getc(f)) != EOF && ch != ','; np++)
		*np = ch;
	    *np = '\0';
	    if (fscanf(f, "%ld ", &uniqueID) != 1) {
		objectto(f, "box:  missing , after component name");
		return dataobject::BADFORMAT;
	    }
	    if (fgetstring(f, "}") != 0) {
		objectto(f, "box:  missing closing brace after begindata");
		return dataobject::BADFORMAT;
	    }
	    if (fgetstring(f, "\n") != 0)
		objectto(f, "box:  extra stuff after begindata");
	    self->contents = (class dataobject *)ATK::NewObject(dataname);
	    if (self->contents == NULL) {
		printf("Could not create %s object.\n", dataname);
		return dataobject::OBJECTCREATIONFAILED;
	    }
	    self->contents->SetID(uniqueID);
	    (self->contents)->Read( f, uniqueID);
	    (self)->SetModified();
	    (self)->NotifyObservers( observable::OBJECTCHANGED);

	    if (debug)
		printf("done adding component\n");
	}

	else if (ch == 'e')	{   /* enddata coming */
	    if (fscanf(f, "enddata{%255[^,}\n],%ld}\n", dataname, &uniqueID) != 2) {
		objectto(f, "box:  expected enddata or another component");
		return dataobject::BADFORMAT;
	    }
	    else if (strcmp(dataname, (self)->GetTypeName()) != 0) {
		objectto(f, "box: wrong data name in enddata");
		return dataobject::BADFORMAT;
	    }
	    else if (uniqueID != id) {
		objectto(f, "box:  wrong unique ID in enddata");
		return dataobject::BADFORMAT;
	    }
	    return dataobject::NOREADERROR;
	}

	else {
	    objectto(f, "box:  bad input line");
	    return dataobject::BADFORMAT;
	}
    } /* end of reading loop */
    printf("box:  premature EOF");
    return dataobject::PREMATUREEOF;
}

/* read box from file */

long				    /* returns read error status */
box::Read(FILE  * f			    /* input file */, long  id			    /* unique identifier in data stream */)
{
    long rc;

    if (debug)
	printf("box_Read(%ld)\n", id);

    (this)->SetModified();

    rc = readASCII(this, f, id);
    (this)->NotifyObservers( observable::OBJECTCHANGED);
    if (debug)
	printf("box_Read rc = %ld\n", rc);

    return rc;
}

/* check to see if modified */

long
box::GetModified()
{
    long rc, cc;

    rc = (this)->dataobject::GetModified();
    if (!this->inGetModified && this->contents != NULL) {
	this->inGetModified = TRUE;
	cc = (this->contents)->GetModified();
	if (cc > rc) rc = cc;
	this->inGetModified = FALSE;
    }

    if (debug)
	printf("box_GetModified = %ld\n", rc);

    return rc;
}

/* fill in contents */

void
box::FillInContents(const char  *name)
{
    class dataobject *newobject;

    if (debug)
	printf("box_FillInContents(%s)\n", name);

    newobject = (class dataobject *) ATK::NewObject(name);
    if (newobject != NULL) {
	if (this->contents != NULL)
	    (this->contents)->Destroy();
	this->contents = newobject;
    }
    (this)->SetModified();
    (this)->NotifyObservers( observable::OBJECTCHANGED);
}
