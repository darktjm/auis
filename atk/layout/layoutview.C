/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

#define layoutview_MINIMUMSIZE 100
#define CHILD_MENULIST_KEY 999

#define viewname(v) ((v) == NULL ? "<NO VIEW>" : (((v)->GetName())->First())->Name())

#include <andrewos.h>
ATK_IMPL("layoutview.H")
#include <rect.h>
#include <assert.h>

#include <atomlist.H>
#include <bind.H>
#include <cel.H>
#include <cursor.H>
#include <dataobject.H>
#include <environ.H>
#include <filetype.H>
#include <im.H>
#include <keymap.H>
#include <keystate.H>
#include <menulist.H>
#include <message.H>
#include <region.H>
#include <texttroff.H>
#include <view.H>

#include <layout.H>
#include <layoutview.H>

static char layout_debug=0;

#define	POINTINGTHRESHHOLD 6	    /* max distance away from object pointed at */
#define	MOTIONTHRESHHOLD 10	    /* min distance for dragging */

/* menu mode mask */

#define layoutview_EXEC_MASK 1
#define layoutview_AUTHOR_MASK 2

#define layoutview_SELECTION_MASK 4

#define layoutview_VARIABLE_MASK 8
#define layoutview_FIXED_MASK 16

/* internal interfaces */

#define getView(self) (*(self))
#define getLayout(self) ((class layout *)(self)->GetDataObject())


/* static data for entire class */

static long SetRunMode(ATK *v, long c)
{
    ((class layoutview *)v)->SetRunMode();
    return 0;
}

static long SetAuthoringMode(ATK *v, long c)
{
    ((class layoutview *)v)->SetAuthoringMode();
    return 0;
}

static long Paste(ATK *v, long c)
{
    ((class layoutview *)v)->Paste();
    return 0;
}

static long SetChild(ATK *v, long c)
{
    ((class layoutview *)v)->SetChild();
    return 0;
}

static long MakeBackground(ATK *v, long c)
{
    ((class layoutview *)v)->MakeBackground();
    return 0;
}

static long DestroyComponent(ATK *v, long c)
{
    ((class layoutview *)v)->DestroyComponent();
    return 0;
}

static long Demote(ATK *v, long c)
{
    ((class layoutview *)v)->Demote();
    return 0;
}

static long MakeVariable(ATK *v, long c)
{
    ((class layoutview *)v)->MakeVariable();
    return 0;
}

static long MakeFixed(ATK *v, long c)
{
    ((class layoutview *)v)->MakeFixed();
    return 0;
}

static long SetCreateMode(ATK *v, long c)
{
    ((class layoutview *)v)->SetCreateMode((enum createmode_enum)c);
    return 0;
}

static long SetGranularity(ATK *v, long c)
{
    ((class layoutview *)v)->SetGranularity(c);
    return 0;
}

static long ToggleDebug(ATK *v, long c)
{
    ((class layoutview *)v)->ToggleDebug();
    return 0;
}

static class keymap *mainmap = (class keymap *) NULL;
static class menulist *mainmenus = (class menulist *) NULL;
static struct bind_Description layoutview_bindings[] = {

    {"layout-run", "\033x", 0, "Exit Authoring~60", 0, layoutview_AUTHOR_MASK, ::SetRunMode, "Enter execution mode"},
    {"layout-author", 0, 0, "Enter Authoring~60", 0, layoutview_EXEC_MASK, ::SetAuthoringMode, "Enter authoring mode"},

    {"layout-toggle-debug", "\033\033", 0, 0, 0, 0, ::ToggleDebug, "Toggle Debug"},
    {"layout-paste-object", "\031", 0, "Authoring~1,Paste object~20", 0, layoutview_SELECTION_MASK, ::Paste, "Paste into current selection"},
    {"layout-imbed-object", "\033\t", 0, "Authoring~1,Imbed object~21", 0, layoutview_SELECTION_MASK, ::SetChild, "Prompt for object and place in current selection"},
    {"layout-destroy-object", "\004", 0, "Authoring~1,Cut object~22", 0, layoutview_SELECTION_MASK, ::DestroyComponent, "Destroy current selection"},

    {"layout-move-to-back", "\033d", 0, "Authoring~1,Move to back~30", 0, layoutview_SELECTION_MASK, ::Demote, "Move current selection to back"},
    {"layout-background", "\033b", 0, "Authoring~1,Background~31", 0, layoutview_SELECTION_MASK, ::MakeBackground, "Expand current selection and move to back"},
    {"layout-make-variable", "\033v", 0, "Authoring~1,Make variable~35", 0, layoutview_FIXED_MASK, ::MakeVariable, "Allow component to vary"},
    {"layout-make-fixed", "\033f", 0, "Authoring~1,Make fixed~35", 0, layoutview_VARIABLE_MASK, ::MakeFixed, "Component varies only in authoring mode"},

    {"layout-create-0", 0, 0, "Authoring~1,Null on create~40", CREATE_NULL, layoutview_AUTHOR_MASK, ::SetCreateMode, "Do not fill in newly created data objects"},
    {"layout-create-filler", 0, 0, "Authoring~1,Filler on create~41", CREATE_FILLER, layoutview_AUTHOR_MASK, ::SetCreateMode, "Use filler for newly created data objects"},
    {"layout-create-paste", 0, 0, "Authoring~1,Paste on create~42", CREATE_PASTE, layoutview_AUTHOR_MASK, ::SetCreateMode, "Paste into newly created data objects"},

    {"layout-fine-granularity", 0, 0, "Authoring~1,Fine granularity~50", 0, layoutview_AUTHOR_MASK, ::SetGranularity, "Fine (1 pixel) granularity of object placement"},
    {"layout-medium-granularity", 0, 0, "Authoring~1,Medium granularity~51", 6, layoutview_AUTHOR_MASK, ::SetGranularity, "Medium (6 pixel) granularity of object placement"},
    {"layout-coarse-granularity", 0, 0, "Authoring~1,Coarse granularity~52", 18, layoutview_AUTHOR_MASK, ::SetGranularity, "Coarse (18 pixel) granularity of object placement"},

    {0, 0, 0, 0, 0, 0, 0}
};

/* initialize static data shared by all instances of layoutview */


ATKdefineRegistry(layoutview, view, layoutview::InitializeClass);
static void  InitializeGraphic(class layoutview  *self);
static const char * GetClassName(class layoutview  *self, class dataobject  *object);
static void InitComponent(class layoutview  *self);
static void DrawRubberBox(class layoutview  *self);
static void Update(class layoutview  *self, enum view_UpdateType  how		/* kind of update */, struct rectangle  *updateRect		/* rectangle affected; or NULL for update */, boolean  geometryChanged		/* geometry changed since last update */);
static void SetAuthoringMask(class layoutview  *self);
static void SetRubberBox(class layoutview  *self, long  x			/* current position relative to self->dragx */, long  y			/* current position relative to self->dragy */);
static struct component *	/* returns component containing x, y or NULL */ FindContainingComponent(class layoutview  *self, long  x				/* point to be found */, long  y, long  thresh			/* tolerance outside component allowed */);
static void DestroySubviews(class layoutview  *self);
static void PrintComponents(class layoutview  *self, FILE  *f, const char  *processor			/* processor */, const char  *finalFormat			/* final format */, struct component  *c			/* current component to be printed */, int  saveno				/* number of state-restoring macro */);


boolean					/* always returns TRUE */
layoutview::InitializeClass()
{
    mainmap = new keymap;
    mainmenus = new menulist;
    bind::BindList(layoutview_bindings, mainmap, mainmenus, &layoutview_ATKregistry_ );

    return TRUE;
}

/* initialize layout view */

layoutview::layoutview()
{
	ATKinit;
    this->hasInputFocus = FALSE;
    this->updateRequested = FALSE;
    this->lastUpdate = 0;
    this->menulistp = (mainmenus)->DuplicateML( this);
    this->keystatep = keystate::Create(this, mainmap);
    this->authoringCursor = NULL;
    this->subviews = NULL;
    this->selection = NULL;
    this->hitmode = HIT_EXEC;
    this->createmode = CREATE_FILLER;
    this->granularity = 0;
    (this->menulistp)->SetMask( layoutview_EXEC_MASK);
    THROWONFAILURE( TRUE);
}

/* initialize graphic-dependent data */

static void 
InitializeGraphic(class layoutview  *self)
{
   
    if(layout_debug)
	printf("layoutview_InitializeGraphic\n");

    if ((self)->GetDrawable() == NULL) {
	printf("InitializeGraphic called without drawable.\n");
	return;
    }
    self->authoringCursor = cursor::Create(self);
    (self->authoringCursor)->SetStandard( Cursor_SmallCross);
}

/* get printable class name of an object */

static const char *
GetClassName(class layoutview  *self, class dataobject  *object)
{
    const char *result;

    if (object == NULL)
	result = "<NO OBJECT>";
    else {
	result = (char *)(object)->GetTypeName();
	if (strcmp(result, "filler") == 0 || strcmp(result, "cel") == 0)
	    result = ((class cel *)object)->GetObjectName();
    }

    return result;
}

/* find or create new viewlist entry */

class view *		    /* returns view, or NULL if error */
layoutview::FindSubview(struct component  *c		    /* component for which view is needed */)
{
    struct layoutviewlist *vl;
    const char *subviewname;			/* name for new view */

    if (c == NULL)
	return NULL;
    if (cData(c) == NULL)		/* can not create view for non-object */
	return NULL;

    forallsubviews(this, vl)
	if (vComponent(vl) == c)
	    return vChild(vl);		/* it already exists, so return its child view */

    /* construct new layoutviewlist entry */

    vl = (struct layoutviewlist *) malloc(sizeof *vl);
    if (vl == NULL) {
	printf("out of space for subview list\n");
	return NULL;
    }
    subviewname = (cData(c))->ViewName();
    vl->component = c;
    vl->child = (class view *) ATK::NewObject(subviewname);
    if (vl->child == NULL) {
	printf("unable to create child view\n");
	free(vl);
	return NULL;
    }
    (vChild(vl))->SetDataObject( cData(c));
    (vChild(vl))->LinkTree( &getView(this));
    (vChild(vl))->AddObserver( this);

    if (layout_debug)
	printf(".. created subview %s\n", subviewname); 
    vl->nextview = this->subviews;
    this->subviews = vl;

    return vChild(vl);
}

/* remove layoutviewlist entry */

struct component *
layoutview::RemoveSubview(class view  *child)
{
    struct layoutviewlist *vl, *uvl;
    struct component *c;

    uvl = NULL;
    if (this->subviews != NULL && child == vChild(this->subviews)) {
	uvl = this->subviews;
	this->subviews = uvl->nextview;
    }
    else {
	forallsubviews(this, vl) {
	    if (vl->nextview != NULL && child == vChild(vl->nextview)) {
		uvl = vl->nextview;
		vl->nextview = uvl->nextview;
		break;
	    }
	}
    }
    if (uvl != NULL) {
	if (layout_debug)
	    printf(".. removed subview %s\n", viewname(child)); 
	c = vComponent(uvl);
	free((char *)uvl);
    }
    else
	c = NULL;
    (child)->RemoveObserver( this);
    return c;
}

/* replace contents of a component */

void
layoutview::ReplaceComponent(struct component  *c, const char  *dataname)
{
    class view *child;
    char foo[81];
    struct layoutviewlist *vl;

    if(! ATK::IsTypeByName(dataname, "dataobject")){
        sprintf(foo, "%s is not a dataobject", dataname);
	message::DisplayString(&getView(this), 0, foo);
        return;
    }

    child = NULL;
    forallsubviews(this, vl)
      if (vComponent(vl) == (this)->Selection()) {
	  child = vChild(vl);
	  break;
      }
    if (child != NULL) {
	(this)->RemoveSubview( child); /* prevents removing component in view_Destroy */
	(child)->UnlinkTree();
	(child)->Destroy();
    }

    (getLayout(this))->FillInComponent( dataname, c);
}

/* Init new component */

static void
InitComponent(class layoutview  *self)
{
    class view *child;
    child = (self)->FindSubview( (self)->Selection());
    if (child != NULL)
	(child)->WantInputFocus( child);
}

/* negotiate size of view */

view_DSattributes			/* returns indication of what it wants */
layoutview::DesiredSize(long  width				/* width being offered by parent */, long  height				/* height being offered */, enum view_DSpass  pass			/* what parent is willing to give */, long  *dWidth				/* set to desired width */, long  *dHeight				/* set to desired height */)

/*  layoutview asks for just enough space to display all contained components */

{
    struct component *c;
    long desiredWidth;
    long desiredHeight;

    if (layout_debug)
	printf("layoutview_DesiredSize(, %ld, %ld, %d, .. )\n", width, height, (int)pass);

    desiredWidth = desiredHeight = layoutview_MINIMUMSIZE;
    forallcomponents(getLayout(this), c) {
	if (cWidth(c) > 0 && desiredWidth < cRight(c))
	    desiredWidth = cRight(c);
	if (cHeight(c) > 0 && desiredHeight < cBottom(c))
	    desiredHeight = cBottom(c);
    }

    *dWidth = (pass == view_WidthSet) ? width : desiredWidth;
    *dHeight = (pass == view_HeightSet) ? height : desiredHeight;

    return (view_DSattributes)
      ((int)((*dWidth > desiredWidth) ? view_WidthFlexible : view_WidthLarger)
      | (int)((*dHeight > desiredHeight) ? view_HeightFlexible : view_HeightLarger));
}

/* draw rubber-band box */

static void
DrawRubberBox(class layoutview  *self)
{
    short savetransfermode;

    savetransfermode = (self)->GetTransferMode();
    (self)->SetTransferMode( graphic_INVERT);

    (self)->DrawRectSize( self->rubberleft - 1, self->rubbertop - 1, self->rubberwidth + 1, self->rubberheight + 1);
 
    (self)->SetTransferMode( savetransfermode);
}

/* update image */

#define ReallyDrawing(how, updateRect) (updateRect == NULL || (how != view_Remove && how != view_MoveNoRedraw))

static void
Update(class layoutview  *self, enum view_UpdateType  how		/* kind of update */, struct rectangle  *updateRect		/* rectangle affected; or NULL for update */, boolean  geometryChanged		/* geometry changed since last update */)
{
    struct component *c;
    class view *child;
    class region *visualRegion;	/* visual region of entire layout */
    class region *remainingRegion;	/* region still to be updated */
    struct rectangle childRect;		/* rectangle containing child */
    class region *childRegion;		/* visual region available to child */
    struct rectangle frameRect;		/* rectangle containing child and frame */
    class region *frameRegion;		/* region containing child plus frame */
    struct rectangle vb;		/* visual bounds for cursor */
    class graphic *fillpattern;	/* fill pattern bitmap */

    /* deal with cursors */

    if (self->hasInputFocus && how != view_Remove && (self)->Hitmode() != HIT_EXEC) {
	(self)->GetVisualBounds( &vb);
	(self)->PostCursor( &vb, self->authoringCursor);
    }
    else
	(self)->RetractCursor( self->authoringCursor);

    /* initialize region being updated */

    visualRegion = (self)->GetVisualRegion( region::CreateEmptyRegion());
    if (updateRect != NULL) {
	remainingRegion = region::CreateRectRegion(updateRect);
	(remainingRegion)->IntersectRegion( visualRegion, remainingRegion);
    }
    else {
	remainingRegion = region::CreateEmptyRegion();
	region::CopyRegion(remainingRegion, visualRegion);
    }
    (self)->SetClippingRegion( remainingRegion);
    if (ReallyDrawing(how, updateRect) && geometryChanged)
	(self)->EraseVisualRect();
    else if ((self)->Hitmode() == HIT_DRAGGING || (self)->Hitmode() == HIT_CREATING)
	DrawRubberBox(self);

    forallcomponents(getLayout(self), c) {

	rectangle_SetRectSize(&childRect, vLeft(self, c), vTop(self, c), vWidth(self, c), vHeight(self, c));
	childRegion = region::CreateRectRegion(&childRect);
	(childRegion)->IntersectRegion( remainingRegion, childRegion);

	/* draw frame around child */

	if ((self)->Hitmode() != HIT_EXEC) {
	    if (ReallyDrawing(how, updateRect)) {
		(self)->SetClippingRegion( remainingRegion);
		if (c != (self)->Selection())
		    fillpattern = (self)->GrayPattern( 8, 16);
		else if ((self)->Hitmode() != HIT_DRAGGING && (self)->Hitmode() != HIT_CREATING)
		    fillpattern = (self)->BlackPattern();
		else 
		    fillpattern = (self)->WhitePattern();
		if (cTop(c) > 0)
		    (self)->FillRectSize( vLeft(self, c) - 1, cTop(c) - 1, vWidth(self, c) + 1, 1, fillpattern);
		if (cWidth(c) > 0)
		    (self)->FillRectSize( cRight(c), vTop(self, c) - 1, 1, vHeight(self, c) + 1, fillpattern);
		if (cHeight(c) > 0)
		    (self)->FillRectSize( vLeft(self, c), cBottom(c), vWidth(self, c) + 1, 1, fillpattern);
		if (cLeft(c) > 0)
		    (self)->FillRectSize( cLeft(c) - 1, vTop(self, c), 1, vHeight(self, c) + 1, fillpattern);
	    }
	    rectangle_SetRectSize(&frameRect, vLeft(self, c) - 1, vTop(self, c) - 1, vWidth(self, c) + 2, vHeight(self, c) + 2);
	    frameRegion = region::CreateRectRegion(&frameRect);
	    (remainingRegion)->SubtractRegion( frameRegion, remainingRegion);
	    delete frameRegion;
	}
	else
	    (remainingRegion)->SubtractRegion( childRegion, remainingRegion);

	/* update child itself */

	(self)->SetClippingRegion( childRegion);

	child  = (self)->FindSubview( c);
	if (!geometryChanged)
	    /* do nothing */ ;
	else if (child == NULL) {
	    if (ReallyDrawing(how, updateRect))
		(self)->FillRect( &childRect, (self)->BlackPattern());
	}
	else if (updateRect != NULL) {
	    if (layout_debug)
		printf("FullUpdating %s\n", (cData(c))->ViewName());
	    (child)->InsertView( self, &childRect);
	    (childRegion)->OffsetRegion( -vLeft(self, c), -vTop(self, c));
	    (child)->SetVisualRegion( childRegion);
	    (child)->FullUpdate( how, rectangle_Left(updateRect), rectangle_Top(updateRect), rectangle_Width(updateRect), rectangle_Height(updateRect));
	}
	else {
	    if (layout_debug)
		printf("Redrawing %s\n", (cData(c))->ViewName());
	    (child)->InsertView( self, &childRect);
	    (childRegion)->OffsetRegion( -vLeft(self, c), -vTop(self, c));
	    (child)->SetVisualRegion( childRegion);
	    (child)->FullUpdate( view_FullRedraw, 0, 0, vWidth(self, c), vHeight(self, c));
	}
	delete childRegion;

    }

    /* clean up clipping region, memory allocation, and active rubberbanding */

    delete remainingRegion;
    (self)->SetClippingRegion( visualRegion);
    delete visualRegion;

    if ((self)->Hitmode() == HIT_DRAGGING || (self)->Hitmode() == HIT_CREATING)
	DrawRubberBox(self);
}

/* full update when window changes */

void
layoutview::FullUpdate(enum view_UpdateType  how		/* kind of update */, long  left				/* updated rectangle (for certain kinds) */, long  top, long  width, long  height)
{
    struct rectangle cliprect;		/* actual updated rectangle */

    if (layout_debug)
	printf("layoutview_FullUpdate(%d, %ld, %ld, %ld, %ld)\n", (int)how, left, top, width, height);

    this->updateRequested = FALSE;

    /* deferred initialization of graphic */

    if (this->authoringCursor == NULL)
	InitializeGraphic(this);

    /* force authoring mode if the layout is empty */

    if ((this)->Hitmode() == HIT_EXEC && (getLayout(this))->GetFirstComponent() == NULL)
	(this)->SetAuthoringMode();

    /* define rectangle actually being updated */

    if (how == view_PartialRedraw || how == view_LastPartialRedraw)
	rectangle_SetRectSize(&cliprect, left, top, width, height);
    else {
	rectangle_SetRectSize(&cliprect, (this)->GetVisualLeft(), (this)->GetVisualTop(), (this)->GetVisualWidth(), (this)->GetVisualHeight());
    }
    /* perform the update */

    (this)->SetTransferMode( graphic_COPY);
    ::Update(this, how, &cliprect, TRUE);
    if (how == view_FullRedraw)
	this->lastUpdate = (getLayout(this))->GetModified();
}

/* partial update in response to WantUpdate request */

void
layoutview::Update()
{
    if (layout_debug)
	printf("layoutview_Update requested=%d\n", this->updateRequested);

    ::Update(this, view_FullRedraw, NULL, (this->lastUpdate < (getLayout(this))->GetModified()));
    this->lastUpdate = (getLayout(this))->GetModified();
}

/* request update */
#if 0
/* not declared in the .ch file so it wasn't being used before
 and still won't be -rr2b XXX */
void
layoutview::WantUpdate(class view  *requestor)
{

    if (layout_debug)
	printf("layoutview_WantUpdate(%x,%x) requested = %d\n", this, requestor, this->updateRequested);

    if (&getView(this) == requestor) {
	if (this->updateRequested)
	    return;
	this->updateRequested = TRUE;
    }	
    (this)->view::WantUpdate( requestor);
}
#endif

/* set authoring menu mask */

static void
SetAuthoringMask(class layoutview  *self)
{
    int m;

    m = layoutview_AUTHOR_MASK;

    if ((self)->Selection() != NULL) {
	m |= layoutview_SELECTION_MASK;
	if (cVaries((self)->Selection()))
	    m |= layoutview_VARIABLE_MASK;
	else
	    m |= layoutview_FIXED_MASK;

	if (layout_debug)
	    printf("  authoring mask = %d  varies=%d\n", m, cVaries((self)->Selection()));
    }

    if ((self->menulistp)->SetMask( m))
	(self)->PostMenus( NULL);
}

/* make new selection */

void
layoutview::SetSelection(struct component  *c			/* component to selected, or NULL */)
{
    if ((this)->Selection() != c) {
	(this)->Selection() = c;
	if ((this)->Hitmode() != HIT_EXEC)
	    (this)->WantUpdate( &getView(this));
    }
    if ((this)->Hitmode() != HIT_INITIALIZING && (this)->Hitmode() != HIT_EXEC)
	SetAuthoringMask(this);
}

/* set new selection size and position */

boolean				/*  returns TRUE if real selection created */
layoutview::SetSelectionSize(long  x , long  y , long  w , long  h)
{
    if (w < 0) {
	w = -w;
	x = x - w;
    }
    if (h < 0) {
	h = -h;
	y = y - h;
    }

    if (w < POINTINGTHRESHHOLD || h < POINTINGTHRESHHOLD) {	/* too small - ditch it */
	if ((this)->Selection() != NULL)
	    (this)->DestroyComponent();
    }
    else {
	if (x + w >= (this)->GetVisualRight() - POINTINGTHRESHHOLD)
	    w = 0;
	if (y + h >= (this)->GetVisualBottom() - POINTINGTHRESHHOLD)
	    h = 0;
	if (x <= POINTINGTHRESHHOLD)
	    x = 0;
	if (y <= POINTINGTHRESHHOLD)
	    y = 0;
	if ((this)->Selection() == NULL)
	    (this)->SetSelection( (getLayout(this))->CreateComponent());
	if ((this)->Selection() != NULL) {
	    (getLayout(this))->SetComponentSize( (this)->Selection(), x, y, w, h);
	    return TRUE;
	}
    }
    return FALSE;
}

/* set rubber band box size and position */

static void
SetRubberBox(class layoutview  *self, long  x			/* current position relative to self->dragx */, long  y			/* current position relative to self->dragy */)
{
    struct component *c = (self)->Selection();
    long xx, yy, rr, bb;

    if (c != (getLayout(self))->GetFirstComponent())
	(self)->Promote();
    xx = (self->dragleft ? x - self->dragx : 0);
    yy = (self->dragtop ? y - self->dragy : 0);
    rr = (self->dragright ? x - self->dragx : 0);
    bb = (self->dragbottom ? y - self->dragy : 0);
    self->rubberleft = (self)->ApplyGranularity( vLeft(self, c) + xx);
    self->rubbertop = (self)->ApplyGranularity( vTop(self, c) + yy);
    self->rubberwidth = (self)->ApplyGranularity( vWidth(self, c) - xx + rr);
    self->rubberheight = (self)->ApplyGranularity( vHeight(self, c) - yy + bb);
}

/* Find component containing (x, y) */

static struct component *	/* returns component containing x, y or NULL */
FindContainingComponent(class layoutview  *self, long  x				/* point to be found */, long  y, long  thresh			/* tolerance outside component allowed */)
{
    struct component *c;

    forallcomponents(getLayout(self), c) {
	if ((cLeft(c) <= 0 || cLeft(c) - thresh <= x) && (cWidth(c) <= 0 || cRight(c) + thresh > x) && (cTop(c) <= 0 || cTop(c) - thresh <= y) && (cHeight(c) <= 0 || cBottom(c) + thresh > y)) {
	    return c;
	}
    }
    return NULL;
}

/* process mouse hit */

class view *			/* returns view which should get follow-up events*/
layoutview::Hit(enum view_MouseAction  action	/* which button; what it did */, long  x				/* where the mouse points */, long  y, long  numberOfClicks		/* number of hits at same place */)
{
    struct component *c;
    class view *result;
    class view *child;
    char whereline[80];

    if (layout_debug)
	printf("layoutview_Hit(%d, %ld, %ld, %ld)\n", (int) action, x, y, numberOfClicks);

    if ((this)->Hitmode() == HIT_DRAGGING || (this)->Hitmode() == HIT_CREATING)
	DrawRubberBox(this);

    c = FindContainingComponent(this, x, y, 0);

    /* pass hit to child when running or initializing */

    if ((this)->Hitmode() == HIT_INITIALIZING && c != (this)->Selection()) {
	(this)->SetAuthoringMode();
	(this)->SetSelection( c);
    }
    if ((this)->Hitmode() == HIT_AUTHORING && (action == view_LeftDown || action == view_RightDown) && c != NULL && numberOfClicks > 1) {
	(this)->SetInitMode();
	numberOfClicks -= 1;
    }

    if (c != NULL && (((this)->Hitmode() == HIT_EXEC && cVaries(c)) || (this)->Hitmode() == HIT_INITIALIZING)) {
	(this)->SetSelection( c);

	child = (this)->FindSubview( c);

	if (child == NULL) {
	    if (layout_debug)
		printf("Null hit at %ld %ld\n", x - vLeft(this, c), y - vTop(this, c));
	    result = NULL;
	}

	else {
	    if (layout_debug)
		printf("Passing hit to child %p at %ld %ld\n", child, x - vLeft(this, c), y - vTop(this, c));
	    result = (child)->Hit( action, x - vLeft(this, c), y - vTop(this, c), numberOfClicks);
	}

	if (layout_debug)
	    printf("Child hit returned %p\n", result);
	if (result != NULL)
	    return result;
    }

    /* create new component or drag existing one */

    else if ((action == view_LeftDown || action == view_RightDown) && (this)->Hitmode() == HIT_AUTHORING) { // tjm - changed logic
	c = FindContainingComponent(this, x, y, POINTINGTHRESHHOLD);
	(this)->SetSelection( c);
	this->dragx = x;
	this->dragy = y;
	if (c == NULL) {

	    /* initialize create */
	    this->dragleft = this->dragtop = FALSE;
	    this->dragright = this->dragbottom = TRUE;
	    this->rubberleft = (this)->ApplyGranularity( this->dragx);
	    this->rubbertop = (this)->ApplyGranularity( this->dragy);
	    this->rubberheight = this->rubberwidth = 0;
	}
	else {

	    /* initialize drag */

	    this->dragleft = x < vLeft(this, c) + POINTINGTHRESHHOLD && x < vRight(this, c);
	    this->dragright = x >= vRight(this, c) - POINTINGTHRESHHOLD && x >= vLeft(this, c);
	    this->dragtop = y < vTop(this, c) + POINTINGTHRESHHOLD && y < vBottom(this, c);
	    this->dragbottom = y >= vBottom(this, c) - POINTINGTHRESHHOLD && y >= vTop(this, c);
	    if (!this->dragleft && !this->dragright && !this->dragtop && !this->dragbottom)
		this->dragleft = this->dragright = this->dragtop = this->dragbottom = TRUE;
	}
    }

    /* continue creating or dragging */

    else if ((action == view_RightUp || action == view_RightMovement // tjm - changed logic
	       || action == view_LeftUp || action == view_LeftMovement) &&
	      ((this)->Hitmode() == HIT_CREATING || (this)->Hitmode() == HIT_DRAGGING
	       || (this)->Hitmode() == HIT_AUTHORING)
	       && (x < this->dragx - MOTIONTHRESHHOLD
		|| x > this->dragx + MOTIONTHRESHHOLD
		|| y < this->dragy - MOTIONTHRESHHOLD
		|| y > this->dragy + MOTIONTHRESHHOLD)) {

	c = (this)->Selection();

	/* do not drag background, but OK to resize it */
	if ((this)->Hitmode() == HIT_AUTHORING
	    && this->dragleft && this->dragright && this->dragtop && this->dragbottom
	    && cLeft(c) <= 0 && cTop(c) <= 0 && cWidth(c) <= 0 && cHeight(c) <= 0) {
	    c = NULL;
	    (this)->SetSelection( c);
	    this->dragleft = this->dragtop = FALSE;
	    this->dragright = this->dragbottom = TRUE;
	    this->rubberleft = (this)->ApplyGranularity( this->dragx);
	    this->rubbertop = (this)->ApplyGranularity( this->dragy);
	    this->rubberwidth = this->rubberheight = 0;
	}

	if (c == NULL ) {

	    /* actually begin creating */

	    if ((this)->Hitmode() == HIT_AUTHORING)
		this->hitmode = HIT_CREATING;

	    /* continue creating */

	    this->rubberwidth = (this)->ApplyGranularity( x - this->dragx);
	    this->rubberheight = (this)->ApplyGranularity( y - this->dragy);
	    if (action == view_LeftUp || action == view_RightUp) {

		/* finish creating */

		message::DisplayString(&getView(this), 0, "");
		if ((this)->SetSelectionSize( this->rubberleft, this->rubbertop, this->rubberwidth, this->rubberheight)) {
		    (this)->SetInitMode();
		    if (cData((this)->Selection()) == NULL) {
			if ((this)->Createmode() == CREATE_FILLER)
			    (this)->SetChildByName( "filler");
			else if ((this)->Createmode() == CREATE_PASTE)
			    (this)->Paste();
		    }
		}
		else {
		    (this)->SetSelection( NULL);
		    (this)->SetAuthoringMode();
		}
	    }
	}
	else {
	    if ((this)->Hitmode() == HIT_AUTHORING) {

		/* actually begin dragging */

		this->hitmode = HIT_DRAGGING;
		SetRubberBox(this, this->dragx, this->dragy);
		DrawRubberBox(this);
	    }

	    /* continue dragging */

	    SetRubberBox(this, x, y);
	    if (action == view_LeftUp || action == view_RightUp) {

		/* finish dragging */

		DrawRubberBox(this);
		message::DisplayString(&getView(this), 0, "");
		(this)->SetSelectionSize( this->rubberleft, this->rubbertop, this->rubberwidth, this->rubberheight);
		this->hitmode = HIT_AUTHORING;
		if (layout_debug)
		    printf("Done dragging, back to authoring\n");
	    }
	}
    }

    if (!this->hasInputFocus && (this)->Hitmode() != HIT_INITIALIZING)
	(this)->WantInputFocus( &getView(this));

    if ((this)->Hitmode() == HIT_DRAGGING || (this)->Hitmode() == HIT_CREATING) {
	DrawRubberBox(this);
	*whereline = '\0';
	if (this->dragleft)
	    sprintf(whereline + strlen(whereline), "left =%4ld ", this->rubberleft);
	if (this->dragright)
	    sprintf(whereline + strlen(whereline), "right =%4ld ", this->rubberleft + this->rubberwidth);
	if (this->dragtop)
	    sprintf(whereline + strlen(whereline), "top =%4ld ", this->rubbertop);
	if (this->dragbottom)
	    sprintf(whereline + strlen(whereline), "bottom =%4ld ", this->rubbertop + this->rubberheight);
	if (this->dragleft != this->dragright)
	    sprintf(whereline + strlen(whereline), "width =%4ld ", this->rubberwidth);
	if (this->dragtop != this->dragbottom)
	    sprintf(whereline + strlen(whereline), "height =%4ld ", this->rubberheight);
	message::DisplayString(&getView(this), 0, whereline);
    }

    return &getView(this);
}

/* input focus obtained; highlight something and post menus */

void
layoutview::ReceiveInputFocus()
{
    if (layout_debug)
	printf("layoutview_ReceiveInputFocus\n");

    if (!(this->hasInputFocus)) {
	this->hasInputFocus = TRUE;
	this->keystatep->next = NULL;
	(this)->PostKeyState( this->keystatep);
	(this)->PostMenus( NULL);
    }
}

/* input focus lost; remove highlighting */

void
layoutview::LoseInputFocus()
{
    if (layout_debug)
	printf("layoutview_LoseInputFocus\n");

    if (this->hasInputFocus) {
	this->hasInputFocus = FALSE;
    }
}

/* handle request to post menus */

void
layoutview::PostMenus(class menulist	 *ml			/* list of menus to post */)
{
    if (layout_debug) {
	if (ml == NULL)
	    printf("layoutview_PostMenus NULL\n");
	else
	    printf("layoutview_PostMenus %p %s\n", ml, viewname((class view *)ml->object));
    }

    (this->menulistp)->UnchainML( CHILD_MENULIST_KEY);
    if (ml != NULL)
	(this->menulistp)->ChainAfterML( ml, CHILD_MENULIST_KEY);

    (this)->view::PostMenus( this->menulistp);
}

/* handle child's request for a new size */

void
layoutview::WantNewSize(class view  *requestor			/* view requesting a new size */)
{
    struct layoutviewlist *vl;
    struct component *c;
    long dWidth, dHeight;

    if (layout_debug)
	printf("layoutview_WantNewSize(%s)\n", viewname(requestor));

    forallsubviews(this, vl) {
	if (vChild(vl) == requestor) {
	    c = vComponent(vl);
	    (requestor)->DesiredSize( vWidth(this, c), vHeight(this, c), view_NoSet, &dWidth, &dHeight);
	    if (layout_debug)
		printf(" .. ignored %ld %ld\n", dWidth, dHeight);
	}
    }
}

/* destroy selected component including its view */

void
layoutview::DestroyComponent()
{
    struct layoutviewlist *vl;
    class view *child;
    FILE *cutFile;
    char buffer[81];

    if (layout_debug)
	printf("layoutview_DestroyComponent(%s)\n", GetClassName(this, cData((this)->Selection())));

    if ((this)->Selection() == NULL)
	return;
    child = NULL;
    forallsubviews(this, vl)
      if (vComponent(vl) == (this)->Selection()) {
	  child = vChild(vl);
	  break;
      }
    if (child != NULL) {
	(this)->RemoveSubview( child); /* prevents removing component in view_Destroy */
	(child)->UnlinkTree();
	(child)->Destroy();
    }
    if (cData((this)->Selection()) != NULL ) {
	cutFile = ((this)->GetIM())->ToCutBuffer();
	(cData((this)->Selection()))->Write( cutFile, im::GetWriteID(), 0);
	((this)->GetIM())->CloseToCutBuffer( cutFile);
	sprintf(buffer, "Poof!  Your %s vanishes into the cutbuffer.", GetClassName(this, cData((this)->Selection())));
	message::DisplayString(this, 0, buffer);
    }
    else
	message::DisplayString(this, 0, "Poof!");
    (getLayout(this))->RemoveComponent( (this)->Selection());
    (this)->SetSelection( NULL);
}

/* destroy all subviews of a layout */

static void
DestroySubviews(class layoutview  *self)
{
    struct layoutviewlist *vl;
    class view *child;

    while (self->subviews != NULL) {
	vl = self->subviews;
	child = vChild(vl);
	(child)->UnlinkTree();
	(child)->Destroy();
    }
}

/* tear down a layoutview */

layoutview::~layoutview()
{
	ATKinit;

    if (layout_debug)
	printf("layoutview_FinalizeObject\n");

    DestroySubviews(this);
    delete this->menulistp;
    delete this->keystatep;
}

/* build tree of views */

void
layoutview::LinkTree(class view  *parent		/* parent into which to link self */)
{
    struct layoutviewlist *vl;

    if (layout_debug)
	printf("layoutview_LinkTree to %s\n", viewname(parent));

    forallsubviews(this, vl)
      (vChild(vl))->LinkTree( this);

    (this)->view::LinkTree( parent);
}

/* notification that observed object changed */

void
layoutview::ObservedChanged(class observable  *changed		/* that which changed */, long  status				/* OBJECTDESTROYED is used to signal deletion */)
{
    struct component *c;

    if (layout_debug)
	printf("layoutview_ObservedChanged(%ld)\n", status);

    if (changed	== this->GetDataObject()) {	/* if it is my dataobject */

	if (status == observable_OBJECTDESTROYED)
	    DestroySubviews(this);
	(this)->WantUpdate( &getView(this));
    }

    else if (status == observable_OBJECTDESTROYED) {
	c = (this)->RemoveSubview( (class view *)changed);
	if (c != NULL) {
	    (getLayout(this))->RemoveComponent( c);
	    if (layout_debug)
		printf("removing subview %s\n", GetClassName(this, cData(c)));
	}
    }

    else { /* not my dataobject */
	(this)->view::ObservedChanged( changed, status);

    }
}

/* toggle layout_debug */

void
layoutview::ToggleDebug()
{

/*
    the rather peculiar if statement below is designed to work properly
    in either the case that layout and layoutview are loaded separately or
    loaded together (ie two or one copies of 'debug'
*/
    if (layout_debug) {
	(getLayout(this))->ToggleDebug();
	layout_debug = 0;
    } else {
	(getLayout(this))->ToggleDebug();
	layout_debug = 1;
    }
}

/* enter execution mode */

void
layoutview::SetRunMode()
{
    if (layout_debug)
	printf("layoutview_SetRunMode() hitmode=%d\n", (this)->Hitmode());

    if ((this)->Hitmode() != HIT_EXEC) {
	if ((this->menulistp)->SetMask( layoutview_EXEC_MASK))
	    (this)->PostMenus( NULL);
	(this)->WantUpdate( &getView(this));
	this->lastUpdate = -1;
	this->hitmode = HIT_EXEC;
    }
}

/* enter initialization mode */

void
layoutview::SetInitMode()
{
    if (layout_debug)
	printf("layoutview_SetInitMode() hitmode=%d\n", (this)->Hitmode());

    if ((this)->Hitmode() != HIT_INITIALIZING) {
	if ((this->menulistp)->SetMask( layoutview_EXEC_MASK))
	    (this)->PostMenus( NULL);
	if ((this)->Hitmode() != HIT_AUTHORING) {
	    (this)->WantUpdate( &getView(this));
	    this->lastUpdate = -1;
	}
	this->hitmode = HIT_INITIALIZING;
    }
}

/* enter authoring mode */

void
layoutview::SetAuthoringMode()
{
    if (layout_debug)
	printf("layoutview_SetAuthoringMode() hitmode=%d\n", (this)->Hitmode());

    if ((this)->Hitmode() != HIT_AUTHORING) {
	SetAuthoringMask(this);
	if ((this)->Hitmode() != HIT_INITIALIZING) {
	    (this)->WantUpdate( &getView(this));
	    this->lastUpdate = -1;
	}
	this->hitmode = HIT_AUTHORING;
    }
}

/* past object into selected component */

void
layoutview::Paste()
{
    FILE *pasteFile;
    long objectID;
    const char *objectName;

    if (layout_debug)
	printf("layoutview_Paste()\n");

    if((this)->Selection() == NULL)
	return;
    pasteFile = ((this)->GetIM())->FromCutBuffer();
    objectName = filetype::Lookup(pasteFile, NULL, &objectID, NULL); /* For now, ignore attributes. */
    if(objectName == NULL)
	objectName = "text";
    (this)->ReplaceComponent( (this)->Selection(), objectName);
    if (cData((this)->Selection()) != NULL)
	(cData((this)->Selection()))->Read( pasteFile, objectID);
    ((this)->GetIM())->CloseFromCutBuffer( pasteFile);
    InitComponent(this);
}

/* insert dataobject by name */

void
layoutview::SetChildByName(const char  *dataname				/* dataobject name */)
{
    if(Selection()==NULL) return;
    
    if (dataname == NULL || *dataname == '\0')
	dataname = "filler";

    if (layout_debug)
	printf("layoutview_SetChildByName(%s)\n", dataname);

    (this)->ReplaceComponent( (this)->Selection(), dataname);
    InitComponent(this);
}

/* prompt for name and insert data object */

void
layoutview::SetChild()
{
    char dataname[100];

    if(Selection()==NULL) {
        message::DisplayString(this, 0, "Please select the component to receive the data object.");
        return;
    }
    dataname[0] = '\0';
    if (message::AskForString(&getView(this), 0, "Data object to enter here (filler): ", "", dataname, sizeof dataname))
	return;
    (this)->SetChildByName( dataname);
}

/* promote selection to front */

void
layoutview::Promote()
{
    if (layout_debug)
	printf("layoutview_Promote()\n");

    if ((this)->Selection() != NULL)
	(getLayout(this))->Promote( (this)->Selection());
}


/* demote selection to back of list */

void
layoutview::Demote()
{
    if (layout_debug)
	printf("layoutview_Demote()\n");

    if ((this)->Selection() != NULL)
	(getLayout(this))->Demote( (this)->Selection());
}

/* fill background of layout with selection */

void
layoutview::MakeBackground()
{
    if (layout_debug)
	printf("layoutview_MakeBackground()\n");

    if ((this)->Selection() != NULL) {
	(getLayout(this))->Demote( (this)->Selection());
	(getLayout(this))->SetComponentSize( (this)->Selection(), 0, 0, 0, 0);
	SetAuthoringMask(this);
    }
}

/* Allow selected component to vary */

void
layoutview::MakeVariable()
{
    if (layout_debug)
	printf("layoutview_MakeVariable()\n");

    if ((this)->Selection() != NULL) {
	(getLayout(this))->MakeVariable( (this)->Selection());
	SetAuthoringMask(this);
    }
}

/* Require selected component to vary only when authoring */

void
layoutview::MakeFixed()
{
    if (layout_debug)
	printf("layoutview_MakeFixed()\n");

    if ((this)->Selection() != NULL) {
	(getLayout(this))->MakeFixed( (this)->Selection());
	SetAuthoringMask(this);
    }
}

/* Do not fill newly created data objects */

void
layoutview::SetCreateMode(enum createmode_enum  createmode)
{
    if (layout_debug)
	printf("layoutview_SetCreateMode(%d)\n", createmode);

    this->createmode = createmode;
}

/* Set granularity */

void
layoutview::SetGranularity(int  granularity)
{
    if (layout_debug)
	printf("layoutview_SetGranularity(%d)\n", granularity);

    this->granularity = granularity;
}

/* print components back to front */

static void
PrintComponents(class layoutview  *self, FILE  *f, const char  *processor			/* processor */, const char  *finalFormat			/* final format */, struct component  *c			/* current component to be printed */, int  saveno				/* number of state-restoring macro */)
{
    class view *child;
    struct rectangle childRect;

    /* use recursion to print from back to front */

    if (c == NULL)
	return;
    PrintComponents(self, f, processor, finalFormat, c->nextcomponent, saveno);

    /* OK with those behind, now print this individual component */

    child = (self)->FindSubview( c);
    fprintf(f, "\\\"component: %s\n", viewname(child));

    fprintf(f, ".rs\n");			/* be sure we are really spacing */
    fprintf(f, ".sp %ldp\n", vTop(self, c));		/* space to top of component */
    fprintf(f, ".in \\n(.iu+%ldp\n", vLeft(self, c));	/* indent to left of component */
    if (cWidth(c) > 0)
	fprintf(f, ".ll \\n(.iu+%ldp\n", cWidth(c));	/* set width for component */

    if (child) {
	rectangle_SetRectSize(&childRect, vLeft(self, c), vTop(self, c), vWidth(self, c), vHeight(self, c));
	(child)->InsertView( self, &childRect);
	(child)->Print( f, processor, finalFormat, FALSE);
    }

    fprintf(f, ".br\n");			/* flush line buffer */
    fprintf(f, ".%d\n", saveno);		/* restore state */
    fprintf(f, ".rt\n");			/* return to top of layout */
}

/* print as part of larger document */

void
layoutview::Print(FILE  *f					/* output file */, const char  *processor				/* processor */, const char  *finalFormat				/* final format */, boolean	 toplevel				/* am I the top level view? */)
{
    long height;
    struct component *c;
    static int saveno = 89;

    if (layout_debug)
	printf("layoutview_Print(%p, %p, %s, %s, %d)\n", this, f, processor, finalFormat, toplevel);

    /* set up  top-level stuff */

    if (ATK::LoadClass("texttroff") == NULL)
	printf("Can't load texttroff - document initializtion missing.\n");
    if (toplevel && ATK::IsLoaded("texttroff")) {
	texttroff::BeginDoc(f);
	fprintf(f, ".nr IN 0\n");	/* suppress indent at outer level */
	fprintf(f, ".nr PO 0\n");	/* ditto left margin */
	fprintf(f, ".nr HM 0\n");	/* ditto header margin */
	fprintf(f, ".nr FM 0\n");	/* ditto footer margin */
	fprintf(f, ".RS\n");
    }

    fprintf(f, "\\\"layout begins\n");

    /* save state in macro 90, 91, ... */

    saveno += 1;
    fprintf(f, ".de %d\n", saveno);	/* macro to restore state */
    fprintf(f, ".if \\n(.u .fi\n");	/* filling */
    fprintf(f, ".if \\n(.j .ad\n");	/* adjusting */
    fprintf(f, ".if \\n(.j=0 .na\n");
    fprintf(f, ".ft \\n(.f\n");		/* font */
    fprintf(f, ".ps \\n(.s\n");		/* point size */
    fprintf(f, ".vs \\n(.vu\n");	/* vertical spacing */
    fprintf(f, ".ll \\n(.lu\n");	/* line length */
    fprintf(f, ".in \\n(.iu\n");	/* indentation */
    fprintf(f, "..\n");

    /* ask for necessary space on page and remember starting point */

    height = 0;
    forallcomponents(getLayout(this), c) {
	if (cHeight(c) > 0 && height < cBottom(c))
	    height = cBottom(c);
    }

    fprintf(f, ".rs\n");		/* be sure we are really spacing */
    fprintf(f, ".sp -\\n(VSu\n");	/* compensate for baseline offset */
    fprintf(f, ".ne %ldp\n", height);	/* insure enough space to next trap */
    fprintf(f, ".mk\n");		/* mark this starting position */

    /* print all components back to front */

    PrintComponents(this, f, processor, finalFormat, (getLayout(this))->GetFirstComponent(), saveno);

    fprintf(f, ".sp %ldp\n", height);	/* space to bottom of layout */
    saveno -= 1;
    fprintf(f, "\\\"layout ends\n");

    if (toplevel && ATK::IsLoaded("texttroff"))
	texttroff::EndDoc(f);

    (this)->WantUpdate( &getView(this));		/* redraw to fix up visual regions */
    this->lastUpdate = -1;
}
