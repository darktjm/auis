/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

#include <andrewos.h>
ATK_IMPL("layout.H")
#include <stdio.h>
#include <assert.h>


#include <ctype.h>
#include <dataobject.H>
#include <view.H>
#include <graphic.H>

#include <layout.H>

#define SafeFromNull(s) (((s) != NULL) ? (s) : "(null)")

#define getDataObject(self) (*(self))

static boolean debug=FALSE;

#define classname(do) ((do) == NULL ? "<NO OBJECT>" : (do)->GetTypeName())
#define safename(c) ((c) == NULL ? "<NULL DATA>" : classname(cData(c)))
#undef MIN
#undef MAX
#define MIN(x, y) ((x) < (y) ? (x) : (y))
#define MAX(x, y) ((x) > (y) ? (x) : (y))

#define	MINSIZE	5	    /* minimum width/height resulting from constraints */

/* initialize entire class */


ATKdefineRegistry(layout, dataobject, layout::InitializeClass);
static void objectto(FILE  *f			    /* input file containing offending material */, const char  *message			    /* error message */);
static boolean			    /* returns TRUE for success */ fgetstring(FILE  *f			    /* input file */, const char  *string			    /* desired input string */);
static long			    /* returns read error status */ readASCII(class layout  *self, FILE  *f			    /* input file */, long  id			    /* unique identifier in data stream */);


boolean layout::InitializeClass()
{

    return TRUE;
}

/* get corresponding view name */

const char *					/* returns "layoutview */
layout::ViewName()
{
    return "layoutview";
}

/* Initialize new data layout */

layout::layout()
{
	ATKinit;


    this->firstcomponent = NULL;
    this->inGetModified = FALSE;
    THROWONFAILURE( TRUE);
}

/* tear down a layout */

layout::~layout()
{
	ATKinit;

}

/* toggle debugging flag */

void layout::ToggleDebug()
{
    if (debug) {
	printf("layout debugging off\n");
	debug = 0;
    } else {
	debug = 1;
	printf("layout debugging on - ESC-ESC turns it off\n");
    }
}

/* write layout to file */

long					/* returns id of object written */
layout::Write(FILE  * f				/* file to be written */, long  writeID				/* unique ID of object in output file */, int  level				/* nesting level */)
{
    struct component *c;

    if (debug)
	printf("layout_Write(%ld, %d)\n", writeID, level);

    if (getDataObject(this).GetWriteID() != writeID) {
	getDataObject(this).SetWriteID(writeID);
	fprintf (f, "\\begindata{%s,%ld}\n", (this)->GetTypeName(), (this)->GetID());
	forallcomponents(this, c) {
	    fprintf(f, "<%ld,%ld,%ld,%ld", cLeft(c), cTop(c), cWidth(c), cHeight(c));
	    if (cData(c) != NULL) {
		fprintf(f, ",");
		if (cVaries(c))
		    fprintf(f, "V");
		else
		    fprintf(f, "F");
		fprintf(f, ">");
		fprintf(f, "\n");
		if (debug)
		    printf("Writing out %s\n", (cData(c))->GetTypeName());
		(cData(c))->Write ( f, (this)->dataobject::GetWriteID(), level + 1);
	    }
	    else
		fprintf(f, ">\n");  /* no child present */
	}
	fprintf (f, "\\enddata{%s,%ld}\n", (this)->GetTypeName(), (this)->GetID());
    }
    return (this)->GetID();
}

/* object to and print out bad input */

static void objectto(FILE  *f			    /* input file containing offending material */, const char  *message			    /* error message */)
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

/* read ASCII representation of a layout */

/*
   Layout data stream:

The layout data stream follows ATK standards, starting with a begindata and
ending with an enddata directive.  In between comes a sequence of components,
ordered front to back.  Each component starts with a line of the form:

<left,top,width,height,properties>

where the first four are decimal numbers giving the geometry of the component's
rectangle, and the fifth is a set of letters giving specific properites:

  V or F for variable or fixed components (* is an acceptable substitute for V)

This line is followed by the ATK representation for the component object.
A null component is represented by omitting the fifth field and data entirely.

*/


static long			    /* returns read error status */
readASCII(class layout  *self, FILE  *f			    /* input file */, long  id			    /* unique identifier in data stream */)
{
    int ch;
    struct component *c;
    char dataname[256];
    char *np;
    long uniqueID;
    boolean havechild;
    long left, top, width, height;

    while ((ch = getc(f)) != EOF) {
	switch(ch) {

	    case '\0':
		printf("stopped on zero character");
		return dataobject::BADFORMAT;

	    case '<': /* another component coming */
		c = (self)->CreateComponent();
		if (fscanf(f, "%ld,%ld,%ld,%ld", &left, &top, &width, &height) != 4) {
		    objectto(f, "layout:  expected four numbers separated by commas");
		    (self)->RemoveComponent( c);
		    return dataobject::BADFORMAT;
		}
		(self)->SetComponentSize( c, left, top, width, height);
		c->varies = TRUE;
		ch = getc(f);
		havechild = FALSE;
		if (ch != ',')
		    ungetc(ch, f);
		else {
		    havechild = TRUE;
		    for (;;) {
			ch = getc(f);
			if (ch == '>')
			    break;
			else if (ch == '*' || ch == 'V')
			    c->varies = TRUE;
			else if (ch == 'F')
			    c->varies = FALSE;
			else {
			    ungetc(ch, f);
			    objectto(f, "layout:  unknown property or > missing");
			    (self)->RemoveComponent( c);
			    return dataobject::BADFORMAT;
			}
		    };
		}
		if (debug)
		    printf("Got <%ld,%ld,%ld,%ld,%c> havechild=%d\n", cLeft(c), cTop(c), cWidth(c), cHeight(c), (cVaries(c) ? 'V' : 'F'), havechild);
		if (fgetstring(f, "\n") != 0) {
		    objectto(f, "layout:  trash after coordinates");
		    (self)->RemoveComponent( c);
		    return dataobject::BADFORMAT;
		}

		if (havechild) {
		    if (fgetstring(f, "\\begindata{") != 0) {
			objectto(f, "layout:  missing component begindata");
			(self)->RemoveComponent( c);
			return dataobject::BADFORMAT;
		    }
		    for (np = dataname; np < dataname + sizeof dataname - 1 && (ch = getc(f)) != EOF && ch != ','; np++)
			*np = ch;
		    *np = '\0';
		    if (fscanf(f, "%ld ", &uniqueID) != 1) {
			objectto(f, "layout:  missing , after component name");
			(self)->RemoveComponent( c);
			return dataobject::BADFORMAT;
		    }
		    if (fgetstring(f, "}") != 0) {
			objectto(f, "layout:  missing closing brace after begindata");
			(self)->RemoveComponent( c);
			return dataobject::BADFORMAT;
		    }
		    if (fgetstring(f, "\n") != 0)
			objectto(f, "layout:  extra stuff after begindata");
		    c->data = (class dataobject *)ATK::NewObject(dataname);
		    if (cData(c) == NULL) {
			printf("Could not create %s object.\n", dataname);
			return dataobject::OBJECTCREATIONFAILED;
		    }
		    cData(c)->SetID(uniqueID);
		    (cData(c))->Read( f, uniqueID);

		}
		(self)->Demote( c);
		if (debug)
		    printf("done adding component\n");
		break;

	    case '\\': /* enddata coming */
		if (fscanf(f, "enddata{%255[^,}\n],%ld}\n", dataname, &uniqueID) != 2) {
		    objectto(f, "layout:  expected enddata or another component");
		    return dataobject::BADFORMAT;
		}
		else if (strcmp(dataname, (self)->GetTypeName()) != 0) {
		    objectto(f, "layout: wrong data name in enddata");
		    return dataobject::BADFORMAT;
		}
		else if (uniqueID != id) {
		    objectto(f, "layout:  wrong unique ID in enddata");
		    return dataobject::BADFORMAT;
		}
		return dataobject::NOREADERROR;

	    default:
		ungetc(ch, f);
		objectto(f, "layout:  bad input line");
		return dataobject::BADFORMAT;

	} /* end switch */
    } /* end reading loop */
    printf("layout:  premature EOF");
    return dataobject::PREMATUREEOF;
}

/* read layout from file */

long				    /* returns read error status */
layout::Read(FILE  * f			    /* input file */, long  id			    /* unique identifier in data stream */)
{
    long rc;

    if (debug)
	printf("layout_Read(%ld)\n", id);

    (this)->SetModified();

    rc = readASCII(this, f, id);
    (this)->NotifyObservers( observable::OBJECTCHANGED);
    if (debug)
	printf("layout_Read rc = %ld\n", rc);

    return rc;
}

/* remove a component */

void layout::RemoveComponent(struct component  *c		    /* component to be removed */)
{
    struct component *prev;

    if (debug)
	printf("layout_RemoveComponent(%s)\n", safename(c));

    if (cData(c) != NULL) {
	/* dataobject::Destroy(cData(c)); */
	c->data = NULL;
    }

    if (c == this->firstcomponent)
	this->firstcomponent = c->nextcomponent;
    else {
	forallcomponents(this, prev) {
	    if (prev->nextcomponent == c) {
		prev->nextcomponent = c->nextcomponent;
		break;
	    }
	}
	if (prev == NULL)
	    printf("layout:  attempt to remove non-component\n");
    }
    free(c);
    (this)->SetModified();
    (this)->NotifyObservers( observable::OBJECTCHANGED);

    if (debug)
	printf("imbedded remove complete\n");
}

/* fill in component */

void layout::FillInComponent(const char  *name				/* name of dataobject subclass */, struct component  *c			/* component to be filled in */)
{
    class dataobject *newobject;

    if (debug)
	printf("layout_FillInComponent(%s)\n", name);

    newobject = (class dataobject *) ATK::NewObject(name);
    if (newobject != NULL) {
	if (cData(c) != NULL)
	    (cData(c))->Destroy();
	c->data = newobject;
    }
    (this)->SetModified();
    (this)->NotifyObservers( observable::OBJECTCHANGED);
}

/* create component */

struct component *layout::CreateComponent()
{
    struct component *c;

    if (debug)
	printf("layout_CreateComponent\n");

    c = (struct component *)malloc(sizeof(struct component));
    if (c == NULL) {
	printf("Insufficient memory for component\n");
	exit(4);
    }
    c->data = NULL;
    c->left = 0;
    c->top = 0;
    c->width = 0;
    c->height = 0;
    c->varies = TRUE;
    c->nextcomponent = this->firstcomponent;
    this->firstcomponent = c;
    /* we don't notify anybody of changes yet because this component is still vacant */
    return c;
}

/* change component size */

void layout::SetComponentSize(struct component  *c			/* component to change */, long  x	, long  y , long  w , long  h			/* new position and size */)
{
    if (debug)
	printf("layout_SetComponentSize(%s, %ld, %ld, %ld, %ld)\n", classname(cData(c)), x, y, w, y);

    if (cLeft(c) != x || cTop(c) != y || cWidth(c) != w || cHeight(c) != h) {
	c->left = x;
	c->top = y;
	c->width = w;
	c->height = h;
	(this)->SetModified();
	(this)->NotifyObservers( observable::OBJECTCHANGED);
    }
}

/* check to see if modified */

long layout::GetModified()
{
    struct component *c;
    long rc, cc;

    rc = (this)->dataobject::GetModified();
    if (!this->inGetModified) {
	this->inGetModified = TRUE;
	forallcomponents(this, c) {
	    if (cData(c) != NULL) {
		cc = (cData(c))->GetModified();
		if (cc > rc) rc = cc;
	    }
	}
	this->inGetModified = FALSE;
    }

    if (debug)
	printf("layout_GetModified = %ld\n", rc);

    return rc;
}

/* see if b would completely obscure c */

#define obscures(self, b, c) ((cLeft(b) <= 0 || (cLeft(c) > 0 && cLeft(b) <= cLeft(c))) && (cWidth(b) <= 0 || (cWidth(c) > 0 && cRight(b) >= cRight(c))) && (cTop(b) <= 0 || (cTop(c) > 0 && cTop(b) <= cTop(c))) && (cHeight(b) <= 0 || (cHeight(c) > 0 && cBottom(b) >= cBottom(c))))

/* see if b and c overlap */

#define overlaps(self, b, c) ((cLeft(b) <= 0 || cWidth(c) <= 0 || cLeft(b) <= cRight(c)) && (cWidth(b) <= 0 || cLeft(c) <= 0 ||cRight(b) >= cLeft(c)) && (cTop(b) <= 0 || cHeight(c) <= 0 || cTop(b) <= cBottom(c)) && (cHeight(b) <= 0 || cTop(c) <= 0 || cBottom(b) >= cTop(c)))

/* promote component to front of stack*/

void layout::Promote(struct component  *c			/* component to be promoted to front */)
{
    struct component *prev;
    boolean changed;

    if (debug)
	printf("layout_Promote(%s)\n", classname(cData(c)));

    changed = FALSE;
    if (c != this->firstcomponent) {
	forallcomponents(this, prev) {
	    if (overlaps(this, prev, c))
		changed = TRUE;
	    if (prev->nextcomponent == c) {
		prev->nextcomponent = c->nextcomponent;
		c->nextcomponent = this->firstcomponent;
		this->firstcomponent = c;
		break;
	    }
	}
	if (prev == NULL)
	    printf("Layout: Attempt to promote non-component\n");
    }
    if (changed) {
	(this)->SetModified();
	(this)->NotifyObservers( observable::OBJECTCHANGED);
    }
}

/* demote component to back of the bus */

void layout::Demote(struct component  *c			/* component to be demoted to back */)
{
    struct component *prev;

    if (debug)
	printf("layout_Demote(%s)\n", safename(c));

    if (c == this->firstcomponent)
	this->firstcomponent = c->nextcomponent;
    else {
	forallcomponents(this, prev) {
	    if (prev->nextcomponent == c) {
		prev->nextcomponent = c->nextcomponent;
		break;
	    }
	}
	if (prev == NULL)
	    printf("layout:  attempt to demote non-component\n");
    }
    
    if (this->firstcomponent == NULL) {
	c->nextcomponent = this->firstcomponent;
	this->firstcomponent = c;
    }
    else {
	forallcomponents(this, prev)
	  if (prev->nextcomponent == NULL || obscures(this, prev->nextcomponent, c)) {
	      c->nextcomponent = prev->nextcomponent;
	      prev->nextcomponent = c;
	      break;
	  }
    }

    (this)->SetModified();
    (this)->NotifyObservers( observable::OBJECTCHANGED);
}

/* make object variable */

void layout::MakeVariable(struct component  *c			/* component which may vary */)
{
    if (debug)
	printf("layout_MakeVariable(%s)\n", safename(c));

    c->varies = TRUE;
}

/* make object fixed */

void layout::MakeFixed(struct component  *c			/* component which may vary */)
{
    if (debug)
	printf("layout_MakeFixed(%s)\n", safename(c));

    c->varies = FALSE;
}
