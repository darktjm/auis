/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

#define viewname(v) ((v) == NULL ? "<NO VIEW>" : (((v)->GetName())->First())->Name())
#define classname(do) ((do) == NULL ? "<NO OBJECT>" : (do)->GetTypeName())

/* internal interfaces */

#define getView(self) (*(self))
#define getBox(self) ((class box *)(self)->GetDataObject())

#include <andrewos.h>
ATK_IMPL("boxview.H")
#include <assert.h>
#include <rect.h>

#include <atomlist.H>
#include <dataobject.H>
#include <filetype.H>
#include <im.H>
#include <message.H>
#include <region.H>
#include <texttroff.H>
#include <view.H>

#include <box.H>
#include <boxview.H>

static boolean boxview_debug=0;
/* replace child view with another object */

			/* forward reference */


ATKdefineRegistry(boxview, view, boxview::InitializeClass);
void InitChild(class boxview  *self);
void Update(class boxview  *self, enum view_UpdateType  how		/* kind of update */, struct rectangle  *updateRect		/* rectangle affected; or NULL for update */, boolean  contentsChanged		/* contents changed since last update */);
void RequestUpdate(class boxview  *self);


static void
ReplaceChild(class boxview  *self, class view  *child			/* child to be replaced */, const char  *dataname				/* name of replacement dataobject */)
{
    char foo[81];

    if (boxview_debug)
	printf("ReplaceChild(%s, %s)\n", viewname(child), dataname);

    assert(child == self->child || self->child == NULL);

    if(! ATK::IsTypeByName(dataname, "dataobject")){
        sprintf(foo, "%s is not a dataobject", dataname);
	message::DisplayString(&getView(self), 0, foo);
        return;
    }

    if (child != NULL) {
	(child)->UnlinkTree();
	(child)->Destroy();
	self->child = NULL;
    }

    (getBox(self))->FillInContents( dataname);
    InitChild(self);
    if (self->child != NULL)
	(self->child)->WantInputFocus( self->child);
}

/* initialize child view corresponding to box contents */

void
InitChild(class boxview  *self)
{
    const char *subviewname;			/* name for new view */

    if (getBox(self)->contents == NULL) {
	ReplaceChild(self, self->child, "filler");
    }

    if (self->child != NULL)
	return;

    subviewname = (getBox(self)->contents)->ViewName();
    self->child = (class view *) ATK::NewObject(subviewname);
    if (self->child == NULL) {
	printf("unable to create child view\n");
	return;
    }
    (self->child)->SetDataObject( getBox(self)->contents);
    (self->child)->LinkTree( &getView(self));

    if (boxview_debug)
	printf("InitChild created %s\n", subviewname); 
}

boolean					/* always returns TRUE */
boxview::InitializeClass()
{

    return TRUE;
}

/* initialize box view */

boxview::boxview()
{
	ATKinit;

    this->updateNeeded = FALSE;
    this->lastUpdate = 0;
    this->child = NULL;
    /* InitChild(self); */
    THROWONFAILURE( TRUE);
}

/* get width of box */

int					/* returns width of box */
boxview::BoxWidth()
{
    return 3;
}

/* negotiate size of view */

view_DSattributes			/* returns indication of what it wants */
boxview::DesiredSize(long  width				/* width being offered by parent */, long  height				/* height being offered */, enum view_DSpass  pass			/* what parent is willing to give */, long  *dWidth				/* set to desired width */, long  *dHeight				/* set to desired height */)

/*  boxview asks for space for its child plus the box proper */

{
    long desiredWidth;
    long desiredHeight;
    view_DSattributes rc;
    int tw = (this)->BoxWidth();

    if (boxview_debug)
	printf("boxview_DesiredSize(, %ld, %ld, %d, .. )\n", width, height, (int)pass);

    InitChild(this);
    if (this->child == NULL) {
	desiredWidth = desiredHeight = 0;
	rc = view_WidthFlexible | view_HeightFlexible;
    }
    else 
	rc = (this->child)->DesiredSize( width - 2*tw, height - 2*tw, pass, &desiredWidth, &desiredHeight);

    *dWidth = desiredWidth + 2*tw;
    *dHeight = desiredHeight + 2*tw;

    return rc;
}

/* draw box proper */

void
boxview::DrawBox()
{
    (this)->DrawRectSize( (this)->GetLogicalLeft(), (this)->GetLogicalTop(), (this)->GetLogicalWidth() - 1, (this)->GetLogicalHeight() - 1);
}

/* update image */

void
Update(class boxview  *self, enum view_UpdateType  how		/* kind of update */, struct rectangle  *updateRect		/* rectangle affected; or NULL for update */, boolean  contentsChanged		/* contents changed since last update */)
{
    class region *updateRegion;	/* region for this update */
    class region *remainingRegion;	/* region to be updated */
    struct rectangle r;			/* child rectangle */
    class region *childRegion;		/* visual region available to child */
    int tw = (self)->BoxWidth();

    /* make sure child is filled in*/

    InitChild(self);

    /* initialize region being updated */

    remainingRegion = (self)->GetVisualRegion( region::CreateEmptyRegion());
    if (updateRect != NULL) {
	updateRegion = region::CreateRectRegion(updateRect);
	(remainingRegion)->IntersectRegion( updateRegion, remainingRegion);
	delete updateRegion;
    }
    (self)->SetClippingRegion( remainingRegion);
    if (contentsChanged)
	(self)->EraseVisualRect();

    /* redraw box */

    (self)->DrawBox();

    /* update child */

    rectangle_SetRectSize(&r, (self)->GetLogicalLeft() + tw, (self)->GetLogicalTop() + tw, (self)->GetLogicalWidth() - 2*tw, (self)->GetLogicalHeight() - 2*tw);
    childRegion = region::CreateRectRegion(&r);
    (childRegion)->IntersectRegion( remainingRegion, childRegion);
    delete remainingRegion;
    (self)->SetClippingRegion( childRegion);

    if (!contentsChanged)
	/* do nothing */ ;
    else if (self->child == NULL) {
	(self)->FillRect( &r, (self)->BlackPattern());
    }
    else if (updateRect != NULL) {
	if (boxview_debug)
	    printf("FullUpdating %s\n", (getBox(self)->contents)->ViewName());
	(self->child)->InsertView( self, &r);
	(childRegion)->OffsetRegion( -tw, -tw);
	(self->child)->SetVisualRegion( childRegion);
	(self->child)->FullUpdate( how, rectangle_Left(updateRect), rectangle_Top(updateRect), rectangle_Width(updateRect), rectangle_Height(updateRect));
    }
    else {
	if (boxview_debug)
	    printf("Redrawing %s\n", (getBox(self)->contents)->ViewName());
	(self->child)->InsertView( self, &r);
	(childRegion)->OffsetRegion( -tw, -tw);
	(self->child)->SetVisualRegion( childRegion);
	(self->child)->FullUpdate( view_FullRedraw, 0, 0, rectangle_Width(&r), rectangle_Height(&r));
    }
    delete childRegion;
}

/* full update when window changes */

void
boxview::FullUpdate(enum view_UpdateType  how		/* kind of update */, long  left , long  top , long  width , long  height		/* updated rectangle (for certain kinds; */)
{
    struct rectangle cliprect;		/* actual updated rectangle */

    if (boxview_debug)
	printf("boxview_FullUpdate(%d, %ld, %ld, %ld, %ld)\n", (int)how, left, top, width, height);

    /* define rectangle actually being updated */

    if (how == view_PartialRedraw || how == view_LastPartialRedraw)
	rectangle_SetRectSize(&cliprect, left, top, width, height);
    else {
	rectangle_SetRectSize(&cliprect, (this)->GetVisualLeft(), (this)->GetVisualTop(), (this)->GetVisualWidth(), (this)->GetVisualHeight());
    }
    /* perform the update */

    (this)->SetTransferMode( graphic_COPY);
    ::Update(this, how, &cliprect, TRUE);
    if (how == view_FullRedraw) {
	this->updateNeeded = FALSE;
	this->lastUpdate = (getBox(this))->GetModified();
    }
}

/* partial update in response to WantUpdate request */

void
boxview::Update()
{
    if (boxview_debug)
	printf("boxview_Update needed=%d\n", this->updateNeeded);

    ::Update(this, view_FullRedraw, NULL, (this->lastUpdate < (getBox(this))->GetModified()));
    this->updateNeeded = FALSE;
    this->lastUpdate = (getBox(this))->GetModified();
}

/* process mouse hit */

class view *				/* returns view which should get follow-up events*/
boxview::Hit(enum view_MouseAction  action		/* which button; what it did */, long  x , long  y				/* where the mouse points */, long  numberOfClicks			/* number of hits at same place */)
{
    int tw = (this)->BoxWidth();

    if (boxview_debug)
	printf("boxview_Hit(%d, %ld, %ld, %ld)\n", (int) action, x, y, numberOfClicks);

    if (this->child == NULL)
	return &getView(this);
    else 
	return (this->child)->Hit( action, x - tw, y - tw, numberOfClicks);
}

/* update request */

void
RequestUpdate(class boxview  *self)
{
    if (boxview_debug)
	printf("RequestUpdate() already=%d\n", self->updateNeeded);

    if (!self->updateNeeded) {
	self->updateNeeded = TRUE;
	(self)->WantUpdate( &getView(self));
    }
}

/* handle child's request for a new size */

void
boxview::WantNewSize(class view	 *requestor		/* view requesting a new size */)
{
    if (boxview_debug)
	printf("boxview_WantNewSize(%s)\n", viewname(requestor));

    if (this->child == requestor && getView(this).parent != NULL) {
	(getView(this).parent)->WantNewSize( this);
    }
}

/* unlink a view for a component */

void
boxview::UnlinkNotification(class view	 *unlinkedview		/* view being unlinked */)
{
    if (boxview_debug)
	printf("boxview_UnlinkNotification %s\n", viewname(unlinkedview));

    if (unlinkedview == this->child)
	this->child = NULL;

    (this)->view::UnlinkNotification( unlinkedview);
}

/* tear down a boxview */

boxview::~boxview()
{
	ATKinit;

    if (boxview_debug)
	printf("boxview_FinalizeObject\n");

}

/* build tree of views */

void
boxview::LinkTree(class view  *parent			/* parent into which to link self */)
{

    if (boxview_debug)
	printf("boxview_LinkTree to %s\n", viewname(parent));

    if (this->child != NULL)
      (this->child)->LinkTree( this);

    (this)->view::LinkTree( parent);
}

/* notification that observed object changed */

void
boxview::ObservedChanged(class observable  *changed		/* that which changed */, long  status				/* OBJECTDESTROYED is used to signal deletion */)
{
    if (boxview_debug)
	printf("boxview_ObservedChanged(%ld)\n", status);

    if (changed	!= this->GetDataObject())	/* if it is not my dataobject */
	(this)->view::ObservedChanged( changed, status);

    else { /* my dataobject has changed */

	if (status != observable_OBJECTDESTROYED)
	    RequestUpdate(this);
    }
}

/* toggle boxview_debug */

void
boxview::ToggleDebug()
{

/*
    the rather peculiar if statement below is designed to work properly
    in either the case that box and boxview are loaded separately or
    loaded together (ie two or one copies of 'debug')
*/
    if (boxview_debug) {
	(getBox(this))->ToggleDebug();
	boxview_debug = 0;
    } else {
	(getBox(this))->ToggleDebug();
	boxview_debug = 1;
    }
}

/* past object into selected component */

void
boxview::Paste()
{
    FILE *pasteFile;
    long objectID;
    const char *objectName;

    if (boxview_debug)
	printf("boxview_Paste()\n");

    pasteFile = ((this)->GetIM())->FromCutBuffer();
    objectName = filetype::Lookup(pasteFile, NULL, &objectID, NULL); /* For now, ignore attributes. */
    if(objectName == NULL)
	objectName = "text";
    ReplaceChild(this, this->child, objectName);
    if (getBox(this)->contents != NULL)
	(getBox(this)->contents)->Read( pasteFile, objectID);
    ((this)->GetIM())->CloseFromCutBuffer( pasteFile);

}

/* print as part of larger document */

void
boxview::Print(FILE  *f			/* output file */, const char  *processor		/* processor */, const char  *finalFormat		/* final format */, boolean	 toplevel		/* am I the top level view? */)
{
    int tw = (this)->BoxWidth();

    if (boxview_debug)
	printf("boxview_Print(%p, %p, %s, %s, %d)\n", this, f, processor, finalFormat, toplevel);

    /* set up  top-level stuff */

    if (ATK::LoadClass("texttroff") == NULL)
	printf("Can't load texttroff - document initializtion missing.\n");
    if (toplevel && ATK::IsLoaded("texttroff"))
	texttroff::BeginDoc(f);

    InitChild(this);

    fprintf(f, ".sp -\\n(VSu\n");	/* move up one line */
    fprintf(f, "\\D'l %ldp 0'", (this)->GetLogicalWidth());  /* draw box */
    fprintf(f, "\\D'l 0 %ldp'", (this)->GetLogicalHeight());
    fprintf(f, "\\D'l %ldp 0'", -(this)->GetLogicalWidth());
    fprintf(f, "\\D'l 0 %ldp'", -(this)->GetLogicalHeight());
    fprintf(f, "\n");			/* I think this moves back down a line */

    fprintf(f, ".sp %dp\n", tw);	/* vertical down to enclosed object */
    fprintf(f, ".in +%dp\n", tw);	/* indent for box */
    fprintf(f, ".ll -%dp\n", 2 * tw);

    if (this->child) {
	(this->child)->InsertViewSize( this, (this)->GetLogicalLeft() + tw, (this)->GetLogicalTop() + tw, (this)->GetLogicalWidth() - 2*tw, (this)->GetLogicalHeight() - 2*tw);
	(this->child)->Print( f, processor, finalFormat, FALSE);
    }

    fprintf(f, ".in -%dp\n", tw);	/* remove box indentation */
    fprintf(f, ".ll +%dp\n", 2 * tw);

    if (toplevel && ATK::IsLoaded("texttroff"))
	texttroff::EndDoc(f);
}
