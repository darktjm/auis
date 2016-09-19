/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

/* reference to the associated data object */

#define getView(V) (*(V))
#define getIM(V) (&getView(V))->GetIM()
#define getDrawable(V) (&getView(V))->GetDrawable()
#define MyFiller(V) ((class filler *)(getView(V).dataobject))

#define fillerview_MINIMUMSIZE 100
#define LEFTMARGIN 5
#define TOPMARGIN 5

#define viewnamestring(v) ((v) == NULL ? "<NO VIEW>" : (((v)->GetName())->First())->Name())


#include <andrewos.h>
ATK_IMPL("fillerview.H")
#include <assert.h>

#include <atomlist.H>
#include <dataobject.H>
#include <environ.H>
#include <fontdesc.H>
#include <menulist.H>
#include <message.H>
#include <proctable.H>
#include <rect.h>
#include <view.H>

#include <filler.H>

#include <fillerview.H>

static boolean debug=FALSE;
/* graphic information */

struct graphicstuff {
    class fontdesc *writingFont;
    int lineheight;
    int linewidth;
};

#define DEBUGPRINTF(x)

/* list of possible replacement objects */

static char **Insets;

/* initialize entire class */

static class menulist *mainmenus = (class menulist *) NULL;
static struct proctable_Entry *replaceProc = (struct proctable_Entry *) NULL;

/* add an inset to the list of candidates */


ATKdefineRegistry(fillerview, celview, fillerview::InitializeClass);
static void addInset(char  *newposs);
static void initializeInsets();
static void InitializeGraphics(class fillerview  *self, struct graphicstuff  *gc);
static int			/* returns y position of top of button */ TopOfTheMark(class fillerview  *self, struct graphicstuff  *gc, int  i				/* index of this button */);
static void showhit(class fillerview  *self, struct graphicstuff  *gc);
static void UpdateScreen(class fillerview  *self, enum view_UpdateType  how		    /* kind of update */, struct rectangle  *updateRect		    /* rectangle affected */);


static void
addInset(char  *newposs)
{
    int n;
    char *b;

    for (n = 0; Insets[n] != NULL; n++)
	if (strcmp(Insets[n], newposs) == 0)
	    return;
    b = strdup(newposs);
    assert(b != NULL);
    Insets = (char **)realloc((char *)Insets, (n+2)*sizeof (char *));
    assert(Insets != NULL);
    Insets[n] = b;
    Insets[n+1] = NULL;
}

static void
initializeInsets()
{
    char buff[81];
    const char *insetlist;
    const char *c;
    char *d;
    int i;

    Insets = (char **)malloc(sizeof (char *));
    assert(Insets != NULL);
    Insets[0] = NULL;

    insetlist = environ::GetProfile("FillerInsets");
    if (insetlist == NULL || *insetlist == '\0')
	insetlist = "Ask...:box:text:raster:table:zip:fad:pushbuttn:clock:timeoday:link";
    for (c = insetlist, d = buff; *c != '\0'; c++) {
	if (*c == ':') {
	    assert(d-buff < sizeof buff);
	    *d++ = '\0';
	    addInset(buff);
	    d = buff;
	}
	else if (*c == ' ')
	    ;
	else
	    *d++ = *c;
    }
    assert(d-buff < sizeof buff);
    *d++ = '\0';
    addInset(buff);

    /* do menus in opposite order so they appear in right order on card */

    for (i = 0; Insets[i] != NULL; i++) ;
    while (--i >= 0) {
	sprintf(buff, "Insets,%s", Insets[i]);
	assert(strlen(buff) < sizeof buff);
	(mainmenus)->AddToML( buff, replaceProc, (long)Insets[i], 0);
    }
}

static int SetDataObjectByName(ATK *v, long c)
{
    ((class fillerview *)v)->SetDataObjectByName((char *)c);
    return 0;
}

/* initialize entire class */

boolean				    /* returns TRUE for success */
fillerview::InitializeClass()
{
    DEBUGPRINTF(("fillerview__InitializeClass(%x)\n", classID));

    mainmenus = new menulist;

    replaceProc = proctable::DefineProc("fillerview-set-data-object-by-name", (proctable_fptr)::SetDataObjectByName, &fillerview_ATKregistry_ , NULL, "Insert named inset");
    initializeInsets();

    return TRUE;
}

/* initialize filler view */

fillerview::fillerview()
{
	ATKinit;

    DEBUGPRINTF(("fillerview__InitializeObject(%x)\n", classID));

    this->hasInputFocus = FALSE;
    this->menulistp = (mainmenus)->DuplicateML( this);
    this->hitindex = -1;
    (this)->WantInputFocus( this);
    THROWONFAILURE( TRUE);
}

/* initialize graphic-dependent data */

static void
InitializeGraphics(class fillerview  *self, struct graphicstuff  *gc)
{
    static const char *wfontname = NULL;
    struct FontSummary *fs;
    int i;
    long w;

    DEBUGPRINTF(("fillerview_InitializeGraphics\n"));

    if (getDrawable(self) == NULL) {
	printf("InitializeGraphics called without drawable.\n");
	return;
    }
    if (wfontname == NULL) {
	wfontname = environ::GetProfile("bodyfontfamily");
	if (wfontname == NULL || !*wfontname) wfontname = "andy";
    }
    gc->writingFont = fontdesc::Create(wfontname, fontdesc_Plain, 12);
    fs = (gc->writingFont)->FontSummary( getDrawable(self));
    gc->lineheight = fs->maxHeight;
    gc->linewidth = 0;
    for (i = 0; Insets[i] != NULL; i++) {
	(gc->writingFont)->StringSize( getDrawable(self), Insets[i], &w, NULL);
	if (w > gc->linewidth)
	    gc->linewidth = w;
    }
}

/* compute top of line hit */

static int			/* returns y position of top of button */
TopOfTheMark(class fillerview  *self, struct graphicstuff  *gc, int  i				/* index of this button */)
{
    return TOPMARGIN * 2 - 2 + (i + 1) * gc->lineheight;
}

/* highlight hit box */

static void
showhit(class fillerview  *self, struct graphicstuff  *gc)
{
    short savetransfermode;

    if (self->hitindex >= 0) {
	savetransfermode = (self)->GetTransferMode();
	(self)->SetTransferMode( graphic_INVERT);
	(self)->DrawRectSize( LEFTMARGIN, TopOfTheMark(self, gc, self->hitindex), gc->linewidth + 3 * LEFTMARGIN, gc->lineheight);
	(self)->SetTransferMode( savetransfermode);
    }
}

/* process mouse hit */

class view *				/* returns view to get subsequent hits */
fillerview::Hit(enum view_MouseAction  action		/* button and what it did */, long  x	, long  y				/* coordinates of mouse */, long  numberOfClicks			/* number of clicks at this location */)
{
    int i;
    struct graphicstuff realgc, *gc = &realgc;

    DEBUGPRINTF(("fillerview_Hit(%d, %ld, %ld, %ld)\n", (int) action, x, y, numberOfClicks));

    if ((this)->GetTrueChild() != NULL)
	return (this)->celview::Hit( action, x, y, numberOfClicks);

    InitializeGraphics(this, gc);

    if (this->hitindex >= 0) {
	showhit(this, gc);  /* turns it off */
	this->hitindex = -1;
    }

    if (y >= TopOfTheMark(this, gc, 0) && x >= 0 && x < gc->linewidth + 3 * LEFTMARGIN) {
	for (i = 0; Insets[i] != NULL; i++) {
	    if (y < TopOfTheMark(this, gc, i + 1)) {
		this->hitindex = i;
		break;
	    }
	}
    }

    if (this->hitindex >= 0 && (action == view_LeftDown || action == view_LeftMovement || action == view_RightDown || action == view_RightMovement)) {
	showhit(this, gc);
    }

    else if (this->hitindex >= 0 && (action == view_LeftUp || action == view_RightUp)) {
	showhit(this, gc);
	(this)->SetDataObjectByName( Insets[this->hitindex]);
	return NULL;
    }

    else
	this->hitindex = -1;

    if (!this->hasInputFocus)
	(this)->WantInputFocus( &getView(this));

    return &getView(this);
}

/* update all components */

static void
UpdateScreen(class fillerview  *self, enum view_UpdateType  how		    /* kind of update */, struct rectangle  *updateRect		    /* rectangle affected */)
{
    int i;
    struct graphicstuff realgc, *gc = &realgc;

    InitializeGraphics(self, gc);

    (self)->SetTransferMode( graphic_COPY);
    (self)->SetClippingRect( updateRect);
    (self)->EraseRect( updateRect);
    (self)->MoveTo( LEFTMARGIN, TOPMARGIN);
    (self)->DrawString( "Select an inset", graphic_ATTOP | graphic_ATLEFT);
    for (i = 0; Insets[i] != NULL; i++) {
	(self)->MoveTo( LEFTMARGIN * 2, TOPMARGIN * 2 + (i + 1) * gc->lineheight);
	(self)->DrawString( Insets[i], graphic_ATTOP | graphic_ATLEFT);
    }
    showhit(self, gc);
}

/* full update when window changes */

void
fillerview::FullUpdate(enum view_UpdateType  how		    /* kind of update */, long  left , long  top , long  width , long  height		    /* rectangle affected (in some cases; */)
{
    struct rectangle vb;

    DEBUGPRINTF(("fillerview_FullUpdate(%d, %d, %d, %d, %d)\n", (int)how, left, top, width, height));

    (this)->celview::FullUpdate( how, left, top, width, height);
    if ((this)->GetTrueChild() != NULL)
	return;

    (this)->GetVisualBounds( &vb);

    switch(how) {

	case view_MoveNoRedraw:
	case view_Remove:
	    break;

	case view_PartialRedraw:
	case view_LastPartialRedraw:
	    rectangle_SetRectSize(&vb, left, top, width, height);
	    /* fall through into default case */
	default:
	    UpdateScreen(this, how, &vb);
	    break;
    }
}

/* partial update */

void
fillerview::Update()
{
    struct rectangle visualRect;

    DEBUGPRINTF(("fillerview_Update\n"));

    (this)->celview::Update();
    if ((this)->GetTrueChild() != NULL)
	return;

    (this)->GetVisualBounds( &visualRect);
    UpdateScreen(this, view_FullRedraw, &visualRect);
}

/* input focus obtained; highlight something */

void
fillerview::ReceiveInputFocus()
{
    DEBUGPRINTF(("fillerview_ReceiveInputFocus\n"));

    (this)->celview::ReceiveInputFocus();
    if ((this)->GetTrueChild() != NULL)
	return;

    if (!(this->hasInputFocus)) {
	this->hasInputFocus = 1;
	(this)->PostMenus( this->menulistp);
    }
}

/* input focus lost; remove highlighting */

void
fillerview::LoseInputFocus()
{
    DEBUGPRINTF(("fillerview_LoseInputFocus\n"));

    (this)->celview::LoseInputFocus();
    if ((this)->GetTrueChild() != NULL)
	return;

    if (this->hasInputFocus) {
	this->hasInputFocus = 0;
    }
}

/* tear down a fillerview */

fillerview::~fillerview()
{
	ATKinit;

    if (debug)
	printf("fillerview_FinalizeObject\n");

    delete this->menulistp;
}

/* set contained data object */

void
fillerview::SetDataObjectByName(const char  *dataname				/*class  dataname of replacement dataobject */)
{
    DEBUGPRINTF(("fillerview_SetDataObjectByName(,%s)\n", dataname));

    if (strncmp(dataname, "Ask", 3) == 0) {
	char newname[100];

	newname[0] = '\0';
	if (message::AskForString(&getView(this), 0, "Data object to enter here (text): ", "", newname, sizeof newname))
	    return;
	if (newname[0] == 0)
	    dataname = "text";
	else
	    dataname = newname;
    }
    (MyFiller(this))->SetObjectByName( dataname);
    (MyFiller(this))->SetViewName( NULL, TRUE);
    (MyFiller(this))->SetRefName( dataname);
    ((class celview *) this)->arb = NULL;
    (this)->ObservedChanged( MyFiller(this), observable_OBJECTCHANGED);
    (this)->WantInputFocus( this);
}
