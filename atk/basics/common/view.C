/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

#include <andrewos.h>
ATK_IMPL("view.H")

#include <im.H>
#include <atom.H>
#include <atomlist.H>
#include <environ.H>
#include <dataobject.H>
#include <fontdesc.H>
#include <graphic.H>
//#include <describer.H>
#include <view.H>

#define min(v1,v2) ((v1)<(v2) ? (v1) : (v2))
#define view_STARTHEIGHT 150
static const class atom * A_name;
static const class atom * A_atomlist;
static const class atom * A_context;
static const class atom *A_printoption, *A_printer, *A_psfile, *A_papersize, *A_file, *A_string;

static struct view_printopt view_printoptels[6]; /* printer; PS file; print to printer/file; landscape/portrait; print scale; paper size */
static struct view_printoptlist view_printopts = {
    view_printoptels,
    6,
    NULL
};

ATKdefineRegistry(view, observable, view::InitializeClass);
static void view_SetDefaultColors(class view  *self , class view  *parent);
static void EnsureName(class view  * self);


boolean view::InitializeClass()
{
    A_name = atom::Intern("name");
    A_context = atom::Intern("context");
    A_atomlist = atom::Intern("atomlist");
    A_printoption = atom::Intern("printoption");
    A_printer = atom::Intern("printer");
    A_psfile = atom::Intern("psfile");
    A_papersize = atom::Intern("papersize");
    A_string = atom::Intern("string");
    A_file = atom::Intern("file");

    view_printoptels[0].name = A_printer;
    view_printoptels[0].type = A_string;
    view_printoptels[0].label = "Name of printer";
    view_printoptels[1].name = A_psfile;
    view_printoptels[1].type = A_file;
    view_printoptels[1].label = "Name of PS file";
    view_printoptels[2].name = atom::Intern("tofile");
    view_printoptels[2].type = atom::Intern("boolean");
    view_printoptels[2].label = "Print to PS file";
    view_printoptels[3].name = atom::Intern("landscape");
    view_printoptels[3].type = atom::Intern("boolean");
    view_printoptels[3].label = "Landscape mode";
    view_printoptels[4].name = atom::Intern("scale");
    view_printoptels[4].type = atom::Intern("int");
    view_printoptels[4].label = "Print scale (percent)";
    view_printoptels[5].name = A_papersize;
    view_printoptels[5].type = A_string;
    view_printoptels[5].label = "Paper size";
    return TRUE;
}

class ScrollInterface *view::GetScrollInterface() {
    // no default ScrollInterface yet.
    return NULL;
}

view::view()
        {
	ATKinit;

    this->imPtr = NULL;
    this->parent = NULL;
    this->dataobject = NULL;
    this->drawable = graphic::CreateGraphic(this);
    this->name = NULL;
    this->className = NULL;
    this->name_explicitly_set = FALSE;
    THROWONFAILURE( TRUE);
}

view::~view()
{
	ATKinit;
    
    if (this->dataobject != NULL)  {
	(this->dataobject)->RemoveObserver( this);
	(this->dataobject)->Destroy();
	this->dataobject=NULL;
    }

    if (this->name != NULL) {
	delete this->name;
	this->name=NULL;
    }

    if(this->className!=NULL) {
	delete this->className;
	this->className=NULL;
    }

    if (this->drawable != NULL)  {
	(this->drawable)->Destroy();
	this->drawable=NULL;
    }
}

boolean view::AcceptingFocus() {
    return TRUE;
}

void view::ChildLosingInputFocus() {
}

void view::ChildReceivingInputFocus() {
}

//void view::Traverse(enum view_Traversal) {
//}

void view::ObservedChanged(class observable  *changed, long  value)
{
    if (changed == (class observable *) this->dataobject)  {
	if (value == observable_OBJECTDESTROYED)
	    this->dataobject = NULL;
	else
	    (this)->WantUpdate( this);
    }
}

void view::SetDataObject(class dataobject  *dataobject)
{
    class atomlist *newname;
    class atomlist *context;

    if(this->dataobject==dataobject)
	return;

    if (this->dataobject != NULL) {
	(this->dataobject)->RemoveObserver( this);
	(this->dataobject)->Destroy();
    }

    this->dataobject=dataobject;
    if (dataobject != NULL) {
	(dataobject)->AddObserver( this);
	(dataobject)->Reference();
    }

    if (!this->name_explicitly_set) {
	if (dataobject != NULL && 
	    (dataobject)->Get( A_name, &A_atomlist, (long *) &newname))
	    this->name = atomlist::Copy(newname);
	else if (this->name == NULL)
	    this->name = atomlist::StringToAtomlist((this)->GetTypeName());
    }
    if (this->className == NULL)
	this->className = atomlist::StringToAtomlist((this)->GetTypeName());

    if (dataobject != NULL && 
	 (dataobject)->Get( A_context, &A_atomlist, (long *) &context))
    {
	(this->name)->JoinToBeginning( context);
	(this->className)->JoinToBeginning( context);
    }
}

class view *view::GetApplicationLayer()
    {
    return this;
}

void view::DeleteApplicationLayer(class view  *applicationLayer)
        {
}


#if 0

char * view::DescriptionObject(char  * format,long  rock /* supposed to be an aribtrarily pointer */)
            {
#define	MaxObjName 128
    /* The default is to take the name of the view, and if it is appended by the string "view", replace "view" with format, otherwise append format. If we don't have any format, then use "describer" as the format */

    static char tmpString[MaxObjName];	    /* Name of describer object */
    const char * viewName;		    /* Name of current view */
    short viewLen;		    /* Length of view name */
    short formatLen;		    /* Length of format description */
    short viewCopyAmount, formatCopyAmount; /* temps for building describer name */

    /* First see if we have a format name, if not, then we will just use
       "describer" */
    if (!format) format = "describer";
    formatLen = strlen(format);

    viewName = (char *)(this)->GetTypeName();
    viewLen = strlen(viewName);
    /* Should we use strncmp or explode the tests? Depends on when we expect to fail */
    if ( (viewLen >= 4) && (viewName[viewLen-4] == 'v') &&
	 (viewName[viewLen-3] == 'i') && (viewName[viewLen-2] == 'e') &&
	 (viewName[viewLen-1] == 'w')) {
	/* We have a cannonical "object||view" name, so replace "view"
	   by the format type */
	viewCopyAmount = min(viewLen-4,MaxObjName-1);
	 }
    else {
	viewCopyAmount = min(viewLen,MaxObjName-1);
    }

    /* Copy over view name (or as much as will fit) */
    strncpy(tmpString,viewName,viewCopyAmount);

    /* See how much of format name can fit */
    if ((viewCopyAmount + formatLen) > (MaxObjName - 1))
	    formatCopyAmount = (MaxObjName - 1) - viewCopyAmount;
    else formatCopyAmount = formatLen;

    /* Copy over format (or as  much as will fit) */
    strncpy(tmpString+viewCopyAmount,format,formatCopyAmount);

    /* Terminate the string */
    tmpString[viewCopyAmount+formatCopyAmount] = '\0';

    /* Note: we reuse the space, hope there are not multiple calls */
    return tmpString;
}

enum view_DescriberErrs view::Describe(char  * format,FILE  * file,long  rock)
                {
    class describer * descObject;
    char * descObjectName;

    /* Get the name of the object to be used for format conversion */
    descObjectName = (this)->DescriptionObject(format,rock);

    /* Make sure we have the name of the description object */
    if (!descObjectName) return(view_NoDescribeString);

    /* Try to load the object */
    descObject = (class describer *) ATK::NewObject(descObjectName);

    /* Make sure we got the object */
    if (!descObject) return(view_NoDescribeObject);

    /* Check to see if the class is correct */
    /* How is this done now adays? Do I have to walk up the classinfo tree myself */
    if (!ATK::IsTypeByName(descObjectName,"describer")) 
	return(view_WrongSubclassDescription);

    /* Everything seems to be in order, call the description procedure */
    return (descObject)->Describe(this,format,file,rock);

}

#endif // 0


void view::FullUpdate(enum view_UpdateType  type, long  left, long  top, long  width, long  height)
                        {
}

void view::Update()
    {
}

void view::Print(FILE  *file, const char  *processor, const char  *finalFormat, boolean  topLevel)
                    {
}

class view *view::Hit(enum view_MouseAction  action, long  x, long  y, long  numberOfClicks)
                    {
    return this;
}

view_DSattributes view::DesiredSize(long  width, long  height, enum view_DSpass  pass, long  *dWidth, long  *dHeight)
                              {
    *dWidth = width;
    *dHeight = (height > 2048) ? view_STARTHEIGHT :height;
    return (view_DSattributes)(view_HeightFlexible | view_WidthFlexible);
}

void view::GetOrigin(long  width, long  height, long  *originX, long  *originY)
                    {
    *originX = 0;
    *originY = 0;
}

void view::GetPrintOrigin(long  width, long  height, long  *originX, long  *originY)
                    {
    *originX = 0;
    *originY = 0;
}

void view::ReceiveInputFocus()
    {
    if(this->parent!=NULL){
	(this)->PostMenus(NULL);
	(this)->PostKeyState(NULL);
    }
}

void view::LoseInputFocus()
    {
}

void view::WantExposure(class view *requestor, struct rectangle *childrect) {
    /* requestor is a direct descendant of this view, childrect is in the descendant's coordinate system.  This default
     implementation should convert childrect from the child's coordinates to this view's coordinates. and call
     parent->WantExposure(this,childrect).  A real implementation should scroll so that the indicated portion of the child
     is visible in the visual region of this view. */
    
    if(requestor!=this) {
	childrect->top+=requestor->GetEnclosedTop();
	childrect->left+=requestor->GetEnclosedLeft();
    }
    if(parent) parent->WantExposure(this,childrect);
}

void view::WantUpdate(class view  *requestor)
        {
    if (this->parent != NULL)
	(this->parent)->WantUpdate( requestor);
}

void view::WantInputFocus(class view  *requestor)
        {
   if (this->parent != NULL)
	(this->parent)->WantInputFocus( requestor);
}

void view::WantNewSize(class view  *requestor)
        {
/* This function tree climbs one level only. */
    if (this == requestor && this->parent != NULL)
        (this->parent)->WantNewSize( requestor);
}

ATK  * view::WantHandler(const char  *handlerName)
        {
    if (this->parent != NULL)
	return (this->parent)->WantHandler( handlerName);
    else
	return NULL;
}

const char *view::WantInformation(const char  *key)
        {
    if (this->parent != NULL)
	return (this->parent)->WantInformation( key);
    else
	return NULL;
}

void view::PostKeyState(class keystate  *keystate)
        {
    if (this->parent != NULL)
	(this->parent)->PostKeyState( keystate);
}

void view::PostMenus(class menulist  *menulist)
        {
    if (this->parent != NULL)
	(this->parent)->PostMenus( menulist);
}

void view::RetractCursor(class cursor  *cursor)
        {
    if (this->parent != NULL)
	(this->parent)->RetractCursor( cursor);
}
void view::RetractViewCursors(class view  *requestor)
        {
    if (this->parent != NULL)
	(this->parent)->RetractViewCursors( requestor);
}
void view::PostCursor(struct rectangle  *rec, class cursor  *cursor)
            {
    if (this->parent != NULL)
	(this->parent)->PostCursor( rec, cursor);
}

void view::PostDefaultHandler(const char  *handlerName, ATK   *handler)
            {
    if (this->parent != NULL)
	(this->parent)->PostDefaultHandler( handlerName, handler);
}

static void
view_SetDefaultColors(class view  *self , class view  *parent)
  {
    const char *foregroundColor, *backgroundColor;
    if ((self)->GetIM()) {
	graphic::GetDefaultColors(&foregroundColor, &backgroundColor);
	if (foregroundColor != NULL)
	    (self)->SetForegroundColor( foregroundColor, 0, 0, 0);
	if (backgroundColor != NULL)
	    (self)->SetBackgroundColor( backgroundColor, 0, 0, 0);
    }
}

void view::InsertViewRegion(class view  *parent, class region  *region)
{
    (this->drawable)->InsertGraphicRegion( parent->drawable, region);
    this->imPtr = parent->imPtr;
    this->parent = parent;
}

void view::InsertView(class view  *parent, struct rectangle  *enclosingRectangle)
        {
    (this->drawable)->InsertGraphic( parent->drawable, enclosingRectangle);
    this->imPtr = parent->imPtr;
    this->parent = parent;
}

void view::InsertViewSize(class view  *parent,long  xOriginInParent ,long  yOriginInParent , long  width , long  height)
{
    (this->drawable)->InsertGraphicSize( parent->drawable, xOriginInParent, yOriginInParent, width, height);
    this->imPtr = parent->imPtr;
    this->parent = parent;
}


const void *view::GetInterface(const char  *type)
        /* Note: This routine is a placeholder for a future function. It is needed now to get scrollbars to work. - William Lott. */
{
    return NULL;
}

void view::LinkTree(class view  *parent)
        {

    if (parent != NULL) {
	if(this->drawable) {
	    this->imPtr = parent->imPtr;
	    this->parent = parent;
	    if((this)->GetIM()) {
		class colormap **inherited = (parent)->GetInheritedColormap(), **current;
		current = (parent)->CurrentColormap();
		if(*inherited != *current) {
		    if(!current || !*current)
			printf("Setting a NULL inherited colormap\n");
		    (this)->SetInheritedColormap( current);
		}
		else {
		    if(!inherited || !*inherited)
			printf("Setting a NULL inherited colormap\n");
		    (this)->SetInheritedColormap( inherited);
		}
	    }
	    drawable->InsertGraphicSize(parent->drawable, 0, 0, 0, 0);
	    if((this)->GetIM()) view_SetDefaultColors(this, parent);
	}
    }
    else {
        this->imPtr = NULL;
	if(this->drawable) {
	    rectangle_SetRectSize(&this->drawable->localBounds, 0, 0, 0, 0);
	    (this)->ClearInheritedColormap();
	}
    }
}

void view::UnlinkTree()
    {
    class view *parent = this->parent;

    if (parent != NULL)
	(parent)->UnlinkNotification( this);
    
    (this)->LinkTree( NULL);
    this->parent = NULL;
}

void view::UnlinkNotification(class view  *unlinkedTree)
        {

    if (this->parent != NULL)
        (this->parent)->UnlinkNotification( unlinkedTree);
}

boolean view::IsAncestor(class view  *possibleAncestor)
{
    class view *self=this;
    while (self != NULL && self != possibleAncestor)
        self = self->parent;
    return self != NULL;
}

void view::ExposeSelf(boolean recurse)
{
    if (this->parent) {
	this->parent->ExposeChild(this);

	/* now expose parent, if recurse is TRUE */
	if (recurse) {
	    this->parent->ExposeSelf(TRUE);
	}
    }
}

void view::ExposeChild(class view *v)
{
    /* expose child -- no action, by default */
}

const char * view::GetWindowManagerType()
{
    if ((this)->drawable) return ((this)->drawable)->GetWindowManagerType();
    return "";
}

long view::GetDevice()
{
    if ((this)->drawable) return ((this)->drawable)->GetDevice();
    return 0;
}

static void EnsureName(class view  * self)
     {
  if (self->name == NULL)
    self->name = atomlist::StringToAtomlist((self)->GetTypeName());
  if (self->className == NULL)
    self->className = atomlist::StringToAtomlist((self)->GetTypeName());
}

void view::SetName( class atomlist  * name )
          {
  this->name = atomlist::Copy(name);
  this->name_explicitly_set = TRUE;
}

class atomlist * view::GetName( )
     {
  EnsureName(this);
  return this->name;
}

class atomlist * view::GetClass( )
     {
  EnsureName(this);
  return this->className;
}



short view::GetParameter( class atomlist  * name, const class atom  * type, long  * data )
                    {
  class atomlist * dup;
  short val;
  dup = atomlist::Copy(name);
  val = (this)->GetResource(  name, dup, type, data );
  delete dup;
  return val;
}


short view::GetResource( class atomlist  * name, class atomlist  * class_c, const class atom  * type, long  * data )
                         {
  struct atoms * nameMark = (name )->Mark( );
  struct atoms * classMark = (class_c )->Mark( );
  short gotit = FALSE;

  EnsureName(this);

  if (this->parent != NULL)
    {
      (name)->JoinToBeginning(  this->name );
      (class_c)->JoinToBeginning(  this->className );
      gotit = (this->parent)->GetResource( name, class_c, type, data );
      (name)->Cut(  nameMark );
      (class_c)->Cut(  classMark );
    }
  return gotit;
}

void view::GetManyParameters( struct resourceList  * resources, class atomlist  * name, class atomlist  * class_c )
                    {
  struct atoms * nameMark = NULL;
  struct atoms * classMark = NULL;
  class atomlist * passname;
  class atomlist * passclass;
  EnsureName(this);
  if (this->parent != NULL)
    {
      if (name == NULL)
	passname = this->name;
      else
	{
	  nameMark = (name)->Mark();
	  (name)->JoinToBeginning(this->name);
	  passname = name;
	}
      if (class_c == NULL)
	passclass = this->className;
      else
	{
	  classMark = (class_c)->Mark();
	  (class_c)->JoinToBeginning(this->className);
	  passclass = class_c;
	}
      (this->parent)->GetManyParameters(  resources, passname, passclass );
      if (name != NULL)
	(name)->Cut(nameMark);
      if (class_c != NULL)
	(class_c)->Cut(classMark);
    }
}

void view::PostResource( class atomlist  * path, const class atom  * type, long  data )
                    {
  struct atoms * pathMark = (path )->Mark( );
  EnsureName(this);
  if (this->parent != NULL)
    {
      (path)->JoinToBeginning(  this->name );
      (this->parent)->PostResource(  path, type, data );
      (path)->Cut(  pathMark );
    }
}


void view::InitChildren()
{
    /*
      All parent views are responsible for overriding this method.
      This is a request to instantiate all one's children.
      i.e. ensure that all child views 
      1. exist and
      2. are linked into the view tree.
 
      view_InitChild() should be called recursively on all children so 
	  this call filters down the view tree. 
    */
}

boolean view::CanView(const char  *TypeName)
{
    /* 
      Views should return TRUE or FALSE depending on whether they are
      capable of viewing dataobjects of the specified name.
      Currently returns TRUE by default for backward compatibility.
	  */
    return TRUE;
}

/* Stubs for selection code. */
void view::LoseSelectionOwnership()
{
    /* nothing needs done here */
}

long view::WriteSelection(FILE  *out)
{
    /* if this is called on a view which doesn't override it there is an error.*/
    return -1;
}

boolean view::RecSearch(struct SearchPattern *pat, boolean toplevel)
{
    return FALSE;
}

boolean view::RecSrchResume(struct SearchPattern *pat)
{
    return FALSE;
}

boolean view::RecSrchReplace(class dataobject *text, long pos, long len)
{
    return FALSE;
}

void view::RecSrchExpose(const struct rectangle &logical, struct rectangle &hit) {
    hit=logical;
}

void view::WantColormap( class view  *requestor, class colormap  **cmap)
{
    if(this->parent)
	(this->parent)->WantColormap( requestor, cmap);
}

void view::ReceiveColormap( class colormap  *cmap )
{
}

void view::LoseColormap( class colormap  *cmap )
{
}

void view::PrintPSDoc(FILE *outfile, long pagew, long pageh)
{
    /* do nothing. Since this does not call print::PSNewPage, the print package will know that this inset type does not print as a top-level document. */
}

void *view::GetPSPrintInterface(const char *printtype)
{
    return NULL;
}

void view::DesiredPrintSize(long width, long height, enum view_DSpass pass, long *desiredwidth, long *desiredheight) 
{
    /* the default is for the inset to use the same strategy for requesting a print size as it does when requesting a view size. */
    this->DesiredSize(width, height, pass, desiredwidth, desiredheight);
}

void view::PrintPSRect(FILE *outfile, long logwidth, long logheight, struct rectangle *visrect)
{
}

	boolean 
view::Gifify(const char *filename, long *pmaxw, long *pmaxh, 
			struct rectangle *visrect) {
	return FALSE;
}

struct view_printoptlist *view::PrintOptions()
{
    return &view_printopts;
}

/* remember, strings (and files) returned from this method are static and may be overwritten by the next call. */
long view::GetPrintOption(const class atom *popt)
{
    const char *prname;
    long value;
    short gotit;
    class dataobject *dobj = this->GetDataObject();

    if (popt==A_printer) {
	/* printer */
	prname = environ::GetPrinter(); /* environment var */
	if (!prname)
	    prname = environ::GetProfile("printer"); /* preference */
	if (!prname)
	    prname = "";
	return (long)prname;
    }
    else if (popt==view_printoptels[1].name) {
	/* file */
	gotit = dobj ? dobj->Get(popt, &A_printoption, &value) : FALSE;
	if (!gotit) {
	    /* figure out a good name. Oh one called Kirok, what is this thing you call WantInformation()? */
	    static char *stro = NULL;
	    const char *str = this->WantInformation("filename");
	    if (stro)
		free(stro);
	    if (str) {
		stro = (char *)malloc(strlen(str)+8);
		sprintf(stro, "%s.ps", str);
	    }
	    else {
		stro = (char *)malloc(16);
		strcpy(stro, "output.ps");
	    }
	    value = (long)stro;
	}
	return value;
    }
    else if (popt==view_printoptels[2].name) {
	/* print to printer/file */
	gotit = dobj ? dobj->Get(popt, &A_printoption, &value) : FALSE;
	if (!gotit) 
	    value = environ::GetProfileSwitch("PrintToPSFile", FALSE);
	return value;
    }
    else if (popt==view_printoptels[3].name) {
	/* landscape/portrait */
	gotit = dobj ? dobj->Get(popt, &A_printoption, &value) : FALSE;
	if (!gotit) {
	    value = !environ::GetProfileSwitch("Portrait", TRUE);
	}
	return value;
    }
    else if (popt==view_printoptels[4].name) {
	/* print scale */
	gotit = dobj ? dobj->Get(popt, &A_printoption, &value) : FALSE;
	if (!gotit) 
	    value = environ::GetProfileInt("PrintScale", 100);
	return value;
    }
    else if (popt==view_printoptels[5].name) {
	/* paper size */
	gotit = dobj ? dobj->Get(popt, &A_printoption, &value) : FALSE;
	if (!gotit)
	    value = (long)environ::GetProfile("PaperSize");
	if (!value)
	    value = (long)"letter";
	return value;
    }
    else {
	/* generic handling facility */
	gotit = dobj ? dobj->Get(popt, &A_printoption, &value) : FALSE;
	if (!gotit) 
	    return 0;
	return value;
    }
}

/* strings (and files) sent to this method are copied into storage */
void view::SetPrintOption(struct view_printopt *vopt, long newval)
{
    class dataobject *dobj;

    if (vopt->name == A_printer) {
	char *str = (char *)newval;
	if (!str || strlen(str)==0)
	    environ::DeletePrinter();
	else
	    environ::PutPrinter((char *)newval);
    }
    else {
	/* generic handling facility */
	dobj = this->GetDataObject();
	if (!dobj)
	    return;
	if (vopt->type == A_string || vopt->type == A_file) {
	    long oldval;
	    char *str;
	    short gotit = dobj->Get(vopt->name, &A_printoption, &oldval);
	    if (gotit && oldval)
		free((char *)oldval);
	    str = strdup((char *)newval);
	    dobj->Put(vopt->name, A_printoption, (long)str);
	}
	else
	    dobj->Put(vopt->name, A_printoption, newval);
    }
}
